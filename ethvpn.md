>>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

## **VPN di reti Ethernet** 

### **Esempio 1**

Nelle reti Informatiche, TUN e TAP sono driver che permettono la creazione di periferiche di rete virtuali. Rispetto alle comuni periferiche (ad es. eth0) che sono controllate direttamente dalle schede di rete, i pacchetti spediti da o verso dispositivi TUN/TAP sono spediti da o verso programmi software. TUN è in grado di simulare una periferica di rete di tipo punto-punto e lavora con pacchetti di tipo IP mentre TAP è in grado di simulare un dispositivo Ethernet e logicamente utilizza i frame Ethernet.

<img src="img/sediVPNL2.png" alt="alt text" width="400">

<img src="img/Tun-tap-osilayers-diagram.png" alt="alt text" width="1100">


<img src="img/sediVPNL2.png" alt="alt text" width="1100">
Quando due reti sono collegate tramite un tunnel bridged L2 OpenVPN, la risoluzione degli indirizzi MAC (ARP - Address Resolution Protocol) attraverso il tunnel avviene in modo simile a una rete locale fisica, ma con alcune differenze dovute alla natura del tunnel VPN.

Ecco come funziona una richiesta ARP tra due reti collegate da un tunnel bridged L2 OpenVPN:

1. **Richiesta ARP da parte del mittente**: Supponiamo che un dispositivo su una delle reti VPN voglia comunicare con un dispositivo sull'altra rete VPN. Il mittente invierà una richiesta ARP broadcast per ottenere l'indirizzo MAC del dispositivo di destinazione.

2. **Trasmissione attraverso il tunnel VPN**: La richiesta ARP broadcast, insieme a tutte le altre trame Ethernet, viene incapsulata in pacchetti VPN dal router VPN locale e inviata attraverso il tunnel VPN al router VPN remoto.

3. **Decapsulamento al router VPN remoto**: Una volta che i pacchetti VPN raggiungono il router VPN remoto, vengono decapsulati per esporre le trame Ethernet originali.

4. **Trasmissione sulla rete locale**: Le trame Ethernet originali, compresa la richiesta ARP broadcast, vengono inoltrate sulla rete locale del router VPN remoto.

5. **Risposta ARP dalla destinazione**: Il dispositivo di destinazione sulla rete VPN remota riceve la richiesta ARP e risponde con il proprio indirizzo MAC.

6. **Inoltro della risposta attraverso il tunnel VPN**: La risposta ARP, insieme a tutte le altre trame Ethernet, viene incapsulata in pacchetti VPN dal router VPN remoto e inviata attraverso il tunnel VPN al router VPN locale.

7. **Decapsulamento al router VPN locale**: Una volta che i pacchetti VPN raggiungono il router VPN locale, vengono decapsulati per esporre le trame Ethernet originali.

8. **Inoltro della risposta alla sorgente**: Il router VPN locale inoltra la risposta ARP alla sorgente originaria attraverso la sua rete locale.

In questo modo, la **risoluzione degli indirizzi MAC** tra le due reti collegate da un tunnel bridged L2 OpenVPN avviene attraverso il **tunnel VPN**, consentendo la comunicazione tra dispositivi su reti distinte come se fossero sulla **stessa rete locale**.

Sitografia:
- https://it.wikipedia.org/wiki/TUN/TAP

>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

