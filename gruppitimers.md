>[Torna all'indice](indextimers.md) >[versione in Python](gruppitimerspy.md)
## **GRUPPI DI TIMERS**

La logica di molti pulsanti potrebbe essere gestita sotto forma di dati strutturati (multivalore) in una delle seguenti maniere: array, struct ed oggetti. 
In ciascuna delle precedenti strutture dati vanno memorizzate almeno 3 cose: lo **stato** del timer, la memoria del suo **istante iniziale**, la **durata** del conteggio del tempo prima del timeout
La differenza tra i vari modi di gestione consiste in come si memorizzano stati, ingressi e logica:
- **Array**. Una sola funzione gestisce la logica di più pulsanti, stato e ingressi passati sono memorizzati sulle celle di due array paralleli. Gli array sono comuni a tutti i timer, ma ogni indice di entrambi gli array si riferisce sempre ad un solo timer.
- **Struct**. Una sola funzione gestisce la logica di più timer, stato e ingressi passati sono memorizzati su una struct separata per ogni timer. Al limite le struct possono essere organizzate in un array, soprattutto in presenza di molti timer.
- **Oggetti**. Stato, istante iniziale, durata sono memorizzati su un oggetto separato per ogni timer. Ogni oggetto possiede anche la funzione per manipolare i dati contenuti al suo interno (metodo). I metodi sono accessibili, a partire dalla variabile oggetto, usando la “dot” notation cioè, il nome della variabile, un punto, ed il nome del metodo. Anche in questo caso, volendo, per molti tasti gli oggetti potrebbero essere organizzati in un array o in una matrice.

Timer ottimizzati per molti task si possono realizzare utilizzando **librerie di terze parti** come SimpleTimer, una libreria molto precisa basata su timer periodici assoluti https://github.com/marcelloromani/Arduino-SimpleTimer/tree/master/SimpleTimer oppure una libreria esterna che realizza uno schedulatore ad eventi basato su time ticks (https://github.com/sebastianomelita/time-tick-scheduler).

```C++
/*
1) Scrivere un programma Arduino che accenda tre led (ingresso, sala, scala). Accenderli con tre pulsanti toggle separati. Lo stato dei led deve essere scritto sulla seriale all'avvenire (occorrenza) di ogni comando.
3) Realizzare, con un timer, l funzione di spegnimento automatico, dopo 10 secondi, della luce della scala. (dispensa timer.doc)
4) Utilizzare un altro timer per gestire lo spegnimento delle luci delle due sale alla pressione prolungata di uno dei loro pulsanti.
5) Realizzare, con un timer, la funzione "antincendio" che accende un led di segnalazione se non riceve via seriale il il comando "sensore":"on" entro 10 secondi.
Se lo riceve non segnala, se non lo riceve segnala. Al massimo aspetta 10 secondi per segnalare.
*/
#define TBASE 100
#define NSTEP 100
#define CMDSCALA 2
#define CMDSALA 3
#define CMDINGRESSO 4
#define LEDINGRESSO 8
#define LEDSALA 9
#define LEDSCALA 10
#define LEDSPIA 11
#define TSCALA 10000
#define TSPEGNI 1000
#define TSICUREZZA 10000

enum btn
{
  SCALA        =0,
  SALA         =1,
  INGRESSO     =2
};
enum timer
{
  TMRSCALA        =0,
  TMRSPEGNI       =1,
  TMRSICUREZZA    =2
};

byte precval[3]={0,0,0};
byte stato[3]={0,0,0};
unsigned long precm=0;
unsigned short step=0;
unsigned long startTime[3];
unsigned long timelapse[3];
bool timerState[3]={false,false,false};


void setup(){
  Serial.begin(115200);
  pinMode(CMDSCALA, INPUT);
  pinMode(CMDSALA, INPUT);
  pinMode(CMDINGRESSO, INPUT);
  pinMode(LEDINGRESSO, OUTPUT);
  pinMode(LEDSALA, OUTPUT);
  pinMode(LEDSCALA, OUTPUT);
  pinMode(LEDSPIA, OUTPUT);
  Serial.begin(9600);
  stopTimer(TMRSCALA);
  stopTimer(TMRSPEGNI);
  stopTimer(TMRSICUREZZA);
  startTimer(TSICUREZZA, TMRSICUREZZA);
}

void startTimer(unsigned long duration, unsigned short n){
	timerState[n]=true;
	timelapse[n]=duration;
	startTime[n]=millis();
}

void stopTimer(unsigned short n){
	timerState[n]=false;
}

//verifica se è arrivato il tempo di far scattare il timer
void aggiornaTimer(unsigned short n){
	if((timerState[n] == true) && (millis() - startTime[n] >= timelapse[n])){
		timerState[n]=false;
		onElapse(n);
	}
}

// azione da compiere allo scadere del timer, definita fuori dal loop
void onElapse(unsigned short n){
	if(n==TMRSCALA){
		stato[SCALA] = LOW;
		digitalWrite(LEDSCALA, stato[SCALA]);
	}else if(n==TMRSPEGNI){
		stato[SALA] = LOW;
		digitalWrite(LEDSALA, stato[SALA]);
		stato[INGRESSO] = LOW;
		digitalWrite(LEDINGRESSO, stato[INGRESSO]);
	}else if(n==TMRSICUREZZA){
		digitalWrite(LEDSPIA, HIGH);
	}
}

bool transizione(byte val,byte n){  //transizione di un pulsante
	bool cambiato=false;
	cambiato = (precval[n] != val);
	precval[n] = val;  
	return cambiato; 
}

void loop(){
  // polling dei tempi
  aggiornaTimer(TMRSCALA);
  aggiornaTimer(TMRSPEGNI);
  aggiornaTimer(TMRSICUREZZA);
  
  byte in;
  if(millis()-precm>=(unsigned long)TBASE){   //schedulatore e antirimbalzo
    precm=millis();   
    step=(step+1)%NSTEP; //conteggio circolare
       
    //polling pulsante SCALA
    in=digitalRead(CMDSCALA);
    if(transizione(in,SCALA)){
	if(in==HIGH){ //se fronte di salita (pressione)
		startTimer(TSCALA, TMRSCALA); 
		stato[SCALA] = !stato[SCALA];
		digitalWrite(LEDSCALA,stato[SCALA]);
		Serial.print("Scala: ");
		Serial.println(stato[SCALA]);
	}
    }
    
    //polling pulsante SALA
    in=digitalRead(CMDSALA);
    if(transizione(in,SALA)){
        if(in==HIGH){ //se fronte di salita (pressione)
		startTimer(TSPEGNI, TMRSPEGNI);
		stato[SALA] = !stato[SALA];
		digitalWrite(LEDSALA,stato[SALA]);
		Serial.print("Sala: ");
		Serial.println(stato[SALA]);
	}else{ // rilascio
		stopTimer(TMRSPEGNI);
	}
    }    

    // polling pulsante INGRESSO
    in=digitalRead(CMDINGRESSO);
    if(transizione(in,INGRESSO)){
        if(in==HIGH){ //se fronte di salita
		startTimer(TSPEGNI, TMRSPEGNI);
		stato[INGRESSO] = !stato[INGRESSO];
		digitalWrite(LEDINGRESSO,stato[INGRESSO]);
		Serial.print("Ingresso: ");
		Serial.println(stato[INGRESSO]);
	}else{ // rilascio
		stopTimer(TMRSPEGNI);
	}
    }
  } //chiudi schedulatore 
  
  if(Serial.available() > 0 ){//anche while va bene!
    short val;
    String instr = Serial.readString();
	
    if(instr.indexOf("\"sensore\":\"on\"") >= 0){
	stopTimer(TMRSICUREZZA);
	digitalWrite(LEDSPIA, LOW);
    }
  }
}
```
Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/foKpKs08UBQ?editbtn=1

>[Torna all'indice](indextimers.md) >[versione in Python](gruppitimerspy.md)
