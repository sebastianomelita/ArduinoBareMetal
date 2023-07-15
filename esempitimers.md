>[Torna a polled timers](polledtimer_event.md) >[versione in Python](catenetimerspy.md)

## **ESEMPI TIMERS**

### **Accensione monostabile**

```C++
/*
Scrivere un programma che realizzi l'accensione di un led tramite un pulsante temporizzato che spegne il led 
dopo un numero di ms impostati da setup. La logica del tasto deve essere senza stato e deve essere sensibile 
al fronte di salita del segnale. Immaginare un collegamento pull down del tasto.
*/
//inizio variabili per un solo pulsante
byte precval;
//fine variabili per un solo pulsante
//inizio variabili timer
unsigned long startTime;
unsigned long timelapse;
byte timerState=0;
//fine variabili timer
int led=13;
int tasto=2;
int in, out;

//switch per un solo pulsante attivo su entrambi i fronti
bool switchdf(byte val){
	bool changed = false;
	changed = (val != precval); 	// campiona tutte le transizioni
	precval = val;              	// valore di val campionato al loop precedente 
	return changed; 				// rivelatore di fronte (salita o discesa)
}

void startTimer(unsigned long duration){
	timerState=1;
	timelapse=duration;
	startTime=millis();
}

void stopTimer(){
	timerState=0;
}

byte getTimerState(){
	return timerState;
}

//verifica se è arrivato il tempo di far scattare il timer
void aggiornaTimer(){
	if((millis()-startTime >= timelapse) && timerState==1){
		timerState=0;
		onElapse();
	}
}	

void onElapse(){
	digitalWrite(led,LOW);
}

void setup(){
	pinMode(led,OUTPUT);
	pinMode(tasto,INPUT);
	digitalWrite(led,LOW);
	digitalWrite(tasto,LOW);
	in=LOW; //inizializzazione stato tasto
}

void loop(){
	aggiornaTimer();
	in=digitalRead(tasto);
	if(switchdf(in)){ // selezione di un fronte qualsiasi
		if(in==HIGH){ // selezione del fronte di salita (in==HIGH)
			digitalWrite(led,HIGH);
			startTimer(5000);
		}
	}
}
```	
Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/fCpauVnNUZh?editbtn=1

### **Un pulsante per tre lampade**

```C++
/*
Scrivere un programma che realizzi l'accensione di un led, due led o tre led tramite la pressione consecutiva di un pulsante 
una, due o tre volte all'interno di un intervallo temporale di un secondo.
Col la pressione prolungata dello stesso pulsante si spengono tutti e tre contemporaneamente.
(Realizzazione del timer senza schedulatore)
*/

//inizio variabili e costanti dello schedulatore (antirimbalzo)
#define tbase    	50        // periodo base in millisecondi
unsigned long prec=0;
//fine variabili e costanti dello schedulatore

//inizio variabili timer
//da impostare con dimensione pari al numero di timer previsti (ad es. 2)
unsigned long startTimeN[2];
unsigned long timelapseN[2]; 
byte timerStateN[2];
byte in;
//fine variabili timer

//inizio variabili switch
byte precval=0;
//fine variabili switch
int led1=13;
int led2=12;
int led3=11;
int tasto1=2;

int count=0;

//fissa l'istante iniziale da cui far partire il timer
void stopTimer(int n){
	//n: numero del timer
	timerStateN[n]=0;
}

//fissa l'istante iniziale da cui far partire il timer
void startTimer(unsigned long duration, byte n){
	timerStateN[n]=1;
	timelapseN[n]=duration;
	startTimeN[n]=millis();
}

//verifica se è arrivato il tempo di far scattare il timer
void aggiornaTimer(int n){
	//n: numero del timer
	if((millis()-startTimeN[n] >= timelapseN[n]) && timerStateN[n]==1){
		timerStateN[n]=0;
		onElapse(n);
	}
}	

byte getTimerState(int n){
	return timerStateN[n];
}
	
bool switchdf(byte val){
	bool changed=false;
	if (val != precval){ //campiona tutte le transizioni
		changed=true;
	}   
	precval = val;            // valore di val campionato al loop precedente 
	return changed;
}

	
void setup(){
	pinMode(tasto1,INPUT);
	pinMode(led1,OUTPUT);
	pinMode(led2,OUTPUT);
	pinMode(led3,OUTPUT);
	digitalWrite(led1,LOW);
	digitalWrite(led2,LOW);
	digitalWrite(led3,LOW);
}

//azione da compiere allo scadere del timer
void onElapse(int n){
	if(n==0){ //timer di conteggio
		if(count>0){
			digitalWrite(14-count,HIGH);
			count=0;
		}
	}else if(n==1){ //timer di spegnimento
		//dall'ultima pressione al rilascio corrente è passato più di un secondo
		//spengo tutto
		digitalWrite(led1,LOW);
		digitalWrite(led2,LOW);
		digitalWrite(led3,LOW);
		count=0;
	}
}

void loop(){
	aggiornaTimer(0);  //timer di conteggio pressioni
	aggiornaTimer(1);  //timer di spegnimento
	
	if((millis()-prec) > tbase) //schedulatore con funzione di antirimbalzo (legge ogni 50 mSec)
	{
		prec = millis(); 
		
		//legge valore attuale dell'ingresso
		in=digitalRead(tasto1);
		//valuto se con il valore attuale si è su un fronte qualsiasi (saita o discesa)
		if(switchdf(in)==HIGH){
			if(in>0){//fronte di salita
				startTimer(1000,1); //parte il timer di spegnimento
				count++;
				if(count==1)
					startTimer(1000,0); //timer di conteggio pressioni
			}else{//fronte di discesa
				//azzero timer di spegnimento
				stopTimer(1);
			}
		}
	}
}
```
Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/f0o52ZDqqcL?editbtn=1

### **Scegli chi on/off**

```C++
/*
 Scrivere un programma che realizzi l'accensione del led1, oppure del led2 oppure del led3 led 
 tramite la pressione consecutiva di un pulsante una, due o tre volte all'interno di 
 un intervallo temporale di un secondo. Col la pressione per almeno un secondo, ma meno di due, dello stesso pulsante si 
 accendono tutti i led, con la pressione dello stesso tasto per più di due secondi si spengono tutti i led.
 (Realizzazione del timer senza schedulatore, tast)
*/

//inizio variabili e costanti dello schedulatore (antirimbalzo)
#define tbase    	50        // periodo base in millisecondi
unsigned long precm=0;
//fine variabili e costanti dello schedulatore

//inizio variabili timer
//inizio variabili timer
unsigned long startTime[3];
unsigned long timelapse[3];
byte timerState[3];
//fine variabili timer
//variabili switch2
int precval;
//fine variabili switch2

int led1=13;
int led2=12;
int led3=11;
int tasto1=2;
int count=0;
int in, out;

void startTimer(unsigned long duration, int n){
	timerState[n]=1;
	timelapse[n]=duration;
	startTime[n]=millis();
}

//verifica se è arrivato il tempo di far scattare il timer
void aggiornaTimer(int n){
	if((millis()-startTime[n] > timelapse[n]) && timerState[n]==1){
		timerState[n]=0;
		onElapse(n);
	}
}

void stopTimer(int n){
	timerState[n]=0;
}

//switch per un solo pulsante attivo su entrambi i fronti
bool switchdf(byte val){
	bool changed = false;
	changed = (val != precval); 	// campiona tutte le transizioni
	precval = val;              	// valore di val campionato al loop precedente 
	return changed;					// rivelatore di fronte (salita o discesa)
}

//azione da compiere allo scadere del timer	
void onElapse(int n){
	//se c'è un conteggio di accensione attivo accendi il led corrispondente al numero raggiunto
	if(n==0){	
		if(count>0){
			digitalWrite(14-count,HIGH);
			count=0;
		}
	}else if(n==1){
		//dall'ultima pressione (startTimer(1)) al rilascio corrente è passato più di un secondo ma meno di due
		digitalWrite(led1,HIGH);
		digitalWrite(led2,HIGH);
		digitalWrite(led3,HIGH);
	}else if(n==2){
		//dall'ultima pressione (startTimer(1)) al rilascio corrente sono passati più di due secondi
		digitalWrite(led1,LOW);
		digitalWrite(led2,LOW);
		digitalWrite(led3,LOW);
	}
}

void setup(){
	pinMode(tasto1,INPUT);
	pinMode(led1,OUTPUT);
	pinMode(led2,OUTPUT);
	pinMode(led3,OUTPUT);
	digitalWrite(led1,LOW);
	digitalWrite(led2,LOW);
	digitalWrite(led3,LOW);
}

void loop(){
	aggiornaTimer(0);
	aggiornaTimer(1);
    aggiornaTimer(2);
	
	if((millis()-precm) > tbase) //schedulatore con funzione di antirimbalzo (legge ogni 50 mSec)
	{
		precm = millis(); 
		
		//ad ogni pressione del tasto entro il tempo prefissato aggiorna il contatore di quel tasto (pressione)
		in=digitalRead(tasto1);
		out=switchdf(in);
		if(out==HIGH){
			if(in==HIGH){//fronte di salita
				//ad ogni pressione fai partire i timers di rilascio
				count++;
				startTimer(1000,1);
				startTimer(2000,2);
				//se il tasto di accensione è premuto la prima volta fai partire il timer di conteggio
				if(count==1){
					startTimer(1000,0);
				}
			}else{ //fronte di discesa
				stopTimer(1);
				stopTimer(2);
			}
		}
	}
}
```
Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/jCV9olOVsBk?editbtn=1

### **Scegli chi on/off un tasto**

```C++
/*
 Scrivere un programma che realizzi l'accensione del led1, oppure del led2 oppure del led3 led 
 tramite la pressione consecutiva di un pulsante una, due o tre volte all'interno di 
 un intervallo temporale di un secondo. Col la pressione per almeno un secondo, ma meno di due, dello stesso pulsante si 
 accendono tutti i led, con la pressione dello stesso tasto per più di due secondi si spengono tutti i led.
 (Realizzazione del timer senza schedulatore, tast)
*/

//inizio variabili e costanti dello schedulatore (antirimbalzo)
#define tbase    	50        // periodo base in millisecondi
unsigned long precm=0;
//fine variabili e costanti dello schedulatore

//inizio variabili timer
//inizio variabili timer
unsigned long startTime[3];
unsigned long timelapse[3];
byte timerState[3];
//fine variabili timer
//variabili switch2
int precval;
//fine variabili switch2

int led1=13;
int led2=12;
int led3=11;
int tasto1=2;
int count=0;
int in, out;

void startTimer(unsigned long duration, int n){
	timerState[n]=1;
	timelapse[n]=duration;
	startTime[n]=millis();
}

//verifica se è arrivato il tempo di far scattare il timer
void aggiornaTimer(int n){
	if((millis()-startTime[n] > timelapse[n]) && timerState[n]==1){
		timerState[n]=0;
		onElapse(n);
	}
}

void stopTimer(int n){
	timerState[n]=0;
}

//switch per un solo pulsante attivo su entrambi i fronti
bool switchdf(byte val){
	bool changed = false;
	changed = (val != precval); 	// campiona tutte le transizioni
	precval = val;              	// valore di val campionato al loop precedente 
	return changed;					// rivelatore di fronte (salita o discesa)
}

//azione da compiere allo scadere del timer	
void onElapse(int n){
	//se c'è un conteggio di accensione attivo accendi il led corrispondente al numero raggiunto
	if(n==0){	
		if(count>0){
			digitalWrite(14-count,HIGH);
			count=0;
		}
	}else if(n==1){
		//dall'ultima pressione (startTimer(1)) al rilascio corrente è passato più di un secondo ma meno di due
		digitalWrite(led1,HIGH);
		digitalWrite(led2,HIGH);
		digitalWrite(led3,HIGH);
	}else if(n==2){
		//dall'ultima pressione (startTimer(1)) al rilascio corrente sono passati più di due secondi
		digitalWrite(led1,LOW);
		digitalWrite(led2,LOW);
		digitalWrite(led3,LOW);
	}
}

void setup(){
	pinMode(tasto1,INPUT);
	pinMode(led1,OUTPUT);
	pinMode(led2,OUTPUT);
	pinMode(led3,OUTPUT);
	digitalWrite(led1,LOW);
	digitalWrite(led2,LOW);
	digitalWrite(led3,LOW);
}

void loop(){
	aggiornaTimer(0);
	aggiornaTimer(1);
    aggiornaTimer(2);
	
	if((millis()-precm) > tbase) //schedulatore con funzione di antirimbalzo (legge ogni 50 mSec)
	{
		precm = millis(); 
		
		//ad ogni pressione del tasto entro il tempo prefissato aggiorna il contatore di quel tasto (pressione)
		in=digitalRead(tasto1);
		out=switchdf(in);
		if(out==HIGH){
			if(in==HIGH){//fronte di salita
				//ad ogni pressione fai partire i timers di rilascio
				count++;
				startTimer(1000,1);
				startTimer(2000,2);
				//se il tasto di accensione è premuto la prima volta fai partire il timer di conteggio
				if(count==1){
					startTimer(1000,0);
				}
			}else{ //fronte di discesa
				stopTimer(1);
				stopTimer(2);
			}
		}
	}
}
```
Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/e55nCkLiALh?editbtn=1

>[Torna a polled timers](polledtimer_event.md) >[versione in Python](catenetimerspy.md)
