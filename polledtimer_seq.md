>[Torna all'indice](timerbase.md) >[versione in Python](polledtimer_seq_py.md)
## **POLLED TIMERS SEQUENZIALI**

E’ possibile realizzare dei timers, con cui programmare **nel futuro** lo stesso evento o una catena di eventi diversi, eseguendo il **tempo di conteggio**. Il polling serve per verificare, ad ogni loop(), che il tempo trascorso (**elpased**) abbia raggiunto il **valore finale** (timeout) stabilito come obiettivo (target). Ciò si può ottenere utilizzando il **test** periodico di una funzione get() che, all'interno del loop(), restituisce il **tempo di conteggio** (elapsed). Se è vera allora il blocco di istruzioni associato all'evento di timeout viene eseguito.

I **timer sequenziali** sono più **intuitivi** e **semplici** da usare rispetto ad i **timer ad eventi** perchè possono essere adoperati usando la stessa logica **sequenziale** e **lineare** che si usa in un normale algoritmo sincrono. Un'**algoritmo sincrono** pianifica le azioni in base alla **posizione** della istruzioni che le determinano nel codice e in base ai tempi stabiliti da eventuali **ritardi bloccanti** (l'esecuzione non va avanti) posti tra un'azione e l'altra o dal **polling di un timer** (test di avvenuto timeout).

Si tratta di un **pattern** (tipo di API) per la realizzazione di timers **molto comune** nella programmazione di **bracci robotici** per uso industriale (vedi bracci Universal Robots) che si adatta bene ad essere impiegato in un contesto in cui la logica dell'algoritmo è realizzata interamente in maniera sequenziale e lineare.

**In** **generale**, possiamo individuare alcune **componenti del timer**:

- **reset** del timer. Accade al verificarsi di una certa **condizione**. Determina sia l'azzeramento del **tempo di conteggio** (```elapsed = 0```).che l'azzeramento della misura del ritardo dall'ultimo riavvio (```last = millis()```).
- **start**. Avvia o riavvia il timer quando si avvera la **condizione di attivazione** posta in punto qualsiasi del loop. Imposta il flag di stato ```timerState```. 
- **stop** del timer. Accade al verificarsi di una certa **condizione** di sospensione del timer posta in punto qualsiasi del loop. In questa occasione il timer **campiona** il ritardo accumulato dall'ultimo riavvio e lo somma al **tempo di conteggio** cumulato all'ultima sospensione tramite ```elapsed += millis() - last```.
- **get** del tempo **tempo di conteggio** (elapsed) dal momento del **primo avvio** del timer. Viene fatto ad ogni ciclo di **loop** (meno se filtrato) e serve a realizzare il confronto con un tempo di timeout mediante un operatore relazionale (```var.get() > timeout```). Il valore restituito dipende dallo stato del timer:
     - se **```timerState == false```** allora restituisce l'ultimo **tempo di conteggio** misurato e memorizzato al momento della chiamata a stop() tramite ```elapsed += millis() - last```.
     - se **```timerState == true```** allora calcola il **tempo di conteggio** corrente campionando il ritardo attuale dall'ultimo riavvio e sommandolo ai ritardo cumulato prima dell'ultima sospensione (```millis() - last + elapsed```).
- **istruzioni triggerate** (scatenate) dal timer. Vengono eseguite in base al **tempo di conteggio misurato** dal timer. Vengono eseguite in maniera sequenziale in un punto stabilito del loop (istruzioni sincrone) in corrispondenza della verità di una certa condizione che coinvolge la funzione ```get()``` come operando.


```C++
typedef struct 
{
	unsigned long elapsed, last;
	bool timerState=false;
	void reset(){
		elapsed = 0;
		last = millis();
	}
	void stop(){
		timerState = false;
    		elapsed += millis() - last;
	}
	void start(){
		timerState = true;
		last = millis();
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

### **Esempi**

```C++
/*
Scrivere un programma che realizzi l'accensione di un led tramite un pulsante temporizzato che spegne il led 
dopo un numero di ms impostati da setup. La logica del tasto deve essere senza stato e deve essere sensibile 
al fronte di salita del segnale. Immaginare un collegamento pull down del tasto.
*/
//inizio variabili per un solo pulsante
int led=13;
int tasto=2;
int in, out;

// attesa evento con tempo minimo di attesa
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
		timerState = false;
    		elapsed += millis() - last;
	}
	void start(){
		timerState = true;
		last = millis();
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
     	digitalWrite(led,LOW);
    }
}
	
```
Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/things/fCpauVnNUZh-accensione-led-monostabile/editel

```C++
/*
Scrivere un programma che realizzi l'accensione di un led, due led o tre led tramite la pressione consecutiva di un pulsante 
una, due o tre volte all'interno di un intervallo temporale di un secondo.
Col la pressione prolungata dello stesso pulsante si spengono tutti e tre contemporaneamente.
(Realizzazione del timer senza schedulatore)
*/

// attesa evento con tempo minimo di attesa
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
		timerState = false;
    		elapsed += millis() - last;
	}
	void start(){
		timerState = true;
		last = millis();
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
        conteggio.stop();
	conteggio.reset();
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

>[Torna all'indice](timerbase.md) >[versione in Python](polledtimer_seq_py.md)
