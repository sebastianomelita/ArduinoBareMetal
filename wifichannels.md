>[Torna a WiFi infrastruttura](archwifi.md)>[Torna a WiFi mesh](archmesh.md)

- [Dettaglio architettura Ethernet](archeth.md)
- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

## **Allocazione dei canali alle celle**

<img src="img/wifichannel.png" alt="alt text" width="500">

### **Allocazione canali a 2.4 GHz**

A 2.4GHz il WiFi adopera 11 canali FDM con sovrapposizione parziale dei vari canali. Solo tre canali di volta in volta non si sovrappongono.
Una disposizione di celle adiacenti ottima dovrebbe utilizzare possibilmente solo tre canali non sovrapposti per minimizzare l’interferenza cocanale.

<img src="img/wifiband.png" alt="alt text" width="700">

Le possibilità sono: 1-6-11, 2-7-12, 3-8-13, 4-9, 5-10 ma rimangono solo 1-6-11 , 5-10 perché le frequenze 12, 13 e 14 non sono autorizzate in Europa (alcuni device supportano il canale 12).
quindi si hanno **più frequenze** solo usando la combinazione **1-6-11**

### **Allocazione canali a 5 GHz**

In questa banda è disponibile uno spettro significativamente più ampio ed anche possibile impostare la larghezza del canale. Cciascun canale occupa la propria sezione a 20 MHz senza sovrapposizioni con altri canali. 

<img src="img/wifiband5g.png" alt="alt text" width="700">

802.11n fornisce la possibilità di utilizzare i canali a 40 MHz, 802.11ac consente canali larghi 80MHz e persino 160MHz. Maggiore ampiezza del canale consente, in teoria, bitrate notevolmente maggiori.
Questi canali più ampi vengono creati collegando insieme i canali a 20 MHz. Raddoppiare l’ampiezza dei canali ha però l’inconveniente di raddoppiare la quantità di rumore e di ridurre le combinazioni di canali che non si sovrappongono, entrambe le circostanze possono di fatto ridurre la bitrate.

<img src="img/reuse5g-20mhz.png" alt="alt text" width="700">

<img src="img/reusepatterns5g.png" alt="alt text" width="700">

### **Pattern di riuso a confronto**

<img src="img/reusepatterns.png" alt="alt text" width="700">


### **Pagine correlate:**

>[Torna a WiFi infrastruttura](archwifi.md)>[Torna a WiFi mesh](archmesh.md)

- [Dettaglio architettura Ethernet](archeth.md)
- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 


### **Sitografia**:
- www.master-iot.it Master Universitario di II Livello "Internet of Things and Big Data" A.A.2018-2019 Università degli Studi di Genova
- https://www.researchgate.net/publication/300111853_Long-Range_IoT_Technologies_The_Dawn_of_LoRa
- https://www.researchgate.net/figure/The-Wi-Fi-trilateration-Technique_fig4_277307295
- https://it.wikipedia.org/wiki/Trilaterazione
- https://en.wikipedia.org/wiki/Beacon_frame
- https://www.metageek.com/training/resources/design-dual-band-wifi.html
- http://mce.it/index.php/airmax-5-ghz
- https://www.semfionetworks.com/blog/co-channel-contention-ccc-explained-with-simple-drawings
- https://www.ekahau.com/blog/2019/04/18/channel-planning-best-practices-for-better-wi-fi/
- https://documentation.meraki.com/Architectures_and_Best_Practices/Cisco_Meraki_Best_Practice_Design/Best_Practice_Design_-_MR_Wireless/High_Density_Wi-Fi_Deployments
- https://7signal.com/802-11ac-migration-part-2-whats-nobodys-telling-you-about-80mhz-and-160mhz-channel-bonding/
- https://www.elettra2000.it/phocadownload/atti-convegni/andreuccetti.pdf
- https://community.cisco.com/t5/wireless/mesh-mode-vs-bridge-mode/td-p/4094691
- https://www.cisco.com/c/en/us/support/docs/wireless-mobility/wireless-lan-wlan/68087-bridges-pt-to-pt.html
- https://www.tp-link.com/it/support/faq/151/
- https://indomus.it/formazione/shelly-mqtt-e-http-comandi-utili/
- https://forum.inductiveautomation.com/t/shelly-relay-1-pro-mqtt-command-for-turn-switch-on-off/81137
- https://www.zeteon.com/www/pages/wireless-mesh/
- https://interline.pl/Information-and-Tips/Mesh-Routing-And-Wireless-Distribution-Systems-WDS
- https://www.researchgate.net/figure/System-architecture-of-Multi-channel-Wireless-Mesh-Network-End-users-mobile-devices_fig1_2911584
- https://www.researchgate.net/publication/324643692_SWAM_SDN-based_Wi-Fi_Small_Cells_with_Joint_Access-Backhaul_and_Multi-Tenant_Capabilities
- https://www.tp-link.com/us/blog/137/tp-link-usa-outdoor-solution-guide/
- https://it.wikipedia.org/wiki/Ad-hoc_On-demand_Distance_Vector
- https://it.wikipedia.org/wiki/Optimized_Link_State_Routing_Protocol
- https://www.draytek.com/support/knowledge-base/6002
- https://stevessmarthomeguide.com/wi-fi-mesh-networks/
- https://www.tp-link.com/cac/support/faq/418/
- https://www.researchgate.net/publication/277227642_Test_reports_-_Confrontation_between_experimental_and_numerical_results_Analysis_of_the_attenuation_of_the_WIFI_signals_inside_and_outside_a_railway_vehicle
- https://packetlife.net/blog/2012/feb/20/aironet-aps-bridge-groups-and-bvi/
- https://www.cisco.com/c/en/us/td/docs/wireless/controller/8-6/config-guide/b_cg86/workgroup_bridges.html
- https://www.cisco.com/c/en/us/td/docs/routers/access/wireless/software/guide/wireless_vlans.html
- https://www.cisco.com/c/en/us/td/docs/internetworking/design/guide/idg4.html
- https://www.cisco.com/c/en/us/td/docs/internetworking/design/guide/idg4.html

>[Torna a reti di sensori](sensornetworkshort.md#classificazione-delle-tecnologie-wsn-in-base-a-velocità-e-copertura)
