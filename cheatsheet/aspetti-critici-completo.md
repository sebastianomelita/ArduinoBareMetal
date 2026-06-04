# Aspetti critici da documentare

Documento unico: prima il **caso generale** (aspetti comuni a tutte le tecnologie),
poi gli aspetti **particolari** di ciascuna. Per ogni tecnologia vanno sempre documentati
anche gli aspetti comuni.

---

## Aspetti comuni (caso generale)

Elementi che ricorrono (in modo identico o quasi) in **tutte** le sezioni — Zigbee, Ethernet, BLE,
WiFi infrastruttura, WiFi Mesh, LoRaWAN, RFID. Vanno sempre documentati bilanciando le convenienze
e argomentando le scelte.

### Schema fisico e logico
- Schema fisico (**planimetria**) dello scenario del problema con la rappresentazione di ambienti
  ed edifici chiave e schema (**indoor** ed **outdoor**) dell'infrastruttura, con **etichettatura univoca**
  di tutti gli asset tecnologici di rete.
- Schema logico (albero degli **apparati attivi**) di tutti i dispositivi di rete con il loro **ruolo**
  e i **link virtuali** astratti ai vari livelli della **pila ISO/OSI** (tipicamente L2, L3, L7).

### Utenti e dispositivi
- Tipologia di **divisione in gruppi** degli utenti e loro caratterizzazione (dislocazione fisica
  delimitata oppure diffusa "a macchia di leopardo").
- Definizione delle **tecnologie dei dispositivi** chiave: sensori/attuatori, **gateway**, e relativo
  **dimensionamento di massima** (quantità, numero di porte, banda, ecc.).
  *(I dettagli specifici — topologie, link radio, tipo di accesso, ecc. — sono nelle sezioni particolari.)*
- Dislocazione di eventuali **gateway**.

### Vincoli
- Eventuali **vincoli normativi** sulle tecnologie in uso: **potenza**, **EIRP**, **ERP**, **duty cycle**.

### Indirizzamento e routing
- **Subnetting** e definizione degli indirizzi (gruppi di utenti, server farm) e degli **indirizzi dei server**.
  *(La struttura specifica delle subnet è dettagliata nelle sezioni particolari.)*
- Definizione posizione dei **servizi di sistema** (DHCP, DNS), dislocati a scelta a bordo del **FW**,
  collegati al **CS**, o inseriti in una **server farm**.
- Definizione del **tipo di routing** (statico o dinamico). In caso di **routing statico**, definizione
  delle **tabelle di routing** più significative.
  *(Eccezione: nel WiFi Mesh il routing è sempre automatico — vedi sezione dedicata.)*

### Autenticazione
- Definizione delle **tecniche di autenticazione degli utenti** (es. **802.1X**): scelta tra autenticazione
  **L2 EAP** (statica per AP o per utente con **RADIUS**) e autenticazione **L7 con Captive portal**.
- Definizione delle tecniche di **autenticazione dei servizi** (openid, psw, sessioni, ecc.).
  *(Nota: la sezione BLE del documento originale non specifica l'autenticazione.)*

### Applicazione e dati IoT
- Definizione della posizione del **broker MQTT**.
- Definizione dei **topic** utili per i casi d'uso richiesti.
- Definizione dei **messaggi JSON** per alcuni dispositivi IoT significativi in merito a **comandi**,
  **stato** o **configurazione**.
- Definizione del percorso dei dati tra sensori ed eventuali attuatori per stabilire la
  **sede dell'elaborazione dei comandi** più opportuna (locale/edge vs remota on-premise/cloud).
  *(Nota: la sezione RFID del documento originale non specifica questo punto.)*
- Definizione (anche in **pseudocodice**) delle **funzioni del firmware** di bordo dei dispositivi IoT.

---

## Aspetti particolari per tecnologia

### Zigbee
- Posizionamento in planimetria dei **nodi** con relativa etichetta, avendo cura che tra essi esista
  almeno **un gateway** che permetta l'accesso a una rete IP. Progettare dei **percorsi alternativi
  (backup)** in caso di guasto del gateway principale.
- Tecnologie dei dispositivi: definizione della **tipologia di servizio** (polling sincrono,
  comando asincrono, ecc.).
- Gestire eventuali **vincoli di prossimità** (mediante controllo di potenza o gestione del roaming)
  ed eventuali **vincoli di posizionamento** (mediante trilaterazione).
- **Subnetting** con struttura "piatta": indirizzi dei vari **gruppi di utenti**, delle **server farm**
  e definizione degli **indirizzi dei server**.

### Ethernet
- Tecnologie dei dispositivi: **topologia** dei sensori/attuatori (**stella**, **bus**, **singolo**);
  **link** (dual radio, three radio); **accesso radio** (allocazione di servizi sincroni **TDM**,
  asincroni **CSMA/CA** o a basso ritardo **slotted CSMA/CA**) con relativo dimensionamento.
- Definizione di **dorsali (wireless)** e di **punti di accesso e aggregazione** dei dispositivi utente.
- **Subnetting** strutturato:
  - subnet di **aggregazione** per i gruppi di utenti (generalmente statica);
  - subnet di **dorsale** (statica o automatica basata su **Link Local**);
  - subnet di **servizio** (server farm e **DMZ**).
- Definizione degli **indirizzi dei server** e dei **range** di quelli dei client.

### BLE
- Tecnologie dei dispositivi: **sensori/attuatori**, **gateway**, **link** (dual radio, three radio),
  **accesso radio** (allocazione di servizi sincroni **TDM**, asincroni **CSMA/CA** o a basso ritardo
  **slotted CSMA/CA**) con relativo dimensionamento.
- **Subnetting** con struttura "piatta": indirizzi dei vari **gruppi di utenti**, delle **server farm**
  e definizione degli **indirizzi dei server**.
- **Nota:** rispetto alle altre tecnologie, la sezione BLE del documento originale **non specifica**
  le tecniche di autenticazione (utenti/servizi).

### WiFi infrastruttura
- Tecnologie dei dispositivi: **topologia** dei sensori/attuatori (**stella**, **bus**, **singolo**);
  **link** (dual radio, three radio); **accesso radio** (**TDM** / **CSMA/CA** / **slotted CSMA/CA**)
  con relativo dimensionamento.
- Definizione di **dorsali wireless** e di **punti di accesso e aggregazione** dei dispositivi utente.
- **Subnetting** strutturato: subnet di **aggregazione** (statica), subnet di **dorsale**
  (statica o Link Local), subnet di **servizio** (server farm e **DMZ**).
- Definizione degli **indirizzi dei server** e dei **range** dei client.
- Definizione delle tecniche di **autenticazione dei nodi AP** (certificati, psw, preshared key, ecc.).
  Può essere **reciproca** (backhaul, link radio) oppure **tra nodi e servizi** (es. autenticazione
  di un AP su RADIUS).
- Gestire i **vincoli di prossimità**, distinguendo tra ambienti **indoor** e **outdoor** (controllo di
  potenza o gestione del roaming) e i **vincoli di posizionamento** (trilaterazione).
- Definizione della **posizione del controller degli AP**.

### WiFi Mesh
- Posizionamento in planimetria dei **nodi** con relativa etichetta, avendo cura che tra essi esista
  almeno **un percorso LOS** (Line Of Sight) con dei vicini. Progettare dei **percorsi alternativi
  (backup)** in caso di guasto dei nodi centrali di traffico.
- Tecnologie dei dispositivi: **topologia** (stella, bus, singolo), **link**, **accesso radio**
  (TDM / CSMA/CA / slotted CSMA/CA) con dimensionamento.
- Definizione di **dorsali wireless** e di **punti di accesso e aggregazione** dei dispositivi utente.
- Definizione di eventuali **link di backhaul** e della relativa tecnologia.
- Definire le **interfacce radio** scegliendo tra **2-band** (dual channel) e **3-band** (three channel).
- Definire i **canali dei link** scegliendoli tra quelli in banda **ISM** e pianificandone il
  **riuso nello spazio**, minimizzando l'**interferenza cocanale**.
- Gestire i **vincoli di prossimità** (indoor/outdoor, controllo potenza/roaming) e i
  **vincoli di posizionamento** (trilaterazione).
- **Subnetting** strutturato: subnet di **aggregazione** (statica), subnet di **dorsale**
  (statica o Link Local), subnet di **servizio** (server farm e **DMZ**); più **indirizzi dei server**
  e **range** dei client.
- Definizione delle tecniche di **autenticazione dei nodi AP** (certificati, psw, preshared key, ecc.);
  reciproca (backhaul/link radio) o tra nodi e servizi (es. AP su RADIUS).
- Definizione del **tipo di rete mesh**: **routed mesh** vs **bridged mesh**.
- **Routing sempre automatico** (AODV, OLSR, Babel) con definizione automatica delle subnet dei link
  tra i nodi router (**LLA**, **SLAAC**). *(Sostituisce il routing statico/dinamico generale.)*
- Se presenti VLAN, definizione del tipo di **associazione VLAN↔SSID** (statica o dinamica con
  **Tunnel-Private-Group-Id**).
- Definizione della **posizione del controller degli AP**.

### LoRaWAN
- Tecnologie dei dispositivi: **topologia** (stella, bus, singolo), **link**, **accesso radio**
  (TDM / CSMA/CA / slotted CSMA/CA) con dimensionamento.
- Definizione di **dorsali wireless** e di **punti di accesso e aggregazione** dei dispositivi utente.
- **Subnetting** strutturato (per le reti IP): subnet di **aggregazione** (statica), subnet di **dorsale**
  (statica o Link Local), subnet di **servizio** (server farm e **DMZ**); più **indirizzi dei server**
  e **range** dei client.
- I **servizi di sistema** (DHCP, DNS) e il **routing** vanno riferiti alle **reti IP** presenti.
- Definizione delle tecniche di **autenticazione dei nodi LoRaWAN**. Può essere **reciproca**
  (backhaul, link radio) oppure **tra nodi e servizi** (es. **Network Server**).
- Definire se è necessaria una **federazione di reti di sensori**: stabilire se **broker** e
  **Network Server** sono individualmente o entrambi parte del router/gateway verso la rete IP,
  oppure se sono **a comune** con più reti LoRaWAN.
- Definizione della posizione, nella rete IP, di: **broker MQTT**, **Join Server**, **Network Server**
  e **Application Server**.
- Gestire i **vincoli di prossimità** (indoor/outdoor, controllo potenza/roaming) e i
  **vincoli di posizionamento** (trilaterazione).

### RFID
- **Tecnologie dei tag** da usare: **passivi**, **attivi**, **semi-passivi (BAP)**. La scelta determina
  **portata**, **costo** e **durata**.
- **Scelta della frequenza di lavoro** (**LF**, **HF**, **UHF**, **microonde**) in funzione di
  **materiali**, **distanza** e **vincoli normativi nazionali**.
- **Densità dei tag** simultanei nel campo (**dense reader environment**) e necessità di eventuali
  protocolli di **anticollisione**.
- Posizionamento in planimetria dei **nodi** con relativa etichetta, avendo cura che tra essi esista
  almeno **un gateway** che permetta l'accesso a una rete IP. Progettare dei **percorsi alternativi
  (backup)** in caso di guasto del gateway principale.
- Gestire eventuali **vincoli di prossimità** (controllo di potenza o gestione del roaming) ed eventuali
  **vincoli di posizionamento** (trilaterazione).
- **Subnetting** con struttura "piatta": indirizzi dei vari **gruppi di utenti**, delle **server farm**
  e definizione degli **indirizzi dei server**.
- **Nota:** rispetto alle altre tecnologie, la sezione RFID del documento originale **non specifica**
  la definizione del percorso dati / sede di elaborazione dei comandi.
