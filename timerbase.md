---


---

<hr>
<hr>
<p>&lt;![endif]–&gt;</p>
<p><strong>POLLED TIMERS</strong></p>
<p>
&lt;![endif]--&gt;
E’ possibile realizzare dei timers ottenuti eseguendo, ad ogni loop(), il <strong>**polling della funzione millis()</strong> per verificare che questa abbia raggiunto il <strong>valore finale</strong> stabilito come target (obiettivo), quando ciò accade, possiamo fare eseguire il blocco di istruzioni <strong>associato</strong> a questo evento.</p>
I timers possono essere <strong>periodici</strong>, ed uno di questi era proprio lo schedulatore adoperato per generare la base degli eventi di sistema, oppure possono essere <strong>aperiodici</strong> (detti anche monostabili), cioè devono essere attivati, con una istruzione apposita, sola quando serve.
<p><strong>In</strong> <strong>generale</strong>, possiamo individuare alcune <strong>componenti del timer</strong>:</p>
<p>&lt;![if !supportLists]&gt;· &lt;![endif]&gt;<strong>reset</strong>**:
</p><p>&lt;![if !supportLists]&gt;· &lt;![e del timer. Accade al verificarsi di una certa <strong>**condizione</strong>.</p>
<p>&lt;![if !supportLists]&gt;· &lt;![endif]&gt;<strong>polling</strong>**.
</p><p>&lt;![if !supportLists]&gt;· &lt;![endi del tempo trascorso. Viene fatto ad ogni ciclo di <strong>loop</strong> (talvolta meno).</p>
<p>&lt;
</p><p>&lt;![if !supportLists]&gt;· &lt;![endif]&gt;<strong>&gt;· &lt;![endif]&gt;**istruzioni triggerate</strong> (scatenate) dal timer. Vengono eseguite (o non eseguite) in base al <strong>tempo</strong> <strong>misurato</strong> dal timer</p>
<p>&lt;al timer
</p><p>&lt;![if !supportLists]&gt;· &lt;&gt;· &lt;![endif]&gt;&gt;(opzionale) la <strong><strong>variabile stato</strong></strong> del timer che <strong>abilita</strong> il timer quando serve e lo <strong>disabilita</strong>** quando non serve per evitare <strong><strong>attivazioni spurie</strong></strong> accidentali.</p>
<p>
</p><p>Un esempio di <strong>timer periodico</strong> (del tutto analogo ad un o schedulatore) potrebbe apparire così:</p>
<p>
</p><p>//Timer periodico</p>
<p>
#define PERIODO  1000</p>
<p>
unsigned long ptimer1;</p>
<p>
bool timerState = false; // stato del timer</p>
<p>void loop()</p>
<p>{</p>
<p>
</p><p>{</p>
<p>if ((timerState) &amp;&amp;&amp; (millis() - ptimer1) = (unsigned long) PERIODO)</p>
<p>{</p>
<p>
</p><p>{</p>
<p>ptimer1 = millis();</p>
<p>…</p>
<p>
</p><p>…</p>
<p>// istruzioni eseguite periodicamente, se attivo…</p>
<p>}</p>
<p>}</p>
<p>
</p><p>}</p>
<p>}</p>
<p>Di seguito è un esempio di <strong>timer aperiodico</strong> che misura il tempo a partire dal verificarsi di una certa condizione fino a che, al superamento di un tempo limite, si decide di fare qualcosa ma solo <strong>dopo</strong> lo scadere del timer:</p>
<p>
</p><p>//Timer aperiodico 1</p>
<p>
</p><p>#define TLIMITE1  1000</p>
<p>
</p><p>unsigned long atimer1;</p>
<p>
</p><p>void loop()</p>
<p>{</p>
<p>
// condizione che valuta l’accadere di un certo evento</p>
<p>
</p><p>// di attivazione del conteggio del tempo</p>
<p>
</p><p>if(condA){</p>
<p>
</p><p>atimer1 = millis();</p>
<p>}</p>
<p>
</p><p>}</p>
<p>if ((millis() - atimer1) &gt;&gt;= (unsigned long) TLIMITE1)</p>
<p>{</p>
<p>…</p>// istruzioni eseguite allo scadere del timer 1
<p>}</p>
<p>}</p>
<p>
</p><p>}</p>
<p>}</p>
<p>Analogamente si potrebbe eseguire un blocco di codice continuamente <strong>solo</strong> all’interno di un tempo massimo prestabilito, cioè solo <strong>prima</strong> dello scadere del timer:</p><br>
&lt;![endif]–&gt;
<p><strong>POLLED TIMERS</strong></p>
<p>E’ possibile realizzare dei timers ottenuti eseguendo, ad ogni loop(), il <strong>polling della funzione millis()</strong> per verificare che questa abbia raggiunto il <strong>valore finale</strong> stabilito come target (obiettivo), quando ciò accade, possiamo fare eseguire il blocco di istruzioni <strong>associato</strong> a questo evento.</p>
<p>I timers possono essere <strong>periodici</strong>, ed uno di questi era proprio lo schedulatore adoperato per generare la base degli eventi di sistema, oppure possono essere <strong>aperiodici</strong> (detti anche monostabili), cioè devono essere attivati, con una istruzione apposita, solamente quando serve.</p>
<p><strong>In</strong> <strong>generale</strong>, possiamo individuare alcune <strong>componenti del timer</strong>:</p>
<p>&lt;![if !supportLists]&gt;· &lt;![endif]&gt;<strong>reset</strong> del timer. Accade al verificarsi di una certa <strong>condizione</strong>.</p>
<p>&lt;![if !supportLists]&gt;· &lt;![endif]&gt;<strong>polling</strong> del tempo trascorso. Viene fatto ad ogni ciclo di <strong>loop</strong> (talvolta meno).</p>
<p>&lt;![if !supportLists]&gt;· &lt;![endif]&gt;<strong>istruzioni triggerate</strong> (scatenate) dal timer. Vengono eseguite (o non eseguite) in base al <strong>tempo</strong> <strong>misurato</strong> dal timer</p>
<p>&lt;![if !supportLists]&gt;· &lt;![endif]&gt;(opzionale) la <strong>variabile stato</strong> del timer che <strong>abilita</strong> il timer quando serve e lo <strong>disabilita</strong> quando non serve per evitare <strong>attivazioni spurie</strong> accidentali.</p>
<p>Un esempio di <strong>timer periodico</strong> (del tutto analogo ad un o schedulatore) potrebbe apparire così:</p>
<p>//Timer periodico</p>
<p>#define PERIODO  1000</p>
<p>unsigned long ptimer1;</p>
<p>bool timerState = false; // stato del timer</p>
<p>void loop()</p>
<p>{</p>
<p>if ((timerState) &amp;&amp; (millis() - ptimer1) &gt;= (unsigned long) PERIODO)</p>
<p>{</p>
<p>ptimer1 = millis();</p>
<p>…</p>
<p>// istruzioni eseguite periodicamente, se attivo…</p>
<p>}</p>
<p>}</p>
<p>Di seguito è un esempio di <strong>timer aperiodico</strong> che misura il tempo a partire dal verificarsi di una certa condizione fino a che, al superamento di un tempo limite, si decide di fare qualcosa ma solo <strong>dopo</strong> lo scadere del timer:</p>
<p>//Timer aperiodico 1</p>
<p>#define TLIMITE1  1000</p>
<p>unsigned long atimer1;</p>
<p>void loop()</p>
<p>{</p>
<p>// condizione che valuta l’accadere di un certo evento</p>
<p>// di attivazione del conteggio del tempo</p>
<p>if(condA){</p>
<p>atimer1 = millis();</p>
<p>}</p>
<p>if ((millis() - atimer1) &gt;= (unsigned long) TLIMITE1)</p>
<p>{</p>
<p>…</p>
<p>// istruzioni eseguite allo scadere del timer 1</p>
<p>}</p>
<p>}</p>
<p>Analogamente si potrebbe eseguire un blocco di codice continuamente <strong>solo</strong> all’interno di un tempo massimo prestabilito, cioè solo <strong>prima</strong> dello scadere del timer:</p>

