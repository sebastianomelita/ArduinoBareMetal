>[Torna a vpn](ethvpn.md)

- [Dettaglio architettura Ethernet](archeth.md)
- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

# **Tunnel GRE** 

GRE (Generic Routing Encapsulation) è un protocollo di tunneling sviluppato da Cisco Systems che può incapsulare una vasta gamma di protocolli di livello rete all'interno di collegamenti punto-punto virtuali. GRE è spesso utilizzato per creare tunnel che collegano reti remote o dispositivi di rete attraverso una rete IP, come Internet.

<img src="img/greencapsulation.jpg" alt="alt text" width="600">

Caratteristiche Principali di GRE
- Encapsulamento di Protocolli Multipli:

- GRE può incapsulare vari protocolli, permettendo il trasporto di pacchetti di rete di diversi tipi all'interno di un singolo tunnel.
Indipendenza dal Protocollo:

- GRE è indipendente dal protocollo e può trasportare qualsiasi protocollo di rete come IP, IPX, AppleTalk, ecc.
Tunneling Punto-a-Punto:

- GRE crea collegamenti punto-a-punto virtuali tra due endpoint di tunnel.
Header di Incapsulamento:

- GRE aggiunge un header di 24 byte al pacchetto incapsulato. L'header include informazioni di controllo, il protocollo incapsulato e un checksum opzionale per la verifica dell'integrità.

Vantaggi di GRE
- Flessibilità: GRE può trasportare qualsiasi protocollo di livello rete, rendendolo utile in diverse situazioni di rete.
- Compatibilità: GRE è ampiamente supportato su molte piattaforme e dispositivi di rete, inclusi router e firewall di vari produttori.
- Semplicità:GRE è relativamente semplice da configurare e gestire rispetto ad altri protocolli di tunneling più complessi.

Limitazioni di GRE
- Sicurezza: GRE di per sé non fornisce meccanismi di crittografia o autenticazione. Se la sicurezza è una preoccupazione, GRE è spesso utilizzato in combinazione con IPsec per fornire sicurezza ai dati incapsulati.
- Overhead: L'header aggiuntivo di GRE introduce un overhead aggiuntivo di 24 byte, che può influire sulle prestazioni e sull'efficienza della rete.

Esempio di Configurazione GRE su un Router Cisco
Supponiamo di avere due router (Router A e Router B) e vogliamo creare un tunnel GRE tra di loro. Ecco come configurarlo:

### **Sede A**

```C++
RouterA(config)# interface Tunnel0
RouterA(config-if)# ip address 10.0.0.1 255.255.255.252
RouterA(config-if)# tunnel source 192.168.1.1
RouterA(config-if)# tunnel destination 192.168.2.1
RouterA(config-if)# exit
```

### **Sede B**

```C++
RouterB(config)# interface Tunnel0
RouterB(config-if)# ip address 10.0.0.2 255.255.255.252
RouterB(config-if)# tunnel source 192.168.2.1
RouterB(config-if)# tunnel destination 192.168.1.1
RouterB(config-if)# exit
```

### **Verifica del Tunnel GRE** 

Per verificare che il tunnel GRE sia operativo, puoi usare i seguenti comandi:
```C++
Router# show ip interface brief
Router# show interfaces Tunnel0
Router# show ip route
Router# ping 10.0.0.2 source 10.0.0.1

```

>[Torna a vpn](ethvpn.md)

- [Dettaglio architettura Ethernet](archeth.md)
- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 


Sitografia:
