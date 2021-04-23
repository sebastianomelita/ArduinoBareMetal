>[Torna all'indice generale](index.md)     >[Versione in C++](timefunc.md)
# **GENERAZIONE DI TEMPI**

## **DELAY() E UPTIME()**

**delay()**

Mette in pausa il programma per la quantità di tempo (in millisecondi) specificata come parametro. (Ci sono 1000 millisecondi in un secondo).

**uptime()**

Restituisce il numero di millisecondi trascorsi da quando la scheda Arduino ha iniziato a eseguire il programma corrente. Questo numero supererà il massimo (e tornerà a zero) dopo circa 50 giorni (overflow).

Il tipo di dato ritornato dalla funzione è `unsigned long`.

Bisogna prestare attenzione alle **operazioni aritmetiche** con **espressioni** che comprendono, oltre ad `uptime()`, anche altre variabili aventi tipi con dinamica minore (più piccoli), come ad esempio `int`. Infatti la capacità di conteggio di variabili di dimensione inferiore ad `unsigned long` è minore di quella di `uptime()` per cui potrebbero andare in overflow in un momento differente rispetto ad `uptime()` generando risultati scorretti.

Le misure di tempo devono sempre tenere conto del problema dell’overflow. Sulle misure di tempo assolute (misurate a partire dall’accensione della macchina) si può fare poco, quelle andranno in overflow comunque; quelle relative tra istanti campionati (e conservati su una variabile) e misurati con la `uptime()` è invece possibile renderle non dannose. Il segreto è lavorare sempre **su differenze di tempo** che non superano mai il valore di overflow, ciò si ottiene assicurandosi di **ricampionare il valore del riferimento**, a partire da cui si misura la differenza, prima che l'evento di overflow accada. 

Se si considera la differenza:
```Python
	uptime() - precm
```
se `precm` è dello stesso tipo di `uptime()`, cioè è di tipo `unsigned long`, allora sia l’una che l’altra andranno in overflow e in istanti di tempo diversi. Per effetto di un evento di overflow i valori assoluti delle variabili perderanno di significato mentre quelli relativi continueranno a mantenerlo: infatti, nonostante le due variabili vadano in overflow in momenti diversi, la differenza relativa si mantiene ugualmente corretta. 

Ciò accade perché l’operazione di sottrazione delle variabili senza segno nei computer avviene modulo il valore massimo di conteggio della variabile.

Infatti si può verificare che il programma seguente restituisce il valore 2.
```Python
def main():
	pulsante = 0
	a = 1
	b = 4294967295
	print(a-b) # output: 2

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
La generazione di più task in tempi diversi risulta **molto semplice** se eseguita con la funzione delay() soprattutto per la natura **lineare** della programmazione che permette di scrivere il codice dei task con lo **stesso ordine** con cui questi verranno effettivamente eseguiti **nel tempo**. Lo svantaggio di questa realizzazione è una sostanziale **inefficienza** perchè il **blocco di un task** comporta anche il blocco di **tutti gli altri**, compreso il programma principale (**main**). Riassumendo, la **schedulazione mediante delay** comporta:
- **vantaggio**. Programmazione lineare molto semplice secondo una logica strettamente sequenziale
- **svantaggio**. Inefficienza intrinseca nell'esecuzione dei programmi.


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

La generazione di più task in tempi diversi risulta **molto eficiente** perchè i task vengono eseguiti priodicamente in istanti stabiliti senza bloccare l'esecuzione degli altri task. La programmazione dei task è però più complessa perchè la programmazione non è più **lineare** in quanto l'ordine di scrittura dei task non rispecchia l'ordine di esecuzione **nel tempo**. Inoltre il gestore delle schedulazioni è un algoritmo che è parte stessa del programma principale, cioè il programma principale, oltre a gestire la logica dell'applicazione deve gestire la logica del gestore delle schedulazioni. Riassumendo, la **schedulazione mediante uptime** comporta:
- **vantaggio**.  efficienza molto elevata nell'esecuzione dei programmi.
- **svantaggio**. programmazione a salti non lineare, secondo una logica iterativa.
- **svantaggio**. l'applicazione deve gestire la logica dello schedulatore

>[Torna all'indice generale](index.md)  >[Versione in C++](timefuncpy.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbMTEwNDI2NDY0N119
-->
