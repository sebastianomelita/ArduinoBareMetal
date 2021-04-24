
>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  
>
# **SCHEDULAZIONE CON I PROtOTHREAD**

Stesso esempio precedente ma eseguito su **Arduino Uno**, con **IDE Arduino** e  con la libreria **porotothread.h**  (https://gitlab.com/airbornemint/arduino-protothreads). I thread sono senza stack e **non preemptive** (solo collaborativi). La programmazione sequenziale del blink del led è emulata tramite una funzione delay() non bloccante fornita dalla libreria PT_SLEEP(pt, 200);

```C++
/*
  Blink
  Turns an LED on for one second, then off for one second, repeatedly. Rewritten with Protothreads.
*/

#include "protothreads.h"

bool blink1_running = true;
bool blink2_running = true;
int led1 = 13;
int led2 = 12;

pt ptBlink1;
int blinkThread1(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop forever
  for(;;) {
	if (blink1_running == true) {
		digitalWrite(led1, HIGH);   // turn the LED on (HIGH is the voltage level)
		PT_SLEEP(pt, 500);
		digitalWrite(led1, LOW);    // turn the LED off by making the voltage LOW
		PT_SLEEP(pt, 500);
	} else {
		digitalWrite(led1, LOW);    // turn the LED off by making the voltage LOW
		PT_YIELD(pt);
	}
  }

  PT_END(pt);
}

pt ptBlink2;
int blinkThread2(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop forever
  for(;;) {
	if (blink2_running == true) {
		digitalWrite(led2, HIGH);   // turn the LED on (HIGH is the voltage level)
		PT_SLEEP(pt, 1000);
		digitalWrite(led2, LOW);    // turn the LED off by making the voltage LOW
		PT_SLEEP(pt, 1000);
	} else {
		digitalWrite(led2, LOW);    // turn the LED off by making the voltage LOW
		PT_YIELD(pt);
	}
  }

  PT_END(pt);
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  PT_INIT(&ptBlink1);
  PT_INIT(&ptBlink2);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
	PT_SCHEDULE(blinkThread1(&ptBlink1));
	PT_SCHEDULE(blinkThread2(&ptBlink2));
  
	int count = 0;
	while(true){
		Serial.print("Doing stuff... ");
		Serial.println(count);
		count += 1;
		if(count >= 10)
			break;
		delay(1000);
	}
	Serial.print("Ending threads...");
	blink1_running = false;
	blink2_running = false;
}
```

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  
