---


---

<h2 id="accensione-di-un-led-mediante-un-pulsante"><strong>ACCENSIONE DI UN LED MEDIANTE UN PULSANTE</strong></h2>
<p><img src="https://db3pap001files.storage.live.com/y4pLA1Tn26nfeXqgSYvnWrNrv0nD3tOnKmWQxhb-a6K7C1DyW23JQ0GkGQvNVVRx1IDiYCUaB0pMxHYFgw4oKdyFJq43nNCzo90NAAzEwcZANi9quPCd3xGLrx0erEtwSX4NVI3O5NJTrc_sQ_CJwxx0DXbXopOsovgl8s9yotoz-Qwdz9wT0tdI4mGGrABEHAn3_afZkmf50hIbXvK0Y0Qfx_imCmgEILOpO-NuVcYdbI/accensioneled.png?psid=1&amp;width=824&amp;height=349" alt="accensioneled"></p>
<p>In realtà dovremmo vedere la pressione del pulsante come un generico <strong>evento di input</strong> al quale il microcontrollore <strong>risponde</strong> generando un <strong>output</strong> dopo avere <strong>elaborato una logica di comando</strong>. Possiamo interpretare <strong>la logica di comando</strong> come l’algoritmo che genera la risposta all’evento.</p>
<p>Il discorso è generale e non vale solo per i pulsanti ma per qualunque <strong>applicazione</strong> eseguita da un microcontrollore, che, non a caso, in ambito industriale è spesso indicato come <strong>Logic Solver</strong>.</p>
<p>Quindi le <strong>fasi</strong> di una <strong>generica applicazione</strong> dovrebbero essere nell’ordine:</p>
<ul>
<li>
<p><strong>Lettura</strong> ingressi</p>
</li>
<li>
<p><strong>Elaborazione logica</strong> di comando</p>
</li>
<li>
<p><strong>Scrittura</strong> uscite</p>
</li>
</ul>
<p>Ad es.:</p>
<pre class=" language-c"><code class="prism ++ language-c">val <span class="token operator">=</span> <span class="token function">digitalRead</span><span class="token punctuation">(</span>pulsante<span class="token punctuation">)</span><span class="token punctuation">;</span>  <span class="token comment">// lettura ingressi</span>

stato <span class="token operator">=</span> <span class="token operator">!</span><span class="token punctuation">(</span>stato<span class="token punctuation">)</span><span class="token punctuation">;</span> <span class="token comment">// calcolo logica di comando</span>

<span class="token function">digitalWrite</span><span class="token punctuation">(</span>led<span class="token punctuation">,</span>stato<span class="token punctuation">)</span><span class="token punctuation">;</span> <span class="token comment">// scrittura uscite</span>
</code></pre>
<p>In un microcontrollore le <strong>tre fasi</strong> sono eseguite in quell’ordine ma, se inserite dentro la funzione loop(), sono eseguite <strong>periodicamente</strong>, all’infinito, fino allo spegnimento della macchina.</p>
<p>Questo fatto impone alcune riflessioni:</p>
<ul>
<li>
<p><strong>Quanto spesso</strong> vengono eseguite queste fasi? Posso controllare la loro periodicità?</p>
</li>
<li>
<p><strong>Devono</strong> <strong>essere eseguite</strong> <strong>sempre</strong>? Posso filtrare la loro esecuzione?</p>
</li>
<li>
<p><strong>Sono con memoria</strong>? Cioè posso legare il risultato di una fase a quello delle fasi eseguite in precedenza?</p>
</li>
</ul>
<p><strong>Quanto spesso</strong>.</p>
<ul>
<li>
<p>Se le metto <strong>“direttamente”</strong> dentro il blocco loop() vengono eseguite molto spesso, anzi più spesso che si può. Il numero di volte al secondo dipende dalla velocità della CPU e dalla velocità delle singole operazioni.</p>
</li>
<li>
<p>Se le metto dentro un blocco di codice la cui esecuzione è forzata ad avvenire in certi istanti prestabiliti (ad esempio ogni 9 loop) allora possono essere eseguite meno frequentemente. La <strong>periodicità</strong> dell’esecuzione è controllabile mediante algoritmi di <strong>scheduling</strong> (pianificazione nel tempo) che possono essere realizzati in vario modo. In genere, <strong>a basso livello</strong>, i modi sono tre:</p>
<ul>
<li>
<p><strong>Ritardo</strong> dell’esecuzione mediante funzione delay() impostabile ad un tempo di millisecondi prefissato.</p>
</li>
<li>
<p><strong>Polling</strong> della funzione millis() nel loop che permette di scegliere l’istante di tempo adatto per eseguire un certo blocco di codice posto all’interno del loop (tipicamente dentro un blocco if-then-else con una condizione che valuta millis())</p>
</li>
<li>
<p><strong>Interrupt</strong> generato da un timer HW che permette di eseguire una funzione definita al di fuori dal loop() allo scadere del timer.</p>
</li>
</ul>
</li>
</ul>
<p><strong>Filtro delle esecuzioni</strong>. Ovviamente blocchi di codice possono essere filtrati tramite <strong>istruzioni di selezione</strong>, quindi inserendoli nel blocco then o in quello else di un <strong>costrutto if-then-else</strong>. La condizione di selezione può valutare il <strong>tempo</strong> (la faccio durare fino ad un certo tempo) oppure altri <strong>ingressi</strong> (confronto il valore attuale di un ingresso con quello di altri ingressi), oppure lo <strong>stato</strong> del sistema (se il motore è in movimento faccio una certa cosa se no non la faccio). Di seguito la fase di scrittura delle uscite non viene eseguita ad ogni loop ma solo se un certo ingresso ha un determinato valore:</p>
<pre class=" language-c"><code class="prism ++ language-c"><span class="token keyword">if</span><span class="token punctuation">(</span>in<span class="token operator">==</span>HIGH<span class="token punctuation">)</span><span class="token punctuation">{</span>
	<span class="token function">digitalWrite</span><span class="token punctuation">(</span>led<span class="token punctuation">,</span>closed<span class="token punctuation">)</span><span class="token punctuation">;</span>  <span class="token comment">//scrittura uscita</span>
<span class="token punctuation">}</span>
</code></pre>
<p><strong>Memoria</strong>. In genere si fanno frequentemente due cose:</p>
<ul>
<li>Tenere memoria degli <strong>ingressi</strong> al loop precedente. Cioè conservare il valore corrente di uno o più ingressi in una variabile per poi poterlo “consumare” cioè leggere ed utilizzarlo durante l’esecuzione del loop successivo.<pre class=" language-c"><code class="prism ++ language-c">pval <span class="token operator">=</span> val<span class="token punctuation">;</span> <span class="token comment">// ultima istruzione che chiude loop()</span>
</code></pre>
</li>
<li>Tenere traccia dello <strong>stato</strong> del mio algoritmo, cioè memoria di informazioni importanti (dedotte dalla storia di ingressi e da quella di altre variabili di stato) conservandole all’interno di una variabile di stato. Ad esempio se<br>
deduco il nuovo stato da quello precedente:<pre class=" language-c"><code class="prism ++ language-c">stato <span class="token operator">=</span> <span class="token operator">!</span>stato 
</code></pre>
oppure, se deduco il nuovo stato da un ingresso e dallo stato precedente:<pre class=" language-c"><code class="prism ++ language-c"><span class="token keyword">if</span><span class="token punctuation">(</span>in <span class="token operator">==</span> HIGH <span class="token operator">&amp;&amp;</span> stato <span class="token operator">==</span> <span class="token number">0</span><span class="token punctuation">)</span> 	stato <span class="token operator">=</span> <span class="token number">1</span><span class="token punctuation">;</span> 
</code></pre>
</li>
</ul>
<p>In <strong>entrambi</strong> i casi precedenti le informazioni devono “<strong>sopravvivere</strong>” tra un <strong>loop e l’altro</strong>, cioè il loro valore non deve essere cancellato al termine dell’esecuzione della funzione loop() e ciò può essere ottenuto dichiarando le <strong>variabili di memoria globali</strong>, cioè dichiarandole <strong>all’esterno</strong> di tutte le funzioni del sistema, compresa la funzione loop().</p>
<p><strong>In conclusione,</strong> quando vogliamo gestire <strong>l’evento di un pulsante</strong> dobbiamo chiederci che <strong>caratteristiche</strong> ha l’evento alla luce delle considerazioni precedenti per capire quale è la <strong>maniera più appropriata</strong> per gestirlo.</p>

