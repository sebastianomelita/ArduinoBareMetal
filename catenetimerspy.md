>[Torna all'indice](indextimers.md) >[versione in C++](catenetimers.md)

## **CATENE TEMPORALI**

Talvolta si potrebbe voler eseguire un blocco di codice **continuamente** ma solo se un evento accade all’interno di un **tempo massimo** prestabilito, cioè solo prima dello scadere di un timer:

```Python
# Timer aperiodico 2
from gpio import *
from time import *

def main():

	TLIMITE	= 1000
	atimer2 = 0

	while True:
		# condizione che valuta l’accadere di un certo evento 
		# di attivazione del conteggio del tempo
		if condA:
			atimer2 = uptime()

		if uptime() – atimer2 < TLIMITE2:
			#.... 
			# istruzioni eseguite finchè NON scade il timer 2
```
Molti timers possono anche essere attivi **mimetizzati** all’interno di funzioni che sono richiamate nel loop(). 

Si potrebbe realizzare:
- un codice che, benchè sia eseguito **linearmente**, viene eseguito aggiungendo nuove parti in istanti via via **successivi**, man mano che passa il tempo. 
- una catena di timer che si attivano man mano che un certo mumero di condizioni vengono soddisfatte in cui, ogni evento che scade prima viene ritenuto **disabilitante** per tutto il resto della funzione, interropendo prematuramente la catena di operazioni:

```Python
# Timer aperiodico 2
from gpio import *
from time import *

def poll():
	if condA): # evento che attiva il timer (può essere ricorrente ma, in generale, è aperiodico)
		atimer1 = millis();

	# finchè non si raggiunge TLIMITE1 ritorna senza fare nulla
	if (millis() – atime1) <  TLIMITE1:
		return 0

	# BLOCCO_A
	# finchè non si raggiunge TLIMITE2 ritorna dopo avere eseguito il blocco di istruzioni A
	if (millis() – atimer2) <  TLIMITE2:
		return 0

	# BLOCCO_B
	# finchè non si raggiunge TLIMITE3 ritorna dopo avere eseguito il blocco di istruzioni 
	# A e B
	if (millis() – atimer1) <  TLIMITE3):
		return 0

def main():

	while True:
		poll()  # funzione con blocchi di codice eseguiti prima o dopo di certi eventi
		# …….
```

```C++
Attenzione ad un errore insidioso:
void poll()
{
if ((millis() – atimer1) < (unsigned long) TLIMITE1)
{
    .... 
    // istruzioni eseguite finchè NON scade il timer 1
}

if ((millis() – atimer2) < (unsigned long) TLIMITE2)
{
    .... 
    // istruzioni eseguite finchè NON scade il timer 2
}

}
```

In questa situazione se scade il primo timer viene comunque controllato lo scadere del secondo. La cascata degli if equivale ad un OR logico sulle condizioni di scadenza.
Se voglio che ne accada solo una posso scrivere così:

```C++
void poll()
{
if ((millis() – atimer1) < (unsigned long) TLIMITE1)
{
    .... 
    // istruzioni eseguite finchè NON scade il timer 1
}else if ((millis() – atimer2) < (unsigned long) TLIMITE2)
{
    .... 
    // istruzioni eseguite finchè NON scade il timer 2
}
}

```
Dove l’ordine stabilisce la priorità di valutazione.
>[Torna all'indice](indextimers.md) >[versione in C++](catenetimers.md)
