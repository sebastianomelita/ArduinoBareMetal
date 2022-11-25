
>[Torna all'indice](indexpulsanti.md) >[versione in Python](gruppipulsantipy.md)

###  **PULSANTE TOGGLE**

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

>[Torna all'indice](indexpulsanti.md) >[versione in Python](gruppipulsantipy.md)
