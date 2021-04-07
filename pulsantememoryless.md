---


---

<h2 id="pulsante-semplice"><strong>PULSANTE SEMPLICE</strong></h2>
<p>Si vuole accendere un led alla pressione del pulsante e spegnerlo al suo rilascio.</p>
<p>La logica di comando consiste nel <strong>selezionare il livello</strong> e generare l’uscita corrispondente.</p>
<p>L’operazione non ha particolari controindicazioni nell’essere eseguita periodicamente alla <strong>massima velocità</strong> consentita dalla CPU, almeno per un programma con un numero limitato di compiti da eseguire (task). Motivo per cui le fasi di azionamento vengono gestite direttamente dentro il loop() senza alcun filtro.</p>
<p>Se il numero di compiti è grande, in relazione alla limitata potenza di calcolo della CPU, allora è opportuno tenere conto del fatto che le operazioni di lettura e scrittura delle periferiche (in questo caso due porte digitali) è sempre abbastanza più lenta di analoghe operazioni su dati in memoria (almeno un ordine di grandezza). In questo caso una <strong>schedulazione</strong> delle operazioni lente per dare spazio all’esecuzione di quelle più veloci potrebbe essere una soluzione opportuna.</p>
<pre><code>int led=13;
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
</code></pre>

