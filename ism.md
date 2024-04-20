>[Torna a reti di sensori](sensornetworkshort.md#interfaccia-radio)

## **Banda ISM**

Le bande libere sono le frequenze di uso libero, non tutelate, che non richiedono concessioni per il loro impiego. Sono spesso indicate come ISM (Industrial, Scientific and Medical)[Nota 1].

In realtà ISM è un sottogruppo di tutte le frequenze disponibili.
L’uso di tali bande è regolamentato in modo da consentirne l’impiego condiviso ed evitare che un utente o un servizio possa monopolizzare la risorsa.

In tabella un elenco parziale con le principali limitazioni:

<img src="ismband.png" alt="alt text" width="600">

L'ente regolatore, in Italia il Ministero dello Sviluppo Economico (MISE), fissa alcuni criteri per stabilire il diritto di accesso quali la distinzione nell’uso privato e non o la destinazione di frequenze ad uso contemporaneo a differenti servizi aventi differenti finalità. L’uso delle bande libere è destinato ad apparati denominati SRD (Short Range Devices). Curiosamente la banda 862-876 MHz (sub-GHz) molto utilizzata da dispositivi SRD, è gestita dal Ministero della Difesa e non dal MISE.  


I **criteri** riguardano anche alcuni dettagli fisici e tecnici:
- Il **Duty Cycle** fa riferimento al rapporto fra il tempo di trasmissione e il tempo di ricezione più il tempo di ricezione. Ad esempio un duty cycle dell'1% impone che a fronte della trasmissione di un pacchetto dati di un secondo, l’apparato non possa trasmettere per 99 secondi. Questo parametro, di fatto, limita a trasmissioni brevi, non frequenti ed esclude gli streaming audio e video. La durata predefinita del periodo di osservazione è di 1 ora, se non diversamente specificato per la banda di frequenza specifica. Attualmente tutte le bande di frequenza utilizzano il periodo di osservazione predefinito di 1 ora.
  
  <img src="img/duty-cycle-multi-band.png" alt="alt text" width="5">
  In questo caso, la stessa sorgente trasmette su due canali in due sottobande diverse con una occupazione di due unità su 10 (cioè 20 su 100) per ciascun ciascun canale. Il duty cycle totale della sorgente, in questo caso, è del 60%.
  
- **Canali e bande**. I **canali** sono intervalli di frequenza adoperati per allocare nel dominio della frequenza la trasmissione di una certa sorgente. Un canale è caratterizzato da una frequenza centrale e da una sua ampiezza (rappresentabile anche come escursione dalla frequenza centrale. Le **bande** sono intervalli di frequenza all'interno delle quali sono allocabili un certo numero di canali. Organizzare i canali in **bande** può servire per isolare **gruppi di canali** che possono essere adoperati per gli **stessi servizi**. Organizzare i canali in **sottobande** può servire per isolare **gruppi di canali** a cui applicare le **stesse regole**. Ad esempio, le sottobande LoraWAN in Europa sono :  

<img src="img/LoRaWAN_Sub-bands.webp" alt="alt text" width="600">

    - K. (863 MHz - 865 MHz): 0.1%, 25 mW ERP       (uplink)
    - L. (865 MHz - 868 MHz): 1%, 25 mW ERP         (uplink)
    - M. (868 MHz - 868.6 MHz): 1%, 25 mW ERP       (uplink)
    - N. (868.7 MHz - 869.2 MHz): 0.1%, 25 mW ERP   (uplink)
    - P. (869.4 MHz - 869.65 MHz): 10%, 500 mW ERP  (downlink)
    - Q. (869.7 MHz - 870 MHz): 1%, 25 mW ERP       (uplink)
    
&nbsp;&nbsp; Di queste, 5 (numerate da B0 a B5) sono utilizzabili dai nodi di terminali e permettono, mediante tecnica FDM che parallelizzi il flusso di un unica sorgente su 5 canali contemporaneamente, un duty cycle complessivo del 3.2%. Il **gateway** LoRaWAN utilizza un'architettura a basso costo e basso consumo energetico che consente il posizionamento di una **coppia di radio** con larghezza di banda di **1 MHz** ovunque nella banda ISM dell'UE. Gli **otto canali** di ricezione LoRa sono posizionati all'interno di queste due bande da 1 MHz. Quindi una applicazione finale, avrà la possibilità di poter spalmare il duty cycle su **due sole sottofasce**. Nella maggior parte delle reti distribuite queste vantaggiosamente si concentreranno sulle allocazioni di uplink dell'1%. Ciò significa che ci si può ragionevolmente aspettare il **2% di duty cycle** aggregato disponibile per una **stessa sorgente**.

&nbsp;&nbsp;   - Uplink:
      -  868.1 - SF7BW125 to SF12BW125
      -  868.3 - SF7BW125 to SF12BW125 and SF7BW250
      -  868.5 - SF7BW125 to SF12BW125
      -  867.1 - SF7BW125 to SF12BW125
      -  867.3 - SF7BW125 to SF12BW125
      -  867.5 - SF7BW125 to SF12BW125
      -  867.7 - SF7BW125 to SF12BW125
      -  867.9 - SF7BW125 to SF12BW125
      -  868.8 - FSK
&nbsp;&nbsp;  -  Downlink:
      -  Uplink channels 1-9 (RX1)
      -  869.525 - SF9BW125 (RX2)


- **Potenza disponibile massima**. La potenza in Watt. In genere dell’ordine dei millesimi di watt (mW) è spesso espressa in dBm.
In taluni casi, in relazione alla larghezza di banda, si fa riferimento alla densità di potenza, ovvero alla potenza, in milliwatt per MHz o per KHz. E’ il caso degli apparati WLAN E HiperLAN.
- **EIRP** e **ERP**. La potenza è generalmente riferita al **segnale irradiato** sotto forma di misura ERP o di misura EIRP. L’antenna è un componente passivo, ma possiede pur sempre una sorta di guadagno. Il guadagno quantifica la capacità dell’antenna di concentrare l’energia irradiata (o ricevuta) in una determinata direzione. Un'antenna si dice **isotropa** quando emette la stessa potenza in tutte le direzioni in quanto non possiede direzioni di emissione privilegiate in cui emette più energia rispetto ad altre. Ciò equivale a dire che, in una sfera avente per centro l'antenna, la densità di potenza è la stessa in ogni punto sulla sua superficie. Ma si tratta solo di una approssimazione ideale. Le antenne reali, a parità di distanza, distribuiscono la loro energia in maniera non uniforme al variare della direzione di un punto nello spazio. Le antenne cosidette **direttive** sono progettate proprio per introdurre di proposito un guadagno ulteriore sulla potenza fornita dal trasmettitore detto guadagno di antenna che è dovuto proprio alla capacità di concentrare la potenza irradiata in una direzione privilegiata. La potenza **EIRP** di una antenna direttiva è invece la potenza con cui dovrebbe essere alimentata un'antenna isotropica per irradiare la stessa potenza che viene emessa dall'antenna direttiva nella sua direzione di massimo irraggiamento.  La potenza **EIRP** è la somma della **potenza erogata** dal trasmettittore più il **guadagno di antenna** (al netto delle perdite sul cavo sempre presenti). Un'altra grandezza considerata nella pratica è la potenza **ERP**, analoga alla precedente ma riferita alla potenza emessa da un dipolo orientato normalmente alla direzione di massima intensità dell'antenna direttiva. Stavolta si valuta il guadagno dell'antenna sotto test rispetto al guadagno di un dipolo standard a mezza lunghezza d'onda. In questo caso il valore ERP dBd è leggermente più basso rispetto al valore di un'antenna istropa EIRP dBi (dBi = dBd + 2,15dB) in quanto il dipolo guadagna 2,15dB in più rispetto al radiale isotropico, anche se soltanto lungo la direzione di massima irradiazione. Un limite comune stabilito dalle regolamentazione è di fissare l'ERP proprio esattamente uguale alla massima potenza disponibile ammessa sul morsetto di antenna, ciò implica che l'antenna non debba guadagnare per nulla rispetto ad un dipolo a mezz'onda o, equivalentemente, che guadagni soltanto 2.15 dB rispetto ad una antenna isotropa alimentata con la stessa potenza disponibile.
- Altri limiti sono le **modalità di accesso** al canale radio. Due **schemi di riferimento** sono **LBT** e **AFA**. Ascolto del canale prima di parlare (LBT) e Agilità di frequenza adattiva (AFA). 
LBT (listen befor Talk) è una modalità di accesso nella quale un dispositivo che deve trasmettere non occupa subito il canale ma, prima di parlare, deve ascoltare se il mezzo è già in uso attivando una funzione di CCA (Clear Channel Assessment). Se il supporto è in uso, allora la successiva trasmissione deve essere spostata o nel tempo o nella frequenza:
    - o il dispositivo attende un intervallo di backoff casuale prima di ritentare il CCA sullo stesso canale
    - o il dispositivo esegue immediatamente un nuovo CCA ma, stavolta, su un altro canale. Quest'ultima tecnica si chiama AFA. 


<img src="img/13638_2019_1502_Fig3_HTML.png" alt="alt text" width="600">


Quando questi 2 aspetti vengono implementati, il duty sycle viene ridotto a 100 s di tempo di trasmissione cumulativo all'ora per ogni possibile intervallo di 200 kHz, che corrisponde a un rapporto del ciclo di lavoro del 2,7%.

Il controllo CCA deve avere una durata minima di 160 μs. Dopo questo controllo il dispositivo deve attendere un tempo morto massimo di 5 ms prima di poter iniziare la trasmissione. La trasmissione stessa ha una durata massima di 1 s o 4 s a seconda del tipo di trasmissione. Si definiscono poi essere una singola trasmissione entrambe le situazioni seguenti: una sequenza continua di bit, oppure un burst di sequenze diverse purchè separate da intervalli inferiori a 5 ms. Dopo una trasmissione, una sorgente deve attendere, prima di trasmettere nuovamente sullo stesso canale, un minimo di 100 ms senza CCA. È comunque ancora consentito utilizzare tale intervallo per il successivo controllo CCA o per trasmettere su altri canali.

### **tecnologia LoraWAN: schema riassuntivo**

<img src="img/eulorapecs.png" alt="alt text" width="400">

I **parametri** e le **limitazioni** discusse sono tecniche volte a ridurre **mutui disturbi** fra i vari servizi dei vari utenti e la **monopolizzazione** di un canale da parte di un singolo utente.

Schemi riassuntivi che ilustrano la situazione normativa per le varie bande ISM:

<img src="433.png" alt="alt text" width="800">
<img src="868.png" alt="alt text" width="800">
<img src="2400.png" alt="alt text" width="800">

Sitografia:
- https://blog.semtech.com/certifying-an-end-device-for-lorawan-european-sub-bands
- https://www.thethingsnetwork.org/docs/lorawan/regional-limitations-of-rf-use/
- https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/eu868/
- https://www.thethingsnetwork.org/docs/lorawan/duty-cycle/
- https://docs.heltec.org/general/lorawan_frequency_plans.html
- https://lora-developers.semtech.com/documentation/tech-papers-and-guides/lora-and-lorawan/
- https://jwcn-eurasipjournals.springeropen.com/articles/10.1186/s13638-019-1502-5
  
>[Torna a reti di sensori](sensornetworkshort.md#interfaccia-radio)



