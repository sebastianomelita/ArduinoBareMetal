>[Torna all'indice generale](index.md)     >[Versione in C++](gnerazionetempi.md)
# **GENERAZIONE DI TEMPI**

## **DELAY() E uptime()**

**Delay()**

Mette in pausa il programma per la quantità di tempo (in millisecondi) specificata come parametro. (Ci sono 1000 millisecondi in un secondo).

**uptime()**

Restituisce il numero di millisecondi trascorsi da quando la scheda Arduino ha iniziato a eseguire il programma corrente. Questo numero supererà (tornerà a zero), dopo circa 50 giorni (overflow).

Il tipo di dato ritornato dalla funzione è unsigned long.

Bisogna prestare attenzione alle **operazioni aritmetiche** con **espressioni** che comprendono, oltre a uptime() anche altre variabili aventi tipi più piccoli, come ad esempio int. Infatti la capacità di conteggio di variabili di dimensione inferiore ad unsigned long è minore di quella di uptime() per cui potrebbero andare in overflow in un momento differente rispetto a uptime() generando risultati scorretti.

Le misure di tempo devono sempre tenere conto del problema dell’overflow.  Sulle misure di tempo assolute (misurate a partire dall’accensione della macchina) si può fare poco, quelle andranno in overflow comunque, su quelle relative tra istanti campionati (e conservati su una variabile) e quelli misurati con la uptime(), è possibile renderlo non dannoso. Il segreto è lavorare sempre **su differenze di tempo** che non superano mai il valore di overflow, ciò si ottiene assicurandosi di **ricampionare il valore del riferimento**, a partire da cui si misura la differenza, prima che l'evento di overflow accada. 

Se si considera la differenza:
```C++
	uptime() - precm
```
se precm è dello stesso tipo di uptime()(), cioè è di tipo unsigned long, allora sia l’una che l’altra andranno in overflow e in istanti di tempo diversi. Per effetto di un evento di overflow i valori assoluti delle variabili perderanno di significato mentre quelli relativi continueranno a mantenerlo infatti, nonostante le due variabili vadano in overflow in momenti diversi, la differenza relativa si mantiene ugualmente corretta. 

Ciò accade perché l’operazione di sottrazione delle variabili senza segno nei computer avviene modulo il valore massimo di conteggio della variabile.

Infatti si può verificare che il programma seguente restituisce il valore 2.
```Python
def main():
	pulsante = 0
	a = 1
	b = 4294967295
	print(a-b)

	while True:
		pass

if __name__ == "__main__":
	main()
```

## **ESEMPI DI SCHEDULATORI**

**Schedulatore con delay().**
```Python
	from gpio import *
from time import *

def main():
	pulsante = 0
	led = 1
	pinMode(led, OUT)
	pinMode(pulsante, IN)

	while True:
		#codice eseguito al tempo stabilito
		precm = uptime()  			   # preparo il tic successivo	
		val = digitalRead(pulsante)    # lettura ingressi
		# Inserire qui la logica di comando
		# ...........
		delay(100)  # ritarda del tempo necessario
			

if __name__ == "__main__":
	main()
```

**Schedulatore con uptime().**
```Python
	from gpio import *
from time import *

def main():
	pulsante = 0
	led = 1
	pinMode(led, OUT)
	pinMode(pulsante, IN)
	precm = 0
	precval = 0

	while True:
		if (uptime() - precm) >= tbase:  	   # schedulatore (e anche antirimbalzo)
			precm = uptime()  			   # preparo il tic successivo	
			val = digitalRead(pulsante)    # lettura ingressi
			# Inserire qui la logica di comando
			#……………………………………	

if __name__ == "__main__":
	main()
```

**_tbase_** rappresenta la **distanza** del **prossimo** tempo “buono” dall’**ultimo** valutato. Ogni **istante stabilito** viene misurato a **partire dall’ultimo** calcolato. Si determina il **tempo attuale** di esecuzione aggiungendo un tbase all’**ultimo tempo buono calcolato** e preventivamente **memorizzato** in precm. Facciamo una simulazione.

Se **tbase** vale 1000 msec e **precm** vale 0, accade che per 0, 1, 2,…,999 msec l’if **non scatta** perché la condizione è falsa **poi**, dopo, al uptime che restituisce il valore 1000, **scatta** e si esegue **il compito schedulato**. In definitiva l’if **ignora** 999 chiamate loop() mentre **agisce** alla millesima che **capita** esattamente dopo un secondo. Dopo questo momento **precm** vale adesso 1000, uptime(), ad ogni loop(), vale, ad es., 1001, 1002,…,1999 msec, l’if **non scatta** perché la condizione è falsa **poi**, dopo, al uptime che restituisce il valore 2000, **scatta,** si **aggiorna** nuovamente **precm** al valore attuale di uptime(), cioè 2000, e si **esegue** nuovamente il **compito schedulato**. In maniera analoga si preparano gli **scatti successivi**.



## **SCHEDULATORE COMPITI**

Di seguito è riportato un esempio di schedulatore che pianifica nel tempo **l’esecuzione periodica** di una serie di **compiti** (task) da eseguire con **cadenza diversa**.

E’ buona norma evitare l’esecuzione frequente di operazioni lente quando queste non sono strettamente necessarie in modo da lasciare spazio ad altre operazioni, quali ad esempio gestione degli eventi di input, che richiedono una velocità maggiore per usufruirne in modo più interattivo.

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
## **MISURE DI TEMPO ASSOLUTE**

Per ottenere una cadenza periodica precisa è necessario usare una forma diversa dal solito schedulatore più adatta a cumulare con precisione lunghe misure di tempo. E’essenziale che l’accumulatore tass venga aggiornato esattamente con il tempo campionato. L’accumulatore unisce i vari campionamenti per ottenere una misura unica. Con questa forma ci si può aspettare un errore di qualche secondo all’ora dipendente solo dall’imprecisione dell’oscillatore.
```Python
from gpio import *
from time import *

def main():
	tass = 0
	periodo = 10
		
	while True:
		# il codice eseguito al tempo massimo della CPU va qui	
		# .........
		if (uptime() - tass) >= periodo:  	   # schedulatore (e anche antirimbalzo)
			tass += periodo 			   
			# ..........	

if __name__ == "__main__":
	main()
```
Invece la forma seguente è errata. La condizione viene valutata in ritardo rispetto al momento ideale, “reimpostando” la variabile ‘tass” al tempo attuale, questo ritardo si aggiunge a tutti i ritardi precedenti. Con questa forma ci si può aspettare un errore di diversi secondi al minuto o anche peggiore.
```Python
from gpio import *
from time import *

def main():
	tass = 0
	periodo = 10
		
	while True:
		# il codice eseguito al tempo massimo della CPU va qui	
		# .........
		if (uptime() - tass) >= periodo:  	   # schedulatore (e anche antirimbalzo)
			tass = uptime()  			   
			# ..........	

if __name__ == "__main__":
	main()
```
>[Torna all'indice generale](index.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbMTEwNDI2NDY0N119
-->
