>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in Python](taskschedpy.md)

## **SCHEDULATORE COMPITI BASATO SUL CONTEGGIO DEI TICK**

Di seguito è riportato un esempio di schedulatore che pianifica nel tempo **l’esecuzione periodica** di una serie di **compiti** (task) da programmare **nel futuro** con **cadenza diversa**.

Il **tempo base** è la base dei tempi di tutte le schedulazioni, viene calcolato **periodicamente** all'accadere di un **evento** (superamento di una soglia di tempo) che viene detto **tick** (analogia con il metronomo per la musica).

Le varie schedulazioni **sono calcolate** a partire da un **multiplo intero** del tempo base, ne segue che il tempo base dovrebbe essere calcolato come il massimo comune divisore (**MCD**) di tutti i tempi che devono essere generati.

Il conteggio dei multipli del tempo base è tenuto da un **contatore circolare** (step) che deve essere **ruotato** dopo aver effettuato un numero di conteggi superiori al **massimo dei multipli** del tempo base necessari.

Se ci sono **pulsanti** da gestire insieme ad altri task il tempo base può essere impostato tra 50 e 200 mSec in maniera da poterlo utilizzare per effettuare un **polling degli ingressi** immune dal fenomeno dei rimbalzi (**antibounce SW**). Per una descrizione dettagliata dell'uso della millis() per generare i time tick si rimanda a [tecniche di schedulazione](timesched.md#SCHEDULATORE-CON-TIMER-TICK-GENERATI-DA-MILLIS())).
```C++
#define tbase  1000  // periodo base in milliseconds
#define nstep  1000  // numero di fasi massimo di un periodo generico
unsigned long precm = 0;
unsigned long step = 0;
byte pari, in;
byte led1 = 13;
byte led2 = 12;

void setup()
{
	pinMode(led1, OUTPUT);
    pinMode(led2, OUTPUT);
}

void loop()
{
	// polling della millis() alla ricerca del tempo in cui scade ogni periodo
	if((millis()-precm) >= (unsigned long) tbase){ 		//se è passato un periodo tbase dal precedente periodo
		precm = millis();  				//preparo il tic successivo azzerando il conteggio del tempo ad adesso
		
		step = (step + 1) % nstep; 			// conteggio circolare (arriva al massimo a nstep-1)

		// task 1
		if(!(step%2)){  // schedulo eventi al multiplo del periodo (2 sec = 2 periodi)
			digitalWrite(led1,!digitalRead(led1)); 	// stato alto: led blink
		}
		// task 2
		if(!(step%3)){  // schedulo eventi al multiplo del periodo (3 sec = 3 periodi)
			digitalWrite(led2,!digitalRead(led2)); 	// stato alto: led blink
		}
		// il codice eseguito al tempo del metronomo (esattamente un periodo) va quì
	}
	// il codice eseguito al tempo massimo della CPU va qui
}
```
Di seguito il link della simulazione online con Tinkercad su Arduino: https://www.tinkercad.com/embed/81ioQDDGQOG?editbtn=1

Di seguito il link della simulazione online con Wowki su esp32: https://wokwi.com/projects/348709453878526548


E’ buona norma evitare l’esecuzione frequente di operazioni lente quando queste non sono strettamente necessarie in modo da lasciare spazio ad altre operazioni, quali ad esempio gestione degli eventi di input, che richiedono una velocità maggiore per usufruirne in modo più interattivo.

**In particolare**, all'interno di un task, andrebbe accuratamente evitata l'introduzione di ritardi prolungati tramite **delay()**. Possono esserci per **breve tempo** (qualche mSec) ma, in ogni caso, non dovrebbero mai far parte della maniera (**algoritmo**) con cui la logica di comando **pianifica i suoi eventi**.

Utilizzando la tecnica della **schedulazione esplicita dei task** nel loop(), la **realizzazione di un algoritmo** non dovrebbe essere pensata in **maniera sequenziale (o lineare)** ma, piuttosto, come una **successione di eventi** a cui corrispondono dei **compiti** (task) che **comunicano** con le periferiche di input/output ed, eventualmente, dialogano tra loro in qualche modo (ad es. variabili globali, buffer, ecc.).

## **Esempi**

**Blink a fasi**

```C++
/*
Realizzzare un programma che fa blinkare un led per 5 sec poi lo fa stare sempre acceso per un altri 5 sec, poi lo fa blinkare di nuovo per altri 5 sec e così via.
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
Di seguito il link della simulazione online con Tinkercad su Arduino: https://www.tinkercad.com/embed/0vP4WlJGycZ?editbtn=1
				      
>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in Python](taskschedpy.md)
