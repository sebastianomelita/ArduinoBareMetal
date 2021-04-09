## **POLLED TIMERS**

E’ possibile realizzare dei timers ottenuti eseguendo, ad ogni loop(), il **polling della funzione millis()** per verificare che questa abbia raggiunto il **valore finale** stabilito come target (obiettivo), quando ciò accade, possiamo fare eseguire il blocco di istruzioni **associato** a questo evento.

I timers possono essere **periodici**, ed uno di questi era proprio lo schedulatore adoperato per generare la base degli eventi di sistema, oppure possono essere **aperiodici** (detti anche monostabili), cioè devono essere attivati, con una istruzione apposita, solamente quando serve.

**In** **generale**, possiamo individuare alcune **componenti del timer**:

- **reset** del timer. Accade al verificarsi di una certa **condizione**.

- **polling** del tempo trascorso. Viene fatto ad ogni ciclo di **loop** (talvolta meno).

- **istruzioni triggerate** (scatenate) dal timer. Vengono eseguite (o non eseguite) in base al **tempo** **misurato** dal timer

- (opzionale) la **variabile stato** del timer che **abilita** il timer quando serve e lo **disabilita** quando non serve per evitare **attivazioni spurie** accidentali.

Un esempio di **timer periodico** (del tutto analogo ad un o schedulatore) potrebbe apparire così:

```C++
//Timer periodico
#define PERIODO  1000
unsigned long ptimer1;
bool timerState = false; // stato del timer

void loop()
{
	if ((timerState) && (millis() - ptimer1) >= (unsigned long) PERIODO)
	{
		ptimer1 = millis();
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
	// condizione che valuta l’accadere di un certo evento
	// di attivazione del conteggio del tempo
	if(condA){
		atimer1 = millis();
	}

	if (millis() - atimer1 >= (unsigned long) TLIMITE1)
	{
		// istruzioni eseguite allo scadere del timer 1
	}
}
```

```C++
//Timer aperiodico 1
#define TLIMITE1  1000
unsigned long atimer1;

void loop()
{
	if (millis() - atimer1 >= (unsigned long) TLIMITE1)
	{
		// istruzioni eseguite allo scadere del timer 1
	}
	
	// condizione che valuta l’accadere di un certo evento
	// di attivazione del conteggio del tempo
	if(condA){
		atimer1 = millis();
	}
}
```

```C++
//Timer aperiodico 1
#define TLIMITE1  1000
unsigned long atimer1;

void onElapse(){
	// istruzioni eseguite allo scadere del timer 1
}

void loop()
{
	if (millis() - atimer1 >= (unsigned long) TLIMITE1)
	{
		onElapse();
	}
	
	// condizione che valuta l’accadere di un certo evento
	// di attivazione del conteggio del tempo
	if(condA){
		atimer1 = millis();
	}
}
```

```C++
//inizio variabili timer
unsigned long startTime;
unsigned long timelapse;
byte timerState=0;
//fine variabili timer

void startTimer(unsigned long duration){
	timerState=1;
	timelapse=duration;
	startTime=millis();
}

void stopTimer(){
	timerState=0;
}

//verifica se è arrivato il tempo di far scattare il timer
void aggiornaTimer(){
	if((timerState == 1) && (millis() - startTime >= timelapse)){
		timerState=0;
		onElapse();
	}
}	

// azione da compiere allo scadere del timer, definita fuori dal loop
void onElapse(){
	//azione da compiere
	//.......
}
	
void loop(){
	aggiornaTimer();  //aggiorna il primo timer
		
		//se accade qualcosa parte il timer
		if(A){
			startTimer(1000);
		}
		
		if(B){ //se accade qualcosa blocco il timer
			stopTimer();   
		}
}

```

