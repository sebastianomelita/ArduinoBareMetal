>[Torna all'indice](indextimers.md) >[versione in C++](gruppitimers.md)
## **GRUPPI DI TIMERS**

La logica di molti pulsanti potrebbe essere gestita sotto forma di dati strutturati (multivalore) in una delle seguenti maniere: array, struct ed oggetti. 
In ciascuna delle precedenti strutture dati vanno memorizzate almeno 3 cose: lo **stato** del timer, la memoria del suo **istante iniziale**, la **durata** del conteggio del tempo prima del timeout
La differenza tra i vari modi di gestione consiste in come si memorizzano stati, ingressi e logica:
- **Array**. Una sola funzione gestisce la logica di più pulsanti, stato e ingressi passati sono memorizzati sulle celle di due array paralleli. Gli array sono comuni a tutti i timer, ma ogni indice di entrambi gli array si riferisce sempre ad un solo timer.
- **Struct**. Una sola funzione gestisce la logica di più timer, stato e ingressi passati sono memorizzati su una struct separata per ogni timer. Al limite le struct possono essere organizzate in un array, soprattutto in presenza di molti timer.
- **Oggetti**. Stato, istante iniziale, durata sono memorizzati su un oggetto separato per ogni timer. Ogni oggetto possiede anche la funzione per manipolare i dati contenuti al suo interno (metodo). I metodi sono accessibili, a partire dalla variabile oggetto, usando la “dot” notation cioè, il nome della variabile, un punto, ed il nome del metodo. Anche in questo caso, volendo, per molti tasti gli oggetti potrebbero essere organizzati in un array o in una matrice.



```Python
#
# 1) Scrivere un programma Arduino che accenda due led (ingresso, sala, scala). Accenderli con tre pulsanti toggle separati. Lo stato dei led deve essere scritto sulla seriale all'avvenire (occorrenza) di ogni comando.
# 3) Realizzare, con un timer, l funzione di spegnimento automatico, dopo 10 secondi, della luce della scala. (dispensa timer.doc)
# 4) Utilizzare un altro timer per gestire lo spegnimento delle luci delle due sale alla pressione prolungata di uno dei loro pulsanti.
# 5) Realizzare, con un timer, la funzione "lampada viva" che accende (o fa lampeggiare) due led di segnalazione, uno per ogni sala, se non riceve via seriale il il comando "sala":"alive" e "ingresso":"alive" entro 10 secondi. 
#   Se lo riceve non segnala, se non lo riceve segnala. Al massimo aspetta 10 secondi per segnalare.
#
from gpio import *
from time import *

def main():
	#inizio variabili timer
	startTime = 0  		# tempo in sec
	timelapse = 0
	timerState = False
	# variabili schedulatore
	TBASE = 0.1
	NSTEP = 100
	step = 0
	
	class Pulsanti:
		INGRESSO = 0
		SALA = 1
		SCALA = 2
		
	class Lampade:
		INGRESSO = 3
		SALA = 4
		SCALA = 5
		SPIA = 6
		
	class Times:
		SICUREZZA = 10
		SCALA = 10
		SPEGNI = 10
		
	class Timers:
		SCALA = 0
		SPEGNI = 1
		SICUREZZA = 2

	class Ambienti:
		SCALA = 0
		SALA = 1
		INGRESSO = 2
	
	# funzione di attivazione
	def startTimer(duration, n):
		timerState[n] = 1 
		timelapse[n] = duration
		startTime[n] = uptime()

	# funzione di disattivazione
	def stopTimer(n):
		timerState[n] = False
		
	# callback: azione standard da compiere allo scadere del timer, definita fuori dal loop
	def onElapse(n):
		# azione da compiere
		#.......

	# polling: verifica se è arrivato il tempo di far scattare il timer
	def aggiornaTimer(n):
		if (timerState[n] == True) and (uptime() - startTime[n] >= timelapse[n]):
			timerState[n] = False
			onElapse(n)
	
	# azione da compiere allo scadere del timer, definita fuori dal loop
	def onElapse(n):
		if n == Timers.SCALA:
			stato[Ambienti.SCALA] = LOW
			digitalWrite(Lampade.SCALA, stato[Ambienti.SCALA])
		else if n == Timers.SPEGNI:
			stato[Ambienti.SCALA] = LOW
			digitalWrite(Lampade.SALA, stato[Ambienti.SALA])
			stato[Ambienti.INGRESSO] = LOW
			digitalWrite(Lampade.INGRESSO, stato[Ambienti.INGRESSO])
		else if n == Timers.SICUREZZA:
			digitalWrite(Lampade.SPIA, HIGH)
    
	def transizione(val, n):  # transizione di un pulsante
		cambiato = False 
		cambiato = (precval[n] != val)
		precval[n] = val  
		return cambiato
		
	precval = [0, 0, 0]
	stato = [0, 0, 0]
	precm = 0
	startTime = [0, 0, 0]
	timelapse = [0, 0, 0]
	timerState = [false, false, false]
	pinMode(Pulsanti.SCALA, IN)
	pinMode(Pulsanti.SALA, IN)
	pinMode(Pulsanti.INGRESSO, IN)
	pinMode(Lampade.INGRESSO, OUT)
	pinMode(Lampade.SALA, OUT)
	pinMode(Lampade.SCALA, OUT)
	pinMode(Lampade.SPIA, OUT)
	stopTimer(Timers.SCALA)
	stopTimer(Timers.SPEGNI)
	stopTimer(Timers.SICUREZZA)
	usb = USB(0, 9600)
	
	while True:
		# polling dei tempi
		aggiornaTimer(Timers.SCALA)
		aggiornaTimer(Timers.SPEGNI)
		aggiornaTimer(Timers.SICUREZZA)
			
		if (uptime() - precm) >= TBASE:  	   	# schedulatore (e anche antirimbalzo)
			precm = uptime()  			   		# preparo il tic successivo	
			step = (step+1) % NSTEP				# conteggio circolare
			
			# polling pulsante SCALA
			val = digitalRead(Pulsanti.SCALA)
			if transizione(val, Ambienti.SCALA) == True:
				if in == HIGH: # se fronte di salita (pressione)
					startTimer(Times.SCALA, Timers.SCALA)
					stato[Ambienti.SCALA] = (stato[Ambienti.SCALA] + 1) % 2;
					digitalWrite(Lampade.SCALA, stato[Ambienti.SCALA]*1023);
			
			# polling pulsante SALA
			val = digitalRead(Pulsanti.SALA)
			if transizione(val, Ambienti.SALA) == True:
				if in == HIGH: # se fronte di salita (pressione)
					startTimer(Times.SPEGNI, Timers.SPEGNI)
					stato[Ambienti.SALA] = (stato[Ambienti.SALA] + 1) % 2
					digitalWrite(Lampade.SALA, stato[Ambienti.SALA]*1023)
				else: # rilascio
					stopTimer(Timers.TMRSPEGNI)
	
			# polling pulsante INGRESSO
			val = digitalRead(Pulsanti.INGRESSO)
			if transizione(val, Ambienti.INGRESSO) == True:
				if in == HIGH: # se fronte di salita (pressione)
					startTimer(Times.SPEGNI, Timers.SPEGNI)
					stato[Ambienti.INGRESSO] = (stato[Ambienti.INGRESSO] + 1) % 2
					digitalWrite(Lampade.INGRESSO, stato[Ambienti.INGRESSO]*1023)
				else: # rilascio
					stopTimer(Timers.TMRSPEGNI)
					
		if usb.inWaiting() > 0: # anche while va bene!			
			instr = usb.readLine()
			if instr.index("\"statosala\":\"on\"") >= 0:
				startTimer(Times.SICUREZZA, Timers.SICUREZZA);
				digitalWrite(Lampade.SPIA, LOW);

if __name__ == "__main__":
	main()
```
>[Torna all'indice](indextimers.md) >[versione in C++](gruppitimers.md)
