>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  

# **SCHEDULAZIONE CON I TIMERS HW**

La **libreria Ticker** è un **wrapper (involucro)** di un'**API** del framework che si **interfaccia direttamente** con l'**HW**. Si adopera con gli **stessi metodi** su due piattaforme molto diverse (**ESP8266** e **ESP32**) ma entrambe dotate di interfaccia WiFi. Il FW di bordo nel caso di ESP8266 è LWIP ed è essenzialmente uno stack TCP/IP minimale per sistemi embedded. Il FW in uso su ESP32 si chiama IDF. Entrambi forniscono librerie per la gestione di timer HW. Nel caso di **ESP8266** si chiama **os_timer**, nel caso di **ESP32** il nome della libreria è **esp_timer**. 

Il motivo dell'**interfaccia comune** è che si tratta di prodotti molto diffusi della stessa casa produttrice ai quali si vuole fornire una interfaccia di utilizzo uniforme per la gestione dei timer. **Funzioni simili** sono implementate in quasi tutti i **sistemi embedded**, Arduino compreso, o tramite **librerie di terze parti** o direttamente all'interno del **core** del framework di gestione dell'HW (librerie o OS). 

Rispetto agli altri metodi di creazione di base dei tempi (polling della millis(), thread e protothread), è tendenzialmente più legato ad uno specifico vendor di HW e ad una specifica linea di prodotti. Le **API dei timer**, pur esendo **molto simili** tra loro, **non sono standardizzate** e la **portabilità** del SW nel tempo potrebbe non essere garantita. In ogni caso **semplificano** parecchio la **gestione delle ISR** associate a timer HW che altrimenti, eseguita a basso livello, richiede una impostazione di **registri interni** della CPU che necessita di conoscenze di dettaglio molto specifiche.

La **logica di utilizzo** è simile a quella di una **usuale ISR**. Una **funzione di callback** viene associata ad un evento di un timer tramite un metodo **attach()**. La dissociazione si fa con il metodo contrario **detach()**. Gli **eventi possibili** sono una chiamata una tantum (**one shot** o timer monostabile) o una chiamata **periodica** o timer bistabile. Nella definizione dell'attach() viene anche impostato il **tempo di scadenza** del timer. 

In genere, in molte implementazioni, callback di timer diverse vengono eseguite **in sequenza** e non su thread paralleli per cui operazioni bloccanti come le ```delay()```, oltre a causare possibili **instabilità** (sono ISR basate su interrupt), **ritardano** l'esecuzione delle callback **a seguire**.

Esempio delle due modalità:
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
-  se si registrano callback multiple per uno stesso timer maggiore sarà il ritardo per le callback che verranno chiamate dopo, dato che per essere eseguite loro devono necessariamente ritornare le precedenti.
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



```C++
#include <TimerOne.h>
#include <TimerThree.h>

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
  Timer1.initialize(500000);
  Timer1.attachInterrupt(periodicBlink, led1); 
  Timer3.initialize(1000000);
  Timer3.attachInterrupt(periodicBlink, led2); 
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
	Timer1.detachInterrupt(); 
	Timer3.detachInterrupt();
}
```

Simulazione su Arduino con Wowki: 

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  
