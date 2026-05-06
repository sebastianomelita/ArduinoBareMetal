>[Torna a reti ethernet](../archeth.md)

- [Dettaglio architettura Zigbee](../archzigbee.md)
- [Dettaglio architettura BLE](../archble.md)
- [Dettaglio architettura WiFi infrastruttura](../archwifi.md)
- [Dettaglio architettura WiFi mesh](../archmesh.md) 
- [Dettaglio architettura LoraWAN](../lorawanclasses.md) 


## **Architettura di una rete di reti** 

Di seguito è riportata l'architettura generale di una **rete di reti** di sensori. Essa è composta, a **livello fisico**, essenzialmente di una **rete di accesso** ai sensori e da una **rete di distribuzione** che fa da collante di ciascuna rete di sensori.

<img src="sensornet1.png" alt="alt text" width="800">

### **Rete di distribuzione** 

I **gateway** utilizzano la **rete internet** e/o una **LAN** per realizzare un collegamento verso il **broker MQTT**, per cui, in definitiva, la topologia risultante è, **fisicamente**, quella di più **reti di accesso** con tecnologia e topologia differente (a maglia nel caso di zigbee) tenute insieme da una **rete di distribuzione** qualsiasi purchè sia di tipo TCP/IP (LAN o Internet).

Avere a disposizione una **rete di distribuzione IP** per i comandi e le letture è utile perchè permette di creare interfacce web o applicazioni per smartphone o tablet per:
- eseguire, in un'unica interfaccia (form), comandi verso attuatori posti su reti con tecnologia differente.
- riassumere in un'unica interfaccia (report) letture di sensori provenienti da reti eterogenee per tecnologia e topologia

## **Documentazione logica della rete (albero degli apparati attivi)** 

### **Reti di sensori federate tramite Internet** 

L'albero degli **apparati attivi** di una rete di sensori + rete di distribuzione **in Internet** + server di gestione e controllo che potrebbe rappresentare **tre edifici** distanti domotizzati tramite **zigbeee** e federati tramite **Internet**: 

<img src="img/integratedGW-Zigbee.png" alt="alt text" width="900">

Il **bridge zigbee** (in realtà è un **gateway** e quindi pure un router) è normalmente anche il **coordinatore** della rete di sensori. 

Il **gateway**, quando collegato direttamente ad **Internet**, è normalmente anche un **firewall** (con funzioni di NAT se si adopera IPv4), mentre se collegato alla **LAN** (attraverso uno SW o un HUB wiereless) ha solamente la **funzioni** di:
- **router applicativo** che **traduce** i messaggi da una rete IP (la LAN) ad una non IP (la rete di sensori).
- **client MQTT** con funzione di **publisher** (sul topic di stato e traduce **da** i dispositivi) e di **subscriber** (sui topic di comando e configurazione e traduce **verso** i dispositivi).

Il **gateway All-In-One** potrebbe essere un dispositivo con **doppia interfaccia**, modem **UMTS** per l'accesso alla rete di distribuzione su **Internet**, **Zigbee** verso la **rete di sensori**. Può essere utile per realizzare un **gateway Zigbee da campo** da adoperare:
- in **contesti occasionali** (fiere, eventi sportivi, infrastrutture di emergenza, grandi mezzi mobili).
- in contesti simili ma **dispersi** in aree geografiche molto distanti tra loro e coperte solo dalla **rete cellulare** terrestre della telefonia mobile o dai **satelliti in orbita bassa (LEO)**.

### **Reti di sensori federate tramite LAN** 

#### **Partizionamento e ridondanza** 

Per quanto riguarda il **numero dei gateway** in una stessa **LAN**, il numero minimo necessario perchè la rete zigbee funzioni è **uno**. Un gateway avente anche funzione di **coordinatore** nelle rete di sensori. Però, data la **criticità** di eventuali **guasti** su questo dispositivo (la rete di sensori diventa nel suo complesso **inaccessibile**), potrebbe essere opportuno prevedere:
- localmente la **ridondanza dei gateway**. Almeno 2 gateway per ogni rete di sensori. Uno master attivo di default, e uno slave che entra in azione quando sente che il proprio master è non raggiungibile.
- globalmente un **partizionamento della rete** di sensori in più settori con frequenze diverse e gestiti da coordinatori diversi inseriti in **più gateway sparsi** in **zone diverse** dell'impianto.

La **partizione** di una rete Zigbee potrebbe essere utile anche in determinate situazioni, specialmente quando si hano un **gran numero** di dispositivi o se si vogliono **separare** i dispositivi **per zone** o **per scopi** diversi. Ecco alcune **situazioni** in cui potrebbe essere **vantaggioso partizionare** una rete Zigbee:

- **Grande numero di dispositivi**: Se hai un'enorme quantità di dispositivi Zigbee sulla stessa rete, potresti voler partizionare la rete in più sotto-reti per migliorare le prestazioni e la gestibilità. Questo può evitare sovraccarichi di traffico eccessivi su una singola rete.

- **Diverse zone o edifici**: Se la tua rete Zigbee si estende su diverse aree fisiche come piani diversi di un edificio o edifici separati, la partizione della rete può essere utile per mantenere la gestione e il controllo in modo più efficiente.

- **Scopi diversi**: Se i dispositivi Zigbee sono utilizzati per scopi diversi, come illuminazione, sicurezza, controllo del clima, ecc., potrebbe essere utile separare i dispositivi in reti dedicate per ciascuno scopo. Questo può semplificare la gestione e consentire configurazioni e politiche di sicurezza specifiche per ogni tipo di dispositivo.

Per **partizionare** una rete Zigbee, si potrebbero creare **più coordinatori** Zigbee, cioè più **gateway**, ciascuno con la propria rete di sensori da gestire, e utilizzare una **LAN** (composta da più switch) per collegare le reti tra loro. 

<img src="img/integratedGW-ZigbeeLAN.png" alt="alt text" width="900">
