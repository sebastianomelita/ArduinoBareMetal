>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  

### **SCHEDULAZIONE CON I DELAY**

```C++

```


### **SCHEDULAZIONE CON I TIME TICK**

```C++
/*
Realizzzare un programma che fa blinkare un led con periodo mezzo secondo per 5 sec e poi lo fa lo blinkare con periodo 100 msec per 3 sec, poi ricomincia d'accapo.
*/
#define tbase  500 // periodo base in milliseconds
#define nstep  1000  // numero di fasi massimo di un periodo generico
unsigned long precm = 0;
unsigned long step = 0;
byte led1 = 12;
bool stato;

void setup()
{
	pinMode(led1, OUTPUT);
	stato = true;
}

void loop()
{
	// polling della millis() alla ricerca del tempo in cui scade ogni periodo
	if((millis()-precm) >= (unsigned long) tbase){ 		//se è passato un periodo tbase dal precedente periodo
		precm = millis();  				//preparo il tic successivo azzerando il conteggio del tempo ad adesso
		
		step = (step + 1) % nstep; 			// conteggio circolare (arriva al massimo a nstep-1)

		// task 1
		if(!(step%1)){  // schedulo eventi al multiplo del periodo (2 sec = 2 periodi)
			if(stato)
				digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
			else
				digitalWrite(led1,LOW);
		}
		// task 2
		if(!(step%10)){  // schedulo eventi al multiplo del periodo (3 sec = 3 periodi)
			stato = !stato;
		}
		// il codice eseguito al tempo del metronomo (esattamente un periodo) va quì
	}
	// il codice eseguito al tempo massimo della CPU va qui
}
```

Link simulazione Tinkercad: https://www.tinkercad.com/embed/7AsMjhD1Mk9?editbtn=1

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  
