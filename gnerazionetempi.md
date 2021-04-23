

>[Torna all'indice generale](index.md)   >[Versione in Python](generazionetempipy.md)
# **GENERAZIONE DI TEMPI**

## **DELAY() E MILLIS()**

**Delay()**

Mette in pausa il programma per la quantità di tempo (in millisecondi) specificata come parametro. (Ci sono 1000 millisecondi in un secondo).

**Millis()**

Restituisce il numero di millisecondi trascorsi da quando la scheda Arduino ha iniziato a eseguire il programma corrente. Questo numero supererà (tornerà a zero), dopo circa 50 giorni (overflow).

Il tipo di dato ritornato dalla funzione è unsigned long.

Bisogna prestare attenzione alle **operazioni aritmetiche** con **espressioni** che comprendono, oltre a millis() anche altre variabili aventi tipi più piccoli, come ad esempio int. Infatti la capacità di conteggio di variabili di dimensione inferiore ad unsigned long è minore di quella di millis() per cui potrebbero andare in overflow in un momento differente rispetto a millis() generando risultati scorretti.

Le misure di tempo devono sempre tenere conto del problema dell’overflow.  Sulle misure di tempo assolute (misurate a partire dall’accensione della macchina) si può fare poco, quelle andranno in overflow comunque, su quelle relative tra istanti campionati (e conservati su una variabile) e quelli misurati con la millis(), è possibile renderlo non dannoso. Il segreto è lavorare sempre **su differenze di tempo** che non superano mai il valore di overflow, ciò si ottiene assicurandosi di **ricampionare il valore del riferimento**, a partire da cui si misura la differenza, prima che l'evento di overflow accada. 

Se si considera la differenza:
```C++
	millis()-precm
```
se precm è dello stesso tipo di millis(), cioè è di tipo unsigned long, allora sia l’una che l’altra andranno in overflow e in istanti di tempo diversi. Per effetto di un evento di overflow i valori assoluti delle variabili perderanno di significato mentre quelli relativi continueranno a mantenerlo infatti, nonostante le due variabili vadano in overflow in momenti diversi, la differenza relativa si mantiene ugualmente corretta. 

Ciò accade perché l’operazione di sottrazione delle variabili senza segno nei computer avviene modulo il valore massimo di conteggio della variabile.

Infatti si può verificare che il programma seguente restituisce il valore 2.
```C++
	void setup() {
		Serial.begin(115200);
		unsigned long a = 1;
		unsigned long b = 4294967295; //unsigned long maximum value
		Serial.println(a-b);
	}

	void loop() {
	}
```

## **ESEMPI DI SCHEDULATORI**

**Schedulatore con delay().**
```C++
	byte led = 13;
	byte pulsante = 2;
	void setup()
	{
		pinMode(led, OUTPUT);
		pinMode(pulsante, INPUT);
	}

	void loop()
	{
		//codice eseguito al tempo stabilito
		in = digitalRead(pulsante);  //pulsante collegato in pulldown
		// Inserire qui la logica di comando
		//……………………………………
		delay(100);  // ritarda del tempo necessario
	}
```
La generazione di più task in tempi diversi risulta **molto semplice** se eseguita con la funzione delay() soprattutto per la natura **lineare** della programmazione che permette di scrivere il codice dei task con lo **stesso ordine** con cui questi verranno effettivamente eseguiti **nel tempo**. Lo svantaggio di questa realizzazione è una sostanziale **inefficienza** perchè il **blocco di un task** comporta anche il blocco di **tutti gli altri**, compreso il programma principale (**main**). Riassumendo, la **schedulazione mediante delay** comporta:
- **vantaggio**. Programmazione lineare molto semplice
- **svantaggio**. Inefficienza intrinseca nell'esecuzione dei programmi.

**Schedulatore con millis().**
```C++
	#define TBASE 100  // periodo base in millisecondi
	byte in;
	byte pulsante = 2;
	unsigned long precm=0;
	void setup()
	{
		pinMode(pulsante, INPUT);
	}

	void loop()
	{
		// polling di millis()
		if((millis()-precm) >= tbase){  // ricerca del nuovo tempo “buono per eseguire”
			// codice eseguito al tempo “buono per eseguire” attuale
			precm = millis();  // memorizzo l’istante dell’ultimo tempo “buono per eseguire”
			in = digitalRead(pulsante);  // lettura ingresso
			// Inserire qui la logica di comando
			//……………………………………
		}
	}
```

**_tbase_** rappresenta la **distanza** del **prossimo** tempo “buono” dall’**ultimo** valutato. Ogni **istante stabilito** viene misurato a **partire dall’ultimo** calcolato. Si determina il **tempo attuale** di esecuzione aggiungendo un tbase all’**ultimo tempo buono calcolato** e preventivamente **memorizzato** in precm. Facciamo una simulazione.

Se **tbase** vale 1000 msec e **precm** vale 0, accade che per 0, 1, 2,…,999 msec l’if **non scatta** perché la condizione è falsa **poi**, dopo, al millis che restituisce il valore 1000, **scatta** e si esegue **il compito schedulato**. In definitiva l’if **ignora** 999 chiamate loop() mentre **agisce** alla millesima che **capita** esattamente dopo un secondo. Dopo questo momento **precm** vale adesso 1000, millis(), ad ogni loop(), vale, ad es., 1001, 1002,…,1999 msec, l’if **non scatta** perché la condizione è falsa **poi**, dopo, al millis che restituisce il valore 2000, **scatta,** si **aggiorna** nuovamente **precm** al valore attuale di millis(), cioè 2000, e si **esegue** nuovamente il **compito schedulato**. In maniera analoga si preparano gli **scatti successivi**.

La generazione di più task in tempi diversi risulta **molto eficciente** perchè i task vengono eseguiti priodicamente in istanti stabiliti senza bloccare l'esecuzione degli altri task. La programmazione dei task è però più complessa perchè la programmazione non è più **lineare** in quanto l'ordine di scrittura dei task non rispecchia l'ordine di esecuzione **nel tempo**. Inoltre il gestore delle schedulazioni è un algoritmo che è parte stessa del programma principale, cioè il programma principale, oltre a gestire la logica dell'applicazione deve gestire la logica del gestore delle schedulazioni. Riassumendo, la **schedulazione mediante millis** comporta:
- **vantaggio**.  efficienza molto elevata nell'esecuzione dei programmi.
- **svantaggio**. programmazione a salti non lineare.
- **svantaggio**. l'applicazione deve gestire la logica dello schedulatore

## **SCHEDULATORE COMPITI**

Di seguito è riportato un esempio di schedulatore che pianifica nel tempo **l’esecuzione periodica** di una serie di **compiti** (task) da eseguire con **cadenza diversa**.

E’ buona norma evitare l’esecuzione frequente di operazioni lente quando queste non sono strettamente necessarie in modo da lasciare spazio ad altre operazioni, quali ad esempio gestione degli eventi di input, che richiedono una velocità maggiore per usufruirne in modo più interattivo.

Il **tempo base** è la base dei tempi di tutte le schedulazioni.

Le varie schedulazioni **sono calcolate** a partire da un **multiplo intero** del tempo base, ne segue che il tempo base dovrebbe essere calcolato come il massimo comune divisore (**MCD**) di tutti i tempi che devono essere generati.

Il conteggio dei multipli del tempo base è tenuto da un **contatore circolare** (step) che deve essere **ruotato** dopo aver effettuato un numero di conteggi superiori al **massimo dei multipli** del tempo base necessari.

Se ci sono **pulsanti** da gestire insieme ad altri task il tempo base può essere impostato tra 50 e 200 mSec in maniera da poterlo utilizzare per effettuare un **polling degli ingressi** immune dal fenomeno dei rimbalzi (**antibounce SW**).
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
	pinMode(led, OUTPUT);
}

void loop()
{
	// polling della millis() alla ricerca del tempo stabilito
	if((millis()-precm) >= tbase){ // lo eseguo se è il tempo stabilito
		precm = millis();  // preparo il tic successivo al tempo stabilito
		step = (step + 1) % nstep;  // conteggio circolare arriva al massimo a nstep-1

		// task 1
		if(!(step%2)){  // schedulo eventi al multiplo del tempo stabilito (2 sec)
			digitalWrite(led1,!digitalRead(led1)); // stato alto: led blink
		}
		// task 2
		if(!(step%3)){  // schedulo eventi al multiplo del tempo stabilito (3 sec)
			digitalWrite(led2,!digitalRead(led2)); // stato alto: led blink
		}
		// il codice eseguito al tempo del metronomo va quì
	}
	// il codice eseguito al tempo massimo della CPU va qui
}
```

## **GENERAZIONE DI TEMPI CON DELAY E THREAD**





## **MISURE DI TEMPO ASSOLUTE**

Per ottenere una cadenza periodica precisa è necessario usare una forma diversa dal solito schedulatore più adatta a cumulare con precisione lunghe misure di tempo. E’essenziale che l’accumulatore tass venga aggiornato esattamente con il tempo campionato. L’accumulatore unisce i vari campionamenti per ottenere una misura unica. Con questa forma ci si può aspettare un errore di qualche secondo all’ora dipendente solo dall’imprecisione dell’oscillatore.
```C++
unsigned long tass = 0;
if ((millis() - tass) >= periodo)
{
	tass += periodo;
	....
}
```
Invece la forma seguente è errata. La condizione viene valutata in ritardo rispetto al momento ideale, “reimpostando” la variabile ‘tass” al tempo attuale, questo ritardo si aggiunge a tutti i ritardi precedenti. Con questa forma ci si può aspettare un errore di diversi secondi al minuto o anche peggiore.
```C++
if ((millis() - tass) >= periodo)
{
	tass = millis();
	....
}
```
>[Torna all'indice generale](index.md) >[Versione in Python](generazionetempipy.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbMTEwNDI2NDY0N119
-->
