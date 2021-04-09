

**POLLED TIMERS**

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

	if ((millis() - atimer1) >= (unsigned long) TLIMITE1)
	{
		//....
		// istruzioni eseguite allo scadere del timer 1
	}
}
```

<!--stackedit_data:
eyJoaXN0b3J5IjpbLTE0NDU1MTUzNSwtOTUzNDU0NjY1XX0=
-->