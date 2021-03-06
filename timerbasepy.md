
>[Torna all'indice](indextimers.md) >[versione in C++](timerbase.md)
## **POLLED TIMERS**

E’ possibile realizzare dei timers ottenuti eseguendo, ad ogni loop(), il **polling della funzione millis()** per verificare che questa abbia raggiunto il **valore finale** stabilito come target (obiettivo), quando ciò accade, possiamo fare eseguire il blocco di istruzioni **associato** a questo evento.

I timers possono essere **periodici**, ed uno di questi era proprio lo schedulatore adoperato per generare la base degli eventi di sistema, oppure possono essere **aperiodici** (detti anche monostabili), cioè devono essere attivati, con una istruzione apposita, solamente quando serve.

**In** **generale**, possiamo individuare alcune **componenti del timer**:

- **reset** del timer. Accade al verificarsi di una certa **condizione**.

- **polling** del tempo trascorso. Viene fatto ad ogni ciclo di **loop** (talvolta meno).

- **istruzioni triggerate** (scatenate) dal timer. Vengono eseguite (o non eseguite) in base al **tempo** **misurato** dal timer

- **condizione di attivazione** posta in punto qualsiasi del loop determina, quando è vera, la partenza del timer

- (opzionale) la **variabile stato** del timer che **abilita** il timer quando serve e lo **disabilita** quando non serve per evitare **attivazioni spurie** accidentali.

Un esempio di **timer periodico** (del tutto analogo ad un o schedulatore) potrebbe apparire così:

```Python
# Timer periodico
from gpio import *
from time import *

def main():
	PERIODO = 1  		# tempo in sec
	prectimer1 = 0
	timerState = False 	# stato del timer

	while True:
		if (timerState == True) and (uptime() - prectimer1) >= PERIODO:
		prectimer1 = uptime()
		#....
		# istruzioni eseguite periodicamente, se attivo…	

if __name__ == "__main__":
	main()
```

Di seguito è un esempio di **timer aperiodico** che misura il tempo a partire dal verificarsi di una certa condizione fino a che, al superamento di un tempo limite, si decide di fare qualcosa ma solo **dopo** lo scadere del timer:

```Python
# Timer aperiodico 1
from gpio import *
from time import *

def main():
	TLIMITE1 = 1  		# tempo in sec
	atimer1 = 0

	while True:
		# blocco condizione di attivazione
		if condA:
			atimer1 = uptime()
			
		# blocco polling
		if uptime() - atimer1 >= TLIMITE1:
			# istruzioni eseguite allo scadere del timer 1

if __name__ == "__main__":
	main()
```
La **condizione**, in realtà, può essere collocata in un punto qualsiasi del loop() (può essere ad esempio attivata dalla pressione di un pulsante):

```Python
# Timer aperiodico 1
from gpio import *
from time import *

def main():
	TLIMITE1 = 1  		# tempo in sec
	atimer1 = 0

	while True:
		# blocco polling
		if uptime() - atimer1 >=  TLIMITE1:
			# istruzioni eseguite allo scadere del timer 1
		
		# blocco condizione di attivazione
		if condA:
			atimer1 = uptime()

if __name__ == "__main__":
	main()
```
Le istruzioni eseguite allo scadere del timer possono essere inserite in una **callback**, funzione dal nome sempre uguale, che, di volta in volta, è invocata dal timer con un diverso corpo di istruzioni:
```Python
# Timer aperiodico 1
from gpio import *
from time import *

def onElapse():
	# istruzioni eseguite allo scadere del timer 1

def main():
	TLIMITE1 = 1  		# tempo in sec
	atimer1 = 0

	while True:
		# blocco polling
		if uptime() - atimer1 >=  TLIMITE1:
			onElapse()
		
		# blocco condizione di attivazione
		if condA:
			atimer1 = uptime()

if __name__ == "__main__":
	main()
```

Reset del timer, polling del tempo trascorso e istruzioni triggerate (scatenate) dal timer potrebbero anche essere **rinchiuse** in altrettante **funzioni**. 
Inoltre viene introdotta una **variabile di stato** che potrebbe essere adoperata sia per **bloccare** il timer in un certo momento come per **riattivarlo** in un momento successivo, per il tempo rimanente prima del timeout:

```Python
# Timer aperiodico 1
from gpio import *
from time import *

def main():
	#inizio variabili timer
	startTime = 0  		# tempo in sec
	timelapse = 0
	timerState = False
	#fine variabili timer
	
	# funzione di attivazione
	def startTimer(duration):
		timerState = 1 
		timelapse = duration
		startTime = uptime()

	# funzione di disattivazione
	def stopTimer():
		timerState = False
		
	# callback: azione standard da compiere allo scadere del timer, definita fuori dal loop
	def onElapse():
		# azione da compiere
		#.......

	# polling: verifica se è arrivato il tempo di far scattare il timer
	def aggiornaTimer():
		if (timerState == True) and (uptime() - startTime >= timelapse):
			timerState = False
			onElapse()

	while True:
		# blocco polling
		aggiornaTimer()  # aggiorna il primo timer
			
		if(A):	# blocco condizione di attivazione
			startTimer(1)

		if(B): 	# blocco condizione di disattivazione
			stopTimer()  

if __name__ == "__main__":
	main()
```
>[Torna all'indice](indextimers.md)  >[versione in C++](timerbase.md)
