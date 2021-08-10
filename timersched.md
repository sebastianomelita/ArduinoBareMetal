>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  

# **SCHEDULAZIONE CON I TIMERS**

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
