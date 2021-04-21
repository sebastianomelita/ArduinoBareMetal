>[Torna all'indice](indextimers.md)  >[versione in C++](timersschedulati.md)
## **POLLED TIMERS SCHEDULATI**

Si possono realizzare timer anche a partire dalla base dei tempi misurata da uno schedulatore semplicemente contando i passi raggiunti finchè questi non arrivano ad un valore target, prima del quale, o dopo il quale, far accadere qualcosa.
Il codice di seguito accende un led alla pressione di un pulsante solo se questo è stato premuto per un tempo minimo, oppure lo spegne solo se questo è rilasciato per almeno un tempo minimo, in tutti gli altri casi non fa nulla (filtra i comandi)
```Python
from gpio import *
from time import *

def main():
			
	NSTEP = 1000
	lastTime = 0  
	timerDelay = 2
	txtime = 0
	btntime = 0
	step = 0  
	start = False
	cmdin = 0
	led = 2
	pinMode(cmdin, IN)
	pinMode(led, OUT)
	precval = [0]
	
	def transizione(val, n): 
		cambiato = False
		cambiato = (precval[n] != val)
		precval[n] = val
		return cambiato

	while True:
		if (uptime() - lastTime) > timerDelay:
			lastTime = uptime()
			step = (step + 1) % NSTEP
			btntime = (btntime + 1) % NSTEP
		
			val = digitalRead(cmdin)
		
			if transizione(val,0) == True: 	# rivelatore di fronte (salita e discesa)
				print("Ho una transizione dell'ingresso")
				if start == True:
					start = False
					print("Ho filtrato un comando")
				else:
					start = True
					print("Ho abilitato un comando")
				btntime = 0
		
		# se premo il pulsante sufficientemente a lungo accendo il led
		# ma se lo rilascio sufficientemente a lungo lo spengo
		if start == True and (btntime >= txtime):
			digitalWrite(led, val*1023)
			start = False

if __name__ == "__main__":
	main()

```
>[Torna all'indice](indextimers.md) >[versione in C++](timersschedulati.md)
