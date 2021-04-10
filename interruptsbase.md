>[Torna all'indice](indexinterrupts.md)
## **INTERRUPT IN ARDUINO**

Cosa tenere a mente quando si scrive una ISR():
-	Tenerla breve
-	Non usare al suo interno l’istruzione delay()
-	millis() e micros() tuttal’più forniscono il tempo di esecuzione della ISR ma non possono incrementarsi al suo interno
-	Non effettuare scritture sulla seriale
-	Qualificare le variabili condivise tra ISR() e codice principale come volatile
-	Le variabili condivise con il codice principale devono essere, all’interno di questo, protette da accessi concorrenti rinchiudendole dentro “sezioni critiche”
-	Non cercare di attivare o disattivare interrupt al suo interno quando non necessario

All’interno di una ISR di default gli interrupt sono disabilitati per cui, tutte le funzioni che ne fanno uso, invocate in una ISR, non funzionerebbero correttamente. Millis() e micros() aggiornano il loro tic con un interrupt che non può quindi essere raccolto in una ISR. Possono però al limite essere usate per ottenere una istantanea del tempo in cui viene eseguita una ISR, ma anche questo uso potrebbe non essere consigliabile: se millis() non è aggiornata con l’interrupt di un timer in tempo utile, allora perde il conteggio di un tic. In generale, il problema non è grave se la ISR viene eseguita velocemente. Millis() per funzionare correttamente deve ricevere un interrupt entro 500 μs, ma ciò non dovrebbe essere un problema visto che una buona ISR non dovrebbe durare più di qualche microsecondo (5 μs per essere ritenuta ben fatta).
Esempio di un interrupt che rileva la transizione di stato di un pulsante e accende un led:

```C++
const byte LED = 13;
const byte BUTTON = 2;

// Interrupt Service Routine (ISR)
void switchPressed ()
{
  if (digitalRead(BUTTON) == HIGH)
    digitalWrite (LED, HIGH);
  else
    digitalWrite(LED, LOW);
}  // end of switchPressed

void setup ()
{
  pinMode(LED, OUTPUT);  	  // so we can update the LED
  digitalWrite(BUTTON, HIGH);  // internal pull-up resistor
  // attach interrupt handler
  attachInterrupt(digitalPinToInterrupt(BUTTON), switchPressed, CHANGE);  
  
}  // end of setup

void loop ()
{
  // loop doing nothing 
} 
``` 
La struttura del codice ricalca lo schema di principio (modello a destra) tipico di una invocazione ISR che avviene tutta all’esterno del loop e in maniera indipendente da questo (che può anche non fare nulla). 
Il modello a sinistra è invece il tipo schema di gestione della periferica mediante polling puro, cioè la gestione delle periferiche avviene tutta all’interno del loop col programma principale, a margine del task principale.
Le due righe di codice seguenti sortiscono lo stesso effetto, cioè realizzare l’associazione tra una ISR definita dall’utente e un certo evento di interrupt su una certa porta:
```C++
attachInterrupt (0, switchPressed, CHANGE);    // that is, for pin D2
attachInterrupt (digitalPinToInterrupt (2), switchPressed, CHANGE); 
```
ma la seconda è più portabile su varie versioni di Arduino (i pin interrupt non necessariamente coincidono con i pin delle porte digitali).
Di seguito è riportata una lista completa degli interruppt di un Arduino:
```C++
 1  Reset 
 2  External Interrupt Request 0  (pin D2)          (INT0_vect)
 3  External Interrupt Request 1  (pin D3)          (INT1_vect)
 4  Pin Change Interrupt Request 0 (pins D8 to D13) (PCINT0_vect)
 5  Pin Change Interrupt Request 1 (pins A0 to A5)  (PCINT1_vect)
 6  Pin Change Interrupt Request 2 (pins D0 to D7)  (PCINT2_vect)
 7  Watchdog Time-out Interrupt                     (WDT_vect)
 8  Timer/Counter2 Compare Match A                  (TIMER2_COMPA_vect)
 9  Timer/Counter2 Compare Match B                  (TIMER2_COMPB_vect)
10  Timer/Counter2 Overflow                         (TIMER2_OVF_vect)
11  Timer/Counter1 Capture Event                    (TIMER1_CAPT_vect)
12  Timer/Counter1 Compare Match A                  (TIMER1_COMPA_vect)
13  Timer/Counter1 Compare Match B                  (TIMER1_COMPB_vect)
14  Timer/Counter1 Overflow                         (TIMER1_OVF_vect)
15  Timer/Counter0 Compare Match A                  (TIMER0_COMPA_vect)
16  Timer/Counter0 Compare Match B                  (TIMER0_COMPB_vect)
17  Timer/Counter0 Overflow                         (TIMER0_OVF_vect)
18  SPI Serial Transfer Complete                    (SPI_STC_vect)
19  USART Rx Complete                               (USART_RX_vect)
20  USART, Data Register Empty                      (USART_UDRE_vect)
21  USART, Tx Complete                              (USART_TX_vect)
22  ADC Conversion Complete                         (ADC_vect)
23  EEPROM Ready                                    (EE_READY_vect)
24  Analog Comparator                               (ANALOG_COMP_vect)
25  2-wire Serial Interface  (I2C)                  (TWI_vect)
26  Store Program Memory Ready                  (SPM_READY_vect)
```
Quelli riportati tra parentesi sono i nomi delle ISR che, internamente al sistema, vengono richiamate all’attivazione di un interrupt.
L'ordine di priorità è la sequenza in cui il processore controlla gli eventi di interrupt. Più in alto è in elenco, maggiore è la priorità. Quindi, ad esempio, una richiesta di interrupt esterno 0 (pin D2) verrebbe servita prima della richiesta di interrupt esterno 1 (pin D3).

Le ragioni principali che potrebbero spingere ad usare un interrupt sono:
-	To rilevare cambiamenti di valori nelle porte (eg. rotary encoders, button presses)
-	Watchdog timer (egse non accade nulla dopo 8 seconds, interrompimi)
-	Timer interrupts – usati per realizzare timer 
-	Trasferimenti di dati via BUS SPI
- Trasferimenti di dati via BUS I2C
-	Trasferimenti di dati via BUS USART
-	conversions ADC (da analogico a digitale)
- rilevare lo stato pronto alla lettura della EEPROM 
-	rilevare lo stato pronto della Flash memory 

>[Torna all'indice](indexinterrupts.md)
