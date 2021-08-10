>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  

# **SCHEDULAZIONE CON I TIMERS**

La libreria Ticker è un wrapper (involucro) di un'API del framework che si interfaccia direttamente con l'HW. Si adopera con gli stessi metodi su due piattaforme molto diverse (ESP8266 e ESP32) ma entrambe dotate di interfaccia WiFi. Il FW di bordo nel caso di ESP8266 è LWIP ed è essenzialmente uno stack TCP/IP minimale per sistemi embedded. Il FW in uso su ESP32 si chiama IDF. Entrambi forniscono librerie di timer SW. Nel caso di esp8266 si chiama os_timer, nel caso di esp32 il nome della libreria è esp_timer.

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
 
  int maxExecutionsCount=10;
   
  periodicTicker.attach_ms(5000, periodicPrint, maxExecutionsCount);
}
 
void loop() {}
```
L'esp_timer è un set di APIs che fornisce timer one-shot e periodici, con risoluzione di tempo dei microsecondo e 64-bit di range di conteggio. In ogn caso è bene tenere presente che:
- la libreria Ticker fornisce però la precisione del millisecondo quindi se si ha bisogno di qualcosa di più granulare, è utile sapere che le funzioni sottostanti sono più flessibili.
- le funzioni di callback non è detto che vengano eseguite immediatamente quando si attiva il timer hardware. L'effettiva implementazione dell'API IDF esegue la chiamata annidata di una funzion ausiliaria che potrebbe ritornare con un certo ritardo.
-  se si registrano callback multiple per uno stesso timer maggiore sarà il ritardo per le callback che verranno chiamate dopo, dato che per essere eseguite loro devono necessariamente ritornare le precedenti.
- le callback sono chiamate da funzioni ISR lanciate da segnali di interrupt provenienti dai timer. Le ISR di norma dovrebbero essere molto brevi e, in ogni caso, mai bloccanti, per evitare instabilità del sistema.

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

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  
