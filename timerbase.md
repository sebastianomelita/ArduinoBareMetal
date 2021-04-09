---


---

<h2 id="polled-timers"><strong>POLLED TIMERS</strong></h2>
<p>E’ possibile realizzare dei timers ottenuti eseguendo, ad ogni loop(), il <strong>polling della funzione millis()</strong> per verificare che questa abbia raggiunto il <strong>valore finale</strong> stabilito come target (obiettivo), quando ciò accade, possiamo fare eseguire il blocco di istruzioni <strong>associato</strong> a questo evento.</p>
<p>I timers possono essere <strong>periodici</strong>, ed uno di questi era proprio lo schedulatore adoperato per generare la base degli eventi di sistema, oppure possono essere <strong>aperiodici</strong> (detti anche monostabili), cioè devono essere attivati, con una istruzione apposita, solamente quando serve.</p>
<p><strong>In</strong> <strong>generale</strong>, possiamo individuare alcune <strong>componenti del timer</strong>:</p>
<ul>
<li>
<p><strong>reset</strong> del timer. Accade al verificarsi di una certa <strong>condizione</strong>.</p>
</li>
<li>
<p><strong>polling</strong> del tempo trascorso. Viene fatto ad ogni ciclo di <strong>loop</strong> (talvolta meno).</p>
</li>
<li>
<p><strong>istruzioni triggerate</strong> (scatenate) dal timer. Vengono eseguite (o non eseguite) in base al <strong>tempo</strong> <strong>misurato</strong> dal timer</p>
</li>
<li>
<p>(opzionale) la <strong>variabile stato</strong> del timer che <strong>abilita</strong> il timer quando serve e lo <strong>disabilita</strong> quando non serve per evitare <strong>attivazioni spurie</strong> accidentali.</p>
</li>
</ul>
<p>Un esempio di <strong>timer periodico</strong> (del tutto analogo ad un o schedulatore) potrebbe apparire così:</p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token comment">//Timer periodico</span>
<span class="token macro property">#<span class="token directive keyword">define</span> PERIODO  1000</span>
<span class="token keyword">unsigned</span> <span class="token keyword">long</span> ptimer1<span class="token punctuation">;</span>
bool timerState <span class="token operator">=</span> false<span class="token punctuation">;</span> <span class="token comment">// stato del timer</span>

<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	<span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token punctuation">(</span>timerState<span class="token punctuation">)</span> <span class="token operator">&amp;&amp;</span> <span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> <span class="token operator">-</span> ptimer1<span class="token punctuation">)</span> <span class="token operator">&gt;=</span> <span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span><span class="token punctuation">)</span> PERIODO<span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
		ptimer1 <span class="token operator">=</span> <span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
		<span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span>
		<span class="token comment">// istruzioni eseguite periodicamente, se attivo…</span>
	<span class="token punctuation">}</span>
<span class="token punctuation">}</span>
</code></pre>
<p>Di seguito è un esempio di <strong>timer aperiodico</strong> che misura il tempo a partire dal verificarsi di una certa condizione fino a che, al superamento di un tempo limite, si decide di fare qualcosa ma solo <strong>dopo</strong> lo scadere del timer:</p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token comment">//Timer aperiodico 1</span>
<span class="token macro property">#<span class="token directive keyword">define</span> TLIMITE1  1000</span>
<span class="token keyword">unsigned</span> <span class="token keyword">long</span> atimer1<span class="token punctuation">;</span>
<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	<span class="token comment">// condizione che valuta l’accadere di un certo evento</span>
	<span class="token comment">// di attivazione del conteggio del tempo</span>
	<span class="token keyword">if</span><span class="token punctuation">(</span>condA<span class="token punctuation">)</span><span class="token punctuation">{</span>
		atimer1 <span class="token operator">=</span> <span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token punctuation">}</span>

	<span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> <span class="token operator">-</span> atimer1<span class="token punctuation">)</span> <span class="token operator">&gt;=</span> <span class="token punctuation">(</span><span class="token keyword">unsigned</span> 	<span class="token keyword">long</span><span class="token punctuation">)</span> TLIMITE1<span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
		<span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span>
		<span class="token comment">// istruzioni eseguite allo scadere del timer 1</span>
	<span class="token punctuation">}</span>
<span class="token punctuation">}</span>
</code></pre>
<p>Analogamente si potrebbe eseguire un blocco di codice continuamente <strong>solo</strong> all’interno di un tempo massimo prestabilito, cioè solo <strong>prima</strong> dello scadere del timer:</p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token comment">//Timer aperiodico 2</span>
<span class="token macro property">#<span class="token directive keyword">define</span> TLIMITE  1000</span>
<span class="token keyword">unsigned</span> <span class="token keyword">long</span> atimer2<span class="token punctuation">;</span>
<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	<span class="token comment">// condizione che valuta l’accadere di un certo evento</span>
	<span class="token comment">// di attivazione del conteggio del tempo</span>
	<span class="token keyword">if</span><span class="token punctuation">(</span>condA<span class="token punctuation">)</span><span class="token punctuation">{</span>
		atimer2 <span class="token operator">=</span> <span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token punctuation">}</span>

	<span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> – atimer2<span class="token punctuation">)</span> <span class="token operator">&lt;</span> <span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span><span class="token punctuation">)</span> TLIMITE2<span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
		<span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span>
		<span class="token comment">// istruzioni eseguite finchè NON scade il timer 2</span>
	<span class="token punctuation">}</span>
<span class="token punctuation">}</span>
</code></pre>
<p>I timers possono anche essere in funzione <strong>all’interno di funzioni</strong> che sono richiamate all’interno del loop():</p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	<span class="token function">poll</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>  <span class="token comment">// funzione con **blocchi** di codice eseguiti **prima** o **dopo** di certi eventi</span>
	……<span class="token punctuation">.</span>
<span class="token punctuation">}</span>

<span class="token keyword">void</span> <span class="token function">poll</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	<span class="token keyword">if</span><span class="token punctuation">(</span>condA<span class="token punctuation">)</span><span class="token punctuation">{</span> <span class="token comment">// evento che attiva il timer (può essere ricorrente ma, in generale, è aperiodico)</span>
		atimer1 <span class="token operator">=</span> <span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token punctuation">}</span>

	<span class="token comment">// finchè non si raggiunge TLIMITE1 ritorna senza fare nulla</span>
	<span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> – atime1<span class="token punctuation">)</span> <span class="token operator">&lt;</span> <span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span><span class="token punctuation">)</span> TLIMITE1<span class="token punctuation">)</span> <span class="token keyword">return</span><span class="token punctuation">;</span>
	<span class="token comment">//BLOCCO_A</span>

	<span class="token comment">// finchè non si raggiunge TLIMITE2 ritorna dopo avere eseguito il blocco di istruzioni A</span>
	<span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> – atimer2<span class="token punctuation">)</span> <span class="token operator">&lt;</span> <span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span><span class="token punctuation">)</span> TLIMITE2<span class="token punctuation">)</span> <span class="token keyword">return</span><span class="token punctuation">;</span>
	<span class="token comment">//BLOCCO_B</span>

	<span class="token comment">// finchè non si raggiunge TLIMITE3 ritorna dopo avere eseguito il blocco di istruzioni</span>

	<span class="token comment">// A e B</span>
	<span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> – atimer1<span class="token punctuation">)</span> <span class="token operator">&lt;</span> <span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span><span class="token punctuation">)</span> TLIMITE3<span class="token punctuation">)</span> <span class="token keyword">return</span><span class="token punctuation">;</span>
<span class="token punctuation">}</span>
</code></pre>
<p>Attenzione ad un <strong>errore insidioso</strong>:</p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token keyword">void</span> <span class="token function">poll</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	<span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> – atimer1<span class="token punctuation">)</span> <span class="token operator">&lt;</span> <span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span><span class="token punctuation">)</span> TLIMITE1<span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
	<span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span>
	<span class="token comment">// istruzioni eseguite finchè NON scade il timer 1</span>
	<span class="token punctuation">}</span>
	<span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> – atimer2<span class="token punctuation">)</span> <span class="token operator">&lt;</span> <span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span><span class="token punctuation">)</span> TLIMITE2<span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
	<span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span>
	<span class="token comment">// istruzioni eseguite finchè NON scade il timer 2</span>
	<span class="token punctuation">}</span>
<span class="token punctuation">}</span>
</code></pre>
<p>In questa situazione se scade il primo timer <strong>viene comunque controllato</strong> lo scadere del secondo. La cascata degli if equivale ad un OR logico sulle condizioni di scadenza.</p>
<p>Se voglio che <strong>ne accada solo una</strong> posso scrivere così:</p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token keyword">void</span> <span class="token function">poll</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	<span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> – atimer1<span class="token punctuation">)</span> <span class="token operator">&lt;</span> <span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span><span class="token punctuation">)</span> TLIMITE1<span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
		<span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span>
		<span class="token comment">// istruzioni eseguite finchè NON scade il timer 1</span>
	<span class="token punctuation">}</span><span class="token keyword">else</span> <span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> – atimer2<span class="token punctuation">)</span> <span class="token operator">&lt;</span> <span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span><span class="token punctuation">)</span> TLIMITE2<span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
		<span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span>
		<span class="token comment">// istruzioni eseguite finchè NON scade il timer 2</span>
	<span class="token punctuation">}</span>
<span class="token punctuation">}</span>
</code></pre>
<p>Dove <strong>l’ordine</strong> stabilisce la <strong>priorità</strong> di valutazione.</p>
<p>Alcuni codici, ritenendo un evento disabilitante per tutto il resto della funzione, talvolta fanno così:</p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token keyword">void</span> <span class="token function">poll</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	<span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> – atimer1<span class="token punctuation">)</span> <span class="token operator">&lt;</span> <span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span><span class="token punctuation">)</span> TLIMITE1<span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
		<span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span>
		<span class="token comment">// istruzioni eseguite finchè NON scade il timer 1</span>
		<span class="token keyword">return</span> <span class="token punctuation">;</span> <span class="token comment">// NON SONO PRONTO! RICONTROLLA AL PROSSIMO GIRO…</span>
	<span class="token punctuation">}</span>
	<span class="token comment">// ISTRUZIONI IMPORTANTI SI MA CHE…QUALCHE VOLTA…NON SI FANNO…</span>
<span class="token punctuation">}</span>
</code></pre>
<p><strong>Reset</strong> del timer, <strong>polling</strong> del tempo trascorso e <strong>istruzioni triggerate</strong> (scatenate) dal timer potrebbero anche essere rinchiuse in altrettante <strong>funzioni</strong>:</p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token comment">//inizio variabili timer</span>
<span class="token keyword">unsigned</span> <span class="token keyword">long</span> startTime<span class="token punctuation">;</span>
<span class="token keyword">unsigned</span> <span class="token keyword">long</span> timelapse<span class="token punctuation">;</span>
byte timerState<span class="token operator">=</span><span class="token number">0</span><span class="token punctuation">;</span>
<span class="token comment">//fine variabili timer</span>

<span class="token keyword">void</span> <span class="token function">startTimer</span><span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span> duration<span class="token punctuation">)</span><span class="token punctuation">{</span>
	timerState<span class="token operator">=</span><span class="token number">1</span><span class="token punctuation">;</span>
	timelapse<span class="token operator">=</span>duration<span class="token punctuation">;</span>
	startTime<span class="token operator">=</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
<span class="token punctuation">}</span>

<span class="token keyword">void</span> <span class="token function">stopTimer</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
	timerState<span class="token operator">=</span><span class="token number">0</span><span class="token punctuation">;</span>
<span class="token punctuation">}</span>

<span class="token comment">//verifica se è arrivato il tempo di far scattare il timer</span>
<span class="token keyword">void</span> <span class="token function">aggiornaTimer</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
	<span class="token keyword">if</span><span class="token punctuation">(</span><span class="token punctuation">(</span>timerState <span class="token operator">==</span> <span class="token number">1</span><span class="token punctuation">)</span> <span class="token operator">&amp;&amp;</span> <span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> <span class="token operator">-</span> startTime <span class="token operator">&gt;=</span> timelapse<span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
		timerState<span class="token operator">=</span><span class="token number">0</span><span class="token punctuation">;</span>
		<span class="token function">onElapse</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token punctuation">}</span>
<span class="token punctuation">}</span>

<span class="token comment">// azione da compiere allo scadere del timer, definita fuori dal loop</span>
<span class="token keyword">void</span> <span class="token function">onElapse</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
	<span class="token comment">//azione da compiere</span>
	<span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span><span class="token punctuation">.</span>
<span class="token punctuation">}</span>

<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
	<span class="token function">aggiornaTimer</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>  <span class="token comment">//aggiorna il primo timer</span>
	<span class="token comment">//se accade qualcosa parte il timer</span>
	<span class="token keyword">if</span><span class="token punctuation">(</span>A<span class="token punctuation">)</span><span class="token punctuation">{</span>
		<span class="token function">startTimer</span><span class="token punctuation">(</span><span class="token number">1000</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token punctuation">}</span>

	<span class="token keyword">if</span><span class="token punctuation">(</span>B<span class="token punctuation">)</span><span class="token punctuation">{</span> <span class="token comment">//se accade qualcosa blocco il timer</span>
		<span class="token function">stopTimer</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token punctuation">}</span>
<span class="token punctuation">}</span>
</code></pre>
<p><strong>POLLED TIMERS SCHEDULATI</strong></p>
<p>Si possono realizzare timer <strong>anche</strong> a partire dalla <strong>base dei tempi</strong> misurata da uno <strong>schedulatore</strong> semplicemente <strong>contando i passi</strong> raggiunti finchè questi non arrivano ad un valore <strong>target</strong>, <strong>prima</strong> del quale, o <strong>dopo</strong> il quale, far <strong>accadere</strong> qualcosa.</p>
<p>Il codice di seguito fa partire un comando alla pressione di un pulsante solo se questo è stato <strong>premuto per un tempo minimo</strong>, in caso contrario non fa nulla (filtra i comandi)</p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token keyword">unsigned</span> <span class="token keyword">long</span> lastTime <span class="token operator">=</span> <span class="token number">0</span><span class="token punctuation">;</span>
<span class="token keyword">unsigned</span> <span class="token keyword">long</span> timerDelay <span class="token operator">=</span> TBASE<span class="token punctuation">;</span>  <span class="token comment">// send readings timer</span>
<span class="token keyword">unsigned</span> step <span class="token operator">=</span> <span class="token number">0</span><span class="token punctuation">;</span>
bool start<span class="token operator">=</span>false<span class="token punctuation">;</span>
<span class="token keyword">unsigned</span> <span class="token keyword">short</span>  val<span class="token punctuation">;</span>
byte precval<span class="token operator">=</span><span class="token number">0</span><span class="token punctuation">;</span> <span class="token comment">//switchdf e toggle</span>
byte cmdin<span class="token operator">=</span><span class="token number">2</span><span class="token punctuation">;</span>

<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span> <span class="token punctuation">{</span>
	<span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> <span class="token operator">-</span> lastTime<span class="token punctuation">)</span> <span class="token operator">&gt;</span> timerDelay<span class="token punctuation">)</span> <span class="token punctuation">{</span>
		lastTime <span class="token operator">=</span> <span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
		step <span class="token operator">=</span> <span class="token punctuation">(</span>step <span class="token operator">+</span> <span class="token number">1</span><span class="token punctuation">)</span> <span class="token operator">%</span> NSTEP<span class="token punctuation">;</span>
		btntime <span class="token operator">=</span> <span class="token punctuation">(</span>btntime <span class="token operator">+</span> <span class="token number">1</span><span class="token punctuation">)</span> <span class="token operator">%</span> NSTEP<span class="token punctuation">;</span>
		val <span class="token operator">=</span> <span class="token operator">!</span><span class="token function">digitalRead</span><span class="token punctuation">(</span>cmdin<span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">;</span> <span class="token comment">// pulsante pull up</span>

		<span class="token keyword">if</span><span class="token punctuation">(</span><span class="token function">switchdf</span><span class="token punctuation">(</span>val<span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">{</span> <span class="token comment">//rivelatore di fronte (salita e discesa)</span>
			Serial<span class="token punctuation">.</span><span class="token function">println</span><span class="token punctuation">(</span><span class="token string">"Ho una transizione dell'ingresso"</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
			<span class="token keyword">if</span><span class="token punctuation">(</span>val <span class="token operator">==</span> HIGH<span class="token punctuation">)</span><span class="token punctuation">{</span> <span class="token comment">// ho un fronte di salita</span>
				<span class="token keyword">if</span><span class="token punctuation">(</span>start<span class="token operator">==</span>true<span class="token punctuation">)</span><span class="token punctuation">{</span>
					start <span class="token operator">=</span> false<span class="token punctuation">;</span>
					Serial<span class="token punctuation">.</span><span class="token function">println</span><span class="token punctuation">(</span><span class="token string">"Ho filtrato un comando"</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
				<span class="token punctuation">}</span><span class="token keyword">else</span><span class="token punctuation">{</span>
					start <span class="token operator">=</span> true<span class="token punctuation">;</span>
					Serial<span class="token punctuation">.</span><span class="token function">println</span><span class="token punctuation">(</span><span class="token string">"Ho abilitato un comando"</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
				<span class="token punctuation">}</span>
			<span class="token punctuation">}</span>
			btntime <span class="token operator">=</span> <span class="token number">0</span><span class="token punctuation">;</span>
		<span class="token punctuation">}</span>
		<span class="token comment">// se premo il pulsante sufficientemente a lungo accendi il led</span>
		<span class="token keyword">if</span><span class="token punctuation">(</span>start <span class="token operator">&amp;&amp;</span> <span class="token punctuation">(</span>btntime <span class="token operator">&gt;=</span> txtime<span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">{</span>
			<span class="token function">digitalWrite</span><span class="token punctuation">(</span>led<span class="token punctuation">)</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
			start <span class="token operator">=</span> false<span class="token punctuation">;</span>
		<span class="token punctuation">}</span>
	<span class="token punctuation">}</span>

<span class="token comment">//switch per un solo pulsante attivo su entrambi i fronti</span>
bool <span class="token function">switchdf</span><span class="token punctuation">(</span>byte val<span class="token punctuation">)</span><span class="token punctuation">{</span>
	bool changed <span class="token operator">=</span> false<span class="token punctuation">;</span>
	changed <span class="token operator">=</span> <span class="token punctuation">(</span>val <span class="token operator">!=</span> precval<span class="token punctuation">)</span><span class="token punctuation">;</span> <span class="token comment">// campiona tutte le transizioni</span>
	precval <span class="token operator">=</span> val<span class="token punctuation">;</span>  <span class="token comment">// valore di val campionato al loop precedente</span>
	<span class="token keyword">return</span> changed<span class="token punctuation">;</span>  <span class="token comment">// rivelatore di fronte (salita o discesa)</span>
<span class="token punctuation">}</span>
</code></pre>

## **POLLED TIMERS**

E’ possibile realizzare dei timers ottenuti eseguendo, ad ogni loop(), il **polling della funzione millis()** per verificare che questa abbia raggiunto il **valore finale** stabilito come target (obiettivo), quando ciò accade, possiamo fare eseguire il blocco di istruzioni **associato** a questo evento.

I timers possono essere **periodici**, ed uno di questi era proprio lo schedulatore adoperato per generare la base degli eventi di sistema, oppure possono essere **aperiodici** (detti anche monostabili), cioè devono essere attivati, con una istruzione apposita, solamente quando serve.

**In** **generale**, possiamo individuare alcune **componenti del timer**:

- **reset** del timer. Accade al verificarsi di una certa **condizione**.

- **polling** del tempo trascorso. Viene fatto ad ogni ciclo di **loop** (talvolta meno).

- **istruzioni triggerate** (scatenate) dal timer. Vengono eseguite (o non eseguite) in base al **tempo** **misurato** dal timer

- (opzionale) la **variabile stato** del timer che **abilita** il timer quando serve e lo **disabilita** quando non serve per evitare **attivazioni spurie** accidentali.

Un esempio di **timer periodico** (del tutto analogo ad un o schedulatore) potrebbe apparire così:
```C++
//Timer periodico
#define PERIODO  1000
unsigned long ptimer1;
bool timerState = false; // stato del timer

void loop()
{
	if ((timerState) && (millis() - ptimer1) >= (unsigned long) PERIODO)
	{
		ptimer1 = millis();
		....
		// istruzioni eseguite periodicamente, se attivo…
	}
}
```

Di seguito è un esempio di **timer aperiodico** che misura il tempo a partire dal verificarsi di una certa condizione fino a che, al superamento di un tempo limite, si decide di fare qualcosa ma solo **dopo** lo scadere del timer:
```C++
//Timer aperiodico 1
#define TLIMITE1  1000
unsigned long atimer1;
void loop()
{
	// condizione che valuta l’accadere di un certo evento
	// di attivazione del conteggio del tempo
	if(condA){
		atimer1 = millis();
	}

	if ((millis() - atimer1) >= (unsigned 	long) TLIMITE1)
	{
		....
		// istruzioni eseguite allo scadere del timer 1
	}
}
```
Analogamente si potrebbe eseguire un blocco di codice continuamente **solo** all’interno di un tempo massimo prestabilito, cioè solo **prima** dello scadere del timer:
```C++
//Timer aperiodico 2
#define TLIMITE  1000
unsigned long atimer2;
void loop()
{
	// condizione che valuta l’accadere di un certo evento
	// di attivazione del conteggio del tempo
	if(condA){
		atimer2 = millis();
	}

	if ((millis() – atimer2) < (unsigned long) TLIMITE2)
	{
		....
		// istruzioni eseguite finchè NON scade il timer 2
	}
}
```
I timers possono anche essere in funzione **all’interno di funzioni** che sono richiamate all’interno del loop():
```C++
void loop()
{
	poll();  // funzione con **blocchi** di codice eseguiti **prima** o **dopo** di certi eventi
	…….
}

void poll()
{
	if(condA){ // evento che attiva il timer (può essere ricorrente ma, in generale, è aperiodico)
		atimer1 = millis();
	}

	// finchè non si raggiunge TLIMITE1 ritorna senza fare nulla
	if ((millis() – atime1) < (unsigned long) TLIMITE1) return;
	//BLOCCO_A

	// finchè non si raggiunge TLIMITE2 ritorna dopo avere eseguito il blocco di istruzioni A
	if ((millis() – atimer2) < (unsigned long) TLIMITE2) return;
	//BLOCCO_B

	// finchè non si raggiunge TLIMITE3 ritorna dopo avere eseguito il blocco di istruzioni

	// A e B
	if ((millis() – atimer1) < (unsigned long) TLIMITE3) return;
}
```
Attenzione ad un **errore insidioso**:
```C++
void poll()
{
	if ((millis() – atimer1) < (unsigned long) TLIMITE1)
	{
	....
	// istruzioni eseguite finchè NON scade il timer 1
	}
	if ((millis() – atimer2) < (unsigned long) TLIMITE2)
	{
	....
	// istruzioni eseguite finchè NON scade il timer 2
	}
}
```

In questa situazione se scade il primo timer **viene comunque controllato** lo scadere del secondo. La cascata degli if equivale ad un OR logico sulle condizioni di scadenza.

Se voglio che **ne accada solo una** posso scrivere così:
```C++
void poll()
{
	if ((millis() – atimer1) < (unsigned long) TLIMITE1)
	{
		....
		// istruzioni eseguite finchè NON scade il timer 1
	}else if ((millis() – atimer2) < (unsigned long) TLIMITE2)
	{
		....
		// istruzioni eseguite finchè NON scade il timer 2
	}
}
```

Dove **l’ordine** stabilisce la **priorità** di valutazione.

Alcuni codici, ritenendo un evento disabilitante per tutto il resto della funzione, talvolta fanno così:
```C++
void poll()
{
	if ((millis() – atimer1) < (unsigned long) TLIMITE1)
	{
		....
		// istruzioni eseguite finchè NON scade il timer 1
		return ; // NON SONO PRONTO! RICONTROLLA AL PROSSIMO GIRO…
	}
	// ISTRUZIONI IMPORTANTI SI MA CHE…QUALCHE VOLTA…NON SI FANNO…
}
```
**Reset** del timer, **polling** del tempo trascorso e **istruzioni triggerate** (scatenate) dal timer potrebbero anche essere rinchiuse in altrettante **funzioni**:
```C++
//inizio variabili timer
unsigned long startTime;
unsigned long timelapse;
byte timerState=0;
//fine variabili timer

void startTimer(unsigned long duration){
	timerState=1;
	timelapse=duration;
	startTime=millis();
}

void stopTimer(){
	timerState=0;
}

//verifica se è arrivato il tempo di far scattare il timer
void aggiornaTimer(){
	if((timerState == 1) && (millis() - startTime >= timelapse)){
		timerState=0;
		onElapse();
	}
}

// azione da compiere allo scadere del timer, definita fuori dal loop
void onElapse(){
	//azione da compiere
	.......
}

void loop(){
	aggiornaTimer();  //aggiorna il primo timer
	//se accade qualcosa parte il timer
	if(A){
		startTimer(1000);
	}

	if(B){ //se accade qualcosa blocco il timer
		stopTimer();
	}
}
```
**POLLED TIMERS SCHEDULATI**

Si possono realizzare timer **anche** a partire dalla **base dei tempi** misurata da uno **schedulatore** semplicemente **contando i passi** raggiunti finchè questi non arrivano ad un valore **target**, **prima** del quale, o **dopo** il quale, far **accadere** qualcosa.

Il codice di seguito fa partire un comando alla pressione di un pulsante solo se questo è stato **premuto per un tempo minimo**, in caso contrario non fa nulla (filtra i comandi)
```C++
unsigned long lastTime = 0;
unsigned long timerDelay = TBASE;  // send readings timer
unsigned step = 0;
bool start=false;
unsigned short  val;
byte precval=0; //switchdf e toggle
byte cmdin=2;

void loop() {
	if ((millis() - lastTime) > timerDelay) {
		lastTime = millis();
		step = (step + 1) % NSTEP;
		btntime = (btntime + 1) % NSTEP;
		val = !digitalRead(cmdin)); // pulsante pull up

		if(switchdf(val)){ //rivelatore di fronte (salita e discesa)
			Serial.println("Ho una transizione dell'ingresso");
			if(val == HIGH){ // ho un fronte di salita
				if(start==true){
					start = false;
					Serial.println("Ho filtrato un comando");
				}else{
					start = true;
					Serial.println("Ho abilitato un comando");
				}
			}
			btntime = 0;
		}
		// se premo il pulsante sufficientemente a lungo accendi il led
		if(start && (btntime >= txtime)){
			digitalWrite(led));
			start = false;
		}
	}

//switch per un solo pulsante attivo su entrambi i fronti
bool switchdf(byte val){
	bool changed = false;
	changed = (val != precval); // campiona tutte le transizioni
	precval = val;  // valore di val campionato al loop precedente
	return changed;  // rivelatore di fronte (salita o discesa)
}
```
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTE3NjU5OTQ5MTVdfQ==
-->