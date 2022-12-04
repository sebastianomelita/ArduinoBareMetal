>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  


## **EVENTI PERIODICI CONTEMPORANEI**

## **SCHEDULAZIONE CON I DELAY**

Link simulazione Tinkercad: https://www.tinkercad.com/embed/7AsMjhD1Mk9?editbtn=1

```C++
/* 
Genera un blink periodico su un led e una segnalazione di SOS periodica su un altro
*/
// definizione dei pin di ciascun segmento RX
#define PDELAY  500
#define LDELAY  1500

byte led1 = 10;
byte led2 = 11;
byte led3 = 12;
byte led4 = 13;
unsigned long step=0;
byte x;

void setup()
{
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
}
 
void loop()
{
	// 3 punti
	digitalWrite(led1, HIGH); // +
	digitalWrite(led2, HIGH); // *
	delay(500);
	digitalWrite(led1, LOW);  // +
	delay(500);
	digitalWrite(led2, LOW);  // +
	digitalWrite(led1, HIGH); // *
	delay(500);
	digitalWrite(led1, LOW);  // +
	delay(500);
	digitalWrite(led1, HIGH); // +
	digitalWrite(led2, HIGH); // *
	delay(500);
	digitalWrite(led1, LOW);  // +
	delay(500);
	// 1 linea
	digitalWrite(led1, HIGH); // +
	digitalWrite(led2, LOW);  // *
	delay(1000);
	digitalWrite(led2, HIGH); // *
	delay(500);
	digitalWrite(led1, LOW);  // +
	delay(500);
	// 3 punti
	digitalWrite(led1, HIGH); // +	
	digitalWrite(led2, LOW);  // *
	delay(500);
	digitalWrite(led1, LOW);  // +                    
	delay(500);
	digitalWrite(led1, HIGH); // +
	digitalWrite(led2, HIGH); // *
	delay(500);				  
	digitalWrite(led1, LOW);  // +		
	delay(500);
	digitalWrite(led1, HIGH); // +						
	digitalWrite(led2, LOW);  // *
	delay(500);
	digitalWrite(led1, LOW);  // +
	delay(1000);											
	digitalWrite(led2, HIGH); // *
}	

```

## **SCHEDULAZIONE CON I TICK**

Di seguito il link della simulazione online con Tinkercad su Arduino: https://www.tinkercad.com/embed/81ioQDDGQOG?editbtn=1

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/348709453878526548

```C++
/* 
Genera un blink periodico su un led e una segnalazione di SOS periodica su un altro
*/
#define tbase  100  // periodo base in milliseconds
#define nstep  1000  // numero di fasi massimo di un periodo generico
#define PDELAY  500
#define LDELAY  1500
#define BLINKDELAY  300

unsigned long precm = 0;
unsigned long step = 0;
byte led1 = 10;
byte led2 = 11;
byte led3 = 12;
byte led4 = 13;
byte count = 0;


void setup()
{
	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
	pinMode(led3, OUTPUT);
	pinMode(led4, OUTPUT);
}

void loop()
{
	// polling della millis() alla ricerca del tempo in cui scade ogni periodo
	if((millis()-precm) >= (unsigned long) tbase){ 		//se è passato un periodo tbase dal precedente periodo
		precm = millis();  				//preparo il tic successivo azzerando il conteggio del tempo ad adesso
		
		step = (step + 1) % nstep; 			// conteggio circolare (arriva al massimo a nstep-1)

		// task blink
		if(!(step%3)){  // schedulo eventi al multiplo del periodo (300 msec = 3 periodi)
			digitalWrite(led2,!digitalRead(led2)); 	// stato alto: led blink
		}
		// task 2
		if(!(step%5)){  // schedulo eventi al multiplo del periodo (500 msec = 5 periodi)
			if(count < 6){
				digitalWrite(led1,!digitalRead(led1)); 			// led blink
			}else if(count < 9){
				digitalWrite(led1,HIGH); 				// linea
			}else if(count < 16){
				digitalWrite(led1,!digitalRead(led1)); 			// led blink
			}else if(count < 19){
				digitalWrite(led1,LOW); 				// spento
			}else{
				count = -1;
			}
			count++;
		}
		// il codice eseguito al tempo del metronomo (esattamente un periodo) va quì
	}
	// il codice eseguito al tempo massimo della CPU va qui
}

```

## **SCHEDULAZIONE CON I PROTOTHREAD**

Link simulazione su Arduino in Tinkercad: https://www.tinkercad.com/embed/2duYFwQzJgf?editbtn=1

Simulazione su Esp32 in Wowki: https://wokwi.com/projects/348709819084964435

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
//-----------------------------------------------------------------------------------------------------------
// se si usa questa libreria al posto delle macro sopra, togliere il commento iniziale all'include 
// e commentare le macro sopra
//#include "protothreads.h"
/* 
Genera un blink periodico su un led e una segnalazione di SOS periodica su un altro
*/
#define PDELAY  500
#define LDELAY  1500
#define BLINKDELAY  300
byte led1 = 10;
byte led2 = 11;
byte led3 = 12;
byte led4 = 13;

void setup()
{
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  PT_INIT(&ptSos);
  PT_INIT(&ptBlink);
}
 

// definizione protothread del pulsante
pt ptSos;
int SOSThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop del protothread
  while(true) {
	// 3 punti
	digitalWrite(led1, HIGH); // 1
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, LOW);
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, HIGH); // 2
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, LOW);
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, HIGH); // 3
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, LOW);
	PT_SLEEP(pt, PDELAY);
	// 1 linea
	digitalWrite(led1, HIGH); // 1 linea
	PT_SLEEP(pt, LDELAY);
	digitalWrite(led1, LOW);
	PT_SLEEP(pt, PDELAY);
	// 3 punti
	digitalWrite(led1, HIGH); // 1
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, LOW);
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, HIGH); // 2
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, LOW);
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, HIGH); // 3
	PT_SLEEP(pt, PDELAY);
	digitalWrite(led1, LOW);
	PT_SLEEP(pt, LDELAY);
  }
  PT_END(pt);
}

// definizione protothread del lampeggio
pt ptBlink;
int blinkThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop del protothread
  while(true) {
	digitalWrite(led2, HIGH);   	// turn the LED on (HIGH is the voltage level)
	PT_SLEEP(pt, BLINKDELAY);
	digitalWrite(led2, LOW);    	// turn the LED off by making the voltage LOW
	PT_SLEEP(pt, BLINKDELAY);
  }
  PT_END(pt);
}

// loop principale
void loop()
{
	PT_SCHEDULE(SOSThread(&ptSos)); 		// esecuzione schedulatore protothreads
	PT_SCHEDULE(blinkThread(&ptBlink)); 	// esecuzione schedulatore protothreads
}

```
## **SCHEDULAZIONE CON I THREAD**

Simulazione online su Esp32 con wowki: https://wokwi.com/projects/348755366576652882

```C++
/* 
Genera un blink periodico su un led e una segnalazione di SOS periodica su un altro
*/
#include <pthread.h> //libreria di tipo preemptive
pthread_t t1;
pthread_t t2;
int delayTime ;
#define PDELAY  500
#define LDELAY  1500
#define BLINKDELAY  300
byte led1 = 12;
byte led2 = 13;

void * sosThread(void * d)
{
  int time;
  time = (int) d;
  while(true){   			// Loop del thread
		// 3 punti
		digitalWrite(led1, HIGH); // 1
		delay(PDELAY);
		digitalWrite(led1, LOW);
		delay(PDELAY);
		digitalWrite(led1, HIGH); // 2
		delay(PDELAY);
		digitalWrite(led1, LOW);
		delay(PDELAY);
		digitalWrite(led1, HIGH); // 3
		delay(PDELAY);
		digitalWrite(led1, LOW);
		delay(PDELAY);
		// 1 linea
		digitalWrite(led1, HIGH); // 1 linea
		delay(LDELAY);
		digitalWrite(led1, LOW);
		delay(PDELAY);
		// 3 punti
		digitalWrite(led1, HIGH); // 1
		delay(PDELAY);
		digitalWrite(led1, LOW);
		delay(PDELAY);
		digitalWrite(led1, HIGH); // 2
		delay(PDELAY);
		digitalWrite(led1, LOW);
		delay(PDELAY);
		digitalWrite(led1, HIGH); // 3
		delay(PDELAY);
		digitalWrite(led1, LOW);
		delay(PDELAY);
		delay(LDELAY);						// delay bloccanti		
  }
}

void * blinkThread(void * d)
{
  int time;
  time = (int) d;
  while(true){    						// Loop del thread	
		digitalWrite(led2, !digitalRead(led2));
		delay(time);
  }
  return NULL;
}

void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  delayTime = PDELAY;
  if (pthread_create(&t1, NULL, sosThread, (void *)delayTime)) {
         Serial.println("Errore crezione thread 1");
  }
  delayTime = BLINKDELAY;
  if (pthread_create(&t2, NULL, blinkThread, (void *)delayTime)) {
         Serial.println("Errore crezione thread 2");
  } 
}

void loop() {
	delay(10);
}
```

## **Considerazioni finali**

Da una analisi comparativa degli esempi proposti si possono trarre alcune considerazioni pratiche di ordine generale. 
- **eventi periodici contemporanei** si programmano difficilmente con i delay() che, essendo bloccanti, non possono permettere l'esecuzione parallela di altri task all'interno dello stesso loop. I task con compiti diversi devono essere pensati come un unico task in cui le fasi di un compito si alternnano a quelle dell'altro nello stesso codice. In altre parole, bisogna spezzettare i due task in un certo numero di fasi elementari (una sorta di minimo comune multiplo dei compiti) che, una volta eseguite in sequenza tramite i delay(), realizzino contemporaneamente, interlacciandosi tra loro, i compiti di entrambi i task. Questa soluzione è praticabile per pochi task non eccessivamente complessi.
- una soluzione alternativa efficace potrebbe essere mettere i vari compiti in thread separati che pur essendo ancora programmabili in maniera sequnziale hanno almeno due grandi vantaggi:
     - non monopolizzano la risorsa CPU permettendo l'esecuzione parallela sia del loop() che di altri threads.
     - permettono la programmazione sequenziale dei task pensandoli isolati l'uno dall'altro, cioè si possono programmare in maniera indipendente conme se girassero a solo nel loop() principale.
- un'altra soluzione potrebbe essere programmare il solo thread del loop() principale utilizzando una schedulazione basata sui time tick. Questa soluzione, nel contesto di queta categoria di problemi (task contemporanei), appare vantaggiosa perchè una programmazione basata sull'esecuzione di una serie di compiti in istanti fissati e pianificati senza delay permette di poterli trattare in maniera indipendente con più semplicità. Rimane il problema della maggiore complessità della gestione della comunicazione tra i vari task che non è più sequenziale.

**Sitografia:**
- https://9g.lt/blog/protothreads-on-tinkercad

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  
