>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)

Di seguito è riportata l'architettura generale di una rete Lorawan. Essa è composta, a **livello fisico**, essenzialmente di una **rete di accesso** ai sensori e da una **rete di distribuzione** che fa da collante di ciascuna rete di sensori.

<img src="img/lorawanArchitecture.png" alt="alt text" width="800">

La rete di sensori fisica è a stella dove il centro stella è il gateway. Un sensore può anche essere associato a più gateway ed inviare dati a tutti i gateway a cui esso è associato. I dati normalmente arrivano ad un certo dispositivo attraverso un solo gateway.

I gateway utilizzano la rete internet (o una LAN) per realizzare un collegamento diretto **virtuale** con il network server, per cui, in definitiva, la topologia risultante è:
- **fisicamente** quella di più **reti di accesso** a stella tenute insieme da una **rete di distribuzione** qualsiasi purchè sia di tipo TCP/IP (LAN o Internet).
- **logicamente** quella di una stella di reti a stella. Il **centrostella** di livello gerarchico più alto è il **network server** ed aggrega solo gateways, gli altri centrostella sono realizzati dai **gateway** che aggregano solamente **dispositivi IoT**.

  <img src="img/lorawanLogicArchitecture.png" alt="alt text" width="800">

  La specifica LoRaWAN definisce tre classi di dispositivi:

- **A(ll)** Dispositivi alimentati a batteria. Ogni dispositivo effettua il collegamento in uplink al gateway ed è seguito da due brevi finestre di ricezione del downlink.
- **B(eacon)** Come la classe A ma questi dispositivi aprono anche finestre di ricezione aggiuntive a orari programmati.
- **C(continuo)** Uguale ad A ma questi dispositivi sono in ascolto continuo. Pertanto questi dispositivi consumano più energia e sono spesso alimentati dalla rete elettrica.

### **Classe A**

In qualsiasi momento un nodo terminale può trasmettere un segnale. Dopo questa trasmissione uplink (tx) il nodo finale ascolterà una risposta dal gateway.

Il nodo finale apre due slot di ricezione in t1 e t2 secondi dopo una trasmissione uplink. Il gateway può rispondere all'interno del primo slot di ricezione o del secondo slot di ricezione, ma non in entrambi. I dispositivi di classe B e C devono supportare anche la funzionalità di classe A.

  <img src="img/classAlora.png" alt="alt text" width="800">

### **Classe B**

Oltre agli slot di ricezione di Classe A, i dispositivi di classe B aprono slot di ricezione aggiuntivi a orari programmati.

Il nodo terminale riceve un beacon sincronizzato nel tempo dal gateway, consentendo al gateway di sapere quando il nodo è in ascolto. Un dispositivo di classe B non supporta la funzionalità del dispositivo C.

  <img src="img/classBlora.png" alt="alt text" width="800">

### **Classe C**

Oltre agli slot di ricezione di Classe A, un dispositivo di Classe C ascolterà continuamente le risposte dal gateway. Un dispositivo di classe C non supporta la funzionalità del dispositivo B.

  <img src="img/classClora.png" alt="alt text" width="800">



**Sitografia**:
- https://lora.readthedocs.io/en/latest/#lorawan-device-classes
- https://lora-alliance.org/wp-content/uploads/2020/11/2015_-_lorawan_specification_1r0_611_1.pdf



>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)
