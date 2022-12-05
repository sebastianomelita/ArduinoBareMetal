>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  

## **EVENTI PERIODICI CONSECUTIVI**

### **SCHEDULAZIONE CON I DELAY**

```C++
/*
Realizzzare un programma che fa blinkare un led con periodo mezzo secondo per 5 sec e poi lo fa lo blinkare 
con periodo 300 msec per 3 sec, poi ricomincia d'accapo.
*/
byte led1 = 13;

void setup()
{
	pinMode(led1, OUTPUT);
}

void loop()
{
	// task 1
	for(int i=0; i<10; i++){ //5000/500-->10
		digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		delay(500);
	}
	// task 2
	for(int i=0; i<10; i++){ //3000/300-->10
		digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		delay(300);
	}
}

```
Link simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/2GGetlCbg5R?editbtn=1

Link simulazione su ESP32 con Wowki: https://wokwi.com/projects/349386232933909076

Nel codice possiamo isolare:
- i ```delay(500)```, tutti impostati sull tesso ritardo corrispondente al **tempo comune** tra le varie fasi dei due task, cioè il minimo comune multiplo tra il tempo della fase elementare del primo task e di quella corrispondente del secondo task. Si tratta di **momenti speciali** di grazia in cui entrambi i task si incontrano e svolgono le loro operazioni **contemporaneamente**.
- le **operazioni** di ogni task da svolgere insieme, perchè nello stesso istante, in ogni fase come ```digitalWrite(led2, HIGH)```. Non sempre devono essere presenti per entrambi i task dato che magari uno potrebbe rimanere inattivo in una fase elementare in cui è attivo l'altro. 
- una **trama temporale** che racchiude tutti i **tempi elementari** in un **periodo** che può essere **ripetuto uguale** tra un loop e l'altro. E' una **raccolta di fasi** in cui si danno appuntamento le varie operazioni elementari che compongono l'algoritmo all'interno di un loop().
- Si noti che, **in questa categoria di problemi**, le fasi dei due task **non si interlacciano** tra loro per cui è piuttosto **agevole** la loro definizione in **maniera separata** nel codice.

Variante che usa la capacità di iterare intrinseca della funzione loop():

```C++
/*
Realizzzare un programma che fa blinkare un led con periodo mezzo secondo per 5 sec e poi lo fa lo blinkare 
con periodo 300 msec per 3 sec, poi ricomincia d'accapo.
*/
byte led1 = 13;
int i;

void setup()
{
	pinMode(led1, OUTPUT);
	i=0;
}

void loop()
{
	// task 1
	if(i<10){
		digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		delay(500);
		i++;
	}
	// task 2
	else if(i<20){
		digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		delay(300);
		i++;
	}else{
		i=0;
	}
}
```

Link simulazione su Arduino con Tinkercad:https://www.tinkercad.com/embed/apuBTacgbFs?editbtn=1

Link simulazione su ESP32 con Wowki: https://wokwi.com/projects/349386232933909076

### **SCHEDULAZIONE CON I TIME TICK**

```C++
/*
Realizzzare un programma che fa blinkare un led con periodo mezzo secondo per 5 sec e poi lo fa 
lo blinkare con periodo 300 msec per 3 sec, poi ricomincia d'accapo.
*/
#define TBASE  100 // periodo base in milliseconds
#define NSTEP  1000  // numero di fasi massimo di un periodo generico
unsigned long precm = 0;
unsigned long step = 0;
byte led1 = 13;
bool stato;

void setup()
{
	pinMode(led1, OUTPUT);
	stato = true;
}

void loop()
{
	// polling della millis() alla ricerca del tempo in cui scade ogni periodo
	if((millis()-precm) >= (unsigned long) TBASE){ 		//se è passato un periodo tbase dal precedente periodo
		precm = millis();  				//preparo il tic successivo azzerando il conteggio del tempo ad adesso
		
		step = (step + 1) % NSTEP; 			// conteggio circolare (arriva al massimo a nstep-1)
	
		// task 1
		if(!(step%5)){  // schedulo eventi al multiplo del periodo (2 sec = 2 periodi)
			if(stato)
				digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		}
		// task 2
		if(!(step%3)){  // schedulo eventi al multiplo del periodo (2 sec = 2 periodi)
			if(!stato)
				digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		}

		if(step <= 50){  //0 < t < 500 msec
			stato = true;
		}else if(step <= 80){ //500 msec < t < 800 msec
			stato = false;// t >= 800 msec
		}else{
			step = 0;
			stato = true;
			//digitalWrite(led1,LOW);
		}
		// il codice eseguito al tempo del metronomo (esattamente un periodo) va quì
	}
	//delay(1);
	// il codice eseguito al tempo massimo della CPU va qui
}

```

Nel codice possiamo isolare:
- **il timer polled** com il quale viene calcolato il tempo base di riferimento dei vari task Iminimo comune multiplo del tempo minimo di ciascuno)
- il **contatore dei tick** ```step``` che realizza un metronomo su cui si sincronizzano i vari task: ```if((millis()-precm) >= (unsigned long) tbase){```
- le etichette dei **rilevatori di multiplo** come ```step%3``` che sincronizzano i vari task su un multiplo intero del tempo base prefissato
-  i **segnaposto delle fasi** in cui si sviluppa la vita di un singolo task (come ```step < 50```) che, non potendo più essere sviluppate in sequenza nel codice con lo stesso ordine che hanno nel tempo, adesso devono essere:
	- sincronizzate sul tempo base (con i rilevatori di multiplo)
	- riconosciute e trattate come eventi a cui assegnare una risposta appropriata (con il contatore ```count```)
- i **segnaposto** si possono sincronizzare su **contatori ausiliari** (variabili globali) o su **ingressi** o sul **risultato** di altre fasi.
- si noti il conteggio dello **sviluppo ad eventi** delle **fasi** dei singoli task che risulta più elaborato da pensare che nel caso dello **sviluppo sequenziale** con i delay perchè richiede l'introduzione di:
     - un **contatore** di **epoche** ```step``` che isola un **elenco di fasi** caratterizzate da un certo **stato** xhe ne influenza il **le azioni** correlate.
     - una **variabile di stato** ```stato``` che stabilisce il comportamento delle medesime **fasi** della vita di un task in **epoche** diverse.

Link simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/fn66P9O8oJG?editbtn=1

Link simulazione su ESP32 con Wowki: https://wokwi.com/projects/349383452039053908


### **SCHEDULAZIONE CON I THREAD**

```C++
/* 
Realizzzare un programma che fa blinkare un led con periodo mezzo secondo per 5 sec e poi lo fa 
lo blinkare con periodo 300 msec per 3 sec, poi ricomincia d'accapo.
*/
#include <pthread.h> //libreria di tipo preemptive
pthread_t t1;
pthread_t t2;
int delayTime ;
byte led1 = 13;
bool blink1, blink2;

void * blinkThread1(void * d)
{
  int time;
  time = (int) d;
  while(true){    						// Loop del thread	
	if(blink1){
		digitalWrite(led1, !digitalRead(led1));
		delay(time);
	}else{
		delay(10);
	}
}
  return NULL;
}

void * blinkThread2(void * d)
{
  int time;
  time = (int) d;
  while(true){    	
	if(blink2){
		digitalWrite(led1, !digitalRead(led1));
		delay(time);
	}else{
		delay(10);
	}
  }
  return NULL;
}

void setup() {
  pinMode(led1, OUTPUT);
  blink1 = true;
  blink2 = false;

  delayTime = 500;
  if (pthread_create(&t1, NULL, blinkThread1, (void *)delayTime)) {
         Serial.println("Errore crezione thread 1");
  }
  delayTime = 300;
  if (pthread_create(&t2, NULL, blinkThread2, (void *)delayTime)) {
         Serial.println("Errore crezione thread 2");
  } 
}

void loop() {
	digitalWrite(led1, LOW); //evita un difetto al primo loop
	delay(5000);
	blink1 = false;
	blink2 = true;
	delay(3000);
	blink1 = true;
	blink2 = false;
}
```

Link simulazione su ESP32 con Wowki: https://wokwi.com/projects/349388989705224787


### **SCHEDULAZIONE CON I THREAD 2 (CON DELAY)**

```C++
/* 
Realizzzare un programma che fa blinkare un led con periodo mezzo secondo per 5 sec e poi lo fa 
lo blinkare con periodo 300 msec per 3 sec, poi ricomincia d'accapo. 
Prevedere l'accensione e lo spegnimento della funzione mediante un tasto.
*/
#include <pthread.h> //libreria di tipo preemptive
pthread_t t1;
pthread_t t2;
int delayTime ;
byte led1 = 13;
byte stato= LOW;  // variabile globale che memorizza lo stato del pulsante
byte pulsante =12;

// attesa evento con tempo minimo di attesa
void waitUntilInputLow(int btn, unsigned t)
{
    while(!digitalRead(btn)==LOW){
    	delay(t);
    }
}

void * blinkThread1(void * d)
{
  int time;
  time = (int) d;
  while(true){    						// Loop del thread	
	if(stato){
		// task 1
		for(int i=0; i<10; i++){ //5000/500-->10
			digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
			delay(500);
		}
		// task 2
		for(int i=0; i<10; i++){ //3000/300-->10
			digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
			delay(300);
		}
	}else{
		digitalWrite(led1,LOW); 
		delay(10);
	}
}	
  return NULL;
}

void * btnThread(void * d){
	// Loop del thread
	while(true){
		if(digitalRead(pulsante)==HIGH){			// se è alto c'è stato un fronte di salita
			stato = !stato; 				// impostazione dello stato del toggle
			waitUntilInputLow(pulsante,50);			// attendi finchè non c'è fronte di discesa
			if(stato)
				Serial.println("stato ON");
			else
				Serial.println("stato OFF");
		}
		delay(10); 						// equivale a yeld() (10 per le simulazioni 0 in HW)
	}
}

void setup() {
	Serial.begin(115200);
  pinMode(led1, OUTPUT);
  pinMode(pulsante, INPUT);
  delayTime = 500;
  if (pthread_create(&t1, NULL, blinkThread1, (void *)delayTime)) {
         Serial.println("Errore crezione blinkThread1");
  }
  delayTime = 300;
  if (pthread_create(&t2, NULL, btnThread, (void *)delayTime)) {
         Serial.println("Errore crezione btnThread");
  } 
}

void loop() {
	delay(10);
}
```
Link simulazione su ESP32 con Wowki: https://wokwi.com/projects/349402186520724050

### **Considerazioni finali**

Da una analisi comparativa degli esempi proposti si possono trarre alcune considerazioni pratiche di ordine generale. 
- **eventi periodici consecutivi** si programmano **più semplicmente** con i **delay()** che, pur essendo bloccanti, godono della proprietà di poter pensare gli algoritmi in maniera **più intuitiva** e lineare una programmazione che, nello specifico di questa categoria di problemi, è **intrinsecamente sequenziale**.
- se gli eventi periodici in questione sono da eseguire in parallelo ad altro codice che deve essere responsivo, allora:
     - una soluzione potrebbe essere metterli in **thread separati** che pur essendo ancora programmabili in **maniera sequenziale** non monopolizzano la risorsa CPU permettendo l'**esecuzione parallela** sia del loop() che degli altri threads.
     - un'altra soluzione potrebbe essere programmare il solo **thread del loop()** principale utilizzando una schedulazione basata sui **time tick**. Il **difetto** di questa soluzione è che per, questa categoria di problemi, sembra effettivamente un tantino **più difficile** da concepire.




>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  
