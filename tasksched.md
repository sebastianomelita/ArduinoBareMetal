>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in Python](taskschedpy.md)

## **SCHEDULATORE DI COMPITI BASATO SUL POLLING DELLA FUNZIONE MILLIS**

Una categoria di **schedulatori di compiti** che, a differenza di quelli basati sulla ```delay()```, possiede la peculiare proprietà di **non bloccare** l'esecuzione degli atri task del ```loop()``` si può ottenere realizzando,  all'interno del ```loop()```, il **polling periodico** della funzione ```millis()```.

Il **polling** serve a stabilire in **quale ciclo** di loop la funzione millis() assume il valore di **tempo limite** oltre il quale **eseguire** un certo compito.

Gli **schedulatori** di questo tipo sono **periodici** e si possono realizzare sostanzialmente in **due modi**:
- mediante ripetizione di una **sequenza di tick** periodici che fissano il **tempo base**. Un seccessivo **conteggio** o **filtraggio** dei tick discrimina il **momento** in cui deve essere eseguito **un task** piuttosto che un altro.
- mediante **timer periodici** che, per **ogni task**, stabiliscono un **timeout** oltre il quale il **polling** della funzione millis() **segnala** il momento buono per **eseguire** quel task.

E’ buona norma evitare l’esecuzione frequente di **operazioni lente** quando queste non sono strettamente necessarie in modo da lasciare spazio ad altre operazioni, quali ad esempio gestione degli eventi di input, che richiedono una velocità maggiore per usufruirne in modo più interattivo.

**In particolare**, all'interno di un task, andrebbe accuratamente evitata l'introduzione di ritardi prolungati tramite **delay()**. Possono esserci per **breve tempo** (qualche mSec) ma, in ogni caso, non dovrebbero mai far parte della maniera (**algoritmo**) con cui la logica di comando **pianifica i suoi eventi**.

Utilizzando la tecnica della **schedulazione esplicita dei task** nel loop(), la **realizzazione di un algoritmo** non dovrebbe essere pensata in **maniera sequenziale (o lineare)** ma, piuttosto, come una **successione di eventi** a cui corrispondono dei **compiti** (task) che **comunicano** con le periferiche di input/output ed, eventualmente, dialogano tra loro in qualche modo (ad es. variabili globali, buffer, ecc.).

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

### **SCHEDULATORE DI COMPITI BASATO SU FILTRAGGIO DEI TIME TICK**

Uno schedulatore di compiti (task) si può realizzare anche utilizzando **più timers** basati sul polling della funzione millis(). 

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
	precs[0] = 0;
	precs[1] = 0;
	period[0] = 500;
	period[1] = 1000;
	precm = 0;
}

void loop()
{
	unsigned long currTime = millis();
	if(currTime-precm >= tbase){ 	
		precm += tbase;  			
		// task 1
		if ((currTime - precs[0]) >= period[0]) {
			precs[0] += period[0]; 
				digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		}	
		// task 2
		if ((currTime - precs[1]) >= period[1]) {
			precs[1] += period[1]; 
				digitalWrite(led2,!digitalRead(led2)); 	// stato alto: led blink
		}
	}
	// il codice eseguito al tempo massimo della CPU va qui
}
```

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/352691213474403329

### **SCHEDULATORE DI COMPITI GENERICO SENZA MILLIS**

E' possibile realizzare uno schedulatore di task che non necessita di alcuna funzione di misura del tempo (delay() o millis()). Esso si basa sull'**invocazione periodica** di una funzione ad un **tempo base** comune a tutti i task, calcolato col **massimo comune divisore** (M.C.D. o G.C.D) di tutti i tempi dei vari task. Un **contatore** di tempi base determina, per ogni task, il momento buono in cui questo deve essere eseguito **resettando** il proprio contatore subito dopo.

La **base dei tempi** comune può essere realizzata mediante qualunque tecnica di **generazione di tempi periodici**, ma la particolarità dell'assenza di misure di **tempo corrente** (millis()) rendono la realizzazione particolarmente adatta ad essere adoperata su base dei tempi generata mediata **interrupt**. 

La **versione originale** più completa dello schedulatore insieme ad una dettagliata discussione teorica si trova in: https://www.ics.uci.edu/~givargis/pubs/C50.pdf.

```C++
#include <Ticker.h>

Ticker periodicTicker1;
byte led1 = 13;
byte led2 = 12;
unsigned long period[2];
unsigned long elapsedTime[2];
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
			elapsedTime[0] += period[0]; 
			digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
			elapsedTime[0] = 0;
		}
		elapsedTime[0] += tbase;
		// task 2
		if (elapsedTime[1] >= period[1]) {
			elapsedTime[1] += period[1]; 
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

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/352766239477208065

## **Esempi**

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

void epoch10sec(){
	scheduler.disableEvent(1,1000);
	scheduler.disableEvent(1,500);
	digitalWrite(led1, LOW);
	digitalWrite(led2, LOW);
	Serial.println("Ending timers...");
	Serial.print("Frized... ");
}

void epoch20sec(){
	Serial.print("Enable all... ");
	Serial.println(count);
	scheduler.enableEvent(1,1000);
	scheduler.enableEvent(1,500);
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
	scheduler.addPeriodicEvent(epoch10sec, 1, 10000);
	scheduler.addPeriodicEvent(epoch20sec, 1, 20000);
	scheduler.addPeriodicEvent(onSecondEvents, 1, 1000);
	Serial.println(F("Scheduler init"));
	Serial.print(F("Time base: "));
	Serial.println(scheduler.getTimebase());
	Serial.print(F("Nsteps: "));
	Serial.println(scheduler.getNsteps());
}
 
void loop() {
	scheduler.scheduleAll();
	delay(10);
}
```

Di seguito il link della simulazione online con Tinkercad su Arduino: https://wokwi.com/projects/352057010320512001

### **Sitografia:**

- https://github.com/Koepel/Fun_with_millis#fun_with_millis
- http://arduino.cc/forum/index.php/topic,124048.msg932592.html#msg932592
- https://github.com/marcelloromani/Arduino-SimpleTimer/tree/master/SimpleTimer
- https://github.com/sebastianomelita/time-tick-scheduler
- https://www.ics.uci.edu/~givargis/pubs/C50.pdf
- https://www.cs.ucr.edu/~vahid/rios/

				      
>[Torna all'indice generazione tempi](indexgenerazionetempi.md)>[Versione in Python](taskschedpy.md)
