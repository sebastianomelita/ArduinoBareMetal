>[Torna a vpn](ethvpn.md)

- [Dettaglio architettura Ethernet](archeth.md)
- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

# **Configurazione servizio Open VPN su Pfsense**

<img src="img/diagrams-openvpn-server.png" alt="alt text" width="1100">

Questo esempio mostra la configurazione della una VPN Home to Site OpenVPN installata sul firewall commerciale Pfsense.

## **Configurazione protocollo trasporto**

<img src="img/modo-ip_pubblico.png" alt="alt text" width="1100">

In questa fase vengono impostati:
- il **tipo di tunnel**, si può scegliere tra:
    - **tun**: crea una interfacccia L3, cioè il protocollo **passeggero** è l'**IP**
    - **tap**: crea una interfacccia L2, cioè il protocollo **passeggero** è il **MAC**
- l'**indirizzo** dell'interfaccia pubblica, cioè l'**IP pubblico** della **destinazione** a cui si collegherà il client VPN. Il protocollo di **trasporto** è l'IP con indirizzo pubblico di router/server VPN e client VPN (router di casa).
- il **tipo** (TCP o UDP) e il **numero di porta**. Di default sono **UDP** e **1194** ma possono anche esssere modificati a piacimento in TCP e in un qualsiasi altro numero di porta
- il protocollo **carrier** è, ovviamente, **OpenVPN** che introduce un proprio header tra l'intestazione di trasporto  e quella del protocollo passeggero.

## **Configurazione del protocollo passeggero**

<img src="img/opnevpn_tunnel.png" alt="alt text" width="1100">

In questa fase vengono impostati:
- la **subnet di accesso** comune a tutti i client VPN che si connettono (anche contemporaneamente). Questa è la rete virtuale IPv4 utilizzata per le comunicazioni private tra questo server e gli host client espressi utilizzando la notazione CIDR (ad esempio 10.0.8.0/24). Il primo indirizzo utilizzabile nella rete verrà assegnato all'interfaccia virtuale del server. I rimanenti indirizzi utilizzabili verranno assegnati ai client in connessione
- **Elenco delle subnet** della rete del server che saranno accessibili dal client VPN.

<img src="img/openvpn_passeggero.png" alt="alt text" width="1100">

Specifica il metodo utilizzato per fornire un indirizzo IP dell'adattatore virtuale ai client quando si utilizza la modalità TUN su IPv4.

La subnet di accesso può essere partizionata:
- mediante **subnetting automatico**, in tante **subnets /30** punto-punto, allocata dinamicamente a ciascun client che man mano si collega al server. E' la modalità che garantisce maggior isolamento in quanto i vari client non si vedono.
- utilizzando una **subnet comune** non partizionata, assegnando dinamicamente il primo indirizzo libero ad ogni client VPN che man mano si connette al server. E' la modalità che garantisce minor isolamento in quanto i vari client si vedono tra loro.
  
## **Configurazione della crittogfrafia**

<img src="img/openvpn_crittografia.png" alt="alt text" width="1100">

ipconfig

``` C++
Scheda sconosciuta OpenVPN Wintun:

   Suffisso DNS specifico per connessione:
   Indirizzo IPv6 locale rispetto al collegamento . : fe80::3d2b:be72:4201:26ac%7
   Indirizzo IPv4. . . . . . . . . . . . : 10.29.2.3
   Subnet mask . . . . . . . . . . . . . : 255.255.255.0
   Gateway predefinito . . . . . . . . . :
```
route print

``` C++
Elenco interfacce
  7...........................Wintun Userspace Tunnel
 16...00 ff 95 d7 fa 38 ......TAP-Windows Adapter V9
 20...04 6c 59 c3 34 45 ......Microsoft Wi-Fi Direct Virtual Adapter
  6...06 6c 59 c3 34 44 ......Microsoft Wi-Fi Direct Virtual Adapter #2
 18...04 6c 59 c3 34 44 ......Intel(R) Wi-Fi 6 AX201 160MHz
  5...04 6c 59 c3 34 48 ......Bluetooth Device (Personal Area Network)
  1...........................Software Loopback Interface 1
===========================================================================

IPv4 Tabella route
===========================================================================
Route attive:
     Indirizzo rete             Mask          Gateway     Interfaccia Metrica
          0.0.0.0          0.0.0.0     192.168.10.1   192.168.10.187     50
         10.0.0.0      255.128.0.0        10.29.2.1        10.29.2.3    261
        10.29.2.0    255.255.255.0         On-link         10.29.2.3    261
        10.29.2.3  255.255.255.255         On-link         10.29.2.3    261
      10.29.2.255  255.255.255.255         On-link         10.29.2.3    261
        127.0.0.0        255.0.0.0         On-link         127.0.0.1    331
        127.0.0.1  255.255.255.255         On-link         127.0.0.1    331
  127.255.255.255  255.255.255.255         On-link         127.0.0.1    331
       172.16.1.0    255.255.255.0        10.29.2.1        10.29.2.3    261
      192.168.2.0    255.255.255.0        10.29.2.1        10.29.2.3    261
      192.168.3.0    255.255.255.0        10.29.2.1        10.29.2.3    261
     192.168.10.0    255.255.255.0         On-link    192.168.10.187    306
   192.168.10.187  255.255.255.255         On-link    192.168.10.187    306
   192.168.10.255  255.255.255.255         On-link    192.168.10.187    306
        224.0.0.0        240.0.0.0         On-link         127.0.0.1    331
        224.0.0.0        240.0.0.0         On-link         10.29.2.3    261
        224.0.0.0        240.0.0.0         On-link    192.168.10.187    306
  255.255.255.255  255.255.255.255         On-link         127.0.0.1    331
  255.255.255.255  255.255.255.255         On-link         10.29.2.3    261
  255.255.255.255  255.255.255.255         On-link    192.168.10.187    306
===========================================================================
``` 
Sitografia:
- https://docs.netgate.com/pfsense/en/latest/vpn/openvpn/index.html
- https://docs.netgate.com/pfsense/en/latest/recipes/openvpn-ra.html
