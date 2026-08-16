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
//   3 = 5min
//   4 = 10min
//   5 = 30min
#define TX_INTERVAL_PRESET  2

// ---- Modalita' DEBUG ----
// Se attivo (1), sostituisce il deep sleep con un delay normale + restart,
// cosi' l'USB CDC non si stacca e vedi i log nel Monitor Seriale.
// Da disattivare (0) prima di mettere il device a batteria.
#define DEBUG_NO_DEEP_SLEEP  0

const uint32_t TX_INTERVAL_SECONDS[] = {
    10, 20, 60, 300, 600, 1800
};
// L'intervallo di trasmissione runtime e' TX_INTERVAL_SECONDS[cfgTxIntervalPreset]
// dove cfgTxIntervalPreset e' letto da NVS (o default TX_INTERVAL_PRESET) al boot.

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
#define NVS_KEY_TX_INTERVAL   "tx_int"     // uint8_t preset 0-5
#define NVS_KEY_LORAWAN_SF    "lora_sf"    // uint8_t 7-12
#define NVS_KEY_TX_POWER      "tx_pow"     // uint8_t dBm 2-14
#define NVS_KEY_GPS_TIMEOUT   "gps_t"      // uint16_t sec
#define NVS_KEY_VBAT_EMERG    "vbat_em"    // uint16_t mV
#define NVS_KEY_VBAT_RECOV    "vbat_rc"    // uint16_t mV
#define NVS_KEY_ADR           "adr"        // uint8_t 0/1

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
//     0x11 SET_TX_INTERVAL  - byte preset 0-5 (10s/20s/1m/5m/10m/30m)
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

// ---- Schema payload ----
// Identificatore univoco del formato dei byte nel payload uplink. E' il
// primo byte di ogni pacchetto: il codec ChirpStack lo usa per decidere
// come decodificare i byte successivi. Consente evoluzioni future del
// payload senza rompere la retrocompatibilita' (basta aggiungere un nuovo
// SCHEMA_ID e gestirlo nel codec).
//   0x41 = SDS011 + BME280 (25 byte) - progetto precedente
//   0x42 = SCD41 + L76K + batteria (32 byte) - misure ambientali, FPort 1
//   0x43 = State del device (23 byte) - config runtime + diagnostica, FPort 2
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
#define PIN_ADC_CTRL   37   // LOW = abilita partitore VBAT

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
// | 10     | 1    | u8     | cfgTxIntervalPreset  | 0-5                       |
// | 11     | 1    | u8     | cfgLoRaWANSF         | 7-12                      |
// | 12     | 1    | u8     | cfgTxPower           | dBm 2-14                  |
// | 13     | 2    | u16    | cfgGpsTimeoutS       | secondi 10-300            |
// | 15     | 2    | u16    | cfgVbatEmergencyMv   | mV                        |
// | 17     | 2    | u16    | cfgVbatRecoveryMv    | mV                        |
// | 19     | 1    | u8     | cfgAdr               | 0/1                       |
// | 20     | 1    | u8     | featureFlags         | bit-packed (vedi sotto)   |
// | 21     | 1    | u8     | fwVersion            | FW_VERSION define         |
// | 22     | 1    | u8     | resetReason          | esp_reset_reason() coded  |
//
// featureFlags bit layout:
//   Bit 0: USE_OTAA                    (1 = OTAA, 0 = ABP)
//   Bit 1: ENABLE_NVS_PERSISTENCE      (1 = NVS attiva)
//   Bit 2: ENABLE_DOWNLINK_HANDLER     (1 = downlink processati)
//   Bit 3: ENABLE_WATCHDOG             (1 = wdt HW armato)
//   Bit 4: ENABLE_BATTERY_PROTECTION   (1 = emergency sleep abilitato)
//   Bit 5: DEBUG_NO_DEEP_SLEEP         (1 = modalita' debug con restart)
//   Bit 6-7: riservati per estensioni future
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
};
#pragma pack(pop)

static_assert(sizeof(Payload_v0x43) == 23, "Payload state deve essere 23 byte");

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
 * ADC_Ctrl deve essere LOW durante la lettura per abilitare il partitore.
 */
uint16_t readBatteryMv() {
    pinMode(PIN_ADC_CTRL, OUTPUT);
    digitalWrite(PIN_ADC_CTRL, LOW);
    delay(10);

    analogReadResolution(12);
    // Media di 16 letture per stabilità
    uint32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += analogReadMilliVolts(PIN_VBAT_READ);
    }
    uint32_t adcMv = sum / 16;

    digitalWrite(PIN_ADC_CTRL, HIGH);   // disabilita partitore per risparmiare

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

    // Prova a leggere da NVS (se una chiave manca, mantiene il default)
    if (prefs.begin(NVS_NAMESPACE, true)) {   // true = read-only
        cfgTxIntervalPreset = prefs.getUChar (NVS_KEY_TX_INTERVAL,  cfgTxIntervalPreset);
        cfgLoRaWANSF        = prefs.getUChar (NVS_KEY_LORAWAN_SF,   cfgLoRaWANSF);
        cfgTxPower          = prefs.getUChar (NVS_KEY_TX_POWER,     cfgTxPower);
        cfgGpsTimeoutS      = prefs.getUShort(NVS_KEY_GPS_TIMEOUT,  cfgGpsTimeoutS);
        cfgVbatEmergencyMv  = prefs.getUShort(NVS_KEY_VBAT_EMERG,   cfgVbatEmergencyMv);
        cfgVbatRecoveryMv   = prefs.getUShort(NVS_KEY_VBAT_RECOV,   cfgVbatRecoveryMv);
        cfgAdr              = prefs.getUChar (NVS_KEY_ADR,          cfgAdr ? 1 : 0) != 0;
        prefs.end();
    }

    // Sanity check (evita valori assurdi da NVS corrotta)
    if (cfgTxIntervalPreset > 5) cfgTxIntervalPreset = TX_INTERVAL_PRESET;
    if (cfgLoRaWANSF < 7 || cfgLoRaWANSF > 12) cfgLoRaWANSF = LORAWAN_SF;
    if (cfgTxPower < 2 || cfgTxPower > 14) cfgTxPower = 14;
    if (cfgGpsTimeoutS < 10 || cfgGpsTimeoutS > 300) cfgGpsTimeoutS = GPS_FIX_TIMEOUT_COLD_S;
    // cfgAdr e' bool, nessun sanity check necessario
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


// =============================================================
// DOWNLINK HANDLER
// =============================================================
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
#define CFG_SET_TX_INTERVAL   0x11   // 1 byte, preset 0-5 -> NVS "tx_int"
#define CFG_SET_LORAWAN_SF    0x12   // 1 byte, SF 7-12 -> NVS "lora_sf"
#define CFG_SET_TX_POWER      0x13   // 1 byte, dBm 2-14 -> NVS "tx_pow"
#define CFG_SET_GPS_TIMEOUT   0x14   // 2 byte LE, secondi 10-300 -> NVS "gps_t"
#define CFG_SET_BATT_THRESH   0x15   // 4 byte (2xuint16 LE), mV -> NVS "vbat_em"/"vbat_rc"
#define CFG_SET_ADR_ENABLED   0x16   // 1 byte, 0/1 -> NVS "adr" (effettivo solo in OTAA)

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
            if (len < 2 || buf[1] > 5) {
                Serial.println("[DOWNLINK] SET_TX_INTERVAL: preset non valido");
                return;
            }
            if (saveConfigUChar(NVS_KEY_TX_INTERVAL, buf[1])) {
                cfgTxIntervalPreset = buf[1];
                Serial.printf("[DOWNLINK] SET_TX_INTERVAL: preset=%u (%us)\n",
                              buf[1], TX_INTERVAL_SECONDS[buf[1]]);
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
    delay(2000);   // tempo per USB CDC di stabilizzarsi e Monitor Seriale di connettersi

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

    // Carica config runtime da NVS (o default se assenti)
    loadRuntimeConfig();

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

    // --- 2) Init GPS (solo se non in identify) ---
    if (!skipGps) {
        initGps();
    }

    // --- 3) Init SCD41 ---
    if (!initScd41()) {
        Serial.println("SCD41 init fallito, procedo con valori nulli");
    }

    // --- 4) Attendi fix GPS (saltato in identify) ---
    bool gpsOk = false;
    if (!skipGps) {
        // Il primo dato SCD41 arriva dopo ~5 s, il fix GPS 30-90 s.
        uint32_t gpsTimeout = hasWarmData ? GPS_FIX_TIMEOUT_WARM_S : cfgGpsTimeoutS;
        gpsOk = waitForGpsFix(gpsTimeout);
    } else {
        Serial.println("[IDENTIFY] GPS saltato");
    }

    // --- 5) Leggi SCD41 ---
    uint16_t co2 = 0;
    float tempC = 0, humRH = 0;
    bool scdOk = readScd41(co2, tempC, humRH);
    if (scdOk) {
        Serial.printf("SCD41: CO2=%u ppm  T=%.2f °C  RH=%.1f %%\n",
                      co2, tempC, humRH);
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

    payload.co2_ppm    = scdOk ? co2 : 0;
    payload.temp_c100  = scdOk ? (int16_t)(tempC * 100.0f) : 0;
    payload.hum_pct100 = scdOk ? (uint16_t)(humRH * 100.0f) : 0;
    payload.vbat_mv    = vbat_mv;

    if (gpsOk) {
        payload.lat_e7    = (int32_t)(gps.location.lat() * 10000000.0);
        payload.lon_e7    = (int32_t)(gps.location.lng() * 10000000.0);
        payload.alt_m     = (int16_t)gps.altitude.meters();
        payload.hdop_x100 = (uint16_t)(gps.hdop.hdop() * 100.0);
        // Aggiorna cache RTC per il prossimo "warm start"
        lastLat_e7 = payload.lat_e7;
        lastLon_e7 = payload.lon_e7;
        hasWarmData = true;
    } else if (hasWarmData) {
        // Usa l'ultima posizione valida come fallback
        payload.lat_e7    = lastLat_e7;
        payload.lon_e7    = lastLon_e7;
        payload.alt_m     = 0;
        payload.hdop_x100 = 9999;   // marker di "posizione stale"
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
