# Tracce Sistemi e Reti — Raggruppate per ARGOMENTO CENTRALE
## Esame di Stato, indirizzo Informatica e Telecomunicazioni (ITIA/ITTL)

Le prove MIUR e le principali simulazioni dal 2015 al 2025, organizzate in base al **tema tecnico dominante** richiesto dalla parte di Sistemi e Reti. Ogni traccia può toccare più argomenti: è stata inserita nella categoria **predominante**, con eventuali riferimenti incrociati.

---

## 🔐 1. VPN e collegamento tra sedi remote

Tracce in cui il nodo centrale è la connessione sicura tra sedi distaccate via Internet oppure tramite reti private.

### ⭐ 2016 — Simulazione MIUR (Telecomunicazioni) — "Due edifici aziendali a pochi km"
> Due edifici aziendali, distanti qualche km, ma facenti parte della stessa struttura produttiva, impiegano due reti indipendenti. **Edificio 1**: rete interna collegata ad Internet tramite ISP, costituita da due sottoreti distinte (commerciale + contabile con dati sensibili) separate da un router. **Edificio 2**: struttura analoga.
>
> **Richieste di rete:** piano di indirizzamento, separazione commerciale/contabile con VLAN 802.1Q + ACL, **collegamento sicuro fra le contabili via Internet con VPN IPSec** (tunnel mode, AH + ESP, AES-256), alternative: OpenVPN/Cisco AnyConnect SSL-VPN.

### 2019 — Ordinaria (Informatica) — POI turistici (anche VPN)
Nei quesiti di seconda parte appare il classico: *"azienda con due sedi operative e agenti commerciali che, muovendosi, hanno necessità di collegarsi al sistema informativo aziendale"* → analisi VPN site-to-site + remote access.

### 2016 — Ordinaria (Informatica) — Scuola con BYOD
Il quesito 4 della seconda parte chiede esplicitamente: *"Le società che possiedono più sedi, o hanno personale in trasferta, necessitano di tecnologie per scambio dati in tempo reale ma sicuro"* → VPN, IPSec, SSL-VPN.

---

## 🛡️ 2. ACL, Firewall, DMZ e sicurezza perimetrale

Tracce in cui il fulcro è la segmentazione di rete con firewall, DMZ e controllo accessi.

### ⭐ 2018 — Ordinaria (Telecomunicazioni) — Teleriscaldamento IoT scolastico
> Un istituto scolastico decide di dotarsi di un nuovo sistema di teleriscaldamento con tecnologie IoT, composto da termostati intelligenti e valvole comandate in ogni locale. Il sistema deve usare l'attuale rete LAN dati logicamente separata dalla rete didattica.
>
> **Richieste:** schema rete wireless con min. 6 AP/piano, piano di indirizzamento IP per sensori/attuatori (12 locali/piano), **connessione criptata** ai sensori verso server remoto con configurazione firewall, **impedire alla rete didattica di accedere ai sensori** (segmentazione).
>
> **Seconda parte (quesito 3):** DMZ con firewall contro attacchi **IP Spoofing e IP Smurfing** — possibile architettura e configurazione.

### 2024 — Ordinaria (Informatica) — Rete regionale fibra + sanità
**Quesito III seconda parte:** piccola azienda con un solo IP pubblico statico, web server locale da esporre in HTTP/HTTPS e gestibile via SSH → **configurazione NAT/port forwarding** sul router con comandi specifici.

### 2024 — primo punto della prima parte
Il progetto deve garantire che *"ciascuna struttura collegata non possa accedere alle reti di tutte le altre strutture connesse"* → segmentazione con VLAN e ACL sul router Cisco.

---

## 🏗️ 3. Cablaggio strutturato e progettazione LAN

Tracce centrate sulla progettazione fisica e logica di una LAN con backbone multipiano, rack, armadi, patch panel.

### ⭐ 2019 — Straordinaria (Informatica) — Ospedale 6 reparti su 3 piani
> Un ospedale ha sei reparti distribuiti su tre piani (ogni reparto si sviluppa su un unico piano) e vuole innovare la sua infrastruttura tecnologica. Ogni medico, dopo aver visitato un paziente, deve collegarsi in modalità **wireless** ad un server web interno, dislocato in un locale tecnico al seminterrato, per registrare identificativi medico/reparto/paziente, parametri vitali, prescrizione terapeutica.
>
> **Vincoli di sicurezza:** operazioni solo via tablet aziendali, medici identificati all'accesso WiFi, tablet non possono collegarsi a siti web non autorizzati.
>
> **Richieste:** architettura di rete con topologia "fiocco di neve" (backbone verticale + stelle di piano), **cablaggio strutturato** con rack e patch panel per piano, cavi CAT6 FTP/STP, switch Gigabit, **Light AP + WLC (Wireless LAN Controller)** per gestione centralizzata, **RADIUS** per autenticazione tablet nominativi, tunable channels 2.4/5 GHz.

### 2016 — Ordinaria (Informatica) — Scuola 1000 studenti
Scuola con 1000 studenti su edificio a due piani; rete amministrativa (15 postazioni) separata da rete didattica (10 laboratori). Richiesta: unificazione linea Internet, separazione logica con VLAN, cablaggio strutturato, backup ADSL di riserva, DMZ per servizi esposti, BYOD per quinte.

### 2018 — Simulazione 1 ITI Galilei (ITTL) — Scuola 300 studenti
Rete amministrativa/didattica da aggiornare, 2 AP WEP obsoleti da sostituire, 3 laboratori, registro elettronico, LIM. Progetto di aggiornamento del cablaggio con fibra, separazione VLAN, WiFi WPA2 Enterprise.

---

## ☁️ 4. Cloud computing e servizi XaaS

Tracce con architetture cloud-first o cloud-hybrid.

### ⭐ 2024 — Ordinaria (Informatica) — Rete regionale sanità PNRR
> L'amministrazione di una Regione italiana ha sviluppato una infrastruttura di comunicazione in fibra ottica per Enti locali, scuole e strutture sanitarie pubbliche. Il **data-center regionale** raccoglie i dati del fascicolo sanitario elettronico. In ambito PNRR Missione 6 (M6C2) si vuole estendere la connettività anche alle **strutture sanitarie private convenzionate** (circa 2000 in regione).
>
> **Richieste:** piano di indirizzamento 10.100.0.0/16 per 2000 strutture, ogni struttura riceve un **dispositivo (router-appliance)** configurato e controllato da remoto, isolamento tra strutture diverse, garanzia di non-accesso generalizzato Internet, archiviazione sicura dei dati sanitari (3-2-1 backup, TLS, VPN di failover).

### 2018 — Ordinaria (Informatica) — FastDelivery logistica
La soluzione tipica prevede **infrastruttura cloud** (IaaS/PaaS) per i server centrali, data l'azienda distribuita su molte SO + CSR. La seconda parte affronta virtualizzazione e data center.

### 2019 — Straordinaria (Informatica) — Ospedale
**Quesito III seconda parte:** presentare i principali **paradigmi di servizi cloud (XaaS)** disponibili delineando caratteristiche di ciascuno — IaaS, PaaS, SaaS, FaaS, DaaS.

---

## 📡 5. Reti WiFi enterprise, mesh e controller centralizzati

Tracce con focus su design WiFi multi-AP, autenticazione enterprise, ottimizzazione canali.

### ⭐ 2019 — Simulazione MIUR 2 aprile (Informatica) — EasyTrain
> La compagnia ferroviaria EasyTrain fornisce servizi di viaggio a lunga percorrenza. Prenotazione online obbligatoria, biglietto via email/QR. A bordo: WiFi gratuito su tutte le carrozze (stesse credenziali del portale), catalogo ~30 film in streaming. La qualità della connessione Internet può avere problemi per territorio, numero utenti, tecnologie.
>
> **Richieste:** infrastruttura a bordo con **AP per carrozza** alimentati PoE (std 802.11n/ac su 2.4/5 GHz), switch di vagone a cascata, router di frontiera su locomotiva/ristorante, WAN via **4G/satellite/HiperLAN/WiMAX**, server streaming **locale a bordo** (non via WAN), autenticazione con **802.1X + EAP + RADIUS**, captive portal, firewall anti-SQL injection e anti-DDoS.

### 2019 — Straordinaria (Informatica) — Ospedale
Nella soluzione: dimensionamento WiFi con **1 AP per soffitto di reparto**, almeno 2 AP/reparto per copertura, gestione canali **sfalsati di 5 canali** per minimizzare interferenze, **Light AP + WLC** per configurazione centralizzata (DHCP, sicurezza, autenticazione).

### 2018 — Ordinaria (Telecomunicazioni) — Teleriscaldamento
Richiesta di almeno **6 AP per piano** per la rete IoT wireless dei termostati, con configurazione centralizzata.

---

## 🌐 6. IoT e reti di sensori

Tracce centrate su dispositivi IoT, gateway, protocolli M2M.

### ⭐ 2018 — Ordinaria (Telecomunicazioni) — Teleriscaldamento scolastico
> Termostati intelligenti in ogni locale + valvole comandate in ogni radiatore, rete WiFi IoT separata da quella didattica, dati inviati a server remoto per regolazione temperatura e efficienza energetica.

### ⭐ 2019 — Simulazione 1 ITI Galilei (Telecomunicazioni) — Ghiacciaio
> Rete di **16 sensori IoT** in alta montagna per monitoraggio temperatura aria/ghiaccio superficiale/ghiaccio a 10 cm profondità. Ogni sensore: microcontrollore, pannello solare + batterie al litio, IP statico, dispositivo WiFi 802.11 b/g/n su 2.4 GHz, collegamento LOS con **Access Point IoT** in un rifugio. Dati trasmessi al CNR via **WiMax long-range**.
>
> **Richieste:** struttura rete, pacchetto dati sensore→AP, **distanza massima sensore-AP con margine fading 20 dB**, caratteristiche WiMax, connessione protetta (VPN IPSec) verso CNR.

### 2018 — Ordinaria (Informatica) — FastDelivery
Il quesito I della seconda parte chiede palmari con **sensore GPS + app IoT** che trasmette posizione al server ad intervalli predefiniti → gestione IoT di flotta.

---

## 🚚 7. Logistica, tracciamento e applicazioni distribuite

Tracce con scenari aziendali distribuiti su territorio nazionale con mezzi mobili.

### ⭐ 2018 — Ordinaria (Informatica) — FastDelivery
> La società FastDelivery si occupa della spedizione di pacchi su territorio nazionale. Possiede Sedi Operative (SO) in molte città e Centri di Smistamento Regionale (CSR). Vuole automatizzare raccolta, smistamento e consegna con sistema di tracciamento (QR/barcode), palmari per trasportatori, presa in carico online del cliente.
>
> **Richieste di rete:** infrastruttura per SO, CSR e Sede, collegamenti tra SO/CSR/Sede (VPN o MPLS), client/server web, scelta tra CED interno e Cloud, sicurezza HTTPS/TLS, messaggistica aziendale, firma digitale del destinatario via app.

### 2017 — Ordinaria (Informatica) — Car Pooling
> Sistema informativo per applicazione di car pooling — geolocalizzazione utenti, matching tragitti, pagamenti, comunicazione server-app mobile.

### 2019 — Ordinaria (Informatica) — POI Turistici
> Assessorato al Turismo di una città d'arte. Infrastruttura tecnologica per fruizione di contenuti multimediali che descrivono i POI (chiese, luoghi storici, musei, mostre) distribuiti nel centro storico. **Infopoint** dislocati in vari punti della città + **Sede Centrale** con server + **Mini-Tablet** noleggiati ai turisti con tre fasce tariffarie (base/intermedia/piena). Pagine multimediali differenziate, scelta lingua, geolocalizzazione.

---

## 📶 8. Ponti radio, link wireless outdoor e reti WAN private

Tracce con progettazione di tratte radio punto-punto, link-budget, antenne direzionali.

### ⭐ 2019 — Ordinaria (Telecomunicazioni) — Piattaforma petrolifera offshore
> Piattaforma petrolifera a ~10 km dalla costa nel mare del Nord. Sulla terraferma una struttura prefabbricata di supporto con linea Internet cablata. Sulla piattaforma LAN divisa tra parte cablata (uffici operativi) e parte wireless (personale). Collegamento terra-mare con **radio-link punto-punto in LOS** tra antenne paraboliche, banda 13 GHz, trasmettitore 28 dBm, antenne con efficienza 65%, sensibilità ricevitore -91 dBm.
>
> **Richieste:** progetto rete di bordo e di terra, **link-budget del ponte radio** (diametro antenne, posizione, cavi, attenuazioni), rete di sensori fluorimetrici UV per monitoraggio idrocarburi in acqua, connessione sicura tra operatore di terra e sede remota.

### 2017 — Ordinaria (Telecomunicazioni) — Sottoinsieme rete aziendale
Rete con più router interconnessi, fibra ottica verso ISP, **Rete 4 collegata via Access Point** a Router 2. Piano VLSM 192.168.1.x + classe C separata. Seconda parte con quesito sul **link radio tra i due AP**: distanza massima in aria libera priva di ostacoli.

### 2016 — Simulazione (Telecomunicazioni) — Due edifici a km
Oltre alla VPN via Internet, la soluzione alternativa prevede **link radio punto-punto diretto** tra i tetti dei due edifici: se il link attraversa suolo pubblico serve **autorizzazione generale MISE** (ex art. 104 Codice Comunicazioni Elettroniche).

---

## 🔍 9. Altri argomenti ricorrenti (seconda parte)

Temi che compaiono quasi sempre come **quesiti singoli** nella seconda parte, raramente come prima parte:

### Autenticazione e identità digitale
- **2024** — SPID livello 2/3, autenticazione qualificata multi-fattore per FSE
- **2019 straordinaria** — protocolli per identificare tablet aziendali

### Backup e disaster recovery
- **2024** — strategia 3-2-1 sul data-center
- **2019 simulazione** — tecniche di backup completo/incrementale/differenziale

### Troubleshooting di rete
- **2024 quesito IV** — utente non naviga, diagnosticare con ping/tracert/nslookup

### Database e web (quesiti SQL e PHP)
Praticamente ogni anno c'è un quesito SQL nella seconda parte anche per Sistemi e Reti.

---

## 📊 Mappa riassuntiva per categoria

| Categoria | Anni coperti | Numero tracce |
|---|---|---|
| VPN / sedi remote | 2016 sim., 2016, 2019 ord. | 3 principali |
| ACL / Firewall / DMZ | 2018 ITTL, 2024, (quesito ricorrente) | 2-3 |
| Cablaggio strutturato | 2016, 2018 sim., 2019 str. | 3 |
| Cloud / XaaS | 2018, 2019 str. (quesito), 2024 | 3 |
| WiFi enterprise / mesh | 2018 ITTL, 2019 str., 2019 sim. (EasyTrain) | 3-4 |
| IoT / sensori | 2018 ITTL, 2019 sim.1 ITTL, 2018 (logistica) | 3 |
| Logistica / distribuito | 2017, 2018, 2019 ord. POI | 3 |
| Ponti radio outdoor | 2017 ITTL, 2019 ord. ITTL (piattaforma), 2016 sim. | 3 |

---

## 💡 Come studiare in modo efficace

La tua intuizione sulle categorie è ottima. Aggiungerei:

1. **Gli argomenti si combinano**: quasi ogni traccia reale mischia 3-4 categorie. Es. EasyTrain = WiFi mesh + cablaggio + streaming + sicurezza.
2. **Le simulazioni MIUR del 2019 (EasyTrain + Ghiacciaio)** sono i "boss finali" perché concentrano IoT + WiFi + link outdoor + VPN tutti insieme.
3. **Gli esempi Telecomunicazioni** (ITTL) sono spesso più utili perché trattano esplicitamente il calcolo link-budget, argomento che l'articolazione Informatica raramente approfondisce ma che può comparire in simulazioni di istituto.

Se vuoi che approfondisca una categoria specifica (soluzione ragionata, schema di rete tipo, comandi Cisco), dimmelo.
