>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in C++](timesched.md)
## **ESEMPI DI SCHEDULATORI**

**SCHEDULATORE CON DELAY()**
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


**SCHEDULATORE CON UPTIME()**
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

La generazione di più task in tempi diversi risulta **molto efficiente** perchè i task vengono eseguiti priodicamente in istanti stabiliti senza bloccare l'esecuzione degli altri task. La programmazione dei task è però più complessa perchè la programmazione non è più **lineare** in quanto l'ordine di scrittura dei task non rispecchia l'ordine di esecuzione **nel tempo**. Inoltre il gestore delle schedulazioni è un algoritmo che è parte stessa del programma principale, cioè il programma principale, oltre a gestire la logica dell'applicazione deve gestire la logica del gestore delle schedulazioni. Riassumendo, la **schedulazione mediante uptime** comporta:
- **vantaggio**.  efficienza molto elevata nell'esecuzione dei programmi.
- **svantaggio**. programmazione a salti non lineare, secondo una logica iterativa.
- **svantaggio**. l'applicazione deve gestire la logica dello schedulatore

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)    >[Versione in C++](timesched.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbMTEwNDI2NDY0N119
-->
