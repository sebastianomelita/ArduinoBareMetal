>[Torna a reti di sensori](sensornetworkshort.md#servizi-di-accesso-radio-per-WSN)>- [Torna a Dettaglio architettura WiFi mesh](/archmesh.md) 


# Verifica di Sistemi e Reti

**Infrastruttura di rete per la fruizione di contenuti multimediali in un sito archeologico**

*Adattamento didattico della seconda prova scritta dell'Esame di Stato 2018-2019, indirizzo Informatica e Telecomunicazioni — articolazione Informatica.*

---

## Ipotesi aggiuntive di lavoro

Prima di entrare nel merito fisso alcune ipotesi che rendono il problema dimensionabile; esplicitarle è parte integrante della prova.

- **Sito di riferimento**: superficie ~4 ettari, perimetro irregolare di ~800 m, con un nucleo fortificato centrale (mastio/torri), un cortile principale, un antiquarium indoor, una chiesa, camminamenti di ronda e aree archeologiche scoperte.
- **POI**: 12 punti di interesse, di cui 7 outdoor e 5 indoor, distanze tipiche tra POI 30–80 m.
- **Visitatori contemporanei stimati**: picco di 200 minitablet attivi nelle festività, media giornaliera 60.
- **Backhaul Internet**: FWA in banda licenziata da 100/50 Mbps, attestato presso l'InfoPoint dove risiede anche il *server locale dei contenuti* (architettura edge: i contenuti sono in LAN, Internet serve solo per amministrazione, aggiornamenti e ridondanza).
- **Vincolo di sovrintendenza**: divieto di carotaggi murari per cablaggi strutturati; sono ammessi passaggi a vista in canalina rimovibile lungo cornicioni o sottogronda, e installazioni reversibili. Conseguenza: il cablaggio in rame o fibra è impraticabile per coprire l'intero sito, ed è necessario ricorrere a un **wireless backhaul** tra i nodi.

---

## Quesito 1 — Architettura di rete

### 1.a — Scelta dell'architettura wireless

La combinazione di estensione ampia, impossibilità di cablare e unico punto di attestazione Internet porta naturalmente verso una Wireless Mesh Network (WMN) conforme allo standard IEEE 802.11s, oppure realizzata con protocolli proprietari analoghi (Cisco, Aruba, MikroTik, Ubiquiti). In una WMN gli access point — detti *mesh nodes* — si interconnettono tra loro via radio formando un backbone wireless multi-hop, e uno o più di essi (i *mesh gateway*) si attestano sul backhaul cablato verso il server e verso Internet.

Le ragioni che la rendono idonea al contesto sono:

- **Auto-configurazione e auto-riparazione**: i nodi scoprono i vicini ed eleggono i percorsi migliori autonomamente, con metrica tipicamente Airtime Link Metric sotto HWMP (Hybrid Wireless Mesh Protocol, definito da 802.11s).
- **Tolleranza ai guasti**: la caduta di un nodo non isola il resto della rete, perché HWMP ricostruisce percorsi alternativi.
- **Riduzione al minimo dei cablaggi**: solo il nodo gateway presso l'InfoPoint richiede backhaul cablato; gli altri necessitano della sola alimentazione (o PoE locale).
- **Compatibilità con i vincoli di sovrintendenza**: le antenne possono essere installate su supporti reversibili (staffe a morsetto, pali esistenti), senza alterazioni murarie significative.

A questo punto si aprono due varianti architetturali, entrambe ragionevoli, da discutere e confrontare.

#### Variante A — Mesh di bridge (Layer 2)

Tutti i nodi mesh operano come bridge trasparenti: il backbone wireless è una grande LAN broadcast (o segmentata in VLAN ma sempre L2). Gli AP estendono il dominio di livello 2 fino al gateway, dove un router/firewall fa da default gateway per i client.

**Vantaggi**: semplicità di configurazione; roaming dei client tra AP senza cambio di indirizzo IP; gestione centralizzata via controller WLAN (architettura thin AP, eventualmente CAPWAP); ottima per servizi che richiedono continuità di sessione.

**Svantaggi**: il dominio broadcast cresce con la rete; ARP e DHCP attraversano tutto il backbone; con molti hop la latenza cresce. Segmentazione logica solo via VLAN (802.1Q), da pianificare con cura su ogni nodo.

#### Variante B — Mesh di router (Layer 3)

Ogni nodo mesh è un router IP indipendente: il backbone wireless trasporta pacchetti IP instradati hop-by-hop tramite un protocollo di routing dinamico adatto a topologie wireless instabili, tipicamente OLSR (Optimized Link State Routing, RFC 3626) o B.A.T.M.A.N. Advanced in modalità routed. Ciascun AP serve una propria subnet di client.

**Vantaggi**: domini broadcast piccoli e contenuti; scalabilità superiore; isolamento naturale tra rami della rete (utile per la segmentazione dei POI); QoS per-hop più granulare.

**Svantaggi**: configurazione più complessa; roaming L2 dei client non garantito (cambiando AP cambia subnet → re-DHCP); maggior costo degli apparati capaci di routing.

**Nota progettuale**: nella variante L3 le subnet *di transito sui link mesh* non vengono assegnate manualmente: si sfruttano gli indirizzi IPv6 link-local (prefisso `fe80::/10`), generati automaticamente dall'algoritmo SLAAC a partire dal MAC di ciascuna interfaccia radio. Sono univoci sul singolo link e sufficienti a far funzionare un protocollo di routing come OLSRv2 o Babel, che li usa per scambiare hello e topology control. Le **subnet di accesso ai client** invece restano definite manualmente dal sistemista (vedi 1.c), perché devono essere note ai meccanismi di provisioning, autorizzazione e ACL.

#### Scelta motivata

Per questo scenario — utenti che camminano da un POI all'altro tenendo aperta una pagina web e che hanno autenticato una sessione — la scelta primaria è la variante A (mesh di bridge L2) con segmentazione logica per VLAN, perché:

1. il roaming senza interruzione è importante: il visitatore non deve riautenticarsi passando da un AP all'altro;
2. la scala è modesta (≤20 nodi mesh), ben dentro i limiti pratici di un singolo dominio L2 ben dimensionato;
3. la gestione tramite controller WLAN semplifica radicalmente la manutenzione da parte del personale dell'Ente.

La variante B viene discussa in parallelo nei punti successivi, perché diventa preferibile se il sito si espandesse oltre i 20 nodi o se si volesse un isolamento di sicurezza ancora più forte tra POI.

![Schema apparati attivi](../img/apparati.png)

*Figura 1 — Schema apparati attivi (simbologia Cisco): confronto fra mesh L2 (bridge, separazione a macchia di leopardo via 802.1Q) e mesh L3 (router, subnet di accesso fisicamente dislocate, link mesh autoconfigurati su IPv6 link-local).*

### 1.b — Dimensionamento dei nodi

#### Numero di AP

La regola pratica è copertura + capacità. Su 4 ettari con muri spessi:

- **Outdoor**: AP con antenne settoriali o omnidirezionali, raggio utile ~40–60 m in 5 GHz Wi-Fi 6 (802.11ax) → 8 AP outdoor.
- **Indoor (torri, antiquarium, chiesa)**: un AP per ambiente, perché i muri in pietra attenuano fortemente il segnale → 5 AP indoor.
- **Totale**: 13 nodi mesh + 1 nodo gateway presso l'InfoPoint.

#### Criteri di posizionamento

- Ogni POI deve avere almeno un AP entro 15 m con linea di vista sufficiente per garantire RSSI ≥ −65 dBm (necessario per il vincolo di prossimità, vedi Q2).
- Ogni nodo mesh deve avere almeno due vicini visibili, per garantire la ridondanza HWMP.
- Gli AP outdoor vanno installati ad altezza intermedia (3–4 m) e non sulle sommità delle torri, per evitare copertura eccessiva oltre il perimetro (questo serve anche al vincolo di prossimità).

#### Numero di radio per AP e pianificazione cellulare dei canali

Un nodo mesh fa contemporaneamente **access** (serve i client) e **backhaul** (parla con i vicini). Se queste due funzioni condividono la stessa radio si finisce nel classico problema dei single-radio mesh: il throughput utile si dimezza ad ogni hop, perché la radio non può trasmettere e ricevere contemporaneamente sullo stesso canale (CSMA/CA serializza tutto). La via standard per evitarlo è separare le funzioni su radio fisiche distinte.

**Apparati dual-band (2 radio).** Tipicamente una radio 2.4 GHz e una 5 GHz. Si può scegliere:

- mettere il backhaul sui 5 GHz e l'access sui 2.4 GHz: backhaul veloce ma access lento e congestionato (la banda 2.4 GHz ha solo 3 canali non sovrapposti — 1, 6, 11 — e tantissime sorgenti di interferenza);
- mettere access e backhaul entrambi sui 5 GHz, ma su canali diversi: la radio è una sola e va in time-sharing, quindi è la soluzione peggiore in termini di prestazioni.

In pratica gli apparati dual-band sono accettabili solo come ripiego economico per nodi periferici a basso carico.

**Apparati tri-band (3 radio).** Le tre radio coprono in genere 2.4 GHz, 5 GHz lower (canali 36–64) e 5 GHz upper (canali 100–144, DFS). La configurazione consigliata è:

- radio 1 — 2.4 GHz per access ai client legacy (Wi-Fi 4/5, IoT, smartphone vecchi);
- radio 2 — 5 GHz lower per access ai client moderni (minitablet Wi-Fi 6);
- radio 3 — 5 GHz upper DFS dedicata al backhaul mesh, su canali a 80 MHz con potenza più alta consentita dal regolatorio (in EU su DFS si raggiungono i 23 dBm EIRP indoor, 30 dBm outdoor).

Così access e backhaul lavorano in parallelo, non in time-sharing, e si recupera tutto il throughput utile sui multi-hop.

**Apparati con 4 radio o radio aggiuntiva 6 GHz / 60 GHz.** Su tratte punto-punto critiche (es. dal mastio al gateway) si può aggiungere una radio mmWave 60 GHz (802.11ad/ay) con antenna direttiva: throughput dell'ordine del Gbps, immune all'interferenza degli altri canali Wi-Fi, ma richiede line-of-sight. È la scelta classica per i due-tre link più importanti del backbone.

**Vantaggi/svantaggi a confronto:**

| Apparato | Vantaggi | Svantaggi |
|---|---|---|
| Dual-band (2 radio) | Costo basso; meno antenne da installare; alimentazione PoE inferiore. | Access e backhaul si contendono lo spettro; throughput utile dimezzato ad ogni hop; sopravvive solo per nodi foglia a basso carico. |
| Tri-band (3 radio) | Backhaul dedicato e parallelo all'access; throughput costante anche su 2–3 hop; sfrutta tutti i canali Wi-Fi disponibili in EU; supporta client legacy senza penalizzare i moderni. | Costo più alto; più antenne; maggiore consumo elettrico (PoE+ o PoE++); pianificazione canali più articolata. |
| Tri-band + radio 60 GHz | Backbone capace di Gbps su tratte critiche; nessuna interferenza con il resto della rete Wi-Fi; latenza minima. | Richiede line-of-sight perfetta; sensibile alla pioggia intensa; costo per link elevato; ha senso solo per i 2–3 link principali del backbone. |

**Scelta per il progetto:** apparati **tri-band** per tutti i nodi mesh, con eventuale aggiunta di un link 60 GHz dedicato tra il gateway dell'InfoPoint e il nodo di sommità del mastio (che è il punto di rilancio naturale verso il resto del sito).

#### Pianificazione cellulare dei canali

Quando si dispongono più AP in un'area ristretta, va evitata l'interferenza co-canale (CCI), cioè AP vicini che trasmettono sullo stesso canale e si rubano tempo d'aria a vicenda. La tecnica consolidata, mutuata dalle reti cellulari, è il riuso di frequenza con schema esagonale: si assegnano i canali in modo che due AP che operano sulla stessa frequenza siano il più lontano possibile, mentre due AP fisicamente vicini ricevano canali ben separati nello spettro.

**Regola pratica:** dispositivi *vicini nello spazio* → frequenze *lontane*; dispositivi *lontani nello spazio* → frequenze anche *vicine* (il riuso diventa possibile perché l'attenuazione di propagazione li disaccoppia).

**Banda 2.4 GHz (access legacy).** Solo 3 canali non sovrapposti in EU: 1, 6, 11. Schema di riuso a 3 colori, sufficiente per la maggior parte dei layout.

**Banda 5 GHz (access moderno e backhaul).** In EU lo spettro 5 GHz è diviso in canali base da 20 MHz. Per avere throughput maggiori si **aggregano canali adiacenti** in canali più larghi: 40 MHz (coppia), 80 MHz (quadrupla), 160 MHz (ottupla). Più ampio è il canale, più alto il throughput ma anche più spettro occupato.

In totale ci sono 19 canali base da 20 MHz, distribuiti in tre sotto-bande regolatorie:

- **U-NII-1** (non-DFS): canali 36, 40, 44, 48
- **U-NII-2A** (DFS): canali 52, 56, 60, 64
- **U-NII-2C** (DFS): canali 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140

Aggregandoli si ottengono 9 canali a 40 MHz oppure **4 canali a 80 MHz non sovrapposti**, identificati dal numero del primo canale base che li compone:

| Nome breve | Slot da 20 MHz occupate | Sotto-banda | DFS |
|---|---|---|---|
| **ch 36 @ 80 MHz** | 36, 40, 44, 48 | U-NII-1 | no |
| **ch 52 @ 80 MHz** | 52, 56, 60, 64 | U-NII-2A | sì |
| **ch 100 @ 80 MHz** | 100, 104, 108, 112 | U-NII-2C | sì |
| **ch 116 @ 80 MHz** | 116, 120, 124, 128 | U-NII-2C | sì |

Questi 4 canali a 80 MHz sono spettralmente disgiunti: due AP che usano canali diversi della tabella non si interferiscono, indipendentemente da quanto siano vicini fisicamente. È esattamente quello che serve per il **riuso a 4 colori** della pianificazione cellulare: si associa un colore a ciascun canale e si distribuiscono i colori sulla griglia degli AP in modo che celle adiacenti abbiano colori diversi.

Sul nostro sito (Figura 2):

- **Gruppo A** = ch 36 @ 80 MHz (occupa 36–48)
- **Gruppo B** = ch 52 @ 80 MHz (occupa 52–64)
- **Gruppo C** = ch 100 @ 80 MHz (occupa 100–112)
- **Gruppo D** = ch 116 @ 80 MHz (occupa 116–128)

Restano disponibili, **fuori da questi 4 colori**, i canali alti **ch 132 @ 80 MHz** (occupa 132–144, ancora DFS) e **ch 149 @ 80 MHz** (occupa 149–161, banda U-NII-3 non-DFS, massima EIRP outdoor consentita): si usano per il backhaul mesh, completamente separati dallo spettro access.

---

Note rapide sulla notazione:
- "**ch X @ 80 MHz**" significa "canale a 80 MHz che inizia con il canale base numero X", quindi occupa X, X+4, X+8, X+12.
- È il modo standard di indicare i canali aggregati nei manuali Wi-Fi e nei tool di site survey (Ekahau, NetSpot).

**Esempio concreto sul nostro sito.** Supponiamo le 4 torri perimetrali (T1, T2, T3, T4) e il mastio centrale (M):

```
          T1                            T2
             \                       /
              \                     /
               \         M         /
                \       (gw)      /
                 \               /
          T4 -----+-------------+----- T3
```

Assegnazione canali 5 GHz (access, larghezza 40 MHz):

- `T1 → ch 36/40    (gruppo A)`
- `T2 → ch 52/56    (gruppo B)`
- `T3 → ch 100/104  (gruppo C)`
- `T4 → ch 116/120  (gruppo D)`
- `M  → ch 36/40    (gruppo A, riusato — è lontano da T1 ed è circondato da B, C, D)`

I quattro AP perimetrali sono vicini fra loro a due a due (T1-T2, T2-T3, T3-T4, T4-T1) e ricevono gruppi tutti distinti; il mastio M, riusa lo stesso gruppo della torre più lontana (T1), perché lo schermo delle altre torri e la distanza diagonale lo isolano sufficientemente. Questo è il classico riuso N=4 di una rete cellulare.

**Pianificazione dei canali backhaul.** Per i link mesh si usa una porzione di spettro **completamente disgiunta** da quella access. In EU, dopo aver speso 36/40, 52/56, 100/104 e 116/120 per i quattro gruppi di celle, restano disponibili due canali a 80 MHz nella banda alta U-NII-2C/U-NII-3: il **canale 132** e il **canale 149**. Sono ideali per il backhaul perché:

- lontani dai canali access (no autointerferenza fra access e backhaul sullo stesso AP);
- in U-NII-3 (149 e oltre) il regolatorio EU consente la massima EIRP outdoor (fino a 30 dBm), utile per i link a distanza maggiore;
- il canale 132 è DFS ma stabile su tratte fisse, dove gli AP non vengono spostati e quindi gli eventi radar restano un rischio gestibile.

**Distribuzione dei canali sulla topologia ad albero.** Sulla nostra topologia (Figura 2) la pianificazione si fa per livelli, alternando 132 e 149:

- **T5 (root)** ha 2 radio mesh: la radio "ramo ovest" su `ch 132` verso T2, la radio "ramo est" su `ch 149` verso T3. Più una radio 60 GHz direttiva verso T7-GW.
- **T2 (concentratore ovest)** ha 2 radio mesh: una su `ch 132` verso T5 (per parlare al root), una su `ch 149` verso le foglie T1 e T6. I canali sono **opposti** per evitare che le due radio di T2 si pestino.
- **T3 (concentratore est)** simmetricamente: una radio su `ch 149` verso T5, una su `ch 132` verso le foglie T4 e T8.
- **Foglie (T1, T6, T4, T8)**: una sola radio mesh, sul canale del concentratore. T1 e T6 sullo stesso canale `ch 149` verso T2: CSMA/CA gestisce la contesa, ma sono soli 2 interlocutori che condividono la banda, accettabile.

**Vantaggio della pianificazione alternata.** Le 4 trasmissioni concorrenti sullo stesso istante (T5↔T2 su 132, T5↔T3 su 149, T2↔T1/T6 su 149, T3↔T4/T8 su 132) avvengono **su due canali diversi**, quindi in parallelo. Senza l'alternanza, tutte e 4 sarebbero costrette a CSMA/CA sullo stesso canale, dimezzando il throughput utile.

**Strumenti operativi:** un site survey con strumenti come Ekahau o NetSpot consente di misurare RSSI e SNR effettivi nei punti critici, e di affinare la pianificazione iniziale. Molti controller WLAN moderni offrono anche un meccanismo di *Auto-RF* (es. Cisco RRM, Aruba ARM) che assegna e ribilancia i canali dinamicamente in base alle misure on-air; resta comunque buona pratica impostare manualmente i canali del backhaul (per non destabilizzare i link punto-punto), lasciando l'auto-RF solo sull'access.

![Planimetria fisica del sito](../img/planimetria.png)

*Figura 2 — Planimetria fisica del sito archeologico: disposizione di AP mesh (T1–T8), POI, gateway, server e InfoPoint. Le celle (access) sono circolari per via delle antenne omnidirezionali e usano i canali dei gruppi A/B/C/D (36/52/100/116) con riuso N=4. La topologia di backhaul è ad albero a due livelli: T5 (mastio centrale) è la radice, T2 e T3 sono concentratori intermedi che aggregano rispettivamente T1/T6 (settore ovest) e T4/T8 (settore est). I link backhaul usano canali 5 GHz alti (ch 132 e ch 149) alternati lungo l'albero, in modo che le due radio mesh di uno stesso concentratore non si interferiscano. Il link tra T5 e T7-GW è in 60 GHz direttivo (linea rossa). In Figura 3 si analizza il comportamento in caso di guasto di T5.*

#### Gateway verso Internet

- Router/firewall integrato (next-generation firewall con IPS e web filtering).
- Terminazione del backhaul FWA (CPE radio del provider).
- DHCP server, server RADIUS per autenticazione 802.1X, captive portal.
- Server applicativo locale con i contenuti (NAS o piccolo server x86) collegato in LAN cablata diretta al gateway.

### 1.c — Indirizzamento e subnetting

Individuo cinque gruppi di utenti/dispositivi logicamente distinti, ciascuno in VLAN + subnet separata (RFC 1918).

| VLAN | Funzione | Subnet | DHCP | Note |
|---|---|---|---|---|
| 10 | Management AP e infrastruttura | `10.0.10.0/24` | statico | accesso solo da rete admin |
| 20 | Server applicativo, RADIUS, captive portal | `10.0.20.0/24` | statico | server farm locale |
| 30 | Minitablet visitatori | `10.0.30.0/23` | dinamico | 510 host, sopra il picco |
| 40 | InfoPoint e cassa | `10.0.40.0/24` | statico | postazioni operatori |
| 50 | Telecamere/IoT di servizio | `10.0.50.0/24` | statico | isolata da utenti |

Le VLAN sono trasportate sul backbone mesh come tag 802.1Q (la maggior parte delle implementazioni 802.11s supporta VLAN tagging sui mesh link). Il default gateway di ogni VLAN risiede sul firewall, che applica le ACL inter-VLAN secondo il principio del minimo privilegio (vedi Q3).

**Variante L3 — indirizzamento dei link mesh:** le subnet IPv4 dei collegamenti *punto-punto* tra nodi mesh non vengono assegnate. Ogni interfaccia radio mesh ottiene automaticamente un indirizzo IPv6 link-local del tipo `fe80::/10`, generato via SLAAC dal MAC. Il protocollo di routing dinamico (Babel o OLSRv2 sono i candidati naturali su mesh wireless) scambia hello e topology control direttamente su questi indirizzi: validi su un singolo link, non instradati, sufficienti per costruire la tabella di forwarding. Le **subnet di accesso ai client**, invece, restano pianificate dal sistemista perché devono essere note al DHCP, al firewall, alle ACL e al sistema di provisioning. Ogni AP riceve una /27 dello spazio 10.0.30.0/23 (es. AP1 = 10.0.30.0/27, AP2 = 10.0.30.32/27, …), annunciata via Babel/OLSRv2 al gateway. Lo spazio di numerazione è statico e pianificato; ciò che è autoconfigurato sono solo le adiacenze dei link mesh.

### 1.d — Routing

Con la variante L2 il "routing" è in realtà inter-VLAN routing sul firewall, completamente statico (le subnet sono direttamente connesse al firewall stesso). La tabella di routing del firewall ha forma:

```
Destinazione         Next hop         Interfaccia
0.0.0.0/0            <ISP gateway>    WAN (FWA)
10.0.10.0/24         0.0.0.0          VLAN10
10.0.20.0/24         0.0.0.0          VLAN20
10.0.30.0/23         0.0.0.0          VLAN30
10.0.40.0/24         0.0.0.0          VLAN40
10.0.50.0/24         0.0.0.0          VLAN50
```

Sul backbone mesh il "routing" è in realtà forwarding L2 secondo HWMP (path selection a livello 2), non IP routing.

Nella variante L3, invece, sui link mesh si usa un protocollo di routing dinamico. Esempio di tabella su un nodo mesh intermedio (AP3) in una topologia a 5 nodi (Gateway, AP1, AP2, AP3, AP4); gli indirizzi sui link mesh sono IPv6 link-local autoconfigurati, le subnet di accesso ai client sono /27 statiche:

```
# Tabella IPv6 (link mesh, autoconfigurata)
Destinazione                 Next hop                   IF
fe80::/64                    ::                          mesh0   diretto

# Tabella IPv4 (accesso client, pianificata + appresa via Babel/OLSRv2)
Destinazione         Next hop (mesh nbr)   Interfaccia   Metrica
10.0.30.0/27         fe80::aa:bb:cc:01     mesh0         2 hop  (client di AP1)
10.0.30.32/27        0.0.0.0               br-client     diretto (client di AP3)
10.0.30.64/27        fe80::aa:bb:cc:02     mesh0         1 hop  (client di AP2)
10.0.30.96/27        fe80::aa:bb:cc:04     mesh0         1 hop  (client di AP4)
0.0.0.0/0            fe80::aa:bb:cc:00     mesh0         1 hop  (default → gateway)
```

La scelta ibrida (statico sul gateway, dinamico sul mesh) è quella adottata in pratica anche dalle distribuzioni mesh community (Freifunk, Ninux): il backbone si autoconfigura, ma le rotte verso l'esterno e l'indirizzamento dei client sono fissi e annunciati centralmente.

#### Path selection in L2 (HWMP) vs L3 (OSPF/Babel)

Una differenza concettuale importante fra le due varianti riguarda **come** viene scelto il percorso fra due nodi qualsiasi della mesh, e su che livello protocollare avviene il forwarding. Va sottolineata perché ha conseguenze su roaming, dominio broadcast e bilanciamento del carico.

**Mesh L2 (bridge / HWMP).** HWMP — *Hybrid Wireless Mesh Protocol*, definito da 802.11s — combina due modalità di path selection:

- **Proattiva (tree-based)**: un *root mesh STA* (tipicamente il gateway) diffonde periodicamente messaggi PREQ a tutta la mesh; ogni nodo impara un percorso verso il root, formando un albero radicato. È ottimizzata per il traffico verso Internet/server, che nelle reti mesh è il flusso dominante.
- **Reattiva (on-demand, AODV-like)**: quando un nodo deve raggiungere una destinazione interna alla mesh non coperta dall'albero, o vuole un percorso laterale migliore di quello via root, emette un PREQ in broadcast cercando proprio quella destinazione; la destinazione risponde con un PREP che torna lungo il percorso a metrica migliore, costruendo un path diretto specifico fra quella coppia (sorgente, destinazione). I messaggi PREQ/PREP sono identici a quelli di AODV (RFC 3561), che è a tutti gli effetti un **distance-vector** on-demand. La metrica standard è la **Airtime Link Metric**, radio-consapevole, che stima il tempo d'aria necessario per trasmettere un frame considerando bitrate, dimensione e tasso d'errore.

Quindi HWMP **non è limitato a un albero unico**: fornisce un albero per il traffico verso il root e, parallelamente, percorsi diretti su richiesta per il traffico interno. Quello che resta invariato è il **livello** del forwarding: il routing è a livello 2 sui MAC, l'intera mesh è un singolo dominio broadcast L2, le decisioni di inoltro sono prese dai mesh STA su un piano di forwarding bridge-like.

**Mesh L3 (router / OSPF, Babel, OLSRv2).** Qui il routing è di livello 3 e ogni nodo è un router IP a tutti gli effetti. Ogni router **calcola la propria tabella di routing** verso tutte le destinazioni della rete, applicando un algoritmo a sua scelta:

- **OSPF** usa *link-state*: ogni router conosce la topologia completa, esegue Dijkstra localmente, e ricava il proprio *Shortest Path Tree* (SPT) verso tutte le destinazioni;
- **Babel** usa *distance-vector* proattivo con *feasibility condition* (anti-loop), particolarmente efficace su reti wireless instabili;
- **OLSRv2** usa *link-state* ottimizzato con *MPR* (Multipoint Relay) per ridurre il flooding.

Tutti convergono allo stesso risultato concettuale: ciascun nodo conosce il percorso a costo minimo verso ogni destinazione. Per le destinazioni *esterne* (server, Internet, tipicamente raggiungibili solo via gateway) gli alberi calcolati dai vari router sono sostanzialmente identici e coincidono con l'albero proattivo HWMP. Per traffico *interno* gli alberi sono diversi (uno per sorgente), e in questo coincidono con la modalità reattiva di HWMP — la differenza è solo che in L3 i percorsi sono pre-calcolati invece che on-demand.

**Differenza pratica vera.** A livello di *selezione del percorso* i due approcci sono sostanzialmente equivalenti. Quello che cambia davvero è:

- **Livello del forwarding**: bridge L2 (HWMP) vs router IP (OSPF/Babel). Il primo lavora sui MAC, il secondo sugli indirizzi IP.
- **Dominio broadcast**: uno solo esteso a tutta la mesh (L2) vs uno per ciascun AP (L3).
- **Roaming**: trasparente in L2 (il client non cambia IP né subnet), comporta un re-DHCP in L3 perché cambiando AP cambia subnet.
- **Stato per nodo**: in HWMP il nodo memorizza solo i path che ha imparato/usa attivamente; in OSPF link-state il nodo deve mantenere la topologia completa della rete (overhead maggiore, ma convergenza più rapida dopo eventi di topologia).

Sulla nostra topologia ad albero (Figura 2) entrambi gli approcci hanno comportamento simile, perché il grafo è sostanzialmente lineare e non ci sono percorsi alternativi nel funzionamento normale. La differenza emerge quando un nodo cade e il routing deve ricalcolare, oppure quando si introducono link laterali (anche solo come adiacenze potenziali fra AP vicini). È esattamente quello che mostra la Figura 3.

#### Ridondanza e ricalcolo automatico delle rotte

La topologia di backhaul mostrata nella Figura 2 è un **albero a due livelli** radicato su T5 (mastio): T5 si collega al gateway via 60 GHz, e a sua volta serve due concentratori intermedi T2 e T3 che aggregano i quattro AP foglia (T1, T6 a ovest, T4, T8 a est). Questa struttura limita a 3 il numero di link backhaul incidenti su ciascun nodo, evitando interferenza co-canale eccessiva sul mastio. T5 resta però un **single point of failure** apparente: se cade, sembrerebbe che le foglie restino isolate.

In realtà la rete sopravvive, perché alcuni AP perimetrali sono fisicamente vicini abbastanza da **sentirsi a vicenda**: in particolare T2 e T3 (i due concentratori, entrambi affacciati sul cortile centrale) e T8 e T7-GW (entrambi nel settore sud-est del sito). Fra di loro esistono **adiacenze radio potenziali** non disegnate in Figura 2 perché nel funzionamento normale il protocollo di routing le scarta in favore del percorso a minor costo. Sono comunque rilevabili dai messaggi *hello* periodici di HWMP, OSPF o Babel. Quando il protocollo rileva la caduta di T5 — tipicamente in pochi secondi, dopo qualche hello mancato — ricalcola le rotte attivando quelle adiacenze dormienti. La Figura 3 mostra l'effetto del ricalcolo: il percorso da T1 al gateway passa ora da T2 a T3 (riconnettendo le due metà del sito) e poi via T8 al gateway, con qualche hop in più ma la connettività preservata.

![Scenario di guasto e ricalcolo](../img/failover.png)

*Figura 3 — Effetto del crollo del nodo centrale T5 sul backbone mesh. A sinistra la topologia normale ad albero: T5 è la radice, T2 e T3 concentratori intermedi; il percorso da T1 al gateway compie 3 hop sul mesh. A destra: T5 caduta, i link verso il centro sono disattivati (grigi tratteggiati), il protocollo di routing dinamico ricalcola attivando le adiacenze dormienti T2↔T3 e T8↔T7-GW (azzurro). Il nuovo percorso T1→T2→T3→T8→T7-GW (frecce arancioni) compie 4 hop. La rete continua a funzionare in modalità degradata fino al ripristino del mastio.*

La ridondanza non richiede modifiche di progetto: dipende solo dal fatto che gli AP siano **abbastanza vicini** da poter dialogare anche senza il mastio. La pianificazione dei canali a riuso N=4 favorisce naturalmente questo scenario, perché i canali sono già pensati per limitare l'interferenza fra celle adiacenti.

---

## Quesito 2 — Vincoli di prossimità

L'obiettivo è che il minitablet possa visualizzare la pagina di un certo POI solo se fisicamente vicino al POI stesso. Si combinano quattro meccanismi complementari.

### Approccio 1 — Associazione AP ↔ POI

Ogni POI ha un AP "vicino" identificato dal suo BSSID. Quando il minitablet richiede la pagina del POI X, il middleware applicativo legge il BSSID a cui il dispositivo è associato (esposto via API di sistema dell'app, non dal browser) e lo confronta con la mappa POI↔BSSID configurata nel server. Se non corrisponde, restituisce HTTP 403. È il metodo più robusto perché sfrutta la radio: per associarsi a un AP bisogna esserne nel raggio, e la potenza degli AP è tarata per ~15–20 m utili.

### Approccio 2 — RSSI threshold

Oltre al BSSID, si richiede che il livello di segnale RSSI verso l'AP del POI sia superiore a una soglia (es. −65 dBm). Esclude i casi in cui il dispositivo è ancora agganciato all'AP precedente per inerzia di roaming.

### Approccio 3 — Geofencing GPS

Il minitablet ha già il GPS attivo per l'antifurto: si sfrutta la stessa funzione per definire un geofence circolare attorno a ogni POI (raggio 20–30 m), e l'app verifica la posizione prima di richiedere la pagina. Limite: il GPS indoor è inaffidabile (torri, antiquarium); all'interno conviene quindi appoggiarsi all'Approccio 1.

### Approccio 4 — Codice QR del POI

Sul pannello informativo del POI viene affisso un **codice QR** che contiene un identificativo firmato del punto di interesse, ad esempio una URL del tipo `https://app.sito.local/poi/T2?nonce=…&sig=…`. La firma è generata dal server applicativo con una chiave simmetrica (HMAC) e ruotata periodicamente (es. ogni 24 h); il nonce previene il replay.

Il flusso operativo diventa molto naturale per il visitatore: si avvicina alla bacheca, inquadra il QR con l'app del minitablet, e l'app fa partire la richiesta della pagina aggiungendo il QR token al normale token di sessione del biglietto. Il server applicativo valida tre cose insieme:

1. la firma del QR (deve essere quella vigente del POI dichiarato);
2. la prossimità radio/GPS (Approcci 1–3, come ulteriore difesa contro QR fotografati a distanza);
3. il livello tariffario del biglietto (base/intermedia/piena), per decidere se servire la pagina base o avanzata.

Vantaggi: esperienza utente intuitiva (è ormai un gesto familiare), forte associazione fisica fra il visitatore e il POI specifico (non basta essere genericamente "vicini", bisogna essere davanti alla bacheca), zero costi infrastrutturali (un foglio plastificato per POI), e gestione semplice della tariffa intermedia: i primi tre QR scansionati "consumano" le tre pagine avanzate previste.

**In aggiunta,** è prassi consolidata usare beacon BLE (Bluetooth Low Energy, profilo iBeacon o Eddystone) collocati nei POI: l'app sente il beacon entro pochi metri, ed è il segnale di prossimità più preciso possibile, specialmente indoor dove GPS e geolocalizzazione Wi-Fi falliscono. È la soluzione adottata da molti musei reali (Louvre, Tate Modern). I beacon possono coesistere con i QR: il QR avvia esplicitamente la fruizione di un certo POI, il beacon conferma in background la prossimità fisica.

**Implementazione consigliata:** l'app, prima di richiedere `GET /poi/<id>`, allega un proximity token che contiene {BSSID corrente, RSSI, lat/lon GPS, ultimo QR scansionato, eventuali UUID di beacon visti}. Il server applicativo valida il token contro la tabella POI prima di rilasciare il contenuto.

---

## Quesito 3 — Sicurezza

### 3.a — Impedire l'accesso alla rete a dispositivi non autorizzati

Sul sito convivono due popolazioni molto diverse di utenti, che richiedono trattamenti diversi:

- **Dipendenti / operatori InfoPoint / personale tecnico**: utenza *persistente*, identificabile, su dispositivi *aziendali* controllati. Identità nota a priori, profili gestiti centralmente. Per loro l'approccio corretto è **802.1X con EAP a livello 2**: autenticazione forte basata su credenziali o certificati, prima ancora che il dispositivo ottenga un indirizzo IP.
- **Visitatori**: utenza *transitoria*, anonima al momento dell'acquisto del biglietto, in numero potenzialmente alto e con prerequisiti tecnici nulli. Configurare 802.1X richiede profili WPA-Enterprise sul dispositivo che un visitatore casuale non sa creare, e per il personale InfoPoint sarebbe insostenibile preparare un profilo per ogni biglietto giornaliero. Per loro l'approccio corretto è **captive portal** su SSID dedicato: la rete è aperta a livello radio (o protetta con WPA3-Personal a password nota), l'autenticazione vera avviene a **livello applicativo** con la password del biglietto, intercettata dal portale prima che il traffico possa uscire.

#### Dipendenti — 802.1X / EAP a livello 2

L'autenticazione 802.1X si basa su una **triade di attori** definita dallo standard IEEE:

- **Supplicant**: il software client che gira sul dispositivo dell'utente (es. wpa_supplicant su Linux, il servizio Wlansvc su Windows). È lui che presenta le credenziali, non l'utente direttamente.
- **Autenticatore (NAS, *Network Access Server*)**: l'access point Wi-Fi (o lo switch, in caso di Ethernet). Non valida le credenziali, ma le inoltra; agisce come *ponte levatoio* che apre o chiude l'accesso in base alla risposta del server.
- **Server di autenticazione**: un server RADIUS — tipicamente FreeRADIUS — che custodisce o consulta il database delle identità (LDAP, MySQL, Active Directory) e decide se autorizzare l'accesso.

**Doppio incapsulamento delle credenziali.** Il dato che viaggia è sempre lo stesso (un pacchetto EAP, ereditato dal vecchio protocollo PPP delle reti dial-up), ma cambia incapsulamento sulle due tratte:

- Sulla tratta **supplicant ↔ AP**, l'EAP viaggia direttamente sopra Ethernet/Wi-Fi a livello 2, in un tunnel chiamato **EAPoL** (*EAP Over LAN*). È necessario perché in questa fase il dispositivo non ha ancora un indirizzo IP — l'IP lo prenderà solo dopo essere stato autenticato, via DHCP.
- Sulla tratta **AP ↔ server RADIUS**, le credenziali EAP viaggiano dentro pacchetti **RADIUS**, a livello 7, perché entrambi i nodi hanno già un indirizzo IP (sono nella LAN cablata). Il pacchetto RADIUS è autenticato con uno *shared secret* configurato in `clients.conf` di FreeRADIUS.

**Porte controllate e porte non controllate.** Un AP che parla 802.1X attiva, dietro la propria porta radio fisica, **due porte virtuali**: la **porta non controllata**, sempre aperta, ma che lascia passare *soltanto* i frame EAPoL del dialogo di autenticazione; e la **porta controllata**, che è chiusa per default e si apre — lasciando passare il traffico dati IP — solo dopo che il server RADIUS ha emesso un `Access-Accept`. Finché non arriva l'autorizzazione, il dispositivo può solo parlare di autenticazione, nient'altro.

**Metodo EAP consigliato.** Per gli operatori si possono adottare due approcci, in ordine crescente di robustezza:

- **PEAP/MSCHAPv2** (o EAP-TTLS): basato su *username + password*, con un tunnel TLS che protegge le credenziali interne. Richiede solo un certificato sul server, non sui client — quindi è facile da deployare per nuovi operatori. Il client autentica il server tramite il certificato (deve essere installato il CA aziendale), il server autentica l'utente tramite hash della password contro il database LDAP. Lo scambio interno tipicamente MS-CHAPv2 è *debole* in sé (vulnerabile a brute-force offline), ma è incapsulato nel tunnel TLS, quindi un attaccante esterno non lo vede.
- **EAP-TLS** (autenticazione mutua con certificati su entrambi i lati): è l'approccio più robusto, perché non c'è alcuna password da rubare. Per gli operatori è praticabile se l'Ente ha già una PKI interna o se si gestiscono pochi profili. Per i **minitablet** invece è l'opzione naturale: i certificati vengono pre-installati al momento del provisioning, una sola volta, e poi il dispositivo si autentica automaticamente.

**Autenticazione del server e del certificato.** In entrambi i metodi è cruciale che il client autentichi il server prima di rivelare le proprie credenziali. Il client verifica che il certificato del server: (a) sia ancora dentro il periodo di validità; (b) sia firmato da una CA fidata, presente nel *trust store* del client; (c) abbia un *subject* che coincida col nome di dominio atteso del server RADIUS. Se uno di questi controlli fallisce, il supplicant deve rifiutare la connessione, altrimenti un AP malevolo (rogue AP) potrebbe spacciarsi per quello legittimo e raccogliere le credenziali.

**Autenticazione dell'AP verso il server.** L'AP stesso deve farsi riconoscere dal RADIUS, altrimenti chiunque potrebbe collegare un AP rogue allo switch e cominciare a inoltrare richieste di autenticazione. In FreeRADIUS, ogni AP autorizzato è elencato in `clients.conf` con il proprio IP e una shared secret univoca.

#### Assegnazione dinamica della VLAN via RADIUS

Una caratteristica molto potente di 802.1X è che il RADIUS non si limita a dire "sì/no": nella risposta `Access-Accept` può anche dire al NAS *in quale VLAN* mettere l'utente appena autenticato. Si usano gli attributi standard definiti dalla RFC 2868:

```
Tunnel-Type        = VLAN
Tunnel-Medium-Type = IEEE-802
Tunnel-Private-Group-Id = "10"     # VLAN ID di destinazione
```

Questo permette di **unificare l'SSID** per tutti gli operatori (un unico SSID *SitoArcheologico-Staff*) e di smistare ciascun utente nella VLAN giusta sulla base del suo profilo nel database. In FreeRADIUS la regola si scrive nel file `users` attaccandosi a un attributo dell'utente — tipicamente il *gruppo LDAP* di appartenenza:

```
DEFAULT Ldap-Group == "cn=tecnici,ou=staff,dc=sito,dc=it"
        Tunnel-Type             = VLAN,
        Tunnel-Medium-Type      = IEEE-802,
        Tunnel-Private-Group-Id = "10"     # → VLAN Mgmt

DEFAULT Ldap-Group == "cn=infopoint,ou=staff,dc=sito,dc=it"
        Tunnel-Type             = VLAN,
        Tunnel-Medium-Type      = IEEE-802,
        Tunnel-Private-Group-Id = "40"     # → VLAN InfoPoint

DEFAULT Auth-Type := Reject
```

Il vantaggio operativo è enorme: aggiungere un nuovo tecnico significa creare l'utenza in LDAP e assegnarla al gruppo `cn=tecnici`, senza toccare la configurazione né degli AP né degli switch. Lo stesso meccanismo si presta a confinare automaticamente in una **VLAN di quarantena** gli utenti il cui profilo è stato sospeso, semplicemente spostandoli di gruppo in LDAP.

#### Visitatori — captive portal

Per i visitatori 802.1X è inadeguato. La soluzione standard è il **captive portal**: un meccanismo che intercetta il primo tentativo di traffico HTTP/HTTPS del client appena associato e lo reindirizza a una pagina di login. Tipico funzionamento:

1. Il visitatore vede l'SSID aperto `SitoArcheologico-Visitatori` e si associa. La rete è **aperta** a livello radio — oppure protetta con WPA3-Personal a password singola scritta sul biglietto — perché un PSK individuale è poco sostenibile e WPA-Enterprise impraticabile.
2. Sull'SSID è attiva la cifratura OWE (Opportunistic Wireless Encryption, RFC 8110): il traffico è cifrato fra client e AP anche senza password, evitando intercettazioni del traffico dei portali altrui.
3. Il dispositivo prende un IP via DHCP, ma viene messo in una *walled garden*: l'unica destinazione raggiungibile è il server del portale captive. Ogni tentativo di traffico HTTP/HTTPS verso l'esterno viene catturato dal firewall del gateway e reindirizzato.
4. La pagina di login chiede la **password del biglietto** (univoca giornaliera). La verifica avviene contro il database dei biglietti emessi dalla cassa InfoPoint.
5. Se la password è valida, il firewall *promuove* la sessione del client: viene aggiunta una regola dinamica che gli consente l'accesso al server contenuti (e solo a quello). Il client passa quindi dalla VLAN 30-Q (quarantena visitatori non autenticati) alla VLAN 30 piena, eventualmente con la stessa logica di `Tunnel-Private-Group-Id` usata per gli operatori.
6. Il captive portal può anche essere implementato come funzionalità del **controller WLAN** (Ubiquiti UniFi, MikroTik, Aruba ClearPass) che si occupa di promuovere la sessione lato AP senza coinvolgere il firewall centrale.

**Vantaggi del captive portal per i visitatori:** zero configurazione lato utente (basta aprire un qualsiasi sito); compatibile con tutti i dispositivi (smartphone iOS/Android, tablet, laptop); meccanismo familiare (lo stesso degli hotspot pubblici); le credenziali sono le stesse del biglietto cartaceo già emesso, niente da memorizzare.

**Limiti e mitigazioni:** le credenziali viaggiano via HTTPS verso il portale, quindi sono protette in transito, ma la rete di accesso è *di per sé* aperta. Per questo:

- si usa OWE per cifrare il traffico anche prima dell'autenticazione;
- il portale autorizza una sessione solo per coppia (utente, dispositivo) e con scadenza giornaliera coincidente con il biglietto;
- una volta autenticato, il client è comunque sottoposto a **client isolation cross-AP** (vedi 3.d): non può parlare con gli altri client né con dispositivi di altre VLAN.

#### Sinottico

| Caratteristica | Dipendenti / operatori | Visitatori |
|---|---|---|
| Meccanismo | 802.1X / EAP a livello 2 | Captive portal a livello 7 |
| Metodo EAP / auth | PEAP/MSCHAPv2 (operatori), EAP-TLS (minitablet) | Password del biglietto in form HTTPS |
| Cifratura radio | WPA3-Enterprise (AES-CCMP) | OWE (Opportunistic Wireless Encryption) |
| Identità | persistente, in LDAP | transitoria, in DB biglietti |
| SSID | SitoArcheologico-Staff (unificato) | SitoArcheologico-Visitatori (aperto) |
| Assegnazione VLAN | dinamica via Tunnel-Private-Group-Id (RFC 2868) in base al gruppo LDAP | VLAN fissa 30 (visitatori), eventualmente quarantena 30-Q prima del login |
| Compromissione mitigata da | certificati server (anti rogue AP), shared secret AP, tunnel TLS | HTTPS sul portale, scadenza giornaliera, client isolation, ACL inter-VLAN |
| Onere lato utente | profilo WPA-Enterprise una tantum (gestito IT) | nessuno: si associa e digita la password |

**Componente comune: il controller WLAN.** Per gestire una flotta di una quindicina di AP distribuiti su tutto il sito è essenziale un **controller centralizzato** (es. Ubiquiti UniFi, MikroTik CAPsMAN, Aruba Mobility Controller). Le sue funzioni:

- configurazione e aggiornamento firmware degli AP da un'unica console;
- survey radio continuo: rilevazione di interferenze e di rogue AP che cercano di spacciarsi per il sito;
- rilevazione di MAC spoofing (IP duplicati nella stessa VLAN);
- gestione centralizzata del captive portal e dei voucher per i visitatori;
- regole di filtraggio (ACL) distribuite su gruppi di AP.

**Modalità di emergenza:** per coprire il caso in cui il servizio RADIUS centralizzato non sia disponibile (manutenzione, guasto), si può tenere pronto un SSID di *fallback* WPA3-Personal a password nota agli operatori, attivabile manualmente. Va attivato solo come ultima risorsa e disabilitato non appena il servizio principale torna operativo.

L'SSID dei visitatori è broadcast (nasconderlo sarebbe security through obscurity), così come quello degli staff; l'SSID di management dell'infrastruttura — usato dai tecnici sui propri portatili — è invece hidden e separato in VLAN 10.

### 3.b — Impedire l'accesso ai contenuti di un POI da subnet non autorizzate

Tutti i minitablet sono nella stessa subnet (VLAN 30), quindi "subnet non autorizzate" va inteso come altre VLAN (InfoPoint, IoT) che non devono poter scaricare i contenuti, e come minitablet non in prossimità del POI.

- **ACL sul firewall** che permettono il traffico HTTPS verso il server applicativo (VLAN 20) solo dalla VLAN 30. Le altre VLAN sono droppate.
- **Autorizzazione applicativa**: come descritto in Q2, il server verifica il proximity token (BSSID + RSSI + GPS + QR) prima di servire il contenuto. È un controllo L7 indipendente dalla rete.
- **Token di sessione legato al biglietto**: il server traccia per ciascun token quali POI sono già stati "sbloccati" (utile per la tariffa intermedia, dove l'utente sceglie 3 POI avanzati).

### 3.c — Proteggere il backhaul tra i nodi di rete

Il backbone mesh è radio, quindi intercettabile da un attaccante con un'antenna direzionale puntata sul sito. Le contromisure sono:

- **Cifratura del mesh link**: 802.11s prevede SAE (Simultaneous Authentication of Equals), lo stesso meccanismo di WPA3, per autenticare i peer mesh e derivare una chiave di sessione. Tutti i frame sul backbone vengono cifrati AES-CCMP. Solo i nodi che condividono il mesh passphrase (o, meglio, il certificato di nodo) entrano nella mesh.
- **Separazione fisica fra access e backhaul**: la radio backhaul opera su una banda dedicata (5 GHz upper DFS o 60 GHz direzionale) con SSID e credenziali diverse da quelle dei client. Un client compromesso non vede il traffico mesh.
- **VPN over mesh (opzionale, per management)**: tutto il traffico verso la VLAN 10 può essere incapsulato in un tunnel IPsec o WireGuard fra ciascun AP e il gateway, in modo che la compromissione di un singolo mesh link non esponga le credenziali admin.
- **Disabilitazione dei mesh portal non necessari**: solo il gateway è autorizzato a fare da uscita verso Internet (campo Mesh Portal di 802.11s settato solo sul gateway); gli altri nodi non possono diventare gateway rogue.

### 3.d — Politiche di accesso, matrice e ACL

Le misure 3.a–3.c proteggono il perimetro radio (chi entra in rete) e il backhaul (chi può ascoltare); resta però aperto il problema dei flussi interni leciti e illeciti fra le diverse popolazioni di dispositivi. Senza politiche esplicite vale il principio implicito "tutto parla con tutto", che è esattamente l'opposto di quello che si vuole: un minitablet manomesso non deve poter scansionare la VLAN dei server, le telecamere non devono poter parlare con i tablet, e i tablet non devono potersi vedere a vicenda.

**Postura default-deny / least privilege.** Si adotta un modello in cui ogni flusso è vietato per default, e si aprono esplicitamente solo i flussi necessari al servizio. Il progetto va sviluppato in due fasi: prima si scrive una *matrice degli accessi* (vista logica, alta), poi una *tabella delle regole ACL* (vista operativa, ancora indipendente dalla sintassi del vendor), e solo a quel punto si traduce in comandi concreti. Questa è anche una buona disciplina d'esame: chi non ricorda la sintassi specifica del firewall scriva almeno la tabella.

#### Client isolation: come funziona in pratica

Prima di parlare di ACL inter-subnet, va affrontato un caso particolare: il traffico fra due client *sulla stessa VLAN* (es. due minitablet entrambi in VLAN 30). Questo traffico è **L2**, resta interno alla subnet e quindi **non passa mai dal firewall**: nessuna ACL inter-VLAN può vederlo. Per bloccarlo serve un meccanismo che intervenga sull'AP stesso, chiamato **client isolation** (o *peer-to-peer block*, *AP isolation*, *station separation* a seconda del vendor).

**Meccanismo di base.** L'AP è configurato per non inoltrare i frame Ethernet ricevuti da un client verso altri client associati. Quando il tablet A trasmette un frame con MAC destinazione = tablet B, l'AP — invece di replicarlo sulla propria interfaccia wireless come farebbe un bridge trasparente normale — lo scarta o lo redirige verso la sola porta di uplink (verso il gateway). Il risultato: tablet A può parlare con il gateway (e da lì col server contenuti, se permesso), ma non può parlare direttamente con tablet B.

**Tre livelli di isolamento, in ordine di severità:**

- **Isolamento *per-AP* (intra-BSS)**: l'AP blocca solo il traffico fra client associati a *sé stesso*. Due tablet su AP diversi possono ancora parlarsi attraversando il backbone. È l'opzione di default su molti router consumer.
- **Isolamento *cross-AP* (esteso)**: il controller WLAN propaga la regola a tutti gli AP, in modo che due tablet su AP diversi nemmeno si vedano. Si implementa con tabelle di MAC autorizzati a comunicare (allow-list di peer leciti, tipicamente solo il default gateway), oppure con un'apposita funzione del controller (Cisco PMF + Peer-to-Peer Block, Aruba Deny Inter User Bridging, MikroTik `default-forwarding=no`).
- **Private VLAN** (RFC 5517): segmentazione L2 a livello di switch in cui la VLAN 30 viene divisa in *isolated* (le porte/SSID dei tablet) e *promiscuous* (la porta del gateway). Le porte isolated non possono parlare fra loro né con altre isolated, solo con la promiscuous. È la soluzione più rigorosa: implementa l'isolamento a livello di forwarding switch, non come filtro applicativo dell'AP, ed è quindi meno aggirabile.

**Sul nostro impianto** si attiva l'isolamento *cross-AP* sull'SSID dei visitatori (VLAN 30), con eccezione esplicita per il MAC del default gateway (necessario perché altrimenti anche il traffico verso il gateway sarebbe bloccato). Analogamente sull'SSID/VLAN delle telecamere (V50): peer-to-peer block fra telecamere, con eccezione solo verso l'NVR. Nelle VLAN di management (V10) e InfoPoint (V40), invece, **non** si applica isolamento, perché le postazioni operatori devono potersi vedere fra loro per condividere stampanti e cassetto biglietti.

**Limite del client isolation:** protegge solo a livello 2 e solo nello stesso AP/SSID. Non sostituisce le ACL inter-VLAN, che restano necessarie per filtrare il traffico routato. I due meccanismi sono complementari.

#### Matrice degli accessi (vista logica)

La matrice dice chi può parlare con chi e per quale servizio. Le righe sono le sorgenti, le colonne le destinazioni. Le celle indicano i protocolli/porte permesse; un trattino significa flusso bloccato.

| da → a | V10 Mgmt | V20 Server | V30 Tablet | V40 InfoPoint | V50 IoT | Internet |
|---|---|---|---|---|---|---|
| **V10 Mgmt** | full | SSH, HTTPS-admin | SSH-admin | SSH, HTTPS-admin | SSH, HTTPS, RTSP | DNS, NTP, updates |
| **V20 Server** | syslog | full | risp. HTTPS | risp. HTTPS | — | DNS, NTP, updates |
| **V30 Tablet** | — | HTTPS al server contenuti | — (client isolation) | — | — | — |
| **V40 InfoPoint** | — | HTTPS app cassa | — | full | — | HTTPS gestionali |
| **V50 IoT** | — | RTSP/HTTPS al NVR | — | — | — (peer-to-peer block) | NTP |
| **Internet** | — | — | — | — | — | — |

**Osservazioni sulla matrice:**

- la cella **V30→V30** è chiusa: è esattamente quello che implementa il client isolation. Stesso ragionamento per V50→V50 (telecamere fra loro, non c'è motivo);
- la riga **V30 → Internet** è chiusa: i tablet non hanno bisogno di Internet, tutto è in LAN; chiudere questa via blocca eventuali canali di comando e controllo se un tablet venisse manomesso;
- la riga **V10 Mgmt** può andare ovunque (è il piano di controllo), ma in entrata accetta solo da sé stessa: l'accesso alla VLAN di management va vincolato a una postazione dedicata;
- le voci "risp." indicano traffico di *risposta* (ritorno di una connessione iniziata altrove): il server contenuti non apre mai connessioni spontanee verso i tablet, risponde soltanto.

#### Tabella delle regole ACL (vista operativa, indipendente dal vendor)

La matrice si traduce in un elenco ordinato di regole. L'ordine è significativo perché valutato dall'alto verso il basso: la prima regola che fa match decide. È buona norma terminare con un deny esplicito e log, in modo che ogni flusso non previsto venga registrato.

| # | Sorgente | Destinazione | Protocollo/porta | Azione | Log | Note |
|---|---|---|---|---|---|---|
| 1 | V10 Mgmt | any | any | PERMIT | no | management ha accesso pieno |
| 2 | V30 Tablet | host Server contenuti | TCP/443 (HTTPS) | PERMIT | no | flusso principale dei visitatori |
| 3 | V50 IoT | host NVR | TCP/554 (RTSP), TCP/443 | PERMIT | no | streaming video verso NVR |
| 4 | V50 IoT | any | UDP/123 (NTP) | PERMIT | no | sincronizzazione oraria |
| 5 | V40 InfoPoint | V20 Server | TCP/443 | PERMIT | no | app cassa / biglietteria |
| 6 | V40 InfoPoint | Internet | TCP/443 | PERMIT | no | gestionali esterni |
| 7 | V20 Server | Internet | TCP/443, UDP/53, UDP/123 | PERMIT | no | updates, DNS, NTP |
| 8 | any | V10 Mgmt | any | DENY | sì | management non accetta connessioni esterne |
| 9 | any | any | any | DENY | sì | default-deny finale |

**Note di lettura:**

- le regole **PERMIT** specifiche vengono *prima* delle DENY ad ampio raggio, per il principio dell'ordine;
- le regole **stateful** permettono automaticamente il traffico di ritorno: una regola che apre V30→Server su TCP/443 lascia tornare anche la risposta del server senza bisogno di una regola simmetrica;
- le regole con **log = sì** alimentano il sistema SIEM o syslog: ogni denial registrato è un indizio di scansione, errore di configurazione o tentativo di intrusione;
- questa tabella è *astratta*: uno studente che non ricordi la sintassi del vendor specifico può scriverla così com'è e ottenere comunque pieno punteggio sul progetto di sicurezza.

---

## Seconda parte — Quesito I

*Estensione della fruizione ai dispositivi personali*

### Ipotesi A — Solo minitablet forniti

Sintesi degli strumenti già introdotti in Q3.a:

- EAP-TLS con certificati client preinstallati: un BYOD non possiede il certificato e non si associa.
- Pinning del MAC nel RADIUS.
- Applicazione nativa preinstallata sul minitablet che firma le richieste con una chiave hardware-backed (Android Keystore / Secure Element): il server accetta solo richieste firmate dall'app legittima. Un browser su smartphone, anche se in qualche modo riuscisse a connettersi, non potrebbe forgiare la firma.
- Provisioning controllato: i minitablet sono in modalità kiosk (single-app mode), senza accesso al Play Store né alla possibilità di installare browser alternativi.

### Ipotesi B — BYOD ammesso (smartphone personali)

Cambia il modello: non si può più affidare la sicurezza ai dispositivi (non sono controllati), e bisogna basarla solo su:

1. **App ufficiale del sito archeologico** scaricabile dagli store, che diventa l'unico client autorizzato. Il visitatore la scarica, inserisce la password del biglietto, e l'app gestisce: autenticazione, presentazione dei contenuti, verifica della prossimità (anche tramite QR e beacon BLE).
2. **SSID visitatori "BYOD"** separato (es. SitoArcheologico-Guest), con WPA3-Personal o OWE (Opportunistic Wireless Encryption, RFC 8110, per reti aperte cifrate), e captive portal che richiede il biglietto.
3. **Backend autorizzativo basato sul biglietto**: il token associato alla password del biglietto è l'unico elemento di autorizzazione; non si fa più allowlist hardware. Il rate-limit per token e il vincolo "una sessione attiva alla volta per biglietto" mitigano la condivisione.
4. **Vincoli di prossimità identici** (Q2): l'app personale richiede permesso di posizione + scansione Wi-Fi/BLE e invia gli stessi proximity token del minitablet. Il QR del POI funziona da innesco esplicito anche per il BYOD.
5. **Tariffa applicata server-side**: i privilegi (base/intermedia/piena) sono attributi del token, non del dispositivo. Il server decide cosa servire.

Vantaggi della soluzione B: minore investimento in hardware (meno minitablet), miglior esperienza utente (dispositivo familiare), riduzione del rischio antifurto. Svantaggi: impossibilità di controllare l'integrità del client, necessità di sviluppare app multipiattaforma (iOS + Android), utenti senza smartphone compatibile da gestire comunque con minitablet di riserva. La soluzione realistica è quindi ibrida.

---

## Allegato A — Configurazione Cisco IOS completa delle ACL

Questo allegato traduce in sintassi **Cisco IOS** la tabella astratta delle regole ACL del paragrafo 3.d. Tutte le ACL sono **extended**, nominate (per leggibilità) e applicate in ingresso sull'interfaccia di ciascuna VLAN, in modo che il filtro avvenga al confine della VLAN sorgente, prima che il traffico venga instradato verso le altre.

### A.0 — Scelta dell'architettura: router-on-a-stick

L'inter-VLAN routing può essere realizzato in due modi tipici, e la scelta condiziona *dove* si applicano le ACL ma **non** la loro logica:

- **Router-on-a-stick (RoAS)**: un router separato dallo switch L2, con **un'unica interfaccia fisica** verso lo switch configurata come **trunk 802.1Q**. Su quell'interfaccia si creano tante **sub-interfaces** logiche, una per VLAN, ciascuna con il proprio IP di default gateway. Il routing inter-VLAN è fatto dal router, che riceve il pacchetto su una sub-interface, lo instrada e lo rispedisce sulla stessa interfaccia fisica con un tag VLAN diverso. Economico, ottimo come esempio didattico, ma il trunk diventa un collo di bottiglia per il traffico inter-VLAN.
- **Multilayer switch (L3 switch)**: uno switch capace di fare routing in hardware (ASIC). Le VLAN non hanno sub-interfaces ma **SVI** (*Switch Virtual Interface*): interfacce logiche del piano di routing dello switch, una per VLAN. L'inter-VLAN routing avviene *dentro* lo switch, a velocità di linea. Più costoso ma molto più performante.

**In questo allegato si adotta la configurazione router-on-a-stick**, perché ha valore didattico più immediato — è il modello canonico dei manuali Cisco CCNA. Va però rimarcato che la stessa logica di ACL si trasferisce all'L3 switch con una sostituzione meccanica:

- le `interface FastEthernet0/0.X` diventano `interface VlanX`;
- si rimuove la riga `encapsulation dot1Q X` (l'associazione VLAN è implicita nella SVI);
- si aggiunge il comando globale `ip routing` (sui multilayer switch è disabilitato per default);
- i comandi `ip address`, `ip access-group` restano *identici*.

In altre parole, la matrice degli accessi, la tabella delle regole e il contenuto delle ACL sono **invarianti rispetto all'architettura**; cambia solo il tipo di interfaccia su cui si applicano. Per il nostro sito, in produzione, sarebbe preferibile l'L3 switch (le 200 sessioni HTTPS contemporanee ai picchi giustificano il forwarding in ASIC); in sede d'esame la versione RoAS è quella che meglio mostra il meccanismo del routing inter-VLAN.

**Architettura complessiva.** In entrambi i casi è opportuno avere **anche un firewall vero** in serie tra il router/L3 switch e l'uplink FWA verso Internet, dedicato al confine WAN: NAT, stateful inspection robusta, IPS, eventuale VPN site-to-site. Il firewall non si occupa del traffico inter-VLAN interno (sarebbe troppo lento per quello), ma solo del traffico verso/da Internet.

**Indirizzi di riferimento utilizzati:**

- `10.0.20.10` = host del Server contenuti / RADIUS / captive portal (VLAN 20);
- `10.0.20.20` = host NVR per le telecamere (VLAN 20);
- `10.0.10.0/24` = subnet Management; `10.0.20.0/24` = Server; `10.0.30.0/23` = Tablet; `10.0.40.0/24` = InfoPoint; `10.0.50.0/24` = IoT.

### A.1 — Configurazione del trunk e delle sub-interfaces

Lato switch L2, la porta che collega lo switch al router (es. `FastEthernet0/24`) è configurata come **trunk 802.1Q** e trasporta tutte le VLAN:

```cisco
! Configurazione lato switch L2
interface FastEthernet0/24
 description Uplink al router (RoAS)
 switchport mode trunk
 switchport trunk encapsulation dot1q
 switchport trunk allowed vlan 10,20,30,40,50
```

Lato router, l'interfaccia fisica resta *senza indirizzo IP*, e su di essa si creano cinque sub-interfaces, una per VLAN. Ogni sub-interface dichiara la VLAN che trasporta con `encapsulation dot1Q` e ospita il default gateway della relativa subnet:

```cisco
! Configurazione lato router
interface FastEthernet0/0
 description Trunk verso lo switch L2 (RoAS)
 no ip address
 no shutdown
!
interface FastEthernet0/0.10
 description MGMT - default gateway 10.0.10.1
 encapsulation dot1Q 10
 ip address 10.0.10.1 255.255.255.0
!
interface FastEthernet0/0.20
 description SERVER - default gateway 10.0.20.1
 encapsulation dot1Q 20
 ip address 10.0.20.1 255.255.255.0
!
interface FastEthernet0/0.30
 description TABLET - default gateway 10.0.30.1
 encapsulation dot1Q 30
 ip address 10.0.30.1 255.255.254.0       ! /23 = 510 host
!
interface FastEthernet0/0.40
 description INFOPOINT - default gateway 10.0.40.1
 encapsulation dot1Q 40
 ip address 10.0.40.1 255.255.255.0
!
interface FastEthernet0/0.50
 description IOT - default gateway 10.0.50.1
 encapsulation dot1Q 50
 ip address 10.0.50.1 255.255.255.0
```

### A.2 — ACL per ciascuna VLAN

#### A.2.1 — ACL-V10-IN: management

La VLAN 10 è il piano di controllo: ha accesso pieno a tutto. Una sola riga di permit più la deny implicita.

```cisco
ip access-list extended ACL-V10-IN
 remark === Management: accesso totale ===
 permit ip 10.0.10.0 0.0.0.255 any
 deny   ip any any log
```

#### A.2.2 — ACL-V20-IN: server

Il server contenuti deve potersi aggiornare (HTTPS verso Internet), risolvere nomi (DNS) e sincronizzare l'orologio (NTP). Non ha motivo di iniziare connessioni verso le altre VLAN — solo di rispondere, e il traffico di ritorno è gestito automaticamente dal reflexive ACL o dal CBAC/ZBF se il dispositivo lo supporta.

```cisco
ip access-list extended ACL-V20-IN
 remark === Server: solo updates / DNS / NTP verso Internet ===
 permit tcp 10.0.20.0 0.0.0.255 any eq 443
 permit udp 10.0.20.0 0.0.0.255 any eq domain
 permit udp 10.0.20.0 0.0.0.255 any eq ntp
 permit udp 10.0.20.0 0.0.0.255 host 10.0.10.5 eq syslog
 deny   ip any any log
```

#### A.2.3 — ACL-V30-IN: tablet visitatori

La regola centrale del progetto: i tablet possono parlare solo HTTPS verso il server contenuti, nient'altro. Niente Internet, niente altre subnet, niente peer-to-peer (quest'ultimo è bloccato anche dal client isolation sull'SSID, ma il filtro qui è una seconda linea).

```cisco
ip access-list extended ACL-V30-IN
 remark === Tablet: solo HTTPS verso il server contenuti ===
 permit tcp 10.0.30.0 0.0.1.255 host 10.0.20.10 eq 443
 permit udp 10.0.30.0 0.0.1.255 host 10.0.20.10 eq domain   ! DNS interno
 permit udp 10.0.30.0 0.0.1.255 host 10.0.20.10 eq bootps   ! DHCP (relay)
 deny   ip any any log
```

#### A.2.4 — ACL-V40-IN: InfoPoint / cassa

Gli operatori InfoPoint accedono all'app cassa (sul server contenuti) e a gestionali esterni via HTTPS, ma non devono potersi infilare nella VLAN Management né in quella dei tablet.

```cisco
ip access-list extended ACL-V40-IN
 remark === InfoPoint: server interno + gestionali esterni HTTPS ===
 permit tcp 10.0.40.0 0.0.0.255 10.0.20.0 0.0.0.255 eq 443
 permit udp 10.0.40.0 0.0.0.255 host 10.0.20.10 eq domain
 permit tcp 10.0.40.0 0.0.0.255 any eq 443
 deny   ip any any log
```

#### A.2.5 — ACL-V50-IN: telecamere / IoT

Le telecamere parlano solo con l'NVR (RTSP/HTTPS) e con un server NTP. Nessun traffico verso altre direzioni, in particolare verso i tablet o verso Internet.

```cisco
ip access-list extended ACL-V50-IN
 remark === IoT: solo RTSP/HTTPS verso NVR, NTP ===
 permit tcp 10.0.50.0 0.0.0.255 host 10.0.20.20 eq 554
 permit tcp 10.0.50.0 0.0.0.255 host 10.0.20.20 eq 443
 permit udp 10.0.50.0 0.0.0.255 any eq ntp
 deny   ip any any log
```

#### A.2.6 — ACL-INET-IN: traffico in ingresso da Internet

Sulla porta WAN del firewall, filtraggio classico anti-spoofing e default-deny per il traffico non sollecitato proveniente da Internet. Il return-traffic delle connessioni iniziate dall'interno viene comunque permesso dal motore stateful.

```cisco
ip access-list extended ACL-INET-IN
 remark === Anti-spoofing: scarta sorgenti private/loopback ===
 deny   ip 10.0.0.0 0.255.255.255 any log
 deny   ip 172.16.0.0 0.15.255.255 any log
 deny   ip 192.168.0.0 0.0.255.255 any log
 deny   ip 127.0.0.0 0.255.255.255 any log
 remark === Tutto il resto: solo return traffic via stateful inspection ===
 deny   ip any any log
```

### A.3 — Applicazione delle ACL alle interfacce

Le ACL sono applicate in ingresso sulle sub-interfaces del router: il filtro avviene prima del routing inter-VLAN, sul confine della VLAN sorgente. Questo è il pattern raccomandato perché blocca il traffico vietato al primo hop.

```cisco
interface FastEthernet0/0.10
 ip access-group ACL-V10-IN in
!
interface FastEthernet0/0.20
 ip access-group ACL-V20-IN in
!
interface FastEthernet0/0.30
 ip access-group ACL-V30-IN in
!
interface FastEthernet0/0.40
 ip access-group ACL-V40-IN in
!
interface FastEthernet0/0.50
 ip access-group ACL-V50-IN in
!
! Interfaccia WAN verso il firewall / CPE FWA
interface GigabitEthernet0/1
 description WAN - uplink verso firewall e CPE FWA
 ip address 192.0.2.2 255.255.255.252
 ip access-group ACL-INET-IN in
```

**Variante L3 switch:** se si scegliesse l'altra architettura, ciascun blocco diventerebbe semplicemente `interface Vlan10`, `interface Vlan20`, …, sostituendo la sub-interface con la SVI omonima. Il comando `ip access-group … in` resta letterale.

### A.4 — Stateful inspection (CBAC o Zone-Based Firewall)

Le ACL extended di Cisco IOS sono di per sé stateless: una regola che apre TCP/443 da V30 verso il server lascia partire la richiesta, ma non basta a far tornare automaticamente la risposta. Per gestire correttamente il return traffic ci sono due opzioni:

- **CBAC** (*Context-Based Access Control*): comando `ip inspect name <NOME> tcp`, applicato in uscita sull'interfaccia; tiene una tabella di sessioni e apre automaticamente i ritorni;
- **Zone-Based Firewall (ZBF)**: approccio più moderno (IOS 12.4(6)T+), si definiscono *zones* e *zone-pairs* con policy specifiche. È preferibile per progetti nuovi.

Esempio minimo CBAC:

```cisco
ip inspect name STATEFUL tcp
ip inspect name STATEFUL udp
ip inspect name STATEFUL icmp
!
interface GigabitEthernet0/1
 ip inspect STATEFUL out
```

### A.5 — Logging e contatori

Le righe terminanti con la parola chiave `log` inviano una notifica syslog ogni volta che una ACL drop fa match. Sul firewall di gateway si configurano la destinazione e la frequenza:

```cisco
access-list logging interval 60
logging host 10.0.10.5            ! syslog server in MGMT
logging trap informational
```

Il throttling a 60 s evita flood. Per ottenere contatori per riga (utile in debug) basta il comando in modalità privilegiata:

```
show access-lists ACL-V30-IN
! Extended IP access list ACL-V30-IN
!     10 permit tcp 10.0.30.0 0.0.1.255 host 10.0.20.10 eq 443 (12847 matches)
!     20 deny   ip any any log (143 matches)
```

I 143 match di deny della seconda riga sono il nostro indicatore principale: ogni numero che cresce velocemente segnala un tablet che sta tentando flussi non previsti.

---

## Appendice B — Ruolo di LDAP nell'autenticazione e nella gestione dei biglietti

**LDAP** (*Lightweight Directory Access Protocol*, RFC 4511) è il protocollo standard per interrogare un **directory service**: una base dati gerarchica e ottimizzata per la lettura, pensata per ospitare identità, gruppi, certificati e in generale tutto quello che è *chi* e *cosa* all'interno di un'organizzazione. Le implementazioni più diffuse sono OpenLDAP, Microsoft Active Directory, 389 Directory Server e — in ambito open source educativo — Univention Corporate Server (UCS) che integra LDAP, Samba e RADIUS in un'unica appliance.

Nel nostro progetto LDAP non è strettamente necessario, ma diventa molto utile non appena si superano le poche unità di utenti gestiti. Lo collochiamo nella **VLAN 20** (server farm) insieme al RADIUS e al server contenuti.

### B.1 — Modello dati: l'albero delle identità

LDAP organizza i dati in un **albero** (DIT, *Directory Information Tree*) in cui ogni nodo è caratterizzato da un **Distinguished Name** (DN). Esempio di organizzazione per il nostro sito:

```
dc=sito-archeologico,dc=it                      ← radice
 ├── ou=people                                   ← rami "utenti"
 │    ├── cn=mario.rossi,ou=people,dc=...        ← un tecnico
 │    ├── cn=lucia.bianchi,ou=people,dc=...      ← un operatore InfoPoint
 │    └── cn=biglietto-2026-05-23-0001,ou=people,dc=...  ← un biglietto
 │
 ├── ou=groups                                   ← rami "gruppi"
 │    ├── cn=tecnici,ou=groups,dc=...            ← gruppo tecnici
 │    ├── cn=infopoint,ou=groups,dc=...          ← gruppo operatori
 │    ├── cn=visitatori-base,ou=groups,dc=...    ← biglietti tariffa base
 │    ├── cn=visitatori-intermedi,ou=groups,...
 │    └── cn=visitatori-piena,ou=groups,...
 │
 └── ou=devices                                  ← inventario dispositivi
      ├── cn=tablet-001,ou=devices,dc=...        ← un minitablet
      └── cn=ap-T3,ou=devices,dc=...             ← un AP
```

Ogni utenza ha attributi standard (uid, cn, mail, userPassword) e attributi specifici dell'applicazione (es. tariffa del biglietto, data di validità, MAC del tablet associato). L'appartenenza ai gruppi è espressa con l'attributo memberOf (overlay standard in OpenLDAP) o con i campi member nel gruppo stesso.

### B.2 — LDAP come backend del RADIUS

Il server FreeRADIUS è già stato introdotto nel paragrafo 3.a per l'autenticazione 802.1X dei dipendenti. Per default, FreeRADIUS può cercare le credenziali in un file di testo (`users`), in MySQL/PostgreSQL, o in LDAP. La via LDAP è la più scalabile per due ragioni:

- **Singola fonte di verità**: le utenze sono gestite in un solo posto, dove vivono anche le altre informazioni sugli utenti (mail, profilo, gruppo). Aggiungere un nuovo dipendente significa creare un'utenza in LDAP, non in tre sistemi diversi.
- **Mappatura gruppo → VLAN**: FreeRADIUS legge l'attributo `memberOf` (o equivalente) dell'utente autenticato, e in base al gruppo decide in quale VLAN metterlo, valorizzando l'attributo RADIUS `Tunnel-Private-Group-Id` nella risposta (RFC 2868).

Esempio di configurazione del modulo LDAP in FreeRADIUS (file mods-available/ldap):

```
ldap {
    server = "ldap.sito-archeologico.it"
    port   = 636                      # LDAPS (TLS)
    identity = "cn=freeradius,ou=services,dc=sito-archeologico,dc=it"
    password = "***********"
    base_dn  = "dc=sito-archeologico,dc=it"
    user {
        base_dn = "ou=people,${..base_dn}"
        filter  = "(uid=%{User-Name})"
    }
    group {
        base_dn = "ou=groups,${..base_dn}"
        filter  = "(objectClass=groupOfNames)"
        membership_attribute = "memberOf"
    }
}
```

E nel file users (o equivalente file di policy), si scrive la mappatura gruppo→VLAN:

```
DEFAULT Ldap-Group == "cn=tecnici,ou=groups,dc=sito-archeologico,dc=it"
        Tunnel-Type             = VLAN,
        Tunnel-Medium-Type      = IEEE-802,
        Tunnel-Private-Group-Id = "10"     # → VLAN Mgmt

DEFAULT Ldap-Group == "cn=infopoint,ou=groups,dc=sito-archeologico,dc=it"
        Tunnel-Type             = VLAN,
        Tunnel-Medium-Type      = IEEE-802,
        Tunnel-Private-Group-Id = "40"     # → VLAN InfoPoint

DEFAULT Auth-Type := Reject
```

Il vantaggio operativo è enorme: spostare un utente da una VLAN all'altra significa cambiare un suo gruppo in LDAP, non riconfigurare RADIUS o switch.

### B.3 — LDAP per la gestione dei biglietti

La stessa infrastruttura LDAP può ospitare anche i **biglietti dei visitatori**. L'operatore InfoPoint, quando emette un biglietto alla cassa, crea una nuova utenza nel ramo `ou=people` con:

- un **uid** generato automaticamente (es. `bgl-20260523-0001`) che corrisponde alla password stampata sul biglietto cartaceo;
- una **userPassword** hashata che è la password dello stesso biglietto;
- un attributo personalizzato `ticketTariff` (custom schema) con il valore `base`, `intermedia` o `piena`;
- un attributo `ticketValidity` che indica la data di validità (giornaliera);
- l'appartenenza a uno dei gruppi `cn=visitatori-base`, `cn=visitatori-intermedi` o `cn=visitatori-piena`.

A questo punto il captive portal del sito interroga LDAP per autenticare il visitatore:

1. il visitatore inserisce nella pagina del portale la password stampata sul biglietto;
2. il portale fa una **bind LDAP** con `uid=<biglietto>,ou=people,...` e password fornita;
3. se la bind ha successo, il biglietto è valido e attivo; il portale legge l'attributo ticketTariff e ticketValidity;
4. il portale promuove la sessione del tablet (apre la regola dinamica sul firewall, eventualmente assegna VLAN via `Tunnel-Private-Group-Id`) e memorizza la tariffa in un cookie applicativo che il server contenuti userà per decidere se servire la pagina base o avanzata.

**Pulizia giornaliera.** A fine giornata un cron job elimina (o disabilita ponendo `ticketValidity` nel passato) tutte le utenze biglietto della giornata precedente. Lo stesso job può generare un **report** (numero biglietti per tariffa, distribuzione oraria, POI più visitati) interrogando LDAP, perché la struttura ad albero è ideale per filtraggi rapidi.

### B.4 — LDAP come inventario dispositivi

Il ramo `ou=devices` può ospitare anche un piccolo **CMDB** (Configuration Management Database) con un'entry per ciascun minitablet e ciascun AP. Attributi tipici:

- `macAddress` del dispositivo;
- `serialNumber` e modello;
- stato (`attivo`, `in-manutenzione`, `rubato`);
- certificato client X.509 utilizzato per EAP-TLS (attributo `userCertificate`).

Questa centralizzazione semplifica le operazioni di sicurezza: se un tablet viene segnalato come rubato (dal sistema antifurto GPS), si cambia il suo attributo `status` in `stolen` e immediatamente:

- FreeRADIUS, alla prossima richiesta di autenticazione del MAC corrispondente, rifiuta;
- il captive portal mostra un messaggio specifico per quel device;
- il SIEM riceve un alert evidente.

### B.5 — Riepilogo

| Funzione | Cosa fa LDAP | Componenti che lo interrogano |
|---|---|---|
| Autenticazione dipendenti | custodisce username, password hash, gruppo (tecnici/infopoint) | FreeRADIUS via modulo ldap |
| Autenticazione biglietti | una entry per biglietto, con tariffa e validità | captive portal del sito |
| Assegnazione VLAN dinamica | gruppo dell'utente → VLAN ID via Tunnel-Private-Group-Id | FreeRADIUS |
| Inventario dispositivi | MAC, certificato, stato di ciascun tablet/AP | RADIUS, controller WLAN, monitoraggio |
| Report e statistiche | interrogazioni ad albero sui rami biglietti/utenti | cron job, dashboard amministrativa |

**Nota finale.** LDAP non è obbligatorio: per un sito piccolo con pochi operatori, lo stesso lavoro lo fanno un database SQL e una manciata di tabelle. Diventa interessante quando le entità da gestire (utenti, biglietti, dispositivi, certificati, ruoli) crescono e si vuole una **singola sorgente di verità** interrogabile da tutti i componenti dell'infrastruttura — RADIUS, captive portal, controller WLAN, SIEM, sistema di ticketing della cassa. Il vantaggio della scelta LDAP è la sua maturità (esiste dagli anni '90), la disponibilità di librerie client per ogni linguaggio, e l'ottima integrazione con FreeRADIUS che è il server di autenticazione di riferimento del nostro progetto.
