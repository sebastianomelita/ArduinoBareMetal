>[Torna all'indice](indextimers.md) >[versione in Python](timersschedulatipy.md)
## **POLLED TIMERS SCHEDULATI**

Si possono realizzare timer anche a partire dalla base dei tempi misurata da uno schedulatore semplicemente contando i passi raggiunti finchè questi non arrivano ad un valore target, prima del quale, o dopo il quale, far accadere qualcosa.

Il codice di seguito accende un led alla pressione di un pulsante solo se questo è stato premuto per un tempo minimo, oppure lo spegne solo se questo è rilasciato per almeno un tempo minimo, in tutti gli altri casi non fa nulla (filtra i comandi)
```C++
#define TBASE 10
#define NSTEP 1000
unsigned long lastTime = 0;  
unsigned long timerDelay = TBASE;  // send readings timer
unsigned step = 0;  
unsigned btntime = 0;
unsigned txtime = 100;
bool start;
unsigned short  val;
byte precval=0; //switchdf e toggle
byte cmdin, led;

//switch per un solo pulsante attivo su entrambi i fronti
bool transizione(byte val){
	bool changed = false;
	changed = (val != precval); 	// campiona tutte le transizioni
	precval = val;              	// valore di val campionato al loop precedente 
	return changed;		// rivelatore di fronte (salita o discesa)
}

void setup(){
	cmdin = 3;
	led = 10;
	start=false;
	pinMode(cmdin, INPUT);
	pinMode(led, OUTPUT);
	Serial.begin(115200);
}

void loop() {
	if ((millis() - lastTime) > timerDelay) {
		lastTime = millis();
		step = (step + 1) % NSTEP;
		btntime = (btntime + 1) % NSTEP;
		
		val = digitalRead(cmdin); 
		
		if(transizione(val)){ 	//rivelatore di fronte (salita e discesa)
			Serial.println("Ho una transizione dell'ingresso");
			if(start==true){
				start = false;
				Serial.println("Ho filtrato un comando");
			}else{
				start = true;
				Serial.println("Ho abilitato un comando");
			}
		    	btntime = 0;
		}
		
		// se premo il pulsante sufficientemente a lungo accendo il led
		// se rilascio il pulsante sufficientemente a lungo spengo il led
		if(start && (btntime >= txtime)){
			digitalWrite(led, val);
			start = false; //consento l'abilitazione del comando opposto
		}
	}
}
```
Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/8DGq6Uro9uQ?editbtn=1

>[Torna all'indice](indextimers.md) >[versione in Python](timersschedulatipy.md)
