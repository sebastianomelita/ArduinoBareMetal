# MPLS L3VPN: dispensa di approfondimento

> **Target**: 5ª ITI Informatico
> **Prerequisiti**: routing IP, OSPF, BGP, concetto di VPN, incapsulamento

---

## 1. Il paradigma del circuito virtuale

Le reti a commutazione di pacchetto si dividono in due grandi famiglie a seconda di **come ogni nodo decide dove inoltrare un pacchetto**.

### 1.1 Datagram (es. IP)

Ogni pacchetto porta con sé l'**indirizzo completo della destinazione**. Ogni router lungo il cammino:

1. Estrae l'indirizzo destinazione.
2. Consulta la propria tabella di routing.
3. Decide autonomamente il next-hop.

I pacchetti dello stesso flusso possono in teoria seguire percorsi diversi, e il router non ha memoria della "conversazione": ogni pacchetto è trattato in modo indipendente. È **stateless** e robusto, ma il lookup è costoso (longest prefix match su tabelle con centinaia di migliaia di rotte) e non è facile garantire QoS o ingegneria del traffico.

### 1.2 Circuito virtuale (es. MPLS)

Prima di trasmettere si **stabilisce un cammino** tra sorgente e destinazione, identificato da una piccola **etichetta** (label, tag). I pacchetti portano solo l'etichetta, non l'indirizzo completo. Ogni nodo intermedio:

1. Legge l'etichetta in ingresso.
2. La usa come **chiave diretta** in una tabella locale.
3. Trova in O(1) la porta di uscita e la nuova etichetta.

Tutti i pacchetti dello stesso flusso seguono lo stesso percorso pre-determinato. È **stateful** (il nodo deve aver imparato il binding label→azione) ma il forwarding è velocissimo e il percorso è prevedibile.

### 1.3 Proprietà fondamentali del circuito virtuale

Tre proprietà tornano sempre, indipendentemente dalla tecnologia specifica:

- **Etichette a significato locale**: l'etichetta vale solo sulla tratta tra due nodi adiacenti. Ogni nodo intermedio fa **swap**: legge l'etichetta in ingresso, la sostituisce con quella concordata col nodo successivo. Questo evita di dover coordinare globalmente uno spazio di nomi.
- **Separazione tra signalling e data plane**: c'è una fase preliminare (signalling) in cui i nodi si scambiano informazioni e popolano le tabelle di forwarding; poi la fase operativa (data plane) usa quelle tabelle per inoltrare i pacchetti veri.
- **Forwarding per indice, non per ricerca**: la label è progettata per essere usata come indice in una tabella, non come chiave da cercare. Lookup costante e velocissimo.

### 1.4 MPLS in questa cornice

MPLS è una realizzazione moderna del paradigma del circuito virtuale, fatta girare sopra reti IP. I circuiti virtuali si chiamano **LSP** (Label Switched Path), le etichette **label MPLS** (32 bit, di cui 20 di valore vero), il signalling è fatto da protocolli come **LDP**, e il data plane fa swap di label a velocità di linea.

> 💡 **In una frase**: MPLS = circuito virtuale a pacchetto, sopra IP, con signalling distribuito.

---

## 2. Anatomia di una rete MPLS: CE, PE, P

In una rete MPLS gestita da un ISP per offrire servizi a organizzazioni clienti, ci sono tre ruoli ben distinti.

```
   Sede A AziendaX                Backbone ISP                 Sede B AziendaX
  ┌──────────────┐         ┌──────────────────────┐         ┌──────────────┐
  │              │         │                      │         │              │
  │   CE-A   ────┼─────────┤ PE1 ── P1 ── P2 ── PE2 ├─────────┼── CE-B       │
  │              │         │                      │         │              │
  └──────────────┘         └──────────────────────┘         └──────────────┘
   rete cliente             dominio MPLS dell'ISP             rete cliente
```

### 2.1 CE — Customer Edge

È il **router di bordo nella sede del cliente**. Caratteristiche:

- Parla con il PE usando un **protocollo di routing standard**: eBGP, OSPF, RIP, oppure semplici rotte statiche.
- **Non sa nulla di MPLS, di VPN, di label**: pensa di parlare con un router IP qualsiasi.
- Annuncia al PE le rotte interne della propria sede (es. "io ho dietro la subnet `192.168.10.0/24`") e impara dal PE le rotte verso le altre sedi della stessa azienda.

Il fatto che il CE sia "ignorante" della tecnologia MPLS è un grande vantaggio: permette al cliente di non dover cambiare hardware o competenze.

### 2.2 PE — Provider Edge

È il **router dell'ISP che si affaccia ai CE**. È qui che vive tutta la complessità del servizio L3VPN:

- Mantiene una **VRF separata per ogni cliente** (vedi §3).
- Imposta le **doppie label MPLS** sui pacchetti uscenti verso il backbone.
- Toglie le label sui pacchetti entranti dal backbone e li consegna al CE giusto.
- Partecipa a **MP-BGP** per scambiare con gli altri PE le rotte dei clienti.

I PE sono i nodi "intelligenti" della rete: sono le uniche entità che conoscono la mappatura tra clienti e VPN.

### 2.3 P — Provider core

Sono i **router interni del backbone ISP**. Il loro lavoro è semplicissimo:

- Fanno **solo label switching MPLS**: ricevono un pacchetto etichettato, leggono la label esterna, fanno swap, inoltrano.
- **Non conoscono i clienti**, non hanno VRF, non parlano BGP per le rotte VPN.
- Partecipano solo all'IGP interno (OSPF/IS-IS) e a LDP per imparare le label transport.

> 🔑 **Perché questo design scala**: i P, che sono numerosi e nel cuore della rete, restano semplici. La complessità è confinata ai PE, che sono al bordo. Aggiungere un nuovo cliente significa configurare due PE (quelli alle sue sedi), non toccare tutto il core.

### 2.4 Cliente = organizzazione, non PC

Vale la pena ribadirlo: nel contesto L3VPN il "cliente" è un'**organizzazione** (un'azienda, un ente, un'università) che possiede una propria rete IP distribuita su più sedi geografiche. L'ISP fornisce il servizio di interconnessione tra queste sedi facendole apparire come un'unica rete privata. I PC degli utenti finali sono dietro i CE e nemmeno sanno che esiste l'ISP: vedono solo un'unica grande rete aziendale.

---

## 3. Le tre tabelle del forwarding MPLS L3VPN

Per capire come un pacchetto attraversa la rete, dobbiamo conoscere le tre tabelle che entrano in gioco. Stanno su nodi diversi e sono indicizzate in modo diverso.

### 3.1 La VRF (Virtual Routing and Forwarding) — sui PE

È una **tabella di routing IP normale**, identica a quella di un qualsiasi router, **moltiplicata per cliente**. Ogni PE ne ha una per ogni cliente collegato.

| Prefisso IP destinazione | Next-hop | Interfaccia / Label VPN out |
|---|---|---|
| 192.168.10.0/24 | CE-A locale | eth0/1 (uscita diretta) |
| 192.168.20.0/24 | PE2 (loopback `10.255.0.2`) | label VPN 5012 |
| 192.168.30.0/24 | PE3 (loopback `10.255.0.3`) | label VPN 7001 |

Indicizzata per **prefisso IP**. Quando un pacchetto entra dal CE, il PE sa a quale VRF appartiene (in base all'interfaccia di ingresso) e fa lookup IP normale lì dentro. Il risultato dice **dove andare** (quale PE remoto) e **con quale label VPN consegnarlo**.

### 3.2 La LFIB (Label Forwarding Information Base) — su tutti i nodi MPLS

È la tabella che fa il **label switching vero e proprio**. Vive su ogni P e PE.

| Label in | Azione | Label out | Interfaccia out |
|---|---|---|---|
| 100 | SWAP | 200 | eth1 |
| 200 | SWAP | 300 | eth2 |
| 300 | POP | — | eth3 |
| 5012 | POP + consegna a VRF AziendaX | — | (verso CE-B) |

Indicizzata per **label MPLS**. È quella che fa il forwarding velocissimo nel core: arriva un pacchetto, leggo la label esterna, indice diretto in tabella, swap, fuori. Nessun lookup IP, nessuna ricerca, nessuna decisione.

### 3.3 La tabella di routing globale — sui PE e P

È la tabella IP "tradizionale" del provider, popolata dall'IGP interno (OSPF o IS-IS). Contiene le rotte verso i loopback di tutti i PE e P della rete dell'ISP. Serve da **base** per costruire le label transport tramite LDP (vedi §5).

| Prefisso | Next-hop | Interfaccia |
|---|---|---|
| 10.255.0.2/32 (loopback PE2) | P1 | eth0 |
| 10.255.0.3/32 (loopback PE3) | P1 | eth0 |

> 📊 **Riepilogo delle tre tabelle**:
>
> | Tabella | Dove vive | Indicizzata per | A cosa serve |
> |---|---|---|---|
> | **VRF** | PE | prefisso IP cliente | decidere dove va il pacchetto del cliente |
> | **LFIB** | P, PE | label MPLS | switching veloce nel core |
> | **Routing globale** | P, PE | prefisso IP del provider | base per LDP, raggiungibilità interna |

---

## 4. Forwarding in azione: due esempi

### 4.1 Esempio A: forwarding diretto (stessa VPN, stesso PE)

Scenario: AziendaX ha due sedi entrambe attaccate allo stesso PE1.

```
   Sede A                   Sede B
     │                        │
   CE-A ── eth0/1 ── PE1 ── eth0/2 ── CE-B
                      │
                      └── (resto della rete ISP)
```

Un pacchetto da `192.168.10.5` (Sede A) verso `192.168.20.7` (Sede B):

1. Pacchetto arriva su PE1 da `eth0/1`. PE1 sa che `eth0/1` è nella VRF di AziendaX.
2. PE1 fa lookup di `192.168.20.7` nella **VRF AziendaX**. Trova: next-hop = CE-B, uscita = `eth0/2`.
3. PE1 inoltra il pacchetto su `eth0/2` come pacchetto IP normale, **senza alcuna label MPLS**.

Non c'è nessun coinvolgimento di MPLS: il PE si comporta come un router IP qualsiasi. Le label entrano in gioco solo quando il pacchetto deve attraversare il backbone per raggiungere un PE remoto.

### 4.2 Esempio B: forwarding remoto (stessa VPN, PE diversi)

Scenario classico: due sedi di AziendaX, una su PE1 e una su PE2, separate dal backbone.

```
  CE-A ── PE1 ── P1 ── P2 ── PE2 ── CE-B
          │                  │
        VRF X              VRF X
```

Pacchetto da `192.168.10.5` verso `192.168.20.7`:

**Passo 1 — Ingress PE (PE1)**
- Pacchetto arriva sull'interfaccia "VRF AziendaX".
- PE1 fa lookup IP nella VRF X: trova "destinazione raggiungibile via PE2 (loopback `10.255.0.2`), label VPN = 5012".
- PE1 fa lookup nella **routing globale** per `10.255.0.2`: next-hop = P1.
- PE1 consulta la sua **LFIB** per la label transport verso `10.255.0.2`: trova 100.
- PE1 imposta lo stack di label e spedisce: `[transport=100][vpn=5012][IP cliente]`.

**Passo 2 — Core router P1**
- Riceve pacchetto con label esterna 100.
- Lookup in LFIB: `100 → SWAP 200 → eth-verso-P2`.
- Spedisce: `[transport=200][vpn=5012][IP cliente]`. **Non vede né label VPN né IP cliente.**

**Passo 3 — Penultimo hop P2 (PHP, Penultimate Hop Popping)**
- Riceve pacchetto con label esterna 200.
- Lookup in LFIB: `200 → POP → eth-verso-PE2`.
- Spedisce: `[vpn=5012][IP cliente]`. Ha tolto la label transport per alleggerire il lavoro di PE2.

**Passo 4 — Egress PE (PE2)**
- Riceve pacchetto con label 5012 (la VPN, ora in cima allo stack).
- Lookup in LFIB: `5012 → POP + consegna a VRF AziendaX su eth-verso-CE-B`.
- Spedisce a CE-B il pacchetto IP nudo.

**Passo 5 — Consegna finale**
- CE-B riceve un pacchetto IP normale e lo inoltra a `192.168.20.7` con il suo routing interno.

> 🔑 **Il ruolo delle due label**:
> - La **label transport** (100, 200, ...) cambia hop-by-hop e dice ai P "come arrivare al PE di destinazione".
> - La **label VPN** (5012) resta invariata per tutto il viaggio e dice al PE di destinazione "in quale VRF consegnare".
>
> È esattamente lo stesso schema di un'etichetta postale con doppia indicazione: "Spedire a Roma" (transport) + "Reparto contabilità" (VPN).

---

## 5. Come si popolano le label: signalling distribuito

Le label non nascono dal nulla: sono il risultato di un processo di signalling distribuito che coinvolge due protocolli che lavorano in tandem.

### 5.1 Fase 1 — IGP: chi è raggiungibile e dove sta

L'ISP fa girare al suo interno un **IGP** (di solito **OSPF** o **IS-IS**). Questo protocollo distribuisce la topologia: ogni router (P e PE) impara come raggiungere i loopback di tutti gli altri.

Esempio di cosa OSPF popola nella tabella di routing globale di PE1:

```
10.255.0.2/32 (loopback PE2) → next-hop P1, costo 30
10.255.0.3/32 (loopback PE3) → next-hop P1, costo 50
```

OSPF risponde alla domanda **"dove sta PE2?"** ma non dice ancora **"con quale label MPLS lo raggiungo?"**.

### 5.2 Fase 2 — LDP: assegnazione e annuncio delle label

LDP (Label Distribution Protocol) si appoggia sopra l'IGP. Per **ogni prefisso che l'IGP gli passa** (in particolare i loopback dei PE), ogni router:

1. **Genera localmente una label** (un numero scelto liberamente dal proprio pool, perché ha significato solo locale).
2. La **annuncia ai vicini** con un messaggio LDP: "se vuoi raggiungere il prefisso X attraverso di me, usa la label Y".
3. **Riceve gli annunci dei vicini** e popola la propria LFIB di conseguenza.

### 5.3 Esempio di costruzione di un LSP

Topologia: `PE1 — P1 — P2 — PE2`. Costruiamo l'LSP verso il loopback di PE2 (`10.255.0.2/32`).

**Annunci LDP (in ordine, partendo dalla destinazione)**:

1. **PE2 → P2**: "per raggiungere `10.255.0.2/32`, usa label 300".
   PE2 popola la sua LFIB: `300 → POP, consegna locale`.

2. **P2** sceglie localmente la label 200 e popola LFIB: `200 → SWAP 300, uscita verso PE2`.
   **P2 → P1**: "per raggiungere `10.255.0.2/32`, usa label 200".

3. **P1** sceglie localmente la label 100 e popola LFIB: `100 → SWAP 200, uscita verso P2`.
   **P1 → PE1**: "per raggiungere `10.255.0.2/32`, usa label 100".

4. **PE1** registra: "per spedire a PE2, label transport iniziale = 100".

L'LSP è costruito. Quando PE1 dovrà spedire un pacchetto verso PE2, saprà già che label imporre, senza dover decidere nulla in tempo reale.

> 💡 **Come si "agganciano" le due fasi**: OSPF dice **"ecco le destinazioni e i percorsi"**, LDP dice **"per ogni destinazione, ecco la label da usare al primo hop"**. Senza IGP, LDP non saprebbe a quali prefissi associare le label. Senza LDP, IGP non basterebbe perché manca la distribuzione delle etichette.

### 5.4 E le label VPN? Le distribuisce MP-BGP

Le label transport sono solo metà della storia. Le **label VPN** (quella interna dello stack, es. 5012) le distribuiscono i PE tra loro tramite **MP-BGP VPNv4**. Quando PE2 annuncia a PE1 una rotta cliente, l'annuncio include:

- Il prefisso IP del cliente (con il suo Route Distinguisher, per renderlo unico).
- La label VPN che PE2 si aspetta di ricevere su quel traffico.
- Il next-hop BGP, che è il loopback di PE2.

PE1, ricevendo questo annuncio, popola la sua VRF con il prefisso e si segna che dovrà imporre quella label VPN. Per la label transport verso PE2, invece, consulta quanto ha imparato da LDP. Le due label vengono impilate ed ecco lo stack `[transport][vpn][IP]` che vediamo in §4.2.

> 🎯 **Divisione del lavoro**:
> - **OSPF/IS-IS**: raggiungibilità interna (dove stanno i PE).
> - **LDP**: label transport (come arrivare ai PE attraverso il core).
> - **MP-BGP**: rotte cliente + label VPN (cosa c'è dietro ogni PE e come consegnarglielo).

---

## 6. RD e RT: identificare clienti e VPN (in breve)

Quando un PE annuncia in MP-BGP la rotta `192.168.10.0/24` di AziendaX, c'è un problema: AziendaY potrebbe usare lo stesso prefisso. BGP non saprebbe distinguerle.

Si usano due identificatori:

- **Route Distinguisher (RD)**: 8 byte che si **antepongono al prefisso** per renderlo unico in MP-BGP. `RD_AziendaX:192.168.10.0/24` e `RD_AziendaY:192.168.10.0/24` sono due rotte distinte. È un trucco tecnico per la disambiguazione.

- **Route Target (RT)**: una "etichetta logica" attaccata alla rotta che dice "questa appartiene alla VPN X". I PE sono configurati per **importare** nelle proprie VRF solo le rotte con certi RT. È quello che concettualmente assomiglia di più a un **VPN ID**.

In pratica: l'RD risolve l'ambiguità nel database BGP, l'RT decide chi vede cosa.

---

## 7. Riepilogo: schema di tutto

```
┌────────────────────────────────────────────────────────────────────┐
│                                                                    │
│  CONTROL PLANE (signalling)                                        │
│                                                                    │
│  ┌─────────┐   ┌─────────┐   ┌──────────────────────────┐          │
│  │  OSPF   │──▶│   LDP   │   │  MP-BGP VPNv4            │          │
│  │ (IGP)   │   │ (label  │   │  (rotte cliente +        │          │
│  │         │   │ transp.)│   │   label VPN + RD/RT)     │          │
│  └─────────┘   └─────────┘   └──────────────────────────┘          │
│       │             │                    │                         │
│       ▼             ▼                    ▼                         │
│  popola la    popola le LFIB      popola le VRF                    │
│  routing       di P e PE          dei PE                           │
│  globale                                                           │
│                                                                    │
├────────────────────────────────────────────────────────────────────┤
│                                                                    │
│  DATA PLANE (forwarding)                                           │
│                                                                    │
│  CE ──IP──▶ PE1 ──[T][V][IP]──▶ P ──[T'][V][IP]──▶ PE2 ──IP──▶ CE  │
│             │                    │                  │              │
│         lookup VRF +         swap label        POP label VPN +     │
│         imposta label        transport         consegna a VRF      │
│         transport e VPN                                            │
│                                                                    │
└────────────────────────────────────────────────────────────────────┘
```

---

## Appendice A — Segment Routing: l'evoluzione moderna

Negli ultimi anni si sta affermando un approccio alternativo a LDP, chiamato **Segment Routing (SR)**, in particolare nelle varianti **SR-MPLS** (segment routing su data plane MPLS) e **SRv6** (segment routing su IPv6).

L'idea è radicale: **eliminare LDP**. Invece di un protocollo dedicato che distribuisce label hop-by-hop, le label (chiamate **Segment Identifier**, SID) sono distribuite **direttamente dall'IGP** (OSPF o IS-IS, opportunamente esteso). Ogni router annuncia con OSPF non solo i suoi prefissi, ma anche i SID associati.

Inoltre, il PE di ingresso può **impilare più SID** per specificare l'intero percorso che il pacchetto deve seguire nel core, senza che i nodi intermedi debbano mantenere stato per LSP. Questo si chiama **source routing**: la sorgente decide il cammino, i nodi intermedi eseguono.

Vantaggi di SR rispetto a LDP:

- Un protocollo in meno da gestire (basta IGP esteso).
- Nessuno stato per LSP nei router intermedi → scalabilità.
- Traffic engineering più flessibile (basta cambiare lo stack di SID alla sorgente).
- Compatibile con SDN: un controller centrale può calcolare i percorsi e iniettarli nei PE.

Per le L3VPN, il ruolo di MP-BGP per le label VPN resta invariato: cambia solo il modo in cui vengono costruite le label transport. Concettualmente, tutto quello che hai imparato in questa dispensa sul forwarding a doppia label e sulle VRF continua a valere.

> 💡 SR-MPLS è oggi la direzione principale per le nuove implementazioni di backbone ISP. LDP resta diffusissimo nel mondo reale per ragioni di compatibilità e inerzia, ma è considerato "legacy" nei nuovi design.
