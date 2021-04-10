## **Interrupts sulle porte**

Per gestire un interrupt di cambio stato di un pin è necessario
-	Specificare quale pin nel gruppo. Questa è la variabile PCMSKn (dove n è 0, 1 o 2 dalla tabella seguente). Puoi avere interruzioni su più di un pin.
-	Abilita il gruppo appropriato di interrupt (0, 1 o 2)
-	Fornire un gestore di interrupt come mostrato sopra

|    ACIS1 | ACIS0                           | 
|:-------:|:--------------------------------:|
| D0	  | PCINT16 (PCMSK2 / PCIF2 / PCIE2) |
| D1	  | PCINT17 (PCMSK2 / PCIF2 / PCIE2) |
| D2	  | PCINT18 (PCMSK2 / PCIF2 / PCIE2) |
| D3	  | PCINT19 (PCMSK2 / PCIF2 / PCIE2) |
| D4	  | PCINT20 (PCMSK2 / PCIF2 / PCIE2) |
| D5	  | PCINT21 (PCMSK2 / PCIF2 / PCIE2) |
| D6	  | PCINT22 (PCMSK2 / PCIF2 / PCIE2) |
| D7	  | PCINT23 (PCMSK2 / PCIF2 / PCIE2) |
| D8	  | PCINT0  (PCMSK0 / PCIF0 / PCIE0) |
| D9	  | PCINT1  (PCMSK0 / PCIF0 / PCIE0) |
| D10	  | PCINT2  (PCMSK0 / PCIF0 / PCIE0) |
| D11	  | PCINT3  (PCMSK0 / PCIF0 / PCIE0) |
| D12	  | PCINT4  (PCMSK0 / PCIF0 / PCIE0) |
| D13	  | PCINT5  (PCMSK0 / PCIF0 / PCIE0) |
| A0	  | PCINT8  (PCMSK1 / PCIF1 / PCIE1) |
| A1	  | PCINT9  (PCMSK1 / PCIF1 / PCIE1) |
| A2	  | PCINT10 (PCMSK1 / PCIF1 / PCIE1) |
| A3	  | PCINT11 (PCMSK1 / PCIF1 / PCIE1) |
| A4	  | PCINT12 (PCMSK1 / PCIF1 / PCIE1) |
| A5	  | PCINT13 (PCMSK1 / PCIF1 / PCIE1) 

```C++
ISR (PCINT0_vect)
 {
 	// handle pin change interrupt for D8 to D13 here
	PCIFR  |= bit (PCIF0); // (se la ISR è lenta per evitare non si acceda più al loop a causa di interrupt successivi)
 }  	// end of PCINT0_vect

ISR (PCINT1_vect)
 {
 	// handle pin change interrupt for A0 to A5 here
 }  	// end of PCINT1_vect

ISR (PCINT2_vect)
 {
 	// handle pin change interrupt for D0 to D7 here
 }  	// end of PCINT2_vect

void setup ()
  { 
  	// pin change interrupt (example for D9)
  	PCMSK0 |= bit (PCINT1);  // want pin 9
  	PCIFR  |= bit (PCIF0);   // clear any outstanding interrupts
  	PCICR  |= bit (PCIE0);   // enable pin change interrupts for D8 to D13
 }
```

ATmega2560 Support
External Interrupts 
The following External Interrupts are available on the Arduino:
          
  |Pin   | PORT INT  |   ATmega2560 pin  | 
  |:----:|:-----:|:-------:|
  |21    | PD0 |0 |    43  |
  |20    |PD1  1  |   44   |
  |19    | PD2  2 |    45  |
  |18    | PD3  3  |   46  |
  | 2    |PE4  4  |    6   |
  | 3    | PE5  5  |    7  |
| n/c    |PE6  6  |    8  (fake pin 75) **|
 |n/c    | PE7  7 |     9  (fake pin 76)|
 
Pin Change Interrupts 
ATMEGA2560 Pin Change Interrupts

|  Pin  |  PORT | PCINT  |  Pin  |  PORT |    PCINT |      Pin |     PORT |   PCINT|
|:----:|:-------|------:|:------|:-----:|:---------:|:-------:|:-----:|:--------:|
|  A8   |  PK0  |16    |   10 |    PB4 |  4    |   SS    | PB0 |  0   |
||  A9  |   PK1 | 17  |     11  |   PB5 |  5  |     SCK   | PB1  | 1  |
| A10   |  PK2 | 18   |    12   |  PB6  | 6   |    MOSI  | PB2 |  2  |
| A11  |   PK3 | 19   |    13  |   PB7  | 7   |    MISO  | PB3 |  3  |
| A12  |   PK4 | 20  |     14  |   PJ1 | 10   |       |
| A13  |   PK5 | 21   |    15  |   PJ0 |  9    |        |      |
| A14  |   PK6 | 22   |     0   |  PE0 |  8 - this one is a little odd. * |
| A15  |   PK7 | 23   |        |           |        |

**Port K**

PK0 - PK7 - (89 - 82)  - A8 - A15

EICRA - External Interrupt Control Register A

|:----:|:-------|------:|:------|:-----:|:---------:|:-------:|:-----:|:---------:|
|bit   |      7 |     6  |     5 |  4  |       3   |        2 |     1  |      0   |
|name  |      -  |     -  |     - |  -  |     ISC11 |   ISC10  |  ISC01 |   ISC00 |
|set to |     0   |    0  |     0 |   0 |        0  |        0 |     0   |     1  |

```C++
PCIFR |= (1 << PCIF2);   // clear any outstanding interrupts
PCICR |= (1 << PCIE2);   // set change interrupt
PCMSK2 |= (1 << PCINT20);
ISR (PCINT0_vect) // handle pin change interrupt for ?? here {}
```

PCICR - Pin Change Interrupt Control Register	
	bit         7       6       5       4       3         2          1        0
	name        -       -       -       -       -       PCIE2      PCIE1    PCIE0
	set to      0       0       0       0       0         1          0        0
PCIE2 = 1     enable pin change interrupt on pins PCINT23 through PCINT16              ISR (PCINT2_vect)
PCIE1 = 0     don't enable pin change interrupt on pins PCINT14 through PCINT8      ISR (PCINT1_vect)
PCIE0 = 0     don't enable pin change interrupt on pins PCINT7 through PCINT0	     ISR (PCINT0_vect)
-----------------------------------------------------------------------------------------------------------------------------------------------
PCMSK2 - Pin Change Mask Register 2
bit           7           6           5           4          3            2            1           0
	name       PCINT23     PCINT22     PCINT21     PCINT20     PCINT19     PCINT18      PCINT17     PCINT16
	set to        0           0           0           0           0           1            0           0
//Set PCIE0 to enable PCMSK2 scan.
PCIFR |= (1 << PCIF2);   // clear any outstanding interrupts
PCICR |= (1 << PCIE2);   // set change interrupt
PCMSK2 |= (1 << PCINT20);
ISR (PCINT2_vect) // handle pin change interrupt for A8 - A15 here {}

