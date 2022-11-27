
>[Torna all'indice](indexpulsanti.md) >[versione in Python](gruppipulsantipy.md)

##  **NASTRO TRASPORTATORE**

###  **Gestione di un solo nastro**

Programma per la gestione di un nastro trasportatore realizzato con un timer basato sul polling della funzione ```millis()``` e su rilevatori di transito toggle basati su ```delay()```.

```C++
/*
Scrivere un programma che realizzi la gestione di un nastro traportatore attraverso la lettura di tre sensori di transito e il comando di un motore.
I sensori permangono al livello alto finchè un oggetto ingombra l'area del sensore, dopodichè vanno a livello basso. I sensori sono due all'inizio del nastro (uno per i pezzi bassi ed un'altro per quelli alti) ed uno alla fine del nastro che rileva il pezzo pronto per essere prelevato.
All'attivazione di un qualsiasi sensore di ingresso parte il motore e si resetta (blocca) il timer di volo.
All'attivazione del sensore di uscita si blocca il nastro, alla sua disattivazione riparte il nastro e parte il timer di volo.
Allo scadere del timer di volo si spegne il motore.
*/
byte startSensorHigh = 4;
byte startSensorLow = 3;
byte stopSensor = 2;
byte engineLed = 10;
byte lowStartLed = 9;
byte highStartLed = 8;
byte stopLed = 11;
unsigned flyTime = 4000; //tempo di volo di un pezzo sul nastro
bool engineon;  // variabile globale che memorizza lo stato del motore
//inizio variabili timer
unsigned long startTime;
unsigned long timelapse;
byte timerState=0;
//fine variabili timer

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
	engineon = false; 
	digitalWrite(engineLed, LOW);
	Serial.println("Timer di volo scaduto");
}

// attesa evento con tempo minimo di attesa
void waitUntilInputLow(int btn, unsigned t)
{
    while(!digitalRead(btn)==LOW){
	    delay(t);
    }
}
  
void setup() {
  Serial.begin(115200);
  pinMode(engineLed, OUTPUT);
  pinMode(lowStartLed, OUTPUT);
  pinMode(highStartLed, OUTPUT);
  pinMode(stopLed, OUTPUT);
  pinMode(startSensorHigh, INPUT);
  pinMode(startSensorLow, INPUT);
  pinMode(stopSensor, INPUT); 
  engineon= false;
  stopTimer();
}

// loop principale
void loop() {
	aggiornaTimer();
	if(digitalRead(startSensorLow)==HIGH){				// se è alto c'è stato un fronte di salita
		engineon = true; 	
		digitalWrite(engineLed, HIGH);
		digitalWrite(lowStartLed, HIGH);
		stopTimer();									// c'è almeno un pezzo in transito
		Serial.println("Pezzo basso in ingresso");
		Serial.println("Timer di volo disattivato");
		waitUntilInputLow(startSensorLow,50);			// attendi finchè non c'è fronte di discesa
		Serial.println("Pezzo basso transitato in ingresso");
		digitalWrite(lowStartLed, LOW);
	}if(digitalRead(startSensorHigh)==HIGH){			// se è alto c'è stato un fronte di salita
		engineon = true; 	
		digitalWrite(engineLed, HIGH);
		digitalWrite(highStartLed, HIGH);
		stopTimer();									// c'è almeno un pezzo in transito
		Serial.println("Pezzo alto in ingresso");
		Serial.println("Timer di volo disattivato");
		waitUntilInputLow(startSensorHigh,50);			// attendi finchè non c'è fronte di discesa
		Serial.println("Pezzo alto transitato in ingresso");
		digitalWrite(highStartLed, LOW);
	}else if(digitalRead(stopSensor)==HIGH) {
		engineon = false; 		
		digitalWrite(engineLed, LOW);
		digitalWrite(stopLed, HIGH);
		Serial.println("Pezzo in uscita");
		waitUntilInputLow(stopSensor,50);
		Serial.println("Pezzo prelevato dall'uscita");
		engineon = true; 
		digitalWrite(stopLed, LOW);
		digitalWrite(engineLed, HIGH);
		startTimer(flyTime); 						// se c'è un pezzo in transito arriverà prima dello scadere
		Serial.println("Timer di volo attivato");
	} else {
		
	}
}
```
Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/8UKvLNjeLEQ?editbtn=1

>[Torna all'indice](indexpulsanti.md) >[versione in Python](gruppipulsantipy.md)
