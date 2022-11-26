>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in Python](taskschedpy.md)

## **SCHEDULATORE COMPITI BASATO SUI DELAY**

### **Blink**

```C++
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(1000);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);                      // wait for a second
}

```

Di seguito il link della simulazione online con Tinkercad su Arduino: https://www.tinkercad.com/embed/h6If6Mbz0NB?editbtn=1

### **Lampeggi insieme ad azioni schedulate modo1**

```C++
/*
Dati 4 led, facendo lampeggiare continuamente per mezzo secondo il primo, accendere il secondo dopo 10 lampeggi, 
il terzo dopo 20  e fermare il lampeggio del primo dopo che ha raggiunto 30 lampeggi. 
A questo punto il meccanismo ricomincia ribamtaltato.  mentre lampeggia il quarto led, si accendonno via via il terzo a 10 lampeggi, 
il secondo a 20 e il primo riparte a lampeggiare a 40, momento in cui il quarto smette di lampeggiare. Il processo continua così 
indefinitivamente.
*/

byte led1 = 13;
byte led2 = 12;
byte led3 = 11;
byte led4 = 10;
bool uno;

void setup()
{
  Serial.begin(115200);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  uno = true;
  setLed();
  delay(1000);
  resetLed();
}

void resetLed(){
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
}

void setLed(){
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, HIGH);
}

void blink(int led){
	digitalWrite(led, HIGH);
	delay(500);
	digitalWrite(led, LOW);
	delay(500);
}

void loop()
{
  if(uno){
	for(int i=0; i<10; i++)
		blink(led1);
	digitalWrite(led2, HIGH);
	for(int i=0; i<10; i++)
		blink(led1);
	digitalWrite(led3, HIGH);	
	for(int i=0; i<10; i++)
		blink(led1);
	resetLed();
	uno = false;
  }else{
	for(int i=0; i<10; i++)
		blink(led4);
	digitalWrite(led3, HIGH);
	for(int i=0; i<10; i++)
		blink(led4);
	digitalWrite(led2, HIGH);	
	for(int i=0; i<10; i++)
		blink(led4);
	resetLed();
	uno = true;
   }
}
```

Di seguito il link della simulazione online con Tinkercad su Arduino: https://www.tinkercad.com/embed/3u5b3IxqsIY?editbtn=1

## **Lampeggi insieme ad azioni schedulate modo2**

```C++
/*
Dati 4 led, facendo lampeggiare continuamente per mezzo secondo il primo, accendere il secondo dopo 10 lampeggi, 
il terzo dopo 20  e fermare il lampeggio del primo dopo che ha raggiunto 30 lampeggi. 
A questo punto il meccanismo ricomincia ribamtaltato.  mentre lampeggia il quarto led, si accendonno via via il terzo a 10 lampeggi, 
il secondo a 20 e il primo riparte a lampeggiare a 40, momento in cui il quarto smette di lampeggiare. Il processo continua così 
indefinitivamente.
*/

byte led1 = 13;
byte led2 = 12;
byte led3 = 11;
byte led4 = 10;
int count;
bool uno;

void setup()
{
  Serial.begin(115200);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  count = 1;
  uno = true;
  setLed();
  delay(1000);
  resetLed();
}

void resetLed(){
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
}

void setLed(){
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, HIGH);
}

void loop()
{
  if(uno){
	digitalWrite(led1, HIGH);
	delay(500);
	digitalWrite(led1, LOW);
	delay(500);
	if(count==10){
		digitalWrite(led2, HIGH);
	}
	if(count==20){
		digitalWrite(led3, HIGH);	
	}
	if(count==30){
		resetLed();
		uno = false;
		count = 1;
	}else{
		count++;
	}
  }else{
	digitalWrite(led4, HIGH);
	delay(500);
	digitalWrite(led4, LOW);
	delay(500);
	if(count==10){
		digitalWrite(led3, HIGH);
	}
	if(count==20){
		digitalWrite(led2, HIGH);	
	}
	if(count==30){
		resetLed();
		uno = true;
		count = 1;
	}else{
		count++;
	}
   }
}
```

Di seguito il link della simulazione online con Tinkercad su Arduino: https://www.tinkercad.com/embed/5difrFBglzj?editbtn=1

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in Python](taskschedpy.md)
