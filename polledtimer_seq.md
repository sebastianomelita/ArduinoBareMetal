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

Un **timer periodico** più preciso, adatto a cumulare **lunghe cadenze periodiche** esegue il ricampionamento  con la maggior precisione possibile ottenuta evitando il **ritardo** dovuto alla chiamata della funzione ```millis()```. Un esempio potrebbe apparire così:

```C++
//Timer periodico
#define PERIODO  1000
unsigned long ptimer1;
bool timerState = false; // stato del timer
unsigned long periodo = PERIODO;

void loop()
{
	if ((timerState) && (millis() - ptimer1) >= periodo)
	{
		ptimer1 += periodo; // calcolo esatto del tempo attuale
		//....
		// istruzioni eseguite periodicamente, se attivo…
	}

}
```

Di seguito è un esempio di **timer aperiodico** che misura il tempo a partire dal verificarsi di una certa condizione fino a che, al superamento di un tempo limite, si decide di fare qualcosa ma solo **dopo** lo scadere del timer:

```C++
//Timer aperiodico 1
#define TLIMITE1  1000
unsigned long atimer1;

void loop()
{
	// blocco condizione di attivazione
	if(condA){
		atimer1 = millis();
	}
        
	//blocco polling
	if (millis() - atimer1 >= (unsigned long) TLIMITE1)
	{
		// istruzioni eseguite allo scadere del timer 1
	}
}
```
La **condizione**, in realtà, può essere collocata in un punto qualsiasi del loop() (può essere ad esempio attivata dalla pressione di un pulsante):

```C++
//Timer aperiodico 1
#define TLIMITE1  1000
unsigned long atimer1;

void loop()
{
	//blocco polling
	if (millis() - atimer1 >= (unsigned long) TLIMITE1)
	{
		// istruzioni eseguite allo scadere del timer 1
	}
	
	// blocco condizione di attivazione
	if(condA){
		atimer1 = millis();
	}
}
```
Le istruzioni eseguite allo scadere del timer possono essere inserite in una **callback**, funzione dal nome sempre uguale, che, di volta in volta, è invocata dal timer con un diverso corpo di istruzioni:
```C++
//Timer aperiodico 1
#define TLIMITE1  1000
unsigned long atimer1;

void onElapse(){
	// istruzioni eseguite allo scadere del timer 1
}

void loop()
{
	//blocco polling 
	if (millis() - atimer1 >= (unsigned long) TLIMITE1)
	{
		onElapse();
	}
	
	// blocco condizione di attivazione
	if(condA){
		atimer1 = millis();
	}
}
```

Reset del timer, polling del tempo trascorso e istruzioni triggerate (scatenate) dal timer potrebbero anche essere **rinchiuse** in altrettante **funzioni**. 
Inoltre viene introdotta una **variabile di stato** che potrebbe essere adoperata sia per **bloccare** il timer in un certo momento come per **riattivarlo** in un momento successivo, per il tempo rimanente prima del timeout:

```C++
//inizio variabili timer
unsigned long startTime;
unsigned long timelapse;
byte timerState=0;
//fine variabili timer
.
// funzione di attivazione
void startTimer(unsigned long duration){
	timerState=1;
	timelapse=duration;
	startTime=millis();
}

// funzione di disattivazione
void stopTimer(){
	timerState=0;
}

// polling: verifica se è arrivato il tempo di far scattare il timer
void aggiornaTimer(){
	if((timerState == 1) && (millis() - startTime >= timelapse)){
		timerState=0;
		onElapse();
	}
}	

// callback: azione standard da compiere allo scadere del timer, definita fuori dal loop
void onElapse(){
	//azione da compiere
	//.......
}
	
void loop(){
	//blocco polling
	aggiornaTimer();  //aggiorna il primo timer
		
	// blocco condizione di attivazione
	if(A){
		startTimer(1000);
	}

	if(B){ blocco condizione di disattivazione
		stopTimer();   
	}
}
```
>[Torna all'indice](timerbase.md) >[versione in Python](polledtimer_seq_py.md)
