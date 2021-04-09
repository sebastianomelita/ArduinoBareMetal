```C++
/*
1) Scrivere un programma Arduino che accenda due led (ingresso, sala, scala). Accenderli con tre pulsanti toggle separati. Lo stato dei led deve essere scritto sulla seriale all'avvenire (occorrenza) di ogni comando.
3) Realizzare, con un timer, l funzione di spegnimento automatico, dopo 10 secondi, della luce della scala. (dispensa timer.doc)
4) Utilizzare un altro timer per gestire lo spegnimento delle luci delle due sale alla pressione prolungata di uno dei loro pulsanti.
5) Realizzare, con un timer, la funzione "lampada viva" che accende (o fa lampeggiare) due led di segnalazione, uno per ogni sala, se non riceve via seriale il il comando "sala":"alive" e "ingresso":"alive" entro 10 secondi. 
   Se lo riceve non segnala, se non lo riceve segnala. Al massimo aspetta 10 secondi per segnalare.
*/
#define TBASE 100
#define NSTEP 100
#define CMDSCALA 2
#define CMDSALA 3
#define CMDINGRESSO 4
#define LEDINGRESSO 11
#define LEDSALA 12
#define LEDSCALA 13
#define LEDSPIA 11
#define TSCALA 10000
#define TSPEGNI 10000
#define TSICUREZZA 10000

enum btn
{
  SCALA        =0,
  SALA     	   =1,
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
  pinMode(CMDSCALA, INPUT);
  pinMode(CMDSALA, INPUT);
  pinMode(CMDINGRESSO, INPUT);
  pinMode(LEDINGRESSO, OUTPUT);
  pinMode(LEDSALA, OUTPUT);
  pinMode(LEDSCALA, OUTPUT);
  pinMode(LEDSPIA, OUTPUT);
  Serial.begin(115200);
  stopTimer(TMRSCALA);
  stopTimer(TMRSPEGNI);
  stopTimer(TMRSICUREZZA);
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
		stato[SCALA] = LOW;
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
  aggiornaTimer(TMRSCALA);
  aggiornaTimer(TMRSPEGNI);
  aggiornaTimer(TMRSICUREZZA);
  
  byte in;
  if(millis()-precm>=(unsigned long)TBASE){   //schedulatore e antirimbalzo
    precm=millis();   
    step=(step+1)%NSTEP; //conteggio circolare
    
    // comunicazione stato led
    if(!(step%100)){
      Serial.println();
      Serial.print("stato led scala: ");
      Serial.println(stato[SCALA]);
      Serial.print("stato led sala: ");
      Serial.println(stato[SALA]);
      Serial.print("stato led ingresso: ");
      Serial.println(stato[INGRESSO]);
    }
    
    //polling pulsante SCALA
    in=digitalRead(CMDSCALA);
    if(transizione(in,SCALA)){
      if(in==HIGH){ //se fronte di salita
        startTimer(TSCALA, TMRSCALA);
        stato[SCALA] = !stato[SCALA];
        digitalWrite(LEDSCALA,stato[SCALA]);
        Serial.print("stato led scala: ");
        Serial.println(stato[SCALA]);
      }
    }
    
    //polling pulsante SALA
    in=digitalRead(CMDSALA);
      if(transizione(in,SALA)){
        if(in==HIGH){ //se fronte di salita
	  startTimer(TSPEGNI, TMRSPEGNI);
          stato[SALA] = !stato[SALA];
          digitalWrite(LEDSALA,stato[SALA]);
          Serial.print("stato led sala: ");
      	  Serial.println(stato[SALA]);
	}else{
	  stopTimer(TMRSPEGNI);
	}
    }    

    //INGRESSO
    in=digitalRead(CMDINGRESSO);
      if(transizione(in,INGRESSO)){
        if(in==HIGH){ //se fronte di salita
          startTimer(TSPEGNI, TMRSPEGNI);
          stato[INGRESSO] = !stato[INGRESSO];
          digitalWrite(LEDINGRESSO,stato[INGRESSO]);
          Serial.print("stato led ingresso: ");
      	  Serial.println(stato[INGRESSO]);
	}else{
	  stopTimer(TMRSPEGNI);
	}
    }
  } //chiudi schedulatore 
  
  if(Serial.available() > 0 ){//anche while va bene!
    short val;
    String instr = Serial.readString();
	
    if(instr.indexOf("\"statosala\":\"on\"") >= 0){
	startTimer(TSICUREZZA, TMRSICUREZZA);
	digitalWrite(LEDSPIA, LOW);
    }
  }
}
```
