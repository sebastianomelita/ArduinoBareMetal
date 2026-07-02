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
- **Schemi fondamentali**:
     - Schema fisico (**planimetria**) dello scenario: ambienti ed edifici chiave, infrastruttura
  **indoor**/**outdoor**, con **etichettatura univoca** di tutti i **dispositivi passivi** di rete (armadi) e con la definizione del **tracciato** dei mezzi trasmissivi (cavi ethernet e fibra).
     - Schema **logico** (albero degli **apparati attivi**) di tutti i dispositivi che rappresenti:
          - eventuale **router di confine** della LAN
          - eventuale gerarchia di **switch** che realizzano fisicamente la LAN
          - **link fisici**: dorsali interne alle LAN e dorsali esterne verso lo ISP
          - **link virtuali** ai vari livelli ISO/OSI (tipicamente L2, L3, L7):
              - link L2/L3 **tunnel** (VPN) su rete pubblica ISP — Secure o Trusted Network;
              - link L2/L3 **ponte radio 802.11** (WiFi) Client/Server o bridged;
              - link L7 tra sensori/attuatori IP e broker MQTT, o tra gateway WSN e broker MQTT.
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
    - **Subnetting** dei **link verso le LAN** (fisiche o virtuali) per l'allocazione di gruppi di utenti, server farm e DMZ definizione dell **indirizzo del router di confine** della LAN. Definizione degli **indirizzi dei server**.
    - **Subnetting** dei **link fisici di dorsale** tra i router e definizione degli **indirizzi delle interfacce fisiche** dei router
    - **Subnetting** dei **link logici L3 di dorsale** tra i router e definizione degli **indirizzi delle interfacce virtuali TUN** dei router
    - Definizione del **tipo di routing** (statico o dinamico). In caso di **routing statico**, definizione
  delle **tabelle di routing** più significative.
  *(Eccezione: nel WiFi Mesh il routing è sempre automatico — vedi sezione dedicata.)*
- **Servizi di rete**
     - Posizione dei **servizi di sistema** (DHCP, DNS), dislocati a scelta a bordo del **FW**, collegati al **CS**, o inseriti in una **server farm**.
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
     - Posizione del **sensori/attuatori**.
     - Posizione del **gateway** tra eventuale **rete di aggregazione non IP (WSN o BUS di campo)** e rete di **distribuzione IP**.
     - Posizione del **client MQTT**: a bordo dei sensori/attuatori per **sensori/attuatori IP**, a bordo del **gateway** per **sensori/attuatori non IP**.
     - Posizione del **broker MQTT** (**edge** locale, **centrale on premise**, **centrale in cloud**, **edge locale in bridge con centrale** on premise/cloud)
     - Definizione dei **topic** utili per i casi d'uso richiesti.
     - Definizione dei **messaggi JSON** per alcuni dispositivi IoT significativi in merito a **comandi**,
  **stato** o **configurazione**.
     - Definizione del percorso dei dati tra sensori ed eventuali attuatori per stabilire la
  **sede dell'elaborazione dei comandi** più opportuna (locale/edge vs remota on-premise/cloud).
     - Definizione (anche in **pseudocodice**) delle **funzioni del firmware** di bordo dei dispositivi IoT.

### **Documentazione cablaggio**

Consiste nell'**ordine**:
1. Planimetria senza cablaggio (parte fisica utile ma non necessaria)
2. Planimetria con cablaggio (parte fisica necessaria in tutti i contesti eccetto il cloud, fusa con l'albero degli apparati attivi nelle reti WiFi mesh)
3. Albero degli apparati passivi (parte fisica necessaria in tutti i contesti ethernet + WSN
4. Tabella delle dorsali (parte fisica necessaria in tutti i contesti ethernet + wsn)
5. Albero degli apparati attivi (parte fisica necessaria in tutti i contesti ethernet + WSN)
6. Schema degli armadi (parte fisica necessaria in tutti i contesti ethernet + WSN)
   
Alcuni documenti possono essere **trascurati** il relazione a quanto forte è nel progetto la responsabilità della definizione della parte fisica dei servizi. In linea di principio:
- se i servizi stanno in **cloud** la parte fisica corrispondente è delegata al datacenter che li ospita
- se i servizi stanno **on premise** la parte fisica corrispondente è responsabilità del progetto
- nelle reti **mesh WiFi** e **WSN** (LoraWAN, Zigbee, RFID) in cui il **gateway WSN** coincide con il **gateway di accesso** alla rete Internet (Modem/Firewall) allora la parte di **cablaggio dei cavi** è, in pratica, inesistente.
- nelle reti **mesh WiFi** e **WSN** la planimetria dovrebbe comprendere anche l'albero degli apparati attivi (che quindi va definito solo per la parte ethernet) e dovrebbe rappresentare:
     - la posizione dei nodi
     - una ipotesi di **albero principale** del collegamento wireless (L2 o L3) reciproco tra i nodi. Consigliabile la definizione di una ipotesi di collegamento wireless secondario (backup) in caso di guasto di uno o più nodi principali di traffico.
- se gli scenari del progetto sono diversi e di differente natura allora conviene replicare la planimetria per ciascuno.
- se gli scenari di rete sono assimilabili a **reti di aggregazione di dispositivi** (sensori, attuatori, PC, ecc.) interconnesse da una **rete di distribuzione IP WAN o MAN** tramite un **gateway**, allora potrebbe essere utile definire uno schema logico (ad esempio degli apparati attivi) che le rappresenta insieme come **federazione di reti** con tutti i collegamenti reciproci fisici e logici (tunnel L2/L3 o broker MQTT).
