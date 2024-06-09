>>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

## **Punti essenziali di un progetto di cablaggio strutturato** 

1. Descrizione generale del progetto:
    - analisi dei requisiti utente (servizi chiave e loro vincoli operativi)
    - valutazione criticità e definizione ipotesi aggiuntive
    - definizione delle specifiche del sistema (dettaglio su servizi e funzioni e dimensionamento vincoli
operativi)
2. Planimetria:
    - Etichettatura spazi
    - Localizzazione TO (etichettatura)
    - Percorso canalizzazioni (dorsali e cablaggio orizzontale)
    - Collocazione armadi
    - Dimensionamento TO e cavi
3. Albero apparati passivi
    - Struttura
    - Etichettatura nodi (CD, BD, FD) e foglie (TO)
4. Tabella dorsali:
    - Elenco dorsali con etichetta
    - Tipo di cavo: UTP, MMF (OM2, OM3, OM4), cardinalità ulti fibra (2, 4, 6, 8, 12 fibre per cavo)
5. Definizione armadi:
    - PP (prese RJ45), Cassetto ottico (prese SC o LC), SW, Eventuali server, Spazi di aereazione, UP
6. Albero apparati attivi:
    - Struttura (schema ad albero o ridondato) di AP, switch, routers, host, servers, link ed interfacce virtuali
    - Tecnologia switch e collegamenti: Ethernet, WIFI, Bluetooth, WIMAX, ponte radio, VPN(link virtuali)
    - Etichettatura nodi (CS, DS, AS) e foglie (Hosts)
    - Dimensionamento switches: N.porte, tipo apparato (L2, L3, stackable)
    - Media converter (transceiver SFP o SFP+)
7. Subnetting e connetività IP:
    - Definizione subnets (FLSM, VLSM, VLAN)
    - Assegnazione indirizzi a Host, a Routers (o SW L3) e ai vari server
    - Progettazione del routing statico e definizione di eventuali tabelle di routing su routers o SW L3
8. Servizi di sistema (garantiscono connetività e funzioni di base comuni a tutte le applicazioni)
    - Architettura e collocazione (hosting interno, hosting esterno)
    - Tecnologia (server fisico, virtualizzazione, cloud computing)
    - Protocolli del livello di rete per la gestione delle connessioni: DHCP, DNS, SMB, VOIP manager, Wireless
manager, VPN, etc.
    - Controllo accesso risorse: PPP, AAA server (RADIUS, 802.1x), Captive Portal, Rfeed
    - Gestori generici di contenuti: NAS (mySQL, SQL server, etc.), CMS, LMS, Server WEB (Apache, IIS),
stream server (shoutcast, icecast), server FTP, server mail (POP, SMTP), QR code

9. Servizi applicativi specifici:
    - Architettura: modello C/S (Two-Tier, Three-Tier), oggetti distribuiti, Peer-To-Peer
    - Protocolli e loro paradigmi di interazione: request/response (HTTP, HTTPS), request/response asincrono
(Web hooks), publish/subscriber (MQTT, telegram, chat), stream socket bidirezionali (L4 TCP, L7
websocket).
    - Collocazione (hosting interno, hosting esterno) e integrazione con servizi di sistema (CMS, Server WEB)
    - Tecnologia (server fisico, virtualizzazione, cloud computing)
    - funzionalità, caratteristiche e configurazione dei servizi applicativi:
        - funzioni del livello fisico: lettura sensori (ADC), comando attuatori (DAC)
        - Definizione della logica dei servizi e della presentazione dei contenuti: descrizione delle
funzioni, diagrammi (stato, flusso, classi, etc.), pseudolinguaggio, linguaggio reale (HTML, PHP,
Java, Javascript)
        - Definizione delle interazioni: descrizione in linguaggio naturale, diagrammi

10. Database:
    - Diagramma E/R, Diagramma logico, Query
11. Gestione della sicurezza e della privacy dei sistemi informatici e delle reti:
    - Strategie adottate per la separazione dei servizi tra gruppi di utenti (L1, L2, L3, L7)
    - Censimento tipologie di possibili minacce e relative contromisure;
    - tecniche crittografiche e loro applicazione;

    - configurazione di apparati e servizi per la sicurezza delle reti e dei sistemi (NAT, Firewall, Proxy
server);
    - principali aspetti normativi (GDPR).
12. Gestione della continuità del servizio:
    - Architetture e strategie adottate per il miglioramento della affidabilità dei servizi chiave

Obiettivi della prova:
1. Affrontare situazioni problematiche, utilizzando adeguate strategie cognitive e procedure operative
orientate alla progettazione di soluzioni informatiche e infrastrutture di rete.
2. Scegliere motivatamente e configurare dispositivi, apparati, protocolli e servizi idonei in base alle loro
caratteristiche funzionali ed al contesto proposto.
3. Individuare problematiche di sicurezza e relative soluzioni in riferimento al contesto proposto.
4. Realizzare progetti secondo procedure consolidate e criteri di sicurezza.
5. Redigere relazioni tecniche e documentare le attività di progetto.

AREE DI SVILUPPO DEL PROGETTO DI UNA RETE

1. Descrizione generale del progetto:
    - analisi dei requisiti utente (servizi chiave e vincoli operativi)
    - valutazione criticità e definizione ipotesi aggiuntive
    - definizione delle specifiche del sistema (dettaglio su servizi e funzioni e dimensionamento vincoli
operativi)
2. Layout e cablaggio:
    - Planimetria
    - Albero apparati passivi
    - Definizione armadi
3. H/W di rete, connettività
    - Albero apparati attivi
    - Subnetting
4. Servizi intranet/Internet:
    - Servizi di sistema
    - Servizi applicativi specifici
5. Database
    - Diagramma E/R
    - Diagramma logico
    - Query


>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

