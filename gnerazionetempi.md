---


---

<h1 id="generazione-di-tempi"><strong>GENERAZIONE DI TEMPI</strong></h1>
<h2 id="delay-e-millis"><strong>DELAY() E MILLIS()</strong></h2>
<p><strong>Delay()</strong></p>
<p>Mette in pausa il programma per la quantità di tempo (in millisecondi) specificata come parametro. (Ci sono 1000 millisecondi in un secondo).</p>
<p><strong>Millis()</strong></p>
<p>Restituisce il numero di millisecondi trascorsi da quando la scheda Arduino ha iniziato a eseguire il programma corrente. Questo numero supererà (tornerà a zero), dopo circa 50 giorni (overflow).</p>
<p>Il tipo di dato ritornato dalla funzione è unsigned long.</p>
<p>Bisogna prestare attenzione alle <strong>operazioni aritmetiche</strong> con <strong>espressioni</strong> che comprendono, oltre a millis() anche altre variabili aventi tipi più piccoli, come ad esempio int. Infatti la capacità di conteggio di variabili di dimensione inferiore ad unsigned long è minore di quella di millis() per cui potrebbero andare in overflow in un momento differente rispetto a millis() generando risultati scorretti.</p>
<p>Le misure di tempo devono sempre tenere conto del problema dell’overflow.  Sulle misure di tempo assolute (misurate a partire dall’accensione della macchina) si può fare poco, quelle andranno in overflow comunque, su quelle relative tra istanti campionati (e conservati su una variabile) e quelli misurati con la millis(), è possibile renderlo non dannoso. Il segreto è lavorare sempre <strong>su differenze di tempo</strong> che non superano mai il valore di overflow, ciò si ottiene assicurandosi di <strong>ricampionare il valore del riferimento</strong> a partire da cui si misura la differenza prima che ciò accada. Se si considera la differenza:</p>
<pre class=" language-c"><code class="prism ++ language-c">	<span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token operator">-</span>precm
</code></pre>
<p>se precm è dello stesso tipo di millis(), cioè è di tipo unsigned long, allora sia l’una che l’altra andranno in overflow e in istanti di tempo diversi. Per effetto di un evento di overflow i valori assoluti delle variabili perderanno di significato mentre quelli relativi continueranno a mantenerlo infatti, nonostante le due variabili vadano in overflow in momenti diversi, la differenza relativa si mantiene ugualmente corretta. Ciò accade perché l’operazione di sottrazione delle variabili senza segno nei computer avviene modulo il valore massimo di conteggio della variabile.</p>
<p>Infatti si può verificare che il programma seguente restituisce il valore 2.</p>
<pre class=" language-c"><code class="prism ++ language-c">	<span class="token keyword">void</span> <span class="token function">setup</span><span class="token punctuation">(</span><span class="token punctuation">)</span> <span class="token punctuation">{</span>
		Serial<span class="token punctuation">.</span><span class="token function">begin</span><span class="token punctuation">(</span><span class="token number">115200</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token keyword">unsigned</span> <span class="token keyword">long</span> a <span class="token operator">=</span> <span class="token number">1</span><span class="token punctuation">;</span>
		<span class="token keyword">unsigned</span> <span class="token keyword">long</span> b <span class="token operator">=</span> <span class="token number">4294967295</span><span class="token punctuation">;</span> <span class="token comment">//unsigned long maximum value</span>
		Serial<span class="token punctuation">.</span><span class="token function">println</span><span class="token punctuation">(</span>a<span class="token operator">-</span>b<span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token punctuation">}</span>

	<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span> <span class="token punctuation">{</span>
	<span class="token punctuation">}</span>
</code></pre>
<h2 id="esempi-di-schedulatori"><strong>ESEMPI DI SCHEDULATORI</strong></h2>
<p><strong>Schedulatore con delay().</strong></p>
<pre class=" language-c"><code class="prism ++ language-c">	byte led <span class="token operator">=</span> <span class="token number">13</span><span class="token punctuation">;</span>
	byte pulsante <span class="token operator">=</span> <span class="token number">2</span><span class="token punctuation">;</span>
	<span class="token keyword">void</span> <span class="token function">setup</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
		<span class="token function">pinMode</span><span class="token punctuation">(</span>led<span class="token punctuation">,</span> OUTPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token function">pinMode</span><span class="token punctuation">(</span>pulsante<span class="token punctuation">,</span> INPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token punctuation">}</span>

	<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
		<span class="token comment">//codice eseguito al tempo stabilito</span>
		in <span class="token operator">=</span> <span class="token function">digitalRead</span><span class="token punctuation">(</span>pulsante<span class="token punctuation">)</span><span class="token punctuation">;</span>  <span class="token comment">//pulsante collegato in pulldown</span>
		<span class="token comment">// Inserire qui la logica di comando</span>
		<span class="token comment">//……………………………………</span>
		<span class="token function">delay</span><span class="token punctuation">(</span><span class="token number">100</span><span class="token punctuation">)</span><span class="token punctuation">;</span>  <span class="token comment">// ritarda del tempo necessario</span>
	<span class="token punctuation">}</span>
</code></pre>
<p><strong>Schedulatore con millis().</strong></p>
<pre class=" language-c"><code class="prism ++ language-c">	<span class="token macro property">#<span class="token directive keyword">define</span> TBASE 100  </span><span class="token comment">// periodo base in millisecondi</span>
	byte in<span class="token punctuation">;</span>
	byte pulsante <span class="token operator">=</span> <span class="token number">2</span><span class="token punctuation">;</span>
	<span class="token keyword">unsigned</span> <span class="token keyword">long</span> precm<span class="token operator">=</span><span class="token number">0</span><span class="token punctuation">;</span>
	<span class="token keyword">void</span> <span class="token function">setup</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
		<span class="token function">pinMode</span><span class="token punctuation">(</span>pulsante<span class="token punctuation">,</span> INPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token punctuation">}</span>

	<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
		<span class="token comment">// polling di millis()</span>
		<span class="token keyword">if</span><span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token operator">-</span>precm<span class="token punctuation">)</span> <span class="token operator">&gt;=</span> tbase<span class="token punctuation">)</span><span class="token punctuation">{</span>  <span class="token comment">// ricerca del nuovo tempo “buono per eseguire”</span>
			<span class="token comment">// codice eseguito al tempo “buono per eseguire” attuale</span>
			precm <span class="token operator">=</span> <span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>  <span class="token comment">// memorizzo l’istante dell’ultimo tempo “buono per eseguire”</span>
			in <span class="token operator">=</span> <span class="token function">digitalRead</span><span class="token punctuation">(</span>pulsante<span class="token punctuation">)</span><span class="token punctuation">;</span>  <span class="token comment">// lettura ingresso</span>
			<span class="token comment">// Inserire qui la logica di comando</span>
			<span class="token comment">//……………………………………</span>
		<span class="token punctuation">}</span>
	<span class="token punctuation">}</span>
</code></pre>
<p><strong><em>tbase</em></strong> rappresenta la <strong>distanza</strong> del <strong>prossimo</strong> tempo “buono” dall’<strong>ultimo</strong> valutato. Ogni <strong>istante stabilito</strong> viene misurato a <strong>partire dall’ultimo</strong> calcolato. Si determina il <strong>tempo attuale</strong> di esecuzione aggiungendo un tbase all’<strong>ultimo tempo buono calcolato</strong> e preventivamente <strong>memorizzato</strong> in precm. Facciamo una simulazione.</p>
<p>Se <strong>tbase</strong> vale 1000 msec e <strong>precm</strong> vale 0, accade che per 0, 1, 2,…,999 msec l’if <strong>non scatta</strong> perché la condizione è falsa <strong>poi</strong>, dopo, al millis che restituisce il valore 1000, <strong>scatta</strong> e si esegue <strong>il compito schedulato</strong>. In definitiva l’if <strong>ignora</strong> 999 chiamate loop() mentre <strong>agisce</strong> alla millesima che <strong>capita</strong> esattamente dopo un secondo. Dopo questo momento <strong>precm</strong> vale adesso 1000, millis(), ad ogni loop(), vale, ad es., 1001, 1002,…,1999 msec, l’if <strong>non scatta</strong> perché la condizione è falsa <strong>poi</strong>, dopo, al millis che restituisce il valore 2000, <strong>scatta,</strong> si <strong>aggiorna</strong> nuovamente <strong>precm</strong> al valore attuale di millis(), cioè 2000, e si <strong>esegue</strong> nuovamente il <strong>compito schedulato</strong>. In maniera analoga si preparano gli <strong>scatti successivi</strong>.</p>
<h2 id="schedulatore-compiti"><strong>SCHEDULATORE COMPITI</strong></h2>
<p>Di seguito è riportato un esempio di schedulatore che pianifica nel tempo <strong>l’esecuzione periodica</strong> di una serie di <strong>compiti</strong> (task) da eseguire con <strong>cadenza diversa</strong>.</p>
<p>E’ buona norma evitare l’esecuzione frequente di operazioni lente quando queste non sono strettamente necessarie in modo da lasciare spazio ad altre operazioni, quali ad esempio gestione degli eventi di input, che richiedono una velocità maggiore per usufruirne in modo più interattivo.</p>
<p>Il <strong>tempo base</strong> è la base dei tempi di tutte le schedulazioni.</p>
<p>Le varie schedulazioni <strong>sono calcolate</strong> a partire da un <strong>multiplo intero</strong> del tempo base, ne segue che il tempo base dovrebbe essere calcolato come il massimo comune divisore (<strong>MCD</strong>) di tutti i tempi che devono essere generati.</p>
<p>Il conteggio dei multipli del tempo base è tenuto da un <strong>contatore circolare</strong> (step) che deve essere <strong>ruotato</strong> dopo aver effettuato un numero di conteggi superiori al <strong>massimo dei multipli</strong> del tempo base necessari.</p>
<p>Se ci sono <strong>pulsanti</strong> da gestire insieme ad altri task il tempo base può essere impostato tra 50 e 200 mSec in maniera da poterlo utilizzare per effettuare un <strong>polling degli ingressi</strong> immune dal fenomeno dei rimbalzi (<strong>antibounce SW</strong>).</p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token macro property">#<span class="token directive keyword">define</span> tbase  1000  </span><span class="token comment">// periodo base in milliseconds</span>
<span class="token macro property">#<span class="token directive keyword">define</span> nstep  1000  </span><span class="token comment">// numero di fasi massimo di un periodo generico</span>
<span class="token keyword">unsigned</span> <span class="token keyword">long</span> precm <span class="token operator">=</span> <span class="token number">0</span><span class="token punctuation">;</span>
<span class="token keyword">unsigned</span> <span class="token keyword">long</span> step <span class="token operator">=</span> <span class="token number">0</span><span class="token punctuation">;</span>
byte pari<span class="token punctuation">,</span> in<span class="token punctuation">;</span>
byte led1 <span class="token operator">=</span> <span class="token number">13</span><span class="token punctuation">;</span>
byte led2 <span class="token operator">=</span> <span class="token number">12</span><span class="token punctuation">;</span>

<span class="token keyword">void</span> <span class="token function">setup</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>led<span class="token punctuation">,</span> OUTPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
<span class="token punctuation">}</span>

<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	<span class="token comment">// polling della millis() alla ricerca del tempo stabilito</span>
	<span class="token keyword">if</span><span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token operator">-</span>precm<span class="token punctuation">)</span> <span class="token operator">&gt;=</span> tbase<span class="token punctuation">)</span><span class="token punctuation">{</span> <span class="token comment">// lo eseguo se è il tempo stabilito</span>
		precm <span class="token operator">=</span> <span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>  <span class="token comment">// preparo il tic successivo al tempo stabilito</span>
		step <span class="token operator">=</span> <span class="token punctuation">(</span>step <span class="token operator">+</span> <span class="token number">1</span><span class="token punctuation">)</span> <span class="token operator">%</span> nstep<span class="token punctuation">;</span>  <span class="token comment">// conteggio circolare arriva al massimo a nstep-1</span>

		<span class="token comment">// task 1</span>
		<span class="token keyword">if</span><span class="token punctuation">(</span><span class="token operator">!</span><span class="token punctuation">(</span>step<span class="token operator">%</span><span class="token number">2</span><span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">{</span>  <span class="token comment">// schedulo eventi al multiplo del tempo stabilito (2 sec)</span>
			<span class="token function">digitalWrite</span><span class="token punctuation">(</span>led1<span class="token punctuation">,</span><span class="token operator">!</span><span class="token function">digitalRead</span><span class="token punctuation">(</span>led1<span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">;</span> <span class="token comment">// stato alto: led blink</span>
		<span class="token punctuation">}</span>
		<span class="token comment">// task 2</span>
		<span class="token keyword">if</span><span class="token punctuation">(</span><span class="token operator">!</span><span class="token punctuation">(</span>step<span class="token operator">%</span><span class="token number">3</span><span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">{</span>  <span class="token comment">// schedulo eventi al multiplo del tempo stabilito (3 sec)</span>
			<span class="token function">digitalWrite</span><span class="token punctuation">(</span>led2<span class="token punctuation">,</span><span class="token operator">!</span><span class="token function">digitalRead</span><span class="token punctuation">(</span>led2<span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">;</span> <span class="token comment">// stato alto: led blink</span>
		<span class="token punctuation">}</span>
		<span class="token comment">// il codice eseguito al tempo del metronomo va quì</span>
	<span class="token punctuation">}</span>
	<span class="token comment">// il codice eseguito al tempo massimo della CPU va qui</span>
<span class="token punctuation">}</span>
</code></pre>
<h2 id="misure-di-tempo-assolute"><strong>MISURE DI TEMPO ASSOLUTE</strong></h2>
<p>Per ottenere una cadenza periodica precisa è necessario usare una forma diversa dal solito schedulatore più adatta a cumulare con precisione lunghe misure di tempo. E’essenziale che l’accumulatore tass venga aggiornato esattamente con il tempo campionato. L’accumulatore unisce i vari campionamenti per ottenere una misura unica. Con questa forma ci si può aspettare un errore di qualche secondo all’ora dipendente solo dall’imprecisione dell’oscillatore.</p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token keyword">unsigned</span> <span class="token keyword">long</span> tass <span class="token operator">=</span> <span class="token number">0</span><span class="token punctuation">;</span>
<span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> <span class="token operator">-</span> tass<span class="token punctuation">)</span> <span class="token operator">&gt;=</span> periodo<span class="token punctuation">)</span>
<span class="token punctuation">{</span>
tass <span class="token operator">+</span><span class="token operator">=</span> periodo<span class="token punctuation">;</span>
<span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span>
<span class="token punctuation">}</span>
</code></pre>
<p>Invece la forma seguente è errata. La condizione viene valutata in ritardo rispetto al momento ideale, “reimpostando” la variabile ‘tass” al tempo attuale, questo ritardo si aggiunge a tutti i ritardi precedenti. Con questa forma ci si può aspettare un errore di diversi secondi al minuto o anche peggiore.</p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> <span class="token operator">-</span> tass<span class="token punctuation">)</span> <span class="token operator">&gt;=</span> periodo<span class="token punctuation">)</span>
<span class="token punctuation">{</span>
tass <span class="token operator">=</span> <span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
<span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span>
<span class="token punctuation">}</span>
</code></pre>

