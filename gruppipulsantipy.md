
>[Torna all'indice](indexpulsanti.md) >[versione in C++](gruppipulsanti.md)
## **GESTIONE GRUPPI DI PULSANTI**

La **logica di molti** pulsanti potrebbe essere gestita sotto forma di **dati strutturati** (multivalore) in una delle seguenti maniere: array, struct ed oggetti.

In ciascuna delle precedenti strutture dati vanno memorizzate almeno **due cose**: lo **stato** del pulsante, la memoria del suo **ingresso passato**, cioè il valore dell’ingresso al loop precedente (serve per rilevare una transizione).

La **differenza** tra i vari modi di gestione consiste in come si memorizzano stati, ingressi e logica:

- **Array**. Una **sola funzione** gestisce la logica di più pulsanti, **stato** e **ingressi passati** sono memorizzati sulle celle di **due array paralleli**. Gli array sono comuni a tutti i pulsanti, ma ogni **indice** di entrambi gli array si riferisce sempre ad un solo pulsante.

- **Struct**. Una **sola funzione** gestisce la logica di più pulsanti, **stato** e **ingressi passati** sono memorizzati su una struct **separata** per ogni pulsante. Al limite le struct possono essere organizzate in un array, soprattutto in presenza di molti tasti.

- **Oggetti. Stato** e **ingressi passati** sono memorizzati su un oggetto **separato** per ogni pulsante. Ogni oggetto possiede anche la funzione per manipolare i dati contenuti al suo interno (metodo). I metodi sono accessibili, a partire dalla variabile oggetto, usando la “dot” notation cioè, il nome della variabile, un punto, ed il nome del metodo. Anche in questo caso, volendo, per molti tasti gli oggetti potrebbero essere organizzati in un array o in una matrice.

**Esempio di due pulsanti toggle gestiti con due array paralleli**
```Python
# Scrivere un programma Arduino che accenda due led (cucina, soggiorno).
# Accenderli con due pulsanti toggle separati.
from gpio import *
from time import *

def main():

	TBASE = 0.1

	class Pulsanti:
		SOGGIORNO = 0
		CUCINA = 1
		
	class Lampade:
		SOGGIORNO = 2
		CUCINA = 3
		
	class Ambienti:
		SOGGIORNO = 0
		CUCINA = 1
	
	def transizione(val, n): # transizione di un pulsante
		cambiato = False
		cambiato = (precval[n] != val)
		precval[n] = val
		return cambiato
	
	precval = [0, 0]
	stato = [0, 0]
	precm = 0
	pinMode(Pulsanti.SOGGIORNO, IN)
	pinMode(Pulsanti.CUCINA, IN)
	pinMode(Lampade.SOGGIORNO, OUT)
	pinMode(Lampade.CUCINA, OUT)

	while True:
		if (uptime() - precm) >= TBASE:  	   	# schedulatore (e anche antirimbalzo)
			precm = uptime()  			   		# preparo il tic successivo	
			
			#polling pulsante cucina
			val = digitalRead(Pulsanti.CUCINA)    
			if transizione(val,Ambienti.CUCINA) == True:				
				if val == HIGH: # se fronte di salita
					stato[Ambienti.CUCINA] =  (stato[Ambienti.CUCINA] + 1) % 2
					digitalWrite(Lampade.CUCINA, stato[Ambienti.CUCINA]*1023)
				
			#polling pulsante soggiorno
			val = digitalRead(Pulsanti.SOGGIORNO)    
			if transizione(val,Ambienti.SOGGIORNO) == True:				
				if val == HIGH: # se fronte di salita
					stato[Ambienti.SOGGIORNO] =  (stato[Ambienti.SOGGIORNO] + 1) % 2
					digitalWrite(Lampade.SOGGIORNO, stato[Ambienti.SOGGIORNO]*1023)
					
if __name__ == "__main__":
	main()
```
Si noti l’uso delle due **costanti enum** CUCINA e SOGGIORNO per gestire in maniera **automatica** gli **indici** dei due array paralleli
```Python
stato = [0, 0];
precval = [0, 0];
```
Ad esempio, il valori dello **stato** e dell’**ingresso** **passato** del pulsante CUCINA si trovano rispettivamente:
```C++
statoCucina = stato[Ambienti.CUCINA];
precvalCucina = precval[Ambienti.CUCINA];
```
Negli esempi successivi utilizzeremo una funzione di rilevazione delle transizioni diversa perché mirata a rilevare soltanto i fronti di salita valutando la condizione 
```Python
precval  == LOW  and val == HIGH.
```
In maniera analoga si possono creare rilevatori di fronti di discesa valutando la condizione 
```Python
precval  == HIGH  and val == LOW.
```
L’uso di queste funzioni permette di **spostare** la **logica del comando** toggle (stato = !stato dal loop alla funzione di gestione del tasto, semplificando il codice del loop.

Adesso però conviene **cambiare il nome** della funzione poiché non rappresenta più soltanto un generico rilevatore di transizioni ma anche la logica “toggle” valutata sul fronte di salita.

Volendo, una funzione analoga si poteva usare anche sopra, nel codice dei pulsanti con gli array.

**Esempio di due pulsanti toggle gestiti con due struct**
```Python
# Scrivere un programma Arduino che accenda due led (cucina, soggiorno).
# Accenderli con due pulsanti toggle separati.
from gpio import *
from time import *

def main():

	TBASE = 0.1

	class Pulsanti:
		SOGGIORNO = 0
		CUCINA = 1
		
	class Lampade:
		SOGGIORNO = 2
		CUCINA = 3
	
	class Toggle:
		precval = 0
		stato = 0
			
	def toggleH(val, btn): # transizione di un pulsante
		cambiato = False
		if btn.precval  == LOW  and val == HIGH:
			cambiato = True
			btn.stato = (btn.stato + 1) % 2 
		btn.precval = val
		return cambiato
		
	
	cucina = Toggle()
	soggiorno = Toggle()
	precm = 0
	pinMode(Pulsanti.SOGGIORNO, IN)
	pinMode(Pulsanti.CUCINA, IN)
	pinMode(Lampade.SOGGIORNO, OUT)
	pinMode(Lampade.CUCINA, OUT)

	while True:
		if (uptime() - precm) >= TBASE:  	   	# schedulatore (e anche antirimbalzo)
			precm = uptime()  			   		# preparo il tic successivo	
			
			#polling pulsante cucina
			val = digitalRead(Pulsanti.CUCINA)    
			if toggleH(val, cucina) == True:				
				digitalWrite(Lampade.CUCINA, cucina.stato*1023)
				
			#polling pulsante soggiorno
			val = digitalRead(Pulsanti.SOGGIORNO)    
			if toggleH(val, soggiorno) == True:				
				digitalWrite(Lampade.SOGGIORNO, soggiorno.stato*1023)
					
if __name__ == "__main__":
	main()
```
**Esempio di due pulsanti toggle gestiti con due oggetti**
```Python
# Scrivere un programma Arduino che accenda due led (cucina, soggiorno).
# Accenderli con due pulsanti toggle separati.
from gpio import *
from time import *

def main():

	TBASE = 0.1

	class Pulsanti:
		SOGGIORNO = 0
		CUCINA = 1
		
	class Lampade:
		SOGGIORNO = 2
		CUCINA = 3
			
	class Toggle(object):
		def __init__(self, precval, stato):
			self.precval = precval
			self.stato = stato

		def toggleH(self, val): # transizione di un pulsante
			cambiato = False
			if self.precval  == LOW  and val == HIGH:
				cambiato = True
				self.stato = (self.stato + 1) % 2 
			self.precval = val
			return cambiato
	
	cucina = Toggle(0,0)
	soggiorno = Toggle(0,0)
	precm = 0
	pinMode(Pulsanti.SOGGIORNO, IN)
	pinMode(Pulsanti.CUCINA, IN)
	pinMode(Lampade.SOGGIORNO, OUT)
	pinMode(Lampade.CUCINA, OUT)

	while True:
		if (uptime() - precm) >= TBASE:  	   	# schedulatore (e anche antirimbalzo)
			precm = uptime()  			# preparo il tic successivo	
			
			#polling pulsante cucina
			val = digitalRead(Pulsanti.CUCINA)    
			if cucina.toggleH(val) == True:				
				digitalWrite(Lampade.CUCINA, cucina.stato*1023)
				
			#polling pulsante soggiorno
			val = digitalRead(Pulsanti.SOGGIORNO)    
			if soggiorno.toggleH(val) == True:				
				digitalWrite(Lampade.SOGGIORNO, soggiorno.stato*1023)
					
if __name__ == "__main__":
	main()
```
>[Torna all'indice](indexpulsanti.md) >[versione in C++](gruppipulsanti.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbMTc1MzU3MTYzXX0=
-->
