---


---

<h2 id="gestione-gruppi-di-pulsanti"><strong>GESTIONE GRUPPI DI PULSANTI</strong></h2>
<p>La <strong>logica di molti</strong> pulsanti potrebbe essere gestita sotto forma di <strong>dati strutturati</strong> (multivalore) in una delle seguenti maniere: array, struct ed oggetti.</p>
<p>In ciascuna delle precedenti strutture dati vanno memorizzate almeno <strong>due cose</strong>: lo <strong>stato</strong> del pulsante, la memoria del suo <strong>ingresso passato</strong>, cioè il valore dell’ingresso al loop precedente (serve per rilevare una transizione).</p>
<p>La <strong>differenza</strong> tra i vari modi di gestione consiste in come si memorizzano stati, ingressi e logica:</p>
<ul>
<li>
<p><strong>Array</strong>. Una <strong>sola funzione</strong> gestisce la logica di più pulsanti, <strong>stato</strong> e <strong>ingressi passati</strong> sono memorizzati sulle celle di <strong>due array paralleli</strong>. Gli array sono comuni a tutti i pulsanti, ma ogni <strong>indice</strong> di entrambi gli array si riferisce sempre ad un solo pulsante.</p>
</li>
<li>
<p><strong>Struct</strong>. Una <strong>sola funzione</strong> gestisce la logica di più pulsanti, <strong>stato</strong> e <strong>ingressi passati</strong> sono memorizzati su una struct <strong>separata</strong> per ogni pulsante. Al limite le struct possono essere organizzate in un array, soprattutto in presenza di molti tasti.</p>
</li>
<li>
<p><strong>Oggetti. Stato</strong> e <strong>ingressi passati</strong> sono memorizzati su un oggetto <strong>separato</strong> per ogni pulsante. Ogni oggetto possiede anche la funzione per manipolare i dati contenuti al suo interno (metodo). I metodi sono accessibili, a partire dalla variabile oggetto, usando la “dot” notation cioè, il nome della variabile, un punto, ed il nome del metodo. Anche in questo caso, volendo, per molti tasti gli oggetti potrebbero essere organizzati in un array o in una matrice.</p>
</li>
</ul>
<p><strong>Esempio di due pulsanti toggle gestiti con due array paralleli</strong></p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token comment">/*
Scrivere un programma Arduino che accenda due led (cucina, soggiorno).
Accenderli con due pulsanti toggle separati.
*/</span>
<span class="token macro property">#<span class="token directive keyword">define</span> TBASE 100</span>
<span class="token macro property">#<span class="token directive keyword">define</span> CMDSOGGIORNO 2</span>
<span class="token macro property">#<span class="token directive keyword">define</span> CMDCUCINA 3</span>
<span class="token macro property">#<span class="token directive keyword">define</span> LEDSOGGIORNO 12</span>
<span class="token macro property">#<span class="token directive keyword">define</span> LEDCUCINA 13</span>
<span class="token keyword">enum</span> btn
<span class="token punctuation">{</span>
	CUCINA  <span class="token operator">=</span><span class="token number">0</span><span class="token punctuation">,</span>
	SOGGIORNO  <span class="token operator">=</span><span class="token number">1</span>
<span class="token punctuation">}</span><span class="token punctuation">;</span>

byte precval<span class="token punctuation">[</span><span class="token number">2</span><span class="token punctuation">]</span><span class="token operator">=</span><span class="token punctuation">{</span><span class="token number">0</span><span class="token punctuation">,</span><span class="token number">0</span><span class="token punctuation">}</span><span class="token punctuation">;</span>
byte stato<span class="token punctuation">[</span><span class="token number">2</span><span class="token punctuation">]</span><span class="token operator">=</span><span class="token punctuation">{</span><span class="token number">0</span><span class="token punctuation">,</span><span class="token number">0</span><span class="token punctuation">}</span><span class="token punctuation">;</span>
<span class="token keyword">unsigned</span> <span class="token keyword">long</span> precm<span class="token operator">=</span><span class="token number">0</span><span class="token punctuation">;</span>

<span class="token keyword">void</span> <span class="token function">setup</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>CMDSOGGIORNO<span class="token punctuation">,</span> INPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>CMDCUCINA<span class="token punctuation">,</span> INPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>LEDSOGGIORNO<span class="token punctuation">,</span> OUTPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>LEDCUCINA<span class="token punctuation">,</span> OUTPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
<span class="token punctuation">}</span>

bool <span class="token function">transizione</span><span class="token punctuation">(</span>byte val<span class="token punctuation">,</span>byte n<span class="token punctuation">)</span><span class="token punctuation">{</span> <span class="token comment">//transizione di un pulsante</span>
	bool cambiato<span class="token operator">=</span>false<span class="token punctuation">;</span>
	cambiato <span class="token operator">=</span> <span class="token punctuation">(</span>precval<span class="token punctuation">[</span>n<span class="token punctuation">]</span> <span class="token operator">!=</span> val<span class="token punctuation">)</span><span class="token punctuation">;</span>
	precval<span class="token punctuation">[</span>n<span class="token punctuation">]</span> <span class="token operator">=</span> val<span class="token punctuation">;</span>
	<span class="token keyword">return</span> cambiato<span class="token punctuation">;</span>
<span class="token punctuation">}</span>

<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
	byte in<span class="token punctuation">;</span>

	<span class="token keyword">if</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token operator">-</span>precm<span class="token operator">&gt;=</span><span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span><span class="token punctuation">)</span>TBASE<span class="token punctuation">)</span><span class="token punctuation">{</span> <span class="token comment">//schedulatore e antirimbalzo</span>
		precm<span class="token operator">=</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token comment">//polling pulsante cucina</span>
		in<span class="token operator">=</span><span class="token function">digitalRead</span><span class="token punctuation">(</span>CMDCUCINA<span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token keyword">if</span><span class="token punctuation">(</span><span class="token function">transizione</span><span class="token punctuation">(</span>in<span class="token punctuation">,</span>CUCINA<span class="token punctuation">)</span><span class="token punctuation">)</span>				<span class="token punctuation">{</span>
			<span class="token keyword">if</span><span class="token punctuation">(</span>in<span class="token operator">==</span>HIGH<span class="token punctuation">)</span><span class="token punctuation">{</span> <span class="token comment">//se fronte di salita</span>
				stato<span class="token punctuation">[</span>CUCINA<span class="token punctuation">]</span> <span class="token operator">=</span> <span class="token operator">!</span>stato<span class="token punctuation">[</span>CUCINA<span class="token punctuation">]</span><span class="token punctuation">;</span>
				<span class="token function">digitalWrite</span><span class="token punctuation">(</span>LEDCUCINA<span class="token punctuation">,</span>stato<span class="token punctuation">[</span>CUCINA<span class="token punctuation">]</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
			<span class="token punctuation">}</span>
		<span class="token punctuation">}</span>
		<span class="token comment">//polling pulsante soggiorno</span>
		in<span class="token operator">=</span><span class="token function">digitalRead</span><span class="token punctuation">(</span>CMDSOGGIORNO<span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token keyword">if</span><span class="token punctuation">(</span><span class="token function">transizione</span><span class="token punctuation">(</span>in<span class="token punctuation">,</span>SOGGIORNO<span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
			<span class="token keyword">if</span><span class="token punctuation">(</span>in<span class="token operator">==</span>HIGH<span class="token punctuation">)</span><span class="token punctuation">{</span> <span class="token comment">//se fronte di salita</span>
				stato<span class="token punctuation">[</span>SOGGIORNO<span class="token punctuation">]</span> <span class="token operator">=</span> <span class="token operator">!</span>stato<span class="token punctuation">[</span>SOGGIORNO<span class="token punctuation">]</span><span class="token punctuation">;</span>
				<span class="token function">digitalWrite</span><span class="token punctuation">(</span>LEDSOGGIORNO<span class="token punctuation">,</span>stato<span class="token punctuation">[</span>SOGGIORNO<span class="token punctuation">]</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
			<span class="token punctuation">}</span>
		<span class="token punctuation">}</span>
	<span class="token punctuation">}</span> <span class="token comment">//chiudi schedulatore</span>
<span class="token punctuation">}</span>
</code></pre>
<p>Si noti l’uso delle due <strong>costanti enum</strong> CUCINA e SOGGIORNO per gestire in maniera <strong>automatica</strong> gli <strong>indici</strong> dei due array paralleli</p>
<pre class=" language-c"><code class="prism ++ language-c">byte stato<span class="token punctuation">[</span><span class="token number">2</span><span class="token punctuation">]</span><span class="token punctuation">;</span>
byte precval<span class="token punctuation">[</span><span class="token number">2</span><span class="token punctuation">]</span><span class="token punctuation">;</span>
</code></pre>
<p>Ad esempio, il valori dello <strong>stato</strong> e dell’<strong>ingresso</strong> <strong>passato</strong> del pulsante CUCINA si trovano rispettivamente:</p>
<pre class=" language-c"><code class="prism ++ language-c">statoCucina <span class="token operator">=</span> stato<span class="token punctuation">[</span>CUCINA<span class="token punctuation">]</span><span class="token punctuation">;</span>
precvalCucina <span class="token operator">=</span> precval<span class="token punctuation">[</span>CUCINA<span class="token punctuation">]</span><span class="token punctuation">;</span>
</code></pre>
<p>Negli esempi successivi utilizzeremo una funzione di rilevazione delle transizioni diversa perché mirata a rilevare soltanto i fronti di salita valutando la condizione</p>
<pre class=" language-c"><code class="prism ++ language-c">precval  <span class="token operator">==</span> LOW  <span class="token operator">&amp;&amp;</span> val <span class="token operator">==</span> HIGH<span class="token punctuation">.</span>
</code></pre>
<p>In maniera analoga si possono creare rilevatori di fronti di discesa valutando la condizione</p>
<pre class=" language-c"><code class="prism ++ language-c">precval  <span class="token operator">==</span> HIGH  <span class="token operator">&amp;&amp;</span> val <span class="token operator">==</span> LOW<span class="token punctuation">.</span>
</code></pre>
<p>L’uso di queste funzioni permette di <strong>spostare</strong> la <strong>logica del comando</strong> toggle (stato = !stato dal loop alla funzione di gestione del tasto, semplificando il codice del loop.</p>
<p>Adesso però conviene <strong>cambiare il nome</strong> della funzione poiché non rappresenta più soltanto un generico rilevatore di transizioni ma anche la logica “toggle” valutata sul fronte di salita.</p>
<p>Volendo, una funzione analoga si poteva usare anche sopra, nel codice dei pulsanti con gli array.</p>
<p><strong>Esempio di due pulsanti toggle gestiti con due struct</strong></p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token comment">/*
Scrivere un programma Arduino che accenda due led (cucina, soggiorno).
Accenderli con due pulsanti toggle separati.
*/</span>
<span class="token macro property">#<span class="token directive keyword">define</span> TBASE 100</span>
<span class="token macro property">#<span class="token directive keyword">define</span> CMDSOGGIORNO 2</span>
<span class="token macro property">#<span class="token directive keyword">define</span> CMDCUCINA 3</span>
<span class="token macro property">#<span class="token directive keyword">define</span> LEDSOGGIORNO 12</span>
<span class="token macro property">#<span class="token directive keyword">define</span> LEDCUCINA 13</span>
<span class="token keyword">struct</span> Toggle
<span class="token punctuation">{</span>
	uint8_t precval<span class="token punctuation">;</span>
	uint8_t stato<span class="token punctuation">;</span>
<span class="token punctuation">}</span> cucina<span class="token punctuation">,</span> soggiorno<span class="token punctuation">;</span>

<span class="token keyword">unsigned</span> <span class="token keyword">long</span> precm<span class="token operator">=</span><span class="token number">0</span><span class="token punctuation">;</span>
<span class="token keyword">void</span> <span class="token function">setup</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>CMDSOGGIORNO<span class="token punctuation">,</span> INPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>CMDCUCINA<span class="token punctuation">,</span> INPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>LEDSOGGIORNO<span class="token punctuation">,</span> OUTPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>LEDCUCINA<span class="token punctuation">,</span> OUTPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
<span class="token punctuation">}</span>

bool <span class="token function">toggleH</span><span class="token punctuation">(</span>byte val<span class="token punctuation">,</span> <span class="token keyword">struct</span> Toggle <span class="token operator">&amp;</span>btn<span class="token punctuation">)</span> <span class="token punctuation">{</span> 		<span class="token comment">//transizione di un pulsante</span>
	bool cambiato <span class="token operator">=</span> false<span class="token punctuation">;</span>
	<span class="token keyword">if</span> <span class="token punctuation">(</span>btn<span class="token punctuation">.</span>precval  <span class="token operator">==</span> LOW  <span class="token operator">&amp;&amp;</span> val <span class="token operator">==</span> HIGH<span class="token punctuation">)</span><span class="token punctuation">{</span>
		cambiato <span class="token operator">=</span> true<span class="token punctuation">;</span>
		btn<span class="token punctuation">.</span>stato <span class="token operator">=</span> <span class="token operator">!</span>btn<span class="token punctuation">.</span>stato<span class="token punctuation">;</span>
	<span class="token punctuation">}</span>
	<span class="token keyword">return</span> cambiato<span class="token punctuation">;</span>
<span class="token punctuation">}</span>

<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
	byte in<span class="token punctuation">;</span>
	
	<span class="token keyword">if</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token operator">-</span>precm<span class="token operator">&gt;=</span><span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span><span class="token punctuation">)</span>TBASE<span class="token punctuation">)</span><span class="token punctuation">{</span> 	<span class="token comment">//schedulatore e antirimbalzo</span>
		precm<span class="token operator">=</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token comment">//polling pulsante cucina</span>
		in<span class="token operator">=</span><span class="token function">digitalRead</span><span class="token punctuation">(</span>CMDCUCINA<span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token keyword">if</span><span class="token punctuation">(</span><span class="token function">toggleH</span><span class="token punctuation">(</span>in<span class="token punctuation">,</span>cucina<span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
			<span class="token function">digitalWrite</span><span class="token punctuation">(</span>LEDCUCINA<span class="token punctuation">,</span> cucina<span class="token punctuation">.</span>stato<span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token punctuation">}</span>
		<span class="token comment">//polling pulsante soggiorno</span>
		in<span class="token operator">=</span><span class="token function">digitalRead</span><span class="token punctuation">(</span>CMDSOGGIORNO<span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token keyword">if</span><span class="token punctuation">(</span><span class="token function">toggleH</span><span class="token punctuation">(</span>in<span class="token punctuation">,</span>soggiorno<span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
			<span class="token function">digitalWrite</span><span class="token punctuation">(</span>LEDSOGGIORNO<span class="token punctuation">,</span> soggiorno<span class="token punctuation">.</span>stato<span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token punctuation">}</span>
	<span class="token punctuation">}</span> <span class="token comment">//chiudi schedulatore</span>
<span class="token punctuation">}</span>
</code></pre>
<p><strong>Esempio di due pulsanti toggle gestiti con due oggetti</strong></p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token comment">/*
Scrivere un programma Arduino che accenda due led (cucina, soggiorno).
Accenderli con due pulsanti toggle separati.
*/</span>
<span class="token macro property">#<span class="token directive keyword">define</span> TBASE 100</span>
<span class="token macro property">#<span class="token directive keyword">define</span> CMDSOGGIORNO 2</span>
<span class="token macro property">#<span class="token directive keyword">define</span> CMDCUCINA 3</span>
<span class="token macro property">#<span class="token directive keyword">define</span> LEDSOGGIORNO 12</span>
<span class="token macro property">#<span class="token directive keyword">define</span> LEDCUCINA 13</span>

class Toggle
<span class="token punctuation">{</span>
	private<span class="token punctuation">:</span>
	uint8_t precval<span class="token punctuation">;</span>
	uint8_t stato<span class="token punctuation">;</span>

	public<span class="token punctuation">:</span>
	<span class="token function">Toggle</span><span class="token punctuation">(</span>uint8_t new_precval <span class="token operator">=</span> <span class="token number">0</span><span class="token punctuation">,</span> uint8_t new_stato <span class="token operator">=</span> <span class="token number">0</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
		precval <span class="token operator">=</span> new_precval<span class="token punctuation">;</span>
		stato <span class="token operator">=</span> new_stato<span class="token punctuation">;</span>
	<span class="token punctuation">}</span>

	bool <span class="token function">toggleH</span><span class="token punctuation">(</span>uint8_t val<span class="token punctuation">)</span><span class="token punctuation">{</span> <span class="token comment">//transizione di un pulsante</span>
		bool cambiato <span class="token operator">=</span> false<span class="token punctuation">;</span>
		<span class="token keyword">if</span> <span class="token punctuation">(</span>precval  <span class="token operator">==</span> LOW  <span class="token operator">&amp;&amp;</span> val <span class="token operator">==</span> HIGH<span class="token punctuation">)</span><span class="token punctuation">{</span>
			cambiato <span class="token operator">=</span> true<span class="token punctuation">;</span>
			stato <span class="token operator">=</span> <span class="token operator">!</span>stato<span class="token punctuation">;</span>
		<span class="token punctuation">}</span>
		precval <span class="token operator">=</span> val<span class="token punctuation">;</span>
		<span class="token keyword">return</span> cambiato<span class="token punctuation">;</span>
	<span class="token punctuation">}</span>

	uint8_t <span class="token function">getPrecval</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
		<span class="token keyword">return</span> precval<span class="token punctuation">;</span>
	<span class="token punctuation">}</span>

	<span class="token keyword">void</span> <span class="token function">setPrecval</span><span class="token punctuation">(</span>uint8_t new_precval<span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
		precval <span class="token operator">=</span> new_precval<span class="token punctuation">;</span>
	<span class="token punctuation">}</span>
	
	uint8_t <span class="token function">getStato</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
		<span class="token keyword">return</span> stato<span class="token punctuation">;</span>
	<span class="token punctuation">}</span>

	<span class="token keyword">void</span> <span class="token function">setStato</span><span class="token punctuation">(</span>uint8_t new_stato<span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
		stato <span class="token operator">=</span> new_stato<span class="token punctuation">;</span>
	<span class="token punctuation">}</span>
<span class="token punctuation">}</span><span class="token punctuation">;</span>

Toggle <span class="token function">cucina</span><span class="token punctuation">(</span><span class="token number">0</span><span class="token punctuation">,</span><span class="token number">0</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
Toggle <span class="token function">soggiorno</span><span class="token punctuation">(</span><span class="token number">0</span><span class="token punctuation">,</span><span class="token number">0</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
<span class="token keyword">unsigned</span> <span class="token keyword">long</span> precm<span class="token operator">=</span><span class="token number">0</span><span class="token punctuation">;</span>

<span class="token keyword">void</span> <span class="token function">setup</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>CMDSOGGIORNO<span class="token punctuation">,</span> INPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>CMDCUCINA<span class="token punctuation">,</span> INPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>LEDSOGGIORNO<span class="token punctuation">,</span> OUTPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>LEDCUCINA<span class="token punctuation">,</span> OUTPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
<span class="token punctuation">}</span>

<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
	uint8_t in<span class="token punctuation">;</span>
	<span class="token keyword">if</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token operator">-</span>precm<span class="token operator">&gt;=</span><span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span><span class="token punctuation">)</span>TBASE<span class="token punctuation">)</span><span class="token punctuation">{</span> 	<span class="token comment">//schedulatore e antirimbalzo</span>
		precm<span class="token operator">=</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token comment">//polling pulsante cucina</span>
		in<span class="token operator">=</span><span class="token function">digitalRead</span><span class="token punctuation">(</span>CMDCUCINA<span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token keyword">if</span><span class="token punctuation">(</span>cucina<span class="token punctuation">.</span><span class="token function">toggleH</span><span class="token punctuation">(</span>in<span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
			<span class="token function">digitalWrite</span><span class="token punctuation">(</span>LEDCUCINA<span class="token punctuation">,</span> cucina<span class="token punctuation">.</span><span class="token function">getStato</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token punctuation">}</span>
		<span class="token comment">//polling pulsante soggiorno</span>
		in<span class="token operator">=</span><span class="token function">digitalRead</span><span class="token punctuation">(</span>CMDSOGGIORNO<span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token keyword">if</span><span class="token punctuation">(</span>soggiorno<span class="token punctuation">.</span><span class="token function">toggleH</span><span class="token punctuation">(</span>in<span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
			<span class="token function">digitalWrite</span><span class="token punctuation">(</span>LEDSOGGIORNO<span class="token punctuation">,</span> 			soggiorno<span class="token punctuation">.</span><span class="token function">getStato</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token punctuation">}</span>
	<span class="token punctuation">}</span> <span class="token comment">//chiudi schedulatore</span>
<span class="token punctuation">}</span>
</code></pre>

