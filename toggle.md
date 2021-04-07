---


---

<h2 id="pulsante-toggle"><strong>PULSANTE TOGGLE</strong></h2>
<p>Si vuole realizzare un pulsante con una memoria dello stato che possa essere modificata ad ogni pressione. Pressioni successive in sequenza accendono e spengono un led. Quindi, a seconda da quando si comincia, pressioni in numero pari accendono mentre quelle in numero dispari spengono, oppure al contrario, pressioni in numero pari spengono mentre quelle in numero dispari accendono.</p>
<p>Inizialmente si potrebbe essere tentati di provare seguente soluzione, adattando la soluzione del pulsante precedente introducendo una variabile che conservi lo stato del pulsante che chiameremo <em>closed</em>.</p>
<pre class=" language-c"><code class="prism ++ language-c">byte in<span class="token punctuation">;</span>
byte pulsante <span class="token operator">=</span><span class="token number">2</span><span class="token punctuation">;</span>
boolean closed<span class="token operator">=</span>false<span class="token punctuation">;</span> <span class="token comment">// stato pulsante</span>
<span class="token keyword">void</span> <span class="token function">setup</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>pulsante<span class="token punctuation">,</span> INPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
<span class="token punctuation">}</span>

<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	in <span class="token operator">=</span> <span class="token function">digitalRead</span><span class="token punctuation">(</span>pulsante<span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token keyword">if</span><span class="token punctuation">(</span>in<span class="token operator">==</span>HIGH<span class="token punctuation">)</span><span class="token punctuation">{</span> <span class="token comment">// selezione del livello alto (logica di comando)</span>
		closed <span class="token operator">=</span> <span class="token operator">!</span>closed<span class="token punctuation">;</span>
		<span class="token function">digitalWrite</span><span class="token punctuation">(</span>led<span class="token punctuation">,</span>closed<span class="token punctuation">)</span><span class="token punctuation">;</span>  <span class="token comment">//scrittura uscita</span>
	<span class="token punctuation">}</span>
<span class="token punctuation">}</span>
</code></pre>
<p>Purtroppo questa soluzione ha un paio di <strong>problemi</strong> che ne pregiudicano il <strong>funzionamento corretto</strong>.</p>
<p><strong>Il primo</strong> è relativo alla <strong>selezione del tipo di evento in ingresso</strong>. In questa soluzione viene rilevata la pressione del pulsante <strong>sul livello</strong> dell’ingresso. Il problema è che il livello viene rilevato per tutto il tempo che il tasto è premuto mediante una lettura per ogni loop(). Il numero di queste letture è imprevedibile sia perché sono moltissime al secondo e sia perché la durata della pressione dipende dall’utente. In più, ad ogni lettura viene modificato lo stato del pulsante con l’istruzione closed=!closed, ne consegue che lo stato finale del pulsante è il risultato di una catena di commutazioni che termina dopo un tempo casuale: abbiamo realizzato una slot machine!</p>
<p>La <strong>soluzione</strong> è <strong>modificare il tipo di selezione</strong> <strong>dell’evento</strong> in ingresso, che deve stavolta avvenire sui <strong>fronti</strong> piuttosto che sui livelli. I fronti sono due, <strong>salita</strong> o <strong>discesa</strong>. Se si rileva il fronte di <strong>salita</strong> si cattura l’evento di <strong>pressione</strong> del pulsante, se si rileva il fronte di <strong>discesa</strong> si cattura l’evento di <strong>rilascio</strong> del pulsante. Ma come facciamo a capire quando siamo in presenza di un <strong>evento di fronte</strong>? Di salita o di discesa?</p>
<p><img src="https://cbcdzw.db.files.1drv.com/y4pFGNOjnLgaBqA3py1GTGkVZzVtkKdSOWCj7MLB0nWuJ_2Ksre76C387bHgGPPGSKvFUCHmN2Q7Xw_7MXd8Jj3-xxiE0eaoaKNdkWQlpZQFwaZ0rlEqeWYcDz0Auclmti_0FZ3c_l8k33Md4-OF9SuxWA6p7PF2wht2s2rvs_Eqcgs1TSA1IeCI01iQ6u7X8TpuxXdSGqZkfQdtHfE_k7QdQ/fronti.gif?download&amp;psid=1" alt="fronti"></p>
<p>In <strong>entrambi i casi</strong>, un evento di fronte si ha quando si è in presenza di una <strong>transizione di livello</strong> di un ingresso, quindi per <strong>rilevare un fronte</strong> è sufficiente <strong>rilevare una transizione</strong>.</p>
<p>Una <strong>evento di interrupt</strong> si può rilevare in due modi:</p>
<ul>
<li>
<p><strong>Polling degli ingressi.</strong> Lo scopo è rilevare una <strong>transizione di livello</strong>. Si tratta di campionare periodicamente il livello di un ingresso avendo cura di memorizzare sempre l’ultimo campione misurato su una variabile globale. Al loop successivo questo valore verrà confrontato col nuovo alla ricerca di eventuali variazioni. Il polling, cioè il campionamento, può essere effettuato ad ogni loop, o può essere <strong>decimato</strong>, cioè eseguito periodicamente <strong>ogni tot loop scartati</strong> (decimati).</p>
</li>
<li>
<p><strong>Interrupt.</strong> Su alcune porte di un microcontrollore può essere abilitato il riconoscimento di un evento di interrupt, segnalabile, a scelta, su fronte di salita, fronte di discesa, entrambi i fronti. L’interrupt determina la chiamata asincrona di una funzione di servizio dell’interrupt detta ISR (Interrupt Service Routine) che esegue il codice con la logica di comando in risposta all’evento. La funzione è definita esternamente al loop() e la sua esecuzione è del tutto indipendente da questo.</p>
</li>
</ul>
<p>In figura sono evidenziati i <strong>campionamenti</strong> eseguibili, ad esempio, ad ogni loop(), oppure ad intervalli di tempo preordinati stabiliti da uno schedulatore.</p>
<p><img src="https://db3pap001files.storage.live.com/y4p2YHrngT6Hkl9NxAVyfg6wsB7ROl3_kfJ6hziBzaDqWgcnPxTu8H3u-o9_XiAZ_1xGXStDEAK046VFo-E5uhkyk00F_6NURqCV40zIYBRbLLUPt5Q3dA8oFFM_SMQMhCO-_uzCV6xRgersOgV_JhqLjauq33h1PO2u2XU8MYTZ5nGDDeXrjEmqOp4gMGII-hV/transizioni.png?psid=1&amp;width=453&amp;height=103" alt="transizioni"></p>
<p>Si nota chiaramente che in corrispondenza di ogni fronte, prima e dopo, si misurano differenze di livello. In particolare si può avere:</p>
<ul>
<li>
<p>Un <strong>fronte di salita</strong> se la <strong>transizione</strong> avviene dal livello basso a quello alto</p>
</li>
<li>
<p>Un <strong>fronte di discesa</strong> se la <strong>transizione</strong> avviene dal livello alto a quello basso</p>
</li>
</ul>
<p>Se un rilevatore si limita a segnalare un <strong>generico fronte</strong>, allora per stabilire in quale ci troviamo, basta determinare, al momento della segnalazione, in <strong>quale livello</strong> si trova <strong>l’ingresso</strong>: se è al livello alto è un fronte di salita, se è a quello basso è un fronte di discesa.</p>
<p>Il <strong>secondo problema</strong> è costituito dal fenomeno dei <strong>rimbalzi</strong>. Si palesano come una sequenza di rapide oscillazioni che hanno sia fronti di salita che di discesa. Se l’accensione di un led è associata ad un fronte e il suo spegnimento a quello successivo, allora la pressione di un pulsante realizza, di fatto, la solita slot machine…è necessario un algoritmo di debouncing.</p>
<p>Pulsante toggle con rilevazione del fronte di salita (pressione) e con antirimbalzo:</p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token macro property">#<span class="token directive keyword">define</span> tbase  100  </span><span class="token comment">// periodo base in milliseconds</span>
<span class="token keyword">unsigned</span> <span class="token keyword">long</span> precm<span class="token punctuation">;</span>
<span class="token comment">//unsigned long step;</span>
byte pari<span class="token punctuation">,</span> in<span class="token punctuation">;</span>
byte precval<span class="token punctuation">,</span> val<span class="token punctuation">;</span>
byte led <span class="token operator">=</span> <span class="token number">13</span><span class="token punctuation">;</span>
byte pulsante <span class="token operator">=</span><span class="token number">2</span><span class="token punctuation">;</span>
byte stato<span class="token operator">=</span> LOW<span class="token punctuation">;</span>  <span class="token comment">// variabile globale che memorizza lo stato del pulsante</span>
<span class="token comment">// utilizzare variabili globali è una maniera per ottenere</span>
<span class="token comment">// che il valore di una variabile persista tra chiamate di funzione successive</span>
<span class="token comment">// situazione che si verifica se la funzione è richiamata dentro il loop()</span>

<span class="token keyword">void</span> <span class="token function">setup</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	precm<span class="token operator">=</span><span class="token number">0</span><span class="token punctuation">;</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>led<span class="token punctuation">,</span> OUTPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
	<span class="token function">pinMode</span><span class="token punctuation">(</span>pulsante<span class="token punctuation">,</span> INPUT<span class="token punctuation">)</span><span class="token punctuation">;</span>
	precval<span class="token operator">=</span>LOW<span class="token punctuation">;</span>
<span class="token punctuation">}</span>

<span class="token keyword">void</span> <span class="token function">loop</span><span class="token punctuation">(</span><span class="token punctuation">)</span>
<span class="token punctuation">{</span>
	<span class="token keyword">if</span><span class="token punctuation">(</span><span class="token punctuation">(</span><span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token operator">-</span>precm<span class="token punctuation">)</span> <span class="token operator">&gt;=</span> tbase<span class="token punctuation">)</span><span class="token punctuation">{</span>  <span class="token comment">// schedulatore (e anche antirimbalzo)</span>
		precm <span class="token operator">=</span> <span class="token function">millis</span><span class="token punctuation">(</span><span class="token punctuation">)</span><span class="token punctuation">;</span>  <span class="token comment">// preparo il tic successivo</span>
		<span class="token comment">//codice eseguito al tempo stabilito</span>
		val <span class="token operator">=</span> <span class="token function">digitalRead</span><span class="token punctuation">(</span>pulsante<span class="token punctuation">)</span><span class="token punctuation">;</span>  <span class="token comment">// lettura ingressi</span>
		<span class="token keyword">if</span><span class="token punctuation">(</span>precval<span class="token operator">==</span>LOW <span class="token operator">&amp;&amp;</span> val<span class="token operator">==</span>HIGH<span class="token punctuation">)</span><span class="token punctuation">{</span> <span class="token comment">// rivelatore di fronte di salita</span>
			stato <span class="token operator">=</span> <span class="token operator">!</span><span class="token punctuation">(</span>stato<span class="token punctuation">)</span><span class="token punctuation">;</span> <span class="token comment">// impostazione dello stato del toggle</span>
		<span class="token punctuation">}</span>
		precval<span class="token operator">=</span>val<span class="token punctuation">;</span>  <span class="token comment">//memorizzazione livello loop precedente</span>
		<span class="token function">digitalWrite</span><span class="token punctuation">(</span>led<span class="token punctuation">,</span>stato<span class="token punctuation">)</span><span class="token punctuation">;</span> <span class="token comment">// scrittura uscite</span>
	<span class="token punctuation">}</span>
<span class="token punctuation">}</span>
</code></pre>

