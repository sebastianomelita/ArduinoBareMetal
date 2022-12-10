

>[Torna all'indice](indexpulsanti.md)   >[versione in Python](pulsantememorylesspy.md)
## **PULSANTE SEMPLICE**

Si vuole accendere un led alla pressione del pulsante e spegnerlo al suo rilascio.

La **logica di comando** consiste nel fare il **polling** di una porta di ingresso, nel **selezionare il livello** letto e nel generare (**scrivere**) l’uscita corrispondente.

L’operazione non ha particolari controindicazioni nell’essere eseguita periodicamente alla **massima velocità** consentita dalla CPU, almeno per un programma con un numero limitato di compiti da eseguire (task). Motivo per cui le fasi di azionamento vengono gestite direttamente dentro il loop() senza alcun filtro.

Se il numero di compiti è grande, in relazione alla limitata potenza di calcolo della CPU, allora è opportuno tenere conto del fatto che le operazioni di lettura e scrittura delle periferiche (in questo caso due porte digitali) è sempre abbastanza più lenta di analoghe operazioni su dati in memoria (almeno un ordine di grandezza). In questo caso una **schedulazione** più rada delle operazioni lente per dare spazio all’esecuzione di quelle più veloci potrebbe essere una soluzione opportuna.

Simulazione su Arduino con Tinkercad: https://www.tinkercad.com/embed/idptZ93lZ92?editbtn=1

Simulazione su Esp32 con Wowki: https://wokwi.com/projects/348779417449792082

```C++	
	int led=13;
	int tasto=2;
	int in;
	
	void setup(){
		pinMode(led,OUTPUT);
		pinMode(tasto,INPUT);
		digitalWrite(led,LOW);
		digitalWrite(tasto,LOW);
	}

	void loop(){
		in=digitalRead(tasto); //lettura ingresso
		if(in==HIGH){ // logica di comando
			digitalWrite(led,HIGH);  //scrittura uscita
		}else{
			digitalWrite(led,LOW);  //scrittura uscita
		}
	}
```
>[Torna all'indice](indexpulsanti.md) >[versione in Python](pulsantememorylesspy.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTEzMzI0MzE3MDldfQ==
-->
