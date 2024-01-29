>[Torna all'indice](timerbase.md) >[versione in Python](polledtimer_seq_py.md)
## **POLLED TIMERS SEQUENZIALI**

E’ possibile realizzare dei timers, con cui programmare **nel futuro** lo stesso evento o una sequenza di eventi diversi, **senza attese**, cioè senza ```delay()```, eseguendo un polling del **tempo di conteggio** di un **timer**. 

Un **polling** è l'interrogazione periodica di una **variabile** o di un **ingresso** per leggerne il **valore**. Se il valore è maggiore di un **tempo massimo** allora si considera avvenuto un **timeout** e si esegue il **blocco** di codice associato ad esso.

Il polling serve per verificare, ad ogni loop(), che il tempo trascorso (**elapsed**) abbia raggiunto il **valore finale** (timeout) stabilito come obiettivo (target). Ciò si può ottenere eseguendo il **test** periodico di una **condizione** su una funzione di **misura** del tempo, nello specifico una ```get()```, che, ad ogni ```loop()```, restituisce il **tempo di conteggio**. Se la **condizione di uscita** dal conteggio è **vera** allora il **blocco** di istruzioni associato al **timeout del timer** viene **eseguito**.

Un **timeout** si controlla, valutando la **condizione di scadenza** del timer sulla funzione ```get()``` mediante una istruzione di selezione **if**. Ad esempio, eseguendo periodicamente (**polling**) nel ```loop()``` il controllo ```if(t.get() > 10000) {....}```, si può stabilire se, dal momento dell'attivazione del timer, sono pasasti 10 secondi e, in caso affermativo, eseguire le istruzioni nel blocco then che nel codice segue la condizione di scadenza.

La funzione ```get()```  **non è bloccante** (non causa alcun ritardo) e  **non interferisce** con nessun delay del loop corrente, sia esso il loop principale o quello secondario di un thread. Essa è quindi un task che si può tranquillamente adoperare in **sequenza** ad altri task di uno stesso loop, anche se questi contengono delay().

Il **polling** della funzione ```get()``` può essere effettuato in qualunque punto del loop, anche più volte se necessario, in genere si esegue per:
-  se il timer **è attivo**, controllare una **scadenza**, cioè vedere se è passato il tempo necessario per compiere una certa **azione** (modificare una variabile o chiamare una funzione)
-  se il timer **non è attivo**, impostare una **cadenza**, cioè se il timer è stato **bloccato** su un certo tempo con uno ```stop()```, utilizzare quel valore **programmato** di tempo per impostare dinamicamente un **ritardo delay**, oppure per confrontarlo con altri tempi che si stanno **accumulando** su una variabile all'interno del loop.
  
Si tratta di un **pattern** (tipo di API) per la realizzazione di timers **molto comune** nella programmazione di **bracci robotici** per uso industriale (vedi bracci Universal Robots) che si adatta bene ad essere impiegato sia in un contesto in cui la logica dell'algoritmo è realizzata in maniera sequenziale sia in quello in cui gli input sono gestiti con un modello ad eventi.

**In** **generale**, possiamo individuare alcune **componenti del timer**:

- **reset** del timer. Accade al verificarsi di una certa **condizione**. Determina sia l'azzeramento del **tempo di conteggio** che l'azzeramento della misura del ritardo dall'ultimo riavvio. L'azzeramento **non** è mai automatico ed è necessario impostarlo prima del riutilizzo del timer dopo uno o più comandi ``stop()`` (tipicamente al momento della valutazione della condizione di timeout) altrimenti è sufficiente richiamare la funzione ```start()```.  
- **start**. Avvia o riavvia il timer quando si avvera la **condizione di attivazione** posta in punto qualsiasi del loop. Imposta il flag di stato ```timerState```. 
- **stop** del timer. Accade al verificarsi di una certa **condizione** di sospensione del timer posta in punto qualsiasi del loop. In questa occasione il timer **campiona** il ritardo accumulato dall'ultimo riavvio e lo somma al **tempo di conteggio** cumulato in corrispondenza della sospensione precedente. In questo modo il tempo trascorso è memorizzato e può essere valutato in un momento successivo nel codice tramite la funzione ```get()```.
- **get** del tempo **tempo di conteggio** (elapsed) dal momento del **primo avvio** del timer. E' una funzione **non bloccante**, cioè non genera attese dato che restituisce immediatamente il suo risultato. Viene **interrogata** periodicamente (polling) ad ogni ciclo di **loop** (meno se decimato ogni tot loop) e serve a realizzare una espressione di confronto con un tempo di **timeout** (```var.get() > timeout```). Il **valore restituito** dipende dallo stato del timer:
     - se **```timerState == false```** allora restituisce l'ultimo **tempo di conteggio** misurato e memorizzato al momento della chiamata a stop() .
     - se **```timerState == true```** allora calcola il **tempo di conteggio** corrente campionando il ritardo attuale dall'ultimo riavvio e sommandolo ai ritardo cumulato prima dell'ultima sospensione.
- **istruzioni triggerate** (scatenate) dal timer. Vengono eseguite in base al **tempo di conteggio misurato** dal timer. Vengono eseguite in maniera sequenziale in un punto stabilito del loop (istruzioni sincrone) in corrispondenza della **verità** di una certa **condizione** sul tempo di conteggio che coinvolge la funzione ```get()``` come operando. In corrispondenza si dovrebbero eseguire:
  -    **reset()**  per azzerare il timer.
  -    **stop()** per bloccare il timer.

```C++
[#ifndef __URUTILS_H__
#define __URUTILS_H__

// attesa evento con tempo minimo di attesa
void waitUntil(bool &c, unsigned t)
{
    while(!c){
	delay(t);
    }
}

void waitUntilInputLow(int btn, unsigned t)
{
    while(!digitalRead(btn)==LOW){
	delay(t);
    }
}

typedef struct 
{
	unsigned long elapsed, last;
	bool timerState=false;
	void reset(){
		elapsed = 0;
		last = millis();
	}
	void stop(){
		if(timerState){
			timerState = false;
    	                elapsed += millis() - last;
		}	
	}
	void start(){
		if(!timerState){
			timerState = true;
			last = millis();
		}
	}
	unsigned long get(){
		if(timerState){
			return millis() - last + elapsed;
		}
		return elapsed;
	}
	void set(unsigned long e){
		reset();
		elapsed = e;
	}
} DiffTimer;
```

I prossimi esercizi sono realizzati adoperando la libreria che si può scaricare cliccando col tasto sinistro sul link [urutils.h](urutils.h). 

### **Esempi**

```C++
/*
Scrivere un programma che realizzi l'accensione di un led tramite un pulsante temporizzato che spegne il led 
dopo un numero di ms impostati da setup. La logica del tasto deve essere senza stato e deve essere sensibile 
al fronte di salita del segnale. Immaginare un collegamento pull down del tasto.
*/
#include "urutils.h"
//inizio variabili per un solo pulsante
int led=13;
int tasto=2;
int in, out;
DiffTimer acceso;

void setup(){
    pinMode(led,OUTPUT);
    pinMode(tasto,INPUT);
    digitalWrite(led,LOW);
    digitalWrite(tasto,LOW);
}

void loop(){
    if(digitalRead(tasto)==HIGH){
        digitalWrite(led,HIGH);
        waitUntilInputLow(tasto,50);			// attendi finchè non c'è fronte di discesa
      	acceso.start();
    }else if(acceso.get() > 5000){
	//non necessario il reset() perchè mai eseguito uno stop()
     	digitalWrite(led,LOW);
    }
}
	
```
Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/things/fCpauVnNUZh-accensione-led-monostabile/editel

### **Selezione luci**

```C++
/*
Scrivere un programma che realizzi l'accensione di un led, due led o tre led tramite la pressione consecutiva di un pulsante 
una, due o tre volte all'interno di un intervallo temporale di un secondo.
Col la pressione prolungata dello stesso pulsante si spengono tutti e tre contemporaneamente.
(Realizzazione del timer senza schedulatore)
*/
#include "urutils.h"
DiffTimer conteggio, spegnimento;
int led1=13;
int led2=12;
int led3=11;
int tasto=2;
int count=0;

void setup(){
    pinMode(tasto,INPUT);
    pinMode(led1,OUTPUT);
    pinMode(led2,OUTPUT);
    pinMode(led3,OUTPUT);
    digitalWrite(led1,LOW);
    digitalWrite(led2,LOW);
    digitalWrite(led3,LOW);
    Serial.begin(115200);
}

void loop(){	
    //legge valore attuale dell'ingresso
    if(digitalRead(tasto)==HIGH){
	//fronte di salita
        spegnimento.start();
        waitUntilInputLow(tasto,50);			// attendi finchè non c'è fronte di discesa
        //fronte di discesa
        spegnimento.stop();
	//parte alla prima pressione
	count++;
	if(count == 1)
		conteggio.start();
    }else if(conteggio.get() > 1000){
	if(count>0){
		digitalWrite(14-count,HIGH);
		count=0;
	}
    }else if(spegnimento.get() > 3000){
        spegnimento.reset();
        digitalWrite(led1,LOW);
        digitalWrite(led2,LOW);
        digitalWrite(led3,LOW);
        count=0;
    }
    delay(10);
}
```
Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/things/1EIcEp5BkZt-copy-of-selezione-luci-e-spegnimento-con-un-solo-tasto/editel?tenant=circuits

### **Scegli chi ON/OFF un tasto**

```C++
/*
 Scrivere un programma che realizzi l'accensione del led1, oppure del led2 oppure del led3 led 
 tramite la pressione consecutiva di un pulsante una, due o tre volte all'interno di 
 un intervallo temporale di un secondo. Col la pressione per almeno un secondo, ma meno di tre, dello stesso pulsante si 
 accendono tutti i led, con la pressione dello stesso tasto per più di tre secondi si spengono tutti i led.
*/
#include "urutils.h"
DiffTimer conteggio, spegnimento;

int led1=13;
int led2=12;
int led3=11;
int tasto=2;
int count=0;
int in, out;

void setup(){
	pinMode(tasto,INPUT);
	pinMode(led1,OUTPUT);
	pinMode(led2,OUTPUT);
	pinMode(led3,OUTPUT);
	digitalWrite(led1,LOW);
	digitalWrite(led2,LOW);
	digitalWrite(led3,LOW);
}

void loop(){			
	//ad ogni pressione del tasto entro il tempo prefissato aggiorna il contatore di quel tasto (pressione)
	if(digitalRead(tasto)==HIGH){
		//fronte di salita
		spegnimento.start();
		waitUntilInputLow(tasto,50);			// attendi finchè non c'è fronte di discesa
		//fronte di discesa
		spegnimento.stop();
		//parte alla prima pressione
		count++;
		if(count == 1)
			conteggio.start();
	}else if(conteggio.get() > 1000){
		if(count>0){
			digitalWrite(14-count,HIGH);
			count=0;
		}
	}
	if(spegnimento.get() > 1000 && spegnimento.get() < 2000){
		spegnimento.reset();
		digitalWrite(led1,HIGH);
		digitalWrite(led2,HIGH);
		digitalWrite(led3,HIGH);
		count=0;
	}else if(spegnimento.get() > 2000){
		spegnimento.reset();
		digitalWrite(led1,LOW);
		digitalWrite(led2,LOW);
		digitalWrite(led3,LOW);
		count=0;
	}
}
```

Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/things/92WnWXH0OvB-copy-of-scegli-chi-onoff-con-un-tasto/editel?tenant=circuits

### **Scegli chi ON/OFF due tasti**

In questo esempio vengono gestiti due input con i delay in un unico loop(). Questo comporta che possono essere utilizzati entrambi i pulsanti ma in momenti diversi essendo il loop() per 50 msec monopolizzato dal task dell'antirimbalzo di ciascun pulsante. Nel tempo di guardia dell'antirimbalzo nessun task in più può essere svolto in parallelo.  

```C++
/*
 Scrivere un programma che realizzi l'accensione del led1, oppure del led2 oppure del led3 led 
 tramite la pressione consecutiva di un pulsante una, due o tre volte all'interno di 
 un intervallo temporale di un secondo. Col la pressione di un altro pulsante si fa una cosa analoga per spegnerli.
*/
#include "urutils.h"
int led1=13;
int led2=12;
int led3=11;
int tasto1=2;
int tasto2=3;
int count1=0;
int count2=0;

void setup(){
	pinMode(tasto1,INPUT);
	pinMode(tasto2,INPUT);
	pinMode(led1,OUTPUT);
	pinMode(led2,OUTPUT);
	pinMode(led3,OUTPUT);
	digitalWrite(led1,LOW);
	digitalWrite(led2,LOW);
	digitalWrite(led3,LOW);
	//startTimer(1000);
	count1=0;
	count2=0;
}

void loop(){
	if(digitalRead(tasto1)==HIGH){
		//fronte di salita
		waitUntilInputLow(tasto1,50);			// attendi finchè non c'è fronte di discesa
		//fronte di discesa
		//parte alla prima pressione
		count1++;
		if(count1 == 1)
			conteggio1.start();
	}else if(digitalRead(tasto2)==HIGH){
		//fronte di salita
		waitUntilInputLow(tasto2,50);			// attendi finchè non c'è fronte di discesa
		//fronte di discesa
		//parte alla prima pressione
		count2++;
		if(count2 == 1)
			conteggio2.start();
	}
        if(conteggio1.get() > 1000){
		if(count1>0){
			digitalWrite(14-count1,HIGH);
			count1=0;
		}
	}
        if(conteggio2.get() > 1000){
		if(count2>0){
			digitalWrite(14-count2,LOW);
			count2=0;
		}
	}
	delay(10);
}


```
Simulazione su Arduino con Tinkercad: [tenant=circuits](https://www.tinkercad.com/things/1eJwTOD7t8K-copy-of-scegli-chi-onoff/editel?tenant=circuits)https://www.tinkercad.com/things/1eJwTOD7t8K-copy-of-scegli-chi-onoff/editel?tenant=circuits

>[Torna all'indice](timerbase.md) >[versione in Python](polledtimer_seq_py.md)
