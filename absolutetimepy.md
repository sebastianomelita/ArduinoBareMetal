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
