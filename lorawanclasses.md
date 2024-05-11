
>[Torna a reti di sensori](sensornetworkshort.md#classificazione-delle-tecnologie-wsn-in-base-a-velocità-e-copertura)

Di seguito è riportata l'architettura generale di una rete Lorawan. Essa è composta, a **livello fisico**, essenzialmente di una **rete di accesso** ai sensori e da una **rete di distribuzione** che fa da collante di ciascuna rete di sensori.

<img src="img/lorawanArchitecture.png" alt="alt text" width="1000">

La rete di sensori fisica è a stella dove il centro stella è il gateway. Un sensore può anche essere associato a più gateway ed inviare dati a tutti i gateway a cui esso è associato. I dati normalmente arrivano ad un certo dispositivo attraverso un solo gateway.

I gateway utilizzano la rete internet (o una LAN) per realizzare un collegamento diretto **virtuale** con il network server, per cui, in definitiva, la topologia risultante è:
- **fisicamente** quella di più **reti di accesso** a stella tenute insieme da una **rete di distribuzione** qualsiasi purchè sia di tipo TCP/IP (LAN o Internet).
- **logicamente** quella di una stella di reti a stella. Il **centrostella** di livello gerarchico più alto è il **network server** ed aggrega solo gateways, gli altri centrostella sono realizzati dai **gateway** che aggregano solamente **dispositivi IoT**.

  <img src="img/lorawanLogicArchitecture.png" alt="alt text" width="1000">
  <img src="lpwan.png" alt="alt text" width="1000">

**Riassumendo**, alla **rete di distribuzione IP** si collegano, quindi, una o più **reti secondarie** che servono da **rete di accesso** per i dispositivi sensori o attuatori con **interfacce** spesso di tipo **non ethernet** che necessitano di un **gateway** di confine con possibili funzioni di:     
  - **Inoltro**, cioè smistamento dei messaggi da un tipo di rete all'altro di tipo L3 (**routing**) o di tipo L2 (**bridging**). L'inoltro del messaggio di un sensore può essere:
       - **diretto** nella rete di distribuzione tramite link fisico verso il dispositivo di smistamento pubblico (router o switch) più vicino.
       - **indiretto** tramite una dorsale virtuale, cioè un **tunnel**, verso il network server o verso un router di una WAN privata, realizzato, ad esempio, in maniera cifrata tramite un **client di VPN**, oppure in maniera non cifrata tramite un client di tunnel generico **GRE**.
  - **Traduzione di formato** dei messaggi da rete a bus a rete ethernet con eventuale realizzazione del **bridge** L4 tra il livello applicativo in uso nella rete di sensori e quello in uso nella rete di distribuzione.
  - **Interrogazione periodica** (polling) dei dispositivi nella rete di sensori (master di una architettura master/slave)
  - **Raccolta e memorizzazione** delle informazioni per essere trasferite in un **secondo momento** al server di gestione
  - **Protezione della rete di sensori**, cioè di firewall, soprattutto quando questa, tramite il gateway, si connette direttamente alla rete **Internet** mediante un **IP pubblico**.

Esempio di connessione alla rete di distribuzione IP tramite gateway dotati di client VPN:

  <img src="img/integratedGW-LoRa.png" alt="alt text" width="1000">

Funzioni dell'architettura. possono essere distinte su 3 prodotti dispositivi di rete diversi oppure coincidere in un unico dispositivo che le ingloba tutte.

Il **packet forwarder** è il gateway stesso essendo la funzione di forwarding, (inoltro) dei messaggi dal sensore al network server, la funzione principale del gateway. L'inoltro dei messaggi di uno stesso sensore può essere **parallelo** se questo è associato a più di un gateway, circostanza che aumenta la ridondanza a costo della presenza di duplicati in rete. In ogni caso, i **messaggi duplicati** verranno successivamente scartati dal network server, prima dell'inoltro finale verso l'applicazione.

Il **network server**, responsabile dell'inoltro di messaggi senza duplicati verso il **server di applicazione** (ad es. nodered) tramite protocolli **client server** di alto livello in modalità request/response (HTTPS, COAP) oppure publish/subscribe (bridge MQTT).

Il **join server**, o server di associazione,si occupa delle funzioni di autenticazione ed autorizzazione dei sensori in fase di registrazione e poi di riconoscimento e di gestione delle connessioni in fase di sessione:
- **Join Request Validation**: Il Join Server verifica la validità delle richieste di join inviate dai dispositivi end-device. Questo processo include la verifica delle firme digitali e altre informazioni di autenticazione fornite dal dispositivo. Può avvenire tramite il processo **Over-the-Air Activation (OTAA)**, con cui il Join Server fornisce al dispositivo le **chiavi di sessione** necessarie per stabilire una connessione sicura con il Network Server, in maniera automatica. Nel **modo ABP** deve essere invece l'utente a generare ed inserire le chiavi di sessione nel join server.      
- **Gestione delle sessioni**: Il Join Server mantiene il contesto di sicurezza per i dispositivi che sono stati autorizzati a unirsi alla rete. Questo include la gestione delle chiavi di sessione e altre informazioni di sicurezza necessarie per garantire una comunicazione sicura tra il dispositivo e il Network Server.
    
    
### **Formato del payload**

 <img src="img/messaggilora.png" alt="alt text" width="1000">
 
I messaggi scambiati in una rete LoraWAN sono complessivamente di due tipi che si mappano l’uno sull’altro:
- **Messaggi corti**, in formato binario, tra sensore e gateway. Vengono mandati in wireless su **banda ISM** con forti limitazioni di duty cycle, per cui devono essere i più **corti** possibile, anche a discapito della chiarezza. Possono essere **definiti** sotto forma di **struct C** e poi inviati ad una **libreria di serializzazione** che si occupa di trasformali in una **sequenza compatta** di singoli bit.
- **Messaggi lunghi** tra gateway e network server. Vengono mandati **in Internet** e devono essere più che altro chiari e, se possibile, autoesplicativi. Dato che vengono inviati su un mezzo senza particolari limitazioni di banda, possono essere **definiti** in **formato JSON**.

### Esempio JSON solo sensori

```C++
{
  "device_id": "1234567890ABCDEF",
  "timestamp": "2024-05-07T12:30:45Z",
  "sensor_data": {
    "temperature": 25.5,
    "humidity": 60.2,
    "battery_voltage": 3.7
  },
  "other_information": {
    "location": {
      "latitude": 40.7128,
      "longitude": -74.0060
    },
    "signal_strength": -110
  }
}
```

### Esempio JSON sensori + attuatori

```C++
{
  "device_id": "1234567890ABCDEF",
  "timestamp": "2024-05-07T12:30:45Z",
  "sensor_data": {
    "temperature": 25.5,
    "humidity": 60.2,
    "battery_voltage": 3.7
  },
  "actuator_data": {
    "led": {
      "status": "on",
      "color": "red"
    },
    "motor": {
      "status": "off",
      "speed": 0
    }
  },
  "other_information": {
    "location": {
      "latitude": 40.7128,
      "longitude": -74.0060
    },
    "signal_strength": -110
  }
}
```

### Esempio struct sensori + attuatori

```C++
#include <iostream>

const int MAX_STRING_LENGTH = 100; // Lunghezza massima per le stringhe

// Definizione della struttura per il payload completo
struct LoRaPayload {
    char device_id[MAX_STRING_LENGTH];
    char timestamp[MAX_STRING_LENGTH];
    float temperature;
    float humidity;
    float battery_voltage;
    char led_status[MAX_STRING_LENGTH];
    char led_color[MAX_STRING_LENGTH];
    char motor_status[MAX_STRING_LENGTH];
    int motor_speed;
    char location[MAX_STRING_LENGTH];
    int signal_strength;
};
```
### Esempio serializzazione sensori + attuatori con la libreria Cayenne

Per la serializzazione dei dati utilizzando il formato Cayenne LPP (Low Power Payload), possiamo adattare la struttura LoRaPayload in modo che i dati siano organizzati secondo le specifiche di Cayenne LPP. Cayenne LPP è un formato compatto e standardizzato per la trasmissione di dati attraverso reti LPWAN (Low Power Wide Area Network), progettato per dispositivi a basso consumo energetico come i dispositivi IoT. Questo formato consente di rappresentare diversi tipi di dati (ad esempio, temperatura, umidità, tensione) in un formato ottimizzato per la trasmissione su reti LPWAN.

Ecco un esempio di come potrebbe apparire una struttura LoRaPayload con la serializzazione Cayenne LPP:
```C++
#include <iostream>

const int MAX_PAYLOAD_SIZE = 100; // Dimensione massima del payload Cayenne LPP

// Definizione della struttura per il payload Cayenne LPP
struct CayenneLPP {
    uint8_t buffer[MAX_PAYLOAD_SIZE];
    int size; // Dimensione effettiva del payload
};

// Funzione per aggiungere un valore di tipo float al payload Cayenne LPP
void addFloatToCayenneLPP(CayenneLPP &lpp, uint8_t channel, float value) {
    lpp.buffer[lpp.size++] = channel;
    lpp.buffer[lpp.size++] = 0x02; // Tipo di dati (float)
    lpp.buffer[lpp.size++] = value >> 8; // Byte più significativo del float
    lpp.buffer[lpp.size++] = value & 0xFF; // Byte meno significativo del float
}

// Funzione per aggiungere un valore di tipo int al payload Cayenne LPP
void addIntToCayenneLPP(CayenneLPP &lpp, uint8_t channel, int value) {
    lpp.buffer[lpp.size++] = channel;
    lpp.buffer[lpp.size++] = 0x02; // Tipo di dati (int)
    lpp.buffer[lpp.size++] = value >> 8; // Byte più significativo dell'intero
    lpp.buffer[lpp.size++] = value & 0xFF; // Byte meno significativo dell'intero
}

int main() {
    // Esempio di utilizzo della struttura per creare un payload Cayenne LPP
    CayenneLPP lpp;
    lpp.size = 0; // Inizializza la dimensione del payload a 0
    addFloatToCayenneLPP(lpp, 1, 25.5); // Canale 1: Temperatura
    addFloatToCayenneLPP(lpp, 2, 60.2); // Canale 2: Umidità
    addFloatToCayenneLPP(lpp, 3, 3.7);  // Canale 3: Tensione della batteria
    addIntToCayenneLPP(lpp, 101, -110); // Canale 101: Forza del segnale

    // Output dei dati del payload Cayenne LPP
    std::cout << "Cayenne LPP Payload:" << std::endl;
    for (int i = 0; i < lpp.size; ++i) {
        std::cout << std::hex << static_cast<int>(lpp.buffer[i]) << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

## **Messaggi MQTT**

### **Messaggi confermati**

La **conferma** dei messaggi inviati da parte del ricevente normalmente non è necessaria nel caso dei **sensori**. Infatti, se un invio da parte di un sensore non andasse a buon fine, è inutile richiedere la ritrasmissione di un dato che comunque a breve arriva con una misura più aggiornata. 

La conferma, invece, è prevista per funzioni di **comando** o **configurazione**.  Ad esempio  nel caso di pulsanti, rilevatori di transito o allarmi in cui l'invio del messaggio avviene sporadicamente e in maniera del tutto **asincrona** (cioè non prevedibile dal ricevitore), potrebbe essere auspicabile avere un feedback da parte del protocollo mediante un meccanismo di conferma basato su **ack**. Ma non sempre ciò è possibile.

La **conferma**, però, potrebbe pure essere gestita soltanto dal **livello applicativo** (non dal protocollo) utilizzando un **topic di feeedback** (o stato) per inviare il valore dello stato corrente subito dopo che questo viene interessato da un comando in ingresso sul dispositivo. 

### **Definizione di topic e payload**

Sovente, nella rete di distribuzione IP è presente un server col ruolo di **broker MQTT** a cui sono associati:
- su un **topic di misura o attuazione (comando)**:
    - il dispositivo **sensore** è registrato sul broker col ruolo di **publisher** perchè vuole usare questo canale di output per **inviare il comando** verso l'attuatore 
    - il dispositivo **attuatore** è registrato sul broker con il ruolo di **subscriber** perchè è interessato a ricevere, su un canale di input, eventuali comandi di attuazione (motori, cancelli). 
-  su un **topic di feedback (stato)** (dal dispositivo terminale, verso il broker), utile al server applicativo per ricevere la conferma dell'avvenuto cambio di stato dell'attuatore ma anche utile all'utente per conoscere il nuovo stato:
    - il dispositivo **attuatore** è registrato sul broker con il ruolo di **publisher** perchè intende adoperare questo canale di output per **inviare il feedback** con il proprio stato ad un **display** associato al sensore di comando.
    - il dispositivo **sensore**, ma meglio dire il dispositivo **display** associato al dispositivo sensore (un led o uno schermo), è registrato sul broker con il ruolo di **subscriber** perchè è interessato a ricevere, su un canale di input, eventuali  **feedback** sullo stato dell'attuatore per **mostrarli** all'utente. In questo caso è demandato all'utente, e non al protocollo, **decidere** se e quante volte ripetere il comando, nel caso che lo stato del dispositivo non sia ancora quello voluto.
-  su un **topic di configurazione** dove può pubblicare solamente il sever applicativo mentre tutti gli altri dispositivi IoT sono dei subscriber:
    - sia i dispositivi **sensori** che i dispositivi **attuatori** si registrano sul broker con il ruolo di **subscriber** perchè intendono adoperare questo canale di **input** per ricevere **comandi di configurazione** quali, per esempio, attivazione/disattivazione, frequenza di una misura, durata dello stand by, aggiornamenti del firmware via wirelesss (modo OTA), ecc.
    - il **server applicativo** è responsabile della definizione delle impostazioni di configurazione e decide **quali** mandare e a **chi**.

**In realtà**, il topic di configurazione, pur essendo teoricamente appropriato, potrebbe anche essere incorporato nel topic di comando, magari prevedendo un livello più alto di autorizzazione rispetto ai comandi relativi alle funzioni ordinarie.

Il **canale applicativo** su cui vengono inviati i messaggi sono quindi i **topic**. Su un certo **topic** il dispositivo con il ruolo di **output** agisce come un **publisher**, mentre quello con il ruolo di **input** agisce come un **subscriber**.

l'**ID MQTT** è un identificativo che permette di individuare un dispositivo ma non è un indirizzo di livello 3, non individua la macchina host in base al suo IP, piuttosto è un indirizzo di livello applicativo noto solo ad un server centrale, cioè il broker. Un dispositivo IoT non è tenuto a conoscere l'IP di tutti gli altri dispositivi ma solamente quello del broker. Il broker soltanto sa gli indirizzi di tutti i dispositivi, conoscenza che acquisisce durante la fase di **connessione** di un client al broker, momento in cui avviene anche il recupero del'**socket remoto** del client.

Il **broker**, dal canto suo, **associa** ogni **topic** con tutti gli **ID** che sono registrati presso di esso come **subscriber**. Questa associazione è utilizzata per smistare tutti i messaggi che arrivano con un certo topic verso tutti gli ID che ad esso sono associati. Il topic diventa così un **indirizzo di gruppo**. La particolarità di questo indirizzo è che è **gerarchico** secondo una struttura ad **albero**, cioè gruppi di dispositivi possono essere suddivisi in **sottogruppi** più piccoli estendendo il nome del path con un **ulteriore prefisso**, diverso per ciascun sottogruppo. L'operazione può essere ripetuta ulteriormente in maniera **ricorsiva**.

**Ad esempio**, posso individuare le lampade della casa con il path ```luci``` e accenderle e spegnerle tutte insieme, ma posso sezionarle ulteriormente con il path ```luci/soggiorno``` con il quale accendere o spegnere solo quelle del soggiorno oppure con il path ```luci/soggiorno/piantane``` con il quale fare la stessa cosa ma solo con le piantane del soggiorno.

### **Gestione dei topic di comando**

Potremmo a questo punto inserire il comando delle luci nel topic più generale delle misure ed attuazioni che chiameremo ```comandi``` e registrare i pulsanti del soggiorno al topic ```luci/soggiorno/comandi``` come pubblisher, mentre potremmo registrare le attuazioni delle lampade allo stesso topic come subscriber. Il comando potrebbe essere il JSON  ```{"toggle":"true"}```, per cui alla fine tutto intero il path diventerebbe ```luci/soggiorno/comandi/{"toggle":"true"}```. Se volessimo selezionare un solo dispositivo sono possibili due strade alternative:
- inserire il **prefisso mqtt** del dispositivo direttamente **nel path** ```luci/soggiorno/comandi/mydevice1-98F4ABF298AD/{"toggle":"true"}```
- inserire un **id** del dispositivo **nel JSON** ```luci/soggiorno/comandi/{"deviceid":"01", "toggle":"true"}```, dove con ```01``` ci indica un indirizzo univoco solamente all'interno del sottogruppo ```luci/soggiorno```. Con questa soluzione il dispositivo deve saper gestire un secondo livello di indirizzi indipendente dal meccanismo del path dei topic.

### **Gestione dei topic di stato**

Questo canale viene utilizzato per inviare lo **stato** di un dispositivo a tutti coloro che ne sono interessati. L'interesse potrebbe nascere per più motivi:
- **Conferma dell'avvenuta attuazione**, una volta che l'attuatore ha ricevuto un **comando** (ad esempio "on":"true"), questo potrebbe essere tenuto a **notificare** (in modalità PUSH), al **display** associato al sensore (o al **server di processo**) il proprio **stato attuale**, in modo che l'**utente** (o il server di processo) possa verificare l'effettiva **efficacia** dell'ultimo comando di attuazione.
- **Richiesta del server di processo**. Il **server di processo** potrebbe richiedere (in modalità PULL) lo **stato** degli attuatori per **aggiornare un pannello generale** di comando o eseguire delle statistiche o per recuperare gli input di un algoritmo che deve eseguire.
- **Sincronizzazione PULL** di un pannello di controllo. Un **quadro di controllo web** potrebbe richiedere (in modalità PULL) lo **stato degli attuatori**:
    -  una **sola volta**, all'inizio, quando la pagina è stata **caricata/ricaricata** dall'utente
    -  **periodicamente**, per essere certi di avere sempre lo **stato più aggiornato** anche a fronte di una eventuale **disconnessione** di rete che abbia impedito la registrazione dell'ultimo feedback da parte dell'attuatore.
- **sincronizzazione PUSH**. Lo stesso attuatore potrebbe **periodicamente** inviare (in modalità PUSH) il proprio stato a tutti coloro che ne sono interessati (server di processo o tutti i display web che lo comandano). 

Un esempio di **canale MQTT di stato** potrebbe essere: 
- nel caso di **identificazione univoca** del dispositivo via  **path MQTT**: ```luci/soggiorno/stato/mydevice1-98F4ABF298AD/{"state":"on"}```
- nel caso di **identificazione univoca** del dispositivo nel **payload JSON**: ```luci/soggiorno/stato/{"deviceid":"01", "state":"on"}```

### **Gestione dei topic di configurazione**

Questo canale viene utilizzato per inviare **comandi di configurazione** al dispositivo da parte del server di processo. L'interesse potrebbe nascere per più motivi:
- effettuare un aggiornamento del FW di bordo via wireless.
- impostare qualche caratteristica nella definizione delle sue funzioni come, ad esempio, comportarsi come un apricancello o come comando per luci.
- impostare la frequenza di una misura, o l'intervallo di scatto di un allarme, ecc.
- cambiare la sintassi dei JSON di payload o quella di un path MQTT

Un esempio di **canale MQTT di stato** per, ad esempio, impostare il periodo di pubblicazione automatica dello stato potrebbe essere: 
- nel caso di **identificazione univoca** del dispositivo via  **path MQTT**: ```luci/soggiorno/config/mydevice1-98F4ABF298AD/{"stateperiod":"3000"}```
- nel caso di **identificazione univoca** del dispositivo nel **payload JSON**: ```luci/soggiorno/config/{"deviceid":"01", "stateperiod":"3000"}```
  
## **Gestione bridge broker MQTT**

Serve a realizzare un ponte tra: 
- un broker MQTT locale al network server che colleziona tutti i messaggi del modem LoraWAN
- il broker in uso nella rete di distribuzione che è interessato solo ad un sottoinsieme dei topic del network server (vengono filtrati i messaggi di servizio)
- #, # è invece l’impostazione per un bridge privo di filtraggi

<img src="img/brokerBridge.png" alt="alt text" width="600">

## **Gestione firewall**

Serve a proteggere l’accesso alla rete di distribuzione che, di base, è IP con indirizzi pubblici.

Può essere impostato per far passare tutto se la rete di distribuzione è sicura come accade, ad esempio, nel caso di una LAN aziendale.

<img src="img/lorafirewall.png" alt="alt text" width="600">


## **Classi di dispositivi**

  La specifica LoRaWAN definisce tre classi di dispositivi:

- **A(ll)** Dispositivi alimentati a batteria. Ogni dispositivo effettua il collegamento in uplink al gateway ed è seguito da due brevi finestre di ricezione del downlink.
- **B(eacon)** Come la classe A ma questi dispositivi aprono anche finestre di ricezione aggiuntive a orari programmati.
- **C(continuo)** Uguale ad A ma questi dispositivi sono in ascolto continuo. Pertanto questi dispositivi consumano più energia e sono spesso alimentati dalla rete elettrica.

### **Classe A**

In qualsiasi momento un nodo terminale può trasmettere un segnale. Dopo questa trasmissione uplink (tx) il nodo finale ascolterà una risposta dal gateway.

Il nodo terminale apre due slot di ricezione in t1 e t2 secondi dopo una trasmissione uplink. Il gateway può rispondere all'interno del primo slot di ricezione o del secondo slot di ricezione, ma non in entrambi. I dispositivi di classe B e C devono supportare anche la funzionalità di classe A.


  <img src="img/classAlora.png" alt="alt text" width="600">

### **Classe B**

Oltre agli slot di ricezione di Classe A, i dispositivi di classe B aprono slot di ricezione aggiuntivi a orari programmati.

Il nodo terminale riceve un beacon sincronizzato col tempo dal gateway, consentendo al gateway di sapere quando il nodo è in ascolto. Un dispositivo di classe B non supporta la funzionalità del dispositivo C.

Il beacon viene inviato ogni 128 secondi a partire dalle 00:00:00 Coordinated Universal Time 13 (UTC), 1 gennaio 1970 più NwkID più TBeaconDelay. 

TBeaconDelay è un ritardo specifico della rete scelto nell'intervallo [0:50] ms. TBeaconDelay può variare da una rete all'altra ed è pensato per consentire un leggero ritardo di trasmissione dei gateway. TBeaconDelay deve essere lo stesso per tutti i gateway di una determinata rete. Tutti gli slot di ping dei dispositivi terminali utilizzano il tempo di trasmissione del beacon come riferimento temporale, pertanto il server di rete deve tenere in considerazione questo tempo quando pianifica i downlink di classe B.

  <img src="img/classBlora.png" alt="alt text" width="600">

### **Classe C**

Oltre agli slot di ricezione di Classe A, un dispositivo di Classe C ascolterà continuamente le risposte dal gateway. Un dispositivo di classe C non supporta la funzionalità del dispositivo B.

Non c'è un messaggio specifico per un nodo per dire al server che è un nodo di Classe C. Spetta all'applicazione lato server sapere che 13 gestisce i nodi di Classe C in base al contratto passato durante la procedura di join.

  <img src="img/classClora.png" alt="alt text" width="600">

**Uplink confermato**

Potrebbe essere il caso di un pulsante che comanda l'accensione di un motore, oppure un pulsante di allarme, o anche un apri porta.

<img src="img/loraAck.png" alt="alt text" width="800">

1. Il dispositivo finale trasmette innanzitutto un frame di dati confermato contenente il payload Data0 in un istante arbitrario e su un canale arbitrario. Il frame counter Cu è semplicemente derivato aggiungendo 1 al precedente frame counter di uplink.
2. La rete riceve il frame, genera un frame in downlink con il bit ACK impostato, e lo invia esattamente RECEIVE_DELAY1 secondi dopo utilizzando la prima finestra di ricezione del dispositivo finale. Questo frame di downlink utilizza la stessa velocità di dati e lo stesso canale dell'uplink di Dati0. Anche il contatore del frame downlink Cd è derivato aggiungendo 1 all'ultimo downlink verso quello specifico dispositivo finale. Se non c'è alcun payload di downlink in sospeso, la rete genererà un frame senza carico utile.
3. In questo esempio il frame di ACK non viene ricevuto. Se un nodo finale non riceve un frame di ACK in una delle due finestre di ricezione immediatamente dopo la trasmissione, l'uplink può inviare nuovamente lo stesso frame con lo stesso payload e contatore di frame entro ACK_TIMEOUT secondi dopo la seconda finestra di ricezione.
4. Questo nuovo invio deve essere effettuato su un altro canale e deve rispettare la limitazione sul duty cycle come qualsiasi altra normale trasmissione.
5. Se questa volta il dispositivo terminale riceve in downlink l'ACK durante la sua prima finestra di ricezione, appena il frame ACK viene demodulato, il dispositivo terminale è libero di trasmettere un nuovo frame su un nuovo canale.

**Downlink confermato**

Potrebbe essere il caso di una attuazione che è bene che sia confermata, quale un motore, oppure la riuscita di una configurazione, ecc.

<img src="img/loraAckD.png" alt="alt text" width="800">

1. Lo scambio di frame viene avviato dal dispositivo terminale che trasmette un payload dell'applicazione "non confermato" o qualsiasi altro frame sul canale A.
2. La rete utilizza la finestra di ricezione downlink per trasmettere un frame di dati "confermato" verso il dispositivo finale sullo stesso canale A
3. Alla ricezione di questo frame di dati che richiede una conferma, il dispositivo finale trasmette un frame con il bit ACK impostato a sua discrezione. Questo frame potrebbe anche contenere dati (piggybacking) o comandi MAC come carico utile. Questo uplink ACK viene trattato come qualsiasi uplink standard e come tale viene trasmesso su un canale casuale che potrebbe anche essere diverso dal canale A.
   
**Piggy backing**

Per consentire ai dispositivi terminali di essere il più semplici possibile e di mantenere il minor numero di stati possibile, è possibile trasmettere un messaggio di ack puro cioè senza dati possibilmente subito dopo la ricezione di un messaggio di dati che richiede una conferma. In alternativa, il dispositivo finale può dilazionare la trasmissione di un ack per collegarlo al successivo messaggio di dati (tecnica del piggy backing).

### **Formato dei messaggi**

<img src="img/loramsg.png" alt="alt text" width="600">


**Sitografia**:
- https://lora.readthedocs.io/en/latest/#lorawan-device-classes
- https://lora-alliance.org/wp-content/uploads/2020/11/2015_-_lorawan_specification_1r0_611_1.pdf
- https://medium.com/@prajzler/lorawan-confirmations-and-acks-ba784a56d2d7
- chatGPT per struct e JSON di sensori e attuatori



>[Torna a reti di sensori](sensornetworkshort.md#classificazione-delle-tecnologie-wsn-in-base-a-velocità-e-copertura)
