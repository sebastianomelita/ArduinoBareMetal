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

<img src="img/supplicant.png" alt="alt text" width="900">

Le porte controllate possono essere:
- Porte fisiche quali quelle ethernet di uno switch, sono sempre disponibili a livello fisico semplicemente collegando un plug ma possono risultare indisponibili a livello 2 della pila ISO/OSI se l’autenticazione 802.1 X non va a buon fine.
- Porte virtuali quali quelle di un AP. Se un utente Wi-Fi viene autenticato tramite 802.1 X per l'accesso alla rete, viene aperta una porta virtuale sul punto di accesso che consente la comunicazione. Se non è stato autorizzato correttamente, una porta virtuale non è disponibile e le comunicazioni sono bloccate.
Anche nel caso di un solo AP vengono attivate 2 porte virtuali a valle di quella fisica, una per gestire il protocollo di autenticazione e un’altra per controllare ed eventualmente bloccare il traffico dati.

>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

