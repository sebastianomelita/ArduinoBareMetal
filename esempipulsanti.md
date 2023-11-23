
>[Torna all'indice](indexpulsanti.md) >[versione in Python](gruppipulsantipy.md)

###  **Curva pericolosa1**

Realizzare un programma Arduino che dati 4 led (led1, led2, led3, led4) realizza un indicatore di curva pericolosa 
che accende i led in sequenza con un incremento circolare. Usare un tasto che può essere premuto in successione 
per accendere o spegnere l’indicatore (toggleH). Supponiamo che il tasto è collegato secondo lo schema pull down.

```C++
bool button_state; 					//stato del pulsante
int led1, led2, led3, led4, led5;	//pin dei led
int button;							//pin del pulsante
int time_on, time_off;				//tempi di accensione e spegnimento

void setup()
{ 
  //inizializzazione dele variabili corrispondenti ai pin
  led1 = 13;
  led2 = 12;
  led3 = 8;
  led4 = 7;
  led5 = 4;
  button = 2;
  
  
  button_state = false;
  time_on = 300;
  time_off = 50;
  
  pinMode(led1, OUTPUT);	//Pin13 uscita
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  pinMode(button, INPUT);	//Pin2 ingresso
  
}

void loop()
{
  button_state = digitalRead(button);
  
  if(button_state == true)
  {
  	digitalWrite(led5,HIGH);
  	delay(time_on);
  	digitalWrite(led5, LOW);
  	delay(time_off);
  
  	digitalWrite(led4, HIGH);
  	delay(time_on); 
  	digitalWrite(led4, LOW);
  	delay(time_off);
  
  	digitalWrite(led3, HIGH);
  	delay(time_on); 
  	digitalWrite(led3, LOW);
  	delay(time_off);
  
  	digitalWrite(led2, HIGH);
  	delay(time_on); 
  	digitalWrite(led2, LOW);
  	delay(time_off);
  
  	digitalWrite(led1, HIGH);
  	delay(time_on); 
  	digitalWrite(led1, LOW);
  	delay(time_off);
  }
  else
  {
    digitalWrite(led1, HIGH);
  	delay(time_on); 
  	digitalWrite(led1, LOW);
  	delay(time_off); 
  
  	digitalWrite(led2, HIGH);
  	delay(time_on); 
  	digitalWrite(led2, LOW);
  	delay(time_off);
  
  	digitalWrite(led3, HIGH);
  	delay(time_on); 
  	digitalWrite(led3, LOW);
  	delay(time_off);
  
  	digitalWrite(led4, HIGH);
  	delay(time_on); 
  	digitalWrite(led4, LOW);
  	delay(time_off);
  
  	digitalWrite(led5, HIGH);
  	delay(time_on); 
  	digitalWrite(led5, LOW);
  	delay(time_off);
  
  }
  
}
```

Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/1EjA2PYVNZl?editbtn=1

###  **Curva pericolosa2**

Realizzare un programma Arduino che dati 4 led (led1, led2, led3, led4) realizza un indicatore di curva pericolosa 
che accende i led in sequenza con un incremento circolare. Usare un tasto che può essere premuto in successione 
per accendere o spegnere l’indicatore (toggleH). Supponiamo che il tasto è collegato secondo lo schema pull down.

```C++
/*Realizzare un programma Arduino che dati 4 led (led1, led2, led3, led4) realizza un indicatore di curva pericolosa 
che accende i led in sequenza con un incremento circolare. Usare un tasto che può essere premuto in successione 
per accendere o spegnere l’indicatore (toggleH). Supponiamo che il tasto è collegato secondo lo schema pull down.
*/
int led1=13; // led associato alle porte
int led2=12;
int led3=8;
int led4=7;
int led5=4;
int tasto=2;
int count, ledCount; // indica quale led si spegne e poi il successivo che si accende quando lo incrementiamo
bool stato=0;// sono le variabili globali del toggleH stato indica lo stato del toggleH se acceso o spento che poi viene copiato sul led con digitalwrite
int precval=0;//  e precval indica il valore precedente del tasto se premuto è 1 altrimenti 0
int v[5]={4,7,8,12,13};
  
// attesa evento con tempo minimo di attesa
void waitUntilInputLow(int btn, unsigned t)
{
    while(!digitalRead(btn)==LOW){
	    delay(t);
    }
}
  
void setup() 
{ 
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(led3,OUTPUT);
  pinMode(led4,OUTPUT);
  pinMode(led5,OUTPUT);
  pinMode(tasto,INPUT);
  digitalWrite(led1,LOW); // nel pulldown è LOW
  digitalWrite(led2,LOW);
  digitalWrite(led3,LOW);
  digitalWrite(led4,LOW);  
  digitalWrite(led5,LOW); 
  count=0;
}

void loop() 
{
	digitalWrite(v[count],LOW); // il led corrente viene spento
	count=(count+1)%5; 		 // contatore circolare
	if(digitalRead(tasto)==HIGH){			// se è alto c'è stato un fronte di salita
		stato = !stato; 				// impostazi50one dello stato del toggle
		waitUntilInputLow(tasto,0);			// attendi finchè non c'è fronte di discesa
	}
	// e spegne con memoria
	digitalWrite(v[count],stato);// copia lo stato del toggleH sul led successivo
	delay(200);
}
```

Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/fypoZVpvuSa?editbtn=1

###  **Curva pericolosa con time ticks**

Realizzare un programma Arduino che dati 4 led (led1, led2, led3, led4) realizza un indicatore di curva pericolosa 
che accende i led in sequenza con un incremento circolare. Usare un tasto che può essere premuto in successione 
per accendere o spegnere l’indicatore (toggleH). Supponiamo che il tasto è collegato secondo lo schema pull down.

```C++
/*Realizzare un programma Arduino che dati 4 led (led1, led2, led3, led4) realizza un indicatore di curva pericolosa 
che accende i led in sequenza con un incremento circolare. Usare un tasto che può essere premuto in successione 
per accendere o spegnere l’indicatore (toggleH). Supponiamo che il tasto è collegato secondo lo schema pull down.
*/
#define tbase    	50        // periodo base in milliseconds
#define nsteps      10      // numero di fasi massimo di un periodo generico

unsigned int step=0;
unsigned long prec=0;

#define EXECUTEBASE()  	if((millis()-prec) > tbase)  //evento base periodico (con periodo tbase)
#define UPDATEBASE()  		prec = millis();  step = (step + 1) % nsteps   //decimazione del tempo base per calcolo di un periodo generico

#define  ON_200ms()    if (!(step % 4))

int led1=13; // led associato alle porte
int led2=12;
int led3=11;
int led4=10;
int led5=9;
int tasto=2;
int count, ledCount; // indica quale led si spegne e poi il successivo che si accende quando lo incrementiamo
bool stato=0;// sono le variabili globali del toggleH stato indica lo stato del toggleH se acceso o spento che poi viene copiato sul led con digitalwrite
int precval=0;//  e precval indica il valore precedente del tasto se premuto è 1 altrimenti 0

bool toggleH(int val)
{
	bool changed = false;
	if(precval==LOW && val==HIGH){
		stato = !(stato);
		changed=true;
	}
	precval=val;
	return changed;
}

void setup() 
{ 
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(led3,OUTPUT);
  pinMode(led4,OUTPUT);
  pinMode(led5,OUTPUT);
  pinMode(tasto,INPUT);
  digitalWrite(led1,LOW); // nel pulldown è LOW
  digitalWrite(led2,LOW);
  digitalWrite(led3,LOW);
  digitalWrite(led4,LOW);  
  digitalWrite(led5,LOW); 
  count=0;
}

void loop() 
{
	EXECUTEBASE() { //loop eseguito ogni 50ms                
        UPDATEBASE();
		ON_200ms() {
			digitalWrite(ledCount,LOW); // il led corrente viene spento
			count=(count+1)%5; 		 // contatore circolare
			ledCount=count+9;
			toggleH(digitalRead(tasto));// richiamo la funzione toggleH nel loop, essa serve per fare il pulsante toggleH che accende 
			// e spegne con memoria
			digitalWrite(ledCount,stato);// copia lo stato del toggleH sul led successivo
		}
	}
}
```

>[Torna all'indice](indexpulsanti.md) >[versione in Python](gruppipulsantipy.md)
