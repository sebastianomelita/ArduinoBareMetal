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

- Il segnale viene **rigenerato a ogni nodo**: a 1 km di distanza tra nodi adiacenti la rigenerazione è ampiamente sovrabbondante (la fibra monomodale arriva tranquillamente a 40-80 km senza rigenerazione).
- **L'anello è un unico dominio di broadcast** (per ciascuna VLAN): lo switch separa i domini di collisione, non quelli di broadcast. È la **segmentazione in VLAN** — e infine il confine L3 in centrale — a delimitare il broadcast. Proprio perché il dominio è unico e la topologia è ad anello serve ERPS, che blocca l'RPL per impedire i loop. Le R-APS girano su una VLAN di controllo dedicata usando i MAC degli switch, ma è un fatto di forwarding del protocollo, non di partizione dei domini.
- Il traffico verso il CdC attraversa N switch prima di arrivare a destinazione: ogni switch aggiunge tipicamente decine di microsecondi di latenza di store-and-forward. Su un tratto di 50 km, parliamo di ~1-2 ms aggiuntivi sul tempo di volo della luce in fibra (250 µs). Trascurabile.

**Chiusura dell'anello.** L'anello si chiude in due modi possibili:
- **Singolo cavo ottico** con due fibre fisicamente separate: una "va" e una "torna", entrambe nello stesso cavo. È economico ma in caso di taglio del cavo (escavatore, incidente, frana) **entrambi i percorsi vengono persi simultaneamente** e l'anello si spezza.
- **Doppio cavo ottico in percorsi geograficamente diversi**: la fibra dell'andata corre sul guard-rail di destra, quella del ritorno sul guard-rail di sinistra (o nella canalina opposta). Costa di più, ma garantisce sopravvivenza a un singolo guasto fisico. È la scelta corretta per infrastruttura critica come una smart-road.

**Protocollo di failover: ERPS (IEEE G.8032).** Sull'anello gira il protocollo **Ethernet Ring Protection Switching**, che in condizioni normali tiene **bloccata** una delle due tratte (il "Ring Protection Link", RPL) per evitare loop di broadcast. Quando un nodo rileva un guasto:

1. Invia un messaggio R-APS (Ring-APS) di tipo "Signal Fail" ai vicini.
2. Il nodo che teneva bloccato il RPL (RPL Owner) lo sblocca.
3. Tutti i nodi flushano la loro tabella MAC e la riapprendono sul nuovo percorso.
4. Tempo totale di failover: **< 50 ms**, conforme ai requisiti delle reti carrier-grade.

Per il nostro progetto è essenziale: 50 ms è impercettibile sia per la telemetria MQTT che per gli stream video.

### Dal singolo anello alla dorsale: trasporto DWDM

Gli anelli non si concatenano "l'uno nel successivo": ciascuna **tratta** ottiene una propria **lunghezza d'onda (λ)** su un'unica coppia di fibre di dorsale che percorre l'intero asse. Il **DWDM** multiplexa decine di λ indipendenti sulla stessa fibra; a ogni tratta un **OADM/ROADM** *inserisce* la sua λ e lascia transitare le altre in **puro bypass ottico** (nessuna conversione O/E/O, latenza pari alla sola propagazione, ~5 µs/km). Tutte le λ convergono al sito di centrale, dove vengono demultiplexate: ogni tratta vi arriva come un **collegamento punto-punto dedicato**. La topologia logica è quindi una **stella centrata sulla centrale**, anche se fisicamente la fibra di dorsale attraversa in serie tutti i siti.

Sulle tratte lunghe si inseriscono **amplificatori ottici (EDFA)**, tipicamente ogni 60-100 km. Per coerenza con la scelta del doppio cavo fatta per gli anelli, anche la dorsale va instradata su **due percorsi fisici distinti**, con protezione di linea ottica (OLP) o anello DWDM, così da sopravvivere a un singolo taglio.

### Il dispositivo di confine della tratta: due opzioni

### Opzione A1 — confine L2 (switch)

La λ trasporta in modo trasparente un **trunk di VLAN** fino a un **core switch (CS) multilayer** in centrale.

- Videocamere e relativo **server** stanno nella **stessa VLAN/subnet**: il server è attestato su una porta di accesso del CS in quella VLAN, e il traffico videocamere↔server resta **L2 puro, senza alcun router nel percorso**. Idem per il gateway sensori e il suo server di gestione.
- L'eventuale routing inter-VLAN (verso applicazioni del CdC, Internet, ecc.) è **centralizzato sul CS** tramite SVI. Non serve un router per tratta.
- Pro: semplice, trasparente, la *discovery* L2 (ONVIF/multicast) funziona end-to-end. Contro: il dominio di broadcast di ogni VLAN è esteso lungo la dorsale (da contenere con segmentazione VLAN, storm-control, ARP/ND suppression).

### Opzione A2 — confine L3/MPLS (PE router)

"Versione avanzata": il dispositivo di tratta è un **PE (Provider Edge)** con una **subinterfaccia per VLAN**, ciascuna mappata su una VPN.

- **L3VPN (una VRF per servizio):** traffico **instradato**, subnet di tratta ≠ subnet del server, raggiungibilità via routing nella VRF. Comportamento **analogo a una VPN TUN**. I dispositivi restano host con default gateway sul PE: nessun router lato dispositivo.
- **L2VPN (EVPN/VPLS):** **bridging** dei frame, la subnet è **stesa fino alla centrale**. Comportamento **analogo a una VPN TAP**. Si usa quando occorre preservare l'adiacenza L2 attraverso un core a pacchetto.
- Un PE **EVPN con IRB** copre entrambi i casi (bridging + routing integrati).

In alternativa al PE alla tratta si può adottare la topologia MPLS classica: un **router di confine (CE)** alla tratta che dialoga con un **PE in centrale**. Il CE è per definizione **fuori dall'MPLS** (scambia solo rotte IP col PE, non vede etichette né control plane), quindi l'accesso indesiderato all'infrastruttura MPLS è intrinsecamente impedito; l'isolamento tra tratte e verso la centrale è governato dalla **VRF + policy di route-target** sul PE. Il CE aggiunge un **punto di policy/NAT locale** vicino al campo e mantiene il dispositivo MPLS-aware nel sito fisicamente sicuro.

### Segmentazione e indirizzamento

- **B1 (bridging):** una subnet per VLAN di servizio. Caso semplice: estesa a tutte le tratte (dominio di broadcast più ampio). In alternativa: VLAN/subnet dedicate per tratta, comunque terminate sul CS, per confinare il broadcast.
- **B2 L3VPN (routing):** una **subnet isolata per tratta**, ciascuna nella propria VRF; l'L3VPN instrada tra le subnet delle tratte e quella di centrale. (Il modello L2VPN/EVPN riporterebbe invece a una subnet estesa, come in B1 ma su core L3.)

### Qualità del servizio e priorità del traffico

I flussi hanno profili diversi: le **segnalazioni** (telemetria MQTT, messaggi C-ITS, allarmi, comando dei pannelli) sono a bassa banda ma sensibili a latenza e perdita e ad **alta priorità**; lo **streaming video** è ad alta banda e più tollerante alle perdite, a priorità inferiore.

La priorità si ottiene con la **QoS**, non con la mera separazione in VLAN/VRF: classificazione e marcatura (**802.1p/CoS** a L2, **DSCP** a L3) e **code a priorità** ai punti di contesa (uplink dell'anello, uplink del CS, porte verso i server). Questi meccanismi sono **nativi negli switch di B1**: non è necessario MPLS per garantire priorità alle segnalazioni rispetto al video. Con una **λ dedicata per tratta** non c'è inoltre contesa *tra* tratte (lunghezze d'onda distinte): la QoS agisce solo dentro l'anello e sugli uplink centrali. L'MPLS aggiungerebbe il **DiffServ-TE** (LSP dedicati per classe con riserva di banda su un core multi-hop), funzione priva di valore in una topologia a centrale unica.

### Sicurezza: isolamento della zona sensori/OT

L'isolamento del segmento sensori si realizza con un **firewall centrale** che ne presidia il confine L3 (firewall-on-a-stick sul CS, oppure VRF dedicata raggiungibile solo via firewall). La segmentazione vera la fa la **policy a zone** in logica least-privilege: la zona sensori è abilitata **unicamente** verso il network server sui port previsti (es. MQTT 8883, UDP 1700), con divieto di movimento laterale e di egress non necessario. Poiché il traffico dei gateway è **client-initiated** verso il network server, la policy può essere strettamente egress-only.

Il **NAT** non è un meccanismo di sicurezza e si impiega solo se necessario: sovrapposizioni di indirizzamento ("cookie-cutter" per sito) o uscita verso reti esterne/cloud. In un dominio privato sono in genere preferibili indirizzi univoci e stabili per dispositivo (tracciabilità, raggiungibilità server→device per configurazione e firmware), quindi si privilegia un piano d'indirizzamento per tratta rispetto al NAT.

### Scelta architetturale e razionale

Per il presente progetto si adotta l'**Opzione B1**: anelli di accesso in **ERPS (G.8032)**, trasporto su **dorsale DWDM con una λ dedicata per tratta**, terminazione su un **CS multilayer in coppia HA** presso l'unica centrale, isolamento dei servizi tramite VLAN (ed eventualmente VRF-lite sul CS) e **firewall centrale** a presidio della zona sensori/OT.

La scelta è motivata dalla topologia a **centrale unica**. L'MPLS L3VPN (Opzione B2) ripaga la propria complessità solo con più siti centrali, multi-tenancy, traffic engineering o isolamento di routing scalabile via MP-BGP: condizioni qui assenti. Le **λ dedicate** forniscono già isolamento a livello fisico (L1) sul trasporto, e l'unico punto di terminazione rende sufficiente la segmentazione a VLAN più un firewall di zona, senza introdurre un dominio MPLS.

L'architettura resta predisposta a evolvere verso **B2/MPLS** qualora subentrino più siti centrali o l'esigenza di isolamento di routing forte e scalabile (VRF con route-target), distinguendo il modello **L3VPN** (instradato, una subnet isolata per tratta, ≈ tunnel TUN) dal modello **L2VPN/EVPN** (bridging, subnet estesa, ≈ tunnel TAP). L'inserimento di un router di confine (CE) per **NAT o policy locali** vicino al campo resta una scelta **ortogonale**, applicabile anche all'interno di B1.

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

[Ultimo miglio ISP in fibra](../approfondimenti/isp_access.md)

---

