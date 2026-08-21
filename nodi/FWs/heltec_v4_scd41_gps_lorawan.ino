/**
 * Heltec WiFi LoRa 32 V4 - SCD41 + L76K GPS - LoRaWAN OTAA con deep sleep
 * =====================================================================
 *
 * Payload: schema 0x42 (SCD41 + L76K + batteria), 32 byte totali.
 * Formato coerente col decoder JavaScript delle dashboard multi/single.
 *
 * Attivazione: OTAA di default (USE_OTAA=1). Il codice supporta anche ABP
 * (USE_OTAA=0), ma OTAA e' la modalita' testata e consigliata.
 *
 * Strategia di persistenza LoRaWAN (importante per capire il resto del file):
 *   - SESSIONE (contiene il FCnt): vive SOLO in RTC memory. Sopravvive ai
 *     deep sleep (che sono la norma) ma non ai power-off. Non viene mai
 *     scritta in NVS -> zero usura flash in esercizio normale.
 *   - NONCES OTAA (DevNonce/JoinNonce): persistiti in NVS. Sopravvivono ai
 *     power-off, cosi' un rejoin non riusa un DevNonce (che il NS
 *     rifiuterebbe come replay).
 *   - Conseguenza: a un power-off/reflash la sessione si perde e il device
 *     rifa' un join OTAA (una volta, ~7s), operazione sicura proprio grazie
 *     ai nonces persistiti. Nessun rischio di FCnt che "torna indietro".
 *
 * Hardware:
 *   - Heltec WiFi LoRa 32 V4 (ESP32-S3R2 + SX1262, senza OLED)
 *   - Sensirion SCD41 (CO2 / temp / RH) su I2C GPIO 7 (SDA) / 6 (SCL),
 *     alimentato da Vext (spegnibile in deep sleep)
 *   - Quectel L76K GNSS su connettore SH1.25-8P della V4:
 *       GPS TX -> GPIO 39 (RX ESP)
 *       GPS RX <- GPIO 38 (TX ESP)
 *       VGNSS_Ctrl = GPIO 34 (HIGH = GPS on)
 *
 * Librerie richieste (Arduino IDE Library Manager):
 *   - RadioLib                  (Jan Gromeš, >= 6.6)
 *   - TinyGPSPlus               (Mikal Hart)
 *   - Sensirion I2C SCD4x       (Sensirion, ufficiale)
 *   - Preferences               (già inclusa nel core ESP32)
 *
 * Board Manager URL:
 *   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *
 * Board selection in Arduino IDE:
 *   "Heltec WiFi LoRa 32(V3)"  -->  usa lo stesso pinout della V4
 *   USB CDC On Boot: Enabled
 *   Flash Size: 8MB
 *   Partition Scheme: 8M with spiffs
 *
 * Credenziali: sostituire AppKey (OTAA) o DevAddr/NwkSKey/AppSKey (ABP)
 * con quelle registrate sul Network Server (ChirpStack).
 */

#include <RadioLib.h>
#include <TinyGPSPlus.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include <Preferences.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>
#include <WiFi.h>
#include <esp_bt.h>
#include <esp_task_wdt.h>
#include "esp32s3/rtc.h"

// =============================================================
// CONFIGURAZIONE
// =============================================================

// ---- Intervallo di trasmissione ----
// Indice nell'array TX_INTERVAL_SECONDS (definito piu' sotto).
// Valore di FABBRICA: la config runtime salvata in NVS (chiave "tx_int")
// puo' sovrascriverlo se modificato via downlink FPort 20 comando 0x11.
// Al primo boot (NVS vuota) si usa questo valore.
//   0 = 10s (test veloci)
//   1 = 20s
//   2 = 1min (default produzione)
//   3 = 2min
//   4 = 5min
//   5 = 10min
//   6 = 30min
//   7 = 1h
//   8 = STORAGE (deep sleep infinito, wake solo su RST - vedi modulo STORAGE MODE)
#define TX_INTERVAL_PRESET  2

// ---- Modalita' DEBUG ----
// Se attivo (1), sostituisce il deep sleep con un delay normale + restart,
// cosi' l'USB CDC non si stacca e vedi i log nel Monitor Seriale.
// Da disattivare (0) prima di mettere il device a batteria.
#define DEBUG_NO_DEEP_SLEEP  0

const uint32_t TX_INTERVAL_SECONDS[] = {
    10, 20, 60, 120, 300, 600, 1800, 3600, 3600*24, 3600*24*7, 0
//  0   1   2    3    4    5    6     7    8=1d     9=1w      10=STORAGE (0 = infinito)
};
#define TX_PRESET_MAX     10
#define TX_PRESET_STORAGE 10

// L'intervallo di trasmissione runtime e' TX_INTERVAL_SECONDS[cfgTxIntervalPreset]
// dove cfgTxIntervalPreset e' letto da NVS (o default TX_INTERVAL_PRESET) al boot.
// NOTA: preset 10 = STORAGE MODE, il valore 0 nell'array non viene mai usato
// direttamente: la logica STORAGE MODE bypassa esp_sleep_enable_timer_wakeup().
// Preset 8 (1 giorno) e 9 (1 settimana) usano il deep sleep normale con timer:
// il device dorme a lungo ma si sveglia periodicamente per un ciclo TX regolare,
// mantenendo la responsivita' ai downlink (Class A: RX window dopo ogni TX).

// ---- Timeout attesa fix GPS ----
// Al boot, il device aspetta un fix GPS valido per questo tempo massimo.
// Se il timeout scade senza fix, si procede comunque con l'ultima posizione
// nota (in RTC memory) marcata come "stale" (HDOP = 9999).
//
// COLD start (primo fix dopo power-on): il GPS deve scaricare almanacco
// e ephemerides dai satelliti. Puo' richiedere 30-90 secondi anche con
// buona visibilita' del cielo.
//
// WARM start (fix successivi, con dati in cache): il GPS ha gia' almanacco
// e ephemerides valide. Il fix arriva tipicamente in 1-10 secondi.
//
// COLD e' usato al primo boot dopo power-on (hasWarmData==false in RTC).
// WARM e' usato ai boot successivi durante il ciclo di sleep/wake.
// COLD e' modificabile via downlink FPort 20 comando 0x14, WARM e' fisso.
#define GPS_FIX_TIMEOUT_COLD_S  90
#define GPS_FIX_TIMEOUT_WARM_S  30

// ---- Datarate LoRaWAN (Spreading Factor) ----
// Lo Spreading Factor determina il tradeoff velocita' vs robustezza:
//   SF7  = massima velocita' (~5.5 kbps), airtime minimo (~60ms per 32B),
//          minima portata (~2km LOS)
//   SF12 = minima velocita' (~250 bps), airtime massimo (~1500ms per 32B),
//          massima portata (~15km LOS)
// Consumo energetico scala con l'airtime: SF12 consuma ~25x di SF7 per TX.
//
// Consigliato: SF9 per test iniziali (compromesso robustezza/consumo).
// In modalita' OTAA, ADR (Adaptive Data Rate) sovrascrive questo valore
// autonomamente in base alla qualita' del link. In ABP resta fisso.
//
// Valore di FABBRICA, sovrascrivibile via downlink FPort 20 comando 0x12.
#define LORAWAN_SF  9

// ---- Credenziali ABP ----
// Usate solo se USE_OTAA = 0
// DevAddr: 4 byte
uint32_t devAddr = 0x260B262C;

// NwkSKey e AppSKey: 16 byte ciascuno (MSB first)
uint8_t nwkSKey[16] = {
    0xC7, 0x7F, 0xB7, 0x8C, 0x37, 0x7A, 0xDB, 0x11,
    0x80, 0x3F, 0x5B, 0xD3, 0x1D, 0x5B, 0x4F, 0xC2
};
uint8_t appSKey[16] = {
    0x7A, 0x95, 0x96, 0x3A, 0xB4, 0xAD, 0xC6, 0xE7,
    0xEA, 0x25, 0x19, 0x11, 0x4B, 0xD4, 0xAD, 0xF3
};

// ---- Credenziali OTAA ----
// Usate solo se USE_OTAA = 1
// AppEUI (JoinEUI in LoRaWAN 1.1): identita' dell'applicazione, 8 byte.
// Spesso zeri per progetti privati (non richiede registrazione IEEE).
// Alcuni NS pretendono un valore non-zero: verifica sulla UI ChirpStack.
const uint64_t appEui = 0x0000000000000000ULL;

// AppKey: chiave master di 16 byte. Deve corrispondere ESATTAMENTE al valore
// configurato in ChirpStack per questo device (Device Profile → OTAA keys).
// Genera un valore random e usalo qui, in ChirpStack incolla lo stesso.
uint8_t appKey[16] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};

// Il DevEUI e' comune ad ABP e OTAA: viene derivato dal MAC address del chip
// tramite la funzione getDevEuiFromMac() (definita piu' avanti nel file).

// FPort applicativo per gli uplink (misure ambientali).
// FPort e' un campo LoRaWAN che discrimina il tipo di messaggio a livello
// di rete (non di payload). Convenzioni del progetto:
//   FPort 1  = uplink misure (questo)
//   FPort 2  = uplink state (config runtime + info diagnostiche)
//   FPort 10 = downlink comandi ordinari (reboot, identify, get_state, ...)
//   FPort 20 = downlink configurazione persistente (set_tx_interval, ...)
#define LORAWAN_FPORT        1
#define LORAWAN_FPORT_STATE  2

// ---- Protezione batteria da under-discharge (soglia software) ----
// Le celle litio-ione (LiPo, 18650, 14500) si danneggiano irreversibilmente
// se scaricate sotto ~2.5 V: la struttura cristallina interna collassa e la
// cella perde capacita' o entra in un "deep discharge" da cui non si riprende
// nemmeno con ricarica successiva. Nel peggiore dei casi puo' gonfiarsi o
// prendere fuoco alla ricarica.
//
// Questa protezione software rileva quando la tensione scende sotto una
// soglia critica e mette il device in "emergency sleep": deep sleep di lunga
// durata (ore) senza TX, per consumare il minimo indispensabile.
//
// Utile SOPRATTUTTO se il device e' sigillato in un case senza accesso
// visivo o senza monitoraggio remoto affidabile: senza questa protezione
// una batteria scarica non ha alcun modo di essere "protetta" dal firmware,
// e la cella si degrada silenziosamente giorno dopo giorno.
//
// Metti a 0 per disabilitare (es. durante test con alimentatore da banco).
#define ENABLE_BATTERY_PROTECTION  1

// Soglie in millivolt (LiPo 1S standard)
#define VBAT_EMERGENCY_MV     3100    // sotto questa soglia: emergency sleep
#define VBAT_RECOVERY_MV      3300    // sopra: ripresa operativita' normale
#define VBAT_EMERGENCY_SLEEP_S  21600 // 6 ore di sleep in emergenza

// ---- Persistenza LoRaWAN: cosa vive dove ----
// Se attiva (1), la NVS (flash) viene usata per persistere i NONCES OTAA e la
// runtime config. La SESSIONE LoRaWAN (che contiene il FCnt) NON va in NVS:
// vive solo in RTC memory.
//
// Caratteristiche di sopravvivenza dei due storage:
//   RTC memory  : sopravvive al deep sleep. Persa a power-off e reset HW.
//   NVS (flash) : sopravvive a power-off, reset HW e reflash del firmware
//                 (finche' la partition NVS non viene sovrascritta - vale
//                 con lo schema di partizioni standard di Arduino ESP32).
//
// Mappatura adottata (vedi anche l'header in cima al file):
//   SESSIONE  -> solo RTC memory. Aggiornata dopo ogni TX (cacheSessionInRTC).
//                Il deep sleep la conserva, quindi il FCnt incrementa tra i
//                cicli. Un power-off la perde -> il device rifa' un join.
//   NONCES    -> RTC memory + NVS. La NVS li fa sopravvivere al power-off,
//                cosi' un rejoin non riusa un DevNonce gia' bruciato.
//   CONFIG    -> NVS (chiavi tx_int, lora_sf, ...), scritta solo ai downlink.
//
// Perche' NON salviamo la sessione in NVS: la flash ha vita limitata (~100k
// scritture/settore) e salvarla ad ogni ciclo la consumerebbe; salvarla di
// rado (es. ogni N cicli) esporrebbe invece al rischio di riprendere, dopo un
// power-off, un FCnt piu' basso di quello gia' trasmesso -> il NS scarterebbe
// gli uplink come replay finche' il contatore non risale. Rifare un join
// pulito e' piu' semplice e robusto, ed e' gratis in termini di flash.
//
// Disattiva (0) solo per test rapidi in cui non vuoi toccare la flash. Con
// OTAA e' sconsigliato: senza nonces persistiti, dopo un power-off il device
// puo' riusare un DevNonce e il join viene rifiutato (vedi #warning sotto).
#define ENABLE_NVS_PERSISTENCE  1

// Namespace NVS. Preferences organizza i dati in namespace (max 15 char) e
// key (max 15 char). Sotto lo stesso namespace "lora" convivono i nonces OTAA
// e le chiavi di runtime config.
#define NVS_NAMESPACE         "lora"

// Chiave NVS per il session buffer. In OTAA (USE_OTAA=1) NON e' usata: la
// sessione vive solo in RTC. Resta perche' le funzioni save/loadSessionFromNVS
// la referenziano e sono usate dal ramo ABP e dalla modalita' debug.
#define NVS_KEY_FCNT_UP       "fcnt_up"

// Parametro storico NON PIU' USATO. Nella versione attuale la sessione non
// viene mai salvata in NVS in nessuna modalita': in OTAA vive in RTC e si
// rigenera con un join a power-off; in ABP il ripristino non e' supportato da
// RadioLib (-1120) quindi il FCnt riparte da 0 ogni ciclo (gestito lato server
// col frame-counter check disabilitato). Lasciato solo per compatibilita'.
#define FCNT_NVS_SAVE_EVERY   200

// Chiavi NVS per configurazioni modificabili via downlink (FPort 20)
// Se una chiave e' assente, il firmware usa il default hardcoded del #define
#define NVS_KEY_TX_INTERVAL   "tx_int"     // uint8_t preset 0-10 (10=STORAGE)
#define NVS_KEY_LORAWAN_SF    "lora_sf"    // uint8_t 7-12
#define NVS_KEY_TX_POWER      "tx_pow"     // uint8_t dBm 2-14
#define NVS_KEY_GPS_TIMEOUT   "gps_t"      // uint16_t sec
#define NVS_KEY_VBAT_EMERG    "vbat_em"    // uint16_t mV
#define NVS_KEY_VBAT_RECOV    "vbat_rc"    // uint16_t mV
#define NVS_KEY_ADR           "adr"        // uint8_t 0/1

// Chiavi NVS per le nuove feature di risparmio energetico (moduli STORAGE / SKIP)
#define NVS_KEY_GPS_ENABLED   "gps_en"     // uint8_t 0/1 - modulo GPS SKIP
#define NVS_KEY_GPS_SKIP      "gps_skip"   // uint8_t 0-255 - modulo GPS SKIP
#define NVS_KEY_SCD_ENABLED   "scd_en"     // uint8_t 0/1 - modulo SCD DISABLE

// Chiave NVS per nonces OTAA (solo se USE_OTAA=1)
// Persiste DevNonce e JoinNonce cosi' il device non ripete valori dopo
// power-off (il NS rifiuterebbe il join con "MIC mismatch")
#define NVS_KEY_NONCES        "nonces"     // ~24 byte

// ---- Downlink handler ----
// Se attivo (1), il firmware interpreta i downlink LoRaWAN come comandi.
// Il codec ChirpStack fa da traduttore tra JSON (usato lato server) e byte
// binari (spediti via LoRaWAN):
//
//   FPort 10 = COMANDI DI AZIONE (one-shot, non persistenti)
//     0x01 REBOOT           - riavvio software
//     0x02 IDENTIFY         - LED lampeggia 10 volte
//     0x03 FORCE_TX_NOW     - forza un uplink extra nel prossimo ciclo
//     0x04 CLEAR_NVS        - cancella NVS (richiede byte magic 0xA5 dopo)
//
//   FPort 20 = CONFIGURAZIONE (persistente in NVS)
//     0x11 SET_TX_INTERVAL  - byte preset 0-10 (10s/20s/1m/2m/5m/10m/30m/1h/1d/1w/STORAGE)
//     0x12 SET_LORAWAN_SF   - byte SF 7-12
//     0x13 SET_TX_POWER     - byte dBm 2-14
//     0x14 SET_GPS_TIMEOUT  - uint16 LE secondi (10-300)
//     0x15 SET_BATT_THRESH  - 2x uint16 LE mV (emergency, recovery)
//
// Le config modificate via downlink sopravvivono al deep sleep, al power-off
// e al reboot. Vengono lette al boot con getConfigValueFromNVS().
//
// Metti a 0 se non vuoi che i downlink modifichino nulla (device read-only).
#define ENABLE_DOWNLINK_HANDLER  1

// ---- Attivazione LoRaWAN: ABP vs OTAA ----
// 0 = ABP  - chiavi statiche DevAddr + NwkSKey + AppSKey. La sessione LoRaWAN
//            e' gia' attiva al boot, nessuna comunicazione radio richiesta
//            per l'attivazione.
// 1 = OTAA - chiavi statiche DevEUI + AppEUI + AppKey. Il device fa un join
//            via radio al boot: manda JoinRequest, il NS risponde con
//            JoinAccept che contiene DevAddr + materiale per derivare
//            NwkSKey e AppSKey di sessione. FCnt gestito automaticamente.
//
// ===================================================================
//  DIFFERENZE OPERATIVE E CONFIG DA FARE - LEGGERE PRIMA DI CAMBIARE
// ===================================================================
//
// Cambiando questo flag NON serve modificare altro nel firmware, ma va
// riconfigurato il device sul Network Server.
//
// -------------------- OTAA (USE_OTAA = 1) -------------------------
// - Persistenza: la sessione (col FCnt) vive in RTC memory e INCREMENTA
//   tra i deep sleep. A un power-off si perde la RTC e il device rifa' un
//   join pulito (FCnt riparte da 0, legittimo). I nonces sono persistiti
//   in NVS per non riusare un DevNonce.
// - Protezione anti-replay: ATTIVA (FCnt cresce, il NS la valida).
// - Config ChirpStack: Device Profile "Device supports OTAA" = ON,
//   inserire solo AppKey (DevAddr lo assegna il NS al join).
// - E' la modalita' consigliata per la produzione.
//
// -------------------- ABP (USE_OTAA = 0) --------------------------
// - DevAddr FISSO: lo definisci tu (variabile devAddr) e va inserito
//   IDENTICO sul NS, insieme a NwkSKey e AppSKey (byte per byte).
// - Persistenza FCnt: NON possibile con RadioLib 7.x. Il ripristino di
//   sessione ABP viene scartato con -1120, quindi il FCnt RIPARTE DA 0 ad
//   ogni ciclo (ogni wake da deep sleep e ogni power-on). Verificato sul
//   campo. Dettagli tecnici nel ramo ABP di initLoRaWAN().
// - Protezione anti-replay: RINUNCIATA (vedi punto sotto). Se ti serve,
//   usa OTAA.
// - Config ChirpStack (o altro NS):
//     1) Device Profile: "Device supports OTAA" = OFF.
//     2) Inserire l'attivazione ABP: DevAddr + NwkSKey + AppSKey uguali al
//        firmware. (Le chiavi array nel codice = stringa esadecimale sul
//        NS, stesso ordine MSB-first. Se il MIC non torna, prova a
//        invertire l'endianness del solo DevAddr.)
//     3) *** DISABILITARE la validazione del frame counter *** per questo
//        device (in ChirpStack: device -> opzione "Disable frame-counter
//        validation" / "Skip frame-counter check"). SENZA questo, essendo
//        il FCnt sempre 0, il NS scarta ogni uplink dopo il primo come
//        replay. CON questo, li accetta tutti.
//   Suggerimento: per ABP crea un DEVICE NUOVO (DevEUI diverso, profilo ABP
//   puro) invece di riusare un device gia' usato in OTAA, per non
//   trascinarti dietro la vecchia sessione OTAA (DevAddr/chiavi residui).
//
// ---------------- CONFIG GATEWAY (vale per entrambe) --------------
// Sul concentratore (es. WM1302/SX1302 con chirpstack-concentratord su
// OpenWRT/RAK): se il gateway NON ha un fix GPS stabile, DISABILITA il GNSS
// nella config del concentratord (UCI: option gnss '0' nel blocco sx1302,
// poi restart). Con GNSS attivo ma senza fix, il PPS manda i downlink fuori
// finestra ("Too early to enqueue") e il device non riceve mai il JoinAccept
// (OTAA) o i downlink MAC/ADR: si vede rx ok ma tx_emitted sempre 0.
// ===================================================================
#define USE_OTAA  1

// Warning di compilazione: OTAA senza NVS = problemi al secondo power-off
// perche' il DevNonce si perde e il NS rifiuta il nuovo join come replay
#if USE_OTAA && !ENABLE_NVS_PERSISTENCE
#warning "USE_OTAA=1 senza ENABLE_NVS_PERSISTENCE: rischio di join fallito dopo power-off (DevNonce non persistito). Ok solo per test rapidi."
#endif

// ---- Watchdog ----
// Se attivo (1), il firmware arma un task watchdog hardware dell'ESP32-S3
// all'inizio del setup. Se il codice si blocca in un punto imprevisto
// (bug in una libreria, deadlock, loop infinito) e non fa reset del wdt
// entro WDT_TIMEOUT_S secondi, il chip fa un reset automatico.
//
// Il wdt viene "pettinato" nei punti chiave del ciclo (dopo GPS, dopo TX,
// prima di deep sleep) chiamando esp_task_wdt_reset().
//
// Timeout scelto: 120s. Il ciclo attivo dura tipicamente 15-90s (dominato
// dal fix GPS in cold start). Un timeout maggiore del massimo atteso
// evita reset spuri; un timeout non troppo grande permette il recupero
// veloce da situazioni anomale.
//
// Metti a 0 durante il debug con breakpoint (altrimenti il wdt resetta
// il device quando ti fermi a leggere lo stato).
#define ENABLE_WATCHDOG    1
#define WDT_TIMEOUT_S    120

// ---- Modalita' IDENTIFY persistente (option B: sveglio, blink continuo) ----
// Quando attiva, il device NON va in deep sleep: resta sveglio e lampeggia in
// continuo, ritrasmettendo ogni IDENTIFY_TX_INTERVAL_S per aprire finestre RX
// frequenti (identify_off reattivo entro un intervallo). Il GPS e' saltato.
//
// IDENTIFY_TX_INTERVAL_S: ogni quanti secondi ritrasmette durante identify.
//   Piu' corto = identify_off piu' reattivo, piu' airtime. 20s e' un buon
//   compromesso (duty cycle ampiamente sotto l'1% anche a SF9).
// IDENTIFY_BLINK_MS: mezzo periodo del lampeggio (150ms -> ~3.3 Hz, ben visibile).
// IDENTIFY_TIMEOUT_S: timeout di sicurezza. Se nessuno manda identify_off (o si
//   esce dalla portata radio), la modalita' si spegne da sola dopo N secondi.
//   Evita che un identify dimenticato tenga il device sveglio a lampeggiare e
//   trasmettere all'infinito. 1200s = 20 minuti.
#define IDENTIFY_TX_INTERVAL_S  20
#define IDENTIFY_BLINK_MS       150
#define IDENTIFY_TIMEOUT_S      1200

// ---- ADR (Adaptive Data Rate) ----
// Meccanismo standard LoRaWAN in cui il Network Server ottimizza dinamicamente
// SF e potenza TX in base alla qualita' del link. Con link buono il device
// scende a SF7 (airtime ridotto ~8x, batteria piu' lunga), con link scarso
// sale progressivamente fino a SF12.
//
// Politica adottata:
//   USE_OTAA=1 -> ADR abilitato/disabilitato secondo cfgAdr (default ENABLE_ADR)
//   USE_OTAA=0 -> ADR sempre DISABILITATO indipendentemente da cfgAdr
//                 (in ABP il device dimentica lo stato ADR ad ogni reboot,
//                 causando desync col NS; meglio non attivarlo affatto)
//
// Interazione con set_lorawan_sf (downlink FPort 20): se ADR e' attivo,
// il comando salva il valore in NVS ma emette un warning perche' ADR
// sovrascrivera' l'SF nel giro di pochi cicli. Se vuoi forzare SF manuale,
// disabilita prima ADR con set_adr_enabled.
#define ENABLE_ADR    1

// ---- FEATURES DI RISPARMIO ENERGETICO ----
// Flag compile-time per abilitare/disabilitare intere feature.
// Metti a 0 per rimuoverle completamente dal firmware (risparmi flash/RAM
// e semplifichi il flusso, se non ti servono).

// STORAGE MODE (preset TX = 6)
// Permette di mettere il device in deep sleep "infinito" via downlink
// (SET_TX_INTERVAL con preset 6). Consumo ~15 uA, wake solo su reset HW.
// Al reset con USB collegato, torna operativo automaticamente.
#define ENABLE_STORAGE_MODE       1

// GPS SKIP LOGIC (SET_GPS_SKIP)
// Permette di eseguire il fix GPS solo ogni N cicli invece di ogni ciclo.
// Enorme risparmio energetico su installazioni fisse dove la posizione
// non cambia (basta un fix ogni tanto per confermare "e' ancora li'").
#define ENABLE_GPS_SKIP           1

// GPS / SCD DISABLE (SET_GPS_ENABLED, SET_SCD_ENABLED)
// Permette di disattivare completamente GPS o SCD41 via downlink, con
// payload che riporta valori "placeholder" (0xFFFF/0x7FFF) invece dei dati.
// Utile se il device e' installato in un contesto dove alcune misure non
// servono (es. GPS indoor, o SCD41 non installato).
#define ENABLE_SENSOR_DISABLE     1

// TRIPLE RESET SAFETY NET
// Se l'utente preme RST 3 volte consecutive entro pochi secondi, la config
// runtime viene resettata ai default di fabbrica. Utile come "emergency
// unlock" se il device e' in una config sbagliata e non e' raggiungibile
// via radio (es. TX interval troppo alto, o storage mode senza USB).
#define ENABLE_TRIPLE_RST_RESET   1

// SERIAL MENU (manutenzione via USB)
// Menu interattivo su USB CDC che si apre al boot dopo un reset hardware
// (ESP_RST_EXT o ESP_RST_POWERON) se USB e' collegato. Permette
// operazioni di manutenzione senza dover passare per LoRa:
// wake da storage, reset config, status dump, force TX, clear NVS.
// Se non arrivano comandi entro SERIAL_MENU_TIMEOUT_S, il boot prosegue.
#define ENABLE_SERIAL_MENU        1

// LONG PRESS BOOT AT SETUP (spegnimento operativo hardware)
// All'inizio del setup, il firmware legge GPIO 0 (pulsante BOOT).
// Se BOOT e' premuto SUBITO DOPO un reset (RST premuto, poi BOOT
// premuto entro il boot), inizia un polling di LONG_PRESS_STORAGE_MS
// millisecondi. Se BOOT resta premuto per tutta la durata, il
// device salva cfgTxIntervalPreset=8 (STORAGE MODE) in NVS ed
// entra in deep sleep infinito.
// Se BOOT NON e' premuto al boot (99% dei casi), il polling termina
// istantaneamente e il boot prosegue normalmente. Zero costo
// energetico nel caso normale.
// USO: premi RST, poi tieni premuto BOOT per 3 secondi. LED fisso
// mentre tieni premuto, 3 lampi lunghi di conferma quando scatta.
#define ENABLE_LONG_PRESS_STORAGE 1

// ---- Schema payload ----
// Identificatore univoco del formato dei byte nel payload uplink. E' il
// primo byte di ogni pacchetto: il codec ChirpStack lo usa per decidere
// come decodificare i byte successivi. Consente evoluzioni future del
// payload senza rompere la retrocompatibilita' (basta aggiungere un nuovo
// SCHEMA_ID e gestirlo nel codec).
//   0x41 = SDS011 + BME280 (25 byte) - progetto precedente
//   0x42 = SCD41 + L76K + batteria (32 byte) - misure ambientali, FPort 1
//   0x43 = State del device (26 byte) - config runtime + diagnostica, FPort 2
#define SCHEMA_ID        0x42
#define SCHEMA_ID_STATE  0x43

// Versione del firmware (visibile nel payload state 0x43).
// Incrementa a ogni release significativa per permettere alla pagina web
// di riconoscere quali device sono aggiornati e quali no.
#define FW_VERSION  1

// =============================================================
// PIN
// =============================================================

// --- LoRa SX1262 ---
// Pin fissi dalla schematica Heltec V4 (identici a V3). NSS = chip select
// del SPI, DIO1 = interrupt di completamento TX/RX, RST = reset del chip,
// BUSY = handshake attivo durante operazioni interne del chip.
#define PIN_LORA_NSS    8
#define PIN_LORA_SCK    9
#define PIN_LORA_MOSI  10
#define PIN_LORA_MISO  11
#define PIN_LORA_RST   12
#define PIN_LORA_BUSY  13
#define PIN_LORA_DIO1  14

// --- I2C SCD41 ---
// Bus condiviso con altri eventuali sensori I2C. Il SCD41 si alimenta
// da Vext (controllato da PIN_VEXT_CTRL) quindi si spegne completamente
// in deep sleep senza consumo residuo.
#define PIN_I2C_SDA     7
#define PIN_I2C_SCL     6

// --- GPS L76K ---
#define PIN_GPS_RX     39   // GPS TX -> ESP RX
#define PIN_GPS_TX     38   // ESP TX -> GPS RX
#define PIN_VGNSS_CTRL 34   // LOW = GPS acceso, HIGH = GPS spento

// --- Alimentazione periferiche esterne ---
#define PIN_VEXT_CTRL  36   // LOW = Vext acceso (alimenta SCD41)

// --- Batteria ---
#define PIN_VBAT_READ   1   // ADC1_CH0
#define PIN_ADC_CTRL   37   // HIGH = abilita partitore VBAT (V4! sulla V3 era LOW)

// --- Indicatori ---
#define PIN_LED        35

// =============================================================
// SCHEMA PAYLOAD 0x42
// =============================================================
//
// struct '<BBBBQHhHHIIhH' - 32 byte
//
// | offset | size | tipo   | campo         | note                       |
// |--------|------|--------|---------------|----------------------------|
// | 0      | 1    | u8     | schema_id     | 0x42                       |
// | 1      | 1    | u8     | fix_quality   | 0=no, 1=GPS, 2=DGPS        |
// | 2      | 1    | u8     | satellites    | numero satelliti           |
// | 3      | 1    | u8     | battery_pct   | %                          |
// | 4      | 8    | u64    | timestamp     | epoch UTC (o uptime)       |
// | 12     | 2    | u16    | co2_ppm       | CO2 in ppm                 |
// | 14     | 2    | i16    | temp_c100     | T x 100 (centesimi °C)     |
// | 16     | 2    | u16    | hum_pct100    | RH x 100 (centesimi %)     |
// | 18     | 2    | u16    | vbat_mv       | tensione batteria in mV    |
// | 20     | 4    | i32    | lat_e7        | latitudine x 1e7           |
// | 24     | 4    | i32    | lon_e7        | longitudine x 1e7          |
// | 28     | 2    | i16    | alt_m         | altitudine (m sul mare)    |
// | 30     | 2    | u16    | hdop_x100     | HDOP x 100                 |

#pragma pack(push, 1)
struct Payload_v0x42 {
    uint8_t  schema_id;
    uint8_t  fix_quality;
    uint8_t  satellites;
    uint8_t  battery_pct;
    uint64_t timestamp;
    uint16_t co2_ppm;
    int16_t  temp_c100;
    uint16_t hum_pct100;
    uint16_t vbat_mv;
    int32_t  lat_e7;
    int32_t  lon_e7;
    int16_t  alt_m;
    uint16_t hdop_x100;
};
#pragma pack(pop)

static_assert(sizeof(Payload_v0x42) == 32, "Payload deve essere 32 byte");

// =============================================================
// SCHEMA PAYLOAD 0x43 - STATE (config runtime + diagnostica)
// =============================================================
//
// Inviato su FPort 2 (distinto da FPort 1 delle misure). La pagina web
// di configurazione lo usa per popolare i campi del form senza dover
// aspettare un uplink di misura completo.
//
// Trasmesso in tre occasioni:
//   1) Al primo boot dopo un power cycle (bootCount == 1)
//   2) Dopo ogni cambio di configurazione via downlink FPort 20
//   3) Su richiesta esplicita via comando GET_STATE (FPort 10, 0x05)
//
// Contenuto:
//   - Info diagnostiche: bootCount, uptime, batteria, reset reason
//   - Config runtime modificabili via downlink FPort 20 (cfg*)
//   - Feature flags (compile-time, informativi per l'UI)
//   - Versione firmware
//
// | offset | size | tipo   | campo                | note                      |
// |--------|------|--------|----------------------|---------------------------|
// | 0      | 1    | u8     | schema_id            | 0x43                      |
// | 1      | 4    | u32    | bootCount            | dai boot                  |
// | 5      | 4    | u32    | uptime_s             | secondi dal power-on (B)  |
// | 9      | 1    | u8     | battery_pct          | %                         |
// | 10     | 1    | u8     | cfgTxIntervalPreset  | 0-8 (8 = STORAGE MODE)    |
// | 11     | 1    | u8     | cfgLoRaWANSF         | 7-12                      |
// | 12     | 1    | u8     | cfgTxPower           | dBm 2-14                  |
// | 13     | 2    | u16    | cfgGpsTimeoutS       | secondi 10-300            |
// | 15     | 2    | u16    | cfgVbatEmergencyMv   | mV                        |
// | 17     | 2    | u16    | cfgVbatRecoveryMv    | mV                        |
// | 19     | 1    | u8     | cfgAdr               | 0/1                       |
// | 20     | 1    | u8     | featureFlags         | bit-packed (vedi sotto)   |
// | 21     | 1    | u8     | fwVersion            | FW_VERSION define         |
// | 22     | 1    | u8     | resetReason          | esp_reset_reason() coded  |
// | 23     | 1    | u8     | cfgGpsEnabled        | 0/1 (modulo GPS SKIP)     |
// | 24     | 1    | u8     | cfgGpsSkipCycles     | 0-255 (modulo GPS SKIP)   |
// | 25     | 1    | u8     | cfgScdEnabled        | 0/1 (modulo SCD DISABLE)  |
//
// featureFlags bit layout:
//   Bit 0: USE_OTAA                    (1 = OTAA, 0 = ABP)
//   Bit 1: ENABLE_NVS_PERSISTENCE      (1 = NVS attiva)
//   Bit 2: ENABLE_DOWNLINK_HANDLER     (1 = downlink processati)
//   Bit 3: ENABLE_WATCHDOG             (1 = wdt HW armato)
//   Bit 4: ENABLE_BATTERY_PROTECTION   (1 = emergency sleep abilitato)
//   Bit 5: DEBUG_NO_DEEP_SLEEP         (1 = modalita' debug con restart)
//   Bit 6: ENABLE_STORAGE_MODE         (1 = feature storage mode disponibile)
//   Bit 7: ENABLE_TRIPLE_RST_RESET     (1 = feature emergency unlock disponibile)
//
// L'uptime segue la semantica "wall-clock" (B): tempo totale dal power-on
// incluso il tempo passato in deep sleep. Utile per capire da quanto tempo
// il device e' in campo.

#pragma pack(push, 1)
struct Payload_v0x43 {
    uint8_t  schema_id;
    uint32_t bootCount;
    uint32_t uptime_s;
    uint8_t  battery_pct;
    uint8_t  cfgTxIntervalPreset;
    uint8_t  cfgLoRaWANSF;
    uint8_t  cfgTxPower;
    uint16_t cfgGpsTimeoutS;
    uint16_t cfgVbatEmergencyMv;
    uint16_t cfgVbatRecoveryMv;
    uint8_t  cfgAdr;
    uint8_t  featureFlags;
    uint8_t  fwVersion;
    uint8_t  resetReason;
    // ---- Estensione v2: features di risparmio energetico ----
    uint8_t  cfgGpsEnabled;         // 0 = GPS disattivato, 1 = attivo
    uint8_t  cfgGpsSkipCycles;      // 0 = fix ogni ciclo, N = fix ogni N+1 cicli
    uint8_t  cfgScdEnabled;         // 0 = SCD41 disattivato, 1 = attivo
};
#pragma pack(pop)

static_assert(sizeof(Payload_v0x43) == 26, "Payload state deve essere 26 byte");

// =============================================================
// OGGETTI GLOBALI
// =============================================================

// SX1262: RadioLib pinout
SX1262 radio = new Module(PIN_LORA_NSS, PIN_LORA_DIO1, PIN_LORA_RST, PIN_LORA_BUSY);

// LoRaWAN node su banda EU868, subBand 0 (default EU868).
// La modalita' effettiva (OTAA/ABP) e' scelta a runtime in initLoRaWAN()
// in base a USE_OTAA; l'oggetto node e' lo stesso per entrambe.
LoRaWANNode node(&radio, &EU868, 0);

// GPS
TinyGPSPlus gps;
HardwareSerial GpsSerial(1);   // UART1

// SCD41
SensirionI2cScd4x scd4x;

// NVS - Non-Volatile Storage (partizione dedicata nella flash SPI dell'ESP32).
// Preferences e' il wrapper Arduino ufficiale sulla libreria esp_nvs di ESP-IDF.
// Storage tipo key-value: dati organizzati in "namespace" (max 15 char) e
// dentro ogni namespace in "key" (max 15 char). I valori possono essere
// int, float, string, o blob binario (con putBytes/getBytes).
// Nel nostro caso (OTAA): namespace "lora" con la key "nonces" (buffer nonces
// OTAA) e le key di runtime config (tx_int, lora_sf, ...). La sessione NON e'
// qui: vive in RTC. Wear leveling automatico: le scritture vengono distribuite
// sui settori NVS per prolungare la vita della flash.
Preferences prefs;

// Variabili persistenti in RTC memory (sopravvivono al deep sleep,
// perdute su power-off).
RTC_DATA_ATTR uint32_t bootCount   = 0;
RTC_DATA_ATTR uint32_t lastFCntUp  = 0;
RTC_DATA_ATTR int32_t  lastLat_e7  = 0;
RTC_DATA_ATTR int32_t  lastLon_e7  = 0;
RTC_DATA_ATTR bool     hasWarmData = false;   // primo boot -> cold
RTC_DATA_ATTR bool     forceTxNow  = false;   // set da downlink FORCE_TX_NOW: prossimo ciclo TX subito

// Uptime "wall-clock" accumulato attraverso i deep sleep.
// Ad ogni deep sleep aggiungiamo (millis()/1000 + sleep_seconds) qui.
// millis() si azzera ad ogni wake, ma questa variabile persiste in RTC.
// Si azzera solo con power-off/reset HW (che azzera anche bootCount).
RTC_DATA_ATTR uint32_t uptimeAccumulatedS = 0;

// Flag per richiedere l'invio di un payload state 0x43 nel prossimo ciclo TX.
// Settato da:
//   - Comando downlink GET_STATE (FPort 10, 0x05)
//   - Dopo qualsiasi cambio config via downlink FPort 20 (feedback all'utente)
//   - Al primo boot dopo un power cycle (bootCount == 1)
// Il flag persiste in RTC memory: se settato in un ciclo e non consumato
// (es. per errore TX), viene ritentato nel ciclo successivo.
RTC_DATA_ATTR bool sendStateNext = false;

// ---- Modalita' IDENTIFY persistente (per ispezione fisica su edificio) ----
// Attivata da downlink identify_on (0x05), disattivata da identify_off (0x06).
// Quando attiva (option B), il device NON dorme: resta sveglio, lampeggia in
// continuo e ritrasmette ogni IDENTIFY_TX_INTERVAL_S (finestre RX frequenti per
// ricevere identify_off). Salta il GPS. Un timeout di sicurezza la spegne dopo
// IDENTIFY_TIMEOUT_S. Il flag e' in RTC cosi' sopravvive a un eventuale deep
// sleep di rientro (es. join fallito), ma un power-off lo azzera (voluto).
RTC_DATA_ATTR bool     identifyMode = false;   // modalita' identify attiva

// Configurazioni runtime (caricate al boot da NVS o dai default #define)
// Vengono modificate dal downlink handler via FPort 20.
uint8_t  cfgTxIntervalPreset;      // 0-5 (vedi TX_INTERVAL_SECONDS[])
uint8_t  cfgLoRaWANSF;             // 7-12
uint8_t  cfgTxPower;               // dBm 2-14
uint16_t cfgGpsTimeoutS;           // secondi
uint16_t cfgVbatEmergencyMv;       // mV
uint16_t cfgVbatRecoveryMv;        // mV
bool     cfgAdr;                   // ADR abilitato/disabilitato

// ---- Config runtime per features risparmio energetico ----
// Caricate in loadRuntimeConfig() da NVS (o default #define se NVS vuota)
bool     cfgGpsEnabled;            // 0 = GPS disattivato, 1 = attivo (SET_GPS_ENABLED)
uint8_t  cfgGpsSkipCycles;         // 0 = fix ogni ciclo, N = fix ogni N+1 cicli (SET_GPS_SKIP)
bool     cfgScdEnabled;            // 0 = SCD41 disattivato, 1 = attivo (SET_SCD_ENABLED)

// ---- Contatori RTC per feature di risparmio ----
// Sopravvivono al deep sleep, si azzerano al power-off e al reset hardware.
RTC_DATA_ATTR uint32_t rtcGpsSkipCounter = 0;   // conteggio cicli senza fix
RTC_DATA_ATTR int32_t  rtcLastLat_e7 = 0;       // ultima posizione nota (per skip)
RTC_DATA_ATTR int32_t  rtcLastLon_e7 = 0;
RTC_DATA_ATTR int16_t  rtcLastAlt_m = 0;
RTC_DATA_ATTR bool     rtcLastFixValid = false; // true dopo il primo fix riuscito

// ---- Contatori RTC per TRIPLE RESET SAFETY NET ----
// Contano quanti reset "veloci" (< TRIPLE_RST_WINDOW_S) sono avvenuti in
// sequenza. Al raggiungimento di TRIPLE_RST_COUNT reset consecutivi entro
// la finestra temporale, la config runtime viene resettata ai default.
RTC_DATA_ATTR uint8_t  rtcResetCounter = 0;
//RTC_DATA_ATTR uint32_t rtcLastResetMs = 0;      // timestamp millisec ultimo reset

// Stato di ritorno del menu, dice al setup() cosa fare dopo
enum SerialMenuResult {
    SMR_CONTINUE_NORMAL_BOOT = 0,   // esci dal menu, prosegui normalmente
    SMR_FORCE_TX_NOW,               // esci dal menu, force TX
    SMR_REBOOT,                     // reboot immediato (dopo reset config)
    SMR_PRESET_CHANGED_EXPLICITLY   // l'utente ha modificato preset via menu:
                                    // NON attivare fallback VBAT-detect
};

// =============================================================
// UTILITY
// =============================================================

void ledOn()  { digitalWrite(PIN_LED, HIGH); }
void ledOff() { digitalWrite(PIN_LED, LOW);  }

void vextOn() {
    // Vext_Ctrl è active-low: LOW = periferiche accese
    pinMode(PIN_VEXT_CTRL, OUTPUT);
    digitalWrite(PIN_VEXT_CTRL, LOW);
}

void vextOff() {
    digitalWrite(PIN_VEXT_CTRL, HIGH);
}

void gpsPowerOn() {
    pinMode(PIN_VGNSS_CTRL, OUTPUT);
    digitalWrite(PIN_VGNSS_CTRL, LOW);   // LOW = GPS acceso
}

void gpsPowerOff() {
    digitalWrite(PIN_VGNSS_CTRL, HIGH);  // HIGH = GPS spento
}

/**
 * Legge la tensione della batteria in millivolt.
 * ADC_Ctrl deve essere HIGH durante la lettura per abilitare il partitore
 * (nota: sulla V3 la convenzione era opposta - LOW abilita)
 */
uint16_t readBatteryMv() {
    pinMode(PIN_ADC_CTRL, OUTPUT);
    digitalWrite(PIN_ADC_CTRL, HIGH);   // V4: HIGH abilita il partitore
    delay(10);

    analogReadResolution(12);
    // Media di 16 letture per stabilità
    uint32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += analogReadMilliVolts(PIN_VBAT_READ);
    }
    uint32_t adcMv = sum / 16;

    digitalWrite(PIN_ADC_CTRL, LOW);    // V4: LOW disabilita il partitore per risparmiare

    // Il partitore sulla V4 divide per ~4.9 (schema Heltec):
    //   VBAT ----[390k]----+----[100k]---- GND
    //                      |
    //                     ADC (GPIO 1)
    // Ratio = (390+100)/100 = 4.9
    // Quindi VBAT = adcMv * 4.9
    return (uint16_t)(adcMv * 49 / 10);
}

/**
 * Converte VBAT in percentuale (LiPo 1S, 3.3-4.2V).
 */
uint8_t vbatToPercent(uint16_t vbat_mv) {
    if (vbat_mv >= 4200) return 100;
    if (vbat_mv <= 3300) return 0;
    // Curva lineare a tratti approssimativa
    // Sopra 3.7V (~50%) la curva è più ripida
    if (vbat_mv >= 3700) {
        return 50 + (vbat_mv - 3700) * 50 / 500;   // 3700-4200 -> 50-100
    } else {
        return (vbat_mv - 3300) * 50 / 400;         // 3300-3700 -> 0-50
    }
}

void printHex(const uint8_t* buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (buf[i] < 0x10) Serial.print('0');
        Serial.print(buf[i], HEX);
    }
}

// =============================================================
// PERSISTENZA sessione LoRaWAN
// =============================================================
//
// In RadioLib 7.x NON esiste setFCntUp(). Per persistere il FCnt bisogna
// salvare/ripristinare l'intero session buffer con getBufferSession() /
// setBufferSession().
//
// Strategia in OTAA (modalita' di default):
// - Dopo ogni TX si copia la sessione in RTC memory (cacheSessionInRTC).
//   La RTC sopravvive al deep sleep, quindi il FCnt incrementa tra i cicli.
// - Al boot si ripristina SOLO dalla RTC. Nessun fallback su NVS: se il boot
//   arriva da power-off (RTC persa) si rifa' un join, non si ripesca una
//   sessione vecchia dalla flash (che avrebbe un FCnt ormai superato).
//
// Le funzioni save/loadSessionFromNVS qui sotto NON sono usate dal percorso
// OTAA. Restano perche' servono al ramo ABP (dove la sessione statica va
// invece persistita in flash) e alla modalita' debug.

// Buffer per la sessione LoRaWAN, in RTC memory (sopravvive al deep sleep)
RTC_DATA_ATTR uint8_t rtcSessionBuffer[RADIOLIB_LORAWAN_SESSION_BUF_SIZE];
RTC_DATA_ATTR bool    rtcSessionValid = false;

// Buffer per i nonces OTAA (solo se USE_OTAA=1), in RTC memory.
// Contiene DevNonce (2 byte), JoinNonce (3 byte) e altri metadata.
// Persiste anche in NVS per sopravvivere ai power-off.
RTC_DATA_ATTR uint8_t rtcNoncesBuffer[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];
RTC_DATA_ATTR bool    rtcNoncesValid = false;

/**
 * Salva l'attuale buffer di sessione LoRaWAN in NVS.
 * NON PIU' USATA nella versione attuale (ne' OTAA ne' ABP salvano la sessione
 * in flash). Lasciata definita per eventuale uso futuro; puo' generare un
 * warning "defined but not used", innocuo.
 */
bool saveSessionToNVS() {
    uint8_t* sessionBuf = node.getBufferSession();
    if (!sessionBuf) {
        Serial.println("NVS: getBufferSession ha ritornato NULL");
        return false;
    }
    if (!prefs.begin(NVS_NAMESPACE, false)) {
        Serial.println("NVS: apertura namespace R/W fallita");
        return false;
    }
    size_t written = prefs.putBytes(NVS_KEY_FCNT_UP, sessionBuf,
                                    RADIOLIB_LORAWAN_SESSION_BUF_SIZE);
    prefs.end();
    if (written != RADIOLIB_LORAWAN_SESSION_BUF_SIZE) {
        Serial.printf("NVS: scrittura sessione fallita (%u/%d bytes)\n",
                      (unsigned)written, RADIOLIB_LORAWAN_SESSION_BUF_SIZE);
        return false;
    }
    Serial.printf("NVS: sessione LoRaWAN salvata (%d byte)\n",
                  RADIOLIB_LORAWAN_SESSION_BUF_SIZE);
    return true;
}

/**
 * Legge la sessione LoRaWAN da NVS in un buffer.
 * Ritorna true se e' stata letta con successo.
 */
bool loadSessionFromNVS(uint8_t* out) {
    if (!prefs.begin(NVS_NAMESPACE, true)) {
        Serial.println("NVS: nessuna sessione salvata (primo boot?)");
        return false;
    }
    size_t read = prefs.getBytes(NVS_KEY_FCNT_UP, out,
                                 RADIOLIB_LORAWAN_SESSION_BUF_SIZE);
    prefs.end();
    if (read != RADIOLIB_LORAWAN_SESSION_BUF_SIZE) {
        Serial.printf("NVS: lettura sessione parziale (%u/%d bytes)\n",
                      (unsigned)read, RADIOLIB_LORAWAN_SESSION_BUF_SIZE);
        return false;
    }
    Serial.printf("NVS: sessione LoRaWAN letta (%d byte)\n",
                  RADIOLIB_LORAWAN_SESSION_BUF_SIZE);
    return true;
}

/**
 * Copia il buffer sessione corrente in RTC memory (per il deep sleep).
 */
void cacheSessionInRTC() {
    uint8_t* sessionBuf = node.getBufferSession();
    if (sessionBuf) {
        memcpy(rtcSessionBuffer, sessionBuf, RADIOLIB_LORAWAN_SESSION_BUF_SIZE);
        rtcSessionValid = true;
    }
}


// =============================================================
// PERSISTENZA NONCES OTAA
// =============================================================
//
// I nonces (DevNonce, JoinNonce) sono contatori antireplay che RadioLib
// gestisce internamente durante il join OTAA. Se il device fa power-off
// senza persisterli, al boot successivo puo' generare un DevNonce gia'
// usato: il NS rifiuta il join con "MIC mismatch" o "DevNonce reused".
//
// Persistiamo il buffer completo su due livelli, come la sessione:
//   RTC memory  -> veloce, sopravvive a deep sleep
//   NVS (flash) -> lento, sopravvive a power-off e reflash

#if USE_OTAA

bool saveNoncesToNVS() {
    // getBufferNonces() in RadioLib 7.x ritorna un puntatore al buffer
    // interno, non riceve un buffer come parametro (come getBufferSession)
    uint8_t* noncesBuf = node.getBufferNonces();
    if (!noncesBuf) {
        Serial.println("NVS: getBufferNonces ha ritornato NULL");
        return false;
    }
    if (!prefs.begin(NVS_NAMESPACE, false)) {
        Serial.println("NVS: apertura namespace R/W fallita");
        return false;
    }
    size_t written = prefs.putBytes(NVS_KEY_NONCES, noncesBuf,
                                    RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
    prefs.end();
    if (written != RADIOLIB_LORAWAN_NONCES_BUF_SIZE) {
        Serial.printf("NVS: putBytes(nonces) parziale (%u/%u)\n",
                      (unsigned)written,
                      (unsigned)RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
        return false;
    }
    // Aggiorna anche RTC cache
    memcpy(rtcNoncesBuffer, noncesBuf, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
    rtcNoncesValid = true;
    Serial.println("Nonces OTAA salvati in NVS");
    return true;
}

bool loadNoncesFromNVS(uint8_t* out) {
    if (!prefs.begin(NVS_NAMESPACE, true)) {
        return false;
    }
    size_t len = prefs.getBytesLength(NVS_KEY_NONCES);
    if (len != RADIOLIB_LORAWAN_NONCES_BUF_SIZE) {
        prefs.end();
        return false;
    }
    prefs.getBytes(NVS_KEY_NONCES, out, RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
    prefs.end();
    return true;
}

#endif  // USE_OTAA


// =============================================================
// CONFIG RUNTIME - lettura/scrittura da NVS
// =============================================================
//
// I valori delle configurazioni modificabili via downlink (FPort 20) vengono
// letti al boot da NVS. Se una chiave non e' presente, si usa il default
// hardcoded dal corrispondente #define.

void loadRuntimeConfig() {
    // Default hardcoded (usati se NVS vuota)
    cfgTxIntervalPreset = TX_INTERVAL_PRESET;
    cfgLoRaWANSF        = LORAWAN_SF;
    cfgTxPower          = 14;
    cfgGpsTimeoutS      = GPS_FIX_TIMEOUT_COLD_S;
    cfgVbatEmergencyMv  = VBAT_EMERGENCY_MV;
    cfgVbatRecoveryMv   = VBAT_RECOVERY_MV;
    cfgAdr              = (bool)ENABLE_ADR;
    // Default per features risparmio energetico
    cfgGpsEnabled       = true;   // default: GPS attivo
    cfgGpsSkipCycles    = 0;      // default: fix ogni ciclo (nessun skip)
    cfgScdEnabled       = true;   // default: SCD41 attivo

    // Prova a leggere da NVS (se una chiave manca, mantiene il default)
    if (prefs.begin(NVS_NAMESPACE, true)) {   // true = read-only
        cfgTxIntervalPreset = prefs.getUChar (NVS_KEY_TX_INTERVAL,  cfgTxIntervalPreset);
        cfgLoRaWANSF        = prefs.getUChar (NVS_KEY_LORAWAN_SF,   cfgLoRaWANSF);
        cfgTxPower          = prefs.getUChar (NVS_KEY_TX_POWER,     cfgTxPower);
        cfgGpsTimeoutS      = prefs.getUShort(NVS_KEY_GPS_TIMEOUT,  cfgGpsTimeoutS);
        cfgVbatEmergencyMv  = prefs.getUShort(NVS_KEY_VBAT_EMERG,   cfgVbatEmergencyMv);
        cfgVbatRecoveryMv   = prefs.getUShort(NVS_KEY_VBAT_RECOV,   cfgVbatRecoveryMv);
        cfgAdr              = prefs.getUChar (NVS_KEY_ADR,          cfgAdr ? 1 : 0) != 0;
        // Features risparmio energetico
        cfgGpsEnabled       = prefs.getUChar (NVS_KEY_GPS_ENABLED,  cfgGpsEnabled ? 1 : 0) != 0;
        cfgGpsSkipCycles    = prefs.getUChar (NVS_KEY_GPS_SKIP,     cfgGpsSkipCycles);
        cfgScdEnabled       = prefs.getUChar (NVS_KEY_SCD_ENABLED,  cfgScdEnabled ? 1 : 0) != 0;
        prefs.end();
    }

    // Sanity check (evita valori assurdi da NVS corrotta)
    if (cfgTxIntervalPreset > TX_PRESET_MAX) cfgTxIntervalPreset = TX_INTERVAL_PRESET;
    if (cfgLoRaWANSF < 7 || cfgLoRaWANSF > 12) cfgLoRaWANSF = LORAWAN_SF;
    if (cfgTxPower < 2 || cfgTxPower > 14) cfgTxPower = 14;
    if (cfgGpsTimeoutS < 10 || cfgGpsTimeoutS > 300) cfgGpsTimeoutS = GPS_FIX_TIMEOUT_COLD_S;
    // cfgGpsSkipCycles: qualunque valore 0-255 e' valido, no check
    // cfgGpsEnabled, cfgScdEnabled, cfgAdr sono bool, no check
}

/**
 * @brief Resetta la config runtime ai valori di default (#define di fabbrica),
 *        cancellando le chiavi corrispondenti da NVS.
 *
 * Chiamata dal modulo TRIPLE RST RESET quando l'utente preme RST 3 volte,
 * o dal comando CLEAR_NVS. Non tocca i nonces OTAA (che restano validi).
 */
void clearRuntimeConfigToDefaults() {
    if (prefs.begin(NVS_NAMESPACE, false)) {
        prefs.remove(NVS_KEY_TX_INTERVAL);
        prefs.remove(NVS_KEY_LORAWAN_SF);
        prefs.remove(NVS_KEY_TX_POWER);
        prefs.remove(NVS_KEY_GPS_TIMEOUT);
        prefs.remove(NVS_KEY_VBAT_EMERG);
        prefs.remove(NVS_KEY_VBAT_RECOV);
        prefs.remove(NVS_KEY_ADR);
        prefs.remove(NVS_KEY_GPS_ENABLED);
        prefs.remove(NVS_KEY_GPS_SKIP);
        prefs.remove(NVS_KEY_SCD_ENABLED);
        prefs.end();
    }
    // Ricarica le variabili in RAM con i default
    loadRuntimeConfig();
}

// Helper per scrivere una config in NVS
bool saveConfigUChar(const char* key, uint8_t val) {
    if (!prefs.begin(NVS_NAMESPACE, false)) return false;
    prefs.putUChar(key, val);
    prefs.end();
    return true;
}
bool saveConfigUShort(const char* key, uint16_t val) {
    if (!prefs.begin(NVS_NAMESPACE, false)) return false;
    prefs.putUShort(key, val);
    prefs.end();
    return true;
}


// ###################################################################
// #                                                                 #
// #    FEATURE MODULES: risparmio energetico avanzato               #
// #                                                                 #
// #    Ciascun modulo e' isolato in una sezione ben delimitata,     #
// #    con funzioni auto-descrittive e commenti dedicati.           #
// #    Ciascuno puo' essere disabilitato con il flag                #
// #    ENABLE_XXX corrispondente (in cima al file).                 #
// #                                                                 #
// ###################################################################


// ===================================================================
// MODULO: STORAGE MODE (preset TX = 6)
// ===================================================================
//
// COS'E'
// ------
// Modalita' di "spegnimento operativo" del device. Attivata tramite
// downlink SET_TX_INTERVAL con preset 6, mette il device in deep
// sleep infinito (senza timer di wake).
//
// COMPORTAMENTO
// -------------
//   - Consumo elettrico: ~15 uA (praticamente uguale a "device spento")
//   - Durata teorica: >10 anni con batteria 1500 mAh (limite pratico
//     autoscarica ~2%/mese della cella Li-Ion)
//   - Wake: solo tramite reset hardware (pulsante RST)
//
// FLUSSO DI RIATTIVAZIONE
// -----------------------
//   1. Utente collega cavetto USB al device
//   2. Utente preme RST manualmente
//   3. Al boot, il device legge VBAT:
//        - VBAT > 4.25V (USB collegato) -> resetta preset a default
//          (2 = 1 minuto), il device torna operativo
//        - VBAT <= 4.25V (batteria sola) -> il device fa UN uplink di
//          conferma "sono vivo, ancora in storage" e torna a dormire
//          (utile per verifica remota da webapp)
//
// PERCHE' UN PRESET NELL'ARRAY E NON UN COMANDO SEPARATO
// ------------------------------------------------------
// Riusare SET_TX_INTERVAL evita di introdurre un nuovo comando
// downlink dedicato. La logica "device dormiente" e' semanticamente
// un caso limite di "TX interval infinito", coerente con gli altri
// preset (che sono intervalli finiti).
//
// COMPORTAMENTO CON USB IN FUNZIONAMENTO NORMALE
// ----------------------------------------------
// Se il device NON e' in storage mode (preset != 6), USB attaccato
// non ha alcun effetto sul flusso: il device continua a lavorare
// normalmente e la batteria si ricarica in background. Nessuna
// logica speciale.
// -------------------------------------------------------------------

#if ENABLE_STORAGE_MODE

//#define VBAT_USB_DETECT_MV     4250   // sopra questa soglia = USB collegato
#define VBAT_USB_DETECT_MV     4000   // sopra questa soglia = USB collegato

/**
 * @brief Verifica se il device deve entrare in modalita' storage.
 * @return true se il preset TX corrente e' quello di storage
 */
bool isStorageMode() {
    return cfgTxIntervalPreset == TX_PRESET_STORAGE;
}

/**
 * @brief Chiamata al boot, decide se resettare cfgTxIntervalPreset a default.
 *
 * Se il device era in storage E VBAT indica USB collegato, il preset
 * TX viene resettato al default di fabbrica (TX_INTERVAL_PRESET) e
 * salvato in NVS. Il device tornera' operativo dal prossimo TX.
 *
 * Se il device era in storage E VBAT indica batteria sola, il preset
 * resta 6 (il ciclo attuale fara' un TX di conferma e tornera' a
 * dormire).
 *
 * Se il device NON era in storage, questa funzione non fa nulla.
 *
 * @param vbat_mv tensione batteria letta ora (in mV)
 * @return true se e' stato fatto il "wake from storage", false altrimenti
 */
bool checkUsbWakeupAndReset(uint16_t vbat_mv) {
    if (!isStorageMode()) {
        return false;   // device non era in storage, nessuna azione
    }
    if (vbat_mv < VBAT_USB_DETECT_MV) {
        Serial.println("STORAGE MODE: wake da RST ma nessun USB (VBAT < 4.25V)");
        Serial.println("STORAGE MODE: faccio TX di conferma e torno a dormire");
        // Il device resta in storage, il ciclo TX gestira' l'invio
        sendStateNext = true;  // il prossimo TX sara' un payload state
        return false;
    }
    // USB collegato: torna operativo
    Serial.printf("STORAGE MODE: USB rilevato (VBAT=%u mV), reset a preset default %u\n",
                  vbat_mv, TX_INTERVAL_PRESET);
    cfgTxIntervalPreset = TX_INTERVAL_PRESET;
    saveConfigUChar(NVS_KEY_TX_INTERVAL, cfgTxIntervalPreset);
    sendStateNext = true;   // notifica al server che siamo tornati operativi

    // Feedback visivo: 5 blink veloci = "wake from storage OK"
    for (int i = 0; i < 5; i++) {
        ledOn();  delay(100);
        ledOff(); delay(100);
    }
    return true;
}

/**
 * @brief Entra in deep sleep infinito (senza timer di wake).
 *
 * Chiamata al posto di enterDeepSleep() quando il device e' in
 * storage mode. Il wake sara' possibile solo tramite reset hardware.
 * Prima dello sleep, tutte le periferiche vengono spente (Vext,
 * VGNSS) e i GPIO messi in hold.
 */
void enterStorageDeepSleep() {
    Serial.println("STORAGE MODE: entering INFINITE deep sleep (wake only on RST)");
    Serial.flush();

    // Spegni periferiche esterne
    vextOff();
    gpsPowerOff();

    // Hold dei GPIO di controllo
    gpio_hold_en((gpio_num_t)PIN_VEXT_CTRL);
    gpio_hold_en((gpio_num_t)PIN_VGNSS_CTRL);
    gpio_deep_sleep_hold_en();

    // NON chiamare esp_sleep_enable_timer_wakeup()
    // NON abilitare wake su GPIO (fuori scope di questo modulo)
    esp_deep_sleep_start();
}

#endif  // ENABLE_STORAGE_MODE


// ===================================================================
// MODULO: GPS SKIP LOGIC (SET_GPS_SKIP)
// ===================================================================
//
// COS'E'
// ------
// Permette di eseguire il fix GPS solo ogni N+1 cicli invece che ad
// ogni ciclo TX. Su installazioni fisse (nodo che non si sposta),
// non serve un fix GPS ogni minuto.
//
// COMPORTAMENTO
// -------------
//   cfgGpsSkipCycles = 0  -> fix ogni ciclo (default, comportamento normale)
//   cfgGpsSkipCycles = 9  -> fix ogni 10 cicli (uno su 10)
//   cfgGpsSkipCycles = 59 -> fix ogni ora (se TX ogni minuto)
//
// Nei cicli "senza fix", il payload usa l'ultima posizione nota
// memorizzata in RTC memory (marcata come "stale" con HDOP=9999).
//
// RISPARMIO ENERGETICO
// --------------------
// Il GPS e' la periferica piu' energivora del nodo:
//   - Cold start:  ~35 mA per 60-90 secondi   = ~580 uAh per ciclo
//   - Warm start:  ~25 mA per 10-30 secondi   = ~70 uAh per ciclo
//   - Skip totale: ~0 mA (GPS spento)         = ~0 uAh per ciclo
//
// Con TX ogni 60s, skip 9 (fix ogni 10 cicli):
//   - Consumo attivo medio: da ~10 mAh/giorno a ~3 mAh/giorno
//   - Autonomia: da 4 mesi a >1 anno con batteria 1500 mAh
// -------------------------------------------------------------------

#if ENABLE_GPS_SKIP

/**
 * @brief Decide se questo ciclo TX deve fare un fix GPS o saltarlo.
 *
 * Usa il contatore RTC rtcGpsSkipCounter per tenere traccia dei
 * cicli passati dall'ultimo fix. Se cfgGpsSkipCycles = N, il fix
 * avviene ogni N+1 cicli.
 *
 * @return true se il ciclo corrente deve fare il fix GPS
 */
bool shouldDoGpsFix() {
    // Se GPS disabilitato completamente, non fare mai il fix
    if (!cfgGpsEnabled) return false;

    // Se skip = 0, fai il fix ogni volta
    if (cfgGpsSkipCycles == 0) {
        rtcGpsSkipCounter = 0;
        return true;
    }

    // Se non abbiamo ancora un fix "warm", forziamo il fix
    // (serve almeno il primo fix per popolare la RTC memory)
    if (!rtcLastFixValid) {
        rtcGpsSkipCounter = 0;
        return true;
    }

    // Logica skip: fai il fix se il contatore ha raggiunto la soglia
    rtcGpsSkipCounter++;
    if (rtcGpsSkipCounter > cfgGpsSkipCycles) {
        rtcGpsSkipCounter = 0;
        return true;
    }

    // Salta il fix
    return false;
}

/**
 * @brief Popola i campi GPS del payload con l'ultima posizione nota (skip).
 *
 * Chiamata quando shouldDoGpsFix() ha ritornato false. Il payload
 * riporta le coordinate ultime, ma con HDOP=9999 come marker "stale".
 *
 * @param payload puntatore alla struct payload da popolare
 */
void useLastKnownPosition(Payload_v0x42* payload) {
    if (rtcLastFixValid) {
        payload->lat_e7      = rtcLastLat_e7;
        payload->lon_e7      = rtcLastLon_e7;
        payload->alt_m       = rtcLastAlt_m;
        payload->hdop_x100   = 9999;   // marker "posizione stale"
        payload->fix_quality = 1;      // fix precedente valido
        payload->satellites  = 0;      // non stiamo osservando ora
        Serial.printf("GPS SKIP: uso ultima posizione (skip counter %u/%u)\n",
                      rtcGpsSkipCounter, cfgGpsSkipCycles);
    } else {
        // Nessuna posizione nota (mai preso un fix). Segnaliamo con zeri.
        payload->lat_e7      = 0;
        payload->lon_e7      = 0;
        payload->alt_m       = 0;
        payload->hdop_x100   = 9999;
        payload->fix_quality = 0;
        payload->satellites  = 0;
    }
}

/**
 * @brief Memorizza in RTC memory una nuova posizione GPS valida.
 *
 * Chiamata dopo un fix GPS riuscito. La posizione sopravvive al
 * deep sleep, si perde al power-off/reflash.
 */
void saveGpsFixToRTC(int32_t lat_e7, int32_t lon_e7, int16_t alt_m) {
    rtcLastLat_e7   = lat_e7;
    rtcLastLon_e7   = lon_e7;
    rtcLastAlt_m    = alt_m;
    rtcLastFixValid = true;
}

#endif  // ENABLE_GPS_SKIP


// ===================================================================
// MODULO: SENSOR DISABLE (SET_GPS_ENABLED, SET_SCD_ENABLED)
// ===================================================================
//
// COS'E'
// ------
// Permette di disattivare completamente il GPS L76K o il SCD41 via
// downlink. Utile quando il sensore fisicamente non e' presente,
// o quando non serve nel contesto di installazione.
//
// COMPORTAMENTO
// -------------
// Quando un sensore e' disattivato:
//   - Il modulo non viene alimentato (via Vext/VGNSS)
//   - Il payload riporta valori "placeholder" ben definiti:
//       CO2  = 0xFFFF (65535 = valore invalido)
//       T    = 0x7FFF (32767 = valore invalido)
//       RH   = 0xFFFF (65535 = valore invalido)
//   - Il codec ChirpStack riconosce questi placeholder e restituisce
//     'null' invece dei valori, cosi' la dashboard non mostra dati
//     spuri.
//
// USO TIPICO
// ----------
// GPS disabilitato: nodo installato indoor senza fix necessario
// SCD41 disabilitato: nodo montato senza sensore CO2 collegato
// -------------------------------------------------------------------

#if ENABLE_SENSOR_DISABLE

// Placeholder per valori "sensore disattivato"
#define SENSOR_DISABLED_U16_INVALID    0xFFFFu   // per CO2 (ppm) e RH (%x100)
#define SENSOR_DISABLED_I16_INVALID    0x7FFF    // per temperatura (°Cx100)

/**
 * @brief Popola i campi SCD41 del payload con valori "disattivato".
 *
 * Chiamata quando cfgScdEnabled = false. Il codec deve riconoscere
 * questi placeholder e restituire null invece dei valori.
 */
void useScdDisabledPayload(Payload_v0x42* payload) {
    payload->co2_ppm     = SENSOR_DISABLED_U16_INVALID;
    payload->temp_c100   = SENSOR_DISABLED_I16_INVALID;
    payload->hum_pct100  = SENSOR_DISABLED_U16_INVALID;
    Serial.println("SCD41 DISABLED: valori placeholder nel payload");
}

/**
 * @brief Popola i campi GPS del payload con valori "disattivato".
 *
 * Chiamata quando cfgGpsEnabled = false. Segnala la disattivazione
 * con fix_quality=0 e HDOP=0xFFFF.
 */
void useGpsDisabledPayload(Payload_v0x42* payload) {
    payload->lat_e7      = 0;
    payload->lon_e7      = 0;
    payload->alt_m       = 0;
    payload->hdop_x100   = SENSOR_DISABLED_U16_INVALID;
    payload->fix_quality = 0;
    payload->satellites  = 0;
    Serial.println("GPS DISABLED: valori placeholder nel payload");
}

#endif  // ENABLE_SENSOR_DISABLE


// ===================================================================
// MODULO: TRIPLE RESET SAFETY NET (emergency unlock hardware)
// ===================================================================
//
// COS'E'
// ------
// Se l'utente preme RST 3 volte consecutive entro una finestra
// temporale breve, la config runtime viene resettata ai valori di
// default di fabbrica. Utile come "escape hatch" quando il device
// e' in una config sbagliata (es. storage mode senza USB, TX
// interval troppo alto, ADR sbagliato) e non e' raggiungibile via
// radio.
//
// COMPORTAMENTO
// -------------
// Al boot, se il reset e' avvenuto entro TRIPLE_RST_WINDOW_S dal
// precedente, il contatore rtcResetCounter viene incrementato.
// Se il contatore raggiunge TRIPLE_RST_COUNT (3), la config viene
// resettata e il contatore azzerato.
//
// Se il tempo dal reset precedente e' maggiore di TRIPLE_RST_WINDOW_S,
// il contatore viene azzerato (nuovo tentativo di sequenza).
//
// FEEDBACK VISIVO
// ---------------
// Quando il reset ai default viene applicato, il LED lampeggia 3
// volte lentamente (~500ms on/off) per confermare visivamente
// all'utente che il reset e' avvenuto.
// -------------------------------------------------------------------

#if ENABLE_TRIPLE_RST_RESET

#define TRIPLE_RST_COUNT      3
#define TRIPLE_RST_WINDOW_MS  8000

// Tempo RTC che sopravvive ai reset hardware.
// NON usare millis(): millis() riparte da zero ad ogni reboot.
RTC_DATA_ATTR uint64_t rtcLastResetUs = 0;


// -------------------------------------------------------------
// Blink di conferma del reset configurazione
// -------------------------------------------------------------
void tripleResetBlinkConfirm() {
    Serial.println("[BLINK] tripleResetBlinkConfirm() chiamata - inizio 3 blink lunghi");
    // 1. Disabilita la ritenuta globale e quella specifica del pin LED
    gpio_deep_sleep_hold_dis();
    gpio_hold_dis((gpio_num_t)PIN_LED);
    
    // 2. FONDAMENTALE: Ripristina la direzione del pin come output
    pinMode(PIN_LED, OUTPUT); 

    delay(10); // Piccolo ritardo per stabilizzare i registri

    // Partiamo da LED spento
    ledOff();
    delay(100);

    for (uint8_t i = 0; i < TRIPLE_RST_COUNT; i++) {
        Serial.printf("[BLINK] Blink %u/%u\n", i+1, TRIPLE_RST_COUNT);
        ledOn();
        delay(1000);   // 1 secondo acceso (LUNGO, ben visibile)

        ledOff();
        delay(300);    // 300ms spento
    }

    // Assicurati che lo stato finale desiderato sia impostato prima del blocco
    ledOff(); 
    Serial.println("[BLINK] Fine sequenza blink");
    
    // 3. Riabilita la ritenuta per il Deep Sleep
    gpio_hold_en((gpio_num_t)PIN_LED);
    gpio_deep_sleep_hold_en();
}

// -------------------------------------------------------------
// Controllo sequenza 3 reset entro TRIPLE_RST_WINDOW_MS
// -------------------------------------------------------------
bool checkTripleResetToDefaults() {
    esp_reset_reason_t reason = esp_reset_reason();
    // Consideriamo validi per la sequenza:
    // - power-on
    // - reset esterno tramite RST
    // - ESP.restart()
    // - reset da USB (auto-reset da upload firmware IDE Arduino)
    //
    // NON consideriamo validi wake da deep sleep, watchdog,
    // brownout, panic ecc.
    bool isResetForTriple =
        (reason == ESP_RST_POWERON) ||
        (reason == ESP_RST_EXT)     ||
        (reason == ESP_RST_SW)      ||
        (reason == ESP_RST_USB);

    if (!isResetForTriple) {
        rtcResetCounter = 0;
        rtcLastResetUs = 0;
        return false;
    }
    // ---------------------------------------------------------
    // Legge il timer RTC.
    //
    // A differenza di millis(), questo riferimento temporale
    // continua a essere disponibile attraverso il reset.
    // ---------------------------------------------------------
    uint64_t nowUs = esp_rtc_get_time_us();

    // Primo reset della sequenza.
    if (rtcResetCounter == 0 || rtcLastResetUs == 0) {
        rtcResetCounter = 1;
        rtcLastResetUs = nowUs;
        Serial.printf(
            "TRIPLE RST: 1/%u - premi RST ancora entro %u s\n",
            TRIPLE_RST_COUNT,
            TRIPLE_RST_WINDOW_MS / 1000
        );
        return false;
    }
    // ---------------------------------------------------------
    // Calcola il tempo trascorso dall'ultimo reset.
    // ---------------------------------------------------------
    uint64_t elapsedUs = nowUs - rtcLastResetUs;
    uint64_t windowUs =
        (uint64_t)TRIPLE_RST_WINDOW_MS * 1000ULL;
    // ---------------------------------------------------------
    // Troppo tempo: nuova sequenza.
    // ---------------------------------------------------------
    if (elapsedUs > windowUs) {
        Serial.printf(
            "TRIPLE RST: finestra scaduta (%llu ms) -> riparto da 1\n",
            elapsedUs / 1000ULL
        );
        rtcResetCounter = 1;
        rtcLastResetUs = nowUs;
        Serial.printf(
            "TRIPLE RST: 1/%u - premi RST ancora entro %u s\n",
            TRIPLE_RST_COUNT,
            TRIPLE_RST_WINDOW_MS / 1000
        );
        return false;
    }
    // ---------------------------------------------------------
    // Reset avvenuto entro gli 8 secondi.
    // Incrementa il contatore.
    // ---------------------------------------------------------
    rtcResetCounter++;
    rtcLastResetUs = nowUs;
    Serial.printf(
        "TRIPLE RST: %u/%u - intervallo %llu ms\n",
        rtcResetCounter,
        TRIPLE_RST_COUNT,
        elapsedUs / 1000ULL
    );
    // ---------------------------------------------------------
    // TERZO RESET ENTRO 8 SECONDI
    // ---------------------------------------------------------
    if (rtcResetCounter >= TRIPLE_RST_COUNT) {
        Serial.println();
        Serial.println("========================================");
        Serial.println("*** TRIPLE RST DETECTED ***");
        Serial.println("*** RESET CONFIG TO DEFAULTS ***");
        Serial.println("========================================");
        // Cancella la configurazione runtime dalla NVS.
        clearRuntimeConfigToDefaults();
        // Sequenza completata.
        rtcResetCounter = 0;
        rtcLastResetUs = 0;
        // Feedback visivo IMMEDIATO.
        tripleResetBlinkConfirm();
        return true;
    }
    return false;
}
#endif  // ENABLE_TRIPLE_RST_RESET


// ===================================================================
// MODULO: SERIAL_MENU (manutenzione via USB)
// ===================================================================
//
// COS'E'
// ------
// Menu interattivo su USB CDC che si apre al boot dopo un reset
// hardware (RST o power-on) se USB e' collegato. Offre operazioni di
// manutenzione avanzate senza dover passare per LoRa: wake da storage,
// reset config, status dump, force TX, clear NVS.
//
// TRIGGER DI APERTURA
// -------------------
// Il menu si apre SE E SOLO SE:
//   1. Il boot e' avvenuto per reset hardware (ESP_RST_EXT) o
//      power-on (ESP_RST_POWERON), NON per wake da deep sleep
//   2. VBAT indica USB collegato (VBAT > 4.25V)
//
// La condizione 1 evita di aprire il menu ad ogni ciclo TX (che
// pagherebbe ~10s di consumo attivo inutili). La condizione 2 evita
// di aprire il menu quando non c'e' terminale connesso.
//
// FALLBACK SILENZIOSO SU VBAT-DETECT
// ----------------------------------
// Se il menu si apre ma va in timeout (utente non digita nulla) E
// il device era in storage mode, viene eseguito automaticamente il
// wake-from-storage (equivalente al VBAT-detect). Cosi' il pattern
// "attacca USB + premi RST + non fare altro" continua a funzionare
// come prima.
//
// COMANDI SUPPORTATI
// ------------------
//   W  Wake da storage (resetta solo cfgTxIntervalPreset al default)
//   R  Reset totale ai default di fabbrica (tutte le config runtime)
//   S  Status dump (bootCount, FCnt, VBAT, config, ultimo GPS)
//   T  Force TX now (esci dal menu e vai al ciclo TX)
//   C  Clear NVS completa (nonces OTAA inclusi - forza rejoin)
//   N  Continue Normal boot (esci dal menu, prosegui il boot)
//   ?  Help (ristampa la lista comandi)
//
// I comandi distruttivi (R, C) richiedono conferma "Y" entro 5s.
//
// PERCHE' USB CDC E' DISPONIBILE QUI
// ----------------------------------
// Serial.begin(115200) e' stato gia' chiamato in setup(). Sull'ESP32-S3
// l'USB CDC e' su un controller dedicato (USB-Serial/JTAG) che parte
// automaticamente. Dopo un reset hardware l'host impiega ~1-2s a
// riconnettersi (enumerazione USB), quindi il menu attende brevemente
// prima di iniziare a leggere per non perdere caratteri.
// -------------------------------------------------------------------

#if ENABLE_SERIAL_MENU

// ---- Configurazione del modulo ----
#define SERIAL_MENU_TIMEOUT_MS       10000    // 10 secondi timeout comando
#define SERIAL_MENU_CONFIRM_TIMEOUT_MS 5000   // 5 secondi timeout conferma Y
#define SERIAL_MENU_ENUM_DELAY_MS      1500   // attesa enumerazione USB
#define SERIAL_MENU_HEARTBEAT_MS       1000   // periodo blink "sto ascoltando"

// Flag: settato a true quando il menu ha modificato esplicitamente il
// preset TX (comandi W o I). Il fallback VBAT-detect nel setup lo
// controlla per NON annullare una scelta esplicita dell'utente
// (es. utente digita I,10,Y per andare in storage: il fallback non
// deve resettare il preset a 2!).
bool menuPresetChangedExplicitly = false;

/**
 * @brief Stampa il banner e la lista comandi sulla seriale.
 */
void serialMenuPrintBanner() {
    Serial.println();
    Serial.println("============================================");
    Serial.println("  HELTEC V4 - MENU DI MANUTENZIONE");
    Serial.printf ("  Timeout: %u secondi\n", SERIAL_MENU_TIMEOUT_MS / 1000);
    Serial.println("--------------------------------------------");
    Serial.println("  W  Wake da storage (reset solo TX interval)");
    Serial.println("  R  Reset TOTALE config runtime ai default");
    Serial.println("  I  Imposta TX interval (preset 0-10)");
    Serial.println("  S  Status dump (VBAT, config, GPS, ecc.)");
    Serial.println("  T  Force TX now (esci ed esegui ciclo TX)");
    Serial.println("  C  Clear NVS COMPLETA (include nonces OTAA)");
    Serial.println("  N  Continue Normal boot (o attendi timeout)");
    Serial.println("  ?  Ristampa questo menu");
    Serial.println("============================================");
    Serial.print  ("> ");
}

/**
 * @brief Stampa uno status dump completo del device sulla seriale.
 *
 * Utile per diagnostica in campo senza dover interrogare la rete LoRa.
 */
void serialMenuPrintStatus() {
    Serial.println();
    Serial.println("---- STATUS DUMP ----");
    Serial.printf ("  Firmware:     v%u\n", FW_VERSION);
    Serial.printf ("  Boot count:   %u\n", bootCount);
    Serial.printf ("  Uptime:       %lu s\n", (unsigned long)(uptimeAccumulatedS + millis() / 1000));
    Serial.printf ("  Reset reason: %d\n", (int)esp_reset_reason());
    Serial.println();
    Serial.println("  ---- Alimentazione ----");
    uint16_t vbat = readBatteryMv();
    Serial.printf ("  VBAT:         %u mV (%u %%)\n", vbat, vbatToPercent(vbat));
    Serial.printf ("  USB detected: %s (soglia 4250 mV)\n",
                   (vbat > VBAT_USB_DETECT_MV) ? "SI" : "NO");
    Serial.println();
    Serial.println("  ---- Config runtime ----");
    if (cfgTxIntervalPreset == TX_PRESET_STORAGE) {
        Serial.println("  TX interval:  STORAGE MODE (sleep infinito)");
    } else {
        Serial.printf ("  TX interval:  preset=%u (%u s)\n",
                       cfgTxIntervalPreset,
                       TX_INTERVAL_SECONDS[cfgTxIntervalPreset]);
    }
    Serial.printf ("  LoRaWAN SF:   %u\n", cfgLoRaWANSF);
    Serial.printf ("  TX power:     %u dBm\n", cfgTxPower);
    Serial.printf ("  GPS timeout:  %u s\n", cfgGpsTimeoutS);
    Serial.printf ("  VBAT emerg:   %u mV\n", cfgVbatEmergencyMv);
    Serial.printf ("  VBAT recov:   %u mV\n", cfgVbatRecoveryMv);
    Serial.printf ("  ADR:          %s\n", cfgAdr ? "ON" : "OFF");
    Serial.printf ("  GPS enabled:  %s\n", cfgGpsEnabled ? "SI" : "NO");
    Serial.printf ("  GPS skip:     %u cicli\n", cfgGpsSkipCycles);
    Serial.printf ("  SCD enabled:  %s\n", cfgScdEnabled ? "SI" : "NO");
    Serial.println();
    Serial.println("  ---- Ultimo GPS noto ----");
    if (rtcLastFixValid) {
        Serial.printf ("  Lat: %.7f  Lon: %.7f  Alt: %d m\n",
                       rtcLastLat_e7 / 1.0e7, rtcLastLon_e7 / 1.0e7,
                       rtcLastAlt_m);
    } else {
        Serial.println("  (nessun fix valido in RTC)");
    }
    Serial.println("---- END STATUS ----");
    Serial.println();
}

/**
 * @brief Attende conferma "Y" dell'utente entro CONFIRM_TIMEOUT_MS.
 * @return true se l'utente ha digitato Y (case-insensitive), false altrimenti
 */
bool serialMenuWaitConfirmation() {
    Serial.print("Confermi? Digita Y entro 5s: ");
    String s = serialMenuReadLine(SERIAL_MENU_CONFIRM_TIMEOUT_MS);

    if (s.length() == 0) {
        Serial.println("timeout, annullato");
        return false;
    }
    char c = s.charAt(0);
    if (c == 'Y' || c == 'y') {
        Serial.println("Y (confermato)");
        return true;
    }
    Serial.printf("%c (annullato)\n", c);
    return false;
}

/**
 * @brief Cancella completamente la NVS del namespace 'lora', inclusi i
 *        nonces OTAA. Al prossimo boot il device rifara' il join.
 */
void serialMenuClearNvsComplete() {
    Serial.print("Cancellazione NVS in corso... ");
    if (prefs.begin(NVS_NAMESPACE, false)) {
        prefs.clear();   // cancella TUTTO il namespace, nonces inclusi
        prefs.end();
        Serial.println("OK");
    } else {
        Serial.println("ERRORE (impossibile aprire NVS)");
    }
}

/**
 * @brief Legge una riga da seriale terminata da '\n' o timeout.
 *
 * IMPORTANTE per USB CDC di ESP32-S3: il timing puo' variare. Aspettiamo
 * un po' che eventuali caratteri in transito arrivino prima di misurare
 * "quanti byte c'e' nel buffer" per decidere se aspettare o meno.
 *
 * Usa readStringUntil('\n') e trima whitespace/CR/spazi. Ritorna la
 * stringa pulita (puo' essere vuota se timeout). Il chiamante analizza
 * il contenuto.
 *
 * @param timeoutMs timeout massimo per leggere la riga
 * @return stringa letta e trimmed
 */
String serialMenuReadLine(uint32_t timeoutMs) {
    // Piccola attesa per far arrivare eventuali char pendenti sul CDC
    delay(30);
    // Se ci sono caratteri residui che sono SOLO whitespace/CR/LF,
    // scartiamoli per evitare che readStringUntil ritorni stringa vuota.
    // Ma se c'e' anche un solo carattere significativo, lo lasciamo
    // (potrebbe essere l'inizio dell'input dell'utente).
    while (Serial.available()) {
        int c = Serial.peek();
        if (c == '\r' || c == '\n' || c == ' ' || c == '\t') {
            Serial.read();   // scarta silenziosamente
        } else {
            break;
        }
    }

    Serial.setTimeout(timeoutMs);
    String s = Serial.readStringUntil('\n');
    s.trim();   // rimuove eventuali CR o spazi residui
    return s;
}

/**
 * @brief Legge un intero da seriale. Ritorna il numero, o -1 se
 *        l'utente ha annullato (input vuoto, Q, o non numerico).
 *
 * @param timeoutMs timeout massimo per completare l'input
 */
int serialMenuReadIntWithEnter(uint32_t timeoutMs) {
    String s = serialMenuReadLine(timeoutMs);

    if (s.length() == 0) {
        Serial.println("(timeout o input vuoto)");
        return -1;
    }

    // Q maiuscola o minuscola: annulla esplicitamente
    if (s.charAt(0) == 'Q' || s.charAt(0) == 'q') {
        Serial.println("Q (annullato)");
        return -1;
    }

    // Verifica che ci siano solo cifre
    for (unsigned i = 0; i < s.length(); i++) {
        char c = s.charAt(i);
        if (c < '0' || c > '9') {
            Serial.printf("Input non numerico: '%s'\n", s.c_str());
            return -1;
        }
    }

    // Echo di quello che l'utente ha digitato (utile per conferma visiva)
    Serial.printf("(letto: %s)\n", s.c_str());
    return s.toInt();
}

/**
 * @brief Gestisce il comando 'I' (Imposta TX interval) dal menu seriale.
 *
 * Stampa il preset corrente e la lista dei preset disponibili, poi
 * chiede all'utente un numero (terminato da Invio). Se il numero e'
 * valido applica la modifica (salva in NVS, imposta sendStateNext).
 * Per il preset STORAGE (10) richiede conferma esplicita 'Y'.
 */
void serialMenuHandleInterval() {
    Serial.println();
    Serial.printf("Preset TX interval attuale: %u", cfgTxIntervalPreset);
    if (cfgTxIntervalPreset == TX_PRESET_STORAGE) {
        Serial.println(" (STORAGE MODE)");
    } else {
        Serial.printf(" (%u s)\n", TX_INTERVAL_SECONDS[cfgTxIntervalPreset]);
    }
    Serial.println("--------------------------------------------");
    Serial.println("  0=10s     1=20s    2=1min    3=2min");
    Serial.println("  4=5min    5=10min  6=30min   7=1h");
    Serial.println("  8=1d      9=1w     10=STORAGE");
    Serial.println("--------------------------------------------");
    Serial.print("Digita numero (0-10) e Invio, o Q per annullare: ");

    int preset = serialMenuReadIntWithEnter(15000);   // 15s per digitare

    // Cancellazione o timeout
    if (preset < 0) {
        Serial.println("Annullato, nessuna modifica");
        return;
    }

    // Range check
    if (preset > TX_PRESET_MAX) {
        Serial.printf("Valore non valido: %d (max %u). Annullato.\n",
                      preset, TX_PRESET_MAX);
        return;
    }

    // Preset STORAGE (10): richiedi conferma esplicita
    if (preset == TX_PRESET_STORAGE) {
        Serial.println();
        Serial.println("*** ATTENZIONE: preset 10 = STORAGE MODE ***");
        Serial.println("Il device entrera' in sleep INFINITO dopo il prossimo TX.");
        Serial.println("Per riattivarlo:");
        Serial.println("  - RST + USB collegato -> menu seriale -> W");
        Serial.println("  - Oppure triplo RST (reset totale ai default)");
        if (!serialMenuWaitConfirmation()) {
            Serial.println("Annullato, nessuna modifica");
            return;
        }
    }

    // Applica: salva in NVS + aggiorna RAM + segnala per feedback state
    cfgTxIntervalPreset = (uint8_t)preset;
    saveConfigUChar(NVS_KEY_TX_INTERVAL, cfgTxIntervalPreset);
    sendStateNext = true;
    menuPresetChangedExplicitly = true;   // segnala al fallback VBAT-detect

    if (preset == TX_PRESET_STORAGE) {
        Serial.println("OK: STORAGE MODE impostato");
        Serial.println("Il device andra' in sleep infinito dopo il prossimo TX");
    } else {
        Serial.printf("OK: TX interval a preset %u (%u s)\n",
                      preset, TX_INTERVAL_SECONDS[preset]);
        Serial.println("Applicato dal prossimo ciclo TX");
    }
}

/**
 * @brief Loop principale del menu seriale. Legge comandi e li esegue
 *        finche' l'utente non decide di uscire o scade il timeout.
 *
 * @return Uno di SMR_CONTINUE_NORMAL_BOOT, SMR_FORCE_TX_NOW, SMR_REBOOT
 */
SerialMenuResult serialMenuLoop() {
    uint32_t startMs = millis();
    uint32_t lastHeartbeatMs = 0;
    bool ledState = false;

    while (true) {
        // Timeout globale del menu
        if (millis() - startMs >= SERIAL_MENU_TIMEOUT_MS) {
            Serial.println();
            Serial.println("(timeout - proseguo boot normale)");
            return SMR_CONTINUE_NORMAL_BOOT;
        }

        // Heartbeat LED (blink lento per indicare "sto ascoltando")
        if (millis() - lastHeartbeatMs >= SERIAL_MENU_HEARTBEAT_MS) {
            lastHeartbeatMs = millis();
            ledState = !ledState;
            digitalWrite(PIN_LED, ledState ? HIGH : LOW);
        }

        // Nessun carattere disponibile: aspetta un po' e riprova
        if (!Serial.available()) {
            delay(50);
            continue;
        }

        char c = Serial.read();

        // Ignora whitespace e caratteri di controllo
        if (c == '\r' || c == '\n' || c == ' ' || c == '\t') continue;

        // Reset del timeout ad ogni comando valido
        startMs = millis();
        ledOff();

        // Case-insensitive
        if (c >= 'a' && c <= 'z') c -= ('a' - 'A');

        Serial.println(c);   // echo

        switch (c) {
            case 'W': {
                // Wake da storage: resetta solo cfgTxIntervalPreset
                if (cfgTxIntervalPreset == TX_PRESET_STORAGE) {
                    cfgTxIntervalPreset = TX_INTERVAL_PRESET;
                    saveConfigUChar(NVS_KEY_TX_INTERVAL, cfgTxIntervalPreset);
                    Serial.printf("OK: uscito da STORAGE, TX interval a preset %u (%u s)\n",
                                  TX_INTERVAL_PRESET,
                                  TX_INTERVAL_SECONDS[TX_INTERVAL_PRESET]);
                    sendStateNext = true;
                    menuPresetChangedExplicitly = true;   // segnala al fallback
                } else {
                    Serial.printf("OK: device gia' operativo (preset TX=%u, %u s). Nessuna azione.\n",
                                  cfgTxIntervalPreset,
                                  TX_INTERVAL_SECONDS[cfgTxIntervalPreset]);
                }
                Serial.print("> ");
                break;
            }

            case 'R': {
                // Reset totale ai default di fabbrica
                Serial.println("*** RESET config runtime ai default ***");
                if (serialMenuWaitConfirmation()) {
                    clearRuntimeConfigToDefaults();
                    Serial.println("OK: config resettata (nonces OTAA preservati)");
                    sendStateNext = true;
                }
                Serial.print("> ");
                break;
            }

            case 'I': {
                // Imposta TX interval: chiede numero (0-10), Invio per confermare
                serialMenuHandleInterval();
                Serial.print("> ");
                break;
            }

            case 'S': {
                // Status dump
                serialMenuPrintStatus();
                Serial.print("> ");
                break;
            }

            case 'T': {
                // Force TX now: esci dal menu ed esegui ciclo TX
                Serial.println("Force TX: esco dal menu e trasmetto");
                ledOff();
                return SMR_FORCE_TX_NOW;
            }

            case 'C': {
                // Clear NVS completa (nonces OTAA inclusi)
                Serial.println("*** CLEAR NVS COMPLETA (include nonces OTAA) ***");
                Serial.println("Al prossimo boot il device rifara' il join LoRaWAN");
                if (serialMenuWaitConfirmation()) {
                    serialMenuClearNvsComplete();
                    Serial.println("Riavvio in corso...");
                    delay(500);
                    return SMR_REBOOT;
                }
                Serial.print("> ");
                break;
            }

            case 'N': {
                // Continue Normal boot
                Serial.println("Proseguo boot normale");
                ledOff();
                return SMR_CONTINUE_NORMAL_BOOT;
            }

            case '?':
            case 'H':
                serialMenuPrintBanner();
                break;

            default:
                Serial.printf("Comando sconosciuto: '%c' (digita ? per help)\n", c);
                Serial.print("> ");
                break;
        }
    }
}

/**
 * @brief Entry point del modulo, chiamato dal setup() al boot.
 *
 * Decide se aprire il menu in base al reset reason e a VBAT. Se il menu
 * si apre, ritorna il SerialMenuResult del comando eseguito (o del
 * timeout). Se il menu NON si apre, ritorna SMR_CONTINUE_NORMAL_BOOT.
 *
 * @param vbat_mv tensione batteria letta al boot (per USB detect)
 * @return SerialMenuResult da usare per decidere il flusso successivo
 */
SerialMenuResult checkAndRunSerialMenu(uint16_t vbat_mv) {
    esp_reset_reason_t reason = esp_reset_reason();

    // Condizione 1: solo dopo reset hardware / power-on / USB
    // ESP_RST_USB (11) e' comune sull'ESP32-S3 quando:
    //   - Hai appena caricato firmware via Arduino IDE (l'IDE fa reset USB)
    //   - Colleghi il cavo USB e c'e' un reset di enumerazione
    //   - L'host USB fa un auto-reset del device
    // Senza includerlo, il menu non si apre mai dopo un upload firmware.
    bool isEligibleReset = (reason == ESP_RST_EXT     ||
                            reason == ESP_RST_POWERON ||
                            reason == ESP_RST_USB);
    if (!isEligibleReset) {
        Serial.printf("[MENU] Non apro (reset_reason=%d, non e' EXT/POWERON/USB)\n", (int)reason);
        return SMR_CONTINUE_NORMAL_BOOT;
    }

    // Condizione 2: USB collegato (VBAT alto)
    if (vbat_mv <= VBAT_USB_DETECT_MV) {
        Serial.printf("[MENU] Non apro (VBAT=%u <= soglia=%u, USB probabilmente scollegato)\n",
                      vbat_mv, VBAT_USB_DETECT_MV);
        return SMR_CONTINUE_NORMAL_BOOT;
    }

    // Entrambe le condizioni soddisfatte: apri il menu
    Serial.println("[MENU] Condizioni OK, apro menu tra 1.5s (attesa enum USB)");
    // Attendi enumerazione USB dell'host (~1-2 s dopo il reset)
    delay(SERIAL_MENU_ENUM_DELAY_MS);

    serialMenuPrintBanner();
    return serialMenuLoop();
}

#endif  // ENABLE_SERIAL_MENU


// ===================================================================
// MODULO: LONG_PRESS_STORAGE (BOOT premuto al setup dopo RST)
// ===================================================================
//
// COS'E'
// ------
// Permette di mettere il device in STORAGE MODE premendo il pulsante
// BOOT (GPIO 0) SUBITO DOPO un reset RST, e tenendolo premuto per
// LONG_PRESS_STORAGE_MS millisecondi (3 secondi).
//
// PATTERN OPERATIVO
// -----------------
//   1. Utente preme RST (senza toccare BOOT)
//   2. Il firmware parte
//   3. All'inizio del setup, il firmware legge GPIO 0
//   4. Se GPIO 0 = LOW (BOOT premuto ora):
//      - Accende LED fisso
//      - Loop di polling per 3 secondi
//      - Se BOOT resta LOW per 3s: salva preset=10 (STORAGE), 3 lampi lunghi,
//        enterStorageDeepSleep()
//      - Se BOOT rilasciato prima: annulla, prosegui boot normale
//   5. Se GPIO 0 = HIGH (BOOT non premuto): esce subito
//
// PERCHE' NON RST+BOOT INSIEME
// ----------------------------
// Sull'ESP32-S3, se GPIO 0 e' LOW AL MOMENTO del reset, il chip
// entra in ROM bootloader mode (aspetta comandi USB per flash).
// Il firmware NON parte. Quindi il pattern corretto e':
//   - Premi RST (BOOT non premuto)
//   - Attendi ~1 secondo che il firmware parta
//   - Premi e tieni BOOT per 3 secondi
//
// PERCHE' NON DURANTE IL CICLO ATTIVO
// -----------------------------------
// Durante il deep sleep normale, il firmware non gira, quindi non
// puo' rilevare GPIO 0. L'utente dovrebbe premere BOOT nella
// piccolissima finestra del ciclo attivo (~15s ogni minuto),
// scomodo. Il pattern "RST + BOOT" e' sempre disponibile senza
// aspettare la finestra utile.
//
// COSTO ENERGETICO
// ----------------
// Trascurabile. Un digitalRead() e' quasi gratuito. Se BOOT non
// e' premuto (99% dei casi), il polling termina in pochi
// microsecondi. Il boot prosegue senza ritardi.
//
// CONFLITTI CON UPLOAD FIRMWARE
// -----------------------------
// Nessuno. Arduino IDE fa upload con RST+BOOT insieme, portando
// il chip in bootloader ROM. Il firmware NON parte, questo polling
// NON viene eseguito. Il flash avviene normalmente.
// -------------------------------------------------------------------

#if ENABLE_LONG_PRESS_STORAGE

#define PIN_BOOT                    0       // GPIO 0 = pulsante BOOT sulla Heltec V4
#define LONG_PRESS_STORAGE_MS       3000    // 3 secondi di pressione = attiva storage
#define LONG_PRESS_POLL_MS          50      // periodo di polling del pin

/**
 * @brief Controlla se BOOT e' premuto all'inizio del setup e attiva
 *        storage mode se resta premuto per LONG_PRESS_STORAGE_MS.
 *
 * Deve essere chiamata UNA VOLTA all'inizio del setup, dopo aver
 * gestito il triplo RST ma prima del resto delle inizializzazioni.
 *
 * Comportamento:
 *   - Se BOOT non premuto: ritorna subito (99% dei casi, zero costo)
 *   - Se BOOT premuto: LED fisso + polling per 3s
 *   - Se BOOT rilasciato prima: annulla, ritorna
 *   - Se BOOT tenuto per 3s: salva preset=10 (STORAGE), feedback visivo,
 *     entra in storage sleep infinito (non ritorna mai)
 */
void checkLongPressBootAtSetup() {
    // FIX B: attivo LONG_PRESS solo dopo reset MANUALE dell'utente.
    // Se il boot avviene per wake da deep sleep (timer o GPIO), NON
    // apriamo la finestra di attesa: l'utente non sta interagendo
    // con il device, sprecheremmo 3s ad ogni ciclo TX.
    // Reset "manuali" riconosciuti:
    //   ESP_RST_EXT     = pulsante RST premuto
    //   ESP_RST_POWERON = alimentazione appena collegata
    //   ESP_RST_USB     = auto-reset da IDE dopo upload firmware
    esp_reset_reason_t reason = esp_reset_reason();
    bool isManualReset = (reason == ESP_RST_EXT ||
                          reason == ESP_RST_POWERON ||
                          reason == ESP_RST_USB);
    if (!isManualReset) {
        // Wake da deep sleep normale: nessun check LONG_PRESS,
        // il boot prosegue subito verso il ciclo TX.
        return;
    }

    // Configura GPIO 0 come input con pull-up interno
    pinMode(PIN_BOOT, INPUT_PULLUP);
    delay(5);   // stabilizzazione pull-up

    // FINESTRA DI ATTESA: diamo 3 secondi all'utente per iniziare a
    // premere BOOT dopo i 3 blink veloci di segnalazione "pronto".
    // Se durante questi 3 secondi BOOT viene premuto, entriamo nella
    // fase di conteggio 3s per storage. Altrimenti proseguiamo boot normale.
    Serial.println("[LONG_PRESS] Finestra 3s per premere BOOT (attiva storage)");
    uint32_t waitStart = millis();
    bool bootPressed = false;
    while (millis() - waitStart < 3000) {
        if (digitalRead(PIN_BOOT) == LOW) {
            bootPressed = true;
            break;
        }
        delay(10);
    }

    if (!bootPressed) {
        Serial.println("[LONG_PRESS] BOOT non premuto, proseguo boot normale");
        return;   // finestra scaduta senza pressione BOOT
    }

    // BOOT premuto durante la finestra: entra in modalita' conteggio
    Serial.println();
    Serial.println("[LONG_PRESS] BOOT premuto! Inizio conteggio 3s per STORAGE...");
    Serial.println("[LONG_PRESS] Rilascia BOOT ora per annullare");
    Serial.println("[LONG_PRESS] Tieni premuto BOOT per 3s per attivare STORAGE");

    // Assicura che il LED possa essere pilotato (non in hold da sleep precedente)
    gpio_deep_sleep_hold_dis();
    gpio_hold_dis((gpio_num_t)PIN_LED);
    pinMode(PIN_LED, OUTPUT);
    ledOn();   // LED fisso come feedback "sto contando"

    uint32_t startMs = millis();
    while (digitalRead(PIN_BOOT) == LOW) {
        uint32_t elapsed = millis() - startMs;
        if (elapsed >= LONG_PRESS_STORAGE_MS) {
            // Raggiunta la soglia: attiva storage mode
            Serial.println();
            Serial.println("========================================");
            Serial.println("*** LONG PRESS BOOT DETECTED ***");
            Serial.println("*** STORAGE MODE ATTIVATO ***");
            Serial.println("========================================");
            ledOff();
            delay(200);

            // Salva preset storage in NVS (persistente)
            cfgTxIntervalPreset = TX_PRESET_STORAGE;
            saveConfigUChar(NVS_KEY_TX_INTERVAL, TX_PRESET_STORAGE);

            // Feedback visivo di conferma: 3 lampi lunghi
            for (uint8_t i = 0; i < 3; i++) {
                Serial.printf("[LONG_PRESS] Lampo %u/3\n", i+1);
                ledOn();  delay(1000);
                ledOff(); delay(300);
            }

            // NON chiamiamo sendStatePayload() qui perche' LoRaWAN NON e'
            // ancora inizializzato in questa fase del setup (radio.begin,
            // OTAA join, ecc. avvengono piu' tardi). Se lo facessimo,
            // sendReceive() fallirebbe silenziosamente.
            //
            // Impostiamo invece sendStateNext = true: il flusso normale
            // del setup completera' tutte le init (GPS, SCD, LoRaWAN),
            // mandera' il payload di misure standard, poi vedra' il flag
            // sendStateNext e mandera' anche il payload state 0x43 con
            // cfgTxIntervalPreset = TX_PRESET_STORAGE. Cosi' la webapp
            // riceve il feedback "device andato in storage".
            //
            // Alla fine del ciclo, il check isStorageMode() nel calcolo
            // del sleep finale vede preset=TX_PRESET_STORAGE ed entra
            // in enterStorageDeepSleep() automaticamente.
            sendStateNext = true;
            // IMPORTANTE: segnala al fallback VBAT-detect che questa e'
            // una scelta esplicita dell'utente, NON annullarla!
            // Senza questo flag, il fallback vedrebbe "storage attivo +
            // USB collegato" e resetterebbe il preset a 2 vanificando
            // il long-press. Stesso meccanismo del menu seriale I/W.
            menuPresetChangedExplicitly = true;

            Serial.println("[LONG_PRESS] Preset salvato. Proseguo boot per");
            Serial.println("[LONG_PRESS] fare TX misure + TX state, poi storage sleep.");
            return;   // esce dalla funzione, ma il setup prosegue normalmente
        }
        delay(LONG_PRESS_POLL_MS);
    }

    // Se siamo qui, BOOT e' stato rilasciato PRIMA della soglia. Annulla.
    ledOff();
    uint32_t held = millis() - startMs;
    Serial.printf("[LONG_PRESS] BOOT rilasciato dopo %u ms (serviva %u ms)\n",
                  held, LONG_PRESS_STORAGE_MS);
    Serial.println("[LONG_PRESS] Annullato, proseguo boot normale");
}

#endif  // ENABLE_LONG_PRESS_STORAGE



//
// Chiamato da initLoRaWAN() quando sendReceive ha ricevuto un downlink.
// Dispatch per FPort:
//   FPort 10 = COMANDI DI AZIONE (non persistenti)
//   FPort 20 = CONFIGURAZIONE (persistente in NVS)

#if ENABLE_DOWNLINK_HANDLER

// ================================================================
// Comandi FPort 10 - AZIONI (one-shot, non persistenti)
// ================================================================
// Formato payload downlink: [command_id][argomenti_opzionali]
// Il codec ChirpStack (chirpstack_codec.js) traduce JSON -> byte:
//   {"cmd":"reboot"}       -> [0x01]
//   {"cmd":"identify"}     -> [0x02]
//   {"cmd":"force_tx_now"} -> [0x03]
//   {"cmd":"clear_nvs"}    -> [0x04, 0xA5]
#define CMD_REBOOT         0x01   // riavvio software del device (ESP.restart())
#define CMD_IDENTIFY       0x02   // LED lampeggia 10 volte per riconoscimento visivo
#define CMD_FORCE_TX_NOW   0x03   // salta lo sleep normale, prossimo TX dopo 2s
#define CMD_CLEAR_NVS      0x04   // cancella NVS namespace 'lora' (richiede 0xA5 magic)
#define CMD_IDENTIFY_ON    0x05   // attiva modalita' identify persistente
#define CMD_IDENTIFY_OFF   0x06   // disattiva modalita' identify persistente
#define CMD_GET_STATE      0x07   // richiede invio payload state 0x43 nel prossimo TX

// ================================================================
// Comandi FPort 20 - CONFIGURAZIONE (persistente in NVS)
// ================================================================
// Formato payload downlink: [config_id][argomenti_binari]
// Ogni comando modifica un parametro runtime salvandolo nella chiave NVS
// corrispondente. La modifica viene applicata subito (nella variabile cfg*)
// e persiste attraverso deep sleep, restart, power-off.
// Al boot successivo, loadRuntimeConfig() rilegge da NVS il valore aggiornato.
#define CFG_SET_TX_INTERVAL   0x11   // 1 byte, preset 0-10 (10=STORAGE) -> NVS "tx_int"
#define CFG_SET_LORAWAN_SF    0x12   // 1 byte, SF 7-12 -> NVS "lora_sf"
#define CFG_SET_TX_POWER      0x13   // 1 byte, dBm 2-14 -> NVS "tx_pow"
#define CFG_SET_GPS_TIMEOUT   0x14   // 2 byte LE, secondi 10-300 -> NVS "gps_t"
#define CFG_SET_BATT_THRESH   0x15   // 4 byte (2xuint16 LE), mV -> NVS "vbat_em"/"vbat_rc"
#define CFG_SET_ADR_ENABLED   0x16   // 1 byte, 0/1 -> NVS "adr" (effettivo solo in OTAA)
// ---- Nuovi comandi features di risparmio energetico ----
#define CFG_SET_GPS_SKIP      0x17   // 1 byte, N=fix ogni N+1 cicli (modulo GPS SKIP)
#define CFG_SET_GPS_ENABLED   0x18   // 1 byte, 0/1 (modulo SENSOR DISABLE)
#define CFG_SET_SCD_ENABLED   0x19   // 1 byte, 0/1 (modulo SENSOR DISABLE)

// Fa lampeggiare il LED N volte per identificare visivamente il device.
// NOTA: durante il ciclo il LED e' gia' acceso (ledOn() nel setup), quindi
// partiamo da SPENTO per dare contrasto, altrimenti i lampeggi si confondono
// con lo stato acceso di base e non si vedono. Alla fine lo rimettiamo acceso
// per non alterare lo stato del ciclo (verra' spento al deep sleep).
void identifyBlink(uint8_t times) {
    ledOff();
    delay(300);
    for (uint8_t i = 0; i < times; i++) {
        ledOn();
        delay(150);
        ledOff();
        delay(150);
    }
    ledOn();   // ripristina lo stato "acceso" del ciclo
}

// Lampeggia il LED in continuo per 'ms' millisecondi, pettinando il watchdog.
// Usato dalla modalita' identify (option B): il device resta sveglio e
// lampeggia tra un uplink e l'altro. A fine funzione il LED resta spento.
void blinkFor(uint32_t ms) {
    uint32_t start = millis();
    bool on = false;
    while (millis() - start < ms) {
        on = !on;
        digitalWrite(PIN_LED, on ? HIGH : LOW);
#if ENABLE_WATCHDOG
        esp_task_wdt_reset();
#endif
        delay(IDENTIFY_BLINK_MS);
    }
    ledOff();
}

// Cancella l'intera NVS (namespace 'lora'). Riavvia poi il device.
void clearNvsAndReboot() {
    Serial.println("[DOWNLINK] CLEAR_NVS: cancellazione namespace 'lora'...");
    if (prefs.begin(NVS_NAMESPACE, false)) {
        prefs.clear();
        prefs.end();
        Serial.println("[DOWNLINK] NVS cancellata, riavvio...");
    } else {
        Serial.println("[DOWNLINK] Errore apertura NVS, riavvio comunque...");
    }
    delay(500);
    ESP.restart();
}

void handleDownlinkPort10(uint8_t* buf, size_t len) {
    if (len < 1) {
        Serial.println("[DOWNLINK] FPort 10: payload vuoto");
        return;
    }
    uint8_t cmd = buf[0];
    switch (cmd) {
        case CMD_REBOOT:
            Serial.println("[DOWNLINK] REBOOT ricevuto, riavvio...");
            delay(500);
            ESP.restart();
            break;

        case CMD_IDENTIFY:
            Serial.println("[DOWNLINK] IDENTIFY: lampeggio LED");
            identifyBlink(10);
            break;

        case CMD_IDENTIFY_ON:
            Serial.println("[DOWNLINK] IDENTIFY_ON: modalita' identify persistente attivata");
            Serial.printf("           blink continuo, TX ogni %us, timeout %us (~%u min)\n",
                          IDENTIFY_TX_INTERVAL_S, IDENTIFY_TIMEOUT_S, IDENTIFY_TIMEOUT_S / 60);
            identifyMode = true;
            break;

        case CMD_IDENTIFY_OFF:
            Serial.println("[DOWNLINK] IDENTIFY_OFF: modalita' identify disattivata");
            identifyMode = false;
            break;

        case CMD_FORCE_TX_NOW:
            Serial.println("[DOWNLINK] FORCE_TX_NOW: prossimo ciclo TX immediato");
            forceTxNow = true;   // resta in RTC memory fino al prossimo boot
            break;

        case CMD_CLEAR_NVS:
            if (len < 2 || buf[1] != 0xA5) {
                Serial.println("[DOWNLINK] CLEAR_NVS: manca byte magic 0xA5, ignoro");
                return;
            }
            clearNvsAndReboot();
            break;

        case CMD_GET_STATE:
            Serial.println("[DOWNLINK] GET_STATE: verra' inviato payload state nel prossimo TX");
            sendStateNext = true;
            break;

        default:
            Serial.printf("[DOWNLINK] Comando FPort 10 sconosciuto: 0x%02X\n", cmd);
            break;
    }
}

void handleDownlinkPort20(uint8_t* buf, size_t len) {
    if (len < 1) {
        Serial.println("[DOWNLINK] FPort 20: payload vuoto");
        return;
    }
    uint8_t cmd = buf[0];
    switch (cmd) {

        case CFG_SET_TX_INTERVAL:
            if (len < 2 || buf[1] > TX_PRESET_MAX) {
                Serial.printf("[DOWNLINK] SET_TX_INTERVAL: preset non valido (max %u)\n",
                              TX_PRESET_MAX);
                return;
            }
            if (saveConfigUChar(NVS_KEY_TX_INTERVAL, buf[1])) {
                cfgTxIntervalPreset = buf[1];
                if (buf[1] == TX_PRESET_STORAGE) {
                    Serial.printf("[DOWNLINK] SET_TX_INTERVAL: preset=%u (STORAGE MODE)\n",
                                  TX_PRESET_STORAGE);
                    Serial.println("           il device entrera' in sleep INFINITO dopo questo TX");
                } else {
                    Serial.printf("[DOWNLINK] SET_TX_INTERVAL: preset=%u (%us)\n",
                                  buf[1], TX_INTERVAL_SECONDS[buf[1]]);
                }
                sendStateNext = true;   // feedback state al prossimo TX
            }
            break;

        case CFG_SET_LORAWAN_SF:
            if (len < 2 || buf[1] < 7 || buf[1] > 12) {
                Serial.println("[DOWNLINK] SET_LORAWAN_SF: SF fuori range 7-12");
                return;
            }
            if (saveConfigUChar(NVS_KEY_LORAWAN_SF, buf[1])) {
                cfgLoRaWANSF = buf[1];
                Serial.printf("[DOWNLINK] SET_LORAWAN_SF: SF%u\n", buf[1]);
#if USE_OTAA
                if (cfgAdr) {
                    Serial.println("[DOWNLINK] WARNING: ADR attivo, sovrascrivera' l'SF");
                    Serial.println("           usa set_adr_enabled=0 prima, se vuoi forzare SF manuale");
                }
#endif
                sendStateNext = true;
            }
            break;

        case CFG_SET_TX_POWER:
            if (len < 2 || buf[1] < 2 || buf[1] > 14) {
                Serial.println("[DOWNLINK] SET_TX_POWER: fuori range 2-14 dBm");
                return;
            }
            if (saveConfigUChar(NVS_KEY_TX_POWER, buf[1])) {
                cfgTxPower = buf[1];
                Serial.printf("[DOWNLINK] SET_TX_POWER: %u dBm\n", buf[1]);
                sendStateNext = true;
            }
            break;

        case CFG_SET_GPS_TIMEOUT: {
            if (len < 3) {
                Serial.println("[DOWNLINK] SET_GPS_TIMEOUT: attesi 2 byte LE");
                return;
            }
            uint16_t sec = buf[1] | (buf[2] << 8);
            if (sec < 10 || sec > 300) {
                Serial.println("[DOWNLINK] SET_GPS_TIMEOUT: fuori range 10-300s");
                return;
            }
            if (saveConfigUShort(NVS_KEY_GPS_TIMEOUT, sec)) {
                cfgGpsTimeoutS = sec;
                Serial.printf("[DOWNLINK] SET_GPS_TIMEOUT: %us\n", sec);
                sendStateNext = true;
            }
            break;
        }

        case CFG_SET_BATT_THRESH: {
            if (len < 5) {
                Serial.println("[DOWNLINK] SET_BATT_THRESH: attesi 4 byte");
                return;
            }
            uint16_t em = buf[1] | (buf[2] << 8);
            uint16_t rc = buf[3] | (buf[4] << 8);
            if (em < 2500 || em > 4200 || rc < 2500 || rc > 4200 || rc <= em) {
                Serial.println("[DOWNLINK] SET_BATT_THRESH: soglie invalide");
                return;
            }
            if (saveConfigUShort(NVS_KEY_VBAT_EMERG, em) &&
                saveConfigUShort(NVS_KEY_VBAT_RECOV, rc)) {
                cfgVbatEmergencyMv = em;
                cfgVbatRecoveryMv  = rc;
                Serial.printf("[DOWNLINK] SET_BATT_THRESH: emergency=%umV recovery=%umV\n",
                              em, rc);
                sendStateNext = true;
            }
            break;
        }

        case CFG_SET_ADR_ENABLED: {
            if (len < 2 || (buf[1] != 0 && buf[1] != 1)) {
                Serial.println("[DOWNLINK] SET_ADR_ENABLED: valore deve essere 0 o 1");
                return;
            }
#if !USE_OTAA
            Serial.println("[DOWNLINK] SET_ADR_ENABLED: valore salvato ma ADR ignorato in modo ABP");
#endif
            if (saveConfigUChar(NVS_KEY_ADR, buf[1])) {
                cfgAdr = (buf[1] != 0);
                Serial.printf("[DOWNLINK] SET_ADR_ENABLED: %s\n",
                              cfgAdr ? "abilitato" : "disabilitato");
                Serial.println("           applicato al prossimo initLoRaWAN (reboot o wake)");
                sendStateNext = true;
            }
            break;
        }

#if ENABLE_GPS_SKIP
        case CFG_SET_GPS_SKIP: {
            if (len < 2) {
                Serial.println("[DOWNLINK] SET_GPS_SKIP: manca il valore");
                return;
            }
            // Nessun sanity check sul valore: 0-255 sono tutti validi
            if (saveConfigUChar(NVS_KEY_GPS_SKIP, buf[1])) {
                cfgGpsSkipCycles = buf[1];
                rtcGpsSkipCounter = 0;   // resetta il contatore
                if (buf[1] == 0) {
                    Serial.println("[DOWNLINK] SET_GPS_SKIP: 0 (fix GPS ogni ciclo)");
                } else {
                    Serial.printf("[DOWNLINK] SET_GPS_SKIP: %u (fix GPS ogni %u cicli)\n",
                                  buf[1], (unsigned)(buf[1] + 1));
                }
                sendStateNext = true;
            }
            break;
        }
#endif

#if ENABLE_SENSOR_DISABLE
        case CFG_SET_GPS_ENABLED: {
            if (len < 2 || (buf[1] != 0 && buf[1] != 1)) {
                Serial.println("[DOWNLINK] SET_GPS_ENABLED: valore deve essere 0 o 1");
                return;
            }
            if (saveConfigUChar(NVS_KEY_GPS_ENABLED, buf[1])) {
                cfgGpsEnabled = (buf[1] != 0);
                Serial.printf("[DOWNLINK] SET_GPS_ENABLED: %s\n",
                              cfgGpsEnabled ? "GPS attivo" : "GPS disattivato");
                sendStateNext = true;
            }
            break;
        }

        case CFG_SET_SCD_ENABLED: {
            if (len < 2 || (buf[1] != 0 && buf[1] != 1)) {
                Serial.println("[DOWNLINK] SET_SCD_ENABLED: valore deve essere 0 o 1");
                return;
            }
            if (saveConfigUChar(NVS_KEY_SCD_ENABLED, buf[1])) {
                cfgScdEnabled = (buf[1] != 0);
                Serial.printf("[DOWNLINK] SET_SCD_ENABLED: %s\n",
                              cfgScdEnabled ? "SCD41 attivo" : "SCD41 disattivato");
                sendStateNext = true;
            }
            break;
        }
#endif

        default:
            Serial.printf("[DOWNLINK] Comando FPort 20 sconosciuto: 0x%02X\n", cmd);
            break;
    }
}

// Entry point del dispatcher: chiamato dopo sendReceive se downlink presente
void handleDownlink(uint8_t port, uint8_t* buf, size_t len) {
    Serial.printf("[DOWNLINK] FPort=%u len=%u data=", port, (unsigned)len);
    for (size_t i = 0; i < len; i++) {
        if (buf[i] < 0x10) Serial.print('0');
        Serial.print(buf[i], HEX);
    }
    Serial.println();

    switch (port) {
        case 10: handleDownlinkPort10(buf, len); break;
        case 20: handleDownlinkPort20(buf, len); break;
        default:
            Serial.printf("[DOWNLINK] FPort %u non gestito\n", port);
            break;
    }
}

#endif  // ENABLE_DOWNLINK_HANDLER


// =============================================================
// SCD41
// =============================================================

// Indirizzo I2C del sensore SCD41 (fisso da datasheet Sensirion, non modificabile)
#define SCD41_I2C_ADDR 0x62

bool initScd41() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
    // NUOVA API: begin(bus, address)
    scd4x.begin(Wire, SCD41_I2C_ADDR);

    // Ferma eventuali misure precedenti (dopo un reset "sporco")
    uint16_t err = scd4x.stopPeriodicMeasurement();
    delay(500);

    // NUOVA API: getSerialNumber ritorna un unico uint64_t
    uint64_t serialNumber = 0;
    err = scd4x.getSerialNumber(serialNumber);
    if (err) {
        Serial.printf("SCD41 getSerialNumber error: %u\n", err);
        return false;
    }
    Serial.printf("SCD41 SN: 0x%016llX\n", serialNumber);

    // Avvia misure periodiche (una ogni 5 secondi)
    err = scd4x.startPeriodicMeasurement();
    if (err) {
        Serial.printf("SCD41 startPeriodicMeasurement error: %u\n", err);
        return false;
    }
    return true;
}

/**
 * Blocca fino a quando SCD41 ha una misura pronta o timeout.
 * Ritorna true se lettura ok.
 */
bool readScd41(uint16_t& co2, float& tempC, float& humRH, uint32_t timeout_ms = 10000) {
    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        // getDataReadyStatus vuole un bool& in questa versione della libreria
        bool dataReady = false;
        uint16_t err = scd4x.getDataReadyStatus(dataReady);
        if (!err && dataReady) {
            err = scd4x.readMeasurement(co2, tempC, humRH);
            if (err) {
                Serial.printf("SCD41 readMeasurement error: %u\n", err);
                return false;
            }
            return true;
        }
        delay(200);
    }
    Serial.println("SCD41 timeout: nessuna misura pronta");
    return false;
}

// =============================================================
// GPS L76K
// =============================================================

void initGps() {
    GpsSerial.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
}

/**
 * Attende un fix GPS valido entro il timeout indicato.
 * Ritorna true se fix ottenuto, false altrimenti.
 */
bool waitForGpsFix(uint32_t timeout_s) {
    Serial.printf("Attendo fix GPS (max %u s)...\n", timeout_s);
    uint32_t start = millis();
    uint32_t lastReport = start;
    uint32_t bytesRx = 0;
    uint32_t sentencesRx = 0;

    while (millis() - start < timeout_s * 1000UL) {
        while (GpsSerial.available()) {
            char c = GpsSerial.read();
            gps.encode(c);
            bytesRx++;
            if (c == '\n') sentencesRx++;
        }
        if (gps.location.isValid() && gps.location.isUpdated()
            && gps.hdop.isValid() && gps.hdop.hdop() < 50.0) {
            Serial.printf("Fix ok in %lu ms: lat=%.6f lon=%.6f sat=%d hdop=%.2f\n",
                          millis() - start,
                          gps.location.lat(), gps.location.lng(),
                          gps.satellites.value(), gps.hdop.hdop());
            return true;
        }

        // Log diagnostico ogni 5 secondi
        if (millis() - lastReport >= 5000) {
            lastReport = millis();

            // Reset del watchdog: il fix GPS puo' richiedere 30-90s in cold
            // start, ben oltre il timeout wdt se non lo resettassimo qui
#if ENABLE_WATCHDOG
            esp_task_wdt_reset();
#endif

            Serial.printf("  [GPS] %lus  RX bytes=%lu  frasi NMEA=%lu  sat.in.fix=%d",
                          (millis() - start) / 1000,
                          bytesRx, sentencesRx,
                          gps.satellites.isValid() ? gps.satellites.value() : 0);
            if (gps.location.isValid()) {
                Serial.printf("  fix=YES (%.4f,%.4f) hdop=%.1f",
                              gps.location.lat(), gps.location.lng(),
                              gps.hdop.isValid() ? gps.hdop.hdop() : 99.9);
            } else {
                Serial.print("  fix=NO");
            }
            Serial.println();
            if (bytesRx == 0) {
                Serial.println("  [GPS] !! NESSUN BYTE RICEVUTO dal GPS !!");
                Serial.println("  [GPS] Verifica: alimentazione VGNSS, cablaggio TX/RX, GPIO 34 alto");
            }
        }

        delay(50);
    }
    Serial.printf("GPS timeout dopo %lu bytes ricevuti, %lu frasi NMEA\n",
                  bytesRx, sentencesRx);
    return false;
}

// =============================================================
// LoRaWAN
// =============================================================

/**
 * Deriva il DevEUI a 64 bit dal MAC address a 48 bit dell'ESP32.
 * Pattern IEEE MAC48-to-EUI64: inserisce FF:FE in mezzo.
 *   MAC 48-bit:  AA:BB:CC:DD:EE:FF
 *   DevEUI 64:   AA:BB:CC:FF:FE:DD:EE:FF
 * Il valore e' stabile per il chip (dipende dall'efuse) e unico globalmente.
 */
void getDevEuiFromMac(uint8_t devEui[8]) {
    uint64_t chipMac = ESP.getEfuseMac();
    uint8_t mac[6];
    for (int i = 0; i < 6; i++) {
        mac[i] = (chipMac >> (8 * i)) & 0xFF;
    }
    devEui[0] = mac[0]; devEui[1] = mac[1]; devEui[2] = mac[2];
    devEui[3] = 0xFF;   devEui[4] = 0xFE;
    devEui[5] = mac[3]; devEui[6] = mac[4]; devEui[7] = mac[5];
}

// Delay che resetta il watchdog durante attese lunghe (es. backoff join).
// Un delay() lungo lascerebbe il wdt senza reset e scatterebbe un reset spurio.
void wdtSafeDelay(uint32_t ms) {
    uint32_t start = millis();
    while (millis() - start < ms) {
#if ENABLE_WATCHDOG
        esp_task_wdt_reset();
#endif
        uint32_t remaining = ms - (millis() - start);
        delay(remaining > 200 ? 200 : remaining);
    }
}

bool initLoRaWAN() {
    Serial.println("Init SPI e SX1262...");
    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI, PIN_LORA_NSS);

    // Init radio con ConfigLoRa_t come nell'esempio ufficiale LoRaWAN_ABP.ino
    ConfigLoRa_t config;
    config.frequency = 868;
    int16_t state = radio.begin(config);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.printf("Radio begin failed: %d\n", state);
        return false;
    }
    Serial.println("Radio begin OK");

#if USE_OTAA
    // ================================================================
    // OTAA - Over-The-Air Activation
    // ================================================================
    // Il device manda una JoinRequest, il NS risponde con JoinAccept.
    // DevAddr e chiavi di sessione (NwkSKey, AppSKey) vengono derivate
    // dinamicamente. Bisogna persistere i nonces per non ripeterli.

    // Deriva il DevEUI come uint64_t (formato richiesto da beginOTAA)
    // La funzione getDevEuiFromMac riempie un array di 8 byte, lo componiamo
    // in un uint64_t rispettando il big-endian del formato LoRaWAN.
    uint8_t devEuiBytes[8];
    getDevEuiFromMac(devEuiBytes);
    uint64_t devEui = 0;
    for (int i = 0; i < 8; i++) {
        devEui = (devEui << 8) | devEuiBytes[i];
    }

    Serial.printf("beginOTAA con AppEUI=%016llX, DevEUI=%016llX\n",
                  appEui, devEui);

    // beginOTAA(joinEUI, devEUI, nwkKey, appKey)
    // In LoRaWAN 1.0.x esiste solo AppKey (nwkKey = NULL o = appKey).
    // Passiamo NULL per nwkKey: RadioLib in modalita' 1.0 usera' appKey
    // per entrambi i ruoli crittografici.
    int16_t rc = node.beginOTAA(appEui, devEui, NULL, (uint8_t*)appKey);
    if (rc != RADIOLIB_ERR_NONE) {
        Serial.printf("beginOTAA failed: %d\n", rc);
        return false;
    }

    // Ripristino stato dai join precedenti per evitare un nuovo join:
    //   NONCES   -> da RTC memory, altrimenti da NVS (servono sempre, anche
    //               per un eventuale rejoin, per non riusare un DevNonce)
    //   SESSIONE -> solo da RTC memory (mai da NVS, vedi sopra)
    // Se manca la sessione valida in RTC -> si esegue un join OTAA.
    bool restored = false;

    if (rtcNoncesValid) {
        int16_t r = node.setBufferNonces(rtcNoncesBuffer);
        if (r == RADIOLIB_ERR_NONE) {
            Serial.println("Nonces ripristinati da RTC memory");
        }
    }
#if ENABLE_NVS_PERSISTENCE
    if (!rtcNoncesValid) {
        uint8_t noncesBuf[RADIOLIB_LORAWAN_NONCES_BUF_SIZE];
        if (loadNoncesFromNVS(noncesBuf)) {
            int16_t r = node.setBufferNonces(noncesBuf);
            if (r == RADIOLIB_ERR_NONE) {
                Serial.println("Nonces ripristinati da NVS");
                memcpy(rtcNoncesBuffer, noncesBuf,
                       RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
                rtcNoncesValid = true;
            }
        }
    }
#endif
    // Ripristino della sessione: UNICA fonte e' la RTC memory.
    // Volutamente NON si legge da NVS: una sessione vecchia in flash avrebbe
    // un FCnt gia' superato e ChirpStack scarterebbe gli uplink come replay.
    // Se la sessione RTC c'e' ed e' valida, saltiamo il join (risparmio airtime);
    // altrimenti (power-off, primo boot) si procede con un join pulito.
    if (rtcSessionValid) {
        if (node.setBufferSession(rtcSessionBuffer) == RADIOLIB_ERR_NONE) {
            // activateOTAA dopo il restore = "formalita'" che marca il nodo
            // attivato; senza, sendReceive puo' dare -1101 NETWORK_NOT_JOINED.
            int16_t a = node.activateOTAA();
            if (a == RADIOLIB_LORAWAN_SESSION_RESTORED) {
                Serial.println("Sessione OTAA ripristinata da RTC, join saltato");
                restored = true;
            } else if (a == RADIOLIB_LORAWAN_NEW_SESSION) {
                // buffer RTC non valido: ha rifatto un join vero
                Serial.println("Restore RTC non valido: nuovo join eseguito");
                restored = true;
#if ENABLE_NVS_PERSISTENCE
                saveNoncesToNVS();
#endif
                cacheSessionInRTC();
            } else {
                Serial.printf("[OTAA] activateOTAA post-restore RTC: %d (rifaccio join)\n", a);
            }
        } else {
            Serial.println("[OTAA] setBufferSession(RTC) fallito, rifaccio join");
        }
    }

    // Nessuna sessione valida -> avvia join con retry
    if (!restored) {
        Serial.println("Avvio join OTAA...");
        const int MAX_JOIN_ATTEMPTS = 3;
        for (int attempt = 1; attempt <= MAX_JOIN_ATTEMPTS; attempt++) {
#if ENABLE_WATCHDOG
            esp_task_wdt_reset();
#endif
            Serial.printf("  Join tentativo %d/%d...\n", attempt, MAX_JOIN_ATTEMPTS);
            int16_t st = node.activateOTAA();

            // In RadioLib 7.x sono ENTRAMBI codici di successo:
            //   RADIOLIB_LORAWAN_NEW_SESSION      (-1118) = nuovo join riuscito
            //   RADIOLIB_LORAWAN_SESSION_RESTORED (-1117) = sessione ripristinata
            //                                              dai buffer nonces/session
            if (st == RADIOLIB_LORAWAN_NEW_SESSION ||
                st == RADIOLIB_LORAWAN_SESSION_RESTORED) {
                if (st == RADIOLIB_LORAWAN_NEW_SESSION) {
                    Serial.println("Join OTAA riuscito (nuova sessione)!");
                } else {
                    Serial.println("Sessione OTAA ripristinata dai buffer!");
                }
                restored = true;
                // Salva subito nonces (contengono il nuovo DevNonce usato)
#if ENABLE_NVS_PERSISTENCE
                saveNoncesToNVS();
#endif
                // Aggiorna nonces in RTC anche se NVS disabilitata
                uint8_t* noncesPtr = node.getBufferNonces();
                if (noncesPtr) {
                    memcpy(rtcNoncesBuffer, noncesPtr,
                           RADIOLIB_LORAWAN_NONCES_BUF_SIZE);
                    rtcNoncesValid = true;
                }
                cacheSessionInRTC();
                break;
            }
            Serial.printf("  Join fallito rc=%d\n", st);
            // Backoff crescente prima del prossimo tentativo
            if (attempt < MAX_JOIN_ATTEMPTS) {
                uint32_t backoff_s = 30UL * attempt;
                Serial.printf("  Attesa %us prima del prossimo tentativo\n", backoff_s);
                wdtSafeDelay(backoff_s * 1000UL);
            }
        }
        if (!restored) {
            Serial.println("Tutti i tentativi di join OTAA falliti");
            return false;
        }
    }

#else
    // ================================================================
    // ABP - Activation By Personalization
    // ================================================================
    // Chiavi statiche (DevAddr + NwkSKey + AppSKey): la sessione e' gia'
    // attiva al boot, nessun join radio. Il DevAddr in ABP e' FISSO e va
    // configurato IDENTICO su device e Network Server (vedi nota in fondo).
    //
    // ------------------------------------------------------------------
    //  COMPORTAMENTO DEL FRAME COUNTER IN ABP - LEGGERE
    // ------------------------------------------------------------------
    // A differenza dell'OTAA, in ABP NON e' possibile persistere il frame
    // counter tra i cicli con questa libreria: RadioLib 7.x scarta il
    // ripristino di sessione ABP con -1120 (RADIOLIB_ERR_SESSION_DISCARDED),
    // sia da RTC sia da NVS, perche' la ricostruzione dello stato canali
    // presuppone un join che in ABP non avviene. Non esiste nemmeno una
    // setFCntUp() pubblica per reiniettare il contatore. L'esempio ufficiale
    // LoRaWAN_ABP.ino infatti fa solo beginABP() + activateABP(), senza
    // alcun restore: e' quello che replichiamo qui.
    //
    // CONSEGUENZA: ad OGNI ciclo (ogni wake da deep sleep e ogni power-on) il
    // FCnt riparte da 0. Verificato sul campo: uplink consecutivi arrivano
    // tutti con FCnt=0.
    //
    // PERCHE' FUNZIONA COMUNQUE: il Network Server deve avere la validazione
    // del frame counter DISABILITATA/RILASSATA per questo device (vedi nota
    // "CONFIG SERVER" in fondo). Cosi' accetta uplink ripetuti con FCnt=0
    // invece di scartarli come replay.
    //
    // COMPROMESSO: con FCnt fisso a 0 + check rilassato si RINUNCIA alla
    // protezione anti-replay del frame counter. Accettabile per telemetria
    // ambientale in rete privata; NON adatto a dati sensibili. Se serve la
    // protezione anti-replay, usare OTAA (USE_OTAA=1).
    // ------------------------------------------------------------------

    // beginABP per LoRaWAN 1.0.x: esiste solo NwkSKey, quindi
    // fNwkSIntKey = sNwkSIntKey = NULL, nwkSEncKey = nwkSKey.
    node.beginABP(devAddr, NULL, NULL, nwkSKey, appSKey);
    Serial.println("beginABP (LoRaWAN 1.0: fNwk/sNwk = NULL)");

    // activateABP: crea la sessione ABP. Restituisce codici NEGATIVI che sono
    // SUCCESSI (NEW_SESSION -1118, SESSION_RESTORED -1117), quindi NON si puo'
    // usare "< 0" per rilevare un errore. Accettiamo i tre codici di successo.
    int16_t abpState = node.activateABP();
    bool abpOk = (abpState == RADIOLIB_ERR_NONE) ||
                 (abpState == RADIOLIB_LORAWAN_NEW_SESSION) ||
                 (abpState == RADIOLIB_LORAWAN_SESSION_RESTORED);
    Serial.printf("activateABP ret=%d %s\n", abpState,
                  abpOk ? "(ok)" : "(ERRORE)");
    if (!abpOk) {
        Serial.println("ABP: attivazione fallita, abort ciclo");
        return false;
    }
    Serial.println("ABP attivo. FCnt riparte da 0 ad ogni ciclo (vedi nota nel codice).");
    Serial.println("Richiede 'relaxed frame-counter' sul Network Server.");

    // NOTA: niente setBufferSession / cacheSessionInRTC qui. In ABP il
    // ripristino non e' supportato (darebbe -1120) e ricreiamo la sessione da
    // zero ogni volta di proposito. Le funzioni save/loadSessionFromNVS
    // restano definite ma non sono usate in ABP.
#endif  // USE_OTAA

    // Imposta potenza TX dalla config runtime (modificabile via downlink)
    // 14 dBm = max legale EU868. Range configurabile: 2-14 dBm
    int16_t txPowerState = node.setTxPower(cfgTxPower);
    if (txPowerState == RADIOLIB_ERR_NONE) {
        Serial.printf("TX Power impostato a %u dBm\n", cfgTxPower);
    } else {
        Serial.printf("setTxPower warning: %d\n", txPowerState);
    }

    // Attivazione ADR (Adaptive Data Rate)
    // Politica: attivo solo in modalita' OTAA. In ABP il device dimentica
    // lo stato ADR ad ogni reboot e va in desync col NS
#if USE_OTAA
    if (cfgAdr) {
        node.setADR(true);
        Serial.println("ADR attivo (OTAA, cfgAdr=1)");
    } else {
        node.setADR(false);
        Serial.println("ADR disabilitato (cfgAdr=0)");
    }
#else
    node.setADR(false);
    Serial.println("ADR ignorato: modo ABP non supporta ADR affidabilmente");
#endif

    Serial.printf("FCnt uplink corrente: %lu\n", node.getFCntUp());
    Serial.printf("LoRaWAN %s setup completato\n", USE_OTAA ? "OTAA" : "ABP");
    return true;
}

// =============================================================
// PAYLOAD STATE 0x43
// =============================================================
//
// Costruisce il payload state con i valori correnti del device.
// I feature flags sono impacchettati in un singolo byte per efficienza.
// La batteria viene letta appena prima (valore fresco).

/**
 * Impacchetta i feature flags compile-time in un singolo byte.
 * L'ordine dei bit e' documentato nel commento della struct Payload_v0x43.
 */
uint8_t buildFeatureFlags() {
    uint8_t flags = 0;
#if USE_OTAA
    flags |= (1 << 0);
#endif
#if ENABLE_NVS_PERSISTENCE
    flags |= (1 << 1);
#endif
#if ENABLE_DOWNLINK_HANDLER
    flags |= (1 << 2);
#endif
#if ENABLE_WATCHDOG
    flags |= (1 << 3);
#endif
#if ENABLE_BATTERY_PROTECTION
    flags |= (1 << 4);
#endif
#if DEBUG_NO_DEEP_SLEEP
    flags |= (1 << 5);
#endif
#if ENABLE_STORAGE_MODE
    flags |= (1 << 6);
#endif
#if ENABLE_TRIPLE_RST_RESET
    flags |= (1 << 7);
#endif
    return flags;
}

/**
 * Codifica esp_reset_reason() in un byte per il payload state.
 * L'enum ESP-IDF sta gia' in un byte, cast diretto.
 * Valori tipici che ci si aspetta:
 *   1 = ESP_RST_POWERON     (power-on da 0)
 *   3 = ESP_RST_SW          (ESP.restart())
 *   4 = ESP_RST_PANIC       (crash / panic)
 *   5 = ESP_RST_INT_WDT     (interrupt watchdog)
 *   6 = ESP_RST_TASK_WDT    (task watchdog - il nostro ENABLE_WATCHDOG)
 *   7 = ESP_RST_WDT         (altri watchdog)
 *   8 = ESP_RST_DEEPSLEEP   (wake da deep sleep)
 *  11 = ESP_RST_BROWNOUT    (calo di tensione)
 */
uint8_t getResetReasonByte() {
    return (uint8_t)esp_reset_reason();
}

/**
 * Costruisce il payload state 0x43 e lo trasmette su FPort 2.
 * Ritorna true se TX riuscito, false altrimenti.
 * Non ha effetti collaterali sulla sessione LoRaWAN oltre il TX stesso.
 */
bool sendStatePayload() {
    Payload_v0x43 state = {};
    state.schema_id           = SCHEMA_ID_STATE;
    state.bootCount           = bootCount;
    // Uptime "wall-clock" (semantica B): tempo accumulato in precedenti sleep
    // + tempo trascorso da questo boot. millis() si azzera ad ogni wake ma
    // uptimeAccumulatedS in RTC persiste attraverso i deep sleep.
    state.uptime_s            = uptimeAccumulatedS + (millis() / 1000);
    // Batteria letta al momento (valore fresco, ~10ms per lettura ADC)
    state.battery_pct         = vbatToPercent(readBatteryMv());
    state.cfgTxIntervalPreset = cfgTxIntervalPreset;
    state.cfgLoRaWANSF        = cfgLoRaWANSF;
    state.cfgTxPower          = cfgTxPower;
    state.cfgGpsTimeoutS      = cfgGpsTimeoutS;
    state.cfgVbatEmergencyMv  = cfgVbatEmergencyMv;
    state.cfgVbatRecoveryMv   = cfgVbatRecoveryMv;
    state.cfgAdr              = cfgAdr ? 1 : 0;
    state.featureFlags        = buildFeatureFlags();
    state.fwVersion           = FW_VERSION;
    state.resetReason         = getResetReasonByte();
    // ---- Estensione v2: features risparmio energetico ----
    state.cfgGpsEnabled       = cfgGpsEnabled ? 1 : 0;
    state.cfgGpsSkipCycles    = cfgGpsSkipCycles;
    state.cfgScdEnabled       = cfgScdEnabled ? 1 : 0;

    Serial.printf("TX STATE %u byte su FPort %u: ",
                  (unsigned)sizeof(state), LORAWAN_FPORT_STATE);
    printHex((uint8_t*)&state, sizeof(state));
    Serial.println();

    // TX senza attesa downlink dedicata (usa la firma minima).
    // Il downlink handler in caso di risposta arrivera' col prossimo uplink
    // di misure standard (piu' frequente).
    int16_t st = node.sendReceive((uint8_t*)&state, sizeof(state),
                                  LORAWAN_FPORT_STATE);
    if (st < RADIOLIB_ERR_NONE) {
        Serial.printf("TX state fallito: %d\n", st);
        return false;
    }
    Serial.println("TX state ok");
    // Aggiorna RTC session cache dopo il TX
    cacheSessionInRTC();
    return true;
}

bool sendPayload(const uint8_t* buf, size_t len) {
    Serial.printf("TX %u byte: ", (unsigned)len);
    printHex(buf, len);
    Serial.println();

    // In RadioLib 7.x, sendReceive() riceve DIRETTAMENTE il buffer di downlink
    // come parametro (buffer + puntatore a size). Il fPort del downlink e'
    // ottenuto tramite LoRaWANEvent_t opzionale.
#if ENABLE_DOWNLINK_HANDLER
    uint8_t          dlBuf[64];
    size_t           dlLen = sizeof(dlBuf);
    LoRaWANEvent_t   dlEvent;
    int16_t state = node.sendReceive((uint8_t*)buf, len, LORAWAN_FPORT,
                                     dlBuf, &dlLen, false,
                                     NULL, &dlEvent);
#else
    int16_t state = node.sendReceive((uint8_t*)buf, len, LORAWAN_FPORT);
#endif

    if (state < RADIOLIB_ERR_NONE) {
		Serial.printf("sendReceive failed: %d\n", state);

		switch (state) {
			case RADIOLIB_ERR_NETWORK_NOT_JOINED:
				Serial.println("  (LoRaWAN: NETWORK NOT JOINED)");
				break;

			case RADIOLIB_ERR_INVALID_BANDWIDTH:
				Serial.println("  (bandwidth non valida)");
				break;

			case RADIOLIB_ERR_INVALID_SPREADING_FACTOR:
				Serial.println("  (spreading factor non valido)");
				break;

			case RADIOLIB_ERR_INVALID_CODING_RATE:
				Serial.println("  (coding rate non valido)");
				break;

			case RADIOLIB_ERR_TX_TIMEOUT:
				Serial.println("  (TX timeout)");
				break;

			case RADIOLIB_ERR_RX_TIMEOUT:
				Serial.println("  (RX timeout)");
				break;

			default:
				Serial.println("  (errore RadioLib non gestito)");
				break;
		}

		return false;
	}
    // state == 0 -> nessun downlink; state 1/2 -> downlink in RX1/RX2
    if (state > 0) {
        Serial.printf("TX ok + downlink in RX%d\n", state);

#if ENABLE_DOWNLINK_HANDLER
        // Il downlink e' gia' in dlBuf/dlLen, il fPort e' in dlEvent.fPort
        if (dlLen > 0) {
            handleDownlink(dlEvent.fPort, dlBuf, dlLen);
        } else {
            Serial.println("[DOWNLINK] ricevuto ma payload vuoto (solo MAC commands)");
        }
#endif

    } else {
        Serial.println("TX ok, nessun downlink");
    }

    // Reset del watchdog: siamo appena arrivati fuori dalla fase piu' lunga
    // del ciclo (GPS + SCD41 + TX + finestre RX). Confermiamo al wdt che
    // il firmware e' vivo e sta procedendo correttamente
#if ENABLE_WATCHDOG
    esp_task_wdt_reset();
#endif

    // === Aggiornamento sessione post-TX ===
    // Leggo il FCnt attuale dallo stack (è già stato incrementato)
    uint32_t currentFCnt = node.getFCntUp();
    Serial.printf("FCnt uplink corrente: %lu\n", currentFCnt);

    // Copia sempre la sessione aggiornata in RTC memory (veloce, nessuna usura)
    cacheSessionInRTC();

    // Aggiorna anche RTC vars legacy (utili per altre logiche)
    lastFCntUp = currentFCnt;
    hasWarmData = true;

    // --- Persistenza sessione ---
    // In ENTRAMBE le modalita' la sessione (col FCnt) sta solo in RTC memory
    // (cacheSessionInRTC qui sopra) e NON viene scritta in flash: zero usura.
#if USE_OTAA
    // OTAA: la RTC conserva la sessione tra i deep sleep -> il FCnt incrementa.
    // A un power-off la RTC si perde e il device rifa' un join (FCnt da 0,
    // legittimo), sicuro grazie ai nonces persistiti in NVS.
#else
    // ABP: NON salviamo la sessione in NVS. Il ripristino ABP non e' supportato
    // da RadioLib (darebbe -1120), quindi persistere la sessione sarebbe
    // inutile: il FCnt riparte comunque da 0 ad ogni ciclo. La ripartenza e'
    // gestita LATO SERVER con la validazione frame-counter disabilitata (vedi
    // nota nella sezione USE_OTAA in cima al file).
#endif
    return true;
}

// =============================================================
// DEEP SLEEP
// =============================================================

void enterDeepSleep(uint32_t seconds) {
    // Prima del sleep (o del restart in debug), rimuoviamo il task corrente
    // dal watchdog: durante il sleep il task e' fermo, se non lo rimuovessimo
    // il wdt scatterebbe. Al prossimo boot verra' riarmato dal setup().
#if ENABLE_WATCHDOG
    esp_task_wdt_delete(NULL);
#endif

    // Accumula uptime "wall-clock" (semantica B): tempo attivo del ciclo
    // corrente + tempo che passeremo in sleep. Serve al payload state 0x43.
    // Si azzera solo con power-off/reset HW (RTC persa).
    uptimeAccumulatedS += (millis() / 1000) + seconds;

#if DEBUG_NO_DEEP_SLEEP
    // === MODALITA' DEBUG ===
    // Qui usiamo ESP.restart() invece del vero deep sleep, cosi' l'USB CDC non
    // si stacca e i log restano visibili. ESP.restart() e' un reset software
    // che AZZERA la RTC memory: quindi la sessione (tenuta solo in RTC) va
    // persa ad ogni ciclo e il device rifa' un join OTAA ad ogni restart.
    // E' sicuro perche' i nonces sono persistiti in NVS (niente DevNonce
    // riusato). In debug quindi vedrai un join ad ogni ciclo: e' atteso.
    // In produzione (DEBUG_NO_DEEP_SLEEP=0) si usa il vero deep sleep, la RTC
    // sopravvive e il join avviene una sola volta.

    Serial.printf("[DEBUG] Attesa %u s (no deep sleep, USB CDC viva)\n", seconds);
    Serial.flush();

    // Spegni le periferiche comunque, per non consumare
    gpsPowerOff();
    vextOff();
    ledOff();

    delay(seconds * 1000UL);

    Serial.println("[DEBUG] Restart via ESP.restart()...");
    Serial.flush();
    delay(100);
    ESP.restart();
    // ESP.restart() non ritorna
#else
    // === MODALITA' PRODUZIONE (vero deep sleep) ===
    // La RTC memory PERSISTE tra un deep sleep e l'altro, quindi la sessione
    // copiata in RTC dopo ogni TX (cacheSessionInRTC in sendPayload) basta a
    // mantenere il FCnt aggiornato al risveglio: il device NON rifa' il join,
    // riprende la sessione e incrementa il contatore. La flash non viene
    // toccata in questo percorso (nessuna scrittura di sessione), coerente con
    // la strategia "sessione solo in RTC" descritta in cima al file.
    //
    Serial.printf("Deep sleep per %u s\n", seconds);
    Serial.flush();

    // Spegni tutto
    gpsPowerOff();
    vextOff();
    ledOff();

    // ---- HOLD GPIO durante il deep sleep ----
    // In deep sleep il dominio "digital" dell'ESP32-S3 viene spento e i pin GPIO
    // ordinari perdono il loro stato: tornano flottanti (o a default pull-up/down)
    // finche' non si risvegliano. Questo e' un problema per i pin che comandano
    // MOSFET di alimentazione: se PIN_VEXT_CTRL torna flottante, il MOSFET P
    // potrebbe accendersi da solo e Vext restare alimentata durante il sleep,
    // consumando corrente e vanificando il risparmio.
    //
    // gpio_hold_en() abilita il "digital hold": il livello logico corrente
    // (che qui abbiamo appena messo a HIGH per spegnere) viene BLOCCATO dal
    // dominio RTC che resta alimentato, e il pin lo mantiene per tutta la
    // durata del sleep, riprendendolo intatto al wake.
    //
    // gpio_deep_sleep_hold_en() e' l'interruttore globale che abilita gli hold
    // ad avere effetto in deep sleep (senza questa chiamata gli hold sono
    // configurati ma inattivi).
    gpio_hold_en((gpio_num_t)PIN_VEXT_CTRL);
    gpio_hold_en((gpio_num_t)PIN_VGNSS_CTRL);
    gpio_deep_sleep_hold_en();

    // Wake su timer RTC
    esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
    esp_deep_sleep_start();
#endif
}

// =============================================================
// SETUP + LOOP
// =============================================================

void setup() {
    Serial.begin(115200);
#if ENABLE_TRIPLE_RST_RESET
    checkTripleResetToDefaults();
#endif

    delay(2000);   // tempo per USB CDC di stabilizzarsi e Monitor Seriale di connettersi

    // ---- FEEDBACK "PRONTO PER BOOT" ----
    // 3 blink veloci per segnalare all'utente che il device e' pronto
    // e sta per fare il check LONG_PRESS. Dopo questi 3 blink hai
    // 3 secondi per tenere premuto BOOT per attivare storage mode.
    gpio_deep_sleep_hold_dis();
    gpio_hold_dis((gpio_num_t)PIN_LED);
    pinMode(PIN_LED, OUTPUT);
    for (int i = 0; i < 3; i++) {
        digitalWrite(PIN_LED, HIGH); delay(80);
        digitalWrite(PIN_LED, LOW);  delay(80);
    }

    // ---- LONG PRESS BOOT: check se BOOT premuto ora ----
    // Se BOOT non e' premuto la funzione esce subito senza costo.
    // Se BOOT premuto: LED fisso + polling 3s, poi entra in storage.
    // NOTA: c'e' una finestra di 3s dai blink per iniziare a premere BOOT.
    // Se lo tieni premuto durante i blink stessi, viene rilevato subito.
#if ENABLE_LONG_PRESS_STORAGE
    checkLongPressBootAtSetup();
#endif

    // Disabilita WiFi e Bluetooth: non li usiamo e consumano corrente + generano rumore
    // (importante specialmente per la ricezione LoRa e per l'alimentazione a batteria)
    WiFi.mode(WIFI_OFF);
    btStop();
    esp_bt_controller_disable();

    // Watchdog hardware: se il ciclo si blocca per piu' di WDT_TIMEOUT_S,
    // il chip fa reset automatico. Verra' resettato nei punti chiave.
    //
    // NOTA: in arduino-esp32 3.x il Task WDT e' gia' auto-inizializzato con
    // timeout 5s. Se chiamiamo solo esp_task_wdt_init() la nostra config
    // viene ignorata perche' il wdt "esiste gia'". Serve prima un deinit
    // per rimuovere quella di default, poi init con la nostra.
#if ENABLE_WATCHDOG
    esp_task_wdt_deinit();   // rimuovi eventuale wdt gia' inizializzato
    esp_task_wdt_config_t wdtCfg = {
        .timeout_ms = WDT_TIMEOUT_S * 1000,
        .idle_core_mask = (1 << 0),   // monitora idle task di CPU0
        .trigger_panic = true         // panic (reset) al timeout
    };
    esp_task_wdt_init(&wdtCfg);
    esp_task_wdt_add(NULL);           // aggiungi il task corrente (loopTask) al wdt
    esp_task_wdt_reset();             // reset iniziale per partire "pulito"
    Serial.printf("Watchdog armato, timeout %us\n", WDT_TIMEOUT_S);
#endif

    bootCount++;

    // Al primo boot dopo un power cycle (bootCount == 1), richiediamo l'invio
    // del payload state per far sapere alla piattaforma che il device e'
    // ripartito e comunicare la sua configurazione corrente.
    if (bootCount == 1) {
        sendStateNext = true;
    }

    // ---- TRIPLE RST SAFETY NET ----
    // DEVE essere chiamata PRIMA di loadRuntimeConfig(), perche' se rileva
    // la sequenza di 3 reset consecutivi, cancella le config da NVS e la
    // successiva loadRuntimeConfig() carichera' i default di fabbrica.
//#if ENABLE_TRIPLE_RST_RESET
//   checkTripleResetToDefaults();
//#endif

    // Carica config runtime da NVS (o default se assenti)
    loadRuntimeConfig();

    // ============================================================
    // GESTIONE INTERAZIONE AL BOOT (moduli SERIAL_MENU + STORAGE)
    // ============================================================
    // Priorita' dei meccanismi di interazione dopo reset:
    //   1. SERIAL_MENU (se USB collegato dopo reset HW / power-on):
    //      apre un menu interattivo per comandi di manutenzione
    //   2. STORAGE VBAT-DETECT (fallback silenzioso del menu):
    //      se il menu va in timeout ma eravamo in storage con USB,
    //      esce automaticamente da storage (comportamento "legacy")
    // Entrambi i meccanismi sono no-op se USB non e' collegato o
    // se il reset e' un normale wake da deep sleep.
    // Il TRIPLE_RST_RESET (safety net offline) e' gia' stato gestito
    // sopra, prima di loadRuntimeConfig().

    uint16_t vbat_boot = readBatteryMv();
    bool doForceTxAfterMenu = false;

    // ---- DEBUG: log dettagliato per diagnosticare menu seriale ----
    // Utile per capire se il menu non si apre per VBAT bassa o per
    // reset reason non riconosciuto
    Serial.printf("[BOOT-DEBUG] VBAT=%u mV (soglia USB=%u mV)\n",
                  vbat_boot, VBAT_USB_DETECT_MV);
    Serial.printf("[BOOT-DEBUG] reset_reason=%d (POWERON=1, EXT=2, DEEPSLEEP=8)\n",
                  (int)esp_reset_reason());

#if ENABLE_SERIAL_MENU
    SerialMenuResult smr = checkAndRunSerialMenu(vbat_boot);
    if (smr == SMR_REBOOT) {
        // Clear NVS eseguito, riavvio immediato per applicare
        delay(200);
        ESP.restart();
    }
    doForceTxAfterMenu = (smr == SMR_FORCE_TX_NOW);
#endif

#if ENABLE_STORAGE_MODE
    // Fallback silenzioso: se il menu seriale non ha agito (o non si e'
    // aperto) E il device e' ancora in storage E VBAT indica USB,
    // eseguiamo il classico wake-from-storage (comportamento legacy).
    //
    // IMPORTANTE: skippiamo il fallback se l'utente ha modificato
    // esplicitamente il preset via menu (comando W o I). In quel caso
    // la sua scelta e' definitiva, non va sovrascritta.
    if (isStorageMode() && !menuPresetChangedExplicitly) {
        Serial.printf("STORAGE MODE ancora attivo dopo menu (nessuna azione utente), VBAT=%u mV\n",
                      vbat_boot);
        checkUsbWakeupAndReset(vbat_boot);
    } else if (isStorageMode() && menuPresetChangedExplicitly) {
        Serial.println("STORAGE MODE confermato dall'utente via menu, nessun fallback VBAT");
    }
#endif

    // Se l'utente ha scelto "T" (force TX) dal menu, saltiamo tutta
    // l'attesa GPS del ciclo normale e forziamo un TX quasi immediato
    if (doForceTxAfterMenu) {
        forceTxNow = true;
    }
    // ============================================================

    Serial.println();
    Serial.println("===================================================");
    Serial.printf(" Heltec V4 - SCD41 + L76K - LoRaWAN %s\n",
                  USE_OTAA ? "OTAA" : "ABP");
    Serial.printf(" schema=0x%02X  boot#%u  TX=%us  SF%u  TxPow=%udBm  ADR=%s\n",
                  SCHEMA_ID, bootCount,
                  TX_INTERVAL_SECONDS[cfgTxIntervalPreset],
                  cfgLoRaWANSF, cfgTxPower,
#if USE_OTAA
                  cfgAdr ? "ON" : "OFF"
#else
                  "OFF(ABP)"
#endif
                  );
    if (forceTxNow) {
        Serial.println(" [FLAG] forceTxNow attivo (da downlink precedente)");
    }
    if (identifyMode) {
        Serial.println(" [FLAG] IDENTIFY attivo (blink continuo, no deep sleep)");
    }

    // Stampa DevEUI derivato dal MAC address (formato standard LoRaWAN)
    uint8_t devEui[8];
    getDevEuiFromMac(devEui);
    Serial.printf(" DevEUI: %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X\n",
                  devEui[0], devEui[1], devEui[2], devEui[3],
                  devEui[4], devEui[5], devEui[6], devEui[7]);
#if !USE_OTAA
    Serial.printf(" DevAddr (configurato): %08X\n", devAddr);
#endif
    Serial.println("===================================================");

    // ---- RILASCIO HOLD GPIO al risveglio ----
    // Se il boot arriva da wake-up di deep sleep, i pin PIN_VEXT_CTRL e
    // PIN_VGNSS_CTRL sono ancora "bloccati" dal dominio RTC al livello impostato
    // prima del sleep (HIGH = spento). Prima di poterli manovrare di nuovo
    // (accendere Vext, accendere il GPS) bisogna rilasciare l'hold, altrimenti
    // le digitalWrite() non hanno effetto e i pin restano fissi.
    //
    // Se invece il boot e' a freddo (power-on, reset HW), le chiamate sono
    // innocue: rilasciano hold che non erano attivi.
    gpio_hold_dis((gpio_num_t)PIN_VEXT_CTRL);
    gpio_hold_dis((gpio_num_t)PIN_VGNSS_CTRL);
    gpio_deep_sleep_hold_dis();

    pinMode(PIN_LED, OUTPUT);
    ledOn();

    uint32_t cycleStart = millis();

    // --- 1) Alimenta periferiche ---
    vextOn();

    // In modalita' IDENTIFY il GPS si salta (ispezione al chiuso, fix inutile
    // e allungherebbe il ciclo). Il blink continuo e la TX periodica sono
    // gestiti dal loop identify piu' avanti, non qui.
    bool skipGps = identifyMode;
    if (!skipGps) {
        gpsPowerOn();
    }
    delay(100);   // stabilizzazione alimentazioni

    // --- Feature: decidi se fare fix GPS questo ciclo ---
    // Combina tre check: identify mode (skip), sensor disable, gps skip
    bool doGpsThisCycle = !skipGps;
#if ENABLE_SENSOR_DISABLE
    if (!cfgGpsEnabled) {
        doGpsThisCycle = false;
        Serial.println("GPS DISABLED: salto init e fix");
    }
#endif
#if ENABLE_GPS_SKIP
    // shouldDoGpsFix() considera cfgGpsEnabled internamente,
    // ma qui lo teniamo separato per il log piu' chiaro sopra.
    if (doGpsThisCycle && !shouldDoGpsFix()) {
        doGpsThisCycle = false;
        // Log gia' stampato dentro shouldDoGpsFix / useLastKnownPosition
    }
#endif

    // --- 2) Init GPS (solo se serve questo ciclo) ---
    if (doGpsThisCycle) {
        initGps();
    }

    // --- 3) Init SCD41 (solo se abilitato) ---
    bool doScdThisCycle = true;
#if ENABLE_SENSOR_DISABLE
    if (!cfgScdEnabled) {
        doScdThisCycle = false;
        Serial.println("SCD41 DISABLED: salto init e lettura");
    }
#endif
    if (doScdThisCycle) {
        if (!initScd41()) {
            Serial.println("SCD41 init fallito, procedo con valori nulli");
        }
    }

    // --- 4) Attendi fix GPS (solo se doGpsThisCycle) ---
    bool gpsOk = false;
    if (doGpsThisCycle) {
        // Il primo dato SCD41 arriva dopo ~5 s, il fix GPS 30-90 s.
        uint32_t gpsTimeout = hasWarmData ? GPS_FIX_TIMEOUT_WARM_S : cfgGpsTimeoutS;
        gpsOk = waitForGpsFix(gpsTimeout);
    } else if (skipGps) {
        Serial.println("[IDENTIFY] GPS saltato");
    }
    // (per GPS DISABLED o GPS SKIP, il log e' gia' stato stampato sopra)

    // --- 5) Leggi SCD41 (solo se abilitato questo ciclo) ---
    uint16_t co2 = 0;
    float tempC = 0, humRH = 0;
    bool scdOk = false;
    if (doScdThisCycle) {
        scdOk = readScd41(co2, tempC, humRH);
        if (scdOk) {
            Serial.printf("SCD41: CO2=%u ppm  T=%.2f °C  RH=%.1f %%\n",
                          co2, tempC, humRH);
        }
    }

    // --- 6) Leggi batteria ---
    uint16_t vbat_mv = readBatteryMv();
    uint8_t  bat_pct = vbatToPercent(vbat_mv);
    Serial.printf("Batteria: %u mV (%u %%)\n", vbat_mv, bat_pct);

    // --- 6b) Protezione batteria da under-discharge ---
    // Se la tensione e' pericolosamente bassa, saltiamo TX e sensori
    // ed entriamo in emergency sleep di lunga durata: la cella LiPo si
    // rovina irreversibilmente sotto ~2.5V, dobbiamo evitare che ci arrivi.
#if ENABLE_BATTERY_PROTECTION
    if (vbat_mv > 0 && vbat_mv < cfgVbatEmergencyMv) {
        // Batteria critica: annulla anche l'eventuale modalita' identify, che
        // tiene il device attivo e lampeggiante -> ultima cosa da fare con la
        // cella quasi scarica.
        if (identifyMode) {
            Serial.println("[IDENTIFY] Batteria critica: modalita' identify forzata OFF");
            identifyMode = false;
        }
        Serial.println();
        Serial.println("############################################");
        Serial.printf(" BATTERIA CRITICA: %u mV < soglia %u mV\n",
                      vbat_mv, cfgVbatEmergencyMv);
        Serial.printf(" Salto TX e sensori, emergency sleep %u ore\n",
                      VBAT_EMERGENCY_SLEEP_S / 3600);
        Serial.println(" Ricarica il device per riprendere l'operativita'");
        Serial.println("############################################");
        Serial.flush();
        // Salta direttamente al deep sleep di emergenza
        enterDeepSleep(VBAT_EMERGENCY_SLEEP_S);
        return;   // ridondante ma esplicito
    }
#endif

    // --- 7) Prepara payload ---
    Payload_v0x42 payload = {};
    payload.schema_id   = SCHEMA_ID;
    payload.fix_quality = gpsOk ? 1 : 0;
    payload.satellites  = gpsOk ? gps.satellites.value() : 0;
    payload.battery_pct = bat_pct;

    if (gpsOk && gps.time.isValid() && gps.date.isValid()) {
        // Costruisci epoch UTC dal timestamp GPS
        struct tm t = {};
        t.tm_year = gps.date.year() - 1900;
        t.tm_mon  = gps.date.month() - 1;
        t.tm_mday = gps.date.day();
        t.tm_hour = gps.time.hour();
        t.tm_min  = gps.time.minute();
        t.tm_sec  = gps.time.second();
        payload.timestamp = (uint64_t)mktime(&t);
    } else {
        payload.timestamp = (uint64_t)(millis() / 1000);
    }

    payload.vbat_mv    = vbat_mv;

    // ---- Popolamento campi SCD41 ----
#if ENABLE_SENSOR_DISABLE
    if (!cfgScdEnabled) {
        // SCD41 disattivato via downlink: valori placeholder invalidi
        useScdDisabledPayload(&payload);
    } else
#endif
    if (scdOk) {
        payload.co2_ppm    = co2;
        payload.temp_c100  = (int16_t)(tempC * 100.0f);
        payload.hum_pct100 = (uint16_t)(humRH * 100.0f);
    } else {
        // Lettura fallita ma sensore abilitato: 0 (legacy)
        payload.co2_ppm    = 0;
        payload.temp_c100  = 0;
        payload.hum_pct100 = 0;
    }

    // ---- Popolamento campi GPS ----
#if ENABLE_SENSOR_DISABLE
    if (!cfgGpsEnabled) {
        // GPS disattivato via downlink: valori placeholder invalidi
        useGpsDisabledPayload(&payload);
    } else
#endif
    if (gpsOk) {
        payload.lat_e7    = (int32_t)(gps.location.lat() * 10000000.0);
        payload.lon_e7    = (int32_t)(gps.location.lng() * 10000000.0);
        payload.alt_m     = (int16_t)gps.altitude.meters();
        payload.hdop_x100 = (uint16_t)(gps.hdop.hdop() * 100.0);
        // Aggiorna cache RTC per il prossimo "warm start" e per GPS SKIP
        lastLat_e7  = payload.lat_e7;
        lastLon_e7  = payload.lon_e7;
        hasWarmData = true;
#if ENABLE_GPS_SKIP
        saveGpsFixToRTC(payload.lat_e7, payload.lon_e7, payload.alt_m);
#endif
    }
#if ENABLE_GPS_SKIP
    else if (!doGpsThisCycle) {
        // Ciclo di GPS SKIP: usa ultima posizione nota
        useLastKnownPosition(&payload);
    }
#endif
    else if (hasWarmData) {
        // Fix fallito ma abbiamo posizione valida in RTC (fallback storico)
        payload.lat_e7    = lastLat_e7;
        payload.lon_e7    = lastLon_e7;
        payload.alt_m     = 0;
        payload.hdop_x100 = 9999;   // marker "posizione stale"
    }

    // --- 8) Init LoRaWAN e trasmetti ---
    bool loraOk = initLoRaWAN();
    if (loraOk) {
        sendPayload((uint8_t*)&payload, sizeof(payload));
        // La prima TX puo' aver ricevuto identify_on (imposta identifyMode)
        // o get_state (imposta sendStateNext).

        // Se richiesto (primo boot, cambio config, comando get_state), invia
        // anche il payload state 0x43 su FPort 2. Consuma un secondo TX slot
        // ma solo occasionalmente (non ad ogni ciclo).
        if (sendStateNext) {
            Serial.println("[STATE] sendStateNext attivo, invio payload state...");
            if (sendStatePayload()) {
                sendStateNext = false;   // consumato solo se TX riuscito
            } else {
                Serial.println("[STATE] TX fallito, ritento al prossimo ciclo");
                // sendStateNext resta true, ritenta al prossimo wake
            }
        }
    }

    // --- 8b) MODALITA' IDENTIFY (option B): sveglio, blink continuo, TX periodica ---
    // Se identifyMode e' attivo (per un identify_on ricevuto ora o in un ciclo
    // precedente), NON si va in deep sleep: si resta svegli lampeggiando in
    // continuo e ritrasmettendo ogni IDENTIFY_TX_INTERVAL_S. Ogni TX apre una
    // finestra RX in cui puo' arrivare identify_off. Esce quando: identify_off
    // ricevuto, timeout scaduto, o batteria critica.
    if (loraOk && identifyMode) {
        Serial.println("[IDENTIFY] Modalita' attiva: resto sveglio, blink continuo");
        gpsPowerOff();   // GPS inutile in identify, spengo per non sprecare
        uint32_t identifyStart = millis();

        while (identifyMode) {
            // Timeout di sicurezza
            if (millis() - identifyStart >= (uint32_t)IDENTIFY_TIMEOUT_S * 1000UL) {
                Serial.println("[IDENTIFY] Timeout raggiunto, disattivo");
                identifyMode = false;
                break;
            }

            // Lampeggio continuo per l'intervallo (pettina il wdt internamente)
            blinkFor((uint32_t)IDENTIFY_TX_INTERVAL_S * 1000UL);

            // Rileggi sensori + batteria per la prossima TX
            uint16_t ico2 = 0; float itmp = 0, ihum = 0;
            bool iscd = readScd41(ico2, itmp, ihum);
            uint16_t ivbat = readBatteryMv();

            // Protezione batteria anche in identify
#if ENABLE_BATTERY_PROTECTION
            if (ivbat > 0 && ivbat < cfgVbatEmergencyMv) {
                Serial.println("[IDENTIFY] Batteria critica: esco e vado in emergency sleep");
                identifyMode = false;
                scd4x.stopPeriodicMeasurement();
                enterDeepSleep(VBAT_EMERGENCY_SLEEP_S);
                return;
            }
#endif

            // Payload identify: niente GPS (fix_quality=0, posizione stale)
            Payload_v0x42 ip = {};
            ip.schema_id   = SCHEMA_ID;
            ip.fix_quality = 0;
            ip.satellites  = 0;
            ip.battery_pct = vbatToPercent(ivbat);
            ip.timestamp   = (uint64_t)(millis() / 1000);
            ip.co2_ppm     = iscd ? ico2 : 0;
            ip.temp_c100   = iscd ? (int16_t)(itmp * 100.0f) : 0;
            ip.hum_pct100  = iscd ? (uint16_t)(ihum * 100.0f) : 0;
            ip.vbat_mv     = ivbat;
            ip.hdop_x100   = 9999;   // marker "posizione non valida"

            // TX: apre la finestra RX in cui puo' arrivare identify_off
            // (handleDownlink azzera identifyMode -> il while esce)
            sendPayload((uint8_t*)&ip, sizeof(ip));
        }
        Serial.println("[IDENTIFY] Uscita dalla modalita', ritorno al ciclo normale");
    }

    // --- 9) Spegni SCD41 (fermando le misure periodiche) ---
    scd4x.stopPeriodicMeasurement();

    // --- 10) Calcola tempo residuo e vai in deep sleep ---
    uint32_t cycleElapsed_s = (millis() - cycleStart) / 1000;
    Serial.printf("Ciclo attivo: %u s\n", cycleElapsed_s);

    uint32_t sleep_s;
    // Se forceTxNow era attivo, resettalo (era gia' stato usato per skippare il sleep normale)
    if (forceTxNow) {
        Serial.println("[forceTxNow] flag consumato, dormo brevemente per prossimo TX");
        forceTxNow = false;
        enterDeepSleep(2);   // dormi 2 secondi e poi trasmetti ancora
        return;
    }

    // ---- STORAGE MODE: sleep infinito senza timer wake ----
    // Se il preset TX corrente e' STORAGE (10), non usiamo il timer di wake:
    // il device si sveglia solo su reset hardware. Fondamentale che questo
    // check sia PRIMA del calcolo di sleep_s: TX_INTERVAL_SECONDS[10] = 0
    // farebbe scattare il fallback sleep_s=1 -> loop di TX continui!
#if ENABLE_STORAGE_MODE
    if (isStorageMode()) {
        enterStorageDeepSleep();
        // Non ritorna mai: sleep infinito
    }
#endif

    uint32_t txIntervalS = TX_INTERVAL_SECONDS[cfgTxIntervalPreset];
    if (cycleElapsed_s >= txIntervalS) {
        sleep_s = 1;   // ciclo più lungo dell'intervallo, dormi pochissimo
    } else {
        sleep_s = txIntervalS - cycleElapsed_s;
    }
    enterDeepSleep(sleep_s);
}

void loop() {
    // mai eseguito: il device è in deep sleep dopo setup()
}
