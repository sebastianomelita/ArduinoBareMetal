>[Torna all'indice generale](index.md)     >[Versione in C++](gnerazionetempi.md)
# **GENERAZIONE DI TEMPI**

## **DELAY() E UPTIME()**

**Delay()**

Mette in pausa il programma per la quantità di tempo (in millisecondi) specificata come parametro. (Ci sono 1000 millisecondi in un secondo).

**uptime()**

Restituisce il numero di millisecondi trascorsi da quando la scheda Arduino ha iniziato a eseguire il programma corrente. Questo numero supererà (tornerà a zero), dopo circa 50 giorni (overflow).

Il tipo di dato ritornato dalla funzione è unsigned long.

Bisogna prestare attenzione alle **operazioni aritmetiche** con **espressioni** che comprendono, oltre a uptime() anche altre variabili aventi tipi più piccoli, come ad esempio int. Infatti la capacità di conteggio di variabili di dimensione inferiore ad unsigned long è minore di quella di uptime() per cui potrebbero andare in overflow in un momento differente rispetto a uptime() generando risultati scorretti.

Le misure di tempo devono sempre tenere conto del problema dell’overflow.  Sulle misure di tempo assolute (misurate a partire dall’accensione della macchina) si può fare poco, quelle andranno in overflow comunque, su quelle relative tra istanti campionati (e conservati su una variabile) e quelli misurati con la uptime(), è possibile renderlo non dannoso. Il segreto è lavorare sempre **su differenze di tempo** che non superano mai il valore di overflow, ciò si ottiene assicurandosi di **ricampionare il valore del riferimento**, a partire da cui si misura la differenza, prima che l'evento di overflow accada. 

Se si considera la differenza:
```Python
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

La generazione di più task in tempi diversi risulta **molto eficciente** perchè i task vengono eseguiti priodicamente in istanti stabiliti senza bloccare l'esecuzione degli altri task. La programmazione dei task è però più complessa perchè la programmazione non è più **lineare** in quanto l'ordine di scrittura dei task non rispecchia l'ordine di esecuzione **nel tempo**. Inoltre il gestore delle schedulazioni è un algoritmo che è parte stessa del programma principale, cioè il programma principale, oltre a gestire la logica dell'applicazione deve gestire la logica del gestore delle schedulazioni. Riassumendo, la **schedulazione mediante uptime** comporta:
- **vantaggio**.  efficienza molto elevata nell'esecuzione dei programmi.
- **svantaggio**. programmazione a salti non lineare, secondo una logica iterativa.
- **svantaggio**. l'applicazione deve gestire la logica dello schedulatore


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

# **SCHEDULAZIONE CON I THREAD**

I thread, detti anche processi leggeri, sono dei **flussi di esecuzione** separati da quello principale (il programma main) che procedono **indipendentemente** l'uno dall'altro e soprattutto in maniera **paralllela** cioè **contemporaneamente** l'uno con l'altro. Il **parallelismo** può essere:
- **reale** se flussi di esecuzione diversi sono eseguiti da core (o CPU) diversi. Possiede la proprietà di effettiva **simultaneità** nell'esezuzione di più istruzioni.
- **emulato** se flussi di esecuzione diversi sono eseguiti dallo stesso core della stessa CPU. La proprietà di **simultaneità** è relativa all'esezuzione di più **programmi** nello stesso momento ma con **istruzioni** dell'uno e dell'altro eseguite in momenti diversi (tenica dell'interleaving).

Normalmente una istruzione delay(x) fa attendere per x secondi non solo l'esecuzione di un certo task ma anche quella di tutti gli altri che quindi, in quel frattempo, sono bloccati. Il motivo consiste nel fatto che tutti i task condividono lo stesso flusso di esecuzione o thread e, se questo viene fermato, viene fermato per tutti.

Se però due o più task vengono eseguiti su thread differenti è possibile bloccarne uno con un delay impedendo temporaneamente ad un suo task di andare avanti, ma laciando liberi tutti gli altri task sugli altri thread di proseguire la loro esecuzione senza interruzioni di sorta. 

Abbiamo visto che usare i delay per progettare i tempi di un task è più semplice perchè la programmazione rimane quella lineare a cui è solito pensae gli algoritmi un programmatore ma, in questo caso è molto meno inefficiente che in un programma a singolo thread dato che la CPU può sempre servire tutti i task nello stesso momento (in maniera reale o simulata). 

Un **altro vantaggio** per il programmatore è che la gestione della schedulazione è trasparente alla applicazione che sta realizzando nel senso che l'algoritmo che deve gestirla non deve essere incluso nel codice dell'applicazione. Qualcuno deve comunque gestire la schedulazione nel tempo dei thread e questo qualcuno è un **modulo SW** diverso dall'applicazione che può:
- essere fornito da una **libreria** apposita
- essere incluso in framework di terze parti (**middleware**)
- essere fornito dal **sistema operativo** presente sulla macchina

Il **costo da pagare** è una certa dose di **inefficienza residua** perchè la shedulazione dei thread può essere fatta in maniera più o meno sofisticata ma comunque richiede l'utilizzo di un certo ammontare della risorsa CPU. Il peso di queste inefficienze potrebbe diventare insostenibile in presenza di **molti task** che girano su sistemi con limitate risorse di calcolo, come sono tipicamente i **sistemi embedded**.

Riassumendo, la **schedulazione mediante thread** comporta:
- **vantaggio**.  Maggiore semplicità nella progettazione dei programmi, grazie alla possibilità di utilizzare uno stile lineare di programmazione.
- **vantaggio**. Maggiore semplicità nella progettazione dei programmi che non devono realizzare la logica della schedulazione dei task ma solo quella interna al singolo task.
- **svantaggio**. maggore inefficienza nell'uso della risorsa CPU che deve comunque eseguire un thread a parte di gestione delle schedulazioni sugli altri thread.
- - **svantaggio**.Bisogna conoscere le API di libreria con cui viene realizzato il multithreading in un certo sistema con un certo linguaggio. Le maniere possono essere parecchie per cui potrebbe essere utile ricorrere ad interfacce standard consolidate (ad esempio Thread POSIX)

```Python
#
# example ESP32 multitasking
# phil van allen
#
# thanks to https://youtu.be/iyoS9aSiDWg
#
import _thread as th
import time
from machine import Pin

blink1_running = True
blink2_running = True

led1 = Pin(4, Pin.OUT)
led2 = Pin(13, Pin.OUT)

def blink1(delay):
     while blink1_running:
         led1.value(not led1.value())
         time.sleep(delay)
     led1.value(0)

def blink2(delay):
     while blink2_running:
         led2.value(not led2.value())
         time.sleep(delay)
     led2.value(0)

print("Starting other tasks...")
th.start_new_thread(blink1, (0.5,))
th.start_new_thread(blink2, (0.25,))

count = 0
while True:
  print("Doing stuff... " + str(count))
  count += 1
  if count >= 10:
    break
  time.sleep(1)

print("Ending threads...")
blink1_running = False
blink2_running = False
```

>[Torna all'indice generale](index.md)  >[Versione in C++](gnerazionetempi.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbMTEwNDI2NDY0N119
-->
