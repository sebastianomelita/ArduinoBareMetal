
>[Torna all'indice generale](index.md) >[versione in Python](togglepy.md)

## **PULSANTI CON LETTURA DI UN FRONTE**

###  **PULSANTE TOGGLE**

Si vuole realizzare un pulsante con una memoria dello stato che possa essere modificata ad ogni pressione. Pressioni successive in sequenza accendono e spengono un led. Quindi, a seconda da quando si comincia, pressioni in numero pari accendono mentre quelle in numero dispari spengono, oppure al contrario, pressioni in numero pari spengono mentre quelle in numero dispari accendono.

Inizialmente si potrebbe essere tentati di provare seguente soluzione, adattando la strategia del pulsante precedente introducendo una variabile che conservi lo stato del pulsante che chiameremo _closed_.
```C++
byte in;
byte pulsante =2;
boolean closed=false; // stato pulsante
void setup()
{
	pinMode(pulsante, INPUT);
}

void loop()
{
	in = digitalRead(pulsante);
	if(in==HIGH){ // selezione del livello alto (logica di comando)
		closed = !closed;
		digitalWrite(led,closed);  //scrittura uscita
	}
}
```
Purtroppo questa soluzione ha un paio di **problemi** che ne pregiudicano il **funzionamento corretto**.

**Il primo** è relativo alla **selezione del tipo di evento in ingresso**. In questa soluzione viene rilevata la pressione del pulsante **sul livello** dell’ingresso. Il problema è che il livello viene rilevato per tutto il tempo che il tasto è premuto mediante una lettura per ogni loop(). Il numero di queste letture è imprevedibile sia perché sono moltissime al secondo e sia perché la durata della pressione dipende dall’utente. In più, ad ogni lettura viene modificato lo stato del pulsante con l’istruzione closed=!closed, ne consegue che lo stato finale del pulsante è il risultato di una catena di commutazioni che termina dopo un tempo casuale: abbiamo realizzato una slot machine!.

###  **RILEVATORE DI TRANSITO**

Si vuole realizzare un rilevatore del transito di un oggetto su una fotocellula. Poichè la fotocellula legge l'assenza del fascio luminoso di ritorno, la sua uscita è ripetutamernte vera fino a che l'oggetto non ha completato il suo passaggio. Un passaggio temporalmente lungo genera letture ripetute del transito segnalando più oggetti in corrispondenza di uno solo.

## **SOLUZIONE**

La **soluzione** è **modificare il tipo di selezione** **dell’evento** in ingresso, che deve stavolta avvenire sui **fronti** piuttosto che sui livelli. I fronti sono due, **salita** o **discesa**. Se si rileva il fronte di **salita** si cattura l’evento di **pressione** del pulsante, se si rileva il fronte di **discesa** si cattura l’evento di **rilascio** del pulsante. Ma come facciamo a capire quando siamo in presenza di un **evento di fronte**? Di salita o di discesa?

![fronti](fronti.gif)

In **entrambi i casi**, un evento di fronte si ha quando si è in presenza di una **transizione di livello** di un ingresso, quindi per **rilevare un fronte** è sufficiente **rilevare una transizione**.

Una **evento di interrupt** si può rilevare in due modi:

- **Polling degli ingressi.** Lo scopo è rilevare una **transizione di livello**. Si tratta di campionare periodicamente il livello di un ingresso avendo cura di memorizzare sempre l’ultimo campione misurato su una variabile globale. Al loop successivo questo valore verrà confrontato col nuovo alla ricerca di eventuali variazioni. Il polling, cioè il campionamento, può essere effettuato ad ogni loop, o può essere **decimato**, cioè eseguito periodicamente **ogni tot loop scartati** (decimati).

- **Interrupt.** Su alcune porte di un microcontrollore può essere abilitato il riconoscimento di un evento di interrupt, segnalabile, a scelta, su fronte di salita, fronte di discesa, entrambi i fronti. L’interrupt determina la chiamata asincrona di una funzione di servizio dell’interrupt detta ISR (Interrupt Service Routine) che esegue il codice con la logica di comando in risposta all’evento. La funzione è definita esternamente al loop() e la sua esecuzione è del tutto indipendente da questo.

In figura sono evidenziati i **campionamenti** eseguibili, ad esempio, ad ogni loop(), oppure ad intervalli di tempo preordinati stabiliti da uno schedulatore.

![transizioni](transizioni.png)

Si nota chiaramente che in corrispondenza di ogni fronte, prima e dopo, si misurano differenze di livello. In particolare si può avere:

- Un **fronte di salita** se la **transizione** avviene dal livello basso a quello alto

- Un **fronte di discesa** se la **transizione** avviene dal livello alto a quello basso

Se un rilevatore si limita a segnalare un **generico fronte**, allora per stabilire in quale ci troviamo, basta determinare, al momento della segnalazione, in **quale livello** si trova **l’ingresso**: se è al livello alto è un fronte di salita, se è a quello basso è un fronte di discesa.

Il **secondo problema** è costituito dal fenomeno dei **rimbalzi**. Si palesano come una sequenza di rapide oscillazioni che hanno sia fronti di salita che di discesa. Se l’accensione di un led è associata ad un fronte e il suo spegnimento a quello successivo, allora la pressione di un pulsante realizza, di fatto, la solita slot machine…è necessario un algoritmo di debouncing.

Pulsante toggle con rilevazione del fronte di salita (pressione) e con antirimbalzo realizzato con una **schedulazione ad eventi (time tick)**  (per una spiegazione dettagliata dei time tick si rimanda a [schedulatore di compiti basato sui time tick](tasksched.md)):

```C++
#define tbase  100  // periodo base in milliseconds
unsigned long precm;
//unsigned long step;
byte pari, in;
byte precval, val;
byte led = 13;
byte pulsante =2;
byte stato= LOW;  // variabile globale che memorizza lo stato del pulsante
// utilizzare variabili globali è una maniera per ottenere
// che il valore di una variabile persista tra chiamate di funzione successive
// situazione che si verifica se la funzione è richiamata dentro il loop()

void setup()
{
	precm=0;
	pinMode(led, OUTPUT);
	pinMode(pulsante, INPUT);
	precval=LOW;
}

void loop()
{
	//schedulatore ad eventi con funzione di antirimbalzo
	if((millis()-precm) >= tbase){  	//se è passato un periodo tbase dal precedente periodo	
		precm = millis();             	//preparo il tic successivo azzerando il conteggio del tempo ad adesso

		//codice eseguito al tempo stabilito
		val = digitalRead(pulsante);  // lettura ingressi
		if(precval==LOW && val==HIGH){ // rivelatore di fronte di salita
			stato = !(stato); // impostazione dello stato del toggle
		}
		precval=val;  //memorizzazione livello loop precedente
		digitalWrite(led,stato); // scrittura uscite
	}
}
```
### Esempi di blink insieme ad antirimbalzo realizzati con tecniche di schedulazione diverse

**Schedulatore basato su time ticks**

Pulsante toggle che realizza blink e  antirimbalzo realizzato con una **schedulazione ad eventi senza ritardi (time tick)**:
```C++
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
#define tbase    100       // periodo base in milliseconds
#define nstep    1000      // numero di fasi massimo di un periodo generico

unsigned long precm;
unsigned long step;
byte pari, val;
byte precval;
byte pulsante=12;
byte led = 13;
byte stato= LOW;	// variabile globale che memorizza lo stato del pulsante
			// utilizzare variabili globali è una maniera per ottenere
			// che il valore di una variabile persista tra chiamate di funzione successive
			// situazione che si verifica se la funzione è richiamata dentro il loop()

void setup()
{
  precm=0;
  step=0;
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  precval=LOW;
}

// loop principale
void loop()
{
  //metronomo
  if((millis()-precm) >= tbase){  	//se è passato un periodo tbase dal precedente periodo	
	precm = millis();             	//preparo il tic successivo azzerando il conteggio del tempo ad adesso

	step = (step + 1) % nstep;  	//conteggio circolare arriva al massimo a nstep-1
	
	// schedulazione degli eventi con periodicità tbase (funzione di antibounce per il digitalread a seguire)
	val = digitalRead(pulsante);		//pulsante collegato in pulldown
	//val = digitalRead(!pulsante);		//pulsante collegato in pullup
	if(precval==LOW && val==HIGH){ 		//rivelatore di fronte di salita
		stato = !stato; 		//impostazione dello stato del toggle	
	}
	precval=val;	

	// schedulazione degli eventi con periodicità 1 sec
	if(!(step%10)){     	//ogni secondo (vero ad ogni multiplo di 10)
		if(stato){      // valutazione dello stato del toggle
			digitalWrite(led,!digitalRead(led)); //stato alto: led blink
		}else{
			digitalWrite(led,LOW);		 //stato basso: led spento
		}
	}
  }
}
```
Simulazione online su Arduino con Tinkercad del codice precedente: https://www.tinkercad.com/embed/2wo4e7wLqr0?editbtn=1

Simulazione online su Esp32 con Wowki del codice precedente: https://wokwi.com/projects/348707844567073364

**Schedulatore basato su protothreads**

Pulsante toggle che realizza blink e  antirimbalzo realizzato con una **schedulazione sequenziale con i ritardi** emulati tramite **protothreads** (per una spiegazione dettagliata dei protothread si rimanda a [schedulazione con i protothread](protothreadsched.md)):
```C++
#define LC_INIT(lc)
struct pt { unsigned short lc; };
#define PT_THREAD(name_args)  char name_args
#define PT_BEGIN(pt)          switch(pt->lc) { case 0:
#define PT_WAIT_UNTIL(pt, c)  pt->lc = __LINE__; case __LINE__: \
                              if(!(c)) return 0
#define PT_END(pt)            } pt->lc = 0; return 2
#define PT_INIT(pt)   LC_INIT((pt)->lc)
#define PT_SLEEP(pt, delay) \
{ \
  do { \
    static unsigned long protothreads_sleep; \
    protothreads_sleep = millis(); \
    PT_WAIT_UNTIL(pt, millis() - protothreads_sleep > delay); \
  } while(false); \
}
#define PT_EXITED  2
#define PT_SCHEDULE(f) ((f) < PT_EXITED)
#define PT_YIELD(pt) PT_SLEEP(pt, 0)
//-----------------------------------------------------------------------------------------------------------
// se si usa questa libreria al posto delle macro sopra, togliere il commento iniziale all'include 
// e commentare le macro sopra
//#include "protothreads.h"
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
byte led = 13;
byte pulsante =2;
byte stato= LOW;  // variabile globale che memorizza lo stato del pulsante
// utilizzare variabili globali è una maniera per ottenere
// che il valore di una variabile persista tra chiamate di funzione successive
// situazione che si verifica se la funzione è richiamata dentro il loop()

// definizione protothread del pulsante
pt ptBtn;
int btnThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop del protothread
  while(true) {
	if(digitalRead(pulsante)==HIGH){		// se è alto c'è stato un fronte di salita
		stato = !(stato); 	// impostazione dello stato del toggle
		PT_SLEEP(pt, 50);		// antirimbalzo
		PT_WAIT_UNTIL(pt, digitalRead(pulsante)==LOW);	// attendi fino al prossimo fronte di discesa
	}
	PT_YIELD(pt);
  }
  PT_END(pt);
}

// definizione protothread del lampeggio
pt ptBlink;
int blinkThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop del protothread
  while(true) {
	if (stato) {
		digitalWrite(led, HIGH);   	// turn the LED on (HIGH is the voltage level)
		PT_SLEEP(pt, 1000);
		digitalWrite(led, LOW);    	// turn the LED off by making the voltage LOW
		PT_SLEEP(pt, 1000);
	} else {
		digitalWrite(led, LOW);    	// turn the LED off by making the voltage LOW
		PT_YIELD(pt);
	}
  }
  PT_END(pt);
}

void setup()
{
	PT_INIT(&ptBtn);
	PT_INIT(&ptBlink);
	pinMode(led, OUTPUT);
	pinMode(pulsante, INPUT);
}

// loop principale
void loop()
{
	PT_SCHEDULE(btnThread(&ptBtn)); 	// esecuzione schedulatore protothreads
	PT_SCHEDULE(blinkThread(&ptBlink)); 	// esecuzione schedulatore protothreads
}
```

Simulazione online su Arduino con Tinkercad del codice precedente: https://www.tinkercad.com/embed/0cAN5RGK8cB?editbtn=1

Simulazione online su Esp32 con Wowki del codice precedente [https://wokwi.com/projects/348705487464694356](https://www.tinkercad.com/things/cPVToRzRLgI-copy-of-accensione-led-modo-toggle2-con-memoria-/editel)

**Schedulatore basato su threads**

Pulsante toggle che realizza blink e  antirimbalzo realizzato con una **schedulazione sequenziale con i ritardi** reali all'interno di **threads** su **core diversi**. La libreria usata è quella nativa dello ESP32 che implementa dalla fabbrica un **middleware RTOS** per cui non è necessario **includere** nessuna libreria esterna (per una spiegazione dettagliata dei thread si rimanda a [schedulatore di compiti basato sui thread](threadsched.md)):

```C++
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
static uint8_t taskCore0 = 0;
static uint8_t taskCore1 = 1;
int led = 13;
byte pulsante =12;
byte stato= LOW;  // variabile globale che memorizza lo stato del pulsante
// utilizzare variabili globali è una maniera per ottenere
// che il valore di una variabile persista tra chiamate di funzione successive
// situazione che si verifica se la funzione è richiamata dentro il loop()

// attesa evento con tempo minimo di attesa
void waitUntilInputLow(int btn, unsigned t)
{
    while(!digitalRead(btn)==LOW){
	    delay(t);
    }
}
 
void btnThread(void * d){
	String taskMessage = "Task running on core ";
    	taskMessage = taskMessage + xPortGetCoreID();
	
	// Loop del thread
	while(true){
		if(digitalRead(pulsante)==HIGH){			// se è alto c'è stato un fronte di salita
			stato = !stato; 				// impostazione dello stato del toggle
			waitUntilInputLow(pulsante,50);			// attendi finchè non c'è fronte di discesa
		}
		delay(10); 
	}
}

void blinkThread(void * d){
	String taskMessage = "Task running on core ";
    	taskMessage = taskMessage + xPortGetCoreID();
	
	// Loop del thread
	while(true){
		if (stato) {
			digitalWrite(led, HIGH);   	// turn the LED on (HIGH is the voltage level)
			delay(500);
			digitalWrite(led, LOW);    	// turn the LED off by making the voltage LOW
			delay(500);
		} else {
			digitalWrite(led, LOW);    	// turn the LED off by making the voltage LOW
			delay(10); 			// equivale a yeld()
		}
	}
}
 
void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  
  Serial.print("Starting to create task on core ");
  Serial.println(taskCore0);
  xTaskCreatePinnedToCore(
		blinkThread,   /* Function to implement the task */
		"blinkThread", /* Name of the task */
		10000,      /* Stack size in words */
		NULL,       /* Task input parameter */
		0,          /* Priority of the task */
		NULL,       /* Task handle. */
		taskCore0);  /* Core where the task should run */
   Serial.println("Task created...");
   delay(500);
   Serial.print("Starting to create task on core ");
   Serial.println(taskCore1);
   xTaskCreatePinnedToCore(
		btnThread,   /* Function to implement the task */
		"btnThread", /* Name of the task */
		10000,      /* Stack size in words */
		NULL,       /* Task input parameter */
		0,          /* Priority of the task */
		NULL,       /* Task handle. */
		taskCore1);  /* Core where the task should run */
  Serial.println("Task created...");
}

// loop principale
void loop() {
	delay(10);
}
```
Simulazione online del codice precedente https://wokwi.com/projects/348705487464694356

**FreeRTOS** è un **SO per sistemi embedded** molto usato e dalle buone prestazioni che però, per l'utilizzo dei thread, espone delle **API proprietarie** che non possono essere usate su sistemi diversi da FreeRTOS. Per i thread è stato sviluppato da anni lo **standard POSIX** detto **phthread** che definisce in maniera **uniforme**, per **i sistemi** (Linux, Microsoft) e per i **linguaggi** (C, C++) ad esso aderenti, una serie di API che rendono il codice che contiene la programmazione dei thread molto **più portabile**.

Di seguito è riportata la gestione di un pulsante toggle che realizza blink e  antirimbalzo realizzato con una **schedulazione sequenziale con i ritardi** reali all'interno di **threads**. La libreria usata è quella standard **phthread** che non è supportata nativamente da ESP32 ma solo indirettamente tramite l'**inclusione** di una libreria di **terze parti** che implementa pthreads **sopra** le API FreeRTOS esistenti:

```C++
/*Alla pressione del pulsante si attiva o disattiva il lampeggo di un led*/
#include <pthread.h> //libreria di tipo preemptive
pthread_t t1;
pthread_t t2;
int delayTime ;
int led = 13;
byte pulsante =12;
byte stato= LOW;  // variabile globale che memorizza lo stato del pulsante
// utilizzare variabili globali è una maniera per ottenere
// che il valore di una variabile persista tra chiamate di funzione successive
// situazione che si verifica se la funzione è richiamata dentro il loop()

// attesa evento con tempo minimo di attesa
void waitUntilInputLow(int btn, unsigned t)
{
    while(!digitalRead(btn)==LOW){
	    delay(t);
    }
}

void * btnThread(void * d)
{
    int time;
    time = (int) d;
    while(true){   			                                // Loop del thread
        if(digitalRead(pulsante)==HIGH){			        // se è alto c'è stato un fronte di salita
            stato = !stato; 	                            // impostazione dello stato del toggle
            waitUntilInputLow(pulsante,time);		// attendi finchè non c'è fronte di discesa
        }
        delay(10); 
    }
}

void * blinkThread(void * d)
{
    int time;
    time = (int) d;
    while(true){    				// Loop del thread	
	if (stato) {
		digitalWrite(led, !digitalRead(led));
		delay(time);
	} else {
		digitalWrite(led, LOW);    	// turn the LED off by making the voltage LOW
		delay(10); 					// equivale a yeld()
	}
    }
    return NULL;
}

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(pulsante, INPUT);
  delayTime = 50;
  if (pthread_create(&t1, NULL, btnThread, (void *)delayTime)) {
         Serial.println("Errore crezione btnThread");
  }
  delayTime = 500;
  if (pthread_create(&t2, NULL, blinkThread, (void *)delayTime)) {
         Serial.println("Errore crezione blinkThread");
  } 
}

void loop() {
 delay(10); // this speeds up the simulation
	
}
```
Simulazione online del codice precedente https://wokwi.com/projects/348523574025257556

>[Torna all'indice](indexpulsanti.md) >[versione in Python](togglepy.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTE1NTkxOTA3OTIsLTk1MzQ1NDY2NV19
-->
