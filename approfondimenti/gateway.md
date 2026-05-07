>[Torna a reti di sensori](../sensornetworkshort.md)

### **Gateway applicativo**

E' il dispositivo posto a cavallo tra la rete di accesso ai sensori e la rete di distribuzione. 

Il **gateway** ha tante **schede di interfaccia** quanti sono i **tipi diversi di BUS** a cui si collega. Inoltre il **gateway** deve possedere almeno **una interfaccia** capace di traffico ethernet (cablata o wifi) che lo colleghi alla **rete di distribuzione**. 

#### **Gateway come router L7**

Avendo più interfacce su reti di tipo diverso sia in L1 che in L2, ha anche le funzioni di **router**. Se la rete di distribuzione è pubblica come **Internet** dovrebbe possedere pure le funzioni di **firewall**. Al limite potrebbe anche smistare messaggi in una **WAN privata** realizzata con **VPN** di tipo **trusted** (MPLS) o **secure** (OpenVPN, IPSec).

Il **gateway** ha anche la funzione di adattare il **formato dei servizi** offerti dalle varie **sottoreti di sensori** nel **formato di servizio unificato** (ad esempio un particolare messaggio JSON) con cui i sensori sono interrogati nella rete di distribuzione IP. I **protocolli di livello applicativo** utilizzati a questo scopo in genere sono **HTTPS** o **COAP** per il paradigma di interazione **Request/response** oppure **MQTT** o **Telegram** per il paradigma di interazione **Publish/Subscribe**, oppure **Websocket**, **Webhooks** e **WebRTC** per richieste asincrone, l'ultimo anche per quelle multimediali.

#### **Gateway chiusi**

Talvolta il livello applicativo dello stesso stack di protocolli, pur a parità di tecnologia, non è compatibile tra dispositivi di diversi fabbricanti (marca diversa). In più i **gateway** verso la rete di sensori IP, nonostante abbiano nella rete di distribuzione una **interfaccia MAC/IP** tradizionale, sono scatolotti **proprietari chiusi** che nascondono la traduzione delle API di servizio HTTP o COAP con i messaggi verso la rete di sensori che gestiscono. 

I fabbricanti spesso tendono a renderli **non bypassabili** perchè non rilasciano di pubblico dominio le API a basso livello per comunicare direttamente con i sensori. Questo significa che potrebbe essere necessario per loro un **ulteriore livello di traduzione**, cioè un altro gateway, per convertire il **formato** delle **API** e dei **protocolli** che espongono nella rete IP (ad es COAP) con quelle in uso nella rete di distribuzione comune per tutti i sensori (ad es. MQTT). 

#### **Gateway aperti**

Questa situazione, per i **protocolli più diffusi** come Zibgee e Bluetooth, **tende a scomparire** perchè quasi tutti i produttori si sono ormai accordati e **adottano** per le API dei servizi uno **standard comune** e pubblico, per cui non è insolito riuscire a comandare dispositivi di una marca X col gateway della marca Y. Inoltre, molti stack hanno reso pubblico il formato del payload applicativo dei loro sensori, oppure esistono bridge che eseguono la conversione di questo in corrispondenti messaggi MQTT in formato JSON.

#### **Sintesi delle funzioni**

**Riassumendo**, alla **rete di distribuzione IP** si collegano, quindi, una o più **reti secondarie** che servono da **rete di accesso** per i dispositivi sensori o attuatori con **interfacce** spesso di tipo **non ethernet** che necessitano di un **gateway** di confine con possibili funzioni di:     
  - **Inoltro**, cioè smistamento dei messaggi da un tipo di rete all'altro di tipo L3 (**routing**) o di tipo L2 (**bridging**). L'inoltro del messaggio di un sensore può essere:
       - **diretto** nella rete di distribuzione tramite link fisico verso il dispositivo di smistamento pubblico (router o switch) più vicino.
       - **indiretto** tramite una dorsale virtuale, cioè un **tunnel**, verso il network server o verso un router di una WAN privata, realizzato, ad esempio, in maniera cifrata tramite un **client di VPN**, oppure in maniera non cifrata tramite un client di tunnel generico **GRE**.
  - **Traduzione di formato** dei messaggi da rete a bus a rete ethernet con eventuale realizzazione del **bridge** L4 tra il livello applicativo in uso nella rete di sensori e quello in uso nella rete di distribuzione.
  - **Interrogazione periodica** (polling) dei dispositivi nella rete di sensori (master di una architettura master/slave)
  - **Raccolta e memorizzazione** delle informazioni per essere trasferite in un **secondo momento** al server di gestione
  - **Protezione della rete di sensori**, cioè di firewall, soprattutto quando questa, tramite il gateway, si connette direttamente alla rete **Internet** mediante un **IP pubblico**.
