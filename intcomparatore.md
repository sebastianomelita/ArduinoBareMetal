## **Comparatore analogico**

Il chip Atmega328 ha un comparatore di tensione analogico integrato. Cioè, può attivare un interrupt quando una tensione in ingresso supera una soglia. Il comparatore ha due ingressi differenziali V+ e V- dei quali esegue la differenza. 
-	Uno degli ingressi, il negativo è usato come riferimento, è il pin 13 sul chip e corrisponde a D7 su Arduino
-	mentre il positivo è usato come ingresso per leggere il valore della tensione da confrontare col riferimento. Sta sul pin 12 del chip corrispondente a D6 sulla scheda

Se la tensione sul pin di ingresso è maggiore (o minore) di quella nel pin di riferimento l’uscita del comparatore assume il valore HIGH (LOW).

All’operazione di confronto del comparatore si possono associare alcuni eventi segnalabili con un interrupt:
-	**rising:** segnala quando la tensione nel pin di ingresso supera quella nel pin di riferimento 
-	**falling:** segnala quando la tensione nel pin di ingresso scende al di sotto di quella nel pin di riferimento
-	**toggle:** segnala entrambi gli eventi precedenti (con soglie differenti per ciascuno per evitare oscillazioni)

Lo schema interno ad Arduino del comparatore è:

 


Un esempio di setup HW delle porte D6 e D7 potrebbe essere:

|    ACIS1 | ACIS0 | Mode           |
|:--------:|:-----:|:---------------|
|    0     |  0    | Toggle         |
|  0       |   1   | Reserved       |
|	  1      |    0  | Falling Edge   |
|	  1      |    1  | Rising Edge    |


Un esempio di sketch SW potrebbe essere:

```C++
volatile boolean triggered;

ISR (ANALOG_COMP_vect)
  {
  triggered = true;
  }

void setup ()
  {
  Serial.begin (115200);
  Serial.println ("Started.");
  ADCSRB = 0;           // (Disable) ACME: Analog Comparator Multiplexer Enable
  ACSR =  bit (ACI)     // (Clear) Analog Comparator Interrupt Flag
        | bit (ACIE)    // Analog Comparator Interrupt Enable
        | bit (ACIS1);  // ACIS1, ACIS0: Analog Comparator Interrupt Mode Select (trigger on falling edge)
   }  // end of setup

void loop ()
  {
  if (triggered)
    {
    Serial.println ("Triggered!"); 
    triggered = false;
    }
  
  }  // end of loop
```
È possibile anche una impostazione che realizza un collegamento interno tra l’uscita del convertitore ADC e l’ingresso del riferimento del comparatore. Questa impostazione permette di modificare il valore del riferimento in base al valore di più ingressi analogici campionati e digitalizzati in intervalli di tempo differenti (multiplexing).

