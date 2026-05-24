
## **Bridge group**

All'**interno** di ogni **AP**, in realtà, sono sempre presenti uno o più **bridge** realizzati in SW (creati  mediante il comando  bridge-group x) che hanno il compito di **associare** il traffico delle **interfacce wireless**  con le **interfacce Ethernet** della reta cablata.

Le **interfacce wireless** fisiche sono divise in più **sotto interfacce** logiche, ciascuna con il proprio **SSID** (veri e propri Hub wireless separati da un ssid).

Anche le interfacce ethernet fisiche sono divise in più **sotto interfacce** logiche, ciascuna con il proprio vlan id.

<img src="..img/ap_bridge_groups.png" alt="alt text" width="1000">

All'interno di ogni AP, in realtà, sono sempre presenti uno o più bridge realizzati in SW (creati mediante il comando bridge-group x) che hanno il compito di collegare il traffico delle interfacce wireless con le interfacce Ethernet della rete LAN
Le interfacce wireless fisiche sono divise in più sotto interfacce logiche, ciascuna con il proprio SSID (veri e propri Hub wireless separati da un ssid). Esiste un bridge diverso associato ad ogni VLAN che collega il traffico wireless 802.11 di una VLAN con il traffico Ethernet 802.3 della stessa VLAN.

Anche le interfacce ethernet fisiche (Trunk) sono divise in più sotto interfacce logiche, ciascuna con il proprio vlan id.

L'**associazione** di una interfaccia ad una VLAN può essere:
- **Statica**, associa ad ogni SSID la sua VLAN e si imposta sul controller degli AP
- **Dinamica**: associa ad ogni username la sua VLAN e si imposta sul server Radius con:
  - **Una ACL** sul file di configurazione users del server radius 
  - **Una impostazione** nel modulo eap in mods-available che setta use_tunneled_reply = yes

Esempio di configurazione di due radio (a 2.4 GHz e 5 GHz) per gestire due SSID ciascuna associati a due VLAN diverse:

#### **Configurazione globale**
```C++
dot11 ssid Corporate
   vlan 10
!
dot11 ssid Guest
   vlan 20
!
bridge irb
```

Un **BVI (Bridge Virtual Interface)** in un Access Point WiFi è un'**interfaccia logica** del bridge utilizzata per ottenere un unico punto di gestione per l'**indirizzamento IP** e altre configurazioni di rete. Con 2 bridge accade che il **bridge group 1** avrà il suo **BVI1**, mentre il **bridge group 2** avrà il suo **BVI2**. 

Due funzioni rilevanti vengono eseguite nello snippet sopra. Innanzitutto, i nostri due SSID (Corporate e Guest) vengono definiti e associati alle VLAN. In secondo luogo, il **routing e il bridging integrati (IRB)** vengono **abilitati** con il comando bridge irb. Ciò consente di definire gruppi di bridge e un BVI.

#### **Configurazione radio 0**
```C++
interface Dot11Radio0
 no ip address
 !
 ssid Corporate
 !
 ssid Guest
 !
 mbssid
!
interface Dot11Radio0.10
 encapsulation dot1Q 10
 bridge-group 1
!
interface Dot11Radio0.20
 encapsulation dot1Q 20
 bridge-group 2
```

#### **Configurazione radio 1**
```C++
 interface Dot11Radio1
 no ip address
 !
 ssid Corporate
 !
 ssid Guest
 !
 mbssid
!
interface Dot11Radio1.10
 encapsulation dot1Q 10
 bridge-group 1
!
interface Dot11Radio1.20
 encapsulation dot1Q 20
 bridge-group 2
```

#### **Configurazione IP**
```C++
 interface BVI1
 ip address 192.168.10.123 255.255.255.0
 no ip route-cache
```
Questa configurazione mantiene il traffico wireless appartenente a un SSID isolato dal traffico appartenente all'altro mentre transita l'access point dall'interfaccia cablata all'interfaccia wireless e viceversa. Nota che poiché non c'è un'interfaccia BVI2, l'access point non ha alcun indirizzo IP raggiungibile direttamente dall'SSID Guest.
