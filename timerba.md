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
		<span class="token comment">//....</span>
		<span class="token comment">// istruzioni eseguite periodicamente, se attivo…</span>
	<span class="token punctuation">}</span>

<span class="token punctuation">}</span>
</code></pre>
<p>Di seguito è un esempio di <strong>timer aperiodico</strong> che misura il tempo a partire dal verificarsi di una certa condizione fino a che, al superamento di un tempo limite, si decide di fare qualcosa ma solo <strong>dopo</strong> lo scadere del timer:</p>
<pre class=" language-c"><code class="prism ++ language-c">
<span class="token comment">//Timer aperiodico 1</span>
<span class="token macro property">#<span class="token directive keyword">define</span> TLIMITE1  1000</span>
<span class="token keyword">unsigned</span> <span class="token keyword">long</span> atimer1<span class="token punctuation">;</span>

<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	<span class="token comment">// condizione che valuta l’accadere di un certo evento</span>
	<span class="token comment">// di attivazione del conteggio del tempo</span>
	<span class="token keyword">if</span><span class="token punctuation">(</span>condA<span class="token punctuation">)</span><span class="token punctuation">{</span>
		atimer1 <span class="token operator">=</span> <span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token punctuation">}</span>

	<span class="token keyword">if</span> <span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span> <span class="token operator">-</span> atimer1 <span class="token operator">&gt;=</span> <span class="token punctuation">(</span><span class="token keyword">unsigned</span> <span class="token keyword">long</span><span class="token punctuation">)</span> TLIMITE1<span class="token punctuation">)</span>
	<span class="token punctuation">{</span>
	<span class="token comment">// istruzioni eseguite allo scadere del timer 1</span>
	<span class="token punctuation">}</span>
<span class="token punctuation">}</span>

</code></pre>

