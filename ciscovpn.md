>[Torna a vpn](ethvpn.md)

- [Dettaglio architettura Ethernet](archeth.md)
- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

# **VPN di reti Ethernet** 

In una configurazione VPN IPsec tra due router Cisco, l'indirizzo IP privato del tunnel (indirizzo di overlay) non è visibile o configurato esplicitamente come si farebbe con una VPN basata su interfaccia virtuale (come GRE o VTI). Invece, la VPN IPsec incapsula il traffico tra le due subnet locali direttamente.

### **Sede A**

```C++
! Configurazione dell'interfaccia WAN e LAN
interface GigabitEthernet0/0
 ip address 203.0.113.1 255.255.255.0
 no shutdown
!
interface GigabitEthernet0/1
 ip address 192.168.1.1 255.255.255.0
 no shutdown
!
! ACL per il traffico IPsec
access-list 100 permit ip 192.168.1.0 0.0.0.255 192.168.2.0 0.0.0.255
!
! Configurazione ISAKMP (Phase 1)
crypto isakmp policy 1
 encr aes
 hash sha
 authentication pre-share
 group 2
 lifetime 86400
!
! Configurazione della chiave precondivisa
crypto isakmp key cisco123 address 198.51.100.2
!
! Configurazione del Transform Set (Phase 2)
crypto ipsec transform-set TS esp-aes esp-sha-hmac
!
! Configurazione del Crypto Map
crypto map VPNMAP 10 ipsec-isakmp
 set peer 198.51.100.2
 set transform-set TS
 match address 100
!
! Applicazione della Crypto Map all'interfaccia WAN
interface GigabitEthernet0/0
 crypto map VPNMAP
!
```

### **Sede B**

```C++
! Configurazione dell'interfaccia WAN e LAN
interface GigabitEthernet0/0
 ip address 198.51.100.2 255.255.255.0
 no shutdown
!
interface GigabitEthernet0/1
 ip address 192.168.2.1 255.255.255.0
 no shutdown
!
! ACL per il traffico IPsec
access-list 100 permit ip 192.168.2.0 0.0.0.255 192.168.1.0 0.0.0.255
!
! Configurazione ISAKMP (Phase 1)
crypto isakmp policy 1
 encr aes
 hash sha
 authentication pre-share
 group 2
 lifetime 86400
!
! Configurazione della chiave precondivisa
crypto isakmp key cisco123 address 203.0.113.1
!
! Configurazione del Transform Set (Phase 2)
crypto ipsec transform-set TS esp-aes esp-sha-hmac
!
! Configurazione del Crypto Map
crypto map VPNMAP 10 ipsec-isakmp
 set peer 203.0.113.1
 set transform-set TS
 match address 100
!
! Applicazione della Crypto Map all'interfaccia WAN
interface GigabitEthernet0/0
 crypto map VPNMAP
!
```



>[Torna a vpn](ethvpn.md)

- [Dettaglio architettura Ethernet](archeth.md)
- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 


Sitografia:
