>[Torna all'indice generazione tempi](indexgenerazionetempi.md) >[versione in C++](timersched.md)

## **SCHEDULAZIONE CON I TIMERS HW**

La **schedulazione dei task** normalmente riguarda quei compiti che devono essere ripetuti in **maniera periodica**, infatti si parla di **loop() principale** e di **loop secondari** eventualmente sullo stesso thread (**protothread**) o su altri **thread**. Lo stesso scopo può essere ottenuto con dei timer HW che realizzano un loop su **base evento**. L'**evento** è l'**interrupt** di un timer HW, il **loop** è rappresentato dalla **calback** associata al timer e che viene viene da esso **periodicamente richiamata**.

Ma un **timer** si può impostare per **generare**:
- **eventi sincroni** cioè **periodici** dopo intervalli fissi e programmabili
- **eventi asincroni** cioè **aperiodici** da eseguire una sola volta (monostabili o one-shot) dopo un certo tempo calcolato **nel futuro** a partire dal **momento** in cui avviene il comando di **attivazione** (start).

Per cui un **timer HW** offre una versatilità in più se, nello stesso progetto, si vogliono **combinare insieme** eventi periodici con eventi aperiodici triggerati da eventi su un **ingresso** o da altri **eventi** (scadenza di timeout o contatori).

La **stessa cosa** è in realtà possibile realizzarla anche con i **timer SW** basati sul polling nel loop principale della funzione millis(). La loro **versatilità** è uguale se non **superiore**, ma sono soggetti ad alcune limitazioni che potrebbero renderli non adatti in certi contesti. L'ambito che **penalizza** di più i timer SW è quello delle applicazioni **mission-critical** (o **critical-time**) in cui si deve prevedere con certezza della puntualità l'esecuzione di certi compiti pena l'**instabilità** del sistema o una sua **perdita di sicurezza**.


### **I TIMERS HW DI ESP32**

Il microcontrollore **esp32** ha 4 **timer HW**. Tutti i timer sono divisi in **2 gruppi**, 2 timer in ciascun gruppo. **Ogni timer** HW può avere una propria **configurazione indipendente**.

Sebbene siano disponibili **timer software** di FreeRTOS (il sistema operativo di ESP32) questi timer presentano alcune limitazioni:
- La **risoluzione massima** è uguale al periodo di tick di RTOS
- Le **callback** del timer vengono lanciate da un task a **bassa priorità**

Similmente ad Arduino, ESP32 permette l'accesso diretto ai timer HW in almeno tre modi:
- accesso ai registri HW del timer per impostare il prescaler
- utilizzo di una libreria di sistema che astrae e semplifica le operazioni del punto precedente
- attraverso librerie di terza parti

I **timer hardware** però, sebbene siano liberi da entrambe le limitazioni precedenti, spesso sono **meno convenienti** da usare. Ad esempio, i componenti dell'applicazione potrebbero richiedere che gli eventi del timer si attivino in determinati **momenti nel futuro**, ma il timer hardware contiene un **unico** valore di "confronto" utilizzato per la generazione di un interrupt. 

Ciò significa che è necessario costruire una **astrazione** in cima ai timer hardware che, nonostante utilizzi ancora il meccanismo delle interruzioni, implementi: 
-  **funzionalità** per gestire l'**elenco** degli **eventi in sospeso** e per richiamare le **callback** per questi eventi man mano che si verificano le interruzioni hardware corrispondenti.
-  una **semplificazione logica** del timer HW (un **timer logico**) che si possa **instanziare molte volte**, ognuna associandola ad una **propria callback**. I timer HW sottostanti rimangono però sempre e soltanto 4.

In ESP32, le **callback** del timer possono essere inviate con due metodi:
- ```ESP_TIMER_TASK```. Le callback del timer vengono inviati da un unico **task** ```esp_timer``` ad **alta priorità**. Poiché tutte le callback vengono smistate dallo **stesso task**, si consiglia di eseguire solo la **minima** quantità di lavoro possibile all'interno di una stessa callback. Se sono in esecuzione altri task con **priorità superiore** a ```esp_timer```, l'invio della callback verrà ritardato fino a quando l'attività esp_timer non avrà la possibilità di essere eseguita.
- ```ESP_TIMER_ISR```. Le callback del timer vengono inviati direttamente dal gestore di interrupt del timer. Questo metodo è utile per alcune semplici richiamate che mirano a una latenza inferiore.



Esempio di dichiarazione e instanziazione di un **oggetto timer**:
```python
from machine import Timer 
```
Esempio delle due modalità di impostazione:
```python
# Create physical timers 
tim1 = Timer(1)
tim1.init(mode=Timer.PERIODIC, period=100, callback=mycallback)
tim2 = Timer(2, mode=Timer.PERIODIC, period=500, callback=mycallback)
tim3 = Timer(3, mode=Timer.ONE_SHOT)
# Create a virtual timer with period 500ms
tim = Timer(-1)
tim.init(period=500, callback = mycallback)
```

La funzione ```Timer.deinit()``` può essere utilizzata per rimuovere un timer dal sistema. Ciò è utile se si desidera arrestare un timer attualmente in esecuzione. Se ci sono molti timer che utilizzano ciascuno la memoria, è una buona pratica rimuoverli quando il loro compito è terminato.


Esempio completo:
```python
import time
from machine import Pin, Timer

def blink(led):
     led.value(not led.value())

def periodicPrint(t):
    global executionsCount
    global led
    print("printing in periodic function. Exec nr: ", (executionsCount+1));
    executionsCount += 1
    blink(led)

    if executionsCount >= maxExecutionsCount:
        tim.deinit()

led = Pin(13, Pin.OUT)
tim = Timer(-1)
tim.init(period=500, callback = periodicPrint)	
maxExecutionsCount = 10
executionsCount = 0

while True:
    time.sleep_ms(1)
```
Simulazione su Esp32 con Wowki: https://wokwi.com/projects/371694109619973121

L'esp_timer è un set di APIs che fornisce timer one-shot e periodici, con risoluzione di tempo dei microsecondo e 64-bit di range di conteggio. In ogni caso è bene tenere presente che:
- la libreria Ticker fornisce però la precisione del millisecondo quindi se si ha bisogno di qualcosa di più granulare, è utile sapere che le funzioni sottostanti sono più flessibili.
- le funzioni di callback non è detto che vengano eseguite immediatamente quando si attiva il timer hardware. L'effettiva implementazione dell'API IDF esegue la chiamata annidata di una funzione ausiliaria che potrebbe ritornare con un certo ritardo.
-  se si registrano callback multiple per uno stesso timer, maggiore sarà il ritardo per le callback che verranno chiamate dopo, dato che per essere eseguite loro devono necessariamente ritornare le precedenti.
- le callback sono chiamate da funzioni ISR lanciate da segnali di interrupt provenienti dai timer. Le ISR di norma dovrebbero essere molto brevi e, in ogni caso, **mai bloccanti**, per evitare instabilità del sistema. Invece di eseguire operazioni bloccanti nella callback del ticker è consigliabile impostare li un **flag**  e **controllare** quel flag all'interno della funzione **loop**.

```python
import time
from machine import Pin, Timer

def blink(led):
    led.value(not led.value())

led1 = Pin(12, Pin.OUT)
led2 = Pin(18, Pin.OUT)
tim1 = Timer(2)
tim1.init(period=500, callback = lambda t: blink(led1))	
tim2 = Timer(3)
tim2.init(period=1000, callback = lambda t: blink(led2))	
count = 0

while True:
    print("Doing stuff... ", count)
    count += 1
    if count >= 10:
        break
    time.sleep_ms(1000)

print("Ending timers...")
tim1.deinit()
tim2.deinit()
```

Simulazione su Esp32 con Wowki: https://wokwi.com/projects/371695217789720577

### **I TIMERS HW DI ARDUINO**

Arduino permette l'accesso diretto ai suoi **timer HW** in almeno **due modi**:
- accesso ai registri HW del timer per impostare il prescaler
- attraverso librerie di terza parti

Le librerie utilizzate di seguito però **non** permettono la realizzazione di **timer logici** ciascuno con una **prpopria callback** e tutti **associati** ad uno stesso **timer HW**, per cui è necessario associare **un task alla volta** ad ogni timer HW utilizzabile nel sistema in uso (ad esempio, 2 in Arduino Uno e 4 in Arduino Mega). Queste limitazioni rendono l'utilizzo esteso dei timer HW come schedulatori di compiti abbastanza problematico.

Una **soluzione** potrebbe essere inserire all'interno della **callback** di un timer HW uno schedulatore di compiti con cui poter realizzare il **filtraggio** degli **eventi** da eseguire **nel futuro** come quelli visti nelle sezioni [Schedulatore di compiti basato sul polling della millis()](tasksched.md) oppure utilizzando librerie di **terze parti** come [SimpleTimer](https://github.com/marcelloromani/Arduino-SimpleTimer/tree/master/SimpleTimer) (https://github.com/marcelloromani/Arduino-SimpleTimer/tree/master/SimpleTimer) oppure quella presentata nell'esempio proposto nel paragrafo sugli scheduler [time tick scheduler](tasksched.md)

Di seguito è riportato un esempio in cui due task che realizzano un blink sono affidati a due timers HW diversi che realizzano una schedulazione la cui tempistica non è per nulla influenzata dai delay nel loop (interrotti da interrupt) ma è li regolata dai comandi di disabilitazione ```detachInterrupt()```.

Arduino con la libreria TimerInterrupt https://github.com/khoih-prog/TimerInterrupt

```C++
#define TIMER_INTERRUPT_DEBUG         0
#define USING_16MHZ     true
#define USING_8MHZ      false
#define USING_250KHZ    false

#define USE_TIMER_0     false
#define USE_TIMER_1     true
#define USE_TIMER_2     true
#define USE_TIMER_3     false

#include "TimerInterrupt.h"

int led1 = 13;
int led2 = 12;
 
void periodicBlink(int led) {
  Serial.print("printing periodic blink led ");
  Serial.println(led);

  digitalWrite(led, !digitalRead(led));
}
 
void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  Serial.begin(115200); 
  // Select Timer 1-2 for UNO, 0-5 for MEGA
  // Timer 2 is 8-bit timer, only for higher frequency
  ITimer1.init();
  ITimer1.attachInterruptInterval(500, periodicBlink,led1);
  // Select Timer 1-2 for UNO, 0-5 for MEGA
  // Timer 2 is 8-bit timer, only for higher frequency
  ITimer2.init();
  ITimer2.attachInterruptInterval(1000, periodicBlink,led2);
}
 
void loop() {
	int count = 0;
	while(true){
		Serial.print("Doing stuff... ");
		Serial.println(count);
		count += 1;
		if(count >= 10)
		    break;
		delay(1000);
	}
	Serial.print("Ending timers...");
	ITimer1.detachInterrupt();
	ITimer2.detachInterrupt();
}
```

### **TIMERS HW DI ARDUINO SCHEDULATO CON IL POLLING DEI MILLIS**

Si tratta della stessa situazione dell'esempio precedente soltanto che adesso c'è un task in più mentre i timer HW a disposizione sono ancora soltanto due. I task complessivamente in esecuzione sono quattro:
- **uno** in esecuzione **nel loop** schedulato da un delay() casuale che simula task pesanti dalla durata impredicibile
- **uno** affidato ad un **proprio timer HW** che ne programma l'esecuzione ad intervalli precisi, eventualmente sottraendo l'esecuzione al task nel loop mediante un segnale di interrupt
- **due** affidati ad un **unico timer HW** condiviso che esegue ad intervalli di tempo precisi uno schedulatore SW basato sul polling della funzione millis. Lo schedulatore viene richiamato in intervalli di tempo **comuni** ai due task che poi vengono **filtrati** mediante dei **timer SW**.

```C++
#define TIMER_INTERRUPT_DEBUG         0
#define USING_16MHZ     true
#define USING_8MHZ      false
#define USING_250KHZ    false

#define USE_TIMER_0     false
#define USE_TIMER_1     true
#define USE_TIMER_2     true
#define USE_TIMER_3     false

#include "TimerInterrupt.h"
int led1 = 13;
int led2 = 12;
int led3 = 11;
int led4 = 10;
unsigned long period[2];
volatile unsigned long precs[2];

void periodicBlink(int led);
void schedule();

void schedule()
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
 
void periodicBlink(int led) {
  Serial.print("printing periodic blink led ");
  Serial.println(led);

  digitalWrite(led, !digitalRead(led));
}
 
void setup() {
	//randomSeed(millis());
	randomSeed(analogRead(0));
	precs[0]=0;
	precs[1]=0;
	period[0] = 300;
	period[1] = 500;
	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
	pinMode(led3, OUTPUT);
	pinMode(led4, OUTPUT);
	Serial.begin(115200); 
	// Select Timer 1-2 for UNO, 0-5 for MEGA
	// Timer 2 is 8-bit timer, only for higher frequency
	ITimer1.init();
	ITimer1.attachInterruptInterval(100, schedule);
	// Select Timer 1-2 for UNO, 0-5 for MEGA
	// Timer 2 is 8-bit timer, only for higher frequency
	ITimer2.init();
	ITimer2.attachInterruptInterval(1000, periodicBlink,led3);
}
 
void loop() {
	unsigned randomDelay = random(10, 2000);
	Serial.print("delay: ");Serial.println(randomDelay);
	delay(randomDelay);
	digitalWrite(led4, !digitalRead(led4));
}
```

### **TIMERS HW DI ARDUINO SCHEDULATO CON TIMES TICK**

Si tratta della stessa situazione dell'esempio precedente in cui ci stanno **tre task** da eseguire con precisione e soltanto **due timer HW** per farlo. I **task** complessivamente in esecuzione sono **quattro**:
- **uno** in esecuzione **nel loop** schedulato da un delay() casuale che simula task pesanti dalla durata impredicibile
- **uno** affidato ad un **proprio timer HW** che ne programma l'esecuzione ad intervalli precisi, eventualmente sottraendo l'esecuzione al task nel loop mediante un segnale di interrupt
- **due** affidati ad un **unico timer HW** condiviso che esegue ad intervalli di tempo precisi uno schedulatore SW basato sul polling della funzione millis. Lo schedulatore viene richiamato in intervalli di tempo **comuni** ai due task che poi vengono **filtrati** mediante conteggio e selezione dei **times tick**.


```C++
#define TIMER_INTERRUPT_DEBUG         0
#define USING_16MHZ     true
#define USING_8MHZ      false
#define USING_250KHZ    false

#define USE_TIMER_0     false
#define USE_TIMER_1     true
#define USE_TIMER_2     true
#define USE_TIMER_3     false

#include "TimerInterrupt.h"
int led1 = 13;
int led2 = 12;
int led3 = 11;
int led4 = 10;
unsigned long period, step, nstep;
volatile unsigned long prec;

void periodicBlink(int led);
void schedule();

void schedule()
{
	// polling della millis() alla ricerca del tempo in cui scade ogni periodo
	if((millis()-prec) >= period){ 		//se è passato un periodo tbase dal precedente periodo
		prec += period;  		//preparo il tic successivo azzerando il conteggio del tempo ad adesso
		step = (step + 1) % nstep; 	// conteggio circolare (arriva al massimo a nstep-1)

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
 
void periodicBlink(int led) {
  Serial.print("printing periodic blink led ");
  Serial.println(led);

  digitalWrite(led, !digitalRead(led));
}
 
void setup() {
	//randomSeed(millis());
	randomSeed(analogRead(0));
	prec=0;
	period = 100;
  step = 0;
	nstep = 100;
	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
	pinMode(led3, OUTPUT);
	pinMode(led4, OUTPUT);
	Serial.begin(115200); 
	// Select Timer 1-2 for UNO, 0-5 for MEGA
	// Timer 2 is 8-bit timer, only for higher frequency
	ITimer1.init();
	ITimer1.attachInterruptInterval(100, schedule);
	// Select Timer 1-2 for UNO, 0-5 for MEGA
	// Timer 2 is 8-bit timer, only for higher frequency
	ITimer2.init();
	ITimer2.attachInterruptInterval(1000, periodicBlink,led3);
}
 
void loop() {
	unsigned randomDelay = random(10, 2000);
	Serial.print("delay: ");Serial.println(randomDelay);
	delay(randomDelay);
	digitalWrite(led4, !digitalRead(led4));
}
```

Simulazione su Arduino con Wowki: https://wokwi.com/projects/352009022804591183

### **Sitografia**

- https://www.coderdojotc.org/micropython/advanced-labs/13-timers/
- https://docs.micropython.org/en/latest/library/machine.Timer.html

>[Torna all'indice generazione tempi](indexgenerazionetempi.md) >[versione in C++](timersched.md) 
