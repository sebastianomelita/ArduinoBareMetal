>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in Python](timeschedpy.md)

## **ESEMPI DI SCHEDULATORI**

### **SCHEDULATORE CON DELAY()**
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
		delay(100);  // ritarda del tempo necessario ad evitare un rimbalzo
	}
```
La generazione di più task in tempi diversi risulta **molto semplice** se eseguita con la funzione delay() soprattutto per la natura **lineare** della programmazione che permette di scrivere il codice dei task con lo **stesso ordine** con cui questi verranno effettivamente eseguiti **nel tempo**. Lo svantaggio di questa realizzazione è una sostanziale **inefficienza** perchè il **blocco di un task** comporta anche il blocco di **tutti gli altri**, compreso il programma principale (**main**). 

Riassumendo, la **schedulazione mediante delay** comporta:
- **vantaggio**. Programmazione lineare molto semplice secondo una **logica strettamente sequenziale**
- **svantaggio**. Inefficienza intrinseca nell'esecuzione dei programmi.


### **SCHEDULATORE CON DELAY() PIU' PRECISO**
 
Se il codice da eseguire prima dell'attesa ha un **tempo di esecuzione** non trascurabile questo può sommarsi all'attesa programmata generando un'**errore** nella **schedulazione** dell'iterazione successiva. L'**errore** è proprio uguale al **tempo di esecuzione** del codice di una singola iterazione.

Per ovviare a questo problema basta misurare la **durata** dell'esecuzione di un ciclo e sottrarla al tempo pianificato per l'esecuzione del ciclo sucessivo. Allo scopo si adopera la funzione **millis()**.

```C++
	byte led = 13;
	unsigned long start, end, duration;
	unsigned long dt = 1000;
    	long randomNumber;
	
	void setup()
	{
		Serial.begin(115200);
       	 	pinMode(led, OUTPUT);
        	randomSeed(analogRead(A0));   
	}

	void loop()
	{
		start = millis();
		//codice eseguito al tempo stabilito
    		digitalWrite(led,!digitalRead(led));
		randomNumber = random(200,700);
    		Serial.println(randomNumber);
    		delay(randomNumber);            //emula un blocco di istruzioni
		end = millis();
		duration = end - start;
	        if(duration < dt)
			delay(dt - duration);
	}
```

La durata del'esecuzione viene sottratta solamente se rimane tempo in eccesso.

Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/2ZpoOZ28vAd?editbtn=1

Simulazione su Esp32 con Wowki: https://wokwi.com/projects/348974627603612242
 
### **SCHEDULATORE CON TIMER TICK GENERATI DA MILLIS()**

Il **tempo base** è la base dei tempi di tutte le schedulazioni, viene calcolato **periodicamente** all'accadere di un **evento** (superamento di una soglia di tempo) che viene detto **tick** (scatto, in analogia con il metronomo per la musica).

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
		// polling della millis() alla ricerca del tempo in cui scade ogni periodo
		unsigned long current_millis = millis();
		if((current_millis-precm) >= (unsigned long) tbase){ 	//se è passato un periodo tbase dal precedente periodo
			precm = current_millis;  	//preparo il tic successivo azzerando il conteggio del tempo ad adesso	
			
			// codice eseguito esattamente ad ogni periodo 
			in = digitalRead(pulsante);  // lettura ingresso
			// Inserire qui la logica di comando
			//……………………………………
		}
	}
```

**_tbase_** rappresenta la **distanza** del **prossimo** "tick" dall’**ultimo** valutato. Ogni **istante stabilito** per un tick viene misurato a **partire dall’ultimo** calcolato. Si determina il **momento del tick attuale** aggiungendo un tbase costante all’**ultimo tick calcolato** e preventivamente **memorizzato** in precm.

Facciamo una simulazione.

Se **tbase** vale 1000 msec e **precm** vale 0, accade che per 0, 1, 2,…,999 msec l’if **non scatta** perché la condizione è falsa **poi**, dopo, al millis che restituisce il valore 1000, **scatta** e si esegue **il compito schedulato**. In definitiva l’if **ignora** 999 chiamate loop() mentre **agisce** alla millesima che **capita** esattamente dopo un secondo. Dopo questo momento **precm** vale adesso 1000, millis(), ad ogni loop(), vale, ad es., 1001, 1002,…,1999 msec, l’if **non scatta** perché la condizione è falsa **poi**, dopo, al millis che restituisce il valore 2000, **scatta,** si **aggiorna** nuovamente **precm** al valore attuale di millis(), cioè 2000, e si **esegue** nuovamente il **compito schedulato**. In maniera analoga si preparano gli **scatti successivi**.

**In pratica**:
1. **dopo un tick** l'espressione ```current_millis-precm``` è minore di ```tbase``` e la funzione ```millis()``` viene campionata ad ogni ```loop()``` fino a che il tempo attuale non diventa un nuovo **multiplo in tero** di ```tbase``` e ciò analiticamente accade quando ```current_millis-precm``` raggiunge di nuovo il valore ```tbase```.
2. se il **task** genera un **ritardo** nel ```loop()``` **minore** di un ```tbase```, allora il tempo del tick successivo (e quello di tutti gli altri) sarà calcolato sempre esattamente pari ad un **multiplo di tbase**.
3. se il **task** genera un **ritardo** nel ```loop()``` **maggiore** di un tbase, allora il tempo del primo tick successivo verrà calcolato **subito dopo** il ritardo e questo tempo in genere **non è** un multiplo intero di ```tbase```. Al **primo tick** dopo il ritardo il tempo attuale potrebbe cumulare **molti multipli** di ```tbase``` per cui più **tick consecutivi** potrebbero essere eseguiti **tutti insieme** uno dopo l'altro, fino a che ```current_millis-precm < tbase```. I tick successivi al primo dopo il ritardo ricadono nella situazione al punto 1. e sono tutti esattamente **multipli** di ```tbase```.

La generazione di più task in tempi diversi risulta **molto efficiente** perchè i task vengono eseguiti **periodicamente** in istanti stabiliti **senza bloccare** l'esecuzione degli altri task. La programmazione dei task è però **più complessa** rispetto alla soluzione con i ritardi perchè la programmazione non è più **lineare** in quanto l'ordine di scrittura dei task non rispecchia l'ordine di esecuzione dei medesimi **nel tempo**. Inoltre il gestore delle schedulazioni è un algoritmo che è parte stessa del programma principale, cioè il programma principale, oltre a gestire la logica dell'applicazione deve gestire la logica del gestore delle schedulazioni. 

Riassumendo, la **schedulazione mediante millis** comporta:
- **vantaggio**.  efficienza molto elevata nell'esecuzione dei programmi.
- **svantaggio**. programmazione a salti non lineare, secondo una **logica iterativa**
- **svantaggio**. l'applicazione deve gestire la logica dello schedulatore
 
 
### **SCHEDULATORE CON TIMER HARDWARE**

Dopo la tecnica basata sulla **pianificazione dei ritardi** delay() e quella basata sul **polling periodico della funzione millis()** per generare una catena di tick esiste un terzo modo di generare con precisione dei tempi per il quale però è necessario impostare un **HW esterno detto Timer**.

Il **timer** in genere è alimentato dal clock della CPU e genera dei tempi a partire da un dispositivo che, in maniera HW, calcola multipli interi del tempo di clock detto **prescaler**. Il prescaler può essere impostato modificando appositi registri della CPU al fine di generare il tempo desiderato. 

Allo scadere dl timer viene generato un segnale di **interrupt** che fa partire una ISR (Interrupt Service Routine) che svolge le istruzioni associate all'evento.

L'operazione di **settaggio diretto** da programma del timer, se eseguita a basso livello, può risultare scomoda o perchè intrinsecamente legata al tipo di CPU da un lato o perchè richiede l'uso di qualche espressione di algebra booleana bit a bit che può risultare un tantino astrusa dall'altro.

La maniera **più comune** di impostare in maniera esplicita un timer in un programma, perchè più portabile e più semplice, è utilizzare le funzioni messe a disposizione per questo scopo da **librerie di terze parti**.

In realtà il timer HW è spesso utilizzato **indirettamente** dai programmi perchè il meccanismo degli interrupt e alla base sia della funzione delay() che della funzione millis() che, internamente, si limitano a contare un prefisato numero di tick generati da un timer HW. In ogni caso l'intervento del timer è sempre dal programmatore  **pianificato  esplicitamente**.

La **gestione diretta** di un timer per programmare nel tempo **molti** task (**schedulazione dei task**) può risultare complicata e inoltre, anche in questo caso, la programmazione dei task è **più complessa** rispetto ad altre soluzioni perchè la programmazione non è più **lineare** in quanto l'ordine di scrittura dei task non rispecchia l'ordine di esecuzione dei medesimi **nel tempo**.  
```C++
/*
 *  Timer1 library example
 *  June 2008 | jesse dot tane at gmail dot com
 *  Sets up PWM output on pin 9 with a 50% duty cycle, and attaches an interrupt that toggles digital pin 10 every half second.
 */
 
#include "TimerOne.h"
 
void setup()
{
  pinMode(13, OUTPUT);
  Timer1.initialize(500000);         // initialize timer1, and set a 1/2 second period
  Timer1.pwm(9, 512);                // setup pwm on pin 9, 50% duty cycle
  Timer1.attachInterrupt(callback);  // attaches callback() as a timer overflow interrupt
}
 
void callback()
{
  digitalWrite(10, !digitalRead(10));
}
 
void loop()
{
  // your program here...
}
```
### **TIMER HW IMPOSTATO AUTOMATICAMENTE (THREAD)**

Spesso la gestione del timer è affidata ad modulo SW esterno ai task per cui l'intervento del timer **non è deciso** dal programmatore ma è deciso da un modulo detto **schedulatore dei thread** che agisce in **maniera trasparente** al di fuori del controllo del programmatore. 

Scopo dello schedulatore è gestire il **multitasking**, cioè l'esecuzione **parallela** di più programmi su un unica CPU. Ciò si ottiene togliendo periodicamente la risorsa CPU ad un programma per darla ad un'altro. L'esecuzione dei task, in questo caso, può essere programmata come al solito, ordinando le istruzioni in **maniera sequenziale**, non badando al fatto che queste possano essere interrotte dallo schedulatore.

Una applicazione in genere non è programmata per essere altruista per cui non cede spontaneamente la CPU. Il **ruolo del timer HW** è proprio quello di **contare** il tempo assegnato ad una applicazione e di **sottrargli la CPU** quando questo è scaduto. Un segnale di interrupt interrompe l'esecuzione del programma corrente e fa partire la ISR() dello schedulatore dei thread che assegna la CPU ad un altro programma. L'operazione viene ripetuta **a turno** per tutti i programmi in esecuzione. Questa tecnica è detta **multi-threading** ed è gestita e coordinata da un modulo software detto schedulatore dei thread. Uno **schedulatore dei thread** può essere **integrato nel SO** oppure inserito in una **libreria a parte**. 

### **Considerazioni finali**

**Riassumendo**, la schedulazione dei task mediante **timers** comporta:

- vantaggio. l'applicazione non deve gestire la logica dello schedulatore
- svantaggio. programmazione diretta a salti non lineare, secondo una logica ad eventi
- vantaggio. programmazione nei sistemi multithreading lineare, secondo una logica sequenziale



Parleremo brevemente del multitasking illustrando qualche esempio in una scheda successiva.

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in Python](timeschedpy.md)
