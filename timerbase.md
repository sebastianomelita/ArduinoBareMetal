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
		....
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

	if ((millis() - atimer1) >= (unsigned 	long) TLIMITE1)
	{
		....
		// istruzioni eseguite allo scadere del timer 1
	}
}
```
Analogamente si potrebbe eseguire un blocco di codice continuamente **solo** all’interno di un tempo massimo prestabilito, cioè solo **prima** dello scadere del timer:
```C++
//Timer aperiodico 2
#define TLIMITE  1000
unsigned long atimer2;
void loop()
{
	// condizione che valuta l’accadere di un certo evento
	// di attivazione del conteggio del tempo
	if(condA){
		atimer2 = millis();
	}

	if ((millis() – atimer2) < (unsigned long) TLIMITE2)
	{
		....
		// istruzioni eseguite finchè NON scade il timer 2
	}
}
```
I timers possono anche essere in funzione **all’interno di funzioni** che sono richiamate all’interno del loop():
```C++
void loop()
{
	poll();  // funzione con **blocchi** di codice eseguiti **prima** o **dopo** di certi eventi
	…….
}

void poll()
{
	if(condA){ // evento che attiva il timer (può essere ricorrente ma, in generale, è aperiodico)
		atimer1 = millis();
	}

	// finchè non si raggiunge TLIMITE1 ritorna senza fare nulla
	if ((millis() – atime1) < (unsigned long) TLIMITE1) return;
	//BLOCCO_A

	// finchè non si raggiunge TLIMITE2 ritorna dopo avere eseguito il blocco di istruzioni A
	if ((millis() – atimer2) < (unsigned long) TLIMITE2) return;
	//BLOCCO_B

	// finchè non si raggiunge TLIMITE3 ritorna dopo avere eseguito il blocco di istruzioni

	// A e B
	if ((millis() – atimer1) < (unsigned long) TLIMITE3) return;
}
```
Attenzione ad un **errore insidioso**:
```C++
void poll()
{
	if ((millis() – atimer1) < (unsigned long) TLIMITE1)
	{
	....
	// istruzioni eseguite finchè NON scade il timer 1
	}
	if ((millis() – atimer2) < (unsigned long) TLIMITE2)
	{
	....
	// istruzioni eseguite finchè NON scade il timer 2
	}
}
```

In questa situazione se scade il primo timer **viene comunque controllato** lo scadere del secondo. La cascata degli if equivale ad un OR logico sulle condizioni di scadenza.

Se voglio che **ne accada solo una** posso scrivere così:
```C++
void poll()
{
	if ((millis() – atimer1) < (unsigned long) TLIMITE1)
	{
		....
		// istruzioni eseguite finchè NON scade il timer 1
	}else if ((millis() – atimer2) < (unsigned long) TLIMITE2)
	{

....

// istruzioni eseguite finchè NON scade il timer 2

}

}

Dove **l’ordine** stabilisce la **priorità** di valutazione.

Alcuni codici, ritenendo un evento disabilitante per tutto il resto della funzione, talvolta fanno così:

void poll()

{

if ((millis() – atimer1) < (unsigned long) TLIMITE1)

{

....

// istruzioni eseguite finchè NON scade il timer 1

**return 0;** // NON SONO PRONTO! RICONTROLLA AL PROSSIMO GIRO…

}

// ISTRUZIONI IMPORTANTI SI MA CHE…QUALCHE VOLTA…NON SI FANNO…

}

**Reset** del timer, **polling** del tempo trascorso e **istruzioni triggerate** (scatenate) dal timer potrebbero anche essere rinchiuse in altrettante **funzioni**:

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

.......

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

**POLLED TIMERS SCHEDULATI**

Si possono realizzare timer **anche** a partire dalla **base dei tempi** misurata da uno **schedulatore** semplicemente **contando i passi** raggiunti finchè questi non arrivano ad un valore **target**, **prima** del quale, o **dopo** il quale, far **accadere** qualcosa.

Il codice di seguito fa partire un comando alla pressione di un pulsante solo se questo è stato **premuto per un tempo minimo**, in caso contrario non fa nulla (filtra i comandi)

unsigned long lastTime = 0;

unsigned long timerDelay = TBASE;  // send readings timer

unsigned step = 0;

bool start=false;

unsigned short  val;

byte precval=0; //switchdf e toggle

byte cmdin=2;

void loop() {

if ((millis() - lastTime) > timerDelay) {

lastTime = millis();

step = (step + 1) % NSTEP;

btntime = (btntime + 1) % NSTEP;

val = !digitalRead(cmdin)); // pulsante pull up

if(switchdf(val)){ //rivelatore di fronte (salita e discesa)

Serial.println("Ho una transizione dell'ingresso");

if(val == HIGH){ // ho un fronte di salita

if(start==true){

start = false;

Serial.println("Ho filtrato un comando");

}else{

start = true;

Serial.println("Ho abilitato un comando");

}

}

btntime = 0;

}

// se premo il pulsante sufficientemente a lungo accendi il led

if(start && (btntime >= txtime)){

digitalWrite(led));

start = false;

}

}

//switch per un solo pulsante attivo su entrambi i fronti

bool switchdf(byte val){

bool changed = false;

changed = (val != precval); // campiona tutte le transizioni

precval = val;  // valore di val campionato al loop precedente

return changed;  // rivelatore di fronte (salita o discesa)

}
<!--stackedit_data:
eyJoaXN0b3J5IjpbMTIxNDU4MDkxMCwtNzA3MjI1MTgyXX0=
-->