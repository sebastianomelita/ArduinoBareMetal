
>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  

# **SCHEDULAZIONE CON I PROTOTHREAD**

I protothread sono un tipo di thread leggeri senza stack progettati per sistemi con vincoli di memoria severi come sottosistemi embedded o
nodi di reti di sensori. I protothreads possono essere usati con o senza un RTOS. 

I **protothread** forniscono un meccanismo di **blocco dei task** (compiti) in cima a un **sistema ad eventi**, senza l'overhead di uno stack per ogni thread. Lo **scopo** dei protothread è quello di implementare un **flusso sequenziale di controllo** senza utilizzare complesse macchine a stati finiti ed evitando l'overhead di un multi-threading completo, cioè quello dotato anche del **modo preemptive**.  I protothread forniscono la sola **modalità cooperativa** e il **rilascio anticipato** delle risorse è realizzato **senza l'utilizzo di interrupt** che generino il **cambio di contesto** dei thread. 

In altre parole, la **gestione dei task** è **ad eventi** all'interno di un **singolo thread reale** ma, tramite delle **macro**, un algoritmo si può programmare in **maniera sequenziale** all'interno di funzioni che girano su dei **thread logici**, cioè emulati. La **concorrenza** dei task è gestita da uno **schedulatore** che, invocato durante il periodo di **sleep** di un task, assegna la risorsa a quei task che, in quel periodo, ne fanno richiesta.

Di seguito è riportato un esempio di **blink sequenziale** in esecuzione su **due thread** separati su scheda **Arduino Uno**, con **IDE Arduino** e  con la libreria **protothread.h**  (https://gitlab.com/airbornemint/arduino-protothreads). I thread sono senza stack e **non preemptive** (solo collaborativi). La **programmazione sequenziale** del blink del led è **emulata** tramite una funzione delay() **non bloccante** ``` PT_SLEEP(pt, 200) ``` fornita dalla libreria ``` protothreads.h ```.

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
Osservazioni:
- il codice non è specifico di alcuna macchina, è realizzato in C puro ed è altamente portabile.
- Un protothread viene eseguito all'interno di una singola funzione C e non può estendersi ad altre funzioni. Un protothread può chiamare normali funzioni C, ma non può bloccore all'interno di una funzione chiamata da altre funzioni. Il blocco all'interno di chiamate di funzioni annidate
è invece ottenuto generando un protothread separato per ciascuna funzione potenzialmente bloccante. Il vantaggio di questo approccio è che
il blocco è esplicito: il programmatore sa esattamente quali funzioni un protothread blocca ciò e quali invece non blocca mai.
- poichè i protothread non salvano il contesto dello stack durante la chiamata di una funzione bloccante, ad es. una delay(), le variabili locali non vengono conservate quando protothread blocca un task. Ciò significa che le variabili locali dovrebbero essere usate con la massima
attenzione. In caso di dubbio, non utilizzare variabili locali all'interno di un protothread!
- Un protothread è guidato da ripetute chiamate alla funzione in cui il protothread è in esecuzione. Ogni volta che viene chiamata la funzione, il 
protothread verrà eseguito fino a quando non si blocca o esce. Pertanto la schedulazione dei protothreads viene eseguito dall'applicazione che utilizza i protothreads.

Di seguito è riportato un esempio su scheda **Arduino Uno**, con **IDE Arduino** e  con la libreria **protothread.h** di un **blink sequenziale** in esecuzione su **un thread** e di gestione del pulsante sul loop principale. I ritardi sleep agscono sul thread secondario ma non bloccano la letura dello stato del pulsante che rimane responsivo nell'accendere il secondo led durante le entrambe le fasi del blink del primo led.

```C++
#include "protothreads.h"

bool blink1_running = true;
int led1 = 13;
int led2 = 12;
byte pulsante=2;

// definizione protothread del lampeggio
pt ptBlink1;
int blinkThread1(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop secondario protothread
  for(;;) {
		digitalWrite(led1, HIGH);   // turn the LED on (HIGH is the voltage level)
		PT_SLEEP(pt, 500);			// delay non bloccanti
		digitalWrite(led1, LOW);    // turn the LED off by making the voltage LOW
		PT_SLEEP(pt, 500);			// delay non bloccanti
  }
  PT_END(pt);
}

// the setup function runs once when you press reset or power the board
void setup() {
  PT_INIT(&ptBlink1);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
}

// the loop function runs over and over again forever
void loop() { // loop principale
	PT_SCHEDULE(blinkThread1(&ptBlink1)); // esecuzione schedulatore protothreads
    
	// gestione pulsante
	if (digitalRead(pulsante) == HIGH) {
		// turn LED on:
		digitalWrite(led2, HIGH);
	}else{
		// turn LED off:
		digitalWrite(led2, LOW);
	}
}
```

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  
