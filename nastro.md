
>[Torna all'indice](indexpulsanti.md) >[versione in Python](gruppipulsantipy.md)

##  **NASTRO TRASPORTATORE**

###  **Gestione di un singolo nastro**

Programma per la gestione di un nastro trasportatore realizzato con un **timer** basato sul polling della funzione ```millis()``` e su **rilevatori di transito** toggle basati su ```delay()```.

```C++
/*
Scrivere un programma che realizzi la gestione di un nastro traportatore attraverso la lettura di tre sensori 
di transito e il comando di un motore.
I sensori permangono al livello alto finchè un oggetto ingombra l'area del sensore, dopodichè vanno a livello basso. 
I sensori sono due all'inizio del nastro (uno per i pezzi bassi ed un'altro per quelli alti) ed uno alla fine del nastro 
che rileva il pezzo pronto per essere prelevato.
All'attivazione di un qualsiasi sensore di ingresso parte il motore e si resetta (blocca) il timer di volo.
All'attivazione del sensore di uscita si blocca il nastro, alla sua disattivazione riparte il nastro e parte il timer di volo.
Allo scadere del timer di volo si spegne il motore.
*/
byte startSensorHigh = 4;
byte startSensorLow = 3;
byte stopSensor = 2;
byte engineLed = 10;
byte lowStartLed = 9;
byte highStartLed = 8;
byte stopLed = 11;
unsigned flyTime = 4000; //tempo di volo di un pezzo sul nastro
bool engineon;  // variabile globale che memorizza lo stato del motore
//inizio variabili timer
unsigned long startTime;
unsigned long timelapse;
byte timerState=0;
//fine variabili timer

// funzione di attivazione
void startTimer(unsigned long duration){
	timerState=1;
	timelapse=duration;
	startTime=millis();
}

// funzione di disattivazione
void stopTimer(){
	timerState=0;
}

// polling: verifica se è arrivato il tempo di far scattare il timer
void aggiornaTimer(){
	if((timerState == 1) && (millis() - startTime >= timelapse)){
		timerState=0;
		onElapse();
	}
}	

// callback: azione standard da compiere allo scadere del timer, definita fuori dal loop
void onElapse(){
	engineon = false; 
	digitalWrite(engineLed, LOW);
	Serial.println("Timer di volo scaduto");
}

// attesa evento con tempo minimo di attesa
void waitUntilInputLow(int btn, unsigned t)
{
    while(!digitalRead(btn)==LOW){
	    delay(t);
    }
}
  
void setup() {
  Serial.begin(115200);
  pinMode(engineLed, OUTPUT);
  pinMode(lowStartLed, OUTPUT);
  pinMode(highStartLed, OUTPUT);
  pinMode(stopLed, OUTPUT);
  pinMode(startSensorHigh, INPUT);
  pinMode(startSensorLow, INPUT);
  pinMode(stopSensor, INPUT); 
  engineon= false;
  stopTimer();
}

// loop principale
void loop() {
	aggiornaTimer();
	if(digitalRead(startSensorLow)==HIGH){				// se è alto c'è stato un fronte di salita
		engineon = true; 	
		digitalWrite(engineLed, HIGH);
		digitalWrite(lowStartLed, HIGH);
		topTimer();						// c'è almeno un pezzo in transito
		Serial.println("Pezzo basso in ingresso");
		Serial.println("Timer di volo disattivato");
		waitUntilInputLow(startSensorLow,50);			// attendi finchè non c'è fronte di discesa
		Serial.println("Pezzo basso transitato in ingresso");
		digitalWrite(lowStartLed, LOW);
	}if(digitalRead(startSensorHigh)==HIGH){			// se è alto c'è stato un fronte di salita
		engineon = true; 	
		digitalWrite(engineLed, HIGH);
		digitalWrite(highStartLed, HIGH);
		stopTimer();						// c'è almeno un pezzo in transito
		Serial.println("Pezzo alto in ingresso");
		Serial.println("Timer di volo disattivato");
		waitUntilInputLow(startSensorHigh,50);			// attendi finchè non c'è fronte di discesa
		Serial.println("Pezzo alto transitato in ingresso");
		digitalWrite(highStartLed, LOW);
	}else if(digitalRead(stopSensor)==HIGH) {
		engineon = false; 		
		digitalWrite(engineLed, LOW);
		digitalWrite(stopLed, HIGH);
		Serial.println("Pezzo in uscita");
		waitUntilInputLow(stopSensor,50);
		Serial.println("Pezzo prelevato dall'uscita");
		engineon = true; 
		digitalWrite(stopLed, LOW);
		digitalWrite(engineLed, HIGH);
		startTimer(flyTime); 					// se c'è un pezzo in transito arriverà prima dello scadere
		Serial.println("Timer di volo attivato");
	} else {
		
	}
}
```
Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/8UKvLNjeLEQ?editbtn=1

###  **Gestione di due nastri**

Programma per la gestione di **due** nastri trasportatori realizzato con un **timer HW** gestito dalla libreria ```Ticker``` e con **rilevatori di transito** toggle basati su istruzioni ```delay()```. Il **timer** di sistema lavora con segnali di **interrupt** che attivano **callback** invocate **in sequenza**, per cui al loro interno sarebbe opportuno perdere poco tempo evitando di usare istruzioni lente (**no delay()**). I **rilevatori di transito** riguardano due nastri e la loro definizione è **indipendente** per ciascuno di essi perchè è realizzata all'interno di due **thread** separati.

Nel ```loop()``` principale è gestito lo **switch** di un **pulsante generale** di sicurezza che **disabilità** la marcia dei motori di entrambi i nastri. La gestione è **non bloccante** e inibisce l'attivazione dei motori anche se i thread ancora non hanno completato il loro flusso di esecuzione arrivando fino all'ultima istruzione. Infatti, l'istruzione ```digitalWrite(n->engineLed, HIGH && isrun)``` accende il motore solo se la variabile globale ```isrun``` è asserita dallo switch nel **loop** principale del programma.

```C++
/*
Scrivere un programma che realizzi la gestione di un nastro traportatore attraverso la lettura di tre sensori 
di transito e il comando di un motore.
I sensori permangono al livello alto finchè un oggetto ingombra l'area del sensore, dopodichè vanno a livello basso. 
I sensori sono due all'inizio del nastro (uno per i pezzi bassi ed un'altro per quelli alti) ed uno alla fine del nastro 
che rileva il pezzo pronto per essere prelevato.
All'attivazione di un qualsiasi sensore di ingresso parte il motore e si resetta (blocca) il timer di volo.
All'attivazione del sensore di uscita si blocca il nastro, alla sua disattivazione riparte il nastro e parte il timer di volo.
Allo scadere del timer di volo si spegne il motore.
*/
#include <pthread.h> //libreria di tipo preemptive
#include <Ticker.h>
pthread_t t1;
pthread_t t2;
bool engineon;  // variabile globale che memorizza lo stato del motore
Ticker fly[2];
uint8_t safetystop;
bool isrun;

typedef struct
{
	uint8_t id;
	uint8_t startSensorHigh;
	uint8_t startSensorLow;
	uint8_t stopSensor;
	uint8_t engineLed;
	uint8_t lowStartLed;
	uint8_t highStartLed;
	uint8_t stopLed;
	unsigned flyTime;
	bool engineon;
} Nastro;

Nastro nastro1, nastro2;

void initNastri(){
	// porte nastro 1
	nastro1.id = 0;
	nastro1.startSensorHigh=14;
	nastro1.startSensorLow=12;
	nastro1.stopSensor=13;
	nastro1.engineLed=33;
	nastro1.lowStartLed=26;
	nastro1.highStartLed=25;
	nastro1.stopLed=27;
	nastro1.flyTime=4000;
	// porte nastro 2
	nastro2.id = 1;
	nastro2.startSensorHigh=4;
	nastro2.startSensorLow=2;
	nastro2.stopSensor=15;
	nastro2.engineLed=21;
	nastro2.lowStartLed=18;
	nastro2.highStartLed=19;
	nastro2.stopLed=5;
	nastro2.flyTime=8000;
	// set porte nastro 1
	pinMode(nastro1.engineLed, OUTPUT);
	pinMode(nastro1.lowStartLed, OUTPUT);
	pinMode(nastro1.highStartLed, OUTPUT);
	pinMode(nastro1.stopLed, OUTPUT);
	//pinMode(nastro1.motorCmd, OUTPUT);
	pinMode(nastro1.startSensorHigh, INPUT);
	pinMode(nastro1.startSensorLow, INPUT);
	pinMode(nastro1.stopSensor, INPUT); 
	// set porte nastro 2
	pinMode(nastro2.engineLed, OUTPUT);
	pinMode(nastro2.lowStartLed, OUTPUT);
	pinMode(nastro2.highStartLed, OUTPUT);
	pinMode(nastro2.stopLed, OUTPUT);
	//pinMode(nastro2.motorCmd, OUTPUT);
	pinMode(nastro2.startSensorHigh, INPUT);
	pinMode(nastro2.startSensorLow, INPUT);
	pinMode(nastro2.stopSensor, INPUT); 
}

// attesa evento con tempo minimo di attesa
void waitUntilInputLow(int btn, unsigned t)
{
    while(!digitalRead(btn)==LOW){
	    delay(t);
    }
}

void stopEngine(Nastro *n){ 
	String id = "Nastro "+String(n->id) + ": ";	 
	n->engineon = false; 
	digitalWrite(n->engineLed, LOW);
	Serial.println(id+"Timer di volo scaduto");
	fly[n->id].detach();
};

void * beltThread(void * d)
{
		Nastro *n;
    n = (Nastro *) d;
    while(true){    	
	String id = "Nastro "+String(n->id) + ": ";			
	if(digitalRead(n->startSensorLow)==HIGH){				// se è alto c'è stato un fronte di salita
		n->engineon = true && isrun; 	
		digitalWrite(n->engineLed, HIGH && isrun);
		digitalWrite(n->lowStartLed, HIGH);
		fly[n->id].detach();						// c'è almeno un pezzo in transito
		Serial.println(id+"Pezzo basso in ingresso");
		Serial.println(id+"Timer di volo disattivato");
		waitUntilInputLow(n->startSensorLow,50);			// attendi finchè non c'è fronte di discesa
		Serial.println(id+"Pezzo basso transitato in ingresso");
		digitalWrite(n->lowStartLed, LOW);
	}if(digitalRead(n->startSensorHigh)==HIGH){				// se è alto c'è stato un fronte di salita
		n->engineon = true && isrun; 	
		digitalWrite(n->engineLed, HIGH && isrun);
		digitalWrite(n->highStartLed, HIGH);
		fly[n->id].detach();						// c'è almeno un pezzo in transito
		Serial.println(id+"Pezzo alto in ingresso");
		Serial.println(id+"Timer di volo disattivato");
		waitUntilInputLow(n->startSensorHigh,50);			// attendi finchè non c'è fronte di discesa
		Serial.println(id+"Pezzo alto transitato in ingresso");
		digitalWrite(n->highStartLed, LOW);
	}else if(digitalRead(n->stopSensor)==HIGH) {
		n->engineon = false; 		
		digitalWrite(n->engineLed, LOW);
		digitalWrite(n->stopLed, HIGH);
		Serial.println(id+"Pezzo in uscita");
		waitUntilInputLow(n->stopSensor,50);
		Serial.println(id+"Pezzo prelevato dall'uscita");
		n->engineon = true && isrun; 
		digitalWrite(n->stopLed, LOW);
		digitalWrite(n->engineLed, HIGH && isrun);
		fly[n->id].attach_ms(n->flyTime,stopEngine,n);
		Serial.println(id+"Timer di volo attivato");
	} else {
		//digitalWrite(led, LOW);    
		delay(10);	
	}
	delay(10);
    }
    return NULL;
}

void setup() {
  Serial.begin(115200);
  safetystop = 32;
  pinMode(safetystop, INPUT);
  engineon= false;
  isrun = false;
  initNastri();
  if (pthread_create(&t1, NULL, beltThread, (void *)&nastro1)) {
         Serial.println("Errore creazione btnThread");
  }
  if (pthread_create(&t2, NULL, beltThread, (void *)&nastro2)) {
         Serial.println("Errore creazione blinkThread");
  } 
}

void loop() {
	if(digitalRead(safetystop)==HIGH){			        // se è alto c'è stato un fronte di salita
		isrun = false;
		digitalWrite(nastro1.engineLed, LOW);	                // impostazione dello stato del toggle
		digitalWrite(nastro2.engineLed, LOW);
	}else{
		isrun = true;
	}
	delay(10); 							// equivale a yeld() (10 per le simulazioni 0 in HW)
}
```
Simulazione su Esp32 con Wowki: https://wokwi.com/projects/349524035968696915

###  **Gestione di due nastri e tasto emergenza con interrupt**


```C++
/*
Scrivere un programma che realizzi la gestione di un nastro traportatore attraverso la lettura di tre sensori 
di transito e il comando di un motore.
I sensori permangono al livello alto finchè un oggetto ingombra l'area del sensore, dopodichè vanno a livello basso. 
I sensori sono due all'inizio del nastro (uno per i pezzi bassi ed un'altro per quelli alti) ed uno alla fine del nastro 
che rileva il pezzo pronto per essere prelevato.
All'attivazione di un qualsiasi sensore di ingresso parte il motore e si resetta (blocca) il timer di volo.
All'attivazione del sensore di uscita si blocca il nastro, alla sua disattivazione riparte il nastro e parte il timer di volo.
Allo scadere del timer di volo si spegne il motore.
*/
#include <pthread.h> //libreria di tipo preemptive
#include <Ticker.h>
#define DEBOUNCETIME 50
pthread_t t1;
pthread_t t2;
bool engineon;  // variabile globale che memorizza lo stato del motore
Ticker fly[2];
uint8_t safetystop;
bool isrun;
//gestione interrupt
volatile unsigned long previousMillis = 0;
volatile unsigned short numberOfButtonInterrupts = 0;
volatile bool lastState;
bool prevState;
//fine gestione interrupt

typedef struct
{
	uint8_t id;
	uint8_t startSensorHigh;
	uint8_t startSensorLow;
	uint8_t stopSensor;
	uint8_t engineLed;
	uint8_t lowStartLed;
	uint8_t highStartLed;
	uint8_t stopLed;
	unsigned flyTime;
	bool engineon;
} Nastro;

Nastro nastro1, nastro2;

void initNastri(){
	// porte nastro 1
	nastro1.id = 0;
	nastro1.startSensorHigh=14;
	nastro1.startSensorLow=12;
	nastro1.stopSensor=13;
	nastro1.engineLed=33;
	nastro1.lowStartLed=26;
	nastro1.highStartLed=25;
	nastro1.stopLed=27;
	nastro1.flyTime=4000;
	// porte nastro 2
	nastro2.id = 1;
	nastro2.startSensorHigh=4;
	nastro2.startSensorLow=2;
	nastro2.stopSensor=15;
	nastro2.engineLed=21;
	nastro2.lowStartLed=18;
	nastro2.highStartLed=19;
	nastro2.stopLed=5;
	nastro2.flyTime=8000;
	// set porte nastro 1
	pinMode(nastro1.engineLed, OUTPUT);
	pinMode(nastro1.lowStartLed, OUTPUT);
	pinMode(nastro1.highStartLed, OUTPUT);
	pinMode(nastro1.stopLed, OUTPUT);
	//pinMode(nastro1.motorCmd, OUTPUT);
	pinMode(nastro1.startSensorHigh, INPUT);
	pinMode(nastro1.startSensorLow, INPUT);
	pinMode(nastro1.stopSensor, INPUT); 
	// set porte nastro 2
	pinMode(nastro2.engineLed, OUTPUT);
	pinMode(nastro2.lowStartLed, OUTPUT);
	pinMode(nastro2.highStartLed, OUTPUT);
	pinMode(nastro2.stopLed, OUTPUT);
	//pinMode(nastro2.motorCmd, OUTPUT);
	pinMode(nastro2.startSensorHigh, INPUT);
	pinMode(nastro2.startSensorLow, INPUT);
	pinMode(nastro2.stopSensor, INPUT); 
}

// attesa evento con tempo minimo di attesa
void waitUntilInputLow(int btn, unsigned t)
{
    while(!digitalRead(btn)==LOW){
	    delay(t);
    }
}

void stopEngine(Nastro *n){ 
	String id = "Nastro "+String(n->id) + ": ";	 
	n->engineon = false; 
	digitalWrite(n->engineLed, LOW);
	Serial.println(id+"Timer di volo scaduto");
	fly[n->id].detach();
};

void * beltThread(void * d)
{
		Nastro *n;
    n = (Nastro *) d;
    while(true){    	
	String id = "Nastro "+String(n->id) + ": ";			
	if(digitalRead(n->startSensorLow)==HIGH){				// se è alto c'è stato un fronte di salita
		n->engineon = true && isrun; 	
		digitalWrite(n->engineLed, HIGH && isrun);
		digitalWrite(n->lowStartLed, HIGH);
		fly[n->id].detach();						// c'è almeno un pezzo in transito
		Serial.println(id+"Pezzo basso in ingresso");
		Serial.println(id+"Timer di volo disattivato");
		waitUntilInputLow(n->startSensorLow,50);			// attendi finchè non c'è fronte di discesa
		Serial.println(id+"Pezzo basso transitato in ingresso");
		digitalWrite(n->lowStartLed, LOW);
	}if(digitalRead(n->startSensorHigh)==HIGH){				// se è alto c'è stato un fronte di salita
		n->engineon = true && isrun; 	
		digitalWrite(n->engineLed, HIGH && isrun);
		digitalWrite(n->highStartLed, HIGH);
		fly[n->id].detach();						// c'è almeno un pezzo in transito
		Serial.println(id+"Pezzo alto in ingresso");
		Serial.println(id+"Timer di volo disattivato");
		waitUntilInputLow(n->startSensorHigh,50);			// attendi finchè non c'è fronte di discesa
		Serial.println(id+"Pezzo alto transitato in ingresso");
		digitalWrite(n->highStartLed, LOW);
	}else if(digitalRead(n->stopSensor)==HIGH) {
		n->engineon = false; 		
		digitalWrite(n->engineLed, LOW);
		digitalWrite(n->stopLed, HIGH);
		Serial.println(id+"Pezzo in uscita");
		waitUntilInputLow(n->stopSensor,50);
		Serial.println(id+"Pezzo prelevato dall'uscita");
		n->engineon = true && isrun; 
		digitalWrite(n->stopLed, LOW);
		digitalWrite(n->engineLed, HIGH && isrun);
		fly[n->id].attach_ms(n->flyTime,stopEngine,n);
		Serial.println(id+"Timer di volo attivato");
	} else {
		//digitalWrite(led, LOW);    
		delay(10);	
	}
	delay(10);
    }
    return NULL;
}

void setup() {
  Serial.begin(115200);
  safetystop = 32;
  pinMode(safetystop, INPUT);
  attachInterrupt(digitalPinToInterrupt(safetystop), switchPressed, CHANGE );  
  engineon= false;
  isrun = true;
  initNastri();
  if (pthread_create(&t1, NULL, beltThread, (void *)&nastro1)) {
         Serial.println("Errore creazione btnThread");
  }
  if (pthread_create(&t2, NULL, beltThread, (void *)&nastro2)) {
         Serial.println("Errore creazione blinkThread");
  } 
}

// Interrupt Service Routine (ISR)
void switchPressed ()
{
  numberOfButtonInterrupts++; // contatore rimbalzi
  lastState = digitalRead(safetystop); // lettura stato pulsante
  previousMillis = millis(); // tempo evento
  if(lastState==HIGH){ // fronte di salita
	isrun = false; 				// impostazione dello stato dei nastri
	digitalWrite(nastro1.engineLed, LOW);	               
	digitalWrite(nastro2.engineLed, LOW);
  }
}  // end of switchPressed

void rearmPoll()
{
   // sezione critica
   // protegge previousMillis che, essendo a 16it, potrebbe essere danneggiata se interrotta da un interrupt
   // numberOfButtonInterrupts è 8bit e non è danneggiabile ne in lettura ne in scrittura
   noInterrupts();
   // il valore lastintTime potrà essere in seguito letto interrotto ma non danneggiato
   unsigned long lastintTime = previousMillis;
   interrupts();
   
   if ((numberOfButtonInterrupts != 0) //flag interrupt! Rimbalzo o valore sicuro? 
        && (millis() - lastintTime > DEBOUNCETIME )//se è passato il transitorio 
	&& prevState != lastState // elimina transizioni anomale LL o HH 
	&& digitalRead(safetystop) == lastState)//se coincide con il valore di un polling
   { 
	Serial.print("HIT: "); Serial.print(numberOfButtonInterrupts);
	numberOfButtonInterrupts = 0; // reset del flag

	prevState = lastState;
	if(lastState){ // fronte di salita
		Serial.println(" in SALITA");
	}else{
		Serial.println(" in DISCESA");
		Serial.println(" Riattivo il nastro dopo blocco sicurezza");
		isrun = true;
	}
    }
}

void loop() {
	rearmPoll();
	delay(10); 							// equivale a yeld() (10 per le simulazioni 0 in HW)
}
```

Simulazione su Esp32 con Wowki: https://wokwi.com/projects/349645533881565780

>[Torna all'indice](indexpulsanti.md) >[versione in Python](gruppipulsantipy.md)
