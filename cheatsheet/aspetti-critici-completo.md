# Aspetti critici da documentare

Documento unico: prima il **caso generale** (aspetti comuni a tutte le tecnologie),
poi gli aspetti **particolari** di ciascuna. Per ogni tecnologia vanno sempre documentati
anche gli aspetti comuni.

---

## **Aspetti critici comuni**
- **Schema fisico**:
     - Schema fisico (**planimetria**) dello scenario del problema con la rappresentazione di ambienti
  ed edifici chiave e schema (**indoor** ed **outdoor**) dell'infrastruttura, con **etichettatura univoca**
  di tutti gli asset tecnologici di rete.
     - Schema logico (albero degli **apparati attivi**) di tutti i dispositivi di rete con il loro **ruolo**
  e i **link virtuali** astratti ai vari livelli della **pila ISO/OSI** (tipicamente L2, L3, L7) che tipicamente consistono in:
          - link L2/L3 **tunnel** (VPN) su rete pubblica di un ISP di tipo Secure Network o Trusted Network
          - link L2/L3 **ponte radio 802.11** (WiFi) di tipo Client/Server o bridged
          - link L7 tra sensori/attuatori IP e il broker MQTT, oppure tra i gateway WSN e il broker MQTT.
- **Utenti e dispositivi**
     - Tipologia di **divisione in gruppi** degli utenti e loro caratterizzazione (dislocazione fisica
  delimitata mediante subnet oppure diffusa "a macchia di leopardo" mediante VLAN).
    - Definizione delle **tecnologie dei dispositivi** chiave: sensori/attuatori, **gateway**, e relativo
  **dimensionamento di massima** (quantità, numero di porte, banda, ecc.).
  *(I dettagli specifici — topologie, link radio, tipo di accesso, ecc. — sono nelle sezioni particolari.)*
    - Dislocazione di eventuali **router/Firewall**.
- **Vincoli**
    - Eventuali **vincoli normativi** sulle tecnologie in uso: **cablaggio strutturato**, **potenza**, **EIRP**, **ERP**, **duty cycle**.

- **Indirizzamento e routing**
    - **Subnetting** e definizione degli indirizzi (gruppi di utenti, server farm) e degli **indirizzi dei server**.
    - Definizione del **tipo di routing** (statico o dinamico). In caso di **routing statico**, definizione
  delle **tabelle di routing** più significative.
  *(Eccezione: nel WiFi Mesh il routing è sempre automatico — vedi sezione dedicata.)*
- **Servizi di rete**
     - Definizione della posizione dei **servizi di sistema** (DHCP, DNS), dislocati a scelta a bordo del **FW**, collegati al **CS**, o inseriti in una **server farm**.
     - Eventuale impostazione della **continuità del  servizio** mediante tecniche di replica agenti lungo il piano del servizio, il piano dei dati e il piano del ripristino (backup).
     - Eventuale installazione, sulle interfacce di **ingresso** (direzione IN) di ciascun router, delle **ACL** che definiscono il processo di **filtraggio dei pacchetti** per autorizzare/negare il traffico dati tra i gruppi di utenti del sistema.
     - Installazione sulla interfaccia WAN verso internet del processo **NAT** di traduzione degli indirizzi privati nel pool di indirizzi pubblici del router di confine sul link verso un ISP.
     - Eventuale installazione sulla interfaccia WAN verso internet e/o nella server farm di un servizio di **reverse proxy** per funzioni di **ALG** e **clustering**.
     - Eventuale installazione sulla interfaccia WAN verso internet di un **servizio di VPN** per la realizzazione di:
          - accesso remoto **home-to-site** per manutenzione occasionale e smartworking
          - di una connessione **site-to-site** (secure o trusted) per il collegamento stabile "like wired" verso una sede remota che fornisca proprietà di **autenticazione del nodo** e di **cifratura dei dati**.
- **Autenticazione**
     - Definizione delle **tecniche di autenticazione degli utenti** (es. **802.1X**) per l'accesso alla **risorsa rete** presso un **supplicant** (NAS) a scelta tra autenticazione:
          - **L2 EAP** di porta fisica per accesso presso uno **switch** (in base al MAC o all'id utente mediante **RADIUS/DIAMETER**)
          - **L2 EAP** di porta logica per accesso presso un **AP WiFi** (in base al MAC o all'id utente mediante **RADIUS/DIAMETER**) 
          - **L7 con Captive portal** per accesso presso uno **switch** o presso un **AP WiFi**, mediante sottomissione di username e password o inserimento di un codice vaucher su un form web.
     - Definizione delle tecniche di **autenticazione dei webservice** (openid, psw, sessioni, ecc.).
     - Definizione delle tecniche di **autorizzazione SSO** (openid, kerberos, ecc.).
     - Definizione delle tecniche di **autenticazione dei nodi sensori/attuatori** (certificati, psw, preshared key, ecc.).
     - Definizione delle tecniche di **autenticazione dei nodi di elaborazione/pubblicazione** (certificati, psw, preshared key, ecc.).
     - Definizione delle tecniche di **autenticazione dei nodi di smistamento** (certificati, vpn, ecc.).
- **Applicazione e dati IoT**
     - Definizione della posizione del **broker MQTT**.
     - Definizione dei **topic** utili per i casi d'uso richiesti.
     - Definizione dei **messaggi JSON** per alcuni dispositivi IoT significativi in merito a **comandi**,
  **stato** o **configurazione**.
     - Definizione del percorso dei dati tra sensori ed eventuali attuatori per stabilire la
  **sede dell'elaborazione dei comandi** più opportuna (locale/edge vs remota on-premise/cloud).
     - Definizione (anche in **pseudocodice**) delle **funzioni del firmware** di bordo dei dispositivi IoT.

---

## Aspetti particolari per tecnologia

### Zigbee
- Posizionamento in planimetria dei **nodi** con relativa etichetta, avendo cura di posizionare tra essi
  almeno **un gateway** che permetta l'accesso a una rete IP. Progettare dei **percorsi alternativi
  (backup)** in caso di guasto del gateway principale.
- Tecnologie dei dispositivi: definizione della **tipologia di servizio** (polling sincrono,
  comando asincrono, ecc.).
- Gestire eventuali **vincoli di prossimità** (mediante controllo di potenza o gestione del roaming)
  ed eventuali **vincoli di posizionamento** (mediante trilaterazione).
- Definizione del percorso dei dati tra sensori ed eventuali attuatori per stabilire la
  **sede dell'elaborazione dei comandi** più opportuna (locale/edge sul gateway vs remota on-premise/cloud).

### Ethernet
- Tecnologie dei dispositivi: **topologia** dei sensori/attuatori (**stella**, **bus**, **singolo**);
  **link** (dual radio, three radio); **accesso radio** (allocazione di servizi sincroni **TDM**,
  asincroni **CSMA/CA** o a basso ritardo **slotted CSMA/CA**) con relativo dimensionamento.
- Definizione di **dorsali (wireless)** e di **punti di accesso e aggregazione** dei dispositivi utente.
- **Subnetting** strutturato:
  - subnet di **aggregazione** per i gruppi di utenti (generalmente statica);
  - subnet di **dorsale** (statica o automatica basata su **Link Local**);
  - subnet di **servizio** (server farm e **DMZ**).
- Definizione degli **indirizzi dei server** e dei **range** di quelli dei **dispositivi client IP** (PC, smartphone, tablets, sensori/attuatori).

### BLE
- Tecnologie dei dispositivi: **sensori/attuatori**, **gateway**, **link** (dual radio, three radio),
  **accesso radio** (allocazione di servizi sincroni **TDM**, asincroni **CSMA/CA** o a basso ritardo
  **slotted CSMA/CA**) con relativo dimensionamento.
- **Nota:** rispetto alle altre tecnologie, la sezione BLE del documento originale **non specifica**
  le tecniche di autenticazione (utenti/servizi).
- Definizione del percorso dei dati tra sensori ed eventuali attuatori per stabilire la
  **sede dell'elaborazione dei comandi** più opportuna (locale/edge sul gateway vs remota on-premise/cloud).


### WiFi infrastruttura
- Definizione degli **indirizzi dei server** e dei **range** di quelli dei **dispositivi client IP** (PC, smartphone, tablets, sensori/attuatori).
- Gestire i **vincoli di prossimità**, distinguendo tra ambienti **indoor** e **outdoor** (controllo di
 potenza o gestione del roaming) e i **vincoli di posizionamento** (trilaterazione).
- Definizione della **posizione del controller degli AP**.
- Definizione delle tecniche di **autenticazione dei nodi AP** (certificati, psw, preshared key, ecc.) presso i servizi (es. AP su RADIUS).
- Se presenti VLAN, definizione del tipo di **associazione VLAN↔SSID** (statica o dinamica con **Tunnel-Private-Group-Id**).

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
- Definizione degli **indirizzi dei server** e dei **range** di quelli dei **dispositivi client IP** (PC, smartphone, tablets, sensori/attuatori).
- Definizione delle tecniche di **autenticazione dei nodi AP** (certificati, psw, preshared key, ecc.);
  reciproca (backhaul/link radio) o tra nodi e servizi (es. AP su RADIUS).
- Definizione del **tipo di rete mesh**: **routed mesh** vs **bridged mesh**.
- **Routing sempre automatico** (AODV, OLSR, Babel) con definizione automatica delle subnet dei link
  tra i nodi router (**LLA**, **SLAAC**). *(Sostituisce il routing statico/dinamico generale.)*
- Se presenti VLAN, definizione del tipo di **associazione VLAN↔SSID** (statica o dinamica con
  **Tunnel-Private-Group-Id**).
- Definizione della **posizione del controller degli AP**.

### LoRaWAN
- schema fisico (planimetria) dello scenario del problema con la rappresentazione di ambienti e edifici chiave e schema (indoor ed outdoor) dell'infrastruttura con etichettatura univoca di asset specifici quali sensori/attuatori, gateway/packet forwardere (PF), network server (NS), join server (JS) e application server (AP).
- Definire se è necessaria una **federazione di reti di sensori**: stabilire se **broker** e
  **Network Server** sono individualmente o entrambi parte del router/gateway verso la rete IP,
  oppure se sono **a comune** con più reti LoRaWAN.
- Definizione del percorso dei dati tra sensori ed eventuali attuatori per stabilire la
  **sede dell'elaborazione dei comandi** più opportuna dove effettuare decifrazione del payload. Ovvero, stabilire se l'AS debba stare o meno sul GW insieme al PF e al NS.
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
- Definizione del percorso dei dati tra sensori ed eventuali attuatori per stabilire la
  **sede dell'elaborazione dei comandi** più opportuna (locale/edge sul gateway vs remota on-premise/cloud).
