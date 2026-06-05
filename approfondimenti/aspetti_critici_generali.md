>[Torna a reti di sensori](sensornetworkshort.md#classificazione-delle-tecnologie-wsn-in-base-a-velocità-e-copertura)

- [Dettaglio architettura Ethernet](../archeth.md)
- [Dettaglio architettura Zigbee](../archzigbee.md)
- [Dettaglio architettura BLE](../archble.md)
- [Dettaglio architettura WiFi infrastruttura](../archwifi.md)
- [Dettaglio architettura WiFi mesh](../archmesh.md) 
- [Dettaglio architettura LoraWAN](../lorawanclasses.md)
- [Dettaglio architettura 5G/6G](../ranprivata.md)
- [Dettaglio architettura RFID](../archrfid.md)


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
     - Installazione sulla interfaccia WAN verso internet del processo **NAT** di traduzione degli indirizzi privati nel pool di indirizzi pubblici del router di confine sul link versi un ISP.
     - Eventuale installazione sulla interfaccia WAN verso internet e/o nella server farm di un servizio di **reverse proxy** per funzioni di **ALG** e **clustering**.
     - Eventuale installazione sulla interfaccia WAN verso internet di un **servizio di VPN** per la realizzazione di:
          - accesso remoto **home-to-site** per manutenzione occasionale e smartworking
          - di una connessione **site-to-site** (secure o trusted) per il collegamento stabile "like wired" verso una sede remota che fornisce proprietà di autenticazione del nodo e di cifratura dei dati.
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
