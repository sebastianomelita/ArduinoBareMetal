>[Torna a reti di sensori](sensornetworkshort.md#servizi-di-accesso-radio-per-WSN)

## **Tecnologie di accesso al canale radio**

### **DSSS**

In realtà DSSS più che una tecnica di  **multiplazione** per l' **accesso al canale radio** è una vera e propria tecnica di **modulazione numerica** complementare alle tradizionali ASK, PSK, FSK, ecc.).

Nel **CDM** la sorgente viene:
- **traslata**: cioè portata alla frequenza centrale della banda di trasmissione
- **espansa**: cioè trasformata da segnale a banda stretta a segnale che occupa tutta la banda di lavoro detto segnale a banda espansa.  

Nei sistemi DSSS (Direct Sequence Spread Spectrum), il **fattore di spreading** è la lunghezza della sequenza di spreading che si riflette nel numero di chip per ciascun simbolo di modulazione.

L’**espansione in frequenza** si ottiene trasformando artificialmente un segnale **lentamente variabile** nel tempo in uno **rapidamente variabile** nel tempo.
Ad ogni sorgente è associato un **codice di espansione** da usare in trasmissione. Lo stesso codice, **usato in ricezione**, riporta a **banda stretta** solo lei lasciando espanse tutte le altre sorgenti.

<img src="spread1.png" alt="alt text" width="1000">

La **proprietà** che viene **divisa** fisicamente è la **potenza** di trasmissione. La proprietà che viene suddivisa logicamente è un insieme di **codici ortogonali**
**In TX** la sorgente viene espansa: cioè trasformata da segnale a banda stretta a segnale che occupa tutta la banda di lavoro, detto segnale a spettro espanso.  

L’**espansione** in frequenza di un segnale:

- mantiene **immutata l’energia** complessiva del segnale originale, cioè l’area sottesa al segnale nel diagramma potenza – frequenza rimane la stessa (si allarga la base del segnale ma, nel contempo, si diminuisce l’altezza).
- si **ottiene nel dominio del tempo** moltiplicando il segnale di partenza per un altro segnale di frequenza molto maggiore detto **chirp (o spreading code)**. L’**energia** viene così **diffusa** su **tutte** le frequenze della banda ma con una **ampiezza** molto bassa. La **moltiplicazione** trasforma un segnale lentamente variabile in uno rapidamente variabile **sostituendo** il **bit originale** con una **sequenza di bit** veloci detti **chirp**.

<img src="spread2.png" alt="alt text" width="1000">

Nel CDM le **sorgenti** occupano e **condividono** tutta la **banda** di frequenza nello **stesso tempo** e in **tutto lo spazio** sovrapponendo l’energia di una con quella dell’altra (interferenza completa). Il **ricevitore** riceve i segnali di più sorgenti sotto **forma espansa** e sovrapposti tra loro (segnali interferenti) che sono, in pratica, indistinguibili.

**In ricezione** la moltiplicazione nel tempo di un segnale ricevuto per lo stesso chirp (o spreading code) con cui era stato espanso in trasmissione, riporta il segnale ricevuto a banda stretta, permettendone la decodifica dei bit. La moltiplicazione con quel codice avviene anche con gli altri segnali ricevuti ma, essendo questo diverso dal loro spreading code, li lascia a spettro espanso. 

<img src="img/DSSS_carrier.png" alt="alt text" width="600">

Aumentando sufficientemente il fattore di spreading è possibile portare la sensibilità sotto la soglia di rumore di fondo dell'universo rendendo, di fatto, le comunicazioni inintercettabili. Inoltre si riesce a far lavorare il sistema in ambienti molto disturbati, persino con rapporti segnale/rumore negativi.

E' una tecnica di accesso usata dal protocollo **Zigbee** e **6LowPan** (IEEE 802.15.4).

### **FHSS**

Anche FHSS potrebbe essere vista come una tecnica di **modulazione numerica**.

Alcune **tecnologie radio** realizzano un **accesso multiplo** al canale radio, **apparentemente** allocando **molti canali** nello **stesso istante**. In realtà la tecnica  alloca un canale alla volta ma **saltando** da uno all'altro con una **velocità** che **alla sorgente** appare **istantanea**. Questo fenomeno ha l'effetto di sparpagliare (**spreading**) l'energia di un canale su **tutta la banda** disponibile. Si tratta di una **variante di CDM** ottenuta per altra via. Infatti, la **sequenza di saltellamento** è diversa per ogni comunicazione ed è associata ad un **SSID**. Le **sequenze** sono **ortogonali** e quindi permettono la ricostruzione in ricezione della **comunicazione originale**. Anche una eventuale **multiplazione TDM** effettuata dentro il canale FDM non si accorgerebbe del saltellamento e procederebbe come al solito, assegnando **diversi slot** temporali a **comunicazioni diverse**. Un messaggio potrebbe **partire** in un canale, passare ad altri 7 e completarsi solo nell' ottavo canale, ovvero l'ultimo visitato. La tecnica si chiama **Frequency hopping** o anche detta **FHSS**. La **distribuzione** della comunicazione su una **gamma ampia** di frequenze rende questo tipo di modulazione **meno sensibile** alle **interferenze**.

Nei sistemi FHSS (Frequency Hopping Spread Spectrum), il **fattore di spreading** è il numero di frequenze portanti su cui salta un simbolo di modulazione.

<img src="fhss.png" alt="alt text" width="600">

E' una tecnica di accesso usata dal protocollo **Bluetooth LBE**

### **LoraWAN**
Uno degli svantaggi di un sistema DSSS è il fatto che richiede un orologio di riferimento altamente accurato (e costoso). La tecnologia LoRa Chirp Spread Spectrum (CSS) di Semtech offre un'alternativa DSSS a basso costo e a basso consumo, ma robusta, che non richiede un clock di riferimento altamente accurato.

La tecnologia di livello fisico chiamata LoRa utilizza una forma **custom** (e non resa di dominio pubblico) di **spettro espanso** per modulare le comunicazioni nella banda ISM sub-GHz. LoRa utilizza bande **ISM** senza licenza. Per fornire la comunicazione viene utilizzata una modulazione **CSS (Chirp Spread Spectrum)**, che **espande** un segnale a **banda stretta** su un canale a larghezza di banda più ampia.

FSK            |  CSS
:-------------------------:|:-------------------------:
![](img/FSKProcess.png)  |  ![](img/loraChirp.png)

Un **chirp** è un segnale che, con continuità, incrementa o decrementa la sua frequenza. Quando l'incremento è positivo si parla di **upchirp**, quando è negativo di **downchirp**.

Le modulazioni (digitali o analogiche) possono essere indagate separatamente nel tempo e nella frequenza o tramite particolari diagrammi che mostrano come la **frequenza varia** nel tempo, detti **spettrogrammi**. Da destra a sinistra ci stanno gli incrementi positivi di frequenza, dall'alto in basso quelli positivi del tempo. In uno spettrogramma:
- una **variazione discreta** tra due frequenza fisse che codificano i bit 1 e 0 (FSK) appare come una successione nel tempo di singole linee verticali che rappresentano due frequenze costanti diverse (dette armoniche).
- una **variazione continua** di frequenza detta scivolamento o chirp appare come un **segmento inclinato** con pendenza crescente nel tempo nel caso di un upchirp o decrescente nel tempo nel caso di un downchirp.
<img src="img/chirpdivisions.webp" alt="alt text" width="600">

Definizioni:
- **Larghezza di banda**: larghezza dello spettro occupato dal chirp.
- **Fattore di spreading (diffusione)**: numero di bit codificato per simbolo (stato RF)
- **Chirp rate**: derivata prima della frequenza del chirp

Un segnale di **chirp** può essere suddiviso in 2^SF valori (128) che **codificano**, nella frequenza, il simbolo da trasmettere. Ad **un simbolo** corrisponde, nel tempo, una certa **sequenza lunga SF** di bit da trasmettere che può essere ricostruita **in ricezione** misurando la **lunghezza in frequenza** del chirp nello sprettogramma (mediante FFT) dove ha un valore compreso tra [0, 2^SF]. 

La **pendenza** (slope) del chirp nello spettrogramma è sempre tale da ottenere uno scivolamento di B Hertz nel tempo in cui si raccolgono SF bit, cioè un un simbolo, e questo per tutti i valori di SF. Per effetto di ciò, all'aumentare di SF nel tempo, i bit verranno diffusi di 2^SF/SF nella frequenza.

L'ampiezza di banda B del canale rappresenta il chip rate. Il reciproco del chip rate rappresenta la lunghezza del chip nel tempo.

Il numero di simboli al secondo trasmessi rappresenta il symbol rate. Il reciproco del symbol rate rappresenta la lunghezza del simbolo nel tempo.

CHIP RATE         |  SYMBOL RATE  |  BIT RATE  | CHIP DURATION        |  SYMBOL DURATION   
:-------------------------:|:-------------------------:|:-------------------------:|:-------------------------:|:-------------------------:
![](img/bw.webp)  |  ![](img/rs.webp)   |   ![](img/rb.webp)  |  ![](img/ts.webp) |  ![](img/tc.webp)

La larghezza di banda BW equivale al **chip rate**, ovvero al numero di segnali elementari al secondo, è la larghezza del canale. Un simbolo di frequenza massima si chiama **chip**. 

Lo **Spreading Factor (SF)** in LoRa è definito come il numero di bit per simbolo e indica il numero di volte che il segnale che viene **allargato in frequenza** rispetto alla larghezza originale in banda stretta. Ad esempio, con SF7, il segnale viene allargato di 2^7/7 volte rispetto alla larghezza di banda originale, un incremento pressochè **esponenziale**.

DR         |  SF
:-------------------------:|:-------------------------:
![](img/SFRate.png)  |  ![](img/SF-band.png)

Il **data rate (DR)** varia da 0 a 5 ed è inversamente proporzionale allo **spreading factor (SF)** che varia da 7 a 12.

Aumentando lo spreading factor, si aumentano le componenti di frequenza dello spettro totale del segnale. In altre parole, l’energia del segnale totale è ora distribuita su una gamma più ampia di frequenze, consentendo al ricevitore di discernere un segnale con un rapporto segnale-rumore (SNR) più basso (cioè peggiore).


<img src="img/chirpedMsg.png" alt="alt text" width="600">

<img src="img/dechirp.png" alt="alt text" width="600">

**Telgram splitting**: questo metodo suddivide un segnale a banda ultrastretta in numerosi sottopacchetti più piccoli, trasmettendoli successivamente come brevi burst radio a varie frequenze e intervalli di tempo. Per ridurre al minimo il potenziale di collisione con altri sottopacchetti, utilizza tempi di trasmissione brevi e pseudo-casualità. Questo approccio migliora sia la robustezza che la scalabilità riducendo al contempo le interferenze

Caratteristiche positive di Lora sono:
- realizzazione di collegamenti con link budget molt elevati
- Resilienza alle interferenze
- Prestazioni a basso consumo
- Resistenza agli effetti multi-percorso
- Resistenza all'effetto Doppler (applicazioni mobili)


### **TSCH**

E' una tecnologia radio che realizza un **accesso al canale singolo**, nel senso che l sorgente invia **per intero** un messaggio **in un canale** per un tempo stabilito da una **schedulazione TDM** costante e preordinata ma, alla **trama successiva** quello **slot** temporale sarà assegnato ad **un'altra frequenza** per tutta la sua durata e così accade per tutti gli altri slot di quella trama. Così, in caso di trasmissione **non riuscita** a causa di interferenze esterne o dissolvenza multi-percorso, la sua **ritrasmissione** avviene su una frequenza diversa, spesso con una migliore probabilità di successo rispetto all'utilizzo della stessa frequenza di prima. Si noti che, identificando una comunicazione con i suoi dispositivi endpoint, allora accade che tutti questi dispositivi, per ogni trama, devono condividere lo stesso slot e lo stesso canale.  La tecnica si chiama **Time Slotted Channel Hopping** o anche detta **TSCH**. Gli hop sono **più lenti** del **segnale modulato** per cui non è considerabile come una tecnica di modulazione del segnale portante. 

TSCH può essere visto come una combinazione dei meccanismi di accesso multiplo a **divisione di tempo** e di accesso multiplo a **divisione di frequenza** poiché utilizza la **diversità** di tempo e frequenza insieme per fornire affidabilità agli strati superiori della rete.

<img src="Time-Slotted-Channel-Hopping-TSCH-slot-channel-matrix-with-a-simple-network-topology.png" alt="alt text" width="600">

A causa della natura **TDM** della comunicazione in una rete TSCH, i nodi devono mantenere una **stretta sincronizzazione**. Si presuppone che tutti i nodi siano dotati di orologi per tenersi sincronizzati tra di loro. Però, perché gli orologi in nodi diversi **derivano** l'uno rispetto all'altro, i nodi adiacenti devono periodicamente **risincronizzarsi**.

Ogni nodo deve sincronizzare periodicamente il proprio orologio di rete con un altro nodo e fornisce anche il tempo di rete ai suoi vicini.
Spetta all'ente che gestisce la pianificazione assegnare una sorgente di sincronizzazione vicina a ciascun nodo detta "time source neighbor". Durante l'impostazione del "time source neighbor", è importante evitare cicli nei percorsi di sincronizzazione, che potrebbero comportare la formazione di gruppi indipendenti di nodi sincronizzati.

E' usata dal protocollo IEEE 802.15.4g/e, livello fisico degli stack **RIIoT**, **OpenWSN** e dalle ultime versioni di **Zigbee** e **6LowPan**.


### **OFDM**

E' una tecnologia in cui i bit di una comunicazione, tramite una conversione seriale-parallelo, vengono inviati contemporaneamente su **più porzioni** di un canale di grande ampiezza (20 Mhz per il WiFi) dette **sottoportanti** (o anche **toni**). Le sottoportanti non solo sono vicine ma si **sovrappongono** in parte di uno spiazzamento stabilito. Lo **spiazzamento calibrato** opportunamente fornisce la proprietà di **ortogonalità** che garantisce alle sottoportanti di **non interferirsi** nonostante le sovrapposizioni reciproche. In pratica una **comunicazione veloce** viene suddivisa in più **comunicazioni lente** che hanno il pregio di poter essere gestite in maniera **meno critica** rispetto ai problemi associati ad un'unica veloce. 

La **distribuzione uniforme** delle sottoportanti lungo tutta la banda fa si che, per effetto di una **interferenza distruttiva** o di un **fading** improvviso (attenuazione) in corrispondenza di una certa sottoportante, si perderanno, o nel caso del fading si amplificheranno, le informazioni trasmesse su quella ma non le rimanenti trasmesse sulle altre sottoportanti. Questo perchè, in ricezione, le sottoportanti vengono demodulate separatamente e solo dopo i bit ricevuti sono ricomposti in un unico flusso. E' il criterio dei **compartimenti stagni** di una nave (le **sottoportanti**) applicato al **canale** di una sorgente (l'intera **nave**). 

All'**interno** del canale OFDM viena fatta una multiplazione **TDM** a contesa (tipicamente CSMA/CA) sia per renderlo **full duplex** sia per consentire l'**accesso multiplo** al canale da parte di più sorgenti, per cui, al variare del tempo, sullo  **stesso canale** verranno allocate **sorgenti diverse**. In ogni caso la trasmissione di **una sorgente** avviene sempre su **tutte** le sottoportanti (**no FDM**).

Le **sorgenti lente** impiegheranno un certo **tempo** per recuperare un numero sufficiente di bit per alimentare adeguatamente tutte le sottoportanti, ne consegue che la loro trasmissione sarà caratterizzata da **brevi impulsi** distanziati da **lunghi ritardi** e saranno pure trasmessi con grande spreco di **potenza**.

Oppure potrebbero esserci molte **trasmissioni brevi** che competono per il canale mediante il TDM a contesa **CSMA/CA**. Questa situazione genera uno scambio di **messaggi di controllo** aventi **dimensione** analoga a quella dei **messaggi dati**. Vuol dire che il canale potrebbe essere **più occupato** dai messaggi necessari per portare avanti il protocollo CSMA /CA che dai messaggi dati (**overhead eccessivo del CSMA/CA**).

<img src="xofdm2.png" alt="alt text" width="1000">

E' adoperato dal **WiFi 5** e dalla telefonia mobile **4G**. Ha la proprietà di trasmettere dati ad **alta velocità** in ambienti con **forti interferenze**. 


### **OFDMA**

Realizza una **parallelizzazione** dell'accesso al canale radio da parte di **più sorgenti** abbinando alla multiplazione nel tempo anche una multiplazione nella frequenza **FDM**. 
Infatti, nell'OFDM può accadere che la multiplazione TDM applicata a sorgenti di velocità molto diverse determini che le sorgenti più lente vengano trasmesse con brevi messaggi che, avendo i bit spalmati su tutte le sottoportanti, sono inviati sul canale radio alla massima velocità da questo consentita. L'effetto è quella di una trasmissione di brevi burst inviati, con forte ritardo, su slot molto lontani nel tempo (il tempo necessario per accumulare il numero di bit sufficiente a riempire uno slot). Se non si volesse aspettare, per diminuire il ritardo, si dovrebbe accettare di trasmettere in ogni slot lasciandolo sempre mezzo vuoto.

L'**analogia del canale** è quella dei **TIR** che in **giorni diversi** portano poco carico quando il **carico complessivo** dei tre giorni avrebbe potuto benissimo essere accomodato su **un unico TIR**, direttamente al primo giorno, **riducendo** i **tempi di attesa**. 
**Servono** per far ciò: 
- la possibilità di inviare, nello **stesso slot** temporale, messaggi di **sorgenti diverse** su **sottoportanti diverse** dello stesso canale (multiplazione FDM), realizzando una parallelizzazione della trasmissione delle sorgenti.
- una **schedulazione efficace** che sappia **riordinare** i messaggi sulle sottoportanti (slot FDM) cercando di rispettare le **richieste di servizio** in termini di **latenza** che erano state **prenotate** per ogni sorgente in fase di **setup** della connessione. 

<img src="ofdma.jpg" alt="alt text" width="1000">

La **principale differenza** tra un sistema **OFDM** e un sistema **OFDMA** (Orthogonal Frequency Division Multiple Access) sta nel fatto che nella OFDM l'utente è allocato nel dominio del **tempo soltanto**. Durante l'utilizzo di un **sistema OFDMA**, invece, l'utente viene allocato sia **in base al tempo** che **in base alla frequenza**. Il sistema in un **certo istante** è in grado di trasmettere o ricevere comunicazioni di **più dispositivi** in **parallelo** (su sottoportanti diverse) diminuendo i **tempi di attesa**. Le **sottoportanti** di un **canale** (256 nello OFDMA di WiFi 6) sono dinamicamente **distribuite** su un certo numero di **sottocanali**. Ogni **sottocanale** è assegnato ad una **sorgente** alla volta. Il **numero dei sottocanali** è **variabile** e dipende da quante sottoportanti vengono assegnate a ciascuno di essi. La **ripartizione viene**, per **ogni slot** temporale, modificata e ottimizzata da uno **scheduler**.

Di seguito sono elencati **tutti i sottocanali allocabili** con il **numero** delle sottoportanti da essi **occupate**. Sono possibili anche **combinazioni di ampiezza diversa**. L'unico **vincolo** è che la **somma complessiva delle sottoportanti** allocate dai vari **sottocanali** in un certo **slot temporale** non superi mai il **numero complessivo** delle sottoportanti dati (242 per il WiFi 6). Dal grafico si evince chiaramente che possono essere allocate, ad esempio, 9 sottocanali da 26 sottoportanti ciascuno. Oppure un sottocanale da 106 sottoportanti più 2 da 52. Se, in un certo slot temporale, si vuole utilizzare il massimo della banda disponibile allora si alloca, per un'unica sorgente, un unico sottocanale da 242 sottoportanti.

<img src="ruofdma.png" alt="alt text" width="500">

OFDMA divide lo spettro in **unità di risorse (RU)** tempo-frequenza, cioè una sorta di **calendario** (misurato in **slot** invece che in giorni) dove, **per ogni slot**, sono segnate **le sottoportanti** che confluiranno nei sottocanali da **assegnare** alle **sorgenti** che, in quello slot, devono essere **trasmesse in parallelo**. Le **RU** sono proprio i **sottocanali** che sono stati allocati in un certo **slot temporale** alle **sorgenti** da trasmettere in **parallelo**. Un'**entità di coordinamento centrale** (lo scheduler dell'AP in 802.11ax) assegna le RU per la ricezione o la trasmissione a non più di **una sorgente** alla volta. La **pianificazione centralizzata** delle RU permette, inviando più messaggi brevi contemporaneamente sul mezzo radio, di evitare un **sovraccarico** (overhead) di contesa CSMA del canale, il che aumenta l'efficienza in contesti **affollati** di brevi messaggi come le **reti IoT**. OFDMA in, sostanza, **scala** meglio le risorse adattandole a differenti **mix di traffico**, nel contempo, **riducendo l'overhead** delle comunicazioni. Per l'utente un **minore overhead** si traduce in un **ritardo** di ricezione più basso.

<img src="scheduler.png" alt="alt text" width="700">

Si potrebbe anche pianificare la **QoS** in base alla **frequenza**. Ad esempio, sarebbe possibile sfruttare il fatto che l'utente  potrebbe avere una **migliore qualità** del collegamento radio su specifiche sottoportanti della banda disponibile, evitando di trasmettere sulle altre che in quel momento sono molto disturbate. 

<img src="ofdm_ofdma.png" alt="alt text" width="700">

Nelle trasmissioni asincrone **a contesa** (WiFi 6), l'**allocazione delle RU** alle varie stazioni (sorgenti) che intendono trasmettere **in parallelo** è inviata all'**inizio** della comunicazione tramite un **messaggio di controllo** multicast detto **trigger**. Ricevuto il trigger, le stazioni **rispondono in parallelo** e l'avvenuta ricezione da parte dell'AP viene confermata con un unico **ack multicast**. Il trigger contiene anche la **potenza** del segnale che l'AP si aspetta di ricevere da ogni stazione, tramite questa informazione ciascun client può regolare la potenza della propria trasmissione.

<img src="triggerofdma.png" alt="alt text" width="700">

**OFDMA è adoperato** dal **downlink** di **WiFi 6**, telefonia mobile **5G** e **NB IoT**.

### **SC-FDMA**

**SC-FDMA** ha attirato grande attenzione come **alternativa** interessante a OFDMA, specialmente nelle **comunicazioni uplink** poiché il rapporto di potenza di picco su potenza media (**PAPR**) basso avvantaggia notevolmente **il terminale mobile** in termini di efficienza della potenza di trasmissione e costi ridotti dell'amplificatore di potenza. È stato adottato come schema di accesso multiplo del 5G e del NB-IoT.

Il segnale iniziale viene processato da **due blocchi**. Il **secondo** è un normale **OFDMA** mentre **il primo** si chiama **DFT** ed è una **elaborazione numerica** il cui effetto finale è di **convertire** i simboli dei dati dal dominio del tempo trasportandoli nel dominio della frequenza. Il **risultato** è che essi sono **disposti in frequenza** allo stesso modo in cui erano prima **disposti nel tempo**. Una volta nel dominio della frequenza, vengono trasportati nella posizione desiderata nella larghezza di banda complessiva del canale. 

Nella **figura** sottostante, **4 simboli**, con **4 colori diversi**, erano **inizialmente** trasmessi **in parallelo** su **4 sottoportanti** diverse ma con **ampiezze nel tempo** molto **variabili**. Dopo il **blocco DFT**, hanno **ampiezza costante** nel tempo ma variabile nella frequenza. Inoltre la DFT, di fatto, ha effettuato una **conversione** dei simboli **da parallelo a seriale** perché vengono **trasmessi nel tempo in serie**, uno dopo l'altro.

<img src="sc-fdma.png" alt="alt text" width="800">

E' adoperato dall'**uplink** di **WiFi 6**, della telefonia mobile **5G** e **NB IoT**.

Sitografia:
- https://datatracker.ietf.org/doc/html/rfc7554
- https://en.wikipedia.org/wiki/Time_Slotted_Channel_Hopping
- https://static1.squarespace.com/static/54cecce7e4b054df1848b5f9/t/57489e6e07eaa0105215dc6c/1464376943218/Reversing-Lora-Knight.pdf
- https://wirelesspi.com/understanding-lora-phy-long-range-physical-layer/
- https://thesis.unipd.it/retrieve/d813d8b9-9d45-4158-acbc-eada172983c8/Chinta_Venkata_Rajesh.pdf
- https://interline.pl/Information-and-Tips/What-Technology-Behind-LoRa-Frequency
- https://lora-developers.semtech.com/documentation/tech-papers-and-guides/lora-and-lorawan/
- https://medium.com/kgxperience/what-happens-in-lora-lorawan-communication-ab32d56dfc71
- https://wirelesspi.com/understanding-lora-phy-long-range-physical-layer/

>[Torna a reti di sensori](sensornetworkshort.md#servizi-di-accesso-radio-per-WSN) 




