>[Torna all'indice](indextimers.md)
## **POLLED TIMERS SCHEDULATI**

Si possono realizzare timer anche a partire dalla base dei tempi misurata da uno schedulatore semplicemente contando i passi raggiunti finchè questi non arrivano ad un valore target, prima del quale, o dopo il quale, far accadere qualcosa.
Il codice di seguito fa partire un comando alla pressione di un pulsante solo se questo è stato premuto per un tempo minimo, in caso contrario non fa nulla (filtra i comandi)
```C++
unsigned long lastTime = 0;  
unsigned long timerDelay = TBASE;  // send readings timer
unsigned step = 0;  
bool start=false;
unsigned short  val;
byte precval=0; //switchdf e toggle
byte cmdin=2;

//switch per un solo pulsante attivo su entrambi i fronti
bool transizione(byte val){
	bool changed = false;
	changed = (val != precval); 	// campiona tutte le transizioni
	precval = val;              	// valore di val campionato al loop precedente 
	return changed;		// rivelatore di fronte (salita o discesa)
}

void loop() {
	if ((millis() - lastTime) > timerDelay) {
		lastTime = millis();
		step = (step + 1) % NSTEP;
		btntime = (btntime + 1) % NSTEP;
		
		val = !digitalRead(cmdin)); // pulsante pull up
		
		if(transizione(val)){ 	//rivelatore di fronte (salita e discesa)
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
```
>[Torna all'indice](indextimers.md)
