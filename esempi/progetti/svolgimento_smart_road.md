>[Torna a reti di sensori](sensornetworkshort.md#servizi-di-accesso-radio-per-WSN)>[Torna a Dettaglio architettura LoRaWAN](/(lorawanclasses.md) 
>

# Svolgimento Esame di Stato 2023/24 - Sistemi e Reti
## Progetto Smart-Road - Approfondimenti

> Documento di approfondimento allo svolgimento della seconda prova d'esame.
> Riferimento traccia: Indirizzo Informatica e Telecomunicazioni - Articolazione Informatica.

---

## Indice

- [1. Analisi della realtà di riferimento e ipotesi aggiuntive](#1-analisi-della-realtà-di-riferimento-e-ipotesi-aggiuntive)
- [2. Architettura della rete - dettaglio](#2-architettura-della-rete---dettaglio)
- [3. Tecnologie di comunicazione tra nodi](#3-tecnologie-di-comunicazione-tra-nodi)
- [4. Piano di indirizzamento dettagliato](#4-piano-di-indirizzamento-dettagliato)
- [5. Continuità di servizio e sicurezza](#5-continuità-di-servizio-e-sicurezza)
- [6. Quesito 1 - Database prenotazioni ricarica (modello logico)](#6-quesito-1---database-prenotazioni-ricarica-modello-logico)
- [7. Quesito 2 - Protocollo applicativo per l'APP guidatori](#7-quesito-2---protocollo-applicativo-per-lapp-guidatori)

---

## 1. Analisi della realtà di riferimento e ipotesi aggiuntive

Prima di entrare nel dettaglio tecnico è opportuno fissare alcune **ipotesi aggiuntive** che restringono il dominio del problema e rendono coerenti le scelte progettuali successive.

### 1.1 Ipotesi sul dominio

- **Tratti sperimentali**: si ipotizzano 20 tratti autostradali da circa 50 km ciascuno, distribuiti uno per regione, per un totale di circa 1.000 km coperti e **~1.000 smart-gate** (uno ogni km).
- **Smart-gate**: ognuno è una stazione "embedded industrial" con doppia alimentazione (rete elettrica + UPS + pannelli fotovoltaici di backup) installata a bordo strada.
- **Centri di controllo (CdC)**: uno per ogni tratto sperimentale, presso una sala operativa regionale della società autostradale, con presidio H24 da parte di un operatore.
- **Centro Nazionale (CN)**: due data-center geograficamente separati in configurazione **active-active** che ospitano il database nazionale, l'API gateway per l'APP utenti, il sistema di analisi BigData, il sistema di prenotazione ricariche.
- **Stazioni di ricarica**: ipotizziamo ~200 stazioni totali, con 4-8 punti di ricarica ciascuna, collegate al CN tramite il loro CdC regionale.
- **Volume di traffico dati**: ogni smart-gate trasmette ~1-2 KB/s di telemetria (sensori a bassa frequenza) e fino a ~5 Mbps in upload se sono attivi stream video dalle telecamere. In totale per un tratto: 50 smart-gate × 5 Mbps = ~250 Mbps di picco.

### 1.2 Vincoli normativi

- **GDPR**: le targhe sono dati personali; i flussi video sono trattati come dati personali. Devono essere previste cifratura at-rest e in-transit, retention limitata, mascheramento per il dataset di analytics.
- **Codice della strada**: la segnaletica variabile è normata; gli smart-gate devono poter operare in **modalità degraded** anche se isolati dal CdC (segnaletica preimpostata di sicurezza).

---

## 2. Architettura della rete - dettaglio

L'architettura proposta è **gerarchica a 3 livelli**, modello che si presta naturalmente al problema perché replica la struttura fisica della rete autostradale (gate locale → tratto regionale → coordinamento nazionale).

![Architettura gerarchica a tre livelli](./../img/architettura_3_livelli.svg)

### 2.1 Livello smart-gate (edge)

Ogni smart-gate è un nodo **edge computing** con le seguenti caratteristiche:

| Componente | Specifica | Motivazione |
|------------|-----------|-------------|
| SoC | Industrial mini-PC con CPU quad-core ARM/x86, 8 GB RAM, SSD 256 GB | Capacità di eseguire localmente algoritmi di computer vision (riconoscimento targhe) e logica di emergenza |
| OS | Linux real-time hardened | Determinismo per il controllo del maxi-schermo, sicurezza |
| Telecamere | 2-4 telecamere IP PoE+ FullHD, una con ottica per targhe (OCR locale) | Riduzione banda: si trasmette il dato strutturato (targa+timestamp), non sempre il video |
| Sensori | Centralina meteo, sensore di luminosità/visibilità, accelerometri sul fondo stradale, sensori PM10/PM2.5/NOx, fonometro | Acquisizione locale via bus Modbus/RS-485 o LoRa-PHY |
| Maxi-schermo | LED matrix controller via Ethernet | Aggiornamento dinamico via API REST locale |
| Connettività primaria | Fibra ottica dedicata (lungo la canalina del guard-rail) | Banda garantita, latenza bassa, alimentazione PoE++ possibile |
| Connettività di backup | Modem 5G/4G LTE con SIM M2M | Continuità in caso di taglio fibra |

Il fatto di concentrare elaborazione locale (edge) è una scelta motivata da:

1. **Riduzione della banda**: non si trasmette il video raw, ma metadati (targa rilevata, conteggio veicoli, classe del veicolo).
2. **Bassa latenza nelle decisioni di sicurezza**: se un sensore rileva ghiaccio sull'asfalto, lo smart-gate può abbassare autonomamente il limite di velocità sul maxi-schermo in < 100 ms, senza attendere il CdC.
3. **Modalità degraded**: in caso di isolamento, lo smart-gate continua a operare in autonomia con segnaletica conservativa.

### 2.2 Livello Centro di Controllo (CdC)

Ogni CdC è un **mini data-center regionale** che funge da aggregatore per ~50 smart-gate. Comprende:

- **Server applicativo** ridondato (cluster Kubernetes a 3 nodi) per l'orchestrazione dei servizi.
- **Broker MQTT** (es. Mosquitto o EMQX) per la messaggistica publish/subscribe verso gli smart-gate.
- **Database operativo locale** (PostgreSQL in HA con streaming replication) per i dati real-time del tratto.
- **Storage video** (NAS con ~100 TB) per la conservazione temporanea (es. 30 giorni) dei flussi delle telecamere.
- **Workstation operatore** con dashboard di monitoraggio e console di override della segnaletica.
- **Firewall di frontiera** (cluster active/standby) con segmentazione VLAN interna.
- **Router di accesso WAN** verso il CN (doppia uscita: MPLS primario + Internet con VPN IPsec come backup).

### 2.3 Livello nazionale (CN)

Il CN è progettato come **due data-center attivo-attivo** geograficamente separati (es. Roma e Milano), con repliche sincrone del DB tramite link in fibra dedicato. Ospita:

- **Database nazionale** (cluster relazionale con sharding geografico) con tutti i dati storici di segnaletica e telemetria.
- **Data lake** per analytics offline (Hadoop/Spark o equivalente cloud).
- **API Gateway** pubblico per l'APP utenti (esposto in HTTPS dietro WAF e CDN).
- **Servizio di prenotazione ricariche** (microservizio a sé stante con il proprio database).
- **Centro operativo nazionale** con monitoraggio aggregato di tutti i tratti.

---

## 3. Tecnologie di comunicazione tra nodi

Le tecnologie scelte differiscono per ogni "salto" della gerarchia, in funzione di banda richiesta, latenza, affidabilità ed esposizione.

### 3.1 Comunicazione interna allo smart-gate

Lo smart-gate ha al suo interno due famiglie di dispositivi che richiedono trattamenti diversi:

- **Telecamere IP + maxi-schermo**: tecnologia cablata Ethernet/IP, perché producono e consumano traffico ad alta banda (FullHD streaming, comandi di segnaletica con conferma). Le telecamere si collegano via **ONVIF/RTSP** su una piccola LAN PoE+ interna allo smart-gate; il maxi-schermo è raggiungibile via TCP/IP attraverso API standard (es. EN 12966 in UE per la segnaletica variabile).
- **Sensori ambientali distribuiti sul km**: tecnologia **wireless LoRaWAN** — vedi la sezione dedicata [§3.2](#32-rete-di-sensori-wireless-lorawan-del-km) qui sotto, perché è una parte sostanziale del progetto e merita una trattazione a sé.

### 3.2 Rete di sensori wireless LoRaWAN del km

Una delle scelte progettuali più caratterizzanti del progetto è realizzare la sensoristica ambientale come **rete wireless LPWA in tecnologia LoRaWAN** distribuita lungo il km di carreggiata di pertinenza di ogni smart-gate. Questa scelta merita un'argomentazione esplicita perché incide su molti aspetti dell'infrastruttura.

#### 3.2.1 Topologia fisica della rete di sensori

![Topologia LoRaWAN del km autostradale](./../img/topologia_lorawan_km.svg)

#### 3.2.2 Sensori (end-device LoRaWAN)

I sensori sono **end-device LoRaWAN in classe A**, distribuiti lungo il km e ancorati al guard-rail. Caratteristiche:

- **Alimentazione**: batteria al litio (es. AA da 3000 mAh) con piccolo **pannello fotovoltaico ausiliario** (formato 5×5 cm o 10×5 cm, tipo "da calcolatrice") e batteria tampone Li-Ion o supercondensatore. Combinazione che garantisce **autonomia energetica indefinita**: il pannellino ricarica la batteria di giorno, la batteria sostiene il funzionamento di notte e durante le giornate nuvolose.
- **Classe A**: il modem radio dorme la maggior parte del tempo e si sveglia solo per trasmettere un breve uplink, seguito da due brevissime finestre di ricezione di downlink. È la classe a minimo consumo energetico — adeguata perché i sensori producono solo telemetria e non hanno bisogno di ricevere comandi tempestivi.
- **Cadenza di trasmissione**: una misura ogni 30-60 secondi a seconda del sensore, compatibile con le limitazioni di **duty cycle** della banda ISM 868 MHz (1% del tempo nelle sotto-bande tipiche).
- **Formato di payload**: messaggi brevi in formato binario compatto, tipicamente **Cayenne LPP** o codec custom dichiarati sotto forma di struct C. Il payload è cifrato con AES tramite la chiave di sessione **AppSKey** ottenuta in fase di OTAA.
- **Identificazione**: ogni sensore ha un **DevEUI** univoco a 64 bit derivato dal MAC Ethernet via EUI64. Non ha indirizzo IP — l'IP entra in gioco solo dal gateway in poi.
- **Custodia**: enclosure IP67/IP68 robusta, fissata al guard-rail con staffe a bullone, resistente a vibrazioni, pioggia, sale, escursioni termiche -20°C/+60°C.
- **Tipologie**: tipicamente 10-30 sensori per km, suddivisi tra centraline meteo, sensori di qualità dell'aria (PM10, PM2.5, NOx), fonometri, sensori di vibrazione/temperatura del fondo stradale, sensori di luminosità ambientale.

Dal punto di vista del **firmware**, il sensore segue un ciclo semplice: dopo un join OTAA iniziale, a ogni risveglio dal deep sleep legge i valori, li codifica in formato compatto (Cayenne LPP), trasmette un uplink LoRaWAN, ascolta brevemente per eventuali downlink e torna a dormire. È lo stesso schema a fasi del firmware MQTT classico (inizializza → leggi → invia a intervalli → ripeti), ma adattato al risparmio energetico estremo di LoRaWAN: niente connessione persistente, deep sleep tra una trasmissione e l'altra.

> **Dettaglio completo** — schema a fasi, macchina a stati, pseudocodice commentato ed esempio in C++ (Arduino/LMIC), gestione dell'energia e formato Cayenne LPP: vedi il file [`dettaglio_firmware_sensore.md`](./dettaglio_firmware_sensore.md).

#### 3.2.3 Gateway LoRaWAN nel cabinet del maxi-schermo

Il gateway LoRaWAN del km è **ospitato all'interno del cabinet del maxi-schermo a portale**, scelta motivata da quattro ragioni concrete:

1. **Posizione baricentrica e in altezza** (5-7 m da terra): l'antenna del gateway è in line-of-sight con tutti i sensori del km, fuori dagli ostacoli (veicoli, guard-rail). Il bilancio di link migliora di 10-20 dB rispetto a un'installazione a livello strada, il che si traduce in margine per data rate più aggressivi e quindi consumi ancora più bassi sui sensori.
2. **Alimentazione condivisa**: il maxi-schermo dispone già di una linea elettrica robusta (rete elettrica con UPS, oppure — nei tratti remoti — impianto fotovoltaico dimensionato per centinaia di watt). Il gateway LoRaWAN consuma 5-10 W, carico marginale che non richiede infrastruttura elettrica aggiuntiva.
3. **Sicurezza fisica e ambientale**: l'enclosure IP65/66 del display protegge anche il gateway da intemperie e vandalismo; manutenzione condivisa con il display.
4. **Connettività IP condivisa**: lo stesso cavo Ethernet che porta i comandi al maxi-schermo viene riutilizzato come uplink IP per il gateway LoRaWAN verso il network server nel CdC. Un solo cavo dati per più funzioni.

Funzioni del gateway:

- **Packet forwarder**: riceve i pacchetti radio dai sensori e li inoltra al network server via IP. Quando un sensore è ricevibile anche dal gateway dello smart-gate adiacente, lo stesso pacchetto arriva al network server da due percorsi diversi — la **ridondanza è gratuita**, e il network server scarta il duplicato tenendo quello con RSSI/SNR migliore.
- **Bridge LoRa→MQTT**: la componente `lora-gateway-bridge` incapsula il messaggio LoRaWAN in un payload MQTT di servizio (in JSON) pubblicato su un broker locale, che il network server consuma.
- **Coordinatore radio**: applica le politiche di **Adaptive Data Rate (ADR)** decise dal network server, assegnando a ciascun sensore data rate e potenza di trasmissione ottimali. Sensori vicini al gateway → data rate alto, potenza bassa (consumo minimo). Sensori lontani → data rate basso (più resistente al rumore), potenza alta.

#### 3.2.4 Modalità "All-In-One" per tratti remoti

Per i tratti autostradali in zone scarsamente coperte dalla fibra (passi montani, contesti isolati), il gateway LoRaWAN può essere realizzato come **gateway All-In-One con doppia interfaccia**: LoRaWAN verso i sensori, modem **5G/4G** o connettività **satellitare LEO** (es. Starlink Direct-to-Cell) verso il network server. È la stessa configurazione utilizzata in agricoltura di precisione e nel monitoraggio ambientale di aree remote.

#### 3.2.5 Confine LoRaWAN / IP

Punto importante da chiarire (è una sorgente classica di confusione in sede d'esame):

| Tratta | Protocollo | Formato messaggi | Identificazione |
|--------|-----------|------------------|-----------------|
| Sensore ↔ Gateway (radio) | LoRaWAN (PHY/MAC LoRa) | Binario compatto (Cayenne LPP), cifrato AES con AppSKey | DevEUI a 64 bit |
| Gateway ↔ Network Server | IP su fibra/5G, MQTT su TLS | JSON di servizio con payload LoRa incapsulato | Indirizzi IP privati |
| Network Server ↔ Server applicativo | IP, MQTT su TLS | JSON applicativo dopo decodifica Cayenne LPP | Indirizzi IP privati |

Il **gateway** è esattamente il **punto di traduzione** tra il mondo LoRa (senza IP) e il mondo IP/MQTT. Sopra LoRaWAN non c'è IP: il sensore non ha alcun indirizzo IP, ha solo il suo DevEUI.

#### 3.2.6 Topic MQTT della rete LoRaWAN

Al modello di topic MQTT visto nella sezione successiva si aggiungono quelli generati dalla rete LoRaWAN. Estendendo il pattern standard:

```
smartroad/<RR>/<TT>/<NNN>/lora/<DEV-EUI>/up      # uplink dal sensore
smartroad/<RR>/<TT>/<NNN>/lora/<DEV-EUI>/down    # downlink verso il sensore
```

Su `up` il dispositivo è publisher (tramite il gateway) e il network server è subscriber. Il payload è un JSON di servizio contenente, oltre a metadati di rete (RSSI, SNR, gateway che ha ricevuto, timestamp, ecc.), il campo `data` con il payload applicativo in BASE64.

Esempio di messaggio sul topic `up` (sensore meteo del km 142 dell'A1, Lombardia tratto 1, smart-gate 042):

```json
{
  "jver": 1,
  "time": "2024-06-15T10:32:14Z",
  "tmst": 561224395,
  "chan": 6,
  "freq": 868.5,
  "modu": "LORA",
  "datr": "SF9BW125",
  "codr": "4/5",
  "rssi": -87,
  "lsnr": 8.5,
  "fcnt": 142,
  "port": 33,
  "data": "AwIBAAEBZwGsAmhMA2hyBAAA",
  "appeui": "8b-6c-f0-8e-ee-df-1b-b6",
  "deveui": "00-80-00-ff-ff-00-2a-3f",
  "gweui": "00-80-00-00-d0-00-01-ff",
  "name": "LO-01-042-METEO-007"
}
```

Il server applicativo del CdC fa **subscribe** sul pattern `smartroad/+/+/+/lora/+/up`, decodifica il campo `data` con il codec Cayenne LPP, e ri-pubblica il dato decodificato sul topic "alto livello" (`smartroad/<RR>/<TT>/<NNN>/misure/meteo`) per il consumo da parte di dashboard, sistema di archiviazione e bridge verso il Centro Nazionale.

#### 3.2.7 Sicurezza della rete LoRaWAN

Ogni sensore si registra al network server tramite **Over-the-Air Activation (OTAA)**. In fabbrica il sensore viene programmato con:

- **DevEUI**: identificatore univoco del dispositivo (derivato dal MAC via EUI64).
- **AppEUI**: identificatore dell'applicazione di destinazione (è un parametro del network server).
- **AppKey**: chiave segreta pre-condivisa a 128 bit, scambiata su canale sicuro tra dispositivo e join server.

Al primo join, il join server usa la AppKey per generare e distribuire al sensore due chiavi di sessione:

- **AppSKey**: utilizzata per cifrare con AES il payload applicativo. Garantisce la **confidenzialità** dei dati: solo il server applicativo (che possiede la stessa chiave) può decifrare.
- **NwkSKey**: utilizzata come chiave in ingresso all'algoritmo **AES-CMAC** per calcolare il **MIC (Message Integrity Code)** di ogni frame. Il sensore allega il MIC al messaggio; il network server ricalcola localmente il MIC con la stessa chiave e verifica la coincidenza. Se i due MIC coincidono, sono provate **simultaneamente l'integrità del messaggio e l'autenticazione del mittente**.

Questo meccanismo è anche un caso applicativo concreto delle **funzioni hash crittografiche** (quesito 4 della seconda parte): AES-CMAC è una funzione di tipo HMAC che produce un'impronta non falsificabile senza conoscere la chiave.

#### 3.2.8 Architettura distribuita dei network server

Una scelta progettuale importante riguarda **dove collocare fisicamente i network server**. La specifica LoRaWAN canonica prevede una topologia "stella di stelle" in cui i gateway sono packet forwarder stupidi e tutta l'intelligenza sta nel network server. Nel nostro progetto questa scelta classica presenta un problema serio.

**Il problema della latenza decisionale.** Consideriamo un caso concreto: un sensore di vibrazione del fondo stradale rileva un picco anomalo compatibile con una buca improvvisa. La catena standard sarebbe:

1. Sensore → uplink LoRa → gateway del km (decine di ms)
2. Gateway → IP/MQTT → network server al CdC regionale (centinaia di km di distanza, decine di ms su fibra, fino a centinaia in caso di backup 5G)
3. Network server → application server → logica di business → decisione "abbassa il limite a 50 km/h sullo smart-gate del km"
4. Decisione → MQTT → SoC dello smart-gate → maxi-schermo

Il tempo totale è dell'ordine di **centinaia di millisecondi nel caso ottimo, secondi nei casi degradati**. Per la sicurezza autostradale è un problema: una buca a 130 km/h è pericolosa, e ogni mezzo secondo in più di latenza significa 18 metri di strada percorsi a velocità non ridotta da ogni veicolo che transita. Inoltre i pacchetti viaggiano sulla WAN regionale anche solo per essere processati e tornare indietro: traffico distribuito su tutta la rete per una decisione che è puramente locale.

**Tre configurazioni possibili.** L'industria LoRaWAN ha riconosciuto questo problema e l'evoluzione architettonica recente prevede di **avvicinare il network server al gateway** (pattern Edge Network Server, diffuso nel contesto Industrial IoT e mission-critical IoT). Le configurazioni ragionevoli sono tre:

| Configurazione | Collocazione del NS | Sensori per NS | Latenza decisione | Numero di NS da gestire | Adatta per |
|----------------|---------------------|----------------|-------------------|--------------------------|------------|
| **A — NS locale per gateway** | Dentro lo smart-gate, sullo stesso SoC | 10-30 | ms (loopback) | ~1.000 | Decisioni di sicurezza in tempo reale |
| **B — NS ogni N gateway** | In uno smart-gate "capo-gruppo" o in armadio dedicato | 50-150 (5-10 km) | decine di ms | ~100-200 | Decisioni coordinate su gruppi di km |
| **C — NS regionale (classica)** | Al CdC regionale | ~1.500 | centinaia di ms | ~20 | Decisioni non time-critical, archiviazione |

La configurazione C corrisponde al pattern "federazione di reti LoRaWAN" raccomandato dalla specifica, in cui ogni regione ha il proprio network server e tutti convergono al CN a livello applicativo:

![Federazione dei network server LoRaWAN](./../img/federazione_network_server.svg)

**Scelta per il progetto: architettura ibrida a due strati.** La scelta corretta non è "scegliere una configurazione e basta", ma realizzare una **gerarchia funzionale** che usa configurazioni diverse per ruoli diversi:

- **Strato edge — Network server locale per gateway (configurazione A)**: gestisce in autonomia tutte le funzioni con vincoli di latenza:
  * decisioni di sicurezza basate sui sensori (ghiaccio, buche, smottamenti, visibilità ridotta)
  * attivazione automatica della segnaletica di emergenza sul maxi-schermo locale
  * gestione dell'ADR (Adaptive Data Rate) dei sensori vicini
  * **modalità degraded autonoma**: anche se la fibra è tagliata e il 5G è giù, lo smart-gate continua a gestire i propri sensori e a prendere decisioni di sicurezza
- **Strato regionale — Network server al CdC (configurazione C, in parallelo)**: gestisce le funzioni di coordinamento sovra-locale e di archiviazione:
  * aggregazione e archiviazione dei dati di tutti gli smart-gate del tratto
  * analytics di tratto (previsione del traffico, pattern stagionali)
  * coordinamento tra smart-gate distanti (es. propagare l'informazione di una chiusura corsia su tutto il tratto)
  * override manuale da parte dell'operatore di sala

La motivazione forte per la configurazione A allo strato edge è l'argomento dell'**autonomia in modalità degraded**: ogni smart-gate deve poter continuare a operare correttamente anche se completamente isolato dalla rete IP, e questo è possibile solo se ha al suo interno tutta la pipeline LoRaWAN locale (gateway + network server + application server locale per le decisioni di sicurezza).

**La gestione dei 1.000 network server locali.** L'obiezione naturale è: "1.000 network server da gestire sono ingestibili". È vero solo con strumenti vecchi. Con il **fleet management moderno** (Ansible/Salt per la configurazione, container Docker o K3s edge per il deployment, Prometheus per il monitoring, OTA firmware update via canale MQTT sicuro) la gestione di 1.000 dispositivi edge identici è un'operazione standardizzata. Le grandi reti CDN e le flotte di POS gestiscono decine o centinaia di migliaia di nodi edge con questi strumenti — 1.000 è un numero piccolo.

**Separazione dei ruoli del network server.** È un altro punto fondamentale per chiarire l'architettura. La specifica LoRaWAN identifica diversi ruoli funzionali che possono stare insieme su una sola macchina o essere distribuiti:

| Ruolo | Cosa fa | Vincolo di latenza | Collocazione nel progetto |
|-------|---------|---------------------|---------------------------|
| **Packet forwarder** | Inoltra i pacchetti radio al NS | Bassissimo | Sempre nel gateway (smart-gate) |
| **Network Server** | Deduplica, gestione MAC, ADR, sessione | Alto per real-time | Locale allo smart-gate (strato edge) + replica al CdC |
| **Join Server** | Autenticazione OTAA, gestione chiavi | Basso (interviene solo al primo join) | Centralizzato — vedi §3.2.9 |
| **Application Server** | Decodifica payload, logica di business | Variabile per funzione | Funzioni real-time in edge, aggregazione in regionale/nazionale |

Questa separazione fisica è esattamente quella che la specifica LoRaWAN raccomanda e che si trova nelle implementazioni reali (ChirpStack, The Things Stack, Actility ThingPark).

#### 3.2.9 Join Server e ridondanza

Il **Join Server** è il componente che gestisce le funzioni di **autenticazione e autorizzazione** dei sensori in fase di registrazione, e di **gestione delle chiavi di sessione** durante la vita operativa del dispositivo. Le sue responsabilità sono:

- **Join Request Validation**: verifica le richieste di join (OTAA) inviate dai sensori, controllando la firma con la chiave **AppKey** pre-condivisa.
- **Generazione delle chiavi di sessione**: produce le chiavi **AppSKey** (cifratura del payload) e **NwkSKey** (calcolo del MIC), le invia in modo sicuro al sensore e al network server di pertinenza.
- **Gestione del ciclo di vita delle chiavi**: rinnovi periodici, revoche in caso di compromissione del dispositivo, rotazione delle chiavi master.
- **Custodia delle AppKey**: mantiene il database delle chiavi master, che è il segreto più importante dell'intera infrastruttura.

**Vincoli di latenza nulli o quasi.** A differenza del network server, il join server **non ha vincoli di latenza real-time**. Interviene solo:

- una volta al primo provisioning del sensore (join iniziale)
- ai rinnovi periodici della sessione (tipicamente ogni qualche giorno o settimana)
- in caso di rejoin esplicito dopo un reset o un disservizio

Si tratta di un'operazione **rara** rispetto alla telemetria normale: ogni sensore fa migliaia di uplink per ogni singolo join. Quindi il join server può tranquillamente essere **centralizzato a livello nazionale**, senza penalizzare le prestazioni della rete.

**Perché centralizzare il join server è la scelta giusta.** Tre ragioni progettuali forti:

1. **Le AppKey sono il segreto più sensibile dell'infrastruttura**: se un attaccante ottiene le AppKey, può clonare i sensori e iniettare dati falsi nella rete. Distribuirle in 1.000 smart-gate al bordo strada è una pessima idea dal punto di vista della sicurezza fisica. Tenerle in un datacenter al CN, dietro HSM (Hardware Security Module) e controlli di accesso fisici stretti, è enormemente più sicuro.
2. **Provisioning unificato**: quando viene fabbricato un nuovo sensore, le sue credenziali devono essere registrate **una sola volta** in un sistema centrale. Avere un join server centralizzato semplifica drasticamente il workflow di onboarding.
3. **Revoche immediate su tutta la rete**: se un sensore viene rubato o compromesso, la revoca deve propagarsi istantaneamente su tutta la rete nazionale. Con un join server centralizzato basta cancellare la chiave in un solo posto.

**Ridondanza del join server.** Centralizzare però significa creare un single point of failure critico: se il join server è down, **nessun sensore nuovo può aggregarsi alla rete** e i rinnovi di sessione falliscono (i sensori esistenti continuano a funzionare con la sessione corrente, ma alla scadenza decadono). La soluzione standard è una **configurazione in alta disponibilità multi-sito**:

```
                         JOIN SERVER (active-active)
                ┌──────────────────────────────────┐
                │                                  │
       ┌────────┴─────────┐          ┌─────────────┴────┐
       │  Join Server #1  │          │  Join Server #2  │
       │  (DC Milano)     │◄────────►│  (DC Roma)       │
       │                  │  replica │                  │
       │  - HSM           │  sincrona│  - HSM           │
       │  - DB chiavi     │          │  - DB chiavi     │
       └──────────────────┘          └──────────────────┘
                │                                  │
                └──────────────┬───────────────────┘
                               │ load balancer geografico (anycast/GSLB)
                               │
                  ┌────────────┴────────────┐
                  │     Network Servers     │
                  │  (edge negli smart-gate │
                  │   + regionali nei CdC)  │
                  └─────────────────────────┘
```

Caratteristiche dell'alta disponibilità del join server:

- **Due istanze in due data-center geograficamente separati** (es. Milano e Roma), in configurazione **active-active**. Entrambe servono richieste, non c'è un'istanza "in caldo" inutilizzata.
- **Database delle chiavi replicato sincronicamente** tra i due siti, su link in fibra dedicato. La replica sincrona è obbligatoria: una nuova chiave creata su un sito deve essere immediatamente visibile sull'altro, altrimenti un sensore appena registrato potrebbe non poter fare join se la richiesta arriva al sito "indietro".
- **HSM (Hardware Security Module)** in ogni sito per la custodia delle chiavi master: le AppKey non lasciano mai l'HSM in chiaro, le operazioni di crittografia avvengono dentro l'HSM, neanche un amministratore di sistema può estrarle.
- **Bilanciamento via GSLB (Global Server Load Balancing)** o **anycast IP**: i network server raggiungono il join server tramite un endpoint logico unico (es. `join.smart-road.example`) che il GSLB risolve sul sito più vicino e funzionante. In caso di failure di un sito, il GSLB rimuove l'IP dalla risoluzione DNS in pochi secondi e tutto il traffico converge sull'altro sito.
- **Disaster Recovery di terzo livello**: backup giornaliero delle chiavi master (cifrato e replicato su storage offline) in un terzo sito, per il caso catastrofico di perdita simultanea di entrambi i data-center primari (incendio, attacco fisico). Recovery time obiettivo nell'ordine delle ore.

**Una piccola nota progettuale.** Esiste in commercio anche la possibilità di affidare il join server a un provider esterno specializzato (es. Actility, Senet, alcuni operatori telco offrono questo come servizio gestito). Per un progetto di infrastruttura critica nazionale come la rete autostradale è però **preferibile mantenere il controllo interno**: le chiavi master sono un asset strategico del paese e affidarle a un terzo introduce dipendenze contrattuali e geopolitiche che vale la pena evitare.

#### 3.2.10 Riassunto dei vantaggi della scelta

- **Zero scavi lungo il km**: nessun cavo di alimentazione né di dati per i sensori. Costo di posa fortemente abbattuto rispetto a soluzioni cablate.
- **Installazione e manutenzione modulare**: ogni sensore è una scatoletta indipendente fissata al guard-rail in mezz'ora.
- **Energia autonoma a tempo indeterminato**: batteria + microfotovoltaico → nessuna manutenzione energetica per anni.
- **Ridondanza gratuita**: sensori al confine tra due km sono ricevibili da entrambi i gateway adiacenti, il network server deduplica.
- **Decisioni di sicurezza in tempo reale**: network server locale a ogni smart-gate (architettura edge) → latenza decisionale di millisecondi, autonomia in modalità degraded.
- **Custodia centralizzata e sicura delle chiavi master**: join server ridondato active-active con HSM in due data-center separati.
- **Sicurezza forte end-to-end**: cifratura del payload (AppSKey), autenticazione e integrità tramite MIC (NwkSKey), OTAA per il provisioning sicuro delle chiavi di sessione.

### 3.3 Comunicazione smart-gate ↔ CdC

Questa è la tratta più delicata: deve essere ad alta banda (per gli stream video on-demand), bassa latenza, sempre disponibile.

- **Fisicamente**: fibra ottica monomodale lungo l'autostrada, con topologia ad **anello** per la resilienza. La scelta dell'apparato attivo che chiude l'anello a ogni km è discussa in dettaglio nella sezione [§3.3.1](#331-topologia-fisica-e-spillamento-della-fibra-lungo-il-tratto).
- **Logicamente**: link Ethernet/IP. Sopra IP si appoggiano:
  - **MQTT su TLS** per la messaggistica asincrona di telemetria, comandi e stato (publish/subscribe verso il broker del CdC).
  - **HTTPS / REST** per gli aggiornamenti di configurazione e per il push di nuove segnaletiche.
  - **RTSP/SRT** per gli stream video on-demand (solo quando l'operatore richiede la visione live).
- **Backup**: connessione **5G/4G LTE** con APN privato della società autostradale, attivata automaticamente da BGP/SD-WAN in caso di failure della fibra.

#### 3.3.1 Topologia fisica e spillamento della fibra lungo il tratto

Come si "spilla" la fibra lungo le decine di km del tratto per servire ogni smart-gate? Esistono tre approcci: un **anello Ethernet attivo con switch L2** (rigenerazione attiva a ogni km, failover ERPS < 50 ms), un **anello IP con router L3** (più costoso e con failover più lento) e una **PON con splitter ottici passivi** (niente apparato attivo a bordo strada, ma nessuna ridondanza ad anello).

Per il progetto si adotta l'**anello Ethernet L2 con ERPS (IEEE G.8032)**: ogni smart-gate ospita uno switch industriale con 2 porte ottiche (anello) e porte di accesso per gli apparati interni; lo switch è alimentato dalla stessa linea del maxi-schermo. Il failover sotto i 50 ms garantisce continuità per gli stream video e la telemetria anche in caso di taglio della fibra. La fibra è posata in canalina sotto il guard-rail (mini-trenching), con cavi a 24-48 fibre di cui buona parte tenuta di scorta (dark fiber) per espansioni future.

![Anello in fibra ottica con switch ERPS a ogni km](./../img/anello_fibra_erps.svg)

> **Dettaglio completo** — confronto tecnico delle tre tecnologie (rigenerazione attiva vs spillamento passivo), meccanica del protocollo ERPS, specifiche dello switch industriale, tracciato fisico della posa: vedi il file [`dettaglio_spillamento_fibra.md`](./dettaglio_spillamento_fibra.md).

#### 3.3.2 Modello dei topic MQTT per uno smart-gate

Estendendo il modello visto nei materiali di riferimento (riferimento didattico: `sebastianomelita/ArduinoBareMetal`), si può definire una gerarchia di topic come segue. Sia `<RR>` la regione, `<TT>` il tratto, `<NNN>` l'identificatore numerico dello smart-gate (es. `LO/01/042` = Lombardia, tratto 1, smart-gate 42):

```
smartroad/<RR>/<TT>/<NNN>/misure/meteo
smartroad/<RR>/<TT>/<NNN>/misure/traffico
smartroad/<RR>/<TT>/<NNN>/misure/aria
smartroad/<RR>/<TT>/<NNN>/misure/fondo
smartroad/<RR>/<TT>/<NNN>/eventi/targhe
smartroad/<RR>/<TT>/<NNN>/comandi/schermo
smartroad/<RR>/<TT>/<NNN>/stato/schermo
smartroad/<RR>/<TT>/<NNN>/config
```

Ruoli sui topic:

- **`misure/*`**: lo smart-gate è **publisher**, il server applicativo del CdC è **subscriber**. Sono usati per inviare la telemetria periodica (cadenza tipica: meteo ogni 30 s, aria ogni 60 s, fondo ogni 10 s).
- **`eventi/targhe`**: lo smart-gate pubblica un messaggio asincrono ogni volta che riconosce una targa. Il CdC è subscriber e inoltra i dati al CN.
- **`comandi/schermo`**: il server del CdC è **publisher** (o l'operatore tramite la console), lo smart-gate è **subscriber**. Tramite questo canale si invia la segnaletica da visualizzare.
- **`stato/schermo`**: lo smart-gate è **publisher** e conferma in modalità PUSH il contenuto attualmente visualizzato. Serve all'operatore come feedback e al sistema per chiusura del loop comando-conferma.
- **`config`**: tutti i parametri di funzionamento (frequenze di misura, soglie, aggiornamenti FW OTA). Il server è publisher, lo smart-gate è subscriber.

Esempio di payload JSON sul topic `misure/meteo`:

```json
{
  "deviceId": "LO-01-042",
  "timestamp": "2024-06-15T10:32:14Z",
  "meteo": {
    "tempC": 18.5,
    "humPct": 67,
    "rainMmH": 0.0,
    "windKmh": 12,
    "visibilityM": 4500
  },
  "qos": {
    "battery": 100,
    "rssiBackupLink": -67
  }
}
```

Esempio di payload sul topic `comandi/schermo`:

```json
{
  "msgId": "c-9f3a-887",
  "timestamp": "2024-06-15T10:32:20Z",
  "originator": "CdC-LO-01",
  "command": "setSignage",
  "params": {
    "speedLimit": 90,
    "lane1": "open",
    "lane2": "open",
    "lane3": "closed",
    "infoText": "Pioggia intensa, ridurre velocità",
    "ttlSec": 300
  }
}
```

### 3.4 Comunicazione CdC ↔ CN

- **Connessione primaria**: rete **MPLS L3VPN** fornita da un operatore telco. Garantisce SLA contrattuali, classi di servizio (QoS) e isolamento.
- **Connessione di backup**: tunnel **VPN IPsec site-to-site** su Internet pubblica, da firewall a firewall.
- **Protocolli applicativi**:
  - **MQTT bridge** (è il pattern classico): il broker del CdC fa da bridge verso il broker del CN, replicando solo i topic di interesse nazionale (es. dati aggregati di traffico, prenotazioni ricariche, allarmi, eventi targhe).
  - **HTTPS/REST** per le chiamate sincrone (es. recupero di dati storici, push di configurazioni globali dal CN ai CdC).
  - **gRPC** in alternativa al REST quando occorre throughput più alto e contratti tipizzati (Protocol Buffers).

### 3.5 Comunicazione APP utenti ↔ CN

- **HTTPS/REST** (versionato `/v1/...`) per le chiamate stateless del client.
- **WebSocket Secure (WSS)** per il push real-time delle segnaletiche e dello stato dei punti di ricarica.
- In alternativa, **MQTT over WebSocket Secure** se si vuole riusare l'infrastruttura broker (il client APP si registra come subscriber su topic di pubblico interesse).
- Autenticazione utenti con **OAuth 2.0 + OpenID Connect** per le funzioni che richiedono profilazione (prenotazione ricarica). Le funzioni di sola lettura della segnaletica sono accessibili in modo anonimo.

### 3.6 Comunicazione stazioni di ricarica ↔ rete

Le stazioni di ricarica utilizzano lo standard **OCPP (Open Charge Point Protocol) 1.6 o 2.0.1** su WebSocket Secure verso un CSMS (Charging Station Management System) che, nel nostro progetto, è un microservizio del CN. Questo dà accesso a:

- stato in tempo reale di ogni punto (libero, occupato, in errore);
- avvio/interruzione remota di una sessione di ricarica;
- prenotazioni (OCPP supporta nativamente i messaggi `ReserveNow` e `CancelReservation`).

---

## 4. Piano di indirizzamento dettagliato

Si adotta un piano basato su **RFC 1918** all'interno della rete privata della società autostradale e indirizzi pubblici solo per i servizi esposti su Internet (APP, sito istituzionale).

### 4.1 Spazio di indirizzamento privato

Spazio scelto: `10.0.0.0/8`. Strutturato gerarchicamente in modo che dall'IP si possa risalire alla regione e al tratto, semplificando le ACL e il troubleshooting.

```
10.<RR>.<TT>.<NNN>
        │   │    │
        │   │    └── host nel tratto (0-255)
        │   └─────── numero tratto nella regione (0-255)
        └─────────── codice regione (1-20)
```

| Range | Uso |
|-------|-----|
| `10.0.0.0/16` | Riservato al Centro Nazionale e ai servizi corporate |
| `10.0.0.0/24` | Sottorete server del CN (DB, API, gateway) |
| `10.0.1.0/24` | DMZ del CN (frontend esposto, WAF) |
| `10.0.2.0/24` | Management out-of-band del CN |
| `10.0.10.0/24` | Sottorete operatori, monitoring SOC, jump host |
| `10.<RR>.0.0/16` | Una /16 per regione (es. `10.3.0.0/16` per Lombardia) |
| `10.<RR>.<TT>.0/24` | Una /24 per ogni tratto autostradale (50 smart-gate + servizi) |
| `10.<RR>.<TT>.1` | Gateway del tratto (router di accesso del CdC) |
| `10.<RR>.<TT>.2-10` | Server del CdC (broker MQTT, app server, DB, NAS) |
| `10.<RR>.<TT>.11-50` | Workstation operatori, dispositivi management |
| `10.<RR>.<TT>.51-200` | Smart-gate (1 IP ciascuno; ne basterebbero 50 ma riserva per espansioni) |
| `10.<RR>.<TT>.201-250` | Stazioni di ricarica del tratto |

### 4.2 Segmentazione interna di uno smart-gate

Lo smart-gate ha al suo interno una piccola LAN con VLAN dedicate per separare i sottosistemi (segmentazione di sicurezza):

| VLAN | Subnet | Uso |
|------|--------|-----|
| 10 | `192.168.10.0/29` | Telecamere IP (sottorete isolata, non raggiungibile dall'esterno) |
| 20 | `192.168.20.0/29` | Sensori IP-based |
| 30 | `192.168.30.0/29` | Controller maxi-schermo |
| 99 | `192.168.99.0/29` | Management (SSH, SNMP) |

Il SoC centrale fa da gateway tra queste VLAN e l'uplink verso il CdC.

### 4.3 VLAN del CdC

| VLAN | Subnet | Uso |
|------|--------|-----|
| 100 | `10.<RR>.<TT>.0/27` | Server cluster (broker, app, DB) |
| 110 | `10.<RR>.<TT>.32/27` | Storage / NAS |
| 120 | `10.<RR>.<TT>.64/27` | Workstation operatore |
| 130 | `10.<RR>.<TT>.96/27` | Management/IPMI |
| 200 | `10.<RR>.<TT>.128/25` | Subnet smart-gate (transit) |

### 4.4 Routing

- **Routing dinamico interno**: protocollo **OSPF** sulle aree regionali, con area 0 (backbone) tra i CdC e il CN.
- **Routing tra CdC e CN su MPLS**: **BGP** verso il PE dell'operatore telco (BGP per le route private nella VPN MPLS).
- **VPN IPsec di backup**: route statiche oppure BGP-over-IPsec.

### 4.5 NAT e indirizzi pubblici

Solo i servizi rivolti agli utenti dell'APP hanno indirizzi pubblici. Si usa un piccolo blocco IPv4 (es. `203.0.113.0/29`) e/o IPv6 nativo, dietro load balancer di frontiera con WAF (Web Application Firewall).

---

## 5. Continuità di servizio e sicurezza

### 5.1 Continuità di servizio (alta affidabilità)

| Livello | Tecniche adottate |
|---------|-------------------|
| Smart-gate | Doppia alimentazione (rete + UPS + fotovoltaico); doppia connettività (fibra + 5G); **modalità degraded autonoma** in caso di isolamento dal CdC |
| Rete tratto | Topologia ad **anello ottico** con ERPS (failover < 50 ms); switch industriali con doppia alimentazione |
| CdC | Cluster Kubernetes con 3+ nodi, repliche dei pod su nodi diversi; database in HA con streaming replication; UPS + gruppo elettrogeno |
| WAN CdC↔CN | Doppia uscita MPLS + Internet con SD-WAN; BFD per failover rapido |
| CN | **Due data-center attivo-attivo** geograficamente separati; bilanciamento DNS globale (GSLB); replica sincrona del DB sulla coppia primaria di sito, asincrona verso siti DR |
| Dati | Backup giornalieri + replica geografica; piano di Disaster Recovery con RTO < 4 h e RPO < 15 min |
| Servizi APP | CDN davanti all'API gateway; rate limiting per resistere a spike di traffico |

### 5.2 Sicurezza

La sicurezza è organizzata per **strati** (defense in depth):

**Sicurezza fisica**: gli smart-gate sono in armadi blindati con sensori di apertura (tamper switch) che generano allarmi.

**Sicurezza di rete**:

- Tutto il traffico inter-sito viaggia in **MPLS VPN** o **IPsec**.
- Firewall stateful al perimetro di ogni CdC e del CN; IDS/IPS con regole signature-based + analisi comportamentale.
- Segmentazione **micro** tramite VLAN e ACL: gli smart-gate non possono parlare tra loro, ma solo col CdC.
- **NAC (Network Access Control)** con 802.1X sugli switch del CdC per autenticare ogni dispositivo che si collega.

**Sicurezza applicativa**:

- Tutto in **TLS 1.3** (HTTPS, MQTTS, WSS).
- Autenticazione dei dispositivi smart-gate verso il broker MQTT con **certificati X.509** (mutual TLS). Ogni smart-gate ha il proprio certificato emesso da una PKI interna; il certificato viene installato in fabbrica e ruotato annualmente.
- Autenticazione degli utenti dell'APP con **OAuth 2.0 + OIDC** e MFA per le funzioni che muovono denaro (prenotazione e pagamento ricarica).
- Le password sugli account utente sono salvate con **bcrypt/argon2** (mai in chiaro, mai con hash veloci come MD5/SHA1).

**Crittografia**:

- **At-rest**: dischi cifrati (LUKS/BitLocker), DB con TDE (Transparent Data Encryption). Le targhe vengono pseudonimizzate prima di entrare nel data lake.
- **In-transit**: TLS 1.3 ovunque, con perfect forward secrecy (ECDHE).
- **Integrità messaggi**: dove serve, HMAC-SHA256 sui payload critici (vedi quesito 4 sugli hash crittografici).

**Sicurezza operativa**:

- **SIEM** centrale al CN che aggrega log da tutti i firewall, server, switch, broker.
- **SOC** che monitora gli alert H24.
- Aggiornamenti firmware sugli smart-gate via OTA (Over-The-Air), con firmware firmato e verificato dallo smart-gate prima dell'installazione.
- Politica di **least privilege** e **separation of duties** sugli account amministrativi; accessi tramite jump host con MFA.

---

## 6. Quesito 1 - Database prenotazioni ricarica (modello logico)

Si modella la porzione del database nazionale dedicata alle **stazioni di ricarica e alle prenotazioni** per veicoli elettrici. Le altre porzioni (storico segnaletiche, telemetria, utenti) sono lasciate fuori da questo schema per coerenza con il quesito.

### 6.1 Analisi dei requisiti

Dal testo della traccia:

- Le stazioni di ricarica sono **dislocate sulla rete autostradale**.
- Ogni stazione ha **più punti di ricarica**.
- Si deve gestire lo **stato in tempo reale** dei punti (libero, occupato, guasto, in manutenzione).
- Si devono gestire **prenotazioni** sulla base dell'**orario di arrivo** stimato e della **durata** stimata.
- Le prenotazioni sono fatte da utenti dell'APP.

### 6.2 Modello concettuale (ER)

Entità individuate:

- **UTENTE** (chi prenota): id, nome, cognome, email, password_hash, telefono.
- **VEICOLO** (auto elettrica dell'utente): targa, modello, capacità_batteria_kWh, tipo_connettore, potenza_max_kW. Un utente può avere più veicoli registrati.
- **STAZIONE_RICARICA** (sito fisico in autostrada): id, nome, latitudine, longitudine, autostrada, km, direzione, id_tratto_autostradale.
- **PUNTO_RICARICA** (singola colonnina/connettore): id, id_stazione, tipo_connettore (CCS, CHAdeMO, Type2, ecc.), potenza_kW, stato (libero/occupato/guasto/manutenzione).
- **TARIFFA**: id, nome, prezzo_kWh, prezzo_min_occupazione.
- **PRENOTAZIONE**: id, id_utente, id_veicolo, id_punto, ora_arrivo_prevista, durata_prevista_min, stato (attiva/in_corso/completata/annullata/no_show), data_creazione, id_tariffa.
- **SESSIONE_RICARICA** (la ricarica vera, registrata dopo che è avvenuta): id, id_prenotazione, ora_inizio_effettiva, ora_fine_effettiva, energia_kWh, costo_totale.

Relazioni principali:

- UTENTE 1—N VEICOLO (un utente ha più veicoli).
- STAZIONE_RICARICA 1—N PUNTO_RICARICA (una stazione ha più punti).
- UTENTE 1—N PRENOTAZIONE.
- VEICOLO 1—N PRENOTAZIONE.
- PUNTO_RICARICA 1—N PRENOTAZIONE.
- TARIFFA 1—N PRENOTAZIONE.
- PRENOTAZIONE 1—1 SESSIONE_RICARICA (opzionale: alcune prenotazioni non sfociano in una sessione, es. no-show).

### 6.3 Schema logico relazionale

Notazione: chiave primaria sottolineata `[PK]`, chiave esterna `[FK→tabella.campo]`.

**UTENTE**(<u>id_utente</u> [PK], email [UNIQUE NOT NULL], password_hash, nome, cognome, telefono, data_registrazione)

**VEICOLO**(<u>targa</u> [PK], id_utente [FK→UTENTE.id_utente], modello, capacita_batteria_kwh, tipo_connettore, potenza_max_kw)

**STAZIONE_RICARICA**(<u>id_stazione</u> [PK], nome, latitudine, longitudine, autostrada, km, direzione, indirizzo)

**PUNTO_RICARICA**(<u>id_punto</u> [PK], id_stazione [FK→STAZIONE_RICARICA.id_stazione], tipo_connettore, potenza_kw, stato_corrente, ultimo_aggiornamento_stato)

**TARIFFA**(<u>id_tariffa</u> [PK], nome, prezzo_kwh, prezzo_min_occupazione, data_validita_inizio, data_validita_fine)

**PRENOTAZIONE**(<u>id_prenotazione</u> [PK], id_utente [FK→UTENTE.id_utente], targa [FK→VEICOLO.targa], id_punto [FK→PUNTO_RICARICA.id_punto], id_tariffa [FK→TARIFFA.id_tariffa], ora_arrivo_prevista, durata_prevista_min, stato, data_creazione)

**SESSIONE_RICARICA**(<u>id_sessione</u> [PK], id_prenotazione [FK→PRENOTAZIONE.id_prenotazione UNIQUE], ora_inizio_effettiva, ora_fine_effettiva, energia_kwh, costo_totale)

### 6.4 Diagramma logico (schematico)

```
UTENTE ──< VEICOLO
   │           │
   │           │
   └───────────┴──< PRENOTAZIONE >── PUNTO_RICARICA ──> STAZIONE_RICARICA
                         │
                         ├──> TARIFFA
                         │
                         └──1:1── SESSIONE_RICARICA
```

### 6.5 Vincoli di integrità rilevanti

- Una **prenotazione** non può sovrapporsi a un'altra sulla stessa coppia (id_punto, intervallo_temporale). Vincolo applicativo o tramite **exclusion constraint** (in PostgreSQL `EXCLUDE USING gist`).
- Il tipo di connettore della prenotazione deve essere compatibile col veicolo: vincolo da implementare a livello applicativo o con trigger.
- `stato` di PRENOTAZIONE ∈ {attiva, in_corso, completata, annullata, no_show}.
- `stato_corrente` di PUNTO_RICARICA ∈ {libero, occupato, guasto, manutenzione, prenotato}.

### 6.6 Query di esempio

Punti liberi compatibili con il veicolo dell'utente, in una stazione, in una finestra oraria:

```sql
SELECT p.id_punto, p.tipo_connettore, p.potenza_kw
FROM PUNTO_RICARICA p
WHERE p.id_stazione = :id_stazione_richiesta
  AND p.tipo_connettore = :tipo_connettore_veicolo
  AND p.stato_corrente IN ('libero', 'prenotato')
  AND NOT EXISTS (
    SELECT 1 FROM PRENOTAZIONE pr
    WHERE pr.id_punto = p.id_punto
      AND pr.stato IN ('attiva', 'in_corso')
      AND tsrange(
            pr.ora_arrivo_prevista,
            pr.ora_arrivo_prevista + (pr.durata_prevista_min * INTERVAL '1 minute')
          ) && tsrange(:arrivo_richiesto, :arrivo_richiesto + (:durata_richiesta * INTERVAL '1 minute'))
  );
```

### 6.7 Considerazioni di scalabilità

- Le tabelle ad alta frequenza di scrittura (PUNTO_RICARICA.stato_corrente) andrebbero affiancate da una cache in-memory (Redis) per servire le query di stato all'APP con latenza < 100 ms.
- Le sessioni di ricarica chiuse possono essere migrate su una tabella storica partizionata per anno/mese per non appesantire la tabella attiva.

---

## 7. Quesito 2 - Protocollo applicativo per l'APP guidatori

### 7.1 Scelta della tecnologia

L'APP guidatori deve:

1. **Consultare in tempo reale la segnaletica e le informazioni di ogni smart-gate** lungo il proprio percorso.
2. Visualizzare lo **stato dei punti di ricarica** lungo il percorso.
3. **Prenotare** un punto di ricarica.
4. Ricevere notifiche **push** in caso di eventi rilevanti (chiusura corsia, incidente avanti, congestione).

Le esigenze sono miste: alcune sono richiesta-risposta sincrone (prenotare, leggere lo storico), altre sono push asincrono (notifica di un nuovo evento sul tratto in cui l'utente sta viaggiando).

La scelta proposta è **ibrida**:

- **REST su HTTPS** (HTTP/2 o HTTP/3) per le operazioni request/response (CRUD prenotazioni, ricerca stazioni, login). È lo standard de-facto per le API mobile, ben supportato dai framework client.
- **MQTT su WebSocket Secure (MQTTS-over-WSS)** per il push asincrono. L'APP si registra come **subscriber** sui topic relativi al tratto autostradale che sta percorrendo, ricevendo in real-time gli aggiornamenti di segnaletica.

Questa combinazione è elegante perché **riusa la stessa infrastruttura di brokering MQTT** già presente per la comunicazione interna smart-gate → CdC → CN: il broker centrale espone su Internet (dietro TLS e autenticazione) un sottoinsieme dei topic, e l'APP è semplicemente un altro client del broker.

### 7.2 Architettura della comunicazione APP↔CN

```
   ┌──────────┐   HTTPS REST    ┌─────────────┐
   │          ├────────────────►│ API Gateway │──► Microservizi (utenti,
   │          │                 │             │     prenotazioni, ...)
   │   APP    │                 └─────────────┘
   │ guidatore│
   │          │   MQTTS / WSS   ┌─────────────┐
   │          ├────────────────►│ Broker MQTT │──► Bridge ai broker dei CdC
   │          │                 │  pubblico   │
   └──────────┘                 └─────────────┘
```

### 7.3 Specifica del protocollo applicativo REST

Convenzioni:

- Base URL: `https://api.smart-road.example/v1`
- Formato payload: **JSON** (UTF-8).
- Autenticazione: header `Authorization: Bearer <jwt_access_token>`, ottenuto via OAuth 2.0 (flow Authorization Code with PKCE per app mobile). Le risorse di sola lettura della segnaletica sono accessibili anche **senza autenticazione** (dato pubblico).
- Codici di stato HTTP standard (`200 OK`, `201 Created`, `400 Bad Request`, `401 Unauthorized`, `404 Not Found`, `409 Conflict`, `5xx`).
- Paginazione tramite `?page=1&size=20` con header `X-Total-Count`.

#### 7.3.1 Risorse principali

| Metodo | Endpoint | Descrizione | Auth |
|--------|----------|-------------|------|
| GET | `/segments` | Elenco dei tratti sperimentali | no |
| GET | `/segments/{id}/gates` | Smart-gate di un tratto | no |
| GET | `/gates/{id}/signage` | Segnaletica corrente di uno smart-gate | no |
| GET | `/gates/{id}/conditions` | Misure ambientali correnti | no |
| GET | `/stations` | Elenco stazioni di ricarica (filtrabile per autostrada/km) | no |
| GET | `/stations/{id}/points` | Punti di ricarica e loro stato | no |
| GET | `/stations/{id}/availability?from=...&to=...` | Disponibilità in finestra oraria | no |
| POST | `/auth/login` | Login utente | no |
| GET | `/users/me` | Profilo utente | sì |
| GET | `/users/me/vehicles` | Veicoli registrati | sì |
| POST | `/users/me/vehicles` | Aggiungere un veicolo | sì |
| GET | `/users/me/reservations` | Storico prenotazioni utente | sì |
| POST | `/reservations` | Creare una prenotazione | sì |
| GET | `/reservations/{id}` | Dettaglio prenotazione | sì |
| DELETE | `/reservations/{id}` | Annullare prenotazione | sì |

#### 7.3.2 Esempio: lettura segnaletica corrente

**Request**:

```http
GET /v1/gates/LO-01-042/signage HTTP/2
Host: api.smart-road.example
Accept: application/json
```

**Response**:

```http
HTTP/2 200 OK
Content-Type: application/json
Cache-Control: max-age=10
X-Updated-At: 2024-06-15T10:32:20Z

{
  "gateId": "LO-01-042",
  "position": { "autostrada": "A1", "km": 142, "direzione": "Nord" },
  "signage": {
    "speedLimit": 90,
    "lanes": [
      { "n": 1, "state": "open" },
      { "n": 2, "state": "open" },
      { "n": 3, "state": "closed" }
    ],
    "infoText": "Pioggia intensa, ridurre velocità",
    "ttlSec": 300
  },
  "lastUpdate": "2024-06-15T10:32:20Z"
}
```

#### 7.3.3 Esempio: creazione prenotazione

**Request**:

```http
POST /v1/reservations HTTP/2
Host: api.smart-road.example
Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5...
Content-Type: application/json

{
  "vehicleTarga": "EV123XY",
  "stationId": "ST-LO-007",
  "connectorType": "CCS",
  "expectedArrival": "2024-06-15T13:30:00+02:00",
  "expectedDurationMin": 30
}
```

**Response (successo)**:

```http
HTTP/2 201 Created
Location: /v1/reservations/r-44f7d9
Content-Type: application/json

{
  "id": "r-44f7d9",
  "status": "active",
  "assignedPointId": "PT-LO-007-03",
  "estimatedCost": 12.50,
  "qrCode": "data:image/png;base64,iVBORw0...",
  "validFrom": "2024-06-15T13:25:00+02:00",
  "validUntil": "2024-06-15T14:05:00+02:00"
}
```

**Response (errore - punto non più disponibile)**:

```http
HTTP/2 409 Conflict
Content-Type: application/json

{
  "error": "ConflictReservation",
  "message": "Nessun punto compatibile disponibile nella finestra richiesta",
  "alternatives": [
    { "stationId": "ST-LO-007", "earliestAvailable": "2024-06-15T14:15:00+02:00" },
    { "stationId": "ST-LO-008", "earliestAvailable": "2024-06-15T13:30:00+02:00" }
  ]
}
```

### 7.4 Specifica del canale push (MQTT)

L'APP, dopo l'apertura, individua il tratto autostradale in cui si trova l'utente (tramite GPS + endpoint `/segments`) e si sottoscrive ai topic di interesse sul broker pubblico:

```
smartroad/pub/<RR>/<TT>/+/signage     # tutti gli smart-gate di un tratto, stato schermo
smartroad/pub/<RR>/<TT>/+/conditions  # condizioni meteo/visibilità
smartroad/pub/<RR>/<TT>/alerts        # avvisi aggregati (incidenti, code)
smartroad/pub/<RR>/<TT>/charging      # variazioni stato punti di ricarica
```

Note implementative:

- I topic pubblici (`smartroad/pub/...`) sono un **sottoinsieme filtrato** dei topic interni: lo smart-gate pubblica su un topic interno, il bridge MQTT del CN replica solo i campi pubblici (senza dati sensibili come targhe) sui topic `pub`.
- Il broker pubblico richiede autenticazione utente via JWT (gli utenti anonimi possono usare token "guest" con permessi limitati).
- QoS: si usa **QoS 1** (at-least-once) per gli alert importanti, **QoS 0** per le misure ambientali.
- L'APP filtra lato client gli smart-gate troppo lontani (oltre 20 km), in modo da limitare il numero di sottoscrizioni attive.

Payload di esempio su `smartroad/pub/LO/01/042/signage`:

```json
{
  "gateId": "LO-01-042",
  "timestamp": "2024-06-15T10:35:00Z",
  "speedLimit": 70,
  "lanesClosed": [3],
  "infoText": "Incidente al km 145, rallentamenti",
  "ttlSec": 600
}
```

### 7.5 Sicurezza del protocollo

- Tutti gli scambi sono in **TLS 1.3** con certificati firmati da una CA pubblica (server) e con possibile **pinning** lato APP per resistere ad attacchi MITM con CA compromesse.
- I JWT hanno scadenza breve (15 min) e sono rinnovati tramite **refresh token** (rotazione del refresh token a ogni uso).
- **Rate limiting** per IP e per token sull'API Gateway, contro abusi e attacchi DoS.
- Validazione lato server di tutti gli input (lunghezza, formato, range), per prevenire injection.
- I dati di prenotazione restano nel DB nazionale; l'APP non memorizza dati sensibili (targhe, ecc.) oltre la sessione corrente.

### 7.6 Vantaggi della soluzione proposta

- **REST/HTTPS** è universalmente supportato, semplice da debuggare (curl, Postman), facile da cachare lato CDN per i dati pubblici (segnaletica, stazioni).
- **MQTT/WSS** dà push real-time efficiente in termini di batteria e banda (connessione persistente, payload compatti), passa attraverso firewall e proxy aziendali (porta 443).
- Il riuso del broker MQTT già presente nell'infrastruttura riduce costi e complessità.
- Versionamento dell'API (`/v1`, `/v2`) consente evoluzioni senza rompere i client già distribuiti.
