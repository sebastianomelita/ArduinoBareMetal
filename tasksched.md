>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in Python](taskschedpy.md)

## **SCHEDULATORE DI COMPITI BASATO SUL POLLING DEL TEMPO CORRENTE**

Una categoria di **schedulatori di compiti** che, a differenza di quelli basati sulla ```delay()```, possiede la peculiare proprietà di **non bloccare** l'esecuzione degli atri task del ```loop()``` si può ottenere realizzando,  all'interno del ```loop()```, il **polling periodico** della funzione ```millis()```. 

Il **polling** della funzione ```millis()``` serve a stabilire in **quale ciclo** di loop il **tempo corrente** assume il valore di **tempo limite** (timeout) oltre il quale **eseguire** un certo **compito**.

E’ buona norma evitare, all'interno dei task, l’esecuzione frequente di **operazioni lente** quando queste non sono strettamente necessarie, in modo da lasciare spazio ad altre operazioni, quali ad esempio gestione degli eventi di input, che richiedono una velocità elevata per essere usufruite in modo più interattivo.

**In particolare**, all'interno di un task, andrebbe accuratamente evitata l'introduzione di ritardi prolungati tramite **delay()**. Possono esserci per **breve tempo** (qualche mSec) ma, in ogni caso, non dovrebbero mai far parte della maniera (**algoritmo**) con cui la logica di comando **pianifica i suoi eventi**.

Utilizzando la tecnica della **schedulazione esplicita dei task** nel loop(), la **realizzazione di un algoritmo** non dovrebbe essere pensata in **maniera sequenziale (o lineare)** ma, piuttosto, come una **successione di eventi** a cui corrispondono dei **compiti** (task) che **comunicano** con le periferiche di input/output ed, eventualmente, dialogano tra loro in qualche modo (ad es. variabili globali, buffer, ecc.).


### **Momento del campionamento**

Il **polling** (o campionamento periodico) della millis() può essere fatto:
- all'**inizio di ogni ```loop()```** per cui la misura del tempo attuale è **la stessa** per tutti i task e con questa, task per task, ci si regola per stabilire il ciclo di loop() in cui un determinato task diventa "maturo", cioè, quello in cui per lui il tempo misurato supera il **timeout** oltre il quale deve andare in esecuzione.
  ```C++
  // task 1
  unsigned long currMillis = millis();
  if ((currMillis - precs[0]) >= period[0]) {
	precs[0] += period[0]; 
	//................
  }	
  // task 2
  if (((currMillis - precs[1]) >= period[1]) {
	precs[1] += period[1]; 
	//................
  }
  ```
- all'**inizio di ogni task** in modo da **compensare**, nella valutazione precedente del tempo del task, anche di un eventuale **ritardo** cumulato dai task precedentemente eseguiti. 
  E' una variante **più precisa** della precedente anche se **più costosa computazionalmente** (eccessivi polling della millis() quando ci sono molti task).
  ```C++
	// un timer SW indipendente per ogni task
  	// task 1
	if ((millis() - precs[0]) >= period[0]) {
		precs[0] += period[0]; 
        	//..........
	}	
	// task 2
	if (((millis() - precs[1]) >= period[1]) {
		precs[1] += period[1]; 
        	//...........
	} 
    ```
  
### **Precisione del timer SW**

I timer SW basati sul polling possono essere realizzati in due forme:
- senza compensazione del ritardo di esecuzione della millis():
	```C++
	unsigned long tassoluto = 0;
	if ((millis() - tassoluto) >= periodo)
	{
		tassoluto = millis();
		....
	}
	```
 	E' poco preciso sulle lunghe corse se si è interessati anche al tempo assoluto (deriva dopo pochi minuti)
  
 - Con compensazione del ritardo di esecuzione della assegnazione che coinvolge la millis():
	```C++
	unsigned long tassoluto = 0;
	if ((millis() - tassoluto) >= periodo)
	{
		tassoluto += periodo;
		....
	}
	```
	E' più preciso sulle lunghe corse se si è interessati anche al tempo assoluto (deriva dopo parecchie ore)

### **Tipi di schedulazione**

Una volta stabilito ad ogni loop() il valore del tempo corrente, ci sono molti modi per determininare da esso i tempi dei vari task. Ognuna di queste tecniche, in realtà, è adoperabile a prescindere dal campionamento della millis(), tutte richiedono soltanto che lo schedulatore venga chiamato al tempo usato come **riferimento** per la generazione di tutti gli altri tempi. Questo tempo è detto **tempo base**. 

Il **tempo base** è un tempo fisso comune a tutti i task, la loro **periodicità** deve quindi essere un **multiplo intero** del tempo base.

Gli **schedulatori** di questo tipo sono **periodici** e si possono realizzare sostanzialmente in **due modi**:
- mediante ripetizione di una **sequenza di tick** periodici che fissano il **tempo base**. Un successivo **conteggio** o **filtraggio** dei tick discrimina il **momento** in cui deve essere eseguito **un task** piuttosto che un altro.
- mediante **timer periodici** che, per **ogni task**, stabiliscono un **timeout** oltre il quale il **polling** della funzione millis() **segnala** il momento buono per **eseguire** quel task.

<img src="schdulatore_generico.jpg" alt="alt text" width="1000">

### **SCHEDULATORE DI COMPITI BASATO SUL CONTEGGIO DEI TICK**

Di seguito è riportato un esempio di schedulatore che pianifica nel tempo **l’esecuzione periodica** di una serie di **compiti** (task) da programmare **nel futuro** con **cadenza diversa**.

Il **tempo base** è la base dei tempi di tutte le schedulazioni, viene calcolato **periodicamente** all'accadere di un **evento** (superamento di una soglia di tempo) che viene detto **tick** (analogia con il metronomo per la musica).

La **soglia di tempo** corrisponde al **timeout** di un **timer SW** realizzato all'interno del ```loop()``` mediante il **polling** della funzione ```millis()```. Ad ogni ```loop()```, viene **campionato** il valore del **tempo attuale** per vedere **quando** esso raggiunge un **tempo futuro** prestabilito. Quando ciò **accade** viene calcolato il **nuovo tick**.

Le **schedulazioni** nel futuro dei diversi **task** vengono calcolate in corrispondenza di **ogni timeout** conteggiando un certo **multiplo intero** del **tempo base**, ne segue che **il tempo base** dovrebbe essere calcolato come il massimo comune divisore (**MCD**) di tutti i **tempi futuri** che devono essere generati.

Il **conteggio** dei multipli interi del tempo base è tenuto da un **contatore circolare** (step) che deve essere **ruotato** dopo aver effettuato un numero di conteggi pari al **minimo comune multiplo** di **tutti** i tempi in gioco, cioè i tempi di attivazione dei task.

Se ci sono **pulsanti** da gestire insieme ad altri task il tempo base può essere impostato tra 50 e 200 mSec in maniera da poterlo utilizzare per effettuare un **polling degli ingressi** immune dal fenomeno dei rimbalzi (**antibounce SW**). Per una descrizione dettagliata dell'uso della millis() per generare i time tick si rimanda a [tecniche di schedulazione](timesched.md#SCHEDULATORE-CON-TIMER-TICK-GENERATI-DA-MILLIS())).

```C++
#define tbase  1000  // periodo base in milliseconds
#define nstep  1000  // numero di fasi massimo di un periodo generico
unsigned long precm = 0;
unsigned long step = 0;
byte led1 = 13;
byte led2 = 12;

void setup()
{
	pinMode(led1, OUTPUT);
    	pinMode(led2, OUTPUT);
}

void loop()
{
	// polling della millis() alla ricerca del tempo in cui scade ogni periodo
	unsigned long currTime = millis();
	if(currTime-precm >= (unsigned long) tbase){ 	//se è passato un periodo tbase dal precedente periodo
		precm = currTime;  			//preparo il tic successivo azzerando il conteggio del tempo ad adesso
		step = (step + 1) % nstep; 		// conteggio circolare (arriva al massimo a nstep-1)

		// task 1
		if(!(step%2)){  // schedulo eventi al multiplo del periodo (2 sec = 2 periodi)
			digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		}
		// task 2
		if(!(step%3)){  // schedulo eventi al multiplo del periodo (3 sec = 3 periodi)
			digitalWrite(led2,!digitalRead(led2)); 	// stato alto: led blink
		}
		// il codice eseguito al tempo del metronomo (esattamente un periodo) va quì
	}
	// il codice eseguito al tempo massimo della CPU va qui
}
```
Di seguito il link della simulazione online con Tinkercad su Arduino: https://www.tinkercad.com/embed/b7Fj3hCPAwi?editbtn=1

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/348709453878526548

Si noti che:
- il timer SW con il polling viene eseguito ad ogni ciclo di ```loop()```
- il timer SW è **uno solo** per cui il suo codice viene eseguito una sola volta
- il calcolo degli N tempi futuri è eseguito N volte (una per ogni tempo) ma **non ad ogni** ciclo di ```loop()``` bensì, in maniera temporalmente più rada, ad **ogni tick**
- la schedulazione è sensibile a **ritardi** di esecuzione di un task che durino più di due tempi base (```delay > 2*tbase```) poichè causano uno ritardo dell'incremento del contatore ```step```. Il ritardo del **contatore** comporta un ritardo di tutti i task dato che questi misurano il loro tempo a partire dal **conteggio** del suo valore ad ogni tick.

### **SCHEDULATORE DI COMPITI BASATO SU TIMER PERIODICI**

Uno schedulatore di compiti (task) si può realizzare anche utilizzando **più timers** basati sul polling della funzione millis(). 

```C++
byte led1 = 13;
byte led2 = 12;
unsigned long period[2];
unsigned long precs[2];

void setup()
{
	pinMode(led1, OUTPUT);
  	pinMode(led2, OUTPUT);
	precs[0]=0;
	precs[1]=0;
	period[0] = 500;
	period[1] = 1000;
}

void loop()
{
	unsigned long current_millis = millis();
	// task 1
	if ((current_millis - precs[0]) >= period[0]) {
		precs[0] += period[0]; 
        	digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
	}	
	// task 2
	if ((current_millis - precs[1]) >= period[1]) {
		precs[1] += period[1]; 
        	digitalWrite(led2,!digitalRead(led2)); 	// stato alto: led blink
	}
	// il codice eseguito al tempo massimo della CPU va qui
}
```

Si noti che:
- il timer SW con il polling viene eseguito ad ogni ciclo di ```loop()```
- i timer SW sono N (uno per ogni tempo futuro)
- il calcolo degli N tempi futuri è eseguito N volte (una per ogni tempo) **ad ogni ciclo** di ```loop()```
- il calcolo può essere reso estremamente **preciso** e realizzato in maniera indipendente tra un tempo e l'altro.

Di seguito il link della simulazione online con Tinkercad su Arduino: https://www.tinkercad.com/embed/fcbmLkC10ms?editbtn=1

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/351933794966569551

**Recupero dei tick persi in un task lento**

In questo caso il **ritardo** di un **task** maggiore di un **tempo base** è compensato. Infatti se dopo un tempo ```t``` pari a ```x``` multipli di ```tbase``` lo scedulatore ricampiona il tempo con la funzione ```millis()```, allora la condizione ```if ((current_millis - precs[0]) >= period[0])``` sarà valutata alla massima velocità del clock e sarà vera per ```x``` volte, ciò causerà un rapido recupero di ```precs[0]``` fino a che la diferenza non sarà minore di ```tbase```. Da questo momento in poi i tick procederanno senza ritardo fino allo scatto della condizione dei vari task. 

**Riordinamento dei task**

Se il primo task veloce è affetto da ritardi casuali può accadere che questi possono ritardare anche i task che occorrono a seguire nello stesso ```loop()```. Questo accade se più task con **periodicità diversa** occorrono nello stesso tempo (tick).

Se più task con **periodicità diversa** occorrono nello stesso tempo (tick), conviene dare **priorità maggiore** a quelli **con periodicità più lunga** perchè un eventuale **ritardo** di un **task veloce** determinerebbe un **errore di tempo** che coinvolgerebbe solo il **primo task breve** a seguire (rimanendo confinato nel tick corrente) e non avrebbe effetto  sui **tick lenti** (di periodicità più grande che agiscono su più tick) dato che questi sarebbero sempre **serviti prima**. In altre parole, si cerca, in questo modo, di **limitare** l'effetto di eventuali ritardi di un task sul minor numero possibile di tick consecutivi.

**Ricampionamento ad ogni task**

Un'**alternativa** praticabile in questa categoria di schedulatori  potrebbe essere **ricampionare** il tempo **corrente** sia ad ogni ```loop()``` e, all'interno di questo, tra ogni task. Il ricampionamente avrebbe due obiettivi:
- recuperare il ritardo di un task che potrebbe ripercuotersi sui task dei **tick successivi**
- recuperare il ritardo di un task facendo in modo che quello successivo, in esecuzione sullo **stesso tick**, parta al tempo giusto

Una soluzione basata sul **ricampionamento** tra un task e l'altro potrebbe farsi così:

```C++
byte led1 = 13;
byte led2 = 12;
unsigned long period[2];
unsigned long precs[2];
unsigned long prevMillis = 0;

void setup()
{
	randomSeed(analogRead(0));
	Serial.begin(115200); 
	pinMode(led1, OUTPUT);
  	pinMode(led2, OUTPUT);
	precs[0]=0;
	precs[1]=0;
	period[0] = 500;
	period[1] = 2000;
}

void loop()
{
	// task 1
	unsigned long current_millis = millis();
	if ((current_millis - precs[0]) >= period[0]) {
		precs[0] = current_millis; 
		unsigned randomDelay = random(1, 200);
		Serial.print("delay: ");Serial.println(randomDelay);
		delay(randomDelay);
    		digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
	}	
	// task 2
	current_millis = millis();
	if ((current_millis - precs[1]) >= period[1]) {
		precs[1] =  current_millis; 
      		unsigned long now = millis();
		unsigned long diff = now-prevMillis;
		//diff = diff%50;
		Serial.print("ontwosec: ");Serial.println(diff);
		digitalWrite(led2,!digitalRead(led2)); 	// stato alto: led blink
		prevMillis = now;
	}
	// il codice eseguito al tempo massimo della CPU va qui
	delay(1);
}
```
Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/352929647651521537

## **SCHEDULATORE DI COMPITI BASATO SU FILTRAGGIO DEL TEMPO BASE**

E' possibile realizzare uno schedulatore di **più task** che agiscono con periodicità **diverse** in tempi diversi a partire da un timer che esegue periodicamente un **task comune** si sincronizzazione che agisce su un **tempo** base comune **sottomultiplo** del tempo di tutti gli altri task. In altre parole, il **periodo** di ciascun task è un **multiplo intero** del periodo base di sincronizzazione comune:
- il **tick** del **tempo base** viene ottenuto mediante il polling della funzione ```millis()``` eseguito ad **ogni ```loop()```**
- ad ogni **tick** del **tempo base** viene aggiornato il momento del prossimo tick mediante l'espressione ```precm += tbase```
- Il **tempo base** comune a tutti i task, si può calcolare col **massimo comune divisore** (M.C.D. o G.C.D) di tutti i tempi dei singoli task.
- **ad ogni tick** del tempo base viene misurato se è maturato il tick del tempo di ciascun task valutando il valore del tempo base fino a quel momento mediante ```(precm - precs[i]) >= period[i]```
- se la **condizione di scatto** del tick di un task è verificata allora viene calcolato il **tempo** del tick **successivo** sommando al tempo del tick corrente la periodicità del task: ```precs[i] += period[i]```


```C++
byte led1 = 13;
byte led2 = 12;
unsigned long period[2];
unsigned long precs[2];
unsigned long precm;
unsigned long tbase;

void setup()
{
	pinMode(led1, OUTPUT);
  	pinMode(led2, OUTPUT);
	period[0] = 500;
	period[1] = 1000;
	precm = 0;
	tbase = 500;
	// inizializzazione dei task
	for(int i=0; i<2; i++){
		precs[i] = precm -period[i];
	}
}

void loop()
{
	if(millis()-precm >= tbase){ 	
		precm += tbase;  			
		// task 1
		if ((precm - precs[0]) >= period[0]) {
			precs[0] += period[0]; 
			digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		}	
		// task 2
		if ((precm - precs[1]) >= period[1]) {
			precs[1] += period[1]; 
			digitalWrite(led2,!digitalRead(led2)); 	// stato alto: led blink
		}
	}
	// il codice eseguito al tempo massimo della CPU va qui
	delay(1);
}
```
- Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/352691213474403329
- Di seguito il link della simulazione online con Tinkercad su Arduino: https://www.tinkercad.com/things/7IIKSJQCbIr-scheduler-by-millis-con-tick-base

**Tempo assoluto**

La generazione del tempo assoluto di riferimento mediante```precm += tbase``` è basata su considerazioni riportate in [Generazione di tempi assoluti](absolutetime.md)

**Inizializzazione dei task**

Per ottenere la partenza sincronizzata di tutti i task al primo tick del sistema bisogna prima inizializzare il tempo trascorso (```precs[i]```) di ogni task al valore del suo intervallo di esecuzione (``period[i]``):

```C++
//Inizializzazione dei task
for(int i=0; i<2; i++){
	precs[i] = precm -period[i];
}
```

**Recupero dei tick persi in un task lento**

In questo caso il **ritardo** di un **task** maggiore di un **tempo base** è compensato. Infatti se dopo un tempo ```t``` pari a ```x``` multipli di ```tbase``` lo scedulatore ricampiona il tempo con la funzione ```millis()```, allora la condizione ```if(millis()-precm >= tbase``` sarà valutata alla massima velocità del clock e sarà vera per ```x``` volte, ciò causerà un rapido recupero di ```precm``` fino a che la diferenza non sarà minore di ```tbase```. Da questo momento in poi i tick procederanno senza ritardo fino allo scatto della condizione dei vari task.

**Riordinamento dei task**

Se il primo task veloce è affetto da ritardi casuali può accadere che questi possono ritardare anche i task che occorrono a seguire nello stesso ```loop()```. Questo accade se più task con **periodicità diversa** occorrono nello stesso tempo (tick).

Se più task con **periodicità diversa** occorrono nello stesso tempo (tick), conviene dare **priorità maggiore** a quelli **con periodicità più lunga** perchè un eventuale **ritardo** di un **task veloce** determinerebbe un **errore di tempo** che coinvolgerebbe solo il **primo task breve** a seguire (rimanendo confinato nel tick corrente) e non avrebbe effetto  sui **tick lenti** (di periodicità più grande che agiscono su più tick) dato che questi sarebbero sempre **serviti prima**. In altre parole, si cerca, in questo modo, di **limitare** l'effetto di eventuali ritardi di un task sul minor numero possibile di tick consecutivi.

In questo caso **non è possibile ricampionare** i task in maniera indipendente l'uno dall'altro perchè quelli con **tempo uguale** devono avvenire nello **stesso tick**.

Una soluzione in base a quanto descritto sopra potrebbe essere:

```C++
byte led1 = 13;
byte led2 = 12;
unsigned long period[2];
unsigned long precs[2];
unsigned long precm;
unsigned long tbase;
unsigned long prevMillis = 0;

void setup()
{
	randomSeed(analogRead(0));
	Serial.begin(115200); 
	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
	period[0] = 500;
	period[1] = 2000;
	precm = 0;
	tbase = 50;
	//Inizializzazione dei task
	for(int i=0; i<2; i++){
		precs[i] = precm -period[i];
	}
}

void loop()
{
	unsigned  long current_millis = millis();
	if(current_millis-precm >= tbase){ 	
		precm += tbase;  		

		// task 2
		if ((precm - precs[1]) >= period[1]) {
			precs[1] +=  period[1]; 
			unsigned long now = millis();
			unsigned long diff = now-prevMillis;
			//diff = diff%50;
			Serial.print("ontwosec: ");Serial.println(diff);
			digitalWrite(led2,!digitalRead(led2)); 	// stato alto: led blink
			prevMillis = now;
		}	
		// task 1
		if ((precm - precs[0]) >= period[0]) {
			precs[0] +=  period[0]; 
			unsigned randomDelay = random(1, 200);
			Serial.print("delay: ");Serial.println(randomDelay);
			delay(randomDelay);
			digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		}	
	}
	// il codice eseguito al tempo massimo della CPU va qui
	delay(1);
}
```

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/353186425425809409

### **SCHEDULATORE DI COMPITI SU BASE DEI TEMPI GENERICA**

E' possibile realizzare uno schedulatore di task che non necessita di alcuna funzione di misura del tempo (delay() o millis()). L'unico **prerequisito** è che avvenga l'**invocazione periodica** di una funzione ad un certo **tempo base** comune a tutti i task, calcolato col **massimo comune divisore** (M.C.D. o G.C.D) di tutti i tempi dei singoli task. Un **contatore** di tempi base determina, per ogni task, il momento buono in cui questo deve essere eseguito **resettando** il proprio contatore subito dopo.

La **base dei tempi** comune può essere realizzata mediante qualunque tecnica di **generazione di tempi periodici**, ma la particolarità dell'assenza di misure di **tempo corrente** (millis()) rendono la realizzazione particolarmente adatta per essere adoperata su una base dei tempi generata mediante **interrupt**. 

Si ricorda che in C e in C++ le variabili globali a comune tra ISR e loop() principale e suscettibili di modifiche da parte di entrambi andrebbero dichiarate tutte con il qualificatore ```volatile```.

La **versione originale** più completa dello schedulatore insieme ad una dettagliata discussione teorica si trova in: https://www.ics.uci.edu/~givargis/pubs/C50.pdf e in https://www.cs.ucr.edu/~vahid/rios/.

```C++
#include <Ticker.h>

Ticker periodicTicker1;
byte led1 = 13;
byte led2 = 12;
unsigned long period[2];
volatile unsigned long elapsedTime[2];
volatile bool processingRdyTasks;
unsigned long tbase;

void setup()
{
	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
	Serial.begin(115200); 
	periodicTicker1.attach_ms(500, scheduleAll);
	elapsedTime[0] = 0;
	elapsedTime[1] = 0;
	period[0] = 500;
	period[1] = 1000;
	tbase = 500;
	// task time init
	processingRdyTasks = false;
	for(int i=0; i<2; i++){
		elapsedTime[i] = period[i];
	}
}

void scheduleAll(){
	if(processingRdyTasks){ 	
		Serial.println("Timer ticked before task processing done");
	}else{  	
		processingRdyTasks = true;		
		// task 1
		if (elapsedTime[0] >= period[0]) {
			digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
			elapsedTime[0] = 0;
		}
		elapsedTime[0] += tbase;
		// task 2
		if (elapsedTime[1] >= period[1]) {
			digitalWrite(led2,!digitalRead(led2)); 	// stato alto: led blink
			elapsedTime[1] = 0;
		}
		elapsedTime[1] += tbase;
		processingRdyTasks = false;
	}
}

void loop()
{
	delay(10);
	// il codice eseguito al tempo massimo della CPU va qui
}
```
Il **flag** ```processingRdyTasks``` servirebbe ad evitare l'interruzione della ISR sopra un'altra ISR dovuta ad un eventuale ritardo di completamento di un task precedente. Questa circostanza nei microcontrollori come Arduino o ESP32 in genere non accade perchè le **interruzioni annidate** sono **di base** (default) **disabilitate**.

Anche in questo caso **non è possibile ricampionare** i task in maniera indipendente l'uno dall'altro perchè quelli con **tempo uguale** devono avvenire nello **stesso tick**.

**Inizializzazione dei task**

Per ottenere la partenza sincronizzata di tutti i task al primo tick del sistema bisogna prima inizializzare il tempo trascorso (elapsed) di ogni task al valore del suo intervallo di esecuzione (periodo):

```C++
for(int i=0; i<2; i++){
	elapsedTime[i] = period[i];
}
```
Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/352766239477208065

**Recupero dei tick persi in un task lento**

In questo caso il **ritardo** di un **task** maggiore di un **tempo base** non potrebbe essere compensato dato che **da un lato** la funzione ```scheduleAll()``` non è interrompibile per cui ritardo di un task ritarda anche il task successivo, **dall'altro** ```elapsedTime[0] += tbase``` viene incrementata ad ogni tick sempre una sola volta. Due scenari possibili:
-  Se i **tick mancanti**, cioè, non effettuati durante il task lento **vanno persi**. Se il ritardo di un task ha superato più tick, e quelli non eseguiti (perchè il task in corso non era interrompibile) **non vengono recuperati**, allora il task successivo **non** è in grado di recuperare (con un solo tick) il suo conteggio del tempo trascorso.
-  Se i **tick mancanti**, cioè, non effettuati durante il task lento **sono recuperati**. Se il ritardo di un task ha superato più tick, e quelli non eseguiti (perchè il task in corso non era interrompibile) **vengono recuperati**, allora il task successivo è in grado di recuperare (cumulando tutti i tick mancanti in rapida successione) il suo conteggio del tempo trascorso. Il recupero dei tick mancanti è possibile se le ISR di ogni interrupt non vengono perse ma messe in una **coda di esecuzione**, in attesa che le ISR chiamate dagl interrupt **avvenuti prima** vengano servite.

Nello ESP32 sembra che sia effettivo il secondo scenario per cui il repero dei tick persi in un task lento è sempre possibile. 


**Riordinamento dei task**

Se il primo task veloce è affetto da ritardi casuali può accadere che questi possono ritardare anche i task che occorrono a seguire nello stesso ```loop()```. Questo accade se più task con **periodicità diversa** occorrono nello stesso tempo (tick).

Se più task con **periodicità diversa** occorrono nello stesso tempo (tick), conviene dare **priorità maggiore** a quelli **con periodicità più lunga** perchè un eventuale **ritardo** di un **task veloce** determinerebbe un **errore di tempo** che coinvolgerebbe solo il **primo task breve** a seguire (rimanendo confinato nel tick corrente) e non avrebbe effetto  sui **tick lenti** (di periodicità più grande che agiscono su più tick) dato che questi sarebbero sempre **serviti prima**. In altre parole, si cerca, in questo modo, di **limitare** l'effetto di eventuali ritardi di un task sul minor numero possibile di tick consecutivi.


Una soluzione parziale in base a quanto descritto sopra potrebbe essere:

```C++
byte led1 = 13;
byte led2 = 12;
unsigned long period[2];
unsigned long precs[2];
unsigned long elapsedTime[2];
unsigned long precm;
unsigned long precm2;
unsigned long tbase, k;
unsigned long prevMillis = 0;
bool shift = false;

void setup()
{
	randomSeed(analogRead(0));
	Serial.begin(115200); 
	pinMode(led1, OUTPUT);
  	pinMode(led2, OUTPUT);
	precs[0] = 0;
	precs[1] = 0;
	period[0] = 500;
	period[1] = 2000;
	elapsedTime[0] = 0;
	elapsedTime[1] = 0;
	precm = 0;
	precm2 = 0;
	tbase = 50;
}

void loop()
{
	unsigned long current_millis = millis();
	if(current_millis - precm >= tbase){ 		
		precm += tbase;
		// task 2
		if (elapsedTime[1] >= period[1]) {
			unsigned long now = millis();
			unsigned long diff = now-prevMillis;
			//diff = diff%50;
			Serial.print("ontwosec: ");Serial.println(diff);
			digitalWrite(led2,!digitalRead(led2)); 	// stato alto: led blink
			prevMillis = now;
			elapsedTime[1] = 0;
		}
		elapsedTime[1] += tbase;
		// task 1
		if (elapsedTime[0] >= period[0]) {
			unsigned randomDelay = random(1, 200);
			Serial.print("delay: ");Serial.println(randomDelay);
			delay(randomDelay);
			digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
			elapsedTime[0] = 0;
			shift = true;
		}	
		elapsedTime[0] += tbase;		
	}
	// il codice eseguito al tempo massimo della CPU va qui
	delay(1);
}

```

Per compensare i **ritardi** di un task, potrebbe essere sfruttata la proprietà di **prerilascio** forzato di un task che possiedono gli interrupt. Se i tick sono **interrupt based** allora essi accadono sempre e comunque nel tempo esatto a loro deputato anche in presenza di un task che ritarda la sua esecuzione. Questo perchè un tick che occorresse ripetutamente su uno stesso task molto lento causa comunque la sua **interruzione** e l'esecuzione di una **ISR** che, pur **non** potendo servire un **nuovo task**, può far partire un algoritmo che **compensi il conteggio** dei tick dei task successivi a quello che si prolunga troppo in maniera da farli accadere al **tempo giusto**. 

Anche se task di tick successivi non si interferiscono, rimane sempre l'influenza che un task **lento a terminarsi** può avere sui task del suo stesso tick che potrebbe essere ridotta grazie all'espediente del **riordinamento** per tempi decrescenti dei task.

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/353034389606720513

```C++
#include <Ticker.h>
// Inspired from https://www.cs.ucr.edu/~vahid/rios/
Ticker periodicTicker1;
int led1 = 13;
int led2 = 12;
int pulsante=27;
byte precval;
byte stato= LOW;
unsigned long period[2];
unsigned long elapsedTime[2];
void (*tickFct[2])(void); 
volatile bool timerFlag;
unsigned long tbase;
uint8_t tasknum = 2;
unsigned long prevMillis = 0;

void periodicBlink500() {
  	unsigned randomDelay = random(1, 200);
	Serial.print("delay: ");Serial.println(randomDelay);
	delay(randomDelay);
	digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
}

void periodicBlink2000(){
	unsigned long now = millis();
	unsigned long diff = now-prevMillis;
	//diff = diff%50;
	Serial.print("ontwosec: ");Serial.println(diff);
	digitalWrite(led2,!digitalRead(led2)); 	// stato alto: led blink
	prevMillis = now;
}

void setup(){
	randomSeed(analogRead(0));
	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
	Serial.begin(115200); 
	periodicTicker1.attach_ms(50, timerISR);
	elapsedTime[0] = 0;
	elapsedTime[1] = 0;
	period[0] = 2000;
	period[1] = 500;
	tickFct[0] = periodicBlink2000;
	tickFct[1] = periodicBlink500;
	tbase = 50;
	// task time init
	timerFlag = false;
	for(int i=0; i<tasknum; i++){
		elapsedTime[i] = period[i];
	}
}

void timerISR(void) {
   if (timerFlag) {
	//Serial.println("Timer ticked before task processing done");
	for(int i=0; i<tasknum; i++){
		elapsedTime[i] += tbase;
	}
   }
   else {
      timerFlag = true;
   }
   return;
}

void scheduleAll(){		
	for(int i=0; i<tasknum; i++){
		if(elapsedTime[i] >= period[i]) {
			(*tickFct[i])();
			//Serial.print("Ticked: ");Serial.println(elapsedTime[i]);
			elapsedTime[i] = 0;
		}
		elapsedTime[i] += tbase;
	}
}

void loop()
{
	if(timerFlag){
		scheduleAll();
		timerFlag = false;
	}
	delay(1);
	// il codice eseguito al tempo massimo della CPU va qui
}
```

Il **flag** ```timerFlag``` serve a:
- **segnalare** nel ```loop()``` il momento buono per eseguire lo schedulatore, cioè lo scadere di un tempo base
- **evitare l'interruzione** dello schedulatore ```scheduleAll()``` sopra un'altra chiamata non terminata della stessa funzione dovuta ad un eventuale ritardo di completamento di un task precedente. Questa circostanza, che in una ISR normalmente non accade, è invece sempre possibile nel loop dove le funzioni sono eseguite in maniera **non atomica** e quindi sono tutte potenzialmente **interrompibili**. Il controllo si effettua **testando** il flag di schedulazione all'interno della **ISR**. 

La **versione originale** più completa dello schedulatore insieme ad una dettagliata discussione teorica si trova in: https://www.ics.uci.edu/~givargis/pubs/C50.pdf e in https://www.cs.ucr.edu/~vahid/rios/.

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/352976196236642305

### **Versione alternativa di schedulatore generico**

Basato su https://github.com/marcelloromani/Arduino-SimpleTimer/tree/master/SimpleTimer e su [Generazione di tempi assoluti](absolutetime.md)

```C++
#include <Ticker.h>

Ticker periodicTicker1;
byte led1 = 13;
byte led2 = 12;
volatile unsigned long  precm = 0;
unsigned long  tbase1 = 500;
volatile unsigned long precs[]= {0, 0};
unsigned long period1[] = {1500, 6000};
int leds1[] = {led1, led2};

void periodicBlink(int led) {
  digitalWrite(led, !digitalRead(led));
}

void scheduleAll(int *leds){
	precm += tbase1;
	// task 1
	if ((precm - precs[0]) >= period1[0]) {
		precs[0] += period1[0]; 
    		periodicBlink(leds[0]);
	}	
	// task 2
	if ((precm - precs[1]) >= period1[1]) {
		precs[1] += period1[1]; 
    		periodicBlink(leds[1]);
	}
}

void setup(){
	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
	Serial.begin(115200); 
	periodicTicker1.attach_ms(500, scheduleAll, leds1);
	// task time init
	for(int i=0; i<2; i++){
		precs[i] = precm -period1[i];
	}
}

void loop()
{
	delay(10);
	// il codice eseguito al tempo massimo della CPU va qui
}
```

Sui dettagli relativi alla generazione del **tempo assoluto**, sul **recupero dei tick persi in un task lento**, sul **riordinamento dei task** valgono le stesse considerazioni già fatte sopra per lo 

Simulazione su Arduino con Wowki: https://wokwi.com/projects/371852832413769729


### **Schedulatore generico realizzato con funzione get()**

Schedulatore realizzato utilizzando una variante ```DiffTimer2``` del timer della libreria [urutils.h](urutils.h) in cui, ad ogni chiamata della funzione ```get()```, viene incrementato il tempo corrente di ciascun timer (elapsed) di una quantità fissa pari al **tempo base**. Il momento dell'incremento è contestuale a quello della chiamata a ```get()``` che, quindi, deve avvenire esattamente ogni tempo base. Il tempo base è il M.C.D. dei tempi in gioco nei vari task.

Il **tempo base** viene generato utilizzando il timer ```DiffTimer1``` che realizza, mediante il polling della sua funzione ```get()``` il polling della funzione ```millis()``` che restituisce il tempo corrente del sistema. 

```C++
/*
Alla pressione del pulsante si attiva o disattiva il lampeggo di un led, mentre un
altro led lampeggia indisturbato.
*/
#include "urutils.h"
int led = 13;
int led2 = 14;
byte pulsante =12;
byte precval, val;
unsigned long tbase = 50;
DiffTimer tmrdeb;
DiffTimer2 tmrblink1, tmrblink2;
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(pulsante, INPUT);
  precval=LOW;
  tmrdeb.start();
  tmrblink2.start();
}

// loop principale
void loop() {
  if(tmrdeb.get() > tbase){  	
    tmrdeb.reset();   
    //task_toggle
    val = digitalRead(pulsante);		
    if(precval==LOW && val==HIGH){ 		//rivelatore di fronte di salita
      tmrblink1.toggle();		
    }
    precval=val;	
    //task_blink1
    if (tmrblink1.get(tbase) > 500) {
      digitalWrite(led, !digitalRead(led));
      tmrblink1.reset();
    } 
    //task_blink2
    if (tmrblink2.get(tbase) > 1000) {
      digitalWrite(led2, !digitalRead(led2));
      tmrblink2.reset();
    } 
  }
  delay(10);
}
```
Di seguito il link della simulazione online con ESP32 su Wokwi: https://wokwi.com/projects/388359604541585409


## **ESEMPI**

### **Blink a fasi**

```C++
/*
Realizzzare un programma che fa blinkare un led per 5 sec poi lo fa stare sempre spento per un altri 5 sec, 
poi lo fa blinkare di nuovo per altri 5 sec e così via.
*/
#define tbase  500 // periodo base in milliseconds
#define nstep  1000  // numero di fasi massimo di un periodo generico
unsigned long precm = 0;
unsigned long step = 0;
byte led1 = 12;
bool stato;

void setup()
{
	pinMode(led1, OUTPUT);
	stato = true;
}

void loop()
{
	// polling della millis() alla ricerca del tempo in cui scade ogni periodo
	if((millis()-precm) >= (unsigned long) tbase){ 	//se è passato un periodo tbase dal precedente periodo
		precm = millis();  		//preparo il tic successivo azzerando il conteggio del tempo ad adesso
		
		step = (step + 1) % nstep; 	// conteggio circolare (arriva al massimo a nstep-1)

		// task 1
		if(!(step%1)){  // schedulo eventi al multiplo del periodo (2 sec = 2 periodi)
			if(stato)
				digitalWrite(led1,!d.igitalRead(led1)); 	// stato alto: led blink
			else
				digitalWrite(led1,LOW);
		}
		// task 2
		if(!(step%10)){  // schedulo eventi al multiplo del periodo (3 sec = 3 periodi)
			stato = !stato;
		}
		// il codice eseguito al tempo del metronomo (esattamente un periodo) va quì
	}
	// il codice eseguito al tempo massimo della CPU va qui
}
```

Di seguito il link della simulazione online con Tinkercad su Arduino: https://www.tinkercad.com/embed/0vP4WlJGycZ?editbtn=1

### **Blink a fasi con libreria di terze parti**

Una libreria molto precisa basata su timer assoluti è https://github.com/marcelloromani/Arduino-SimpleTimer/tree/master/SimpleTimer.

L'esempio di seguito è basato su una libreria esterna che realizza uno schedulatore ad eventi basato su time ticks (https://github.com/sebastianomelita/time-tick-scheduler).

I blink sono comandati dallo schedulatore esterno ma sono abilitati e disabilitati valutando una variabile di conteggio ausiliaria nella funzione ```epochScheduler()``` che realizza in pratica un timer che **attiva o disattiva** gli eventi periodici dello schedulatore.

```C++
#include "Scheduler2.h"
int led1 = 13;
int led2 = 12;
int count = 0;

Scheduler scheduler;

void onMaxSpeedEvents();
void onHalfSecondEvents();
void onSecondEvents();
void epochScheduler();
 
void periodicBlink(int led) {
  Serial.print("printing periodic blink led ");
  Serial.println(led);

  digitalWrite(led, !digitalRead(led));
}

void epochScheduler(){
	if(count < 10){
		Serial.print("Doing stuff... ");
		Serial.println(count);
	}else if(count == 10){
		Serial.println("Disable all... ");
		scheduler.disableEvent(1,1000);
		scheduler.disableEvent(1,500);
		digitalWrite(led1, LOW);
		digitalWrite(led2, LOW);
		Serial.println("Ending timers...");
	}else if(count < 20){
		Serial.print("Frized... ");
		Serial.println(count);
	}else if(count >= 20){
		Serial.print("Enable all... ");
		Serial.println(count);
		scheduler.enableEvent(1,1000);
		scheduler.enableEvent(1,500);
		Serial.println("Starting timers...");
		count = -1;
	}
  count++;
}

void onHalfSecondEvents(){
	periodicBlink(led1);
}
void onSecondEvents(){
	periodicBlink(led2);
}
 
void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  Serial.begin(115200); 
	Serial.println(F("Scheduler init"));
	Serial.print(F("Time base: "));
	Serial.println(scheduler.getTimebase());
	Serial.print(F("Nsteps: "));
	Serial.println(scheduler.getNsteps());
	//scheduler.addEvent(onMaxSpeedEvents, 1, ON0MSEC);
	scheduler.addPeriodicEvent(onHalfSecondEvents, 1, 500);
	scheduler.addPeriodicEvent(onSecondEvents, 1, 1000);
	scheduler.addPeriodicEvent(epochScheduler, 2, 1000);
}
 
void loop() {
	scheduler.scheduleAll();
	delay(10);
}
```

Di seguito il link della simulazione online con Tinkercad su Arduino: https://wokwi.com/projects/351319080732459608


### **Blink a fasi con libreria di terze parti 2**

Stesso esempio precedente in cui tutti gli eventi periodici sono realizzati con lo schedulatore fornito dalla libreria, compresi gli eventi che abilitano e disabilitano parte delle funzioni di schedulazione tramite i comandi ```enableEvent()``` e ```disableEvent()```.

```C++
#include "Scheduler2.h"
int led1 = 13;
int led2 = 12;

Scheduler scheduler;
int count = 0;

void onMaxSpeedEvents();
void onHalfSecondEvents();
void onSecondEvents();
void epochScheduler();
void epoch10sec();
void epoch20sec();
 
void periodicBlink(int led) {
  Serial.print("printing periodic blink led ");
  Serial.println(led);

  digitalWrite(led, !digitalRead(led));
}

void epoch10secDisable(){
	scheduler.disableEvent(1,1000);
	scheduler.disableEvent(1,500);
	scheduler.enableEvent(1,10000);
	scheduler.disableEvent(2,10000);
	digitalWrite(led1, LOW);
	digitalWrite(led2, LOW);
	Serial.println("Ending timers...");
	Serial.print("Frized... ");
}

void epoch10secEnable(){
	Serial.print("Enable all... ");
	Serial.println(count);
	scheduler.enableEvent(1,1000);
	scheduler.enableEvent(1,500);
	scheduler.disableEvent(1,10000);
	scheduler.enableEvent(2,10000);
	Serial.println("Starting timers...");
}

void onHalfSecondEvents(){
	periodicBlink(led1);
}
void onSecondEvents(){
	periodicBlink(led2);
}
 
void setup() {
	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
	Serial.begin(115200); 
	//scheduler.addEvent(onMaxSpeedEvents, 1, 0);
	scheduler.addPeriodicEvent(onHalfSecondEvents, 1, 500);
	scheduler.addPeriodicEvent(epoch10secEnable, 1, 10000, false);
	scheduler.addPeriodicEvent(epoch10secDisable, 2, 10000);
	scheduler.addPeriodicEvent(onSecondEvents, 1, 1000);
	Serial.println(F("Scheduler init"));
	Serial.print(F("Time base: "));
	Serial.println(scheduler.getTimebase());
	//scheduler.addPeriodicEvent(onSecondEvents, 1, 0);
	//scheduler.deletePeriodicEvent(1, 0);
}
 
void loop() {
	scheduler.scheduleAll();
	delay(10);
}
```

Di seguito il link della simulazione online con Tinkercad su Arduino: https://wokwi.com/projects/352057010320512001

### **Pulsante toggle con antirimbalzo insieme a blink**

In questo esempio si utilizza un unico **timer HW** come **base dei tempi** per uno **schedulatore SW** che gestisce la tempistica di **due task**: 
- uno per la relizzazione di un **tasto toggle** con proprietà di antirimbalzo
- un'altra per la realizzazione del **blink periodico** di un led

Le operazioni benchè semplici vengono considerate come prototipi di task più complessi e magari soggetti a **ritardi** considerevoli. In questa circostanza la loro esecuzione all'interno di una ISR è **sconsigliata** per cui essi vengono eseguiti nel ```loop()``` principale su **segnalazione** di un **flag** asserito dentro la ISR del timer.

```C++
#include <Ticker.h>
// Inspired from https://www.cs.ucr.edu/~vahid/rios/
Ticker periodicTicker1;
int led1 = 13;
int led2 = 12;
int pulsante=27;
byte precval;
byte stato= LOW;
unsigned long period[2];
unsigned long elapsedTime[2];
void (*tickFct[2])(void); 
volatile bool timerFlag;
unsigned long tbase;
uint8_t tasknum = 2;

void periodicBlink500() {
  digitalWrite(led1, !digitalRead(led1));
}

void on50msEvents(){
	byte val = digitalRead(pulsante);		//pulsante collegato in pulldown
	//val = digitalRead(!pulsante);	//pulsante collegato in pullup
	if(precval==LOW && val==HIGH){ 	//rivelatore di fronte di salita
		stato = !stato; 							//impostazione dello stato del toggle	
		//Serial.println(stato);
		digitalWrite(led2, stato);
	}
	precval=val;	
}

void setup(){
	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
	Serial.begin(115200); 
	periodicTicker1.attach_ms(50, timerISR);
	elapsedTime[0] = 0;
	elapsedTime[1] = 0;
	period[0] = 50;
	period[1] = 500;
	tickFct[0] = on50msEvents;
	tickFct[1] = periodicBlink500;
	tbase = 50;
	// task time init
	timerFlag = false;
	for(int i=0; i<tasknum; i++){
		elapsedTime[i] = period[i];
	}
}

void timerISR(void) {
   if (timerFlag) {
      Serial.println("Timer ticked before task processing done");
   }
   else {
      timerFlag = true;
   }
   return;
}

void scheduleAll(){		
	for(int i=0; i<tasknum; i++){
		if(elapsedTime[i] >= period[i]) {
			(*tickFct[i])();
			elapsedTime[i] = 0;
		}
		elapsedTime[i] += tbase;
	}
}

void loop()
{
	if(timerFlag){
		scheduleAll();
		timerFlag = false;
	}
	delay(1);
	// il codice eseguito al tempo massimo della CPU va qui
}
```

Di seguito il link della simulazione online con ESP32 su Wokwi: https://wokwi.com/projects/352790112505422849




### **Sitografia:**

- https://github.com/Koepel/Fun_with_millis#fun_with_millis
- http://arduino.cc/forum/index.php/topic,124048.msg932592.html#msg932592
- https://github.com/marcelloromani/Arduino-SimpleTimer/tree/master/SimpleTimer
- https://github.com/sebastianomelita/time-tick-scheduler
- https://www.ics.uci.edu/~givargis/pubs/C50.pdf
- https://www.cs.ucr.edu/~vahid/rios/
- https://wokwi.com/projects/352691213474403329
- https://wokwi.com/projects/352976196236642305

				      
>[Torna all'indice generazione tempi](indexgenerazionetempi.md)>[Versione in Python](taskschedpy.md)
