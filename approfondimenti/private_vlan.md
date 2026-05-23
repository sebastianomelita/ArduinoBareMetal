# Private VLAN (PVLAN)

> Segmentazione fine di una VLAN: bloccare il traffico fra client della stessa subnet
> senza creare una VLAN per ciascuno di loro.

---

## 1. Il problema che risolvono

Quando si progetta una rete a VLAN, si parte dal principio che **una VLAN = un dominio di broadcast = una subnet IP**. Tutti i dispositivi della stessa VLAN si vedono a livello 2: un PC della VLAN 30 può fare ARP, ping e qualunque altra comunicazione diretta con un altro PC della VLAN 30, senza passare dal router.

Questo va bene finché i dispositivi della VLAN si fidano fra loro. Ma in molti scenari reali **non è così**:

- una rete **ospiti** in un albergo o in un sito turistico: ogni cliente è isolato dagli altri, ma tutti devono raggiungere il gateway Internet;
- un **data center con server in colocation**: ogni cliente ha i suoi server, che non devono parlare con quelli degli altri clienti pur stando nella stessa subnet di hosting;
- una **rete IoT** con telecamere o sensori: ognuno parla solo con il proprio NVR/concentratore, non con i fratelli;
- una **rete didattica BYOD** in cui gli studenti sono nella stessa subnet ma non devono potersi sondare a vicenda.

La soluzione "VLAN per ognuno" non scala: significa una subnet per cliente, indirizzamento sprecato (ogni /30 ha 4 indirizzi di cui solo 2 utilizzabili), una sub-interface sul router per ogni VLAN, e una matrice di routing che esplode.

Le **Private VLAN** risolvono questo problema mantenendo una sola VLAN logica (e quindi una sola subnet IP), ma introducendo segmentazione interna a livello 2.

---

## 2. Il concetto base

Una Private VLAN è una **VLAN suddivisa internamente** in tre tipi di porte, con regole di inoltro asimmetriche. La VLAN originale prende un nuovo nome (**primary VLAN**) e ospita una o più sotto-VLAN (**secondary VLAN**), ciascuna di un tipo specifico:

| Tipo di porta       | Può parlare con…                                                  |
|---------------------|-------------------------------------------------------------------|
| **Promiscuous (P)** | tutte le porte della Primary VLAN, di qualunque tipo              |
| **Isolated (I)**    | solo con porte Promiscuous, non con altre Isolated né Community   |
| **Community (C)**   | con porte Promiscuous e con altre Community della *stessa* Comm.  |

Le tre tipologie sono lo standard descritto nella **RFC 5517**.

L'idea chiave è che **tutti i client condividono la stessa subnet IP** e lo stesso default gateway (che sta sulla porta Promiscuous), ma il forwarding L2 è limitato dallo switch in modo che client "ostili" o "indipendenti" non si vedano a vicenda.

---

## 3. Le tre VLAN secondary

### 3.1 Isolated VLAN

Tutte le porte Isolated **non si parlano fra loro**. Possono parlare solo con la porta Promiscuous. È il caso più rigoroso, perfetto per:

- camere d'albergo, ogni stanza isolata dalle altre;
- ospiti di una rete Wi-Fi pubblica;
- visitatori di un sito archeologico (il nostro caso).

In una Primary VLAN ce n'è **una sola** Isolated VLAN: tutte le porte isolate vivono lì dentro.

### 3.2 Community VLAN

Le porte di una stessa Community **si vedono fra loro**, ma non vedono le porte Isolated né le porte di altre Community. È utile quando esistono "gruppi" di dispositivi che devono cooperare al loro interno ma restare invisibili agli altri:

- un cliente in colocation con tre server che si parlano fra loro;
- un'aula scolastica con dispositivi che condividono una stampante locale;
- un dipartimento aziendale ospitato sulla stessa subnet di altri dipartimenti.

In una Primary VLAN possono coesistere **molte** Community VLAN.

### 3.3 Promiscuous

La porta Promiscuous è quella che parla con tutti — di solito è la **porta del router/default gateway**, o di un server che deve essere raggiungibile da tutti (DHCP, DNS, server applicativo). Senza la Promiscuous, i client Isolated non potrebbero uscire dalla subnet.

---

## 4. Un esempio concreto

Immaginiamo un albergo con 4 stanze, un server condiviso per i clienti business e un router Internet, tutti nella stessa subnet `10.0.30.0/24`.

```
                        +-----------+
                        |  ROUTER   |   ← porta Promiscuous
                        |  10.0.30.1|
                        +-----+-----+
                              |
                       ┌──────┴──────┐
                       │   SWITCH    │
                       └─┬─┬─┬─┬─┬─┬─┘
                         │ │ │ │ │ │
                  ┌──────┘ │ │ │ │ └──────┐
                  │ ┌──────┘ │ │ └─────┐  │
       Isolated → S1 S2     S3 S4     C1 C2
                            (Business community)

  Primary VLAN: 30
  Secondary:
    - Isolated   = 301  (stanze S1..S4)
    - Community  = 302  (server business C1, C2)
    - Promiscuous= router

  Traffico permesso (a parte la primary, che è il "contenitore"):
    S1 → router  OK     |   S1 → S2     X
    S1 → C1      X      |   C1 → C2     OK
    C1 → router  OK     |   C1 → S1     X
```

S1 e S2 sono **isolate**: non si vedono, ma vedono il router (Promiscuous).
C1 e C2 sono **community**: si vedono fra loro, ma non vedono S1..S4 né si vedono fuori.

Tutti hanno IP nella stessa `/24`, ma il forwarding L2 sullo switch è governato dalle regole PVLAN.

---

## 5. Come funziona nel forwarding switch

Le PVLAN modificano il comportamento standard di un bridge IEEE 802.1D. In particolare:

- **VLAN tagging**: i frame portano due ID logici, quello della Primary e quello della Secondary. Sul piano di forwarding, lo switch fa il match su entrambi.
- **MAC learning**: avviene normalmente nella Primary VLAN; il fatto che un MAC sia stato visto su una porta Isolated o Community è un metadato aggiuntivo.
- **Filtering**: prima di inoltrare un frame, lo switch verifica la coppia (porta ingresso → porta uscita) contro la matrice di permessi PVLAN. Se la coppia non è permessa, il frame viene scartato.

Il **broadcast** e il **multicast** seguono regole più sottili:
- da una porta Isolated, broadcast e ARP arrivano solo alla Promiscuous;
- da una porta Promiscuous, broadcast e multicast arrivano a tutte le secondary;
- da una porta Community, broadcast arriva alle altre Community della stessa secondary e alla Promiscuous.

Questa asimmetria è il cuore del meccanismo: i client non possono "vedersi" perché i frame di broadcast e ARP non li raggiungono, e quindi nemmeno possono iniziare una comunicazione unicast.

---

## 6. Configurazione su Cisco IOS

La configurazione tipica su uno switch Catalyst:

```cisco
! 1. Modalità VTP transparent (le PVLAN non si propagano via VTP)
vtp mode transparent

! 2. Dichiarazione delle secondary
vlan 301
 private-vlan isolated
vlan 302
 private-vlan community

! 3. Dichiarazione della primary e associazione
vlan 30
 private-vlan primary
 private-vlan association 301,302

! 4. Porte host (le stanze e i server business)
interface range Fa0/1 - 4
 switchport mode private-vlan host
 switchport private-vlan host-association 30 301

interface range Fa0/5 - 6
 switchport mode private-vlan host
 switchport private-vlan host-association 30 302

! 5. Porta promiscuous (verso il router)
interface Fa0/24
 switchport mode private-vlan promiscuous
 switchport private-vlan mapping 30 301,302
```

I comandi chiave da ricordare:

| Comando                                              | A cosa serve                          |
|------------------------------------------------------|---------------------------------------|
| `private-vlan isolated` / `community` / `primary`    | dichiara il tipo di una VLAN          |
| `private-vlan association <list>`                    | lega la primary alle secondary        |
| `switchport mode private-vlan host`                  | porta lato client                     |
| `switchport private-vlan host-association P S`       | mette la porta in una specifica sec.  |
| `switchport mode private-vlan promiscuous`           | porta lato router/server condiviso    |
| `switchport private-vlan mapping P S1,S2,…`          | mappa primary→secondary per la promisc|

### Sul router (SVI inter-VLAN routing)

```cisco
interface Vlan30
 ip address 10.0.30.1 255.255.255.0
 private-vlan mapping 301,302
```

Il `private-vlan mapping` sulla SVI fa sì che il router riceva e risponda al traffico di tutte le secondary, anche se topologicamente è in primary.

---

## 7. Configurazione su altre piattaforme

### Linux (con `bridge` e `vlan_filtering`)

Nelle versioni recenti del kernel, il bridge Linux supporta le PVLAN tramite il flag di porta:

```bash
ip link add br0 type bridge vlan_filtering 1
ip link set eth0 master br0
bridge vlan add dev eth0 vid 30 pvid untagged
bridge vlan add dev eth0 vid 301 pvid untagged
# eth0 = porta isolated nella secondary 301
```

L'astrazione è meno raffinata che su Cisco IOS, ma il concetto è lo stesso.

### Controller WLAN

Tutti i controller WLAN enterprise (Cisco WLC, Aruba, MikroTik CAPsMAN, UniFi) hanno una funzione equivalente, anche se di solito chiamata diversamente:
- **Peer-to-Peer Blocking** (Cisco WLC) — blocca traffico fra client dello stesso SSID, propagato cross-AP via controller;
- **Deny Inter User Bridging** (Aruba);
- **Client Isolation Network-wide** (UniFi);
- **default-forwarding=no** + **AP isolation** (MikroTik).

In sostanza, è il **client isolation di livello SSID**, che realizza in software ciò che le PVLAN realizzano nello switch.

---

## 8. PVLAN vs Client Isolation Wi-Fi

I due meccanismi sono **complementari** e spesso si usano insieme:

| Aspetto              | Client Isolation Wi-Fi              | Private VLAN                       |
|----------------------|-------------------------------------|------------------------------------|
| Dove agisce          | sull'access point / controller WLAN | sullo switch L2                    |
| Cosa protegge        | client wireless sullo stesso SSID   | qualunque porta dello switch       |
| Quanto è rigoroso    | filtro software dell'AP             | filtro nel forwarding ASIC         |
| Aggirabile?          | sì, se l'AP è mal configurato       | molto difficile                    |
| Granularità          | per-SSID (tutti dentro o tutti fuori)| isolated, community, promiscuous   |

In una rete enterprise robusta si applicano entrambi: il client isolation è la prima linea di difesa lato wireless, le PVLAN sono la seconda linea sul backbone cablato.

---

## 9. Pro e contro

### Vantaggi

- **Indirizzamento IP efficiente**: una sola subnet ospita molti gruppi isolati. Niente sprechi di indirizzi e niente subnet `/30` per ognuno.
- **Default gateway unico**: tutti i client puntano allo stesso router; il routing inter-subnet non è coinvolto.
- **Politica di sicurezza chiara**: la regola "tutti i client isolati fra loro" è esplicita nella struttura della VLAN, non in una ACL gigantesca.
- **Scalabilità**: aggiungere un nuovo client isolato significa solo associare una porta alla VLAN Isolated, non creare nuove VLAN.

### Svantaggi

- **Supporto eterogeneo**: non tutti gli switch supportano PVLAN. Tipicamente serve uno switch managed di fascia media o alta (Cisco Catalyst, Juniper EX, HP/Aruba ProCurve). Sugli switch low-cost di solito manca.
- **Configurazione complessa**: la matrice "primary + secondary + association + mapping" è disorientante al primo impatto. Errori di mapping sulla porta Promiscuous causano disservizi (i client isolati restano senza gateway).
- **Diagnosi difficile**: quando "non funziona", bisogna sapere distinguere se il problema è L2 (porta nel tipo sbagliato) o L3 (mancato mapping sulla SVI).
- **VTP transparent obbligatorio**: il protocollo classico di propagazione delle VLAN (VTP v1/v2) non supporta le PVLAN. Su grandi reti questo significa configurare ogni switch manualmente.
- **Spanning Tree e PVLAN**: in topologie non banali, l'interazione fra STP e PVLAN può creare path subottimali. Cisco offre il PVST+ esteso per gestirlo, ma è un'ulteriore complicazione.

---

## 10. Casi d'uso tipici

| Scenario                            | Topologia PVLAN consigliata                                             |
|-------------------------------------|-------------------------------------------------------------------------|
| Hotel, Wi-Fi ospiti, sito turistico | tutti i client in **Isolated**, router/captive portal in **Promiscuous**|
| Colocation data center              | ogni cliente in **Community** propria, gateway in **Promiscuous**       |
| Rete IoT con NVR centralizzato      | telecamere in **Isolated**, NVR in **Promiscuous**                      |
| Aule scolastiche BYOD               | una **Community** per aula, gateway/captive in **Promiscuous**          |
| DMZ con più server pubblici         | server in **Isolated** o **Community**, firewall in **Promiscuous**     |

---

## 11. Pseudo-PVLAN: il "protected port" o "PVLAN edge"

Su switch più semplici (Cisco serie 2960 low-end, ad esempio) esiste una versione ridotta delle PVLAN chiamata **Protected Port** o **PVLAN Edge**. È molto meno espressiva:

```cisco
interface Fa0/1
 switchport protected
```

Tutte le porte marcate `protected` sullo stesso switch non si parlano fra loro, ma parlano con qualunque porta non-protected. Non c'è il concetto di Primary/Secondary, non ci sono Community, e funziona **solo all'interno dello stesso switch** (non si estende sul trunk). Va bene come isolamento di base, ma non sostituisce una PVLAN vera.

---

## 12. Riassunto in tre righe

> Le **Private VLAN** spezzano una VLAN in sotto-categorie di porte
> (**Isolated**, **Community**, **Promiscuous**) tutte nella stessa subnet IP,
> per ottenere isolamento L2 fra client che condividono la rete e il gateway.

---

## Riferimenti

- **RFC 5517** — Cisco Systems' Private VLANs: Scalable Security in a Multi-Client Environment.
- **IEEE 802.1Q** — VLAN tagging (base su cui sono costruite le PVLAN).
- Cisco IOS Configuration Guide — "Configuring Private VLANs".
- Aruba/Juniper documentation — equivalenti vendor delle PVLAN.
