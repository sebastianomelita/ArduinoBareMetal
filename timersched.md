>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  

# **SCHEDULAZIONE CON I TIMERS HW**

Il microcontrollore **esp32** ha 4 **timer HW**. Tutti i timer sono divisi in **2 gruppi**, 2 timer in ciascun gruppo. **Ogni timer** HW può avere una propria **configurazione indipendente**.

Sebbene siano disponibili **timer software** di FreeRTOS (il sistema operativo di ESP32) questi timer presentano alcune limitazioni:
- La **risoluzione massima** è uguale al periodo di tick di RTOS
- Le **callback** del timer vengono lanciate da un task a **bassa priorità**

I **timer hardware** invece sono liberi da entrambe le limitazioni, ma spesso sono **meno convenienti** da usare. Ad esempio, i componenti dell'applicazione potrebbero richiedere che gli eventi del timer si attivino in determinati **momenti nel futuro**, ma il timer hardware contiene un **unico** valore di "confronto" utilizzato per la generazione di un interrupt. 

Ciò significa che è necessario costruire una **astrazione** in cima ai timer hardware che, nonostante utilizzi ancora il meccanismo delle interruzioni, implementi anche le  **funzionalità** per gestire l'**elenco** degli **eventi in sospeso** e per richiamare le **callback** per questi eventi man mano che si verificano le interruzioni hardware corrispondenti.

In ESP32, le **callback** del timer possono essere inviate con due metodi:
- ```ESP_TIMER_TASK```. Le callback del timer vengono inviati da un unico **task** ```esp_timer``` ad **alta priorità**. Poiché tutte le callback vengono smistate dallo **stesso task**, si consiglia di eseguire solo la **minima** quantità di lavoro possibile all'interno di una stessa callback. Se sono in esecuzione altri task con **priorità superiore** a ```esp_timer```, l'invio della callback verrà ritardato fino a quando l'attività esp_timer non avrà la possibilità di essere eseguita.
- ```ESP_TIMER_ISR```. Le callback del timer vengono inviati direttamente dal gestore di interrupt del timer. Questo metodo è utile per alcune semplici richiamate che mirano a una latenza inferiore.

La **libreria Ticker** è un **wrapper (involucro)** delle'**API ```esp_timer```** che si **interfaccia direttamente** con l'**HW** secondo il metodo ```ESP_TIMER_TASK```. Si adopera con gli **stesse API** su due piattaforme molto diverse (**ESP8266** e **ESP32**) ma entrambe dotate di interfaccia WiFi. Il FW di bordo nel caso di ESP8266 è LWIP ed è essenzialmente uno stack TCP/IP minimale per sistemi embedded. Il FW in uso su ESP32 si chiama IDF. Entrambi forniscono librerie per la gestione di timer HW. Nel caso di **ESP8266** si chiama **os_timer**, nel caso di **ESP32** il nome della libreria è **esp_timer**. 

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

Simulazione su Arduino con Wowki: https://wokwi.com/projects/351250406794330712

### **Sitografia**

https://techtutorialsx.com/2021/08/07/esp32-ticker-library/

https://github.com/espressif/arduino-esp32/tree/master/libraries/Ticker

https://github.com/espressif/esp-idf/tree/8131d6f46d690e9cb60cc5cd457863cc5479351f/components/esp_timer

https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_timer.html

https://openlabpro.com/guide/timer-on-esp32/#:~:text=ESP32%20has%20two%20timer%20groups,counters%20and%2016%2Dbit%20prescalers.

https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/peripherals/timer.html

https://www.pjrc.com/teensy/td_libs_TimerOne.html

https://github.com/PaulStoffregen/TimerOne

https://github.com/PaulStoffregen/TimerThree

https://github.com/khoih-prog/TimerInterrupt

https://github.com/khoih-prog/ESP32TimerInterrupt

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  
