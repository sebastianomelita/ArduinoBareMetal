>>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

# **VPN di reti Ethernet** 

## **Interfacce virtuali tun e tap**

Nelle reti Informatiche, TUN e TAP sono driver che permettono la creazione di periferiche di rete virtuali. Rispetto alle comuni periferiche (ad es. eth0) che sono controllate direttamente dalle schede di rete, i pacchetti spediti da o verso dispositivi TUN/TAP sono spediti da o verso programmi software. TUN è in grado di simulare una periferica di rete di tipo punto-punto e lavora con pacchetti di tipo IP mentre TAP è in grado di simulare un dispositivo Ethernet e logicamente utilizza i frame Ethernet.

<img src="img/Tun-tap-osilayers-diagram.png" alt="alt text" width="400">


Le interfacce TAP e tun possono essere utilizzate per instradare il traffico di rete attraverso il tunnel VPN. 

## **VPN L3**

In questo tipo di VPN i pacchetti con indirizzo IP privato, appartenenti ad una subnet in cui è partizionata una **rete LAN**, vengono **imbustati** all'interno dei pacchetti **IP pubblici** della rete **Internet**, realizzando così un **tunnel L3 su L3**. Il processo di **imbustamento** del tunnel avviene per tutti i pacchetti che vengono **inoltrati** verso l'interfaccia viruale **tun**.

Infatti, le **interfacce virtuali tun** compaiono nella **routing table** come **interfacce locali** verso cui inoltrare **pacchetti IP** destinati ad una **subnet direttamente connessa**.

```
Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
0.0.0.0         192.168.1.1     0.0.0.0         UG    0      0        0 eth0
10.8.0.0        0.0.0.0         255.255.255.0   U     0      0        0 tun0
```
 
<img src="img/sediVPNL2.png" alt="alt text" width="1100">

## **VPN L2**

In questo tipo di VPN le trame MAC con indirizzo locale, appartenenti ad una LAN remota in cui è partizionata una **rete LAN**, vengono **imbustati** all'interno dei pacchetti **IP pubblici** della rete **Internet**, realizzando così un **tunnel L2 su L3**. Il processo di **imbustamento** del tunnel avviene per tutti i pacchetti che vengono **inoltrati** verso l'interfaccia viruale **tap**.

Le **interfacce virtuali tap** compaiono nella **tabella di inoltro** del bridge come **interfacce locali** verso cui inoltrare **trame MAC** destinate ad host raggiungibili attraverso quella interfaccia.

Quando due reti sono collegate tramite un tunnel bridged L2 OpenVPN, la risoluzione degli indirizzi MAC (ARP - Address Resolution Protocol) attraverso il tunnel avviene in modo simile a una rete locale fisica, ma con alcune differenze dovute alla natura del tunnel VPN. Ecco come funziona una richiesta ARP tra due reti collegate da un tunnel bridged L2 OpenVPN:
1. **Richiesta ARP da parte del mittente**: Supponiamo che un dispositivo su una delle reti VPN voglia comunicare con un dispositivo sull'altra rete VPN. Il mittente invierà una richiesta ARP broadcast per ottenere l'indirizzo MAC del dispositivo di destinazione.
2. **Trasmissione attraverso il tunnel VPN**: La richiesta ARP broadcast, insieme a tutte le altre trame Ethernet, viene incapsulata in pacchetti VPN dal router VPN locale e inviata attraverso il tunnel VPN al router VPN remoto.
3. **Decapsulamento al router VPN remoto**: Una volta che i pacchetti VPN raggiungono il router VPN remoto, vengono decapsulati per esporre le trame Ethernet originali.
4. **Trasmissione sulla rete locale**: Le trame Ethernet originali, compresa la richiesta ARP broadcast, vengono inoltrate sulla rete locale del router VPN remoto.
5. **Risposta ARP dalla destinazione**: Il dispositivo di destinazione sulla rete VPN remota riceve la richiesta ARP e risponde con il proprio indirizzo MAC.
6. **Inoltro della risposta attraverso il tunnel VPN**: La risposta ARP, insieme a tutte le altre trame Ethernet, viene incapsulata in pacchetti VPN dal router VPN remoto e inviata attraverso il tunnel VPN al router VPN locale.
7. **Decapsulamento al router VPN locale**: Una volta che i pacchetti VPN raggiungono il router VPN locale, vengono decapsulati per esporre le trame Ethernet originali.
8. **Inoltro della risposta alla sorgente**: Il router VPN locale inoltra la risposta ARP alla sorgente originaria attraverso la sua rete locale.

In questo modo, la **risoluzione degli indirizzi MAC** tra le due reti collegate da un tunnel bridged L2 OpenVPN avviene attraverso il **tunnel VPN**, consentendo la comunicazione tra dispositivi su reti distinte come se fossero sulla **stessa rete locale**.

<img src="img/sediVPNL2.png" alt="alt text" width="1100">

Sitografia:
- https://it.wikipedia.org/wiki/TUN/TAP

>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

