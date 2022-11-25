
>[Torna all'indice](indexpulsanti.md) >[versione in Python](gruppipulsantipy.md)


Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/1EjA2PYVNZl?editbtn=1


###  **PULSANTE TOGGLE**

Si vuole realizzare un pulsante con una memoria dello stato che possa essere modificata ad ogni pressione. Pressioni successive in sequenza accendono e spengono un led. Quindi, a seconda da quando si comincia, pressioni in numero pari accendono mentre quelle in numero dispari spengono, oppure al contrario, pressioni in numero pari spengono mentre quelle in numero dispari accendono.

Inizialmente si potrebbe essere tentati di provare seguente soluzione, adattando la strategia del pulsante precedente introducendo una variabile che conservi lo stato del pulsante che chiameremo _closed_.
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

>[Torna all'indice](indexpulsanti.md) >[versione in Python](gruppipulsantipy.md)
