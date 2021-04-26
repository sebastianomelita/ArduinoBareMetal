>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in C++](tasksched.md)

## **SCHEDULATORE COMPITI BASATO SUL CONTEGGIO DEI TICK**

Di seguito è riportato un esempio di schedulatore che pianifica nel tempo **l’esecuzione periodica** di una serie di **compiti** (task) da eseguire con **cadenza diversa**.

Il **tempo base** è la base dei tempi di tutte le schedulazioni.

Le varie schedulazioni **sono calcolate** a partire da un **multiplo intero** del tempo base, ne segue che il tempo base dovrebbe essere calcolato come il massimo comune divisore (**MCD**) di tutti i tempi che devono essere generati.

Il conteggio dei multipli del tempo base è tenuto da un **contatore circolare** (step) che deve essere **ruotato** dopo aver effettuato un numero di conteggi superiori al **massimo dei multipli** del tempo base necessari.

Se ci sono **pulsanti** da gestire insieme ad altri task il tempo base può essere impostato tra 50 e 200 mSec in maniera da poterlo utilizzare per effettuare un **polling degli ingressi** immune dal fenomeno dei rimbalzi (**antibounce SW**).
```Python
from gpio import *
from time import *

def main():
	pulsante = 0
	led1 = 1
	led2 = 2
	pinMode(led1, OUT)
	pinMode(led2, OUT)
	ledstate1 = LOW
	ledstate2 = LOW
	precm = 0
	precval = 0
	step = 0
	tbase = 1
	nstep = 100
	
	while True:
		# il codice eseguito al tempo massimo della CPU va qui	
		# .........
		if (uptime() - precm) >= tbase:  	   # schedulatore (e anche antirimbalzo)
			precm = uptime()  			   # preparo il tic successivo	
			step = (step + 1) % nstep      # conteggio circolare arriva al massimo a nstep-1
			# il codice eseguito al tempo base va quì	
			# ..........
			
			# task 1
			if not(step % 2):      # schedulo eventi al multiplo del tempo stabilito (2 sec)
				ledstate1 = HIGH - ledstate1
				digitalWrite(led1, ledstate1)  # stato alto: led blink
								
			# task 2
			if not(step % 3):      # schedulo eventi al multiplo del tempo stabilito (3 sec)
				ledstate2 = HIGH - ledstate2
				digitalWrite(led2, ledstate2)  # stato alto: led blink
								
			# il codice eseguito al tempo base va quì	
			# ..........
				
		# il codice eseguito al tempo massimo della CPU va qui	
		# ........
		
if __name__ == "__main__":
	main()
```
E’ buona norma evitare l’esecuzione frequente di operazioni lente quando queste non sono strettamente necessarie in modo da lasciare spazio ad altre operazioni, quali ad esempio gestione degli eventi di input, che richiedono una velocità maggiore per usufruirne in modo più interattivo.

**In particolare**, all'interno di un task, andrebbe accuratamente evitata l'introduzione di ritardi prolungati tramite **delay()**. Possono esserci per **breve tempo** (qualche mSec) ma, in ogni caso, non dovrebbero mai far parte della maniera (**algoritmo**) con cui la logica di comando **pianifica i suoi eventi**.

Utilizzando la tecnica della **schedulazione esplicita dei task** nel loop(), la **realizzazione di un algoritmo** non dovrebbe essere pensata in **maniera sequenziale (o lineare)** ma, piuttosto, come una **successione di eventi** a cui corrispondono dei **compiti** (task) che **comunicano** con le periferiche di input/output ed, eventualmente, dialogano tra loro in qualche modo (ad es. variabili globali, buffer, ecc.).
>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in C++](tasksched.md)

