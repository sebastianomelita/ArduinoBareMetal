
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
