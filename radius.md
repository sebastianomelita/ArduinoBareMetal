>>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

# **VPN di reti Ethernet** 

## **Autenticazione 802.1X**

Come accennato, l’autenticazione dell’utente e degli AP (Access Point) avviene:
- A Livello 2 della pila ISO/OSI con protocolli e dispositivi che non abbisognano di indirizzo IP, sfruttano l’incapsulamento (tunnelling) in trame di livello pari o superiori. Il protocollo più comune per il wifi è l’EAP incapsulato su trame L2 MAC (EAPoL). E’ utilizzato per trasportare le credenziali (username e password) lungo una connessione ancora priva di indirizzo IP verso un dispositivo di accesso (NAS, inteso stavolta come Network Access Server) della risorsa accesso WiFi.
- A Livello 7 della pila ISO/OSI con il protocollo di livello applicativo RADIUS. Serve a trasportare le credenziali dall’autenticatore (l’AP) al server RADIUS, dove verranno verificate inserendo l’username in un database centrale (di tipo LDAP o MySQL). AP e server sono, questa volta, entrambi dotati di indirizzo IP essendo adesso dispositivi connessi alla rete LAN.

Le porte controllate possono essere:
- Porte fisiche quali quelle ethernet di uno switch, sono sempre disponibili a livello fisico semplicemente collegando un plug ma possono risultare indisponibili a livello 2 della pila ISO/OSI se l’autenticazione 802.1 X non va a buon fine.
- Porte virtuali quali quelle di un AP. Se un utente Wi-Fi viene autenticato tramite 802.1 X per l'accesso alla rete, viene aperta una porta virtuale sul punto di accesso che consente la comunicazione. Se non è stato autorizzato correttamente, una porta virtuale non è disponibile e le comunicazioni sono bloccate.
Anche nel caso di un solo AP vengono attivate 2 porte virtuali a valle di quella fisica, una per gestire il protocollo di autenticazione e un’altra per controllare ed eventualmente bloccare il traffico dati.

<img src="img/supplicant.png" alt="alt text" width="1000">

Per realizzare l'autenticazione 802.1 X servono 3 dispositivi:
- Supplicant: un client Software in esecuzione sulla workstation Wi-Fi.
- Autenticatore (detto anche NAS): Il punto di accesso Wi-Fi.
- Server di autenticazione: un database di autenticazione, in genere un server RADIUS, nel nostro caso Freeradius incluso nel server Univention.

## **Autenticazione di porta**

Autorizza o meno l’apertura di una connessione di livello 2 ISO/OSI. L’autenticazione di un client detto supplicant passa per il NAS (Network Access Server) a lui più vicino. Un NAS è autorizzato da un server di autenticazione centralizzato comune a tutti i NAS del sistema. Mentre i messaggi del protocollo viaggiano liberamente attraverso la uncontrolled port, quelli dati passano per la controlled port che permette il passaggio solo su autorizzazione del server RADIUS.

<img src="img/porteradius.png" alt="alt text" width="500">

### **Sequenza di autenticazione utente**

La tecnica utilizzata è il PEAP: è uno standard aperto ideato da Cisco Systems, Microsoft e RSA Security, e fornisce un elevato livello di sicurezza. È molto simile a EAPTTLS, richiede solo il certificato lato server e crea un tunnel sicuro con TLS per proteggere l’autenticazione dell’utente, autenticazione effettuata utilizzando altri metodi come MS-CHAPv2 (PEAP/EAPMSCHAPv2) basati su username e password.
Avviene solamente dopo l’autenticazione con successo del server, un sottoprodotto di questa è lo scambio di una chiave crittografica OTP di sessione tra client e server con cui realizzare un canale cifrato sicuro. All’interno di questo tunnel cifrato è possibile procedere all’autenticazione dell’utente, solitamente con autenticazioni deboli (cmq realizzate dentro un canale cifrato inaccessibile) che normalmente prevedono:
- protocollo PAP. Scambio di password in chiaro, soggetta ad attacco replay
- protocollo CHAP o MSCHAP con scambio dell’hash di password + sfida, non soggetto ad attacco di tipo replay.

Un riassunto delle fasi dell’autenticazione CHAP potrebbe essere:
1. Il client invia attraversando in tunnelling il NAS la propria identità al server RADIUS
2. Il RADIUS server recupera la password dal DB, la associa alla sfida e fa un hash MD5 locale di sfida + psw e la mette da parte.
3. Il RADIUS server autentica il client inviando in chiaro una sfida casuale (detta sfida MD5) che attraversa in tunnelling, senza modifiche, il dispositivo di accesso fino ad arrivare al client
4. Il client esegue la propria versione dell’hash di sfida + psw e la invia al server sempre con un tunnel che fa attraversare al messaggio il dispositivo di accesso senza alcuna modifica.
5. Il server confronta l’hash ricevuto con quello messo da parte in precedenza e, se coincidono, autorizza il NAS a consentire l’accesso con un messaggio RADIUS Access-Accept.


### **Autenticazione AP**

L’AP si autentica presso il server RADIUS generalmente tramite una password che è conservata nel files clients.conf di freeradius. Il server Univention lo imposta lui automaticamente dopo che l’utente lo ha inserito come parametro della configurazione dell’AP.

<img src="img/supplicantconf.png" alt="alt text" width="800">

>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

