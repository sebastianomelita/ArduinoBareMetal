>[Torna a reti di sensori](sensornetworkshort.md#servizi-di-accesso-radio-per-WSN)>[Torna a Dettaglio architettura LoRaWAN](/lorawanclasses.md) 

[Azienda reale che si occupa di sensoristica stradale](https://www.yetipi.com/)

[Testo della prova](consegnaSmartroad.md)

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

# 1. Analisi della realtà di riferimento e ipotesi aggiuntive

Prima di entrare nel dettaglio tecnico è opportuno fissare alcune **ipotesi aggiuntive** che restringono il dominio del problema e rendono coerenti le scelte progettuali successive.

## 1.1 Ipotesi sul dominio

- **Tratti sperimentali**: si ipotizzano 20 **catene** di tratti di autostrada che si estendono complessivamente per circa 50 km ciascuno, una catena per regione (20 regioni), per un totale di circa 1.000 km coperti e **~1.000 smart-gate** (uno ogni km).
- **Smart-gate**: ognuno è una stazione "embedded industrial" con doppia alimentazione (rete elettrica + UPS + pannelli fotovoltaici di backup) installata a bordo strada.
- **Centri di controllo (CdC)**: uno per ogni tratto sperimentale, presso una sala operativa regionale della società autostradale, con presidio H24 da parte di un operatore.
- **Centro Nazionale (CN)**: due data-center geograficamente separati in configurazione **active-active** che ospitano il database nazionale, l'API gateway per l'APP utenti, il sistema di analisi BigData, il sistema di prenotazione ricariche.
- **Stazioni di ricarica**: ipotizziamo ~200 stazioni totali, con 4-8 punti di ricarica ciascuna, collegate al CN tramite il loro CdC regionale.
- **Volume di traffico dati**: ogni smart-gate trasmette ~1-2 KB/s di telemetria (sensori a bassa frequenza) e fino a ~5 Mbps in upload se sono attivi stream video dalle telecamere. In totale per un tratto: 50 smart-gate × 5 Mbps = ~250 Mbps di picco.

## 1.2 Vincoli normativi

- **GDPR**: le targhe sono dati personali; i flussi video sono trattati come dati personali. Devono essere previste cifratura at-rest e in-transit, retention limitata, mascheramento per il dataset di analytics.
- **Codice della strada**: la segnaletica variabile è normata; gli smart-gate devono poter operare in **modalità degraded** anche se isolati dal CdC (segnaletica preimpostata di sicurezza).

---

# 2. Architettura della rete di distribuzione - rete IP di reti LoRaWAN

L'architettura proposta è **gerarchica a 3 livelli**, modello che si presta naturalmente al problema perché replica la struttura fisica della rete autostradale (gate locale → tratto regionale → coordinamento nazionale).

<img src="../img/architettura_3_livelli.svg" alt="Architettura gerarchica a tre livelli" width="680">

## 2.1 Livello smart-gate (edge) - rete LAN di tratto

Ogni smart-gate è un nodo **edge computing** con le seguenti caratteristiche:

| Componente | Specifica | Motivazione |
|------------|-----------|-------------|
| SoC | Industrial mini-PC con CPU quad-core ARM/x86, 8 GB RAM, SSD 256 GB | Capacità di eseguire localmente algoritmi di computer vision (riconoscimento targhe) e logica di emergenza |
| OS | Linux real-time hardened | Determinismo per il controllo del PMV (Pannello a Messaggio Variabile), sicurezza |
| Telecamere | 2-4 telecamere IP PoE+ FullHD, una con ottica per targhe (OCR locale) | Riduzione banda: si trasmette il dato strutturato (targa+timestamp), non sempre il video |
| Sensori ambientali | Misure di meteo, visibilità, condizioni del fondo stradale, qualità dell'aria (PM10/PM2.5/NOx) e inquinamento acustico | **Non cablati al SoC**: sono end-device LoRaWAN wireless distribuiti lungo il km sul guard-rail, raccolti dal gateway LoRaWAN del cabinet (vedi §3.2) |
| Gateway LoRaWAN | Concentratore radio 868 MHz (es. Semtech SX1302, 8 canali) | Aggrega i sensori wireless del km e li inoltra via IP al network server |
| PMV (Pannello a Messaggio Variabile) | LED matrix controller via Ethernet | Aggiornamento dinamico via API REST locale |
| Connettività primaria | Fibra ottica dedicata (lungo la canalina del guard-rail) | Banda garantita, latenza bassa, alimentazione PoE++ possibile |
| Connettività di backup | Modem 5G/4G LTE con SIM M2M | Continuità in caso di taglio fibra |

Il fatto di concentrare elaborazione locale (edge) è una scelta motivata da:

1. **Riduzione della banda**: non si trasmette il video raw, ma metadati (targa rilevata, conteggio veicoli, classe del veicolo).
2. **Bassa latenza nelle decisioni di sicurezza**: se un sensore rileva ghiaccio sull'asfalto, lo smart-gate può abbassare autonomamente il limite di velocità sul PMV (Pannello a Messaggio Variabile) in < 100 ms, senza attendere il CdC.
3. **Modalità degraded**: in caso di isolamento, lo smart-gate continua a operare in autonomia con segnaletica conservativa.

## 2.2 Livello Centro di Controllo (CdC) - rete LAN datacenter regionale

Ogni CdC è un **mini data-center regionale** che funge da aggregatore per ~50 smart-gate. Comprende:

- **Server applicativo** ridondato (cluster Kubernetes a 3 nodi) per l'orchestrazione dei servizi.
- **Broker MQTT** (es. Mosquitto o EMQX) per la messaggistica publish/subscribe verso gli smart-gate.
- **Database operativo locale** (PostgreSQL in HA con streaming replication) per i dati real-time del tratto.
- **Storage video** (NAS con ~100 TB) per la conservazione temporanea (es. 30 giorni) dei flussi delle telecamere.
- **Workstation operatore** con dashboard di monitoraggio e console di override della segnaletica.
- **Firewall di frontiera** (cluster active/standby) con segmentazione VLAN interna.
- **Router di accesso WAN** verso il CN (doppia uscita: MPLS primario + Internet con VPN IPsec come backup).

## 2.3 Livello nazionale (CN) - rete LAN datacenter nazionale

Il CN è progettato come **due data-center attivo-attivo** geograficamente separati (es. Roma e Milano), con repliche sincrone del DB tramite link in fibra dedicato. Ospita:

- **Database nazionale** (cluster relazionale con sharding geografico) con tutti i dati storici di segnaletica e telemetria.
- **Data lake** per analytics offline (Hadoop/Spark o equivalente cloud).
- **API Gateway** pubblico per l'APP utenti (esposto in HTTPS dietro WAF e CDN).
- **Servizio di prenotazione ricariche** (microservizio a sé stante con il proprio database).
- **Centro operativo nazionale** con monitoraggio aggregato di tutti i tratti.



## 3.2 Rete WSN del tratto 

Una delle scelte progettuali più caratterizzanti del progetto è realizzare la sensoristica ambientale come **rete wireless LPWA in tecnologia LoRaWAN** distribuita lungo il km di carreggiata di pertinenza di ogni smart-gate. Questa scelta merita un'argomentazione esplicita perché incide su molti aspetti dell'infrastruttura.

### 3.2.1 Topologia fisica della rete di tratto (IP + LoRaWAN)

Topologia della rete LAN dei sensori di un generico tratto:

<img src="../img/topologia_lorawan_km.svg" alt="Topologia LoRaWAN del km autostradale" width="680">

La **rete ethernet** è composta da uno **piccolo switch** che connette: 

- un **mini PC** per l'elaborazione locale dei comandi di sicurezza
- un **gateway LoRaWAN** che si interfaccia con la rete di sensori distribuita sul tratto di autostrada. 
- **Telecamere IP + PMV (Pannello a Messaggio Variabile)**: tecnologia cablata Ethernet/IP, perché producono e consumano traffico ad alta banda (FullHD streaming, comandi di segnaletica con conferma). Le telecamere si collegano via **ONVIF/RTSP** su una piccola LAN PoE+ interna allo smart-gate; il PMV (Pannello a Messaggio Variabile) è raggiungibile via TCP/IP attraverso API standard (es. EN 12966 in UE per la segnaletica variabile).

---

# 4. Confronto e scelta tra tecnologie di rete di distribuzione

Si è scelto di mettere a confronto due tecnologie per la realizzazione di una rete aziendale privata di dimensioni geografiche:
1. una pubblica MPLS nazionale, cioè una rete VPN Trusted fornita da ISP nazionale sia per il tratto regionale che per quello nazionale.
2. una pubblica MPLS nazionale per il tratto regionale e una rete metropolitana privata, gestita direttamente dall'ente autostrade regionale, per la rete regionale.

**Scelta: anello Ethernet L2 con ERPS.** Motivazioni:

1. **Failover sotto i 50 ms** è essenziale per garantire che gli stream video on-demand e la telemetria MQTT non vedano interruzioni percepibili anche durante un guasto.
2. **Banda dedicata** per smart-gate (~1 Gbps) è ampiamente sufficiente per i flussi video FullHD on-demand e per la telemetria.
3. **Switch L2 industriali sono prodotti maturi e standardizzati** (esempi: Hirschmann, Moxa, Cisco IE series, Westermo). Costo accettabile, MTBF molto alto (decine di migliaia di ore).
4. **L'alimentazione dello switch piggy-backa su quella già presente nello smart-gate**: lo switch sta dentro il cabinet del maxi-schermo, alimentato dalla stessa linea, esattamente come il gateway LoRaWAN. Carico aggiuntivo trascurabile (10-20 W).
5. **Apparato fisicamente compatto**: uno switch industriale a 8-16 porte sta in un modulo DIN-rail da poche unità rack, non aumenta significativamente l'ingombro del cabinet.

| Caratteristica | A — Anello L2 ERPS | B — Anello L3 OSPF | C — PON |
|----------------|-------------------|--------------------|---------| 
| Scelta per il progetto | ✅ **Adottata** | ❌ Esagerata | ❌ Insufficiente resilienza |

E' stata selezionata come migliore la seconda opzione in questa fase iniziale di diffusione dei tratti, scelta che può rapidamente connettersi ed espandersi in un contesto nazionale poggiandosi alle reti Trusted MPLS esistenti. 

## 4.1 Tracciato fisico della fibra

La fibra fisica viene posata in modi diversi a seconda del contesto:

- **Tratto a cielo aperto**: cavo in fibra armata, **canalina interrata** sotto la banchina o sotto il guard-rail. Posa con scavi minimi grazie a tecniche di **mini-trenching** (taglio della pavimentazione, posa del cavo, sigillatura). Profondità tipica 30-50 cm.
- **Tratti in galleria**: cavo fissato alla **controsoffittatura** o a bracci metallici lungo la parete del tunnel, con guaine ignifughe LSZH (Low Smoke Zero Halogen).
- **Tratti su viadotto**: cavo armato fissato sotto l'impalcato in alloggiamenti dedicati, con compensazione meccanica per le escursioni termiche.
- **Pozzetti di derivazione** a ogni smart-gate (o ogni paio di km): contengono i giunti ottici e i cassetti di permutazione per spillare la fibra dello smart-gate dal cavo dell'anello. Il pozzetto è impermeabile (IP68), accessibile dalla strada per manutenzione senza chiudere la carreggiata.

Il cavo in fibra tipico per questa applicazione ha **24 o 48 fibre** ottiche monomodali (G.652 standard), di cui solo 4-8 effettivamente utilizzate per la rete dello smart-gate: le altre sono **fibre di scorta ("dark fiber")** per espansioni future, sostituzione di fibre danneggiate, o servizi aggiuntivi (es. videosorveglianza dedicata, connettività per i ristoranti/aree di servizio lungo il tratto).


### 4.1.1 Cluster di tratti 

Come si "spilla" la fibra lungo le decine di km del tratto per servire ogni smart-gate? Esistono tre approcci: un **anello Ethernet attivo con switch L2** (rigenerazione attiva a ogni km, failover ERPS < 50 ms), un **anello IP con router L3** (più costoso e con failover più lento) e una **PON con splitter ottici passivi** (niente apparato attivo a bordo strada, ma nessuna ridondanza ad anello).

Per il progetto si adotta l'**anello Ethernet L2 con ERPS (IEEE G.8032)**: ogni smart-gate ospita uno switch industriale con 2 porte ottiche (anello) e porte di accesso per gli apparati interni; lo switch è alimentato dalla stessa linea del PMV (Pannello a Messaggio Variabile). Il failover sotto i 50 ms garantisce continuità per gli stream video e la telemetria anche in caso di taglio della fibra. La fibra è posata in canalina sotto il guard-rail (mini-trenching), con cavi a 24-48 fibre di cui buona parte tenuta di scorta (dark fiber) per espansioni future.la'adattamento 

E' l'adattamento in ambito metropolitano di una tipica architettura ethernet industriale composta da un albero di LAN ridondate mediante topologia ad anello.

<img src="../img/anello_fibra_erps.svg" alt="Anello in fibra ottica con switch ERPS a ogni km" width="680">

> **Dettaglio completo** — confronto tecnico delle tre tecnologie (rigenerazione attiva vs spillamento passivo), meccanica del protocollo ERPS, specifiche dello switch industriale, tracciato fisico della posa: vedi il file [dettaglio_spillamento_fibra.md](./dettaglio_spillamento_fibra.md).

---
# 5. Topologia logica

## 5.1. Topologia logica della rete di sensori nel caso di rete fisica A2

Il **dispositivo di tratta** è un PE (Provider Edge), cioè un **router di confine** di una rete **MPLS**, assimilabile in pratica ad un **tunnel TAP** tra il PE sul tratto e il router PE nella sede regionale. Essendo un tunnel L2 su di esso sono allocate **subnet condivise**, una per ogni VLAN trasportata dalle dorsali di trunk.

In questo schema i sensori/attuatori non sono **client MQTT diretti** (non parlano MQTT loro stessi via WiFi/IP). In questo caso i sensori parlano **LoRaWAN**, non MQTT, quindi non sono publisher MQTT in prima persona. È l'**application server edge** (co-locato col gateway) che fa il publish MQTT a nome dei sensori, dopo aver decodificato il payload Cayenne LPP (vedi §3.2.7). Quindi dal punto di vista del **piano L7 MQTT** il publisher non è il sensore ma il nodo edge — il sensore è "trasparente" rispetto a MQTT. Lo riflette il disegno facendo partire il link L7 dal nodo edge, non dal sensore:
1. **L3 e L7 non coincidono.** Un sensore del tratto A1 raggiunge il dashboard del CdC fisicamente attraverso quattro nodi IP (gateway edge → firewall A1 → dorsale → firewall CdC → switch CdC → dashboard), ma a livello applicativo MQTT ci sono solo **due salti**: gateway edge → broker, e broker → dashboard. Il broker "appiattisce" la topologia IP dal punto di vista applicativo: chi pubblica e chi si abbona vedono solo il broker, non la rete IP sottostante.
2. **Il broker è il "router applicativo".** A livello IP ha un solo indirizzo (`.10.x` nel CdC) e un solo collegamento al resto della rete. Ma a livello L7 ha molti "vicini" (publisher e subscriber distribuiti su tre tratti diversi), e instrada i messaggi tra di loro **in base al topic**, non in base all'indirizzo IP. È esattamente l'analogia del **router applicativo**: come un router IP smista pacchetti in base all'indirizzo destinazione, il broker MQTT **smista** messaggi in base al **topic destinazione**.

<img src="../img/cdc_lombardia_due_livelli.svg" alt="cdc_lombardia_due_livelli" width="900">
 
La **linea tratteggiata** rappresenta il tipo di servizio "like wired" scelto per interconnettere i gateway dei vari tratti con il loro CdG (Centro di Gestione) regionale, una **Trusted VPN MPLS**. Garantisce SLA contrattuali ruguardo a: classi di servizio (QoS), autenticazione dei nodi gateway e isolamento.

## 5.2. Topologia logica della rete di sensori nel caso di rete fisica A1

In questo caso la rete di sensori è analoga ad una grande LAN industriale composta di soli switch:
- **switch di tratto**, in serie a quello del tratto successivo
- **switch di multiplazione**:
     - con una **porta di loop** si chiude, mediante una fibra lunga quanto un tratto, sul primo switch del loop.
     - Con una seconda **porta di dorsale** si collega ad un MUX/DEMUX che genera, su un'unica fibra ottica comune per tutti i tratti, un **link multiplato** dedicato a quel cluster di tratti, associandolo ad una certa lunghezza d'onda della luce (multiplazione DWDM). Ogni link multiplato arriva su una porta dedicata del **core switch** L2 che, essendo locato nel centro di controllo regionale, crea una stella di cluster di tratti (tratti del cluster tra loro collegati ad anello).

<img src="../img/cdc_lombardia_B1_dwdm.svg" alt="cdc_lombardia_B1_due_livelli" width="900">

Il link equivale ad una **dorsale di trunk** tra due switch e aggrega su di se le **dorsali logiche** di tutte le VLAN di un tratto. Il CS L2 provvederà a fondere insieme le VLAN di ogni tratto. Su ogni VLAN si allocheranno, mediante subnetting, **tre subnet separate** (una per i sensori, una per le videocamere, una per le colonnine di ricarica) condivise su tutti i tratti.

## 5.3. Componenti dello switch a ogni smart-gate

Lo switch tipo per questo scenario ha le seguenti caratteristiche:

- **2 porte SFP+** per la fibra dell'anello (10 Gbps per affrontare il caso di tutti i flussi video aggregati che attraversano lo smart-gate verso il CdC).
- **4-8 porte Gigabit Ethernet** per gli apparati interni allo smart-gate (SoC edge, telecamere PoE+, gateway LoRaWAN, controller maxi-schermo). Alcune con **PoE+** o **PoE++** per alimentare le telecamere.
- **Alimentazione ridondata** (doppia) 24-48 VDC, alimentata dalla rete dello smart-gate con UPS.
- **Temperatura operativa estesa** -40°C / +75°C: lo switch può lavorare anche in galleria d'estate o in montagna d'inverno senza condizionamento.
- **Supporto ERPS** (G.8032) e VLAN 802.1Q nativi.
- **Management out-of-band** via porta console seriale e via SNMPv3/SSH dalla rete di management dedicata.

## 5.4 L'L2 resta in regione: l'anello regionale

Il dominio di livello 2 — l'anello ERPS (o il cluster DWDM, §5.2) che porta in trunk le VLAN-funzione — ha **scope regionale**. Coincide con l'estensione fisica del tratto/cluster (~50 km) e si chiude sul Core Switch del CdC. Non si estende oltre.

La scelta è obbligata, non estetica: **una VLAN è un dominio di broadcast, e un dominio di broadcast esteso per mezza nazione è un dominio di guasto nazionale.** Estendere l'L2 fino a un apparato nazionale significherebbe:

- far convergere **ERPS/STP** su collegamenti WAN di centinaia di km (tempi e stabilità incompatibili con il failover < 50 ms che cerchiamo localmente);
- propagare **broadcast e ARP** di tutti i dispositivi attraverso la dorsale interregionale;
- creare un **failure domain unico**: un singolo loop o una tempesta in una regione si propagherebbe a tutte.

Per questo l'anello, **come adesso, è e resta regionale**. È il livello 2 giusto: locale, veloce, contenuto.

## 5.5 Il router regionale come confine L2/L3

Al CdC si **spilla dallo switch un router regionale** (un router/firewall dedicato, oppure SVI su uno switch multilayer). È l'unico punto in cui l'L2 della regione diventa L3, e svolge tre ruoli:

1. **Primo hop L3 dei quattro piani.** Ha un'interfaccia/SVI *dentro* ciascuna subnet di funzione (telecamere, sensori, controller, management) ed è il loro **default gateway**. Tutto il traffico inter-VLAN *servizi ↔ dispositivi* della regione (dashboard → telecamera, broker → gateway sensori, comando → PMV) viene instradato **sul posto**, al CdC: niente più hairpin verso il nazionale (cfr. §8.2).
2. **Punto di policy.** Applica le ACL fra i piani, fa da **gateway promiscuo** per le Private VLAN (§8.3) e impone che i piani si parlino solo attraverso regole esplicite (gli smart-gate non si parlano tra loro, §10.2).
3. **Confine verso la WAN.** Termina la regione e annuncia verso il nazionale **solo rotte L3** (vedi §5.6).

```
   ── REGIONE (L2) ────────────────┐        ── L3 ──►  NAZIONALE
   anello ERPS / cluster DWDM      │
   VLAN telecamere · sensori ·     │   ┌─────────────────┐
   controller · management         ├──►│   ROUTER        │  rotte sommarizzate
   (dominio di broadcast regionale)│   │  REGIONALE      │──────────► MPLS L3VPN
   chiuso sul Core Switch del CdC  │   │ (confine L2/L3) │           verso il CN
   ────────────────────────────────┘   └─────────────────┘
                                        gateway dei 4 piani
                                        + ACL/PVLAN + de-hairpin
```

> **Variante** — per un singolo nodo di convergenza (es. il collector/broker) è ammissibile un'**applicazione 4-homed** (una sotto-interfaccia 802.1Q per piano) che parla con ogni piano *on-link*, senza routing. In tal caso va **disabilitato l'IP forwarding** sull'host (deve essere un ponte applicativo, non un router occulto fra i piani) e l'host va trattato come confine di sicurezza. Quando i servizi che toccano i piani sono molti, è però preferibile il **router regionale**, più generale e con la policy in un punto solo.

## 5.6 Verso il nazionale: solo livello 3 (MPLS L3VPN)

Tra regione e CN si viaggia **esclusivamente in L3**. Il router regionale annuncia verso il backbone, su **MPLS L3VPN**, poche **rotte sommarizzate** (un prefisso per regione, o uno per funzione — cfr. il piano di §8.1), eventualmente con una **VRF per funzione** se si vuole mantenere la separazione dei piani anche a livello nazionale.

Di conseguenza i **"router nazionali per funzione"** sono punti di **aggregazione/instradamento L3** (VRF): ricevono prefissi *instradati*, **non** estendono l'L2 dei piani. Nessun dominio di broadcast attraversa la WAN. Ogni dominio L2/broadcast/guasto resta circoscritto a una regione, e questo è esattamente l'obiettivo.

## 5.7 Flussi verso il Centro Nazionale

Il CN ha due esigenze diverse, servite da due meccanismi diversi — nessuno dei quali richiede di estendere l'L2 né di dare al CN raggiungibilità L3 grezza verso i dispositivi.

**Segnalazioni → MQTT bridge.** Le misure, gli eventi (targhe), gli allarmi e lo stato salgono via **bridge MQTT** broker-regionale → broker-nazionale (§7.3.1): è pub/sub a livello 7, quindi il CN riceve i dati **senza alcuna rotta L3 verso i dispositivi**. Flusso sempre attivo e leggero.

**Immagini → relay media regionale, on-demand.** Lo streaming (RTSP/SRT) non è pub/sub: per vedere una telecamera dal CN serve un percorso fino ad essa. Invece di esporre al CN migliaia di telecamere, si interpone un **relay/proxy media regionale** (può essere lo stesso nodo di convergenza, già attestato sul piano telecamere). Il CN apre lo stream **a richiesta** verso quell'unico endpoint regionale, attraverso il **firewall**, con autenticazione, logging e mascheramento (GDPR); il proxy tira il flusso dalla telecamera locale e lo rilancia a nord. Quel singolo flusso on-demand sale legittimamente in L3 via MPLS — è inter-regionale, raro, verso un solo endpoint — mentre i flussi video locali non lasciano mai la regione.

## 5.8 Riepilogo dei confini

| Flusso                                   | Dove si chiude        | Meccanismo                                  |
| ---------------------------------------- | --------------------- | ------------------------------------------- |
| Servizi ↔ dispositivi (locale)           | **Regione** (CdC)     | Router regionale (inter-VLAN locale)        |
| Comandi ai PMV / decisioni di sicurezza  | **Regione / edge**    | Router regionale (+ edge in degraded)       |
| Anello, broadcast, dominio di guasto     | **Regione**           | ERPS / DWDM confinati alla regione          |
| Segnalazioni (misure, eventi, allarmi)   | **Nazionale**         | MQTT bridge (pub/sub L7)                     |
| Visione immagini dal CN                  | **On-demand**         | Relay media regionale via firewall          |
| Raggiungibilità inter-regione            | **Nazionale**         | MPLS L3VPN (solo L3, rotte sommarizzate)    |

In una frase: **l'L2 vive in regione (l'anello), il router regionale è il confine, e oltre la regione corre solo l'L3.**

---

# 6 Dispositivi LoRaWAN

## 6.1 Sensori - Coloro che generano e cifrano il payload

I sensori sono **end-device LoRaWAN in classe A**, distribuiti lungo il km e ancorati al guard-rail. Caratteristiche:

- **Alimentazione**: batteria al litio (es. AA da 3000 mAh) con piccolo **pannello fotovoltaico ausiliario** (formato 5×5 cm o 10×5 cm, tipo "da calcolatrice") e batteria tampone Li-Ion o supercondensatore. Combinazione che garantisce **autonomia energetica indefinita**: il pannellino ricarica la batteria di giorno, la batteria sostiene il funzionamento di notte e durante le giornate nuvolose.
- **Classe A**: il modem radio dorme la maggior parte del tempo e si sveglia solo per trasmettere un breve uplink, seguito da due brevissime finestre di ricezione di downlink. È la classe a minimo consumo energetico — adeguata perché i sensori producono solo telemetria e non hanno bisogno di ricevere comandi tempestivi.
- **Cadenza di trasmissione**: una misura ogni 30-60 secondi a seconda del sensore, compatibile con le limitazioni di **duty cycle** della banda ISM 868 MHz (1% del tempo nelle sotto-bande tipiche).
- **Formato di payload**: messaggi brevi in formato binario compatto, tipicamente **Cayenne LPP** o codec custom dichiarati sotto forma di struct C. Il payload è cifrato con AES tramite la chiave di sessione **AppSKey** ottenuta in fase di OTAA.
- **Identificazione**: ogni sensore ha un **DevEUI** univoco a 64 bit derivato dal MAC Ethernet via EUI64. Non ha indirizzo IP — l'IP entra in gioco solo dal gateway in poi.
- **Custodia**: enclosure IP67/IP68 robusta, fissata al guard-rail con staffe a bullone, resistente a vibrazioni, pioggia, sale, escursioni termiche -20°C/+60°C.
- **Tipologie**: tipicamente 10-30 sensori per km, suddivisi tra centraline meteo, sensori di qualità dell'aria (PM10, PM2.5, NOx), fonometri, sensori di vibrazione/temperatura del fondo stradale, sensori di luminosità ambientale.

Dal punto di vista del **firmware**, il sensore segue un ciclo semplice: dopo un join OTAA iniziale, a ogni risveglio dal deep sleep legge i valori, li codifica in formato compatto (Cayenne LPP), trasmette un uplink LoRaWAN, ascolta brevemente per eventuali downlink e torna a dormire. È lo stesso schema a fasi del firmware MQTT classico (inizializza → leggi → invia a intervalli → ripeti), ma adattato al risparmio energetico estremo di LoRaWAN: niente connessione persistente, deep sleep tra una trasmissione e l'altra.

> **Dettaglio completo** — schema a fasi, macchina a stati, pseudocodice commentato ed esempio in C++ (Arduino/LMIC), gestione dell'energia e formato Cayenne LPP: vedi il file [dettaglio_firmware_sensore.md](./dettaglio_firmware_sensore.md).


## 6.2 Gateway LoRaWAN - Colui che smista il payload

I payload ricevuti dal gateway vengono inoltrati in flooding verso il network server (NS). Il **gateway** è esattamente il **punto di traduzione** (significato di gateway) tra il mondo LoRa (senza IP) e il mondo IP/MQTT. Sopra LoRaWAN non c'è IP: il sensore non ha alcun indirizzo IP, ha solo il suo DevEUI.

### 6.2.1 Funzioni del gateway:

- **Packet forwarder**: riceve i pacchetti radio dai sensori e li inoltra al network server via IP. Quando un sensore è ricevibile anche dal gateway dello smart-gate adiacente, lo stesso pacchetto arriva al network server da due percorsi diversi — la **ridondanza è gratuita**, e il network server scarta il duplicato tenendo quello con RSSI/SNR migliore.
- **Bridge LoRa→MQTT**: la componente `lora-gateway-bridge` incapsula il messaggio LoRaWAN in un payload MQTT di servizio (in JSON) pubblicato su un broker locale, che il network server consuma.
- **Coordinatore radio**: applica le politiche di **Adaptive Data Rate (ADR)** decise dal network server, assegnando a ciascun sensore data rate e potenza di trasmissione ottimali. Sensori vicini al gateway → data rate alto, potenza bassa (consumo minimo). Sensori lontani → data rate basso (più resistente al rumore), potenza alta.

### 6.2.2 Allocazione fisica del gateway

Nel cabinet del PMV (Pannello a Messaggio Variabile)

| Tratta | Protocollo | Formato messaggi | Identificazione |
|--------|-----------|------------------|-----------------|
| Sensore ↔ Gateway (radio) | LoRaWAN (PHY/MAC LoRa) | Binario compatto (Cayenne LPP), cifrato AES con AppSKey | DevEUI a 64 bit |
| Gateway ↔ Network Server | IP su fibra/5G, MQTT su TLS | JSON di servizio con payload LoRa incapsulato | Indirizzi IP privati |
| Network Server ↔ Server applicativo | IP, MQTT su TLS | JSON applicativo dopo decodifica Cayenne LPP | Indirizzi IP privati |


Il gateway LoRaWAN del km è **ospitato all'interno del cabinet del PMV (Pannello a Messaggio Variabile) a portale**, scelta motivata da quattro ragioni concrete:

1. **Posizione baricentrica e in altezza** (5-7 m da terra): l'antenna del gateway è in line-of-sight con tutti i sensori del km, fuori dagli ostacoli (veicoli, guard-rail). Il bilancio di link migliora di 10-20 dB rispetto a un'installazione a livello strada, il che si traduce in margine per data rate più aggressivi e quindi consumi ancora più bassi sui sensori.
2. **Alimentazione condivisa**: il PMV (Pannello a Messaggio Variabile) dispone già di una linea elettrica robusta (rete elettrica con UPS, oppure — nei tratti remoti — impianto fotovoltaico dimensionato per centinaia di watt). Il gateway LoRaWAN consuma 5-10 W, carico marginale che non richiede infrastruttura elettrica aggiuntiva.
3. **Sicurezza fisica e ambientale**: l'enclosure IP65/66 del display protegge anche il gateway da intemperie e vandalismo; manutenzione condivisa con il display.
4. **Connettività IP condivisa**: lo stesso cavo Ethernet che porta i comandi al PMV (Pannello a Messaggio Variabile) viene riutilizzato come uplink IP per il gateway LoRaWAN verso il network server nel CdC. Un solo cavo dati per più funzioni.


### 6.2.3 Modalità "All-In-One" per tratti remoti

Per i tratti autostradali in zone scarsamente coperte dalla fibra (passi montani, contesti isolati), il gateway LoRaWAN può essere realizzato come **gateway All-In-One con doppia interfaccia**: LoRaWAN verso i sensori, modem **5G/4G** o connettività **satellitare LEO** (es. Starlink Direct-to-Cell) verso il network server. È la stessa configurazione utilizzata in agricoltura di precisione e nel monitoraggio ambientale di aree remote.


## 6.3.  Network server - Colui che autentica il payload

### 6.3.1. Architettura distribuita vs centralizzata

**Separazione dei ruoli del network server.** È un altro punto fondamentale per chiarire l'architettura. La specifica LoRaWAN identifica diversi ruoli funzionali che possono stare insieme su una sola macchina o essere distribuiti:

| Ruolo | Cosa fa | Vincolo di latenza | Collocazione nel progetto |
|-------|---------|---------------------|---------------------------|
| **Packet forwarder** | Inoltra i pacchetti radio al NS | Bassissimo | Sempre nel gateway (smart-gate) |
| **Network Server** | Deduplica, gestione MAC, ADR, sessione | Alto per real-time | Locale allo smart-gate (strato edge) + replica al CdC |
| **Join Server** | Autenticazione OTAA, gestione chiavi | Basso (interviene solo al primo join) | Centralizzato — vedi §3.2.10 |
| **Application Server** | Decifratura (AppSKey) + decodifica payload (Cayenne LPP), logica di business | Variabile per funzione | **Decodifica all'edge** insieme al NS (§3.2.7); aggregazione/analytics in nazionale |

Questa separazione fisica è esattamente quella che la specifica LoRaWAN raccomanda e che si trova nelle implementazioni reali (ChirpStack, The Things Stack, Actility ThingPark).

Una scelta progettuale importante riguarda **dove collocare fisicamente i network server**. La specifica LoRaWAN canonica prevede una topologia "stella di stelle" in cui i gateway sono packet forwarder stupidi e tutta l'intelligenza sta nel network server. Nel nostro progetto questa scelta classica presenta un problema serio.

**Il problema della latenza decisionale.** Consideriamo un caso concreto: un sensore di vibrazione del fondo stradale rileva un picco anomalo compatibile con una buca improvvisa. La catena standard sarebbe:

1. Sensore → uplink LoRa → gateway del km (decine di ms)
2. Gateway → IP/MQTT → network server al CdC regionale (centinaia di km di distanza, decine di ms su fibra, fino a centinaia in caso di backup 5G)
3. Network server → application server → logica di business → decisione "abbassa il limite a 50 km/h sullo smart-gate del km"
4. Decisione → MQTT → SoC dello smart-gate → PMV (Pannello a Messaggio Variabile)

Il tempo totale è dell'ordine di **centinaia di millisecondi nel caso ottimo, secondi nei casi degradati**. Per la sicurezza autostradale è un problema: una buca a 130 km/h è pericolosa, e ogni mezzo secondo in più di latenza significa 18 metri di strada percorsi a velocità non ridotta da ogni veicolo che transita. Inoltre i pacchetti viaggiano sulla WAN regionale anche solo per essere processati e tornare indietro: traffico distribuito su tutta la rete per una decisione che è puramente locale.

**Tre configurazioni possibili.** L'industria LoRaWAN ha riconosciuto questo problema e l'evoluzione architettonica recente prevede di **avvicinare il network server al gateway** (pattern Edge Network Server, diffuso nel contesto Industrial IoT e mission-critical IoT). Le configurazioni ragionevoli sono tre:

| Configurazione | Collocazione del NS | Sensori per NS | Latenza decisione | Numero di NS da gestire | Adatta per |
|----------------|---------------------|----------------|-------------------|--------------------------|------------|
| **A — NS locale per gateway** | Dentro lo smart-gate, sullo stesso SoC | 10-30 | ms (loopback) | ~1.000 | Decisioni di sicurezza in tempo reale |
| **B — NS ogni N gateway** | In uno smart-gate "capo-gruppo" o in armadio dedicato | 50-150 (5-10 km) | decine di ms | ~100-200 | Decisioni coordinate su gruppi di km |
| **C — NS regionale (classica)** | Al CdC regionale | ~1.500 | centinaia di ms | ~20 | Decisioni non time-critical, archiviazione |

La configurazione C corrisponde al pattern "federazione di reti LoRaWAN" raccomandato dalla specifica, in cui ogni regione ha il proprio network server e tutti convergono al CN a livello applicativo:

<img src="../img/federazione_network_server.svg" alt="Federazione dei network server LoRaWAN" width="680">

**Scelta per il progetto: architettura ibrida a due strati.** La scelta corretta non è "scegliere una configurazione e basta", ma realizzare una **gerarchia funzionale** che usa configurazioni diverse per ruoli diversi:

- **Strato edge — Network server locale per gateway (configurazione A)**: gestisce in autonomia tutte le funzioni con vincoli di latenza:
  * decisioni di sicurezza basate sui sensori (ghiaccio, buche, smottamenti, visibilità ridotta)
  * attivazione automatica della segnaletica di emergenza sul PMV (Pannello a Messaggio Variabile) locale
  * gestione dell'ADR (Adaptive Data Rate) dei sensori vicini
  * **modalità degraded autonoma**: anche se la fibra è tagliata e il 5G è giù, lo smart-gate continua a gestire i propri sensori e a prendere decisioni di sicurezza
- **Strato regionale — Network server al CdC (configurazione C, in parallelo)**: gestisce le funzioni di coordinamento sovra-locale e di archiviazione:
  * aggregazione e archiviazione dei dati di tutti gli smart-gate del tratto
  * analytics di tratto (previsione del traffico, pattern stagionali)
  * coordinamento tra smart-gate distanti (es. propagare l'informazione di una chiusura corsia su tutto il tratto)
  * override manuale da parte dell'operatore di sala

La motivazione forte per la configurazione A allo strato edge è l'argomento dell'**autonomia in modalità degraded**: ogni smart-gate deve poter continuare a operare correttamente anche se completamente isolato dalla rete IP, e questo è possibile solo se ha al suo interno tutta la pipeline LoRaWAN locale (gateway + network server + application server locale per le decisioni di sicurezza).

**La gestione dei 1.000 network server locali.** L'obiezione naturale è: "1.000 network server da gestire sono ingestibili". È vero solo con strumenti vecchi. Con il **fleet management moderno** (Ansible/Salt per la configurazione, container Docker o K3s edge per il deployment, Prometheus per il monitoring, OTA firmware update via canale MQTT sicuro) la gestione di 1.000 dispositivi edge identici è un'operazione standardizzata. Le grandi reti CDN e le flotte di POS gestiscono decine o centinaia di migliaia di nodi edge con questi strumenti — 1.000 è un numero piccolo.

## 6.4. Application  server - Colui che decifra il payload

## 6.4.1. Chiave di sessione

Nella catena LoRaWAN i ruoli agiscono **in sequenza**:

1. Il **network server** riceve il frame, verifica il MIC con la NwkSKey (integrità + autenticazione), deduplica e gestisce l'ADR. **Non legge il contenuto applicativo.**
2. L'**application server** riceve il payload, lo **decifra** con la AppSKey e poi lo **decodifica** dal formato compatto Cayenne LPP al JSON leggibile.

Ne segue una conseguenza architetturale vincolante: **la decodifica del payload può avvenire solo dove c'è l'application server, che a sua volta presuppone a monte il network server.** Per questo, avendo scelto (vedi §3.2.9) di portare le decisioni di sicurezza all'edge, mettiamo **network server + application server insieme all'edge** — a bordo di ogni gateway (configurazione A) o ogni N gateway (configurazione B). È lì che il payload diventa un dato in chiaro.

### 6.4.2. I due livelli di cifratura (indipendenti)

Sul percorso del dato convivono **due cifrature sovrapposte e indipendenti**, che proteggono cose diverse:

| Livello | Cosa cifra | Con quale chiave | Chi può "aprirla" |
|---------|-----------|------------------|-------------------|
| **Applicativo (end-to-end LoRaWAN)** | Il payload del sensore (le misure) | **AppSKey** | Solo l'application server |
| **Trasporto (a salti)** | Il canale IP tra due nodi (involucro MQTT, topic, metadati) | sessione **TLS** | I nodi terminanti TLS (gateway, broker, ecc.) |

La chiave è che **il payload resta cifrato con AppSKey anche dentro il tunnel TLS**: nessun nodo intermedio (gateway che inoltra, broker che smista) può leggere le misure, perché non possiede la AppSKey. Il TLS protegge l'involucro; la AppSKey protegge il contenuto.

### 6.4.3. Come viaggia il dato verso il Centro Nazionale

Poiché network server + application server stanno all'edge, **la decifratura e la decodifica avvengono vicino alla sorgente**. Da quel momento il dato è in chiaro (JSON) e viaggia verso il CN come normale messaggio **MQTT su TLS**: il CN riceve un dato già leggibile e **non ha bisogno della AppSKey**. La AppSKey resta confinata all'edge (dove serve a decifrare) e al join server (dove viene generata); non viene mai propagata al CN. Le chiavi *master* (AppKey) non lasciano mai il join server (§3.2.10).

L'AS sul gateway diventa il publisher dei messaggi MQTT in up verso il server di gestione. L'AS sul gateway è anche il subscriober di eventuali messaggi MQTT in down dal server di gestione verso lo smart gate. 

### 6.4.4. Il rischio della scelta edge e come mitigarlo

Va dichiarato apertamente: **tenere l'application server all'edge è un compromesso, non una soluzione a costo zero.** Il prezzo della bassa latenza è che le **AppSKey** dei sensori risiedono fisicamente in un nodo a bordo strada, accessibile a chi abbia mezzi e determinazione. È un trade-off consapevole tra reattività (decisioni di sicurezza in millisecondi) ed esposizione delle chiavi di sessione. Per renderlo accettabile servono tre difese su piani diversi, da adottare **insieme**.

**1. Mutua autenticazione forte tra nodo edge e join/application server (mTLS).** Quando il join server distribuisce le AppSKey, deve consegnarle *solo* al nodo edge legittimo. Il canale è protetto da **mutual TLS**: sia il join server sia il nodo edge si presentano con un certificato X.509 emesso da una PKI interna. Un nodo edge clonato o sostituito da un attaccante non possiede un certificato valido, quindi **non riesce a farsi consegnare le chiavi** né a riconnettersi al backend. L'identità del nodo è inoltre **vincolata ai DevEUI di sua competenza**: un application server è autorizzato a ricevere le chiavi solo dei sensori del proprio km, non dell'intera rete (principio del minimo privilegio).

**2. Custodia delle chiavi in modulo anti-tampering (Secure Element / HSM all'edge).** L'autenticazione del canale non basta: bisogna proteggere le chiavi *una volta arrivate* sul nodo. Le AppSKey (e la chiave privata del certificato del nodo) vanno custodite in un **Secure Element** o **HSM** integrato nell'apparato edge — un chip che esegue le operazioni crittografiche al proprio interno e **non rivela mai le chiavi in chiaro**, nemmeno al microcontrollore che lo ospita. Aprire fisicamente il cabinet e leggere la memoria flash non basta più a estrarre le chiavi. I moduli anti-tampering di buona qualità **azzerano automaticamente** il contenuto (zeroization) al rilevamento di intrusione fisica (apertura dell'involucro, sbalzo di temperatura/tensione, tentativo di accesso ai bus interni).

**3. Attestazione dell'integrità prima della consegna delle chiavi.** È l'anello che lega le prime due: il join server consegna le AppSKey **solo dopo** che il nodo ha dimostrato di essere integro e non manomesso. Il nodo edge esegue un **secure boot** (avvia solo firmware firmato) e fornisce una **attestazione** basata su TPM/HSM del proprio stato; se l'attestazione fallisce — perché il firmware è stato alterato o il cabinet manomesso — il join server **rifiuta di inviare le chiavi**. Così la mutua autenticazione (chi sei) si combina con l'anti-tampering (sei integro) prima che qualunque segreto scenda all'edge.

**Reti di sicurezza a contorno**, già previste altrove nel progetto:

- **Tamper detection del cabinet** (§5.2): l'apertura genera un allarme immediato al SOC, che **revoca** centralmente certificati e sessioni del nodo. L'attaccante resta con chiavi morte.
- **Rinnovo delle sessioni** (rejoin): le AppSKey eventualmente esfiltrate scadono al rinnovo; un `ForceRejoinReq` in downlink può invalidarle subito, senza mai trasmettere chiavi (la AppKey master non scende mai — §3.2.10).
- **Contenimento del blast radius**: la compromissione di un nodo espone solo i sensori di *quel* km, e solo per la durata della sessione corrente; non si propaga né nello spazio né nel tempo.

In sintesi: la scelta edge resta valida **a condizione** che il nodo edge sia trattato come un dispositivo di sicurezza a sé — autenticato mutuamente, con chiavi in modulo anti-tampering, e con consegna delle chiavi subordinata all'attestazione di integrità. Senza queste difese, l'application server all'edge sarebbe effettivamente un punto debole; con esse, il rischio residuo è circoscritto e gestibile.


## 6.5. Join server - colui che distribuisce le chiavi di sessione

### 6.5.1.  Gestione della sicurezza della rete LoRaWAN

Ogni sensore si registra al network server tramite **Over-the-Air Activation (OTAA)**. In fabbrica il sensore viene programmato con:

- **DevEUI**: identificatore univoco del dispositivo (derivato dal MAC via EUI64).
- **AppEUI**: identificatore dell'applicazione di destinazione (è un parametro del network server).
- **AppKey**: chiave segreta pre-condivisa a 128 bit, scambiata su canale sicuro tra dispositivo e join server.

Al primo join, il join server usa la AppKey per generare e distribuire due chiavi di sessione:

- **AppSKey**: utilizzata per cifrare con AES il payload applicativo. Garantisce la **confidenzialità** dei dati: solo l'application server (che possiede la stessa chiave) può decifrare. Viene distribuita all'**application server edge** competente per quel sensore.
- **NwkSKey**: utilizzata come chiave in ingresso all'algoritmo **AES-CMAC** per calcolare il **MIC (Message Integrity Code)** di ogni frame. Il sensore allega il MIC al messaggio; il network server ricalcola localmente il MIC con la stessa chiave e verifica la coincidenza. Se i due MIC coincidono, sono provate **simultaneamente l'integrità del messaggio e l'autenticazione del mittente**.

Questo meccanismo è anche un caso applicativo concreto delle **funzioni hash crittografiche** (quesito 4 della seconda parte): AES-CMAC è una funzione di tipo HMAC che produce un'impronta non falsificabile senza conoscere la chiave.


### 6.5.2.  Join Server e ridondanza

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

### 6.5.3. Riassunto dei vantaggi della scelta

- **Zero scavi lungo il km**: nessun cavo di alimentazione né di dati per i sensori. Costo di posa fortemente abbattuto rispetto a soluzioni cablate.
- **Installazione e manutenzione modulare**: ogni sensore è una scatoletta indipendente fissata al guard-rail in mezz'ora.
- **Energia autonoma a tempo indeterminato**: batteria + microfotovoltaico → nessuna manutenzione energetica per anni.
- **Ridondanza gratuita**: sensori al confine tra due km sono ricevibili da entrambi i gateway adiacenti, il network server deduplica.
- **Decisioni di sicurezza in tempo reale**: network server locale a ogni smart-gate (architettura edge) → latenza decisionale di millisecondi, autonomia in modalità degraded.
- **Custodia centralizzata e sicura delle chiavi master**: join server ridondato active-active con HSM in due data-center separati.
- **Sicurezza forte end-to-end**: cifratura del payload (AppSKey), autenticazione e integrità tramite MIC (NwkSKey), OTAA per il provisioning sicuro delle chiavi di sessione.
- **Trade-off edge gestito**: l'application server all'edge espone le AppSKey a bordo strada; il rischio è mitigato con mutua autenticazione (mTLS), custodia delle chiavi in modulo anti-tampering (Secure Element/HSM) e attestazione di integrità prima della consegna delle chiavi (§3.2.7).

## 6.6 Server di gestione - Colui che elabora i payload

Non è un dispositivo della gerarchia LoRaWAN, non deve implementare lo stack protocollare LoRaWAN ma deve semplicemente ricevere i payload che l'AS spedisce e inoltra via MQTT.

### 6.7. Comunicazione smart-gate ↔ CdC

Questa è la tratta più delicata: deve essere ad alta banda (per gli stream video on-demand), bassa latenza, sempre disponibile.

- **Fisicamente**: fibra ottica monomodale lungo l'autostrada, con topologia ad **anello** per la resilienza. La scelta dell'apparato attivo che chiude l'anello a ogni km è discussa in dettaglio nella sezione  [dettaglio_spillamento_fibra.md](./dettaglio_spillamento_fibra.md).
- **Logicamente**: link Ethernet/IP. Sopra IP si appoggiano:
  - **MQTT su TLS** per la messaggistica asincrona di telemetria, comandi e stato (publish/subscribe verso il broker del CdC).
  - **HTTPS / REST** per gli aggiornamenti di configurazione e per il push di nuove segnaletiche.
  - **RTSP/SRT** per gli stream video on-demand (solo quando l'operatore richiede la visione live).
- **Backup**: connessione **5G/4G LTE** con APN privato della società autostradale, attivata automaticamente da BGP/SD-WAN in caso di failure della fibra.

---


## 7.2. Modello dei topic MQTT per uno smart-gate

Si può definire una gerarchia di topic come segue. Sia `<RR>` la regione, `<TT>` il tratto, `<NNN>` l'identificatore numerico dello smart-gate (es. `LO/01/042` = Lombardia, tratto 1, smart-gate 42):

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

- **`misure/*`**: pubblicati dall'**application server edge** dopo la decodifica del payload LoRaWAN (Cayenne LPP → JSON, vedi §3.2.7); i subscriber sono la dashboard dell'operatore al CdC e il bridge verso il CN. Telemetria periodica (cadenza tipica: meteo ogni 30 s, aria ogni 60 s, fondo ogni 10 s).
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
## 7.3. Dorsali della rete di gestione

### 7.3.1. Comunicazione CdC ↔ CN

- L3: **Connessione primaria**: rete **MPLS L3VPN** fornita da un operatore telco. Garantisce SLA contrattuali, classi di servizio (QoS) e isolamento.
- L3: **Connessione di backup**: tunnel **VPN IPsec site-to-site** su Internet pubblica, da firewall a firewall.
- L7: **Canale applicativo**:
  - **MQTT bridge** (è il pattern classico): il broker del CdC fa da bridge verso il broker del CN, replicando solo i topic di interesse nazionale (es. dati aggregati di traffico, prenotazioni ricariche, allarmi, eventi targhe).
  - **HTTPS/REST** per le chiamate sincrone (es. recupero di dati storici, push di configurazioni globali dal CN ai CdC).
  - **gRPC** in alternativa al REST quando occorre throughput più alto e contratti tipizzati (Protocol Buffers).

### 7.3.2. Comunicazione APP utenti ↔ CN

- L7: Canale applicativo **HTTPS/REST** (versionato `/v1/...`) per le chiamate stateless del client.
- L7: Canale applicativo **WebSocket Secure (WSS)** per il push real-time delle segnaletiche e dello stato dei punti di ricarica.
- L7: In alternativa, **MQTT over WebSocket Secure** se si vuole riusare l'infrastruttura broker (il client APP si registra come subscriber su topic di pubblico interesse).
- Autenticazione utenti con **OAuth 2.0 + OpenID Connect** per le funzioni che richiedono profilazione (prenotazione ricarica). Le funzioni di sola lettura della segnaletica sono accessibili in modo anonimo.

### 7.3.3. Comunicazione stazioni di ricarica ↔ rete

Le stazioni di ricarica utilizzano:
- **Fisicamente**: fibra ottica monomodale lungo l'autostrada, con topologia ad **anello** per la resilienza. La scelta dell'apparato attivo che chiude l'anello a ogni km è discussa in dettaglio nella sezione  [dettaglio_spillamento_fibra.md](./dettaglio_spillamento_fibra.md).
- **Logicamente**: link Ethernet/IP. Sopra IP si appoggiano:
     - a L7 lo standard applicativo  **OCPP (Open Charge Point Protocol) 1.6 o 2.0.1** su WebSocket Secure verso un CSMS (Charging Station Management System) che, nel nostro progetto, è un microservizio del CN. Questo dà accesso a:

- stato in tempo reale di ogni punto (libero, occupato, in errore);
- avvio/interruzione remota di una sessione di ricarica;
- prenotazioni (OCPP supporta nativamente i messaggi `ReserveNow` e `CancelReservation`).

---

# 8. Piano di indirizzamento completo — IPv6 GUA (senza NAT) e IPv4 (con NAT)

Si definiscono **due scenari** di indirizzamento, mappati in modo netto sulla tecnologia, perché è la scarsità (o l'abbondanza) di indirizzi a dettare la scelta:

| Scenario        | Tecnologia            | NAT  | Raggiungibilità                     |
| --------------- | --------------------- | ---- | ----------------------------------- |
| **Senza NAT**   | **IPv6 GUA**          | no   | end-to-end (ogni dispositivo ha un indirizzo globale, entro firewall) |
| **Con NAT**     | **IPv4 RFC1918** (`10/8`) | sì   | dispositivi dietro NAT al gateway di tratto |

In IPv4 la penuria di indirizzi **impone** il NAT; in IPv6 l'abbondanza di indirizzi globali (GUA) rende il NAT inutile e dannoso, e abilita l'end-to-end.

I due scenari **condividono la gerarchia**: i campi alti codificano **regione (5 bit) + funzione (3 bit)**; cambia solo come si numerano **tratto** e **host** sotto di essi. In entrambi vale il confine di §5.4–5.8: l'L2 resta in regione, il **router regionale** è il primo hop L3 e gateway dei quattro piani (telecamere, sensori, controller, management).

---

## 8.1 Scenario SENZA NAT — IPv6 GUA (soluzione di riferimento)

### 8.1.1 Gerarchia degli indirizzi

Si parte da un blocco **GUA** assegnato dall'RIR (RIPE) all'operatore — tipicamente un **`/32`** (LIR) da cui si ritaglia un **`/40`** per la rete Smart-Road, lasciando 8 bit di slack superiore per il livello nazionale / altri usi. La ripartizione dei 24 bit dal `/40` al `/64`:

```
GUA  ::  [ /40 rete ]  RRRRR fff   TTTTTTTT TTTTTTTT   IID (64 bit)
                       └─5─┘ └3┘   └────── 16 ──────┘
                       reg.  fun.        tratto          host (SLAAC)
                       └──── /45 ─┘
                       └──────── /48 ────┘
                       └─────────────────── /64 ──────┘
```

- **Regione**: 5 bit → **`/45` per regione** (32 regioni: 20 + riserva).
- **Funzione**: 3 bit → **`/48` per funzione** (8 blocchi: 4 usati + 4 di riserva). Il `/48` è l'unità "sito" standard IPv6.
- **Tratto**: 16 bit → **`/64` per tratto** (65.536 tratti per funzione/regione).
- **Host**: i 64 bit di Interface-ID, via **SLAAC** → host praticamente illimitati per tratto.

I confini cadono esatti sugli **hextet**, in perfetto parallelo con l'IPv4: il 3º hextet = regione + funzione, il 4º hextet = tratto. Si legge `<GUA>:<reg×8 + funzione>:<tratto>::/64`.

### 8.1.2 Esempio — Lombardia (regione = 3)

Usando il prefisso di documentazione `2001:db8::/32` (il `/40` operativo è `2001:db8:00::/40`); regione 3 → `reg×8 = 24 = 0x18`, quindi **`/45` regionale `2001:db8:18::/45`**:

| Funzione           | Blocco `/48`          | Tratto 5 (`/64`)          |
| ------------------ | --------------------- | ------------------------- |
| Telecamere (fn 0)  | `2001:db8:18::/48`    | `2001:db8:18:5::/64`      |
| Sensori (fn 1)     | `2001:db8:19::/48`    | `2001:db8:19:5::/64`      |
| Controller (fn 2)  | `2001:db8:1a::/48`    | `2001:db8:1a:5::/64`      |
| Management (fn 3)  | `2001:db8:1b::/48`    | `2001:db8:1b:5::/64`      |
| Riserva (fn 4–7)   | `2001:db8:1c–1f::/48` | —                         |

Esempio di indirizzo: una telecamera nel tratto 5 della Lombardia → `2001:db8:18:5::a/64`.

### 8.1.3 Proprietà

- **End-to-end, niente NAT**: RTSP, SSH e push comandi sono nativi; la sicurezza si fa con **firewall/policy stateful esplicite** (non per effetto collaterale del NAT).
- **Aggregazione pulita**: un `/45` per regione, un `/48` per funzione annunciati a nord.
- **Scala**: 65.536 tratti per funzione/regione, host illimitati.
- *(Alternativa)* per la parte puramente interna si può affiancare **ULA `fc00::/7`** alla GUA, ma per un servizio che deve essere raggiungibile (push comandi, visione immagini) la **GUA** resta l'asse portante.

---

## 8.2 Scenario CON NAT — IPv4 RFC1918 (`10.0.0.0/8`)

### 8.2.1 Gerarchia base (regione `/13`, funzione `/16`)

Spazio `10.0.0.0/8`; i 24 bit sotto il `/8`:

```
10 . [ RRRRR fff ] . [ TTTTTTTT ] . [ HHHHHHHH ]
     └──5─┘ └─3──┘   └────8─────┘   └────8─────┘
      reg.  funz.        tratto          host
      └──── /13 ───┘
      └──────── /16 ─────┘
```

- **Regione**: 5 bit → **`/13` per regione** (32 regioni: 20 + riserva).
- **Funzione**: 3 bit → **`/16` per funzione** (8 blocchi: 4 usati + 4 riserva).

Si legge `10.<reg×8 + funzione>.<tratto>.<host>`. **Esempio Lombardia (reg = 3)** → `/13` regionale `10.24.0.0/13`:

| Funzione           | Blocco `/16`     |
| ------------------ | ---------------- |
| Telecamere         | `10.24.0.0/16`   |
| Sensori            | `10.25.0.0/16`   |
| Controller         | `10.26.0.0/16`   |
| Management         | `10.27.0.0/16`   |
| Riserva            | `10.28–31.0.0/16`|

Cautele: **riservare uno slot `/13` al Centro Nazionale** (es. `10.0.0.0/13` = CN, shared-services, transit, loopback); il `10/8` è interamente consumato → attenzione agli **overlap** con MPLS del telco / cloud.

### 8.2.2 Dimensionamento di tratto e host: due varianti

**Variante (a) — octet-aligned, senza NAT per tratto** (fino a 256 tratti): tratto = ottetto 3, host = ottetto 4 → **`/24` per tratto**, 256 tratti per funzione/regione (512 se si scende a 2 bit di funzione, rinunciando alla riserva). Indirizzamento privato end-to-end, NAT solo al confine Internet.

**Variante (b) — con NAT al gateway di tratto** (densità elevata): si ridisegnano i 16 bit bassi come **13 bit di tratto + 3 bit di host**:

```
10 . [ RRRRR fff ] . [ TTTTTTTT ] . [TTTTT   hhh  ]
      └─5─┘ └─3─┘    └─── tratto 13 ──────┘└host 3┘
      reg.  funz.        (fino a 8192 tratti)       /29: 8 IP, 4 esposti
```

Il **gateway di tratto fa NAT** ed espone solo **4 IPv4 di un pool `/29`**; i dispositivi interni vivono in spazio privato **riutilizzato a ogni tratto** (`192.168.x/29`, vedi §11.2). Footprint per tratto minuscolo → **fino a 8192 tratti** per funzione/regione.

Lo smart-gate ha al suo interno una piccola LAN con VLAN dedicate per separare i sottosistemi (segmentazione di sicurezza):

| VLAN | Subnet | Uso |
|------|--------|-----|
| 10 | `192.168.10.0/29` | Telecamere IP (sottorete isolata, non raggiungibile dall'esterno) |
| 20 | `192.168.20.0/29` | Sensori IP-based |
| 30 | `192.168.30.0/29` | Controller PMV (Pannello a Messaggio Variabile) |
| 99 | `192.168.99.0/29` | Management (SSH, SNMP) |

Il SoC centrale fa da gateway tra queste VLAN e l'uplink verso il CdC.

### 8.2.3 Proprietà / trade-off del NAT

- **MQTT**: i nodi edge *chiamano fuori* verso il broker → il NAT non disturba (telemetria, eventi, comandi-subscribe ok).
- **RTSP in pull / SSH (in ingresso)**: con 4 IP esposti per tratto, gli altri dispositivi richiedono **port-mapping** → scomodo su scala.
- Uso: **conservazione IPv4** e **dispositivi legacy** non IPv6-ready.

---

## 8.3 Confronto e raccomandazione

| Aspetto                       | Senza NAT — IPv6 GUA                | Con NAT — IPv4 RFC1918                  |
| ----------------------------- | ---------------------------------- | --------------------------------------- |
| Unità per funzione/regione    | `/48` (→ `/64` per tratto)         | `/16` (→ `/24` o `/29` per tratto)      |
| Tratti per funzione/regione   | **65.536**                         | 256 / 512 (var. a) · 8192 (var. b, NAT) |
| Host per tratto               | illimitati (SLAAC)                 | 256 (`/24`) · 4 esposti (`/29` + NAT)   |
| Raggiungibilità dispositivi   | diretta (entro firewall)           | solo via NAT / port-map                 |
| RTSP / SSH in ingresso        | nativi                             | richiedono port-mapping                 |
| MQTT (dial-out)               | ok                                 | ok                                      |
| NAT                           | nessuno                            | al gateway di tratto                     |
| Uso                           | **soluzione di riferimento**       | conservazione IPv4 / legacy             |

**Raccomandazione:** **IPv6 GUA senza NAT** come piano di riferimento; **IPv4 RFC1918 con NAT** mantenuto in **dual-stack** per le isole di dispositivi legacy non IPv6-ready, fino alla loro dismissione.

## 8.4 Elementi comuni ai due scenari

- **L2 in regione, router regionale primo hop L3** (§5.4–5.6): verso il nazionale solo rotte L3 sommarizzate (un `/45`÷`/48` IPv6 o un `/13`÷`/16` IPv4 per regione/funzione).
- **Contenimento del broadcast**: Private VLAN + storm control + IGMP snooping sulla subnet di funzione (§8 precedente).
- **Maschere alle foglie**: in IPv4 `/29` interni allo smart-gate, `/31`–`/30` sui link punto-punto, `/32` loopback; in IPv6 `/64` ovunque.
- **Accesso dal CN** (§5.7): segnalazioni via **MQTT bridge** (pub/sub L7, nessuna rotta L3 ai dispositivi), immagini via **relay media regionale** on-demand attraverso il firewall.


---

# 9. Routing e NAT

## 12.1. Routing e NAT
- **Routing dinamico interno**: protocollo **OSPF** sulle aree regionali, con area 0 (backbone) tra i CdC e il CN.
- **Routing tra CdC e CN su MPLS**: **BGP** verso il PE dell'operatore telco (BGP per le route private nella VPN MPLS).
- **VPN IPsec di backup**: route statiche oppure BGP-over-IPsec.

---

## 9.2. NAT e indirizzi pubblici

Solo i servizi rivolti agli utenti dell'APP hanno indirizzi pubblici. Si usa un piccolo blocco IPv4 (es. `203.0.113.0/29`) e/o IPv6 nativo, dietro load balancer di frontiera con WAF (Web Application Firewall).

---

# 10. Continuità di servizio e sicurezza


## 10.1 Continuità di servizio (alta affidabilità)

| Livello | Tecniche adottate |
|---------|-------------------|
| Smart-gate | Doppia alimentazione (rete + UPS + fotovoltaico); doppia connettività (fibra + 5G); **modalità degraded autonoma** in caso di isolamento dal CdC |
| Rete tratto | Topologia ad **anello ottico** con ERPS (failover < 50 ms); switch industriali con doppia alimentazione |
| CdC | Cluster Kubernetes con 3+ nodi, repliche dei pod su nodi diversi; database in HA con streaming replication; UPS + gruppo elettrogeno |
| WAN CdC↔CN | Doppia uscita MPLS + Internet con SD-WAN; BFD per failover rapido |
| CN | **Due data-center attivo-attivo** geograficamente separati; bilanciamento DNS globale (GSLB); replica sincrona del DB sulla coppia primaria di sito, asincrona verso siti DR |
| Dati | Backup giornalieri + replica geografica; piano di Disaster Recovery con RTO < 4 h e RPO < 15 min |
| Servizi APP | CDN davanti all'API gateway; rate limiting per resistere a spike di traffico |

### 10.2 Sicurezza

La sicurezza è organizzata per **strati** (defense in depth):

**Sicurezza fisica**: gli smart-gate sono in armadi blindati con sensori di apertura (tamper switch) che generano allarmi.

**Sicurezza di rete**:

- Tutto il traffico inter-sito viaggia in **MPLS VPN** o **IPsec**.
- Firewall stateful al perimetro di ogni CdC e del CN; IDS/IPS con regole signature-based + analisi comportamentale.
- Segmentazione **micro** tramite VLAN e ACL: gli smart-gate non possono parlare tra loro, ma solo col CdC.
- **NAC (Network Access Control)** con 802.1X sugli switch del CdC per autenticare ogni dispositivo che si collega.
- si abilita 802.1X sulle porte di accesso dello switch dello smart-gate per **proteggere la rete** dagli **accessi rogue** (abusivi):
     - EAP-TLS dove i dispositivi lo supportano (riusando la PKI di 802.1X),
     - MAB+profiling per i dispositivi legacy
     - RADIUS locale o critical-auth per non rompere la modalità degraded. È un rafforzamento che si aggiunge a tamper detection e mutual-TLS, non li sostituisce.

**Sicurezza applicativa**:

- Tutto in **TLS 1.3** (HTTPS, MQTTS, WSS).
- Autenticazione dei dispositivi smart-gate verso il broker MQTT con **certificati X.509** (mutual TLS). Ogni smart-gate ha il proprio certificato emesso da una PKI interna; il certificato viene installato in fabbrica e ruotato annualmente. Serve a **proteggere il broker MQTT** da accessi abusivi.
- Autenticazione degli utenti dell'APP con **OAuth 2.0 + OIDC** e MFA per le funzioni che muovono denaro (prenotazione e pagamento ricarica).
- Le password sugli account utente sono salvate con **bcrypt/argon2** (mai in chiaro, mai con hash veloci come MD5/SHA1).

**Il perchè dell'autenticazione multilivello**

L'autenticazione 802.1X e mTLS (Mutual TLS) sembrano alternativi perché usano lo stesso tipo di credenziale (certificato X.509) e magari la stessa PKI, ma autenticano cose diverse, a livelli diversi, verso interlocutori diversi. Sono complementari, non sostitutivi. Uno **presenta** il certificato **al RADIUS** per entrare in rete, l'altro lo presenta **al broker** per aprire la sessione applicativa:

- **Con solo mTLS**: un apparato rogue infilato nel cabinet non riuscirebbe a fingersi il client MQTT, ma sarebbe comunque sulla LAN a scansionare, attaccare le telecamere o il controller PMV (che magari non parlano mTLS), fare ARP spoofing. L'802.1X glielo impedisce a monte.

- **Con solo 802.1X**: un dispositivo che ha superato l'ammissione (o che ha spoofato il MAC in MAB) potrebbe provare a connettersi al broker; senza mTLS il broker non potrebbe verificarne l'identità applicativa né il traffico sarebbe **cifrato end-to-end** attraverso la WAN, dove passa per nodi intermedi (firewall, dorsale, broker che inoltra).

Un'**analogia**: l'802.1X è il tornello con badge all'ingresso dell'edificio; il mTLS è il mostrare il documento alla persona specifica con cui poi parli, in una stanza insonorizzata. Stesso documento d'identità, due controlli in due punti diversi.

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

# 11. Quesito 1 - Database prenotazioni ricarica (modello logico)

Si modella la porzione del database nazionale dedicata alle **stazioni di ricarica e alle prenotazioni** per veicoli elettrici. Le altre porzioni (storico segnaletiche, telemetria, utenti) sono lasciate fuori da questo schema per coerenza con il quesito.

## 11.1 Analisi dei requisiti

Dal testo della traccia:

- Le stazioni di ricarica sono **dislocate sulla rete autostradale**.
- Ogni stazione ha **più punti di ricarica**.
- Si deve gestire lo **stato in tempo reale** dei punti (libero, occupato, guasto, in manutenzione).
- Si devono gestire **prenotazioni** sulla base dell'**orario di arrivo** stimato e della **durata** stimata.
- Le prenotazioni sono fatte da utenti dell'APP.

## 11.2 Modello concettuale (ER)

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

## 11.3 Schema logico relazionale

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

### 11.3.1 Vincoli di integrità rilevanti

- Una **prenotazione** non può sovrapporsi a un'altra sulla stessa coppia (id_punto, intervallo_temporale). Vincolo applicativo o tramite **exclusion constraint** (in PostgreSQL `EXCLUDE USING gist`).
- Il tipo di connettore della prenotazione deve essere compatibile col veicolo: vincolo da implementare a livello applicativo o con trigger.
- `stato` di PRENOTAZIONE ∈ {attiva, in_corso, completata, annullata, no_show}.
- `stato_corrente` di PUNTO_RICARICA ∈ {libero, occupato, guasto, manutenzione, prenotato}.

### 11.3.2 Query di esempio

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

### 11.3.3 Considerazioni di scalabilità

- Le tabelle ad alta frequenza di scrittura (PUNTO_RICARICA.stato_corrente) andrebbero affiancate da una cache in-memory (Redis) per servire le query di stato all'APP con latenza < 100 ms.
- Le sessioni di ricarica chiuse possono essere migrate su una tabella storica partizionata per anno/mese per non appesantire la tabella attiva.

---

# 12. Quesito 2 - Protocollo applicativo per l'APP guidatori

## 12.1 Scelta della tecnologia

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

## 12.2 Architettura della comunicazione APP↔CN

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

## 12.3 Specifica del protocollo applicativo REST

Convenzioni:

- Base URL: `https://api.smart-road.example/v1`
- Formato payload: **JSON** (UTF-8).
- Autenticazione: header `Authorization: Bearer <jwt_access_token>`, ottenuto via OAuth 2.0 (flow Authorization Code with PKCE per app mobile). Le risorse di sola lettura della segnaletica sono accessibili anche **senza autenticazione** (dato pubblico).
- Codici di stato HTTP standard (`200 OK`, `201 Created`, `400 Bad Request`, `401 Unauthorized`, `404 Not Found`, `409 Conflict`, `5xx`).
- Paginazione tramite `?page=1&size=20` con header `X-Total-Count`.

#### 12.3.1 Risorse principali

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

#### 16.3.2 Esempio: lettura segnaletica corrente

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

#### 12.3.3 Esempio: creazione prenotazione

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

## 12.4 Specifica del canale push (MQTT)

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

### 12.4.1 Sicurezza del protocollo

- Tutti gli scambi sono in **TLS 1.3** con certificati firmati da una CA pubblica (server) e con possibile **pinning** lato APP per resistere ad attacchi MITM con CA compromesse.
- I JWT hanno scadenza breve (15 min) e sono rinnovati tramite **refresh token** (rotazione del refresh token a ogni uso).
- **Rate limiting** per IP e per token sull'API Gateway, contro abusi e attacchi DoS.
- Validazione lato server di tutti gli input (lunghezza, formato, range), per prevenire injection.
- I dati di prenotazione restano nel DB nazionale; l'APP non memorizza dati sensibili (targhe, ecc.) oltre la sessione corrente.

### 12.4.2 Vantaggi della soluzione proposta

- **REST/HTTPS** è universalmente supportato, semplice da debuggare (curl, Postman), facile da cachare lato CDN per i dati pubblici (segnaletica, stazioni).
- **MQTT/WSS** dà push real-time efficiente in termini di batteria e banda (connessione persistente, payload compatti), passa attraverso firewall e proxy aziendali (porta 443).
- Il riuso del broker MQTT già presente nell'infrastruttura riduce costi e complessità.
- Versionamento dell'API (`/v1`, `/v2`) consente evoluzioni senza rompere i client già distribuiti.



