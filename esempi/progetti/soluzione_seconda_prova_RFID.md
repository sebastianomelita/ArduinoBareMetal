>[Torna a Dettaglio architettura RFID](/archrfid.md) 
>


# SIMULAZIONE SECONDA PROVA — SISTEMI E RETI
**Indirizzo:** Informatica e Telecomunicazioni — Articolazione Informatica
**Disciplina:** Sistemi e Reti
**Tema:** sistema metropolitano di bigliettazione elettronica su card NFC/RFID
**Anno scolastico:** 2025/2026

> Documento completo: analisi, architettura della rete di reti, planimetrie e cablaggio, alberi degli apparati, indirizzamento e configurazione, sicurezza e continuità di servizio, quesiti della seconda parte, motivazione delle scelte tecnologiche con riferimenti alla dispensa.

---

# PRIMA PARTE

## 1. Analisi del sistema e ipotesi aggiuntive

Il sistema è un'infrastruttura metropolitana per la gestione di card di trasporto pubblico basata su tecnologia **NFC/RFID HF** (ISO 14443, 13,56 MHz), analoga alle soluzioni Oyster (Londra), Navigo (Parigi), Carta Mia (Torino). Le card sono **MIFARE DESFire EV3** (o equivalente) per autenticazione crittografica e resistenza alla clonazione.

**Ipotesi aggiuntive.** Card fisica e app per smartphone adottano entrambe **NFC ISO 14443-4**. Il **Servizio Unico Metropolitano (SUM)** è un datacenter centralizzato della società partecipata. I controllori usano smartphone/tablet con app che accede via HTTPS al server della sede. Si stimano ~500 luoghi di imbarco/sbarco: ~80 **stazioni ad alta frequenza (Cat. A)** e ~420 **fermate a bassa frequenza (Cat. B)**. Il sistema opera in tempo reale, con latenza massima accettabile di ~1–2 s per il feedback al viaggiatore.

---

## 2. Architettura generale: rete di trasporto IP con gateway di confine

L'infrastruttura è modellata come **rete di reti** a topologia *hub-and-spoke*. Le reti di sito sono **reti laterali (spoke)** attestate, tramite il proprio **router perimetrale di sito** (R-A/R-B/R-C), su una **rete di trasporto IP** metropolitana — un trasporto generico e **non fidato**, che può essere anche Internet pubblica. Il **gateway VPN del SUM** (hub) aggrega tutte le reti laterali: sopra il trasporto corrono **tunnel IPsec punto-punto**, uno per sito, cifrati end-to-end, che costituiscono le dorsali logiche.

<img src="../img/architettura_hub_spoke_vpn_ipsec.svg" alt="Architettura hub-and-spoke: reti laterali via tunnel IPsec su trasporto IP non fidato verso il SUM" width="800px">

Figura 1 — Architettura generale (icone Cisco). Tratteggio nero = flussi L3 IP (dorsali logiche VPN tra i firewall dei siti e il firewall del SUM); punteggiato azzurro = flussi L7 MQTT (client MQTT dei siti ↔ broker centrale); bobina = interfaccia wireless L2 NFC/RFID HF (ISO 14443) tra reader e card.


<img src="../img/1_architettura_flussi_L3_L7.svg" alt="1_architettura_flussi_L3_L7.svg" width="1000px">

*Figura 2 — Architettura hub-and-spoke. Le reti laterali (spoke) — Sito Cat. A, Sito Cat. B, Sede controllori — raggiungono il SUM (hub) tramite **tunnel IPsec cifrati**, instaurati dai router perimetrali R-A/R-B/R-C su una **rete di trasporto IP non fidata**. La cifratura è end-to-end tra ogni R-x e il gateway del SUM, quindi il trasporto resta attraversabile senza doversene fidare. Nel SUM, il **gateway VPN con firewall e WAF** fa da perimetro davanti alla **server farm** (broker MQTT cluster, app server, DB centrale).*


### 2.1 Categoria A — luoghi ad alta frequenza (stazioni treni, metro principali, pontili capolinea)

Reader connessi in **LAN locale** a uno **switch PoE+**, attestati a un **server edge** che esegue il middleware RFID (gestione sessione di viaggio, traduzione semantica dell'UID, risposta locale a bassa latenza) e a un **router/firewall perimetrale** con VPN verso il SUM. Il feedback al viaggiatore è gestito localmente in < 200 ms; verso il SUM si trasmettono eventi aggregati.

*Motivazione e modello offline.* Il server edge riduce latenza e banda verso il SUM, ma la continuità in caso di WAN assente **non** si appoggia a una cache del saldo: l'autorità sul valore è la **card stessa**. Al tap l'edge autentica la card, legge il *value file* DESFire (saldo + stato viaggio), verifica la deny-list locale e **addebita atomicamente sul value file con Transaction MAC**. L'autorizzazione si chiude in locale anche a SUM irraggiungibile; verso il SUM si trasmettono solo eventi già conclusi, per il clearing.

### 2.2 Categoria B — fermate a bassa frequenza (metro secondarie, tram/bus, pontili intermedi)

**Almeno 2 reader** (ridondanza fisica). I reader sono **IP nativi con client MQTT integrato** e si connettono **direttamente al SUM** via WAN cellulare (4G/5G) con un router di confine in armadietto stradale. La validazione del tap è **autonoma sul reader**: autenticazione mutua, lettura/debito atomico sul *value file* della card e controllo della **deny-list locale**, senza dipendere dal SUM. Il SUM resta autorità di **clearing**, non di autorizzazione per singolo tap. Ogni reader mantiene la deny-list (aggiornata con TTL) e un **buffer flash** con gli eventi firmati (UUID) da riconciliare al ripristino del collegamento. Politica di rischio: addebito ammesso fino a una **soglia di saldo negativo** configurata, recuperata al successivo top-up online.

Il comportamento del reader è descritto da una **macchina a stati** con tre stati operativi — *Online*, *Offline*, *Riconciliazione* — le cui transizioni dipendono dalla disponibilità della WAN e dallo svuotamento del buffer. In tutti gli stati la validazione del tap resta locale: se la WAN è assente già all'avvio, il reader entra direttamente in *Offline* e continua a operare.


<img src="../img/macchina_stati_reader_catB.svg" alt="1_architettura_flussi_L3_L7.svg" width="600px">

---

## 3. Planimetrie fisiche e cablaggio strutturato

> Le planimetrie complete sono fornite come schemi grafici a parte. Qui se ne riassume il contenuto progettuale.

### 3.1 Stazione Cat. A (≈ 60 × 45 m)

![Planimetria fisica della stazione Cat. A](../img/2_planimetria_stazione_catA.svg)
*Figura 2 — Planimetria fisica stazione Cat. A: atrio con tornelli, banchine, locale tecnico con armadio CD, armadi FD baricentrici, prese TO, dorsali in fibra e cablaggio orizzontale.*

- **Armadio CD** nel locale tecnico, in posizione **baricentrica**; accorpa i ruoli CD/BD/FD-0 e serve direttamente i tornelli dell'atrio.
- **Armadi FD** di banchina (FD-1, FD-2), baricentrici rispetto alle proprie prese, entro i **90 m** di cablaggio orizzontale.
- **Prese TO** ai tornelli/varchi; **canalizzazione verticale di dorsale in fibra** (FD→CD), **canalizzazione orizzontale in controsoffitto** in UTP Cat6A (TO→FD).
- Vincoli rispettati: cablaggio orizzontale ≤ 90 m, armadi in posizione baricentrica, dorsali in fibra.

### 3.2 Fermata Cat. B (pensilina ≈ 8 × 3 m)

![Planimetria fisica della fermata Cat. B](../img/5_planimetria_fermata_catB.svg)
*Figura 3 — Planimetria fisica fermata Cat. B: due reader ridondati ai due accessi, armadietto stradale IP55 con router 4G/5G e mini-switch PoE.*

- **2 reader NFC ridondati** ai due accessi (prese TO B.01, B.02).
- **Armadietto stradale IP55** con router 4G/5G di confine, mini-switch PoE 4 porte e UPS.
- Cablaggio breve (< 15 m), UTP Cat6A interrato. Nessun armadio strutturato.

### 3.3 Pontile capolinea (Cat. A fluviale)

![Planimetria fisica del pontile capolinea fluviale](../img/6_planimetria_pontile_capolinea.svg)
*Figura 4 — Planimetria fisica pontile capolinea: pontile galleggiante con varchi/prese TO, FD di banchina, CD nel locale tecnico a terra, WAN ridondata fibra + 4G.*

- Imbarcadero con **pensilina di banchina** e **locale tecnico** lato terra (CD).
- Reader ai varchi di imbarco/sbarco (TO); FD di banchina baricentrico; dorsale in fibra verso il CD a terra (cavo per esterni/antiumidità).
- Connettività WAN ridondata (fibra terrestre + backup 4G) data l'esposizione ambientale.

---

## 4. Alberi degli apparati (stazione Cat. A)

### 4.1 Albero degli apparati passivi (topologia logica)

![Albero degli apparati passivi](../img/3_albero_passivi_stazione.svg)
*Figura 5 — Albero degli apparati passivi: radice CD, nodi FD, foglie TO; i rami sono le dorsali.*

```
                         ┌──────────┐
                         │    CD    │  (locale tecnico — radice)
                         └────┬─────┘
            ┌─────────────────┼───────────────────┐   dorsali (rami)
       ┌────┴────┐       ┌────┴────┐         ┌────┴────┐
       │  FD-0   │       │  FD-1   │         │  FD-2   │
       │ (atrio) │       │(banch.1)│         │(banch.2)│
       └────┬────┘       └────┬────┘         └────┬────┘
        TO A.01..A.07     TO 1.01..1.06       TO 2.01..2.05   (foglie)
```

**Tabella dorsali (estratto):**

| Dorsale | Cavo | Categoria | Molteplicità | Lung. | Ruolo |
|---|---|---|---|---|---|
| CD–FD1 | SMF | OS2 | 2 fibre | ~40 m | dorsale banchina 10G |
| CD–FD2 | SMF | OS2 | 2 fibre | ~55 m | dorsale banchina 10G |
| FD1–TO 1.xx | UTP | Cat6A | 4 coppie | ≤ 90 m | orizzontale 1G PoE+ |

### 4.2 Albero degli apparati attivi (router-on-a-stick, con pruning del trunk)

```
                 WAN / rete di trasporto IP
                          │ Gi0/0   (+ Tun0 → SUM)
                  ┌───────┴────────┐  router-on-a-stick:
             DNS  │      R-FW      │     Tun0      10.255.1.1
             DHCP │  apparato L3   │     Gi0/1.10  10.1.1.254   (RFID)
                  └───────┬────────┘     Gi0/1.20  10.1.2.254   (server)
                          │ Gi0/1        Gi0/1.99  10.1.99.254 (mgmt)
             trunk 802.1q │ ───────────── allowed vlan 10,20,99
                  ┌───────┴────────┐
                  │   CS core L2   │── server (VLAN 20, porta di accesso)
                  │  mgmt SVI .99  │      srv-edge .10 / srv-sis .11
                  └──┬─────────┬───┘
       trunk 10,99   │         │  trunk 10,99      (VLAN 20: pruned)
              ┌──────┴──┐  ┌───┴─────┐
              │  AS-0   │  │  AS-2   │  access switch PoE+ (FD)
              │ mgmt.99 │  │ mgmt.99 │
              └────┬────┘  └────┬────┘
            reader VLAN10   reader VLAN10
```

**Tre VLAN, ciascuna dimensionata sulla propria topologia.** Il **management** (VLAN 99) è distribuito a macchia di leopardo — un'interfaccia di gestione per ogni switch — quindi attraversa tutti i trunk: non è separabile fisicamente. L'**RFID** (VLAN 10) raggiunge i reader sugli access switch, quindi viaggia sui trunk CS↔AS. Il **server** (VLAN 20) è concentrato nel locale tecnico: i server sono attestati su **porte di accesso del CS**, e la VLAN 20 è **potata** dai trunk verso gli AS (dove non serve). I trunk CS↔AS trasportano così solo `10,99`; solo il trunk R-FW↔CS trasporta `10,20,99`. Configurazione lato switch:

```cisco
! CS — uplink verso R-FW
interface <uplink-R-FW>
 switchport mode trunk
 switchport trunk allowed vlan 10,20,99
!
! CS — downlink verso gli access switch (VLAN 20 potata)
interface range <downlink-AS>
 switchport mode trunk
 switchport trunk allowed vlan 10,99
!
! CS — porte di accesso dei server
interface range <porte-server>
 switchport mode access
 switchport access vlan 20
```

**Perché router-on-a-stick.** Con un solo apparato L3 le tre subnet interne sono direttamente connesse (rotte automatiche), non servono rotte statiche interne né link di transito. Il tornante del trunk non è un collo di bottiglia con messaggi JSON piccoli. Limite: `R-FW` è *single point of failure*, mitigabile con VRRP/HSRP.

---

## 5. Piano di indirizzamento e VLAN (stazione Cat. A)

| VLAN | Funzione | Subnet | Broadcast | Gateway (sottointerf.) | Range host | DHCP | Estensione |
|---|---|---|---|---|---|---|---|
| 10 | Reader NFC | 10.1.1.0/24 | 10.1.1.255 | 10.1.1.254 (Gi0/1.10) | .1–.253 | .100–.200 | trunk R-FW↔CS↔AS |
| 20 | Server | 10.1.2.0/24 | 10.1.2.255 | 10.1.2.254 (Gi0/1.20) | .1–.253 | statico | solo CS (potata sugli AS) |
| 99 | Management | 10.1.99.0/24 | 10.1.99.255 | 10.1.99.254 (Gi0/1.99) | .1–.253 | statico | tutti i trunk |

**Indirizzi statici VLAN 20:** srv-edge `10.1.2.10` (middleware + broker MQTT), srv-sis `10.1.2.11` (DHCP/DNS · NAS log).

**Interfacce L3 punto-punto (non VLAN):**

| Interfaccia | Subnet | Indirizzi | Ruolo |
|---|---|---|---|
| `Tun0` (R-FW) ↔ PE SUM | 10.255.1.0/30 | .1 ↔ .2 | dorsale VPN IPsec di transito |
| `Gi0/0` (WAN) | assegnata da ISP | dinamica (DHCP) | uscita verso il trasporto IP |

### 5.1 Subnetting del centro SUM (datacenter)

> Convenzione: **gateway sull'ultimo indirizzo utile (`.254`)**, host da `.1` a `.253`, broadcast `.255`.

| Zona | Subnet | Maschera | Range host | Host utili | Gateway | Ruolo |
|---|---|---|---|---|---|---|
| DMZ | 10.0.1.0/24 | 255.255.255.0 | .1–.253 | 253 | 10.0.1.254 | WAF + API gateway controllori (HTTPS/JWT), front-end pubblico |
| Server farm | 10.0.2.0/24 | 255.255.255.0 | .1–.253 | 253 | 10.0.2.254 | broker MQTT cluster, app server, DB centrale (zona interna) |
| Admin / management | 10.0.3.0/24 | 255.255.255.0 | .1–.253 | 253 | 10.0.3.254 | postazioni amministratori, NMS; gestione apparati SUM e stazioni via tunnel |
| Dorsali VPN (lato PE) | 10.255.x.(k*4)/30 | 255.255.255.252 | (.1 ↔ .2)+ k*4 K=0,..,63| 2 | — (punto-punto) | terminazione dei tunnel IPsec, uno per sito)|

**Allocazione statica nella server farm (10.0.2.0/24):**

| Host | Indirizzo | Ruolo |
|---|---|---|
| Broker MQTT (VIP cluster) | 10.0.2.10 | endpoint MQTT/TLS `:8883` (active-active) |
| Broker nodo 1 / nodo 2 | 10.0.2.11 / .12 | nodi del cluster |
| App server | 10.0.2.20 | logica applicativa / backend REST |
| DB master / slave | 10.0.2.30 / .31 | database centrale (replica master-slave) |
| DNS / NTP interni | 10.0.2.5 | servizi infrastrutturali del datacenter |
| Gateway | 10.0.2.254 | router/firewall di confine del SUM (PE) |

**Riepilogo allocazione metropolitana:**

| Blocco | Allocazione | Dettaglio |
|---|---|---|
| 10.0.0.0/16 | Centro SUM | DMZ, server farm, admin (tabella sopra) |
| 10.1.0.0/16 | Stazioni Cat. A | `10.1.N.0/24` per stazione (VLAN 10/20/99) |
| 10.2.0.0/16 | Fermate Cat. B | `10.2.M.0/29` per fermata |
| 10.3.0.0/16 | Sede controllori | `10.3.0.0/24` |
| 10.255.0.0/16 | Dorsali VPN | `10.255.x.(k*4)/30` con K=0,..,63 punto-punto, uno per sito )|

---

## 6. Configurazione di R-FW (Cisco IOS — router-on-a-stick)

Instradamento inter-VLAN collassato sul solo router perimetrale, NAT overload, dorsale VPN verso il SUM, ACL conformi (default-deny su reader/server/WAN/tunnel, default-allow solo sul piano di management) e ritorno stateful via CBAC. Per ogni interfaccia: **mini-tabella ACE astratta + blocco IOS** corrispondente.

### 6.0 Matrice degli accessi (politica di flusso)

> Ogni cella indica il traffico che la **sorgente (riga)** può **iniziare** verso la **destinazione (colonna)**. I ritorni delle sessioni iniziate dall'interno sono aperti dallo stateful (CBAC) e non compaiono. `✗` = negato (default-deny).

| Sorgente \ Destinazione | Reader (10.1.1.0/24) | Server (10.1.2.0/24) | Management (10.1.99.0/24) | SUM broker (10.0.2.0/24) | SUM admin (10.0.3.0/24) | Internet/WAN |
|---|---|---|---|---|---|---|
| **Reader** (VLAN 10) | intra-VLAN (L2) | MQTT/TLS 8883 → edge; DNS 53, NTP 123 → srv-sis | ✗ | ✗ | ✗ | ✗ |
| **Server** (VLAN 20) | ✗ | intra-VLAN (L2) | ✗ | MQTT/TLS 8883 (solo edge, via VPN) | ✗ | DNS 53, NTP 123, HTTPS 443 |
| **Management** (VLAN 99) | accesso pieno | accesso pieno | — | accesso pieno (via VPN) | accesso pieno | ✗ (no NAT) |
| **SUM broker** (10.0.2.0/24) | ✗ | ✗ | ✗ | — | — | — |
| **SUM admin** (10.0.3.0/24) | ✗ | ✗ | SSH 22, SNMP 161 (via tunnel) | ✗ | — | — |
| **Internet/WAN** | ✗ | ✗ | ✗ | — | — | — |

Letture chiave: i **reader** parlano solo col broker edge e con DNS/NTP interni; il **management** raggiunge tutto l'interno per gestirlo ma è isolato in ingresso (solo SUM admin via tunnel) e non esce su Internet; dalla **WAN** si ammette solo l'instaurazione del tunnel IPsec dal PE del SUM.

---

Instradamento inter-VLAN collassato sul solo router perimetrale, NAT overload, dorsale VPN verso il SUM, ACL conformi (default-deny su reader/server/WAN/tunnel; permit-all dalla sola subnet di management) e ritorno stateful via CBAC. Per ogni ACL applicata **in**: mini-tabella ACE astratta + blocco IOS.

### 6.1 — Base: routing, CBAC, DHCP, NAT, interfacce


| Nome Destinazione | Rete di Destinazione | Subnet Mask | Gateway (Via) |
| --- | --- | --- | --- |
| **SUM admin** | 10.0.3.0 | 255.255.255.0 | 10.255.x.(k*4 + 2) con K=0,..,63|


```cisco
hostname R-FW-STAZ-A
ip routing
!
! === Motore stateful (CBAC): apre i ritorni delle sessioni iniziate dall'interno ===
ip inspect name STATEFUL tcp
ip inspect name STATEFUL udp
ip inspect name STATEFUL icmp
!
! === DHCP per i reader (il pool risiede sul router) ===
ip dhcp excluded-address 10.1.1.1 10.1.1.99
ip dhcp excluded-address 10.1.1.201 10.1.1.254
ip dhcp pool VLAN10-READER
 network 10.1.1.0 255.255.255.0
 default-router 10.1.1.1
 dns-server 10.1.2.11
!
! === Interfacce L3 (gateway VLAN + WAN + tunnel) ===
interface GigabitEthernet0/0
 description WAN verso ISP / core MPLS-IP
 ip address dhcp
 ip nat outside
 ip access-group ACL-WAN-IN in
!
interface GigabitEthernet0/1
 description Trunk 802.1q verso CS (allowed vlan 10,20,99)
 no ip address
!
interface GigabitEthernet0/1.10
 description Gateway VLAN 10 - Reader NFC
 encapsulation dot1Q 10
 ip address 10.1.1.254 255.255.255.0
 ip nat inside
 ip access-group VLAN10-IN in
 ip inspect STATEFUL in
!
interface GigabitEthernet0/1.20
 description Gateway VLAN 20 - Server
 encapsulation dot1Q 20
 ip address 10.1.2.254 255.255.255.0
 ip nat inside
 ip access-group VLAN20-IN in
 ip inspect STATEFUL in
!
interface GigabitEthernet0/1.99
 description Gateway VLAN 99 - Management (no NAT: niente uscita Internet)
 encapsulation dot1Q 99
 ip address 10.1.99.254 255.255.255.0
 ip access-group VLAN99-IN in
 ip inspect STATEFUL in
!
interface Tun0
 description Dorsale VPN verso SUM (10.255.1.0/30)
 ip address 10.255.1.1 255.255.255.252
 tunnel source GigabitEthernet0/0
 tunnel destination <IP_PE_SUM>
 tunnel protection ipsec profile VPN-SUM
 ip access-group ACL-TUNNEL-IN in
!
ip route 0.0.0.0 0.0.0.0 GigabitEthernet0/0
! === supponendo sia la prima subnet di dorsale logica ===
ip route 10.0.0.0 255.0.0.0 10.255.0.2
!
! === PNAT (overload), con esclusione del traffico verso il SUM (va nel tunnel) ===
ip access-list extended NAT-INTERNI
 deny   ip 10.1.0.0 0.0.255.255 10.0.0.0 0.255.255.255
 permit ip 10.1.0.0 0.0.255.255 any
ip nat inside source list NAT-INTERNI interface GigabitEthernet0/0 overload
```

### 6.2 — `VLAN10-IN` · Reader NFC (Gi0/1.10) · default-deny

I reader parlano **solo** col broker edge `10.1.2.10`, più DHCP/DNS/NTP. Niente SUM diretto, niente management.

| # | Azione | Proto | Sorgente | Destinazione | Porta | Scopo |
|---|---|---|---|---|---|---|
| 1 | permit | udp | host 0.0.0.0 | host 255.255.255.255 | 67 | DHCP DISCOVER/REQUEST (broadcast) |
| 2 | permit | udp | 10.1.1.0/24 | 10.1.1.254 | 67 | DHCP rinnovo unicast |
| 3 | permit | tcp | 10.1.1.0/24 | 10.1.2.10 | 8883 | Reader → broker edge (MQTT/TLS) |
| 4 | permit | udp | 10.1.1.0/24 | 10.1.2.11 | 53 | DNS |
| 5 | permit | udp | 10.1.1.0/24 | 10.1.2.11 | 123 | NTP (validità certificati TLS) |
| 6 | deny | ip | any | any | — | **Default deny** (log) |

```cisco
ip access-list extended VLAN10-IN
 remark === Reader NFC: solo broker edge + DHCP/DNS/NTP ===
 permit udp host 0.0.0.0 host 255.255.255.255 eq bootps
 permit udp 10.1.1.0 0.0.0.255 host 10.1.1.254 eq bootps
 permit tcp 10.1.1.0 0.0.0.255 host 10.1.2.10 eq 8883
 permit udp 10.1.1.0 0.0.0.255 host 10.1.2.11 eq domain
 permit udp 10.1.1.0 0.0.0.255 host 10.1.2.11 eq ntp
 deny   ip any any log
```

### 6.3 — `VLAN20-IN` · Server (Gi0/1.20) · default-deny

L'edge inoltra al broker centrale del SUM (via VPN), risolve/sincronizza e si aggiorna. Non inizia verso i reader: risponde soltanto (ritorni via CBAC).

| # | Azione | Proto | Sorgente | Destinazione | Porta | Scopo |
|---|---|---|---|---|---|---|
| 1 | permit | tcp | 10.1.2.10 | 10.0.2.0/24 | 8883 | Edge → broker cluster SUM (via VPN) |
| 2 | permit | udp | 10.1.2.0/24 | any | 53 | DNS ricorsivo upstream |
| 3 | permit | udp | 10.1.2.0/24 | any | 123 | NTP |
| 4 | permit | tcp | 10.1.2.0/24 | any | 443 | Aggiornamenti software (HTTPS) |
| 5 | deny | ip | any | any | — | **Default deny** (log) |

```cisco
ip access-list extended VLAN20-IN
 remark === Server: edge->broker SUM + DNS/NTP/updates ===
 permit tcp host 10.1.2.10 10.0.2.0 0.0.0.255 eq 8883
 permit udp 10.1.2.0 0.0.0.255 any eq domain
 permit udp 10.1.2.0 0.0.0.255 any eq ntp
 permit tcp 10.1.2.0 0.0.0.255 any eq 443
 deny   ip any any log
```

### 6.4 — `VLAN99-IN` · Management (Gi0/1.99) · permit-all dalla subnet mgmt

Unica zona con permit-all in uscita: è il control plane e deve raggiungere tutti gli apparati. La protezione è in ingresso (vedi §6.6) e nell'assenza di NAT (niente Internet pubblica).

| # | Azione | Proto | Sorgente | Destinazione | Porta | Scopo |
|---|---|---|---|---|---|---|
| 1 | permit | ip | 10.1.99.0/24 | any | — | Management: accesso pieno (control plane) |
| 2 | deny | ip | any | any | — | **Default deny** (log) — scarta sorgenti spoofate |

```cisco
ip access-list extended VLAN99-IN
 remark === Management: accesso totale dalla subnet mgmt ===
 permit ip 10.1.99.0 0.0.0.255 any
 deny   ip any any log
```

### 6.5 — `ACL-WAN-IN` · WAN (Gi0/0) · anti-spoofing + default-deny

Scarta le sorgenti impossibili, ammette l'instaurazione del tunnel IPsec dal PE del SUM, poi nega il resto. I ritorni NAT li apre CBAC.

| # | Azione | Proto | Sorgente | Destinazione | Porta | Scopo |
|---|---|---|---|---|---|---|
| 1 | deny | ip | host 0.0.0.0 | any | — | Sorgente nulla |
| 2 | deny | ip | 10.0.0.0/8 | any | — | Anti-spoofing RFC1918 |
| 3 | deny | ip | 172.16.0.0/12 | any | — | Anti-spoofing RFC1918 |
| 4 | deny | ip | 192.168.0.0/16 | any | — | Anti-spoofing RFC1918 |
| 5 | deny | ip | 127.0.0.0/8 | any | — | Loopback |
| 6 | deny | ip | 224.0.0.0/4 | any | — | Multicast come sorgente |
| 7 | permit | udp | host «IP_PE_SUM» | any | 500,4500 | IKE / NAT-T del tunnel IPsec |
| 8 | permit | esp | host «IP_PE_SUM» | any | — | ESP del tunnel IPsec |
| 9 | deny | ip | any | any | — | **Default deny** (log) |

```cisco
ip access-list extended ACL-WAN-IN
 remark === Anti-spoofing: sorgenti illegittime ===
 deny   ip host 0.0.0.0 any log
 deny   ip 10.0.0.0 0.255.255.255 any log
 deny   ip 172.16.0.0 0.15.255.255 any log
 deny   ip 192.168.0.0 0.0.255.255 any log
 deny   ip 127.0.0.0 0.255.255.255 any log
 deny   ip 224.0.0.0 15.255.255.255 any log
 remark === Instaurazione tunnel IPsec dal PE del SUM ===
 permit udp host <IP_PE_SUM> any eq isakmp
 permit udp host <IP_PE_SUM> any eq non500-isakmp
 permit esp host <IP_PE_SUM> any
 deny   ip any any log
```

### 6.6 — `ACL-TUNNEL-IN` · dorsale VPN dal SUM (Tun0) · default-deny

Sul tunnel arriva il traffico interno dal SUM. Si ammette solo ciò che il SUM **inizia** (gestione remota verso il management); le risposte alle sessioni dell'edge le apre CBAC.

| # | Azione | Proto | Sorgente | Destinazione | Porta | Scopo |
|---|---|---|---|---|---|---|
| 1 | permit | tcp | 10.0.3.0/28 | 10.1.99.0/24 | 22 | SUM admin → mgmt (SSH) |
| 2 | permit | udp | 10.0.3.0/28 | 10.1.99.0/24 | 161 | SUM admin → mgmt (SNMP) |
| 3 | deny | ip | any | any | — | **Default deny** (log) |

```cisco
ip access-list extended ACL-TUNNEL-IN
 remark === Dal SUM: solo SSH/SNMP verso il management di stazione ===
 permit tcp 10.0.3.0 0.0.0.255 10.1.99.0 0.0.0.255 eq 22
 permit udp 10.0.3.0 0.0.0.255 10.1.99.0 0.0.0.255 eq snmp
 deny   ip any any log
```

### 6.7 Traffico di ritorno e ACL del tunnel (`TUN0-IN`)

Finora il lato `Tunnel0` della stazione era privo di filtri in ingresso: tutto ciò che arrivava dal SUM passava. Lo chiudiamo in **default-deny**, ammettendo solo il traffico lecito. Dal SUM arrivano due categorie:

1. **Ritorni** delle sessioni iniziate dall'edge verso il SUM (ack del broker, risposte DNS/NTP, ecc.): non si scrivono a mano, li apre **CBAC** dinamicamente *sopra* il `deny` finale, perché ispeziona le sessioni in uscita su `Tunnel0`.
2. **Gestione iniziata dal SUM** verso il management di stazione (NMS/jump in `10.0.3.0/28` → VLAN 99): non è un ritorno, quindi va permessa **esplicitamente**.

I **reader** (VLAN 10) non sono mai raggiungibili dal SUM: la regola è esplicita, oltre al deny finale.

| # | Sorgente | Destinazione | Proto:porta | Azione | Note |
| - | -------- | ------------ | ----------- | ------ | ---- |
| — | sessioni edge→SUM | edge `10.1.2.0/24` | (dinamico) | permit | aperto da CBAC sopra il deny |
| 1 | SUM admin `10.0.3.0/28` | mgmt `10.1.99.0/24` | tcp:22 | permit | SSH di gestione |
| 2 | SUM admin `10.0.3.0/28` | mgmt `10.1.99.0/24` | udp:161 | permit | polling SNMP |
| 3 | SUM admin `10.0.3.0/28` | mgmt `10.1.99.0/24` | icmp | permit | ping/monitoraggio |
| 4 | any | reader `10.1.1.0/24` | ip | deny (log) | reader irraggiungibili dal SUM |
| 5 | any | any | ip | deny (log) | default-deny |

```
! Regola di ispezione (sessioni iniziate dalla stazione verso il SUM)
ip inspect name CBAC-OUT tcp
ip inspect name CBAC-OUT udp
ip inspect name CBAC-OUT icmp
!
ip access-list extended TUN0-IN
 remark --- gestione iniziata dal SUM verso il management di stazione ---
 remark ---  SUM possa amministrare gli SW in VLAN 99 --
 permit tcp  10.0.3.0 0.0.0.15 10.1.99.0 0.0.0.255 eq 22
 remark ---  SUM possa amministrare il server edge in VLAN 20 ---
 permit tcp 10.0.3.0 0.0.0.15 host 10.1.2.10 eq 22
 permit udp  10.0.3.0 0.0.0.15 10.1.99.0 0.0.0.255 eq 161
 permit icmp 10.0.3.0 0.0.0.15 10.1.99.0 0.0.0.255
 remark --- i ritorni delle sessioni edge->SUM li inserisce CBAC qui sopra ---
 deny   ip   any 10.1.1.0 0.0.0.255 log    ! reader mai raggiungibili dal SUM
 deny   ip   any any log
!
interface Tunnel0
 description Dorsale VPN verso SUM (10.255.1.0/30)
 ip access-group TUN0-IN in
 ip inspect CBAC-OUT out
```

**Alternativa senza CBAC (solo `established`).** Se non si attiva l'ispezione, i ritorni TCP si ammettono con una riga, ma restano scoperti i ritorni **UDP** (DNS/NTP) ed è una protezione **falsificabile** (basta un pacchetto con ACK forzato):

```
ip access-list extended TUN0-IN
 permit tcp  10.0.0.0 0.255.255.255 10.1.0.0 0.0.255.255 established   ! solo ritorni TCP
 permit tcp  10.0.3.0 0.0.0.15      10.1.99.0 0.0.0.255 eq 22
 permit udp  10.0.3.0 0.0.0.15      10.1.99.0 0.0.0.255 eq 161
 deny   ip   any 10.1.1.0 0.0.0.255 log
 deny   ip   any any log
interface Tunnel0
 ip access-group TUN0-IN in
```

Coerente con la §9.3.1 del SUM: stesso schema *ACL `in` default-deny + `inspect … out`*, qui nella forma semplice perché è la stazione a iniziare le sessioni.


### 6.8 — Logging

```cisco
logging host 10.1.2.11
logging trap informational
```

Crittografia IPsec (ISAKMP policy AES-256/SHA-256/DH14, transform-set ESP-AES-256) come da profilo `VPN-SUM`. `<IP_PE_SUM>` coincide con `tunnel destination`; se il profilo incapsula GRE, aggiungere `permit gre host <IP_PE_SUM> any` in `ACL-WAN-IN`.

> **Nota d'ordine.** Le definizioni `ip inspect name STATEFUL` stanno in §6.1 (prima delle interfacce) perché alcune versioni IOS rifiutano `ip inspect STATEFUL in` se la regola non esiste ancora. Le ACL sono definite in §6.2–6.6 dopo l'`ip access-group`: incollando il config completo i riferimenti si risolvono comunque.



---

## 7. Tecnologie e modalità di comunicazione

### 7.1 Card ↔ reader (interfaccia RF)

ISO 14443-4, 13,56 MHz, distanza operativa 0–10 cm, **AES-128** (DESFire EV3) con **autenticazione mutua** e chiavi diversificate per UID; anticollisione ad **albero binario** (deterministica). La prossimità ridotta è anche difesa nativa dall'eavesdropping.

### 7.2 Reader/edge → SUM (canale di backhaul)

Trasporto **MQTT over TLS 1.3** (porta 8883), modello *publish/subscribe* adatto a molti reader verso un consumatore centrale, **QoS 1** (consegna garantita), riconnessione automatica e persistenza. Payload **JSON ASCII** per interoperabilità multi-vendor.

**Struttura dei topic (gerarchico-spaziale).** La gerarchia ricalca la topologia fisica `sito → reader`, così da poter sottoscrivere o filtrare per stazione, per reader o per tipo di messaggio:

```
sum/sito/<idSito>/reader/<idReader>/tap       (reader → SUM)  pubblica eventi di transito
sum/sito/<idSito>/reader/<idReader>/stato     (reader → SUM)  heartbeat e diagnostica
sum/sito/<idSito>/reader/<idReader>/ack       (SUM → reader)  esito della transazione
sum/sito/<idSito>/reader/<idReader>/config    (SUM → reader)  push di configurazione
sum/sito/<idSito>/controllo/verifica          (controllore → SUM)
sum/sito/<idSito>/controllo/risposta          (SUM → controllore)
```

| Topic     | Direzione      | QoS | Retained | Contenuto                                 |
| --------- | -------------- | --- | -------- | ----------------------------------------- |
| `tap`     | reader → SUM   | 1   | no       | evento di passaggio card al varco         |
| `stato`   | reader → SUM   | 1   | sì       | heartbeat: stato, conteggio tap, buffer   |
| `ack`     | SUM → reader   | 1   | no       | esito + saldo + messaggio per il viaggiatore |
| `config`  | SUM → reader   | 1   | sì       | parametri operativi del reader            |

**Payload — evento di tap (reader → SUM):**

```json
{
  "eventId":   "3f8a2c1d-7b91-4e02-9c4a-1d2e3f4a5b6c",
  "eventType": "tap",
  "timestamp": "2026-06-02T08:34:12.045Z",
  "idReader":  "READER-CAT-B-0042",
  "idSito":    "SITO-METRO-022",
  "uid":       "04:A3:B2:C1:D0:E9:F8",
  "rssi":      -35
}
```

**Payload — risposta/ack (SUM → reader):**

```json
{
  "eventId":      "3f8a2c1d-7b91-4e02-9c4a-1d2e3f4a5b6c",
  "esito":        "OK",
  "tipoEvento":   "inizio_viaggio",
  "saldoResiduo": 18.40,
  "messaggio":    "Buon viaggio! Saldo: €18,40"
}
```

`esito` ∈ { `OK`, `ERRORE_CARD_DISABILITATA`, `ERRORE_SALDO_INSUFFICIENTE`, `ERRORE_CARD_SCONOSCIUTA` }. L'`eventId` (UUID) correla richiesta e risposta e abilita la **deduplica** lato SUM (finestra di 60 s).

**Payload — heartbeat/stato (reader → SUM):**

```json
{
  "eventType":     "heartbeat",
  "timestamp":     "2026-06-02T08:35:00.000Z",
  "idReader":      "READER-CAT-B-0042",
  "stato":         "attivo",
  "tapCount":      1287,
  "bufferPendente": 0,
  "firmware":      "1.4.2"
}
```

Il reader invia `heartbeat` ogni 60 s; se offline accoda gli eventi nel buffer flash e li ritrasmette (QoS 1) al ripristino del collegamento.

### 7.3 Autenticazione del reader

L'identità del reader è verificata su **due livelli complementari**.

A livello **applicativo**, il reader (o il server edge che ne aggrega i flussi) si autentica verso il broker MQTT con un **certificato X.509** dentro la sessione TLS 1.3: solo un dispositivo con coppia chiave/certificato firmata dalla PKI aziendale può pubblicare sui topic. È un'autenticazione *end-to-end* valida sia per i reader IP nativi di Cat. B sia per l'edge di Cat. A.

A livello di **accesso alla rete** si aggiunge **802.1X**, applicabile **ovunque il reader si attesti alla porta di uno switch gestito**: sia all'access switch PoE+ negli armadi FD delle stazioni **Cat. A**, sia al **mini-switch PoE dell'armadietto stradale** delle fermate **Cat. B**. La porta resta non autorizzata e lascia passare solo traffico EAP finché il reader (**supplicant**) non si autentica; lo switch fa da **authenticator** e inoltra via **RADIUS** le credenziali a un **authentication server**. Il metodo è **EAP-TLS**, con lo stesso certificato X.509 usato per il broker (un'unica PKI per entrambi gli usi). Solo dopo l'autenticazione la porta viene aperta e il reader è assegnato (anche dinamicamente, via attributi RADIUS) alla VLAN dei reader. Così 802.1X decide *se* il dispositivo può entrare in rete, mentre X.509/TLS verifica *che* il client che parla col broker sia un reader legittimo; **VLAN + ACL** restano il terzo livello di difesa in profondità.

> **Cat. A vs Cat. B.** In Cat. A l'*authenticator* è l'access switch in FD e il RADIUS può risiedere lato edge/SUM. In Cat. B l'*authenticator* è il mini-switch PoE dell'armadietto: il RADIUS è remoto (al SUM) e raggiunto dallo switch via WAN 4G/5G, quindi serve una politica di **fail-open / critical-VLAN** se il collegamento cellulare cade, e il mini-switch dev'essere **gestito** (capace di 802.1X). Proprio perché l'armadietto stradale è fisicamente più esposto del locale tecnico di stazione, qui 802.1X è ancora più utile: impedisce che, aprendo l'armadietto e scollegando un reader, un dispositivo ostile ottenga l'accesso alla rete. Resta in ogni caso anche l'autenticazione SIM/APN della rete mobile sul lato WAN del router di confine.

### 7.4 Controllori → sede

REST su **HTTPS/TLS 1.3**, autenticazione **JWT** con refresh token; endpoint di verifica card in tempo reale.

---

## 8. Continuità di servizio e sicurezza

**Continuità.** Cat. A: link WAN ridondato (fibra + backup 4G/5G con failover) e server edge che valida in locale. Cat. B: 2 reader ridondati con validazione autonoma e buffer flash con risincronizzazione. SUM: broker MQTT in cluster active-active, DB master-slave, datacenter Tier III. Deduplica degli eventi tramite **UUID** per evento (finestra di 60 s).

**Transazioni in modalità degradata.** Stato e valore risiedono nel *value file* cifrato della card (DESFire EV3): credito/debito atomici autenticati AES-128 con **Transaction MAC** come prova crittografica. Il reader/edge autorizza in locale (< 1 s) anche con WAN assente; l'evento, identificato da **UUID**, viene accodato e inviato al SUM via MQTT/TLS (QoS 1) al ripristino. Il SUM esegue **clearing e deduplica per UUID** e ridistribuisce la **deny-list** (card sospese/rubate) con TTL. Feedback al passeggero definito anche in degradato: "viaggio registrato" su esito positivo, rifiuto su deny-list o saldo oltre soglia.

<img src="../img/transazione_card_nfc_offline.svg" alt="Transazione su card NFC in modalità offline: ciclo di autorizzazione locale e riconciliazione asincrona con il SUM" width="800px">

*Figura 7 — Transazione su card NFC in modalità offline. Cornice superiore: il ciclo di autorizzazione (autenticazione → lettura value file → verifica → debito atomico → feedback) si chiude tra card e reader in < 1 s, senza rete. Cornice inferiore: l'evento firmato viene riconciliato con il SUM in modo asincrono al ripristino della WAN.*

**Sicurezza.** TLS 1.3 / VPN IPsec AES-256 verso il SUM; reader autenticati al broker con certificati **X.509**; segmentazione **VLAN** + ACL inter-VLAN; **firewall** perimetrale, **IDS/IPS**, **WAF** sulle API; separazione DMZ/zona interna; SIEM con retention 12 mesi. Card: autenticazione mutua **AES-128**, chiavi diversificate per UID, dati cifrati. Privacy: la card porta un **value file cifrato e pseudonimizzato** col minimo stato necessario al funzionamento offline; i dati di mobilità nominali restano nel back-end, **pseudonimizzazione** nei log, conformità **GDPR** (diritto alla cancellazione dei dati di mobilità).

**Pseudocodice firmware reader (Cat. B — validazione autonoma offline):**

```text
# --- Avvio ---
all'avvio:
    carica_chiavi_AES_diversificate()            # per autenticazione mutua
    deny_list ← carica_da_flash()                # ultima copia con TTL
    buffer ← carica_eventi_pendenti_da_flash()   # eventi non ancora confermati
    connetti_MQTT_TLS(broker_SUM, porta 8883)    # tentativo, non bloccante
    sottoscrivi(topic = "…/<idReader>/ack")
    sottoscrivi(topic = "…/<idReader>/config")   # aggiornamenti deny-list
    avvia_task(invia_heartbeat, ogni 60 s)
    avvia_task(riconcilia_buffer, quando_online)

# --- Evento di tap (percorso critico: tutto locale, < 1 s) ---
al tap di una card:
    if NOT autenticazione_mutua_AES(card):       # chiave diversificata per UID
        mostra("Card non valida"); return

    uid ← leggi_uid(card)
    if uid in deny_list:                         # controllo offline
        mostra("Card non abilitata"); return

    stato ← leggi_value_file(card)               # { saldo, stato_viaggio }

    if stato.saldo < (tariffa − soglia_negativa): # rischio offline controllato
        mostra("Saldo insufficiente"); return

    # Debito ATOMICO sulla card con prova crittografica
    tmac ← debito_value_file(card, tariffa)      # operazione atomica DESFire
    if tmac == ERRORE:
        mostra("Riprova"); return                # nessun addebito avvenuto
    scrivi_stato_viaggio(card, nuovo_stato)

    mostra(feedback(stato.saldo − tariffa))      # esito immediato, < 1 s

    # Registra l'evento firmato per il clearing (a prescindere dalla rete)
    evento ← { eventId: UUID(),                  # dedup lato SUM
               idReader, uid, tariffa, timestamp,
               saldoPost: stato.saldo − tariffa, tmac }
    buffer.accoda(evento)
    salva_su_flash(buffer)                        # persistenza anti-spegnimento

# --- Riconciliazione asincrona (quando la WAN è disponibile) ---
task riconcilia_buffer():
    while online AND buffer NOT vuoto:
        evento ← buffer.primo()
        pubblica_MQTT(topic="…/tap", evento, QoS=1)  # consegna garantita
        ack ← attendi_ack(timeout)
        if ack.ricevuto:                         # il SUM deduplica per UUID
            buffer.rimuovi(evento)
            salva_su_flash(buffer)
        else:
            break                                # ritenta al ciclo seguente

# --- Manutenzione control plane ---
alla ricezione di config(nuova_deny_list):
    deny_list ← nuova_deny_list
    salva_su_flash(deny_list)                    # con TTL aggiornato

task invia_heartbeat():
    pubblica_MQTT(topic="…/stato",
        { idReader, online, n_eventi_buffer, ts }, QoS=1)
```
---

## 9. Sede centrale (SUM): apparati attivi, accessi e ACL del datacenter

### 9.1 Schema logico degli apparati attivi

Architettura a **due livelli L2** dietro il firewall di confine: un **core switch** che aggrega e **uno switch di accesso per ogni LAN**. Il SUM è il bersaglio più sensibile e i siti remoti hanno livelli di fiducia diversi, quindi le risorse sono separate in tre zone.

```
        Tunnel IPsec dai CE              WAN pubblica 198.64.5.1
        10.255.1/2/3.0/30                 :8883 -> broker · :443 -> WAF
                  \                              /
                   +--------------+---------------+
                          | FW-SUM (PE) +WAF+IDS/IPS |  L3, inter-VLAN, default-deny
                          +------------+-------------+
                                       | trunk 802.1q (VLAN 100/200/300)
                              +--------+--------+
                              | CS-SUM core L2  |  aggregazione
                              +--+----+------+--+
                  uplink 802.1q |    |        | uplink 802.1q
                +---------------+    |        +----------------+
          +-----+-----+    +---------+--------+         +-------+-----+
          | SW-DMZ L2 |    |   SW-FARM L2     |         | SW-ADMIN L2 |
          +-----+-----+    +--+-----+------+--+         +------+------+
                |             |     |      |                   |
          +-----+-----+    app    DB     web            +------+------+
          | broker a-a|   .2.10  .20/.21 .2.30          | NMS  .3.10  |
          | VIP .1.2  |   SIEM .2.40  DNS/NTP .2.5       | jump .3.11  |
          | WAF .1.5  |                                  +-------------+
          +-----------+   server farm 10.0.2.0/24        admin 10.0.3.0/28
           DMZ 10.0.1.0/28
```

*Figura 7 — `../img/7_schema_attivi_sum.svg`.* Un **core switch** CS-SUM aggrega (trunk verso il firewall) e sotto **uno switch di accesso per ogni LAN** (SW-DMZ, SW-FARM, SW-ADMIN), ciascuno nella propria VLAN. Il **FW-SUM** resta l'unico apparato L3, gateway inter-VLAN e punto di policy. La **DMZ** è l'unica zona raggiungibile dai siti (solo broker e WAF); **server farm** e **admin** non lo sono.

### 9.2 Tabella degli accessi del SUM

| Zona | VLAN | Subnet | Gateway | Host statici | Chi vi accede |
| ---- | ---- | ------ | ------- | ------------ | ------------- |
| DMZ (esposta) | 100 | `10.0.1.0/28` | `10.0.1.1` | broker MQTT a-a VIP `10.0.1.2` (nodi `.3/.4`), WAF/RP `10.0.1.5` | siti remoti: **solo** `:8883` (broker) e `:443` (WAF) |
| Server farm (interna) | 200 | `10.0.2.0/24` | `10.0.2.1` | app/core `10.0.2.10`, DB master `10.0.2.20`, DB slave `10.0.2.21`, web/API `10.0.2.30`, SIEM `10.0.2.40`, DNS/NTP `10.0.2.5` | solo DMZ (app/WAF) e admin; **mai** i siti |
| Admin (gestione) | 300 | `10.0.3.0/28` | `10.0.3.1` | NMS `10.0.3.10`, jump host `10.0.3.11` | nessuno: è **sorgente** del management, non destinazione |
| Dorsali VPN | — | `10.255.1/2/3.0/30` | PE `.2` di ogni `/30` | tunnel IPsec verso CE-A/B/C | terminano sul FW-SUM |
| WAN pubblica | — | `198.64.5.1` | ISP | PNAT + port-forward `:8883->.1.2`, `:443->.1.5` | unico ingresso pubblico |

### 9.3 ACL di protezione del datacenter (FW-SUM)

A differenza della stazione Cat. A — dove il filtraggio è già fatto in loco da `R-FW` — le **fermate Cat. B sono reader IP nativi senza alcuna policy locale**: non filtrano nulla. Per questo il SUM **non si fida dei siti** e centralizza la difesa sul proprio firewall, in **default-deny**: ogni classe sorgente raggiunge **una sola** destinazione su **una sola** porta. Le zone sono isolate fra loro: il broker esposto **non** parla direttamente col DB (passa dall'app server) e la zona **admin** è solo sorgente del management.

Le ACL sotto definiscono solo le **connessioni iniziali ammesse**; il traffico di **ritorno** è aperto dinamicamente dall'ispezione **CBAC** (§9.3.1), che inserisce le aperture *sopra* il `deny ip any any log` finale. Non servono quindi righe `established` scritte a mano.

#### Tunnel0 — ingresso dai siti remoti (`SITI-IN`)

| # | Sorgente | Destinazione | Proto:porta | Azione |
| - | -------- | ------------ | ----------- | ------ |
| 1 | `10.1.0.0/16` edge Cat. A | broker `10.0.1.2` | tcp:8883 | permit |
| 2 | `10.2.0.0/16` reader Cat. B | broker `10.0.1.2` | tcp:8883 | permit |
| 3 | `10.3.0.0/24` controllori | WAF `10.0.1.5` | tcp:443 | permit |
| 4 | any | server farm `10.0.2.0/24` | ip | deny |
| 5 | any | admin `10.0.3.0/28` | ip | deny |
| 6 | any | any | ip | deny (log) |

```
ip access-list extended SITI-IN
 permit tcp 10.1.0.0 0.0.255.255 host 10.0.1.2 eq 8883
 permit tcp 10.2.0.0 0.0.255.255 host 10.0.1.2 eq 8883
 permit tcp 10.3.0.0 0.0.0.255  host 10.0.1.5 eq 443
 deny   ip  any 10.0.2.0 0.0.0.255
 deny   ip  any 10.0.3.0 0.0.0.15
 deny   ip  any any log
```

#### Vlan100 — DMZ (`DMZ-IN`)

| # | Sorgente | Destinazione | Proto:porta | Azione |
| - | -------- | ------------ | ----------- | ------ |
| 1 | broker `10.0.1.2` | app `10.0.2.10` | tcp:8883 | permit |
| 2 | WAF `10.0.1.5` | web/API `10.0.2.30` | tcp:443 | permit |
| 3 | DMZ `10.0.1.0/28` | DNS `10.0.2.5` | udp:53 | permit |
| 4 | DMZ `10.0.1.0/28` | DB `10.0.2.20` | ip | deny |
| 5 | DMZ `10.0.1.0/28` | admin `10.0.3.0/28` | ip | deny |
| 6 | any | any | ip | deny (log) |

```
ip access-list extended DMZ-IN
 permit tcp host 10.0.1.2 host 10.0.2.10 eq 8883
 permit tcp host 10.0.1.5 host 10.0.2.30 eq 443
 permit udp 10.0.1.0 0.0.0.15 host 10.0.2.5 eq 53
 deny   ip  10.0.1.0 0.0.0.15 host 10.0.2.20
 deny   ip  10.0.1.0 0.0.0.15 10.0.3.0 0.0.0.15
 deny   ip  any any log
```

#### Vlan200 — Server farm (`FARM-IN`)

| # | Sorgente | Destinazione | Proto:porta | Azione |
| - | -------- | ------------ | ----------- | ------ |
| 1 | app `10.0.2.10` | DB master `10.0.2.20` | tcp:5432 | permit |
| 2 | web `10.0.2.30` | app `10.0.2.10` | tcp:8080 | permit |
| 3 | farm `10.0.2.0/24` | DNS/NTP `10.0.2.5` | udp:53,123 | permit |
| 4 | farm `10.0.2.0/24` | SIEM `10.0.2.40` | udp:514 | permit |
| 5 | farm `10.0.2.0/24` | admin `10.0.3.0/28` | ip | deny |
| 6 | farm `10.0.2.0/24` | siti `10.1/10.2.0.0/16` | ip | deny |
| 7 | any | any | ip | deny (log) |

```
ip access-list extended FARM-IN
 permit tcp host 10.0.2.10 host 10.0.2.20 eq 5432
 permit tcp host 10.0.2.30 host 10.0.2.10 eq 8080
 permit udp 10.0.2.0 0.0.0.255 host 10.0.2.5  eq 53
 permit udp 10.0.2.0 0.0.0.255 host 10.0.2.5  eq 123
 permit udp 10.0.2.0 0.0.0.255 host 10.0.2.40 eq 514
 deny   ip  10.0.2.0 0.0.0.255 10.0.3.0 0.0.0.15
 deny   ip  10.0.2.0 0.0.0.255 10.1.0.0 0.0.255.255
 deny   ip  10.0.2.0 0.0.0.255 10.2.0.0 0.0.255.255
 deny   ip  any any log
```

#### Vlan300 — Admin (`ADMIN-IN`)

| # | Sorgente | Destinazione | Proto:porta | Azione |
| - | -------- | ------------ | ----------- | ------ |
| 1 | admin `10.0.3.0/28` | qualsiasi zona | tcp:22, udp:161 | permit |
| 2 | (nessun'altra sorgente) | admin | — | deny implicito |

```
ip access-list extended ADMIN-IN
 permit tcp 10.0.3.0 0.0.0.15 any eq 22
 permit udp 10.0.3.0 0.0.0.15 any eq 161
 permit ip  10.0.3.0 0.0.0.15 any
```

### 9.3.1 Ispezione stateful con CBAC (Context-Based Access Control)

Le ACL precedenti sono **statiche**: da sole non aprono i ritorni. La risposta del broker verso il sito (`10.0.1.2 -> 10.2.x.x`) rientra dal lato DMZ e, senza stato, verrebbe scartata dal `deny ip any any` di `DMZ-IN`. Per questo si attiva **CBAC** (`ip inspect`): ispeziona le sessioni in uscita verso le risorse protette, tiene una **tabella di stato** e **apre da solo i ritorni**, inserendoli sopra i `deny` finali.

```
! Passo 1 - regola di ispezione: quali protocolli tracciare
ip inspect name CBAC-SUM tcp        ! copre MQTT/TLS 8883, HTTPS 443, SSH 22 ...
ip inspect name CBAC-SUM udp        ! copre DNS 53, NTP 123, SNMP 161, syslog 514
ip inspect name CBAC-SUM icmp       ! ping/traceroute di ritorno
! Nessun ALG (ftp/sip/rtsp): MQTT e HTTPS non aprono porte dinamiche.
!
! Passo 2 - ACL esterne con default-deny esplicito: gia' definite sopra
!           (SITI-IN, DMZ-IN, FARM-IN, ADMIN-IN). CBAC inserisce i ritorni
!           dinamicamente SOPRA il 'deny ip any any log'.
!
! Passo 3 - applicazione: ACL 'in' + ispezione 'out' sulle interfacce
!           che guardano verso le risorse protette
interface Tunnel0
 description Dorsali dai siti remoti
 ip access-group SITI-IN in
 ip inspect CBAC-SUM out          ! apre i ritorni delle sessioni admin -> siti
interface Vlan100
 description DMZ - broker, WAF
 ip access-group DMZ-IN in
 ip inspect CBAC-SUM out          ! apre i ritorni delle sessioni  * -> DMZ
interface Vlan200
 description Server farm
 ip access-group FARM-IN in
 ip inspect CBAC-SUM out          ! apre i ritorni delle sessioni  * -> farm
interface Vlan300
 description Admin - solo sorgente del management
 ip access-group ADMIN-IN in
```

**Come funziona, in breve.** Il comando `ip access-group NOME {in|out}` *aggancia* una ACL a un'interfaccia in un verso (dal punto di vista del router): `in` filtra ciò che entra, `out` ciò che esce. Le ACL qui sono **statiche** (stateless): decidono solo *chi può iniziare* una sessione e verso quale host/porta. CBAC (`ip inspect … out`) aggiunge la **tabella di stato**: ispeziona le sessioni in uscita da un'interfaccia e apre da solo i loro **ritorni** nella ACL `in` della *stessa* interfaccia, inserendoli sopra il `deny` finale e togliendoli a fine sessione. Esempio: il SYN di un reader (`10.2.x.x → 10.0.1.2:8883`) è ammesso da `SITI-IN` su `Tunnel0 in` e ispezionato in uscita su `Vlan100`; la risposta del broker rientra da `Vlan100 in` grazie al permesso temporaneo creato da CBAC, mentre qualsiasi pacchetto **non sollecitato** (es. un reader verso il DB) cade sul `deny ip any any log`.

**Confronto con la stazione Cat. A.** In Cat. A `R-FW` filtra all'origine (`VLAN10/20/99-IN`); il SUM replica la stessa logica *default-deny + named ACL per interfaccia*, ma sul **bordo centrale** e con **ispezione stateful CBAC**, perché deve difendersi anche dai siti che in loco non filtrano (Cat. B). Stessa filosofia applicata due volte: a valle dove si può, comunque a monte sull'asset critico, qui irrobustita dallo stato.


---

# SECONDA PARTE — Quesiti

## Quesito 1 — Progettazione logica del database dei reader

**Entità principali e schema relazionale:**

```
SITO(idSito PK, nome, tipo, indirizzo, lat, lon, categoria)
     tipo ∈ {stazione_treno, stazione_metro, fermata_tram, fermata_bus, pontile}
     categoria ∈ {A, B}
READER(idReader PK, macAddress UNIQUE, modello, firmware, dataInstallazione,
       stato CHECK(∈ 'attivo','guasto','manutenzione'), ipAddress,
       idSito FK→SITO, idStazioneLocale FK→STAZIONE_LOCALE NULLABLE)
STAZIONE_LOCALE(idStazione PK, idSito FK→SITO UNIQUE, ipServerLocale, nomeServer)
LINEA(idLinea PK, nome, mezzo, gestore)
SITO_LINEA(idSito FK, idLinea FK, direzione, PK(idSito,idLinea))   -- N:M
SERVIZIO_MANUTENZIONE(idServizio PK, idReader FK→READER, dataOra, tipo, descrizione, tecnico, esito)
```

**Associazioni:** READER–SITO (N:1), READER–STAZIONE_LOCALE (N:1, NULL per Cat. B), SITO–LINEA (N:M via SITO_LINEA), READER–SERVIZIO_MANUTENZIONE (1:N).

## Quesito 2 — Protocollo applicativo per l'interazione con il SUM

**Trasporto:** MQTT/TLS, QoS 1 (consegna garantita), modello publish/subscribe adatto a molti reader → un consumatore centrale, riconnessione automatica e persistenza.

**Messaggio di tap (reader → SUM):**
```json
{ "eventId":"3f8a2c1d-...","eventType":"tap","timestamp":"2026-06-02T08:34:12.045Z",
  "idReader":"READER-CAT-B-0042","idSito":"SITO-METRO-022",
  "uid":"04:A3:B2:C1:D0:E9:F8","rssi":-35 }
```
**Risposta (SUM → reader):**
```json
{ "eventId":"3f8a2c1d-...","esito":"OK","tipoEvento":"inizio_viaggio",
  "saldoResiduo":18.40,"messaggio":"Buon viaggio! Saldo: €18,40" }
```
`esito` ∈ { OK, ERRORE_CARD_DISABILITATA, ERRORE_SALDO_INSUFFICIENTE, ERRORE_CARD_SCONOSCIUTA }. Inoltre: `heartbeat` periodico (stato reader, conteggio tap, buffer pendente) e canale `verifica`/`risposta` per i controllori.

**Pseudocodice firmware reader (Cat. B):** all'avvio connette il broker in TLS e si sottoscrive al topic `ack`; al tap pubblica l'evento (QoS 1) e attende la risposta entro 2 s, mostrando il feedback; se offline accoda nel buffer flash e ritrasmette al ripristino; invia heartbeat ogni 60 s.

---

# Motivazione delle scelte tecnologiche (con riferimenti alla dispensa)

1. **HF/NFC ISO 14443, non UHF** — portata < 10 cm: al varco si legge *solo* la card avvicinata; la prossimità è anche difesa nativa dall'eavesdropping (*rfid_standard*, *rfid_sicurezza*).
2. **Anticollisione ad albero binario** — deterministica, garantisce la lettura del singolo: caso d'uso ideale "autenticazione singola affidabile" vs lo slotted ALOHA probabilistico dell'UHF (*rfid_anticollisione*).
3. **Reader fisso PoE + NFC dello smartphone** — stesso standard HF 13,56 MHz su card e app, nessun hardware aggiuntivo lato utente (*rfid_tag_reader*).
4. **Middleware on-edge nelle stazioni** — le letture grezze sono inutilizzabili (vanno filtrate, deduplicate, tradotte semanticamente); l'on-edge è scelto per ridurre latenza e traffico → feedback < 1 s in Cat. A (*rfid_architettura*).
5. **Gateway di confine, buffering, VPN, segmentazione** — il gateway è il dispositivo di confine tra rete di accesso e rete IP; buffering persistente per non perdere transazioni; reader embedded poco aggiornati ⇒ VLAN + firewall; connessione indiretta via VPN/tunnel verso il SUM (*rfid_architettura*).
6. **MQTT, topic gerarchico-spaziale, JSON** — formato e gerarchia indicati dalla dispensa; client MQTT a bordo del reader IP nativo (Cat. B) o del gateway/edge (Cat. A) (*rfid_architettura*).
7. **DESFire EV3, AES-128, chiavi per UID, identificativo opaco** — i tag low-end sono clonabili; autenticazione mutua e rolling codes contro clonazione e replay; privacy by design e GDPR per servizi al pubblico (*rfid_sicurezza*).

*Schede di riferimento (collegate da `archrfid.md`):* `rfid_standard.md`, `rfid_anticollisione.md`, `rfid_tag_reader.md`, `rfid_architettura.md`, `rfid_sicurezza.md`.

---


