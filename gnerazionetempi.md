---


---

<h2 id="delay-e-millis"><strong>DELAY() E MILLIS()</strong></h2>
<p><strong>Delay()</strong></p>
<p>Mette in pausa il programma per la quantità di tempo (in millisecondi) specificata come parametro. (Ci sono 1000 millisecondi in un secondo).</p>
<p><strong>Millis()</strong></p>
<p>Restituisce il numero di millisecondi trascorsi da quando la scheda Arduino ha iniziato a eseguire il programma corrente. Questo numero supererà (tornerà a zero), dopo circa 50 giorni (overflow).</p>
<p>Il tipo di dato ritornato dalla funzione è unsigned long.</p>
<p>Bisogna prestare attenzione alle <strong>operazioni aritmetiche</strong> con <strong>espressioni</strong> che comprendono, oltre a millis() anche altre variabili aventi tipi più piccoli, come ad esempio int. Infatti la capacità di conteggio di variabili di dimensione inferiore ad unsigned long è minore di quella di millis() per cui potrebbero andare in overflow in un momento differente rispetto a millis() generando risultati scorretti.</p>
<p>Le misure di tempo devono sempre tenere conto del problema dell’overflow.  Sulle misure di tempo assolute (misurate a partire dall’accensione della macchina) si può fare poco, quelle andranno in overflow comunque, su quelle relative tra istanti campionati (e conservati su una variabile) e quelli misurati con la millis(), è possibile renderlo non dannoso. Il segreto è lavorare sempre <strong>su differenze di tempo</strong> che non superano mai il valore di overflow, ciò si ottiene assicurandosi di <strong>ricampionare il valore del riferimento</strong> a partire da cui si misura la differenza prima che ciò accada. Se si considera la differenza:</p>
<pre><code>millis()-precm
</code></pre>
<p>se precm è dello stesso tipo di millis(), cioè è di tipo unsigned long, allora sia l’una che l’altra andranno in overflow e in istanti di tempo diversi. Per effetto di un evento di overflow i valori assoluti delle variabili perderanno di significato mentre quelli relativi continueranno a mantenerlo infatti, nonostante le due variabili vadano in overflow in momenti diversi, la differenza relativa si mantiene ugualmente corretta. Ciò accade perché l’operazione di sottrazione delle variabili senza segno nei computer avviene modulo il valore massimo di conteggio della variabile.</p>
<p>Infatti si può verificare che il programma seguente restituisce il valore 2.</p>
<pre><code>void setup() {
	Serial.begin(115200);
	unsigned long a = 1;
	unsigned long b = 4294967295; //unsigned long maximum value
	Serial.println(a-b);
}

void loop() {
}
</code></pre>

