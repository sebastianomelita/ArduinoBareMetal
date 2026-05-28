>[Torna a svolgimento Smart Road](svolgimento_smart_road.md)>[Torna a Dettaglio architettura LoRaWAN](/lorawanclasses.md) 
>

# Dettaglio — Spillamento della fibra lungo il tratto autostradale

> Documento di dettaglio collegato allo svolgimento principale (sezione §3.3).
> Approfondisce come la fibra ottica viene "spillata" a ogni km per servire gli smart-gate.

---

## Indice

- [1. Il problema](#1-il-problema)
- [2. Opzione A — Anello Ethernet attivo con switch L2](#2-opzione-a--anello-ethernet-attivo-con-switch-l2)
- [3. Opzione B — Anello IP attivo con router L3](#3-opzione-b--anello-ip-attivo-con-router-l3)
- [4. Opzione C — PON (Passive Optical Network)](#4-opzione-c--pon-passive-optical-network)
- [5. Confronto e scelta](#5-confronto-e-scelta)
- [6. Componenti dello switch a ogni smart-gate](#6-componenti-dello-switch-a-ogni-smart-gate)
- [7. Tracciato fisico della fibra](#7-tracciato-fisico-della-fibra)

---

## 1. Il problema

Come si "spilla" la fibra lungo le decine di km del tratto autostradale per servire ogni smart-gate? Ci sono **tre tecnologie** che competono per questo ruolo, con equilibri costi/benefici diversi. La differenza concettuale chiave è tra:

- **Rigenerazione attiva**: a ogni km c'è un apparato (switch o router) che riceve il segnale ottico, lo converte in elettrico, lo rigenera e lo ritrasmette. Il segnale "riparte nuovo" a ogni nodo.
- **Spillamento passivo**: a ogni derivazione c'è uno splitter ottico (un prisma in fibra) che divide la potenza luminosa senza alcuna elettronica né alimentazione. Tutti ricevono lo stesso segnale e si selezionano logicamente.

![Anello in fibra ottica con switch ERPS a ogni km](../img/anello_fibra_erps.svg)

---

## 2. Opzione A — Anello Ethernet attivo con switch L2

È la scelta più diffusa per le reti di campo lungo infrastrutture lineari (autostrade, ferrovie, oleodotti). In ogni smart-gate è installato uno **switch managed industriale** con almeno **2 porte ottiche** (gli "uplink" verso i due vicini sull'anello) e alcune porte Ethernet di accesso per i dispositivi interni allo smart-gate (SoC, gateway LoRaWAN, telecamere via PoE+, controller del maxi-schermo).

**Come avviene lo "spillamento" fisicamente.** Lo switch industriale **non è un derivatore passivo**: è un dispositivo attivo che riceve il segnale ottico su una porta, lo converte in elettrico, lo elabora (forwarding L2 in base agli indirizzi MAC), lo rigenera e lo trasmette sulla porta di uscita. È a tutti gli effetti una **rigenerazione 3R** (re-amplification, re-shaping, re-timing). Questo significa che:

- Il segnale viene **rigenerato** a ogni nodo: a 1 km di distanza tra nodi adiacenti la rigenerazione è ampiamente sovrabbondante (la fibra monomodale arriva tranquillamente a 40-80 km senza rigenerazione).
- Ogni switch è un **dominio di broadcast separato**: gli switch usano i propri MAC address per scambiarsi i frame del protocollo di controllo dell'anello.
- Il traffico verso il CdC attraversa N switch prima di arrivare a destinazione: ogni switch aggiunge tipicamente decine di microsecondi di latenza di store-and-forward. Su un tratto di 50 km, parliamo di ~1-2 ms aggiuntivi sul tempo di volo della luce in fibra (250 µs). Trascurabile.

**Chiusura dell'anello.** L'anello si chiude in due modi possibili:

- **Singolo cavo ottico** con due fibre fisicamente separate: una "va" e una "torna", entrambe nello stesso cavo. È economico ma in caso di taglio del cavo (escavatore, incidente, frana) **entrambi i percorsi vengono persi simultaneamente** e l'anello si spezza.
- **Doppio cavo ottico in percorsi geograficamente diversi**: la fibra dell'andata corre sul guard-rail di destra, quella del ritorno sul guard-rail di sinistra (o nella canalina opposta). Costa di più, ma garantisce sopravvivenza a un singolo guasto fisico. È la scelta corretta per infrastruttura critica come una smart-road.

**Protocollo di failover: ERPS (IEEE G.8032).** Sull'anello gira il protocollo **Ethernet Ring Protection Switching**, che in condizioni normali tiene **bloccata** una delle due tratte (il "Ring Protection Link") per evitare loop di broadcast. Quando un nodo rileva un guasto:

1. Invia un messaggio R-APS (Ring-APS) di tipo "Signal Fail" ai vicini.
2. Il nodo che teneva bloccato il RPL lo sblocca.
3. Tutti i nodi flushano la loro tabella MAC e la riapprendono sul nuovo percorso.
4. Tempo totale di failover: **< 50 ms**, conforme ai requisiti delle reti carrier-grade.

Per il nostro progetto è essenziale: 50 ms è impercettibile sia per la telemetria MQTT che per gli stream video.

---

## 3. Opzione B — Anello IP attivo con router L3

Variante della precedente, ma a ogni km c'è un **router** invece di uno switch L2. La differenza pratica:

- **Pro**: ogni km è un dominio L3 separato; routing dinamico (OSPF) gestisce automaticamente il failover; segmentazione del traffico migliore; isolamento dei domini di guasto.
- **Contro**: hardware più costoso (router industriali costano significativamente più di switch industriali); convergenza OSPF tipicamente più lenta di ERPS (1-3 secondi anche con timer aggressivi vs <50 ms di ERPS); più configurazione per nodo.

**Quando ha senso usarla?** Solo se il tratto è molto lungo (oltre 100 km con molti km tra nodi), se serve coesistenza con altre reti IP esterne, o se la gestione del traffico richiede policy L3 fine (ACL, QoS DSCP). Per il nostro progetto sperimentale (tratti di ~50 km) **lo switch L2 con ERPS è più adatto**: failover molto più rapido e gestione più semplice.

---

## 4. Opzione C — PON (Passive Optical Network)

Tecnologia radicalmente diversa: a ogni km **non c'è apparato attivo**. La fibra che esce dal CdC arriva a uno **splitter ottico passivo** (un dispositivo che divide la potenza luminosa in N rami, senza alimentazione) collocato in un pozzetto a bordo strada. Ogni ramo dello splitter raggiunge un **ONT (Optical Network Terminal)** dentro uno smart-gate. Lo schema logico è quello di una **rete punto-multipunto** in cui un singolo apparato attivo al CdC (l'**OLT - Optical Line Terminal**) parla con N ONT.

```
   CdC                Splitter passivo
   ┌────┐    fibra    ┌──────────┐    fibra
   │OLT │═══════════► │ 1:32     │═══►ONT km 1 ─► smart-gate
   └────┘             │ splitter │═══►ONT km 2 ─► smart-gate
                      │          │═══►ONT km 3 ─► smart-gate
                      │          │═══►...
                      └──────────┘
```

**Come avviene lo "spillamento" in PON.** Lo splitter è letteralmente un **prisma in fibra ottica**: divide la potenza luminosa in ingresso su N uscite (tipicamente 1:32 o 1:64), senza alimentazione né elettronica. Tutti gli ONT ricevono **tutto il traffico downstream** (è broadcast fisico), e ciascuno **estrae solo i frame destinati al proprio identificativo** (Logical Link Identifier). In upstream, gli ONT trasmettono in TDMA coordinato dall'OLT per evitare collisioni.

**Caratteristiche.**

- **Pro**: zero apparato attivo a bordo strada, quindi zero alimentazione necessaria nei pozzetti intermedi, zero manutenzione di switch/router intermedi, zero punti di guasto attivi. Tecnologia che le telco usano in massa per l'FTTH residenziale.
- **Contro**: niente ridondanza ad anello (è una topologia stella ottica passiva); se si taglia la fibra principale tra OLT e splitter, **tutti gli smart-gate dietro lo splitter perdono connettività**; banda condivisa tra tutti gli ONT (in GPON tipicamente 2.5 Gbps downstream divisa tra 32 ONT = ~80 Mbps medio, in XGS-PON 10 Gbps); cifratura del traffico downstream **obbligatoria** perché tutti vedono tutto.

**Quando ha senso usarla?** Quando il vincolo principale è il **costo di manutenzione** e si accetta una resilienza minore. In autostrada, dove la fibra è soggetta a tagli accidentali, la mancanza di ridondanza ad anello è uno svantaggio rilevante. **Per il nostro progetto la scartiamo** in favore dell'anello attivo.

---

## 5. Confronto e scelta

| Caratteristica | A — Anello L2 ERPS | B — Anello L3 OSPF | C — PON |
|----------------|-------------------|--------------------|---------| 
| Apparato a ogni km | Switch industriale | Router industriale | Solo ONT passivo |
| Splitter intermedi | No (rigenerazione attiva) | No (rigenerazione attiva) | Sì (1:32 o 1:64 passivo) |
| Resilienza | ✅ Anello, failover <50 ms | ✅ Anello, failover 1-3 s | ❌ Stella, nessuna ridondanza |
| Banda per smart-gate | 1-10 Gbps dedicati | 1-10 Gbps dedicati | ~80-300 Mbps condivisi |
| Costo CAPEX a km | Medio | Alto | Basso |
| Costo OPEX (manutenzione) | Medio | Medio | Basso |
| Alimentazione intermedia | Sì (poco) | Sì (poco) | No |
| Scelta per il progetto | ✅ **Adottata** | ❌ Esagerata | ❌ Insufficiente resilienza |

**Scelta: anello Ethernet L2 con ERPS.** Motivazioni:

1. **Failover sotto i 50 ms** è essenziale per garantire che gli stream video on-demand e la telemetria MQTT non vedano interruzioni percepibili anche durante un guasto.
2. **Banda dedicata** per smart-gate (~1 Gbps) è ampiamente sufficiente per i flussi video FullHD on-demand e per la telemetria.
3. **Switch L2 industriali sono prodotti maturi e standardizzati** (esempi: Hirschmann, Moxa, Cisco IE series, Westermo). Costo accettabile, MTBF molto alto (decine di migliaia di ore).
4. **L'alimentazione dello switch piggy-backa su quella già presente nello smart-gate**: lo switch sta dentro il cabinet del maxi-schermo, alimentato dalla stessa linea, esattamente come il gateway LoRaWAN. Carico aggiuntivo trascurabile (10-20 W).
5. **Apparato fisicamente compatto**: uno switch industriale a 8-16 porte sta in un modulo DIN-rail da poche unità rack, non aumenta significativamente l'ingombro del cabinet.

---

## 6. Componenti dello switch a ogni smart-gate

Lo switch tipo per questo scenario ha le seguenti caratteristiche:

- **2 porte SFP+** per la fibra dell'anello (10 Gbps per affrontare il caso di tutti i flussi video aggregati che attraversano lo smart-gate verso il CdC).
- **4-8 porte Gigabit Ethernet** per gli apparati interni allo smart-gate (SoC edge, telecamere PoE+, gateway LoRaWAN, controller maxi-schermo). Alcune con **PoE+** o **PoE++** per alimentare le telecamere.
- **Alimentazione ridondata** (doppia) 24-48 VDC, alimentata dalla rete dello smart-gate con UPS.
- **Temperatura operativa estesa** -40°C / +75°C: lo switch può lavorare anche in galleria d'estate o in montagna d'inverno senza condizionamento.
- **Supporto ERPS** (G.8032) e VLAN 802.1Q nativi.
- **Management out-of-band** via porta console seriale e via SNMPv3/SSH dalla rete di management dedicata.

---

## 7. Tracciato fisico della fibra

La fibra fisica viene posata in modi diversi a seconda del contesto:

- **Tratto a cielo aperto**: cavo in fibra armata, **canalina interrata** sotto la banchina o sotto il guard-rail. Posa con scavi minimi grazie a tecniche di **mini-trenching** (taglio della pavimentazione, posa del cavo, sigillatura). Profondità tipica 30-50 cm.
- **Tratti in galleria**: cavo fissato alla **controsoffittatura** o a bracci metallici lungo la parete del tunnel, con guaine ignifughe LSZH (Low Smoke Zero Halogen).
- **Tratti su viadotto**: cavo armato fissato sotto l'impalcato in alloggiamenti dedicati, con compensazione meccanica per le escursioni termiche.
- **Pozzetti di derivazione** a ogni smart-gate (o ogni paio di km): contengono i giunti ottici e i cassetti di permutazione per spillare la fibra dello smart-gate dal cavo dell'anello. Il pozzetto è impermeabile (IP68), accessibile dalla strada per manutenzione senza chiudere la carreggiata.

Il cavo in fibra tipico per questa applicazione ha **24 o 48 fibre** ottiche monomodali (G.652 standard), di cui solo 4-8 effettivamente utilizzate per la rete dello smart-gate: le altre sono **fibre di scorta ("dark fiber")** per espansioni future, sostituzione di fibre danneggiate, o servizi aggiuntivi (es. videosorveglianza dedicata, connettività per i ristoranti/aree di servizio lungo il tratto).
