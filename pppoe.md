>[Torna a vpn](ethvpn.md)

- [Dettaglio architettura Ethernet](archeth.md)
- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

# **Tunnel PPPoE** 

PPPoE (Point-to-Point Protocol over Ethernet) è un protocollo di rete che incapsula i frame PPP (Point-to-Point Protocol) all'interno di frame Ethernet. Questo permette agli ISP di utilizzare il protocollo PPP per gestire connessioni a larga banda su reti Ethernet. PPPoE è comunemente utilizzato nelle connessioni DSL. 

<img src="img/ppoeencapsulation.jpeg" alt="alt text" width="600">

I **pacchetti IP** viaggiano incapsulati dentro trame PPP per tutta la tratta dall'utente alla centrale. Però, a **monte del modem** le trame PPP, a loro volta, viaggiano in tunnel incapsulate dentro trame MAC mentre dopo, a **valle del modem**, viaggiano da sole fino al modem in centrale.  

<img src="img/pppoe_architecture.gif" alt="alt text" width="500">

Esempio di Configurazione GRE su un Router Cisco

## **Esempio di Configurazione PPPoE su un Router Cisco**

Supponiamo di voler configurare un router Cisco per connettersi a un ISP tramite PPPoE. Ecco come configurarlo:

### **Client PPPoE**

```C++
interface Ethernet0/0
 no ip address
 pppoe enable group global
 pppoe-client dial-pool-number 1
 exit

interface Dialer1
 mtu 1492
 ip address negotiated
 encapsulation ppp
 dialer pool 1
 ppp chap hostname <tuo_username>
 ppp chap password <tua_password>
 end
```

### **Server PPPoE**

```C++
bba-group pppoe global
 virtual-template 1
 sessions per-mac limit 1
 exit

interface Virtual-Template1
 mtu 1492
 ip unnumbered Loopback0
 peer default ip address pool pppoe-pool
 ppp encrypt mppe auto
 ppp authentication chap callin
 end

ip local pool pppoe-pool 192.168.1.1 192.168.1.100
```

### **Verifica del Tunnel GRE** 

Per verificare che il tunnel GRE sia operativo, puoi usare i seguenti comandi:
```C++
show interfaces Dialer1
show running-config interface Ethernet0/0
show pppoe session
show pppoe summary
show pppoe interface Virtual-Template1
```

>[Torna a vpn](ethvpn.md)

- [Dettaglio architettura Ethernet](archeth.md)
- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 


