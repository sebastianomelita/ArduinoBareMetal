>[Torna all'indice](timerbase.md) >[versione in Python](polledtimer_event_py.md)
## **POLLED TIMERS**

E’ possibile realizzare dei timers, con cui programmare **nel futuro** lo stesso evento o una catena di eventi diversi, eseguendo il **polling della funzione millis()**. Il polling serve per verificare, ad ogni loop(), che la **millis()** abbia raggiunto il **valore finale** stabilito come target (obiettivo), quando ciò accade possiamo far eseguire il blocco di istruzioni **associato** a questo evento utilizzando una funzione di **callback**.

I timers possono essere **periodici**, ed uno di questi era proprio lo schedulatore adoperato per generare la base degli eventi di sistema, oppure possono essere **aperiodici** (detti anche monostabili), cioè devono essere attivati, con una istruzione apposita, solamente quando serve.

**In** **generale**, possiamo individuare alcune **componenti del timer**:

- **reset** del timer. Accade al verificarsi di una certa **condizione**.

- **polling** del tempo trascorso. Viene fatto ad ogni ciclo di **loop** (talvolta meno).

- **istruzioni triggerate** (scatenate) dal timer. Vengono eseguite (o non eseguite) in base al **tempo** **misurato** dal timer

- **condizione di attivazione** posta in punto qualsiasi del loop determina, quando è vera, la partenza del timer

- (opzionale) la **variabile stato** del timer che **abilita** il timer quando serve e lo **disabilita** quando non serve per evitare **attivazioni spurie** accidentali.

Un **timer periodico** è del tutto analogo ad un o schedulatore e, ad ogni timeout, necessita del **ricampionamento** del **tempo attuale** per poter di nuovo calcolare il **tempo futuro** del nuovo **timeout**.  Un esempio di **timer periodico** potrebbe apparire così:

```C++
//Timer periodico
#define PERIODO  1000
unsigned long ptimer1;
bool timerState = false; // stato del timer

void loop()
{
	if ((timerState) && (millis() - ptimer1) >= (unsigned long) PERIODO)
	{
		ptimer1 = millis(); // ricampionamento del tempo attuale 
		//....
		// istruzioni eseguite periodicamente, se attivo…
	}

}
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
>[Torna all'indice](timerbase.md) >[versione in Python](polledtimer_event_py.md)
