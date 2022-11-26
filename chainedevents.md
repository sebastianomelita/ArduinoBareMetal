>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  

### **SCHEDULAZIONE CON I DELAY**

```C++
/*
Realizzzare un programma che fa blinkare un led con periodo mezzo secondo per 5 sec e poi lo fa lo blinkare 
con periodo 300 msec per 3 sec, poi ricomincia d'accapo.
*/
byte led1 = 13;

void setup()
{
	pinMode(led1, OUTPUT);
}

void loop()
{
	// task 1
	for(int i=0; i<10; i++){ //5000/500-->10
		digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		delay(500);
	}
	// task 2
	for(int i=0; i<10; i++){ //3000/300-->10
		digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		delay(300);
	}
}

```
Link simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/2GGetlCbg5R?editbtn=1

Link simulazione su ESP32 con Wowki: https://wokwi.com/projects/349386232933909076

Variante che usa la capacità di iterare intrinseca della funzione loop():

```C++
/*
Realizzzare un programma che fa blinkare un led con periodo mezzo secondo per 5 sec e poi lo fa lo blinkare 
con periodo 300 msec per 3 sec, poi ricomincia d'accapo.
*/
byte led1 = 13;
int i;

void setup()
{
	pinMode(led1, OUTPUT);
	i=0;
}

void loop()
{
	// task 1
	if(i<10){
			digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
			delay(500);
			i++;
	}
	// task 2
	else if(i<20){
			digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
			delay(300);
			i++;
	}else{
		i=0;
	}
}
```

Link simulazione su Arduino con Tinkercad:https://www.tinkercad.com/embed/apuBTacgbFs?editbtn=1

Link simulazione su ESP32 con Wowki: https://wokwi.com/projects/349386232933909076

### **SCHEDULAZIONE CON I TIME TICK**

```C++
/*
Realizzzare un programma che fa blinkare un led con periodo mezzo secondo per 5 sec e poi lo fa lo blinkare con periodo 300 msec per 3 sec, poi ricomincia d'accapo.
*/
#define TBASE  100 // periodo base in milliseconds
#define NSTEP  1000  // numero di fasi massimo di un periodo generico
unsigned long precm = 0;
unsigned long step = 0;
byte led1 = 13;
bool stato;

void setup()
{
	pinMode(led1, OUTPUT);
	stato = true;
}

void loop()
{
	// polling della millis() alla ricerca del tempo in cui scade ogni periodo
	if((millis()-precm) >= (unsigned long) TBASE){ 		//se è passato un periodo tbase dal precedente periodo
		precm = millis();  				//preparo il tic successivo azzerando il conteggio del tempo ad adesso
		
		step = (step + 1) % NSTEP; 			// conteggio circolare (arriva al massimo a nstep-1)
	
		// task 1
		if(!(step%5)){  // schedulo eventi al multiplo del periodo (2 sec = 2 periodi)
			if(stato)
				digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		}
		// task 2
		if(!(step%3)){  // schedulo eventi al multiplo del periodo (2 sec = 2 periodi)
			if(!stato)
				digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		}

		if(step <= 50){  //0 < t < 500 msec
			stato = true;
		}else if(step <= 80){ //500 msec < t < 800 msec
			stato = false;// t >= 800 msec
		}else{
			step = 0;
			stato = true;
			//digitalWrite(led1,LOW);
		}
		// il codice eseguito al tempo del metronomo (esattamente un periodo) va quì
	}
	//delay(1);
	// il codice eseguito al tempo massimo della CPU va qui
}

```

Link simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/fn66P9O8oJG?editbtn=1

Link simulazione su ESP32 con Wowki: https://wokwi.com/projects/349383452039053908


>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  
