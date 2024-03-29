>[Torna all'indice generazione tempi](indexgenerazionetempi.md) >[versione in python](timerschedpy.md)

## **SCHEDULAZIONE CON I TIMERS HW**


La **schedulazione dei task** normalmente riguarda quei compiti che devono essere ripetuti in **maniera periodica**, infatti si parla di **loop() principale** e di **loop secondari** eventualmente sullo stesso thread (**protothread** o mediante costrutti **async/await**) o su altri **thread**. Lo stesso risultato può essere ottenuto con dei timer HW che realizzano un loop su **base evento**. L'**evento** è l'**interrupt** di un timer HW, il **loop** è rappresentato dalla **calback** associata al timer e che viene viene da esso **periodicamente richiamata**.

Ma un **timer** si può impostare per **generare**:
- **eventi sincroni** cioè **periodici** dopo intervalli fissi e programmabili
- **eventi asincroni** cioè **aperiodici** da eseguire una sola volta (monostabili o one-shot) dopo un certo tempo calcolato **nel futuro** a partire dal **momento** in cui avviene il comando di **attivazione** (start).

Per cui un **timer HW** offre una versatilità in più se, nello stesso progetto, si vogliono **combinare insieme** eventi periodici con eventi aperiodici triggerati da eventi su un **ingresso** o da altri **eventi** (scadenza di timeout o contatori).

La **stessa cosa** è in realtà possibile realizzarla anche con i **timer SW** basati sul polling nel loop principale della funzione millis(). La loro **versatilità** è uguale se non **superiore**, ma sono soggetti ad alcune limitazioni che potrebbero renderli non adatti in certi contesti. L'ambito che **penalizza** di più i timer SW è quello delle applicazioni **mission-critical** (o **critical-time**). In questo tipo di applicazioni si deve prevedere che l'esecuzione di certi compiti avvenga in maniera estremamente puntuale, pena l'introduzione di **instabilità** nel sistema o di **perdita di sicurezza** per chi lo adopera.

Rispetto agli altri metodi di creazione di base dei tempi (polling della millis(), thread e protothread), è tendenzialmente più legato ad uno specifico vendor di HW e ad una specifica linea di prodotti. Le **API dei timer**, pur esendo **molto simili** tra loro, **non sono standardizzate** e la **portabilità** del SW nel tempo potrebbe non essere garantita. In ogni caso **semplificano** parecchio la **gestione delle ISR** associate a timer HW che altrimenti, eseguita a basso livello, richiede una impostazione di **registri interni** della CPU che necessita di conoscenze di dettaglio molto specifiche.

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
- ```ESP_TIMER_ISR```. Le callback del timer vengono inviati direttamente dal gestore di interrupt del timer. Questo metodo è utile per alcune semplici callback che mirano a una latenza inferiore.

La **libreria Ticker** è un **wrapper (involucro)** delle'**API ```esp_timer```** che si **interfaccia direttamente** con l'**HW** secondo il metodo ```ESP_TIMER_TASK```. Si adopera con le **stesse API** su due piattaforme molto diverse (**ESP8266** e **ESP32**) ma entrambe dotate di interfaccia WiFi. Il FW di bordo nel caso di ESP8266 è LWIP ed è essenzialmente uno stack TCP/IP minimale per sistemi embedded. Il FW in uso su ESP32 si chiama IDF. Entrambi forniscono librerie per la gestione di timer HW. Nel caso di **ESP8266** si chiama **os_timer**, nel caso di **ESP32** il nome della libreria è **esp_timer**. 

Il motivo dell'**interfaccia comune** è che si tratta di prodotti molto diffusi della stessa casa produttrice ai quali si vuole fornire una interfaccia di utilizzo uniforme per la gestione dei timer. **Funzioni simili** sono implementate in quasi tutti i **sistemi embedded**, Arduino compreso, o tramite **librerie di terze parti** o direttamente all'interno del **core** del framework di gestione dell'HW (librerie o OS). 

### **Modalità di utilizzo**

La **modalità di utilizzo** è simile a quella di una **usuale ISR**. Una **funzione di callback** viene associata ad un evento di un timer tramite un metodo **attach()**. La dissociazione si fa con il metodo contrario **detach()**. Gli **eventi possibili** sono una chiamata una tantum (**one shot** o timer monostabile) o una chiamata **periodica** o timer bistabile. Nella definizione dell'attach() viene anche impostato il **tempo di scadenza** del timer.  Gli **eventi possibili** sono:
- una chiamata una tantum (**one shot** o timer monostabile)
- una chiamata **periodica** o timer bistabile.  

I **timer HW** possono essere:
- **fisici**. Cioè **limitati in numero** ai dispositivi HW dedicati a questo scopo effettivamente presenti nel sistema (nell'ESP32 sono 4) e inoltre devono essere riferiti esplicitamente nel codice con un loro specifico id (un numero da 1 a 4).
- **logici o virtuali**. Sono virtualmente di **numero infinito** e rappresentano delle **astrazioni SW** del sottostante HW fisico che è condiviso in qualche modo da uno **strato di gestione** trasparente al programmatore. 

La possibilità di poter **instanziare** un **timer logico** per **ogni task**, a sua volta definito all'interno di una certa **callback**, rende l'utilizzo dei timer una strada **effettivamente percorribile** per la realizzazione di uno **schedulatore di compiti** sia periodici che one shot (esecuzione singola non ripetuta). La **schedulazione** dei compiti inoltre rimane **molto precisa** perchè collegata a ISR eseguite da segnali di interrupt lanciati da **timer fisici**.

Si ricorda che in C e in C++ le variabili globali a comune tra ISR e loop() principale e suscettibili di modifiche da parte di entrambi andrebbero dichiarate tutte con il qualificatore ```volatile```.

### **Limitazioni nell'utilizzo**

I timer richiamano particolari funzioni di callback , le ISR, che sono attivate a seguito di  un interrupt HW (timeout del timer). Nei sistemi multi-core questi interrupt possono essere chiamati solo dal core che ha eseguito il timer. Ciò significa che se si utilizza un timer per attivare un interrupt, è necessario assicurarsi che l'interrupt venga chiamato solo dal core che sta eseguendo il timer.

le ISR di base **non** sono **interrompibili** a meno di impostare apposite istruzioni che lo consentono. **Interrompere una ISR** potrebbe causare inconsistenze nella gestione delle risorse condivise con il loop o altri thread. D'altra parte, una ISR che non si interrompe per molto tempo impedisce la tempestiva esecuzione delle altre ISR dovute ad interruzioni **simultanee** o **temporalmente vicine**.

Esistono **limitazioni speciali** su ciò che può e non può essere fatto all'interno delle ISR nella maggior parte dei controllori: 
- Ad esempio, non è consentito allocare **memoria dinamica** all'interno di una ISR. 
- una ISR che esegue una **logica complessa** potrebbe essere eseguita così lentamente da creare instabilità del sistema dovuta al fatto che altre interruzioni, che gestiscono servizi essenziali del sistema, non sono state prontamente soddisfatte. Un gestore di interrupt dovrebbe essere sempre una funzione **breve** che esegue il **lavoro minimo** necessario per **modificare** delle **variabili esterne**.
- In genere, in molte implementazioni, callback diverse di uno stesso timer vengono eseguite **in sequenza** e non su thread paralleli per cui operazioni bloccanti come le ```delay()```, oltre a causare possibili **instabilità** (sono ISR basate su interrupt), **ritardano** l'esecuzione delle callback **a seguire**.
- eseguire **task complessi** con un timer HW è possibile a patto che questi vengano resi interrompibili senza creare problemi, e ciò si può ottenere eseguendoli in un **altro thread** o nel **loop principale**. Nel **loop principale**, un **task complesso** può sempre essere immediatamente attivato da una ISR che asserisce un opportuno **flag di avvio**.

### **Timers SW vs Timer HW**

Se con la funzione ```get()``` di un **timer SW** si desiderasse controllare una **scadenza**, cioè vedere se è passato il tempo necessario per compiere una certa **azione** (modificare una variabile o chiamare una funzione), allora si dovrebbero soddisfare **due esigenze**:
- verificare la verità della condizione ```get() > timeout``` ed eseguire, eventualmente, l'azione prevista al timeout (scadenza) del timer.
- eseguire la valutazione al punto precedente **periodicamente**, finchè essa non accade.

I **due requisiti** precedenti si traducono nell'azione di eseguire il **polling** della funzione ```get()``` nel ```loop()```, aLla massima velocità o, più lentamente, ogni tot millisecondi. Nella vita reale, si può assimilare ad un polling della ```get()``` l'**osservazione periodica** di un orologio a muro effettuata con lo scopo di individuare il momento esatto in cui deve essere eseguita una certa azione, ad esempio, l'estrazione dal forno di un ciambella.

Se si volesse fare la stessa cosa con un **timer HW** allora ci si renderebbe conto che il polling non è più necessario perchè, **al timeout**, attraverso, un **segnale** proveniente dal timer HW,  viene attivato l'**ISR** associata a quel segnale che, a sua volta, comanda l'esecuzione di una **callback** definita al suo interno. Per rimanere alla metafora precedente, adesso non è più necessario osservare periodicamente l'orologio alla parete, perchè un timer, impostato ad inizio cottura, avviserà con un segnale acustico il pasticciere quando il momento di togliere il dolce dal forno sarà arrivato . 

### **I TIMERS VIRTUALI DI ESP32**

E' una libreria che permette di allocare un numero arbitrario di timers virtuali a partire dal numero presente nelle varie versioni di ESP32 (tipicamente sono 4 timer).

Esempio di dichiarazione e instanziazione di un **oggetto timer**:
```C++
Ticker periodicTicker;
```
Esempio delle due modalità di impostazione:
```C++
periodicTicker.attach_ms(5000, periodicPrint);
onceTicker.once_ms(5000, oncePrint);
```
Esempio completo:
```C++
#include <Ticker.h>
 
Ticker periodicTicker;
int executionsCount = 0;
 
void periodicPrint(int maxExecutionsCount) {
  Serial.print("printing in periodic function. Exec nr: ");
  Serial.println(executionsCount+1);
 
  executionsCount++;
 
  if(executionsCount>=maxExecutionsCount){
    periodicTicker.detach();
  }
}
 
void setup() {
  Serial.begin(115200);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  int maxExecutionsCount=10;
   
  periodicTicker.attach_ms(5000, periodicPrint, maxExecutionsCount);
}
 
void loop() {}
```
Simulazione su Esp32 con Wowki: https://wokwi.com/projects/348968576484377172

L'esp_timer è un set di APIs che fornisce timer one-shot e periodici, con risoluzione di tempo dei microsecondo e 64-bit di range di conteggio. In ogni caso è bene tenere presente che:
- la libreria Ticker fornisce però la precisione del millisecondo quindi se si ha bisogno di qualcosa di più granulare, è utile sapere che le funzioni sottostanti sono più flessibili.
- le funzioni di callback non è detto che vengano eseguite immediatamente quando si attiva il timer hardware. L'effettiva implementazione dell'API IDF esegue la chiamata annidata di una funzione ausiliaria che potrebbe ritornare con un certo ritardo.
-  se si registrano callback multiple per uno stesso timer, maggiore sarà il ritardo per le callback che verranno chiamate dopo, dato che per essere eseguite loro devono necessariamente ritornare le precedenti.
- le callback sono chiamate da funzioni ISR lanciate da segnali di interrupt provenienti dai timer. Le ISR di norma dovrebbero essere molto brevi e, in ogni caso, **mai bloccanti**, per evitare instabilità del sistema. Invece di eseguire operazioni bloccanti nella callback del ticker è consigliabile impostare li un **flag**  e **controllare** quel flag all'interno della funzione **loop**.

```C++
#include <Ticker.h>

Ticker periodicTicker1;
Ticker periodicTicker2;
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
  periodicTicker1.attach_ms(500, periodicBlink, led1);
  periodicTicker2.attach_ms(1000, periodicBlink, led2);
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
	periodicTicker1.detach();
	periodicTicker2.detach();
}
```

Simulazione su Esp32 con Wowki: https://wokwi.com/projects/348969741870694996

### **Semaforo con un timer HW**

Può essere realizzato efficacemente con **un solo timer** in quanto accade che ad ogni **transizione di colore** scade il timer della transizione corrente e, subito dopo, si attiva quello della transizione di colore **successiva**, per cui i tempi di attivazione dei timer coinvolti sono sempre consecutivi e quindi mai sovrapposti.

```C++
#include <Ticker.h>

Ticker onceTicker;
int led_verde = 13;
int led_rosso = 14;
int led_giallo = 12;
int tverde = 5000;
int tgiallo = 2000;
int trosso = 10000;
 
void verde_giallo() {
  digitalWrite(led_verde, LOW);
  digitalWrite(led_giallo, HIGH);
  onceTicker.once_ms(tgiallo, giallo_rossa);
}

void giallo_rossa() {
  digitalWrite(led_giallo, LOW);
  digitalWrite(led_rosso, HIGH);
  onceTicker.once_ms(trosso, rossa_verde);
}

void rossa_verde() {
  digitalWrite(led_rosso, LOW);
  digitalWrite(led_verde, HIGH);
  onceTicker.once_ms(tverde, verde_giallo);
}
 
void setup() {
  pinMode(led_verde, OUTPUT);
  pinMode(led_rosso, OUTPUT);
  pinMode(led_giallo, OUTPUT);
  digitalWrite(led_verde, HIGH);
  onceTicker.once_ms(tverde, verde_giallo );
}
 
void loop() {
  //int count = 0;
  delay(10);
}
```
Simulazione su Esp32 con Wowki: https://wokwi.com/projects/387700784888357889

### **I TIMERS HW DI ARDUINO**

Arduino permette l'accesso diretto ai suoi **timer HW** in almeno **due modi**:
- accesso ai registri HW del timer per impostare il prescaler
- attraverso librerie di terza parti

Le librerie utilizzate di seguito però **non** permettono la realizzazione di **timer logici** ciascuno con una **propria callback** e tutti **associati** ad uno stesso **timer HW**, per cui è necessario associare **un task alla volta** ad ogni timer HW utilizzabile nel sistema in uso (ad esempio, 2 in Arduino Uno e 4 in Arduino Mega). Queste limitazioni rendono l'utilizzo esteso dei timer HW come schedulatori di compiti abbastanza problematico.

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

### **PROBLEMA DEL NUMERO LIMITATO DI TIMER HW**

Esistono dei limiti nel **numero dei timer HW** a bordo di un sistema a microcontrollore. ESP32, ad esempio ne ha solo 4, virtualizzabili in un numero indefinito in C++, non virtualizzabili in python. Arduino, nelle varie versioni (come prortotipo di microcontrollori più semplici) ne ha un numero ridotto e non virtualizzabile.

Se i **task** da mandare in esecuzione **in parallelo** sono in numero maggiore dei **timer allocabili** (HW o virtuali) allora bisogna condividere un timer tra più task e per questo scopo si possono usare le solite tecniche di schedulazione che permettono, a fronte di un tempo comune (tempo base), di generare i **tempi propri** di ciascun task. Invocando lo schedulatore in corrispondenza del momento dello **scadere (elapsed)** di questi tempi, viene invocata la funzione (o il blocco di codice) del task.

<img src="schdulatore_generico.jpg" alt="alt text" width="1000">


### **TIMERS HW SCHEDULATI TRAMITE AGGIORNAMENTO DEL PERIODO DEL TASK**

Gli schedulatori utilizzati sono **due**:
- basato su https://www.ics.uci.edu/~givargis/pubs/C50.pdf e in https://www.cs.ucr.edu/~vahid/rios/
- basato su https://github.com/marcelloromani/Arduino-SimpleTimer/tree/master/SimpleTimer e su [Generazione di tempi assoluti](absolutetime.md)

Entrambi possono essere utilizzati a partire da una generazione di tempi costante (delay, millis(), timer HW). Per una dissertazione più accurata sul loro utilizzo vedi [Schedulatore di compiti basato sul polling della millis()](taskschedpy.md) 

```C++
#include <Ticker.h>

Ticker periodicTicker1;
Ticker periodicTicker2;
int led1 = 12;
int led2 = 14;
int led3 = 27;
int led4 = 5;
int led5 = 4;
int led6 = 2;
int leds1[] = {led1, led2, led3};
int leds2[] = {led4, led5};
//parametri dello sheduler 1
volatile unsigned long  precm = 0;
unsigned long  tbase1 = 500;
volatile unsigned long precs[]= {0, 0};
unsigned long period1[] = {1500, 6000};
//parametri dello sheduler 2
unsigned long elapsedTime[] = {0, 0, 0};
unsigned long period2[] = {500, 2000, 3000};
int tbase2 = 500;

void periodicBlink(int led) {
  digitalWrite(led, !digitalRead(led));
}

void scheduleAll(int *leds){
	// task 1
	if (elapsedTime[0] >= period2[0]) {
		periodicBlink(leds[0]);
		elapsedTime[0] = 0;
	}
	elapsedTime[0] += tbase2;
	// task 2
	if (elapsedTime[1] >= period2[1]) {
		periodicBlink(leds[1]);
		elapsedTime[1] = 0;
	}
	elapsedTime[1] += tbase2;
	// task 3
	if (elapsedTime[2] >= period2[2]) {
		periodicBlink(leds[2]);
		elapsedTime[2] = 0;
	}
	elapsedTime[2] += tbase2;
}

void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  pinMode(led6, OUTPUT);
  Serial.begin(115200); 
  //Inizializzazione dei task
  for(int i=0; i<2; i++){
	precs[i] = precm -period1[i];
  }
  //inizializzazione dello scheduler 2
  for(int i=0; i<3; i++){
	elapsedTime[i] = period2[i];
  }
  //configurazione timers HW
  periodicTicker1.attach_ms(500, scheduleAll, leds1);
  periodicTicker2.attach_ms(1000, periodicBlink, led6);
}
 
void loop() {
	delay(500);
	precm += tbase1;
	// task 1
	if ((precm - precs[0]) >= period1[0]) {
		precs[0] += period1[0]; 
    		periodicBlink(leds2[0]);
	}	
	// task 2
	if ((precm - precs[1]) >= period1[1]) {
		precs[1] += period1[1]; 
    		periodicBlink(leds2[1]);
	}
}
```
Simulazione su Arduino con Wowki: https://wokwi.com/projects/371810634603304961

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

Di seguito il link della simulazione online con Tinkercad su Arduino: https://wokwi.com/projects/352790112505422849

### **TIMERS HW DI ARDUINO SCHEDULATO CON AGGIORNAMENTO DEL TEMPO BASE**

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
volatile unsigned long precm;
unsigned long tbase = 100;

void periodicBlink(int led);
void schedule();

void schedule()
{
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
}
```
Simulazione su Arduino con Wowki: https://wokwi.com/projects/371840713313082369

### **TIMERS HW DI ARDUINO SCHEDULATO CON CONTEGGIO DEI TIMES TICK**

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
	prec += period;  				//preparo il tic successivo azzerando il conteggio del tempo ad adesso
	step = (step + 1) % nstep; 			// conteggio circolare (arriva al massimo a nstep-1)
  	// task 2
	if(!(step%3)){  // schedulo eventi al multiplo del periodo (3 sec = 3 periodi)
		periodicBlink(led2); 	// stato alto: led blink
	}
	// task 1
	if(!(step%2)){  // schedulo eventi al multiplo del periodo (2 sec = 2 periodi)
		periodicBlink(led1); 	// stato alto: led blink
	}
	// il codice eseguito al tempo del metronomo (esattamente un periodo) va quì
}
 
void periodicBlink(int led) {
  digitalWrite(led, !digitalRead(led));
}
 
void setup() {
	//randomSeed(millis());
	randomSeed(analogRead(0));
	prec=0;
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
	ITimer1.attachInterruptInterval(500, schedule);
	// Select Timer 1-2 for UNO, 0-5 for MEGA
	// Timer 2 is 8-bit timer, only for higher frequency
	ITimer2.init();
	ITimer2.attachInterruptInterval(1000, periodicBlink,led3);
}
 
void loop() {
	unsigned randomDelay = random(10, 2000);
	Serial.print("delay: ");Serial.println(randomDelay);
	delay(500);
	periodicBlink(led4);
}
```

Simulazione su Arduino con Wowki: https://wokwi.com/projects/352009022804591183

### **Sitografia**

- https://techtutorialsx.com/2021/08/07/esp32-ticker-library/
- https://github.com/espressif/arduino-esp32/tree/master/libraries/Ticker
- https://github.com/espressif/esp-idf/tree/8131d6f46d690e9cb60cc5cd457863cc5479351f/components/esp_timer
- https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_timer.html
- https://openlabpro.com/guide/timer-on-esp32/#:~:text=ESP32%20has%20two%20timer%20groups,counters%20and%2016%2Dbit%20prescalers.
- https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/peripherals/timer.html
- https://www.pjrc.com/teensy/td_libs_TimerOne.html
- https://github.com/PaulStoffregen/TimerOne
- https://github.com/PaulStoffregen/TimerThree
- https://github.com/khoih-prog/TimerInterrupt
- https://github.com/khoih-prog/ESP32TimerInterrupt
- https://github.com/marcelloromani/Arduino-SimpleTimer/tree/master/SimpleTimer

>[Torna all'indice generazione tempi](indexgenerazionetempi.md) >[versione in python](timerschedpy.md)

