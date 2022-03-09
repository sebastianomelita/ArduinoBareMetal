
>[Torna all'indice](indexseriale.md)
## **Interprete di comandi**

Si tratta di interpretare una frase in linguaggio naturale in cui il comando non è espresso in modo formale ma va dedotto cercando la corrispondenza simultanea di alcune parole chiave. In realtà la variabilità delle frasi umane per esprimere il generico comando di accendere una luce può essere molto ampio, può, ad esempio includere le parole "accendi", "attiva", "on", "illumina", o altre parole con significati analoghi. Il nostro parser dovrebbe provare la corrispondenza con ognuna di esse ma di seguito, per semplicità, si cerca la corrispondenza con la sola parola "accendi".

 **Esempio di interprete di un comando vocale trascritto via seriale**
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
		//parsing di un comando (sottostringa) all'interno di una stringa	
		if(str.indexOf("accendi")){
			digitalWrite(LED, HIGH);
		}
		if(str.indexOf("spegni")){
			digitalWrite(LED, LOW);
		}
	}
}
```
I comandi possono anche essere inviati in maniera meno ambigua utilizzando un formato preventivamente concordato tra mittente e destinatario. In questo caso comandi diversi in genere condividono per lo meno un formato comune. 
I formati dei messaggi più utilizzati in genere sono:
 - Testo formattato con separatori di campo (spazi, virgole, ecc.) e separatori di riga (\0, \n,\r)
 - dati rappresentati sotto forma di oggetti XML
 - dati rappresentati sotto forma di oggetti JSON

In IOT, come in molti altri ambiti, attualmente si preferisce la costruzione di messaggi JSON per la proprietà di compattezza che li contraddistingue.
Nell'esempio seguente utilizzeremo una codifica a tag standard in cui, sia attributi che valori, sono racchiusi tra doppi apici nel formato "attributo":"valore" ma nulla vieta di sostituire  il carattere " con altri tag come, ad esempio, < o >.  Il carattere ':' è utilizzato come delimitatore tra l'attributo ed il suo valore.
 
**Esempio di gestione comandi locali e remoti via seriale**
```C++
/*
Scrivere un programma Arduino che manda sulla seriale ogni due secondi lo stato di un led (cucina).
Accenderlo con un pulsante toggle.
Ricevere dalla seriale il comando di accensione nel formato
"cucina":"on" oppure "cucina":"off>
*/
#define TBASE 100
#define NSTEP 100
#define CMDSOGGIORNO 2
#define CMDCUCINA 3
#define LEDSOGGIORNO 12
#define LEDCUCINA 13

enum btn
{
	CUCINA				=0,
	SOGGIORNO			=1
};
byte precval[2]={0,0};
byte stato[2]={0,0};
unsigned long precm=0;
unsigned short step=0;

void setup(){
	pinMode(CMDSOGGIORNO, INPUT);
	pinMode(CMDCUCINA, INPUT);
	pinMode(LEDSOGGIORNO, OUTPUT);
	pinMode(LEDCUCINA, OUTPUT);
	Serial.begin(115200);
}

bool transizione(byte val,byte n){ 	//transizione di un pulsante
	bool cambiato=false;
	cambiato = (precval[n] != val);
	precval[n] = val;  
	return cambiato; 
}

void loop(){
	byte in;
	if(millis()-precm>=(unsigned long)TBASE){ 	//schedulatore e antirimbalzo
		precm=millis(); 	
		step=(step+1)%NSTEP; //conteggio circolare
		
		// comunicazione stato led
		if(!(step%20)){
			Serial.print("stato led cucina: ");
			Serial.println(stato[CUCINA]);
			Serial.print("stato led soggiorno: ");
			Serial.println(stato[SOGGIORNO]);
		}
			
		// gestione evento comando da seriale
		if(Serial.available() > 0 ){//anche while va bene!
			short val;
			String instr = Serial.readString();
			
			if(instr.indexOf("\"cucina\":\"on\"") >= 0){
			  stato[CUCINA] = HIGH;
			  digitalWrite(LEDCUCINA, stato[CUCINA]);
			}
			if(instr.indexOf("\"cucina\":\"off\"") >= 0){
			  stato[CUCINA] = LOW;
			  digitalWrite(LEDCUCINA, stato[CUCINA]);
			}
			if(instr.indexOf("\"soggiorno\":\"on\"") >= 0){
			  stato[SOGGIORNO] = HIGH;
			  digitalWrite(LEDSOGGIORNO, stato[SOGGIORNO]);
			}
			if(instr.indexOf("\"soggiorno\":\"off\"") >= 0){
			  stato[SOGGIORNO] = LOW;
			  digitalWrite(LEDSOGGIORNO, stato[SOGGIORNO]);
			}
		}
	} 	//chiudi schedulatore	
}
	
```
Nell'esempio seguente utilizzeremo la funzione cmdParser(instr, "parametro", 10) per realizzare un parser che restituisce il valore ricevuto del parametro inserito come argomento. Un valore negativo significa parametro non trovato, uno positivo parametro trovato con il valore restituito. L'utilizzo di questa funzione permette di risparmiare qualche riga di codice ed eventualmente la gestione dinamica dei parametri mediante un ciclo che scorrendo un vettore di parametri li cerca nel messaggio ricevuto e, se esistono, ne restituisce il valore.
	
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
>[Torna all'indice](indexseriale.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbMzAzNTA5ODc2XX0=
-->
