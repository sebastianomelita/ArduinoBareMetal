

>[Torna all'indice](indexpulsanti.md)   >[versione in C++](pulsantememoryless.md)
>
## **PULSANTE SEMPLICE**

Si vuole accendere un led alla pressione del pulsante e spegnerlo al suo rilascio.

La logica di comando consiste nel **selezionare il livello** e generare l’uscita corrispondente.

L’operazione non ha particolari controindicazioni nell’essere eseguita periodicamente alla **massima velocità** consentita dalla CPU, almeno per un programma con un numero limitato di compiti da eseguire (task). Motivo per cui le fasi di azionamento vengono gestite direttamente dentro il loop() senza alcun filtro.

Se il numero di compiti è grande, in relazione alla limitata potenza di calcolo della CPU, allora è opportuno tenere conto del fatto che le operazioni di lettura e scrittura delle periferiche (in questo caso due porte digitali) è sempre abbastanza più lenta di analoghe operazioni su dati in memoria (almeno un ordine di grandezza). In questo caso una **schedulazione** delle operazioni lente per dare spazio all’esecuzione di quelle più veloci potrebbe essere una soluzione opportuna.
```Python	
from gpio import *
from time import *

def main():
	pulsante = 0
	led = 1
	pinMode(pulsante, IN)

	while True:
		val = digitalRead(pulsante)    			# lettura ingressi
		if val == HIGH:      				# rivelatore di fronte di salita
			digitalWrite(led, HIGH);   		# scrittura uscite
		else:
			digitalWrite(led, LOW);  		# scrittura uscite
			
if __name__ == "__main__":
	main()
	
```
>[Torna all'indice](indexpulsanti.md) >[versione in C++](pulsantememoryless.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTEzMzI0MzE3MDldfQ==
-->
