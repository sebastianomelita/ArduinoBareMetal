


## **SERIALE ADUINO**

L’interfaccia seriale di Arduino possiede due **registri dati**, uno di **ricezione** (registro RX) da accedere in lettura ed uno di **trasmissione** (registro TX) da accedere in scrittura. Il primo è collegato alla **porta RX** della seriale, il secondo, ovviamente, a **quella TX**. Entrambi hanno la dimensione di un byte, quindi pochissimo spazio a disposizione, ma entrambi posseggono un **registro di stato** ed **eventi** di segnalazione (**interrupt**) che permettono di gestire prontamente la scrittura o la lettura di nuovi dati sui registri facendo in modo che **non vengano mai persi**.

**La gestione** dei registri è affidata a due ISR, una **ISR di ricezione**,      		_rx_complete_irq(), ed una **ISR di trasmissione,** _tx_udr_empty_irq(void).

Ciascun registro dati e collegato ad una **coda** realizzata in SW e della lunghezza di **64 byte**: una **coda di ricezione** ed una **coda di trasmissione**, detti anche **buffer di ricezione** e **buffer di trasmissione**.

Il **lavoro delle due ISR** in risposta agli eventi esterni è:

- **La ISR di ricezione**, all’attivazione dell’interrupt che segnala **un nuovo dato pronto in RX** prontamente **legge dal registro** RX il dato appena arrivato e lo **scrive in coda** al buffer di ricezione.

- **La ISR di trasmissione**, all’attivazione dell’interrupt che segnala **registro vuoto in TX**, cioè quello che segnala che l’ultima trasmissione in corso (pendente) è ormai terminata, prontamente **legge** **dalla testa** della coda di trasmissione un byte e lo **scrive sul registro** TX.

Ma anche **l’applicazione** genera degli eventi di lettura e scrittura che possono accadere in momenti qualsiasi asincroni con quelli di una ricezione o una trasmissione sulla seriale. In RX che succede se arriva un dato e l’applicazione non è pronta a leggerlo? O si conserva o va perso. In TX che succede se l’applicazione decide di trasmettere un dato ma il canale è occupato da una trasmissione pendente? O riprova più tardi (deve fare un polling) oppure si perde. Le **code** sono **la soluzione** a tutto ciò e servono proprio da **memoria tampone** per gestire queste **asincronie**:

- La **coda di ricezione** serve ad **accumulare** i byte di dati che provengono dalla seriale, in attesa che vengano letti dall’applicazione. La lettura la fa la funzione **Serial.read()** prelevando un byte **dalla testa** del buffer di ricezione. L’indicatore di testa viene **aggiornato** di conseguenza.

- La **coda di trasmissione** serve ad **accumulare** i byte di dati che provengono dalla applicazione, in attesa che vengano trasmessi sul canale. La scrittura la esegue la **Serial.write()** inserendo un byte **in coda** al buffer  di trasmissione. L’indicatore di coda viene **aggiornato** di conseguenza.

Quindi, **a regime** sul buffer di ricezione c’è un equilibrio dinamico (variabile nel tempo) tra byte scritti dalla ISR di RX e byte letti dalla applicazione, che comunque non deve attendere troppo a leggere il buffer pena il suo **overflow** (64 byte si riempiono quasi subito).

Allo stesso modo, **a regime** sul buffer di trasmissione c’è un equilibrio dinamico (variabile nel tempo) tra byte letti dalla ISR di TX e byte scritti dalla applicazione, che comunque non deve scrivere troppi dati troppo spesso pena **l’overflow** del buffer di trasmissione.

La funzione **Serial.overflow()** verifica se si è verificato un overflow del **buffer RX**. La chiamata a questa funzione cancella il flag di overflow, il che significa che le chiamate successive restituiranno false a meno che un altro byte di dati non sia stato ricevuto e scartato nel frattempo.

I **due buffer**, di trasmissione e ricezione, vengono realizzati con una struttura dati (spesso implementata con array) detta **coda circolare** che fa in modo che si possano aggiornare dinamicamente le posizioni degli indicatori di testa e di coda **senza** dover eseguire **spostamenti** del contenuto delle celle nel buffer.

Adesso è possibile capire con più consapevolezza il significato delle istruzioni di Arduino che sovraintendono alla gestione della seriale.

**Serial.available()** restituisce il numero di byte non ancora letti (dalla applicazione) che ci sono sulla coda di ricezione. Chiaramente il suo numero può variare da 0 a 64. Il polling nel loop() di questa funzione, testando che sia maggiore di zero, ci dice se, in un certo momento, sono arrivati nuovi byte dalla seriale. Esempio di **polling** della Serial.available():
```C++
void loop(){
	if(Serial.available() > 0 ){  	//anche while va bene!
		String instr = 	Serial.readString();
		// byte inbyte = Serial.read ();
		…………………………………….
		…………………………………….
	}
}
```
Una volta scoperto che ci sono dati disponibili sul buffer di ricezione, l’applicazione può pensare di **prelevarli**, senza però perdere troppo tempo, pena l’overflow del buffer. Per questo motivo il **polling** della Serial.available() conviene che venga sempre fatto alla **massima velocità** consentita dalla CPU, cioè direttamente dentro il loop **senza schedulazioni**.

Due **funzioni di lettura** sono particolarmente utili allo scopo della **rimozione dei dati** dal buffer di ricezione:

- **Serial.read ().**  Legge dal buffer un **byte** alla volta. Letture successive leggono sempre lo stesso valore. Per leggere nuovi byte, tra una read() e la successiva, è necessario chiamare la Serial.available() che sposta la posizione della testa della coda ponendola dopo l’ultimo byte letto, in corrispondenza del primo byte in coda ancora da leggere.

- **Serial.readString().**  Legge dal buffer una sequenza di byte corrispondente ad una stringa e la mette su una variabile di tipo compatibile (String). la funzione termina allo scadere di un timeout.

- **Serial.parseInt()** Legge dal buffer il primo numero intero a partire dalla posizione corrente (long). La funzione è terminata dal primo carattere che non è un numero intero. Scorre i caratteri prima del numero, legge il numero dopo di essi e si ferma al primo nuovo carattere. Chiamate successive ripetono il procedimento continuando il parsing (non è necessario chiamare la available() per fare questo).

- **Serial.parseFloat()** Legge dal buffer il primo numero float a partire dalla posizione corrente (long). La funzione è terminata dal primo carattere che non è un numero float. Per le chiamate successive si comporta come parseInt().

- **Serial.peek().**  Legge dal buffer un byte alla senza però rimuoverlo dal buffer di ricezione. Letture successive leggono sempre lo stesso valore a meno di non chiamare la Serial.available() dopo.

Si noti che le funzioni precedenti, ad eccezione di read() e peek(), eseguono una operazione di **parsing**, cioè il **processo** di scorrere i caratteri di un flusso di dati continuo (stream, stringa, ecc.) alla ricerca dell’informazione voluta.

La **scrittura sulla seriale** non ha particolari restrizioni, può avvenire in **qualunque parte** del codice (nel loop(), nel setup(), **non nelle ISR** però) e in **qualunque momento** e può essere svolta anche in maniera schedulata o arbitrariamente occasionale. L’unico vincolo è che non si scriva troppo spesso un numero eccessivo di dati.

In genere viene **eseguita** con queste funzioni:

- **Serial.write().** Esegue la scrittura di un byte così come è fornito dalla applicazione, senza effettuare conversioni di sorta.

- **Serial.print(x).** Effettua la scrittura **della arbitraria** sequenza di byte contenuta nella variabile x che può essere di qualunque tipo (char, int, String, ecc.), la funzione effettuerà la conversione dal tipo originale alla codifica ASCII (vengono sempre inviati caratteri).

- **Serial.println(x).** Analoga alla precedente, l’unica differenza è che appende in coda un carattere di newLine (a capo).

In ogni caso è necessario **inizializzare la seriale** impostando la sua velocità che deve essere la stessa sia nel dispositivo in TX che in quello in RX. L’impostazione non è automatica e deve essere fatta dal programmatore (assenza di negoziazione automatica). L’inizializzazione in genere si esegue nel **setup(). Le velocità** possibili vanno da 9600 baud a 115200 baud.
```C++
void setup ()
{
	pinMode(LED, OUTPUT);
	Serial.begin(9600);
}  	// end of setup
```
E’ possibile anche definire una ISR di ricezione customizzata definendo il codice della funzione Serial.serialEvent() per Arduino Uno
```C++
void serialEvent(){
	//statements
}
```
per Arduino Mega ci sono anche altre ISR  per altre le porte seriali a disposizione:
```C++
void serialEvent1(){
	//statements
}

void serialEvent2(){
	//statements
}

void serialEvent3(){
	//statements
}
```
Per **semplificare** la vita agli sviluppatori, alcune ISR sono fornite direttamente dalle **librerie di Arduino**.

Nell’esempio seguente viene adoperata la ISR **serialEvent()**che risponde all’evento arrivo di un nuovo carattere nella seriale e che può essere riempita con **codice personalizzabile**.

**Nell’esempio** la ISR viene utilizzata per creare una stringa, cioè **una parola**, unendo i singoli caratteri che man mano arrivano. La stringa è pronta, e quindi il flag viene attivato, quando arriva un carattere di fine linea “\n”.

Quando il loop principale si accorge del flag attivo ristampa in uscita la parola ottenuta tramite l’istruzione **Serial.println**.
```C++
string inputString = "";  // a String to hold incoming data

boolean volatile stringComplete = false;  // whether the string is complete
byte byteRead;
void setup() {
	Serial.begin(9600);
	inputString.reserve(200);
}

loop(){
	// il polling su una variabile è meno lento di quello su una porta seriale
	if (stringComplete) {
		stringComplete = false; //reset della bandierina (flag)
		Serial.println(inputString);
		// clear the string:
		inputString = "";
	}
}

// ISR (Interrupt Service Routine) che a seguito dell’arrivo di un dato. Crea  // una stringa da una sequenza di caratteri.
void serialEvent(){
	while (Serial.available()){
		// legge il nuovo byte:
		char inChar = (char) Serial.read();
		// lo aggiunge a inputString:
		inputString += inChar;
		// se il carattere in arrivo è un newline,
		// imposta un flag in modo che il main loop
		// possa utilizzarlo per rilevare l’evento di completamento di una stringa:
		if (inChar == '\n') {
			stringComplete = true;
		}
	}
}
```
**Esempio di interprete di un comando vocale trascritto via seriale**
Si tratta di interpretare una frase in linguaggio naturale in cui il comando non è espresso in modo non formale ma va dedotto cercando la corrispondenza simultanea di alcune parole chiave. In realtà la variabilità delle frasi umane per esprimere il generico comando di accendere una luce può essere molto ampio, può, ad esempio includere le parole "accendi", "attiva", "on", "illumina", o altre parole con significati analoghi. Il nostro parser dovrebbe provare la corrispondenza con ognuna di esse ma di seguito, per semplicità, si cerca la corrispondenza con la sola parola "accendi", a cui va abbinata la corrispondenza con la parola che individua l'ambiente prescelto.
```C++
const byte LED = 13;
const byte BUTTON = 2;
void setup ()
{
	pinMode(LED, OUTPUT);
	Serial.begin(9600);
	Serial.setTimeout(10);
}  // end of setup

void loop(){
	if(Serial.available() > 0 ){//anche while va bene!
		//parsing di una stringa dal flusso di dati 
		String str = Serial.readString();
		//parsing di un comando (sottostringa)	
		if(str.indexOf("on")){
			digitalWrite(LED, HIGH);
		}
		if(str.indexOf("spegni")){
			Serial.println("off");
			digitalWrite(LED, LOW);
		}
	}
}
```
**Esempio di gestione comandi locali e remoti via seriale**
```C++
/*
Scrivere un programma Arduino che manda sulla seriale ogni due secondi lo stato di un led (cucina).
Accenderlo con un pulsante toggle.
Ricevere dalla seriale il comando di accensione nel formato
<cucina>:<on> oppure <cucina>:<off>
*/
#define TBASE 100
#define NSTEP 100
#define CMDCUCINA 3
#define LEDCUCINA 13
byte precval=0;
byte stato=0;
unsigned long precm=0;
unsigned short step=0;

void setup(){
	pinMode(CMDCUCINA, INPUT);
	pinMode(LEDCUCINA, OUTPUT);
	Serial.begin(115200);
}

bool toggleH(byte val) { //transizione di un pulsante
	bool cambiato = false;
	if (precval  == LOW  && val == HIGH){
		cambiato = true;
		stato = !stato;
	}
	return cambiato;
}

// ricerca un certo commando all’interno di una stringa e ne restituisce il valore
short cmdParser(String instr, String attributo, unsigned maxlen){
	int start, ends=0;
	int out = -1;
	start = instr.indexOf("<"+attributo+">:<");
	if(start >= 0){
		start += (attributo).length() + 4;
		//scorri l'input finchè non trovi il tag di fine oppure il fine stringa
		for(ends=start+1; (ends < start + maxlen) && instr.charAt(ends)!='>' && ends < instr.length(); ends++);
		String attrval = instr.substring(start, ends);
		if(attrval == "on"){
			out = 1;
		}else if(attrval == "off"){
			out = 0;
		}
	}
	return out;
}

void loop(){
	byte in;
	if(millis()-precm>=(unsigned long)TBASE){ //schedulatore e antirimbalzo
		precm=millis();
		step=(step+1)%NSTEP; //conteggio circolare
		// comunicazione stato led
		if(!(step%20)){
			Serial.print("stato led cucina: ");
			Serial.println(stato);
		}
		//polling pulsante cucina
		in=digitalRead(CMDCUCINA);
		if(toggleH(in)){
			digitalWrite(LEDCUCINA,stato);
		}
	} //chiudi schedulatore

	// gestione evento comando da seriale
	if(Serial.available() > 0 ){//anche while va bene!
		short val;
		String instr = Serial.readString();
		val = cmdParser(instr,"cucina",10);
		if(val>=0){
			stato = val;
			digitalWrite(LEDCUCINA,stato);
		}
	}
}
```
<!--stackedit_data:
eyJoaXN0b3J5IjpbMjEyMDYzODE1Niw0Njc0NjA5OTEsLTg5Nj
Y2ODc1OCwxMjAxMTAzMTc5XX0=
-->