


>[Torna all'indice](indexseriale.md)

La generazione dei messaggi sulla seriale può avvenire in qualunque punto del codice, anche all'interno di uno schedulatore, ma non dentro una ISR.
Un messaggio può essere inviato sulla seriale essenzialmente per:
- **generare un report ** sullo stato del sistema
-  **generare dei comandi ** con cui richiedere un servizio su un dispositivo remoto 

La generazione dei messaggi in genere avviene:
- **periodicamente**, cioè inserita dentro uno schedulatore che la genera a la invia ad intervalli regolari. Utile per creare un report periodico dello stato del sistema.
- **triggerata da eventi**, cioè il messaggio viene generato all'accadere di un evento specifico come la pressione di un pulsante, il superamento di una soglia per il valore di un sensore, ecc.
- **su richiesta**, in seguito al polling eseguito da un master,  si potrebbe generare una risposta che descriva lo stato generale del sistema o di una sua parte specificata come parametro della richiesta.

**Esempio di generazione periodica di un comando**
```C++
/*
Scrivere un programma Arduino che manda sulla seriale ogni due secondi lo stato di due led 
(cucina, soggiorno). 
Accenderli con due pulsanti toggle separati.
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

void loop(){
	byte in;
	if(millis()-precm>=(unsigned long)TBASE){ 	//schedulatore e antirimbalzo
		precm=millis(); 	
		step=(step+1)%NSTEP; //conteggio circolare
		
		// comunicazione stato led
		if(!(step%20)){
			Serial.print("cucina:\"");
			Serial.println(stato[CUCINA]);
			Serial.print("\"");
			Serial.print("soggiorno:\"");
			Serial.println(stato[SOGGIORNO]);
			Serial.print("\"");
		}
	} 	//chiudi schedulatore	
}
```	

**Esempio di generazione triggerata da evento di un comando**
```C++
/*
1) Scrivere un programma Arduino che accenda due led (ingresso, sala). Accenderli con due pulsanti toggle separati. Lo stato dei led deve essere scritto sulla seriale all'avvenire (occorrenza) di ogni comando.
*/
#define TBASE 100
#define NSTEP 100
#define CMDSCALA 2
#define CMDSALA 4
#define CMDINGRESSO 3
#define LEDINGRESSO 11
#define LEDSALA 12

enum btn
{
  SALA     	   = 0,
  INGRESSO     = 1
};

byte precval[2]={0,0};
byte stato[2]={0,0};
unsigned long precm=0;
unsigned short step=0;


void setup(){
  pinMode(CMDSALA, INPUT);
  pinMode(LEDSALA, OUTPUT);
  pinMode(CMDINGRESSO, INPUT);
  pinMode(LEDINGRESSO, OUTPUT);
  Serial.begin(115200);
}

bool transizione(byte val,byte n){  //transizione di un pulsante
  bool cambiato=false;
  cambiato = (precval[n] != val);
  precval[n] = val;  
  return cambiato; 
}

void loop(){
  byte in;
  if(millis()-precm>=(unsigned long)TBASE){   //schedulatore e antirimbalzo
	precm=millis();   
	step=(step+1)%NSTEP; //conteggio circolare
	  
	//polling pulsante SCALA
	in=digitalRead(CMDSALA);
	if(transizione(in,SALA)){
		if(in==HIGH){ //se fronte di salita
			stato[SALA] = !stato[SALA];
			digitalWrite(LEDSALA,stato[SALA]);
			//generazione report ad evento pressione
			Serial.print("sala:\"");
			Serial.println(stato[SALA]);
			Serial.print("\"");
		}
	}

	//polling pulsante SALA
	in=digitalRead(CMDINGRESSO);
	if(transizione(in,INGRESSO)){
		if(in==HIGH){ //se fronte di salita
			stato[INGRESSO] = !stato[INGRESSO];
			digitalWrite(LEDINGRESSO,stato[INGRESSO]);
			//generazione report ad evento pressione
			Serial.print("ingresso:\"");
			Serial.println(stato[INGRESSO]);
			Serial.print("\"");
		}
	}
  }
}

```	

	
			
			
	
	
	
	







>[Torna all'indice](indexseriale.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTU3NjI0MDE2M119
-->