---


---

<p>&lt;![endif]–&gt;</p>
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
<p>//Timer aperiodico 2</p>
<p>#define TLIMITE  1000</p>
<p>unsigned long atimer2;</p>
<p>void loop()</p>
<p>{</p>
<p>// condizione che valuta l’accadere di un certo evento</p>
<p>// di attivazione del conteggio del tempo</p>
<p>if(condA){</p>
<p>atimer2 = millis();</p>
<p>}</p>
<p>if ((millis() – atimer2) &lt; (unsigned long) TLIMITE2)</p>
<p>{</p>
<p>…</p>
<p>// istruzioni eseguite finchè NON scade il timer 2</p>
<p>}</p>
<p>}</p>
<p>I timers possono anche essere in funzione <strong>all’interno di funzioni</strong> che sono richiamate all’interno del loop():</p>
<p>void loop()</p>
<p>{</p>
<p>poll();  // funzione con <strong>blocchi</strong> di codice eseguiti <strong>prima</strong> o <strong>dopo</strong> di certi eventi</p>
<p>…….</p>
<p>}</p>
<p>void poll()</p>
<p>{</p>
<p>if(condA){ // evento che attiva il timer (può essere ricorrente ma, in generale, è aperiodico)</p>
<p>atimer1 = millis();</p>
<p>}</p>
<p>// finchè non si raggiunge TLIMITE1 ritorna senza fare nulla</p>
<p>if ((millis() – atime1) &lt; (unsigned long) TLIMITE1) return;</p>
<p>//BLOCCO_A</p>
<p>// finchè non si raggiunge TLIMITE2 ritorna dopo avere eseguito il blocco di istruzioni A</p>
<p>if ((millis() – atimer2) &lt; (unsigned long) TLIMITE2) return;</p>
<p>//BLOCCO_B</p>
<p>// finchè non si raggiunge TLIMITE3 ritorna dopo avere eseguito il blocco di istruzioni</p>
<p>// A e B</p>
<p>if ((millis() – atimer1) &lt; (unsigned long) TLIMITE3) return;</p>
<p>}</p>
<p>Attenzione ad un <strong>errore insidioso</strong>:</p>
<p>void poll()</p>
<p>{</p>
<p>if ((millis() – atimer1) &lt; (unsigned long) TLIMITE1)</p>
<p>{</p>
<p>…</p>
<p>// istruzioni eseguite finchè NON scade il timer 1</p>
<p>}</p>
<p>if ((millis() – atimer2) &lt; (unsigned long) TLIMITE2)</p>
<p>{</p>
<p>…</p>
<p>// istruzioni eseguite finchè NON scade il timer 2</p>
<p>}</p>
<p>}</p>
<p>In questa situazione se scade il primo timer <strong>viene comunque controllato</strong> lo scadere del secondo. La cascata degli if equivale ad un OR logico sulle condizioni di scadenza.</p>
<p>Se voglio che <strong>ne accada solo una</strong> posso scrivere così:</p>
<p>void poll()</p>
<p>{</p>
<p>if ((millis() – atimer1) &lt; (unsigned long) TLIMITE1)</p>
<p>{</p>
<p>…</p>
<p>// istruzioni eseguite finchè NON scade il timer 1</p>
<p>}else if ((millis() – atimer2) &lt; (unsigned long) TLIMITE2)</p>
<p>{</p>
<p>…</p>
<p>// istruzioni eseguite finchè NON scade il timer 2</p>
<p>}</p>
<p>}</p>
<p>Dove <strong>l’ordine</strong> stabilisce la <strong>priorità</strong> di valutazione.</p>
<p>Alcuni codici, ritenendo un evento disabilitante per tutto il resto della funzione, talvolta fanno così:</p>
<p>void poll()</p>
<p>{</p>
<p>if ((millis() – atimer1) &lt; (unsigned long) TLIMITE1)</p>
<p>{</p>
<p>…</p>
<p>// istruzioni eseguite finchè NON scade il timer 1</p>
<p><strong>return 0;</strong> // NON SONO PRONTO! RICONTROLLA AL PROSSIMO GIRO…</p>
<p>}</p>
<p>// ISTRUZIONI IMPORTANTI SI MA CHE…QUALCHE VOLTA…NON SI FANNO…</p>
<p>}</p>
<p><strong>Reset</strong> del timer, <strong>polling</strong> del tempo trascorso e <strong>istruzioni triggerate</strong> (scatenate) dal timer potrebbero anche essere rinchiuse in altrettante <strong>funzioni</strong>:</p>
<p>//inizio variabili timer</p>
<p>unsigned long startTime;</p>
<p>unsigned long timelapse;</p>
<p>byte timerState=0;</p>
<p>//fine variabili timer</p>
<p>void startTimer(unsigned long duration){</p>
<p>timerState=1;</p>
<p>timelapse=duration;</p>
<p>startTime=millis();</p>
<p>}</p>
<p>void stopTimer(){</p>
<p>timerState=0;</p>
<p>}</p>
<p>//verifica se è arrivato il tempo di far scattare il timer</p>
<p>void aggiornaTimer(){</p>
<p>if((timerState == 1) &amp;&amp; (millis() - startTime &gt;= timelapse)){</p>
<p>timerState=0;</p>
<p>onElapse();</p>
<p>}</p>
<p>}</p>
<p>// azione da compiere allo scadere del timer, definita fuori dal loop</p>
<p>void onElapse(){</p>
<p>//azione da compiere</p>
<p>…</p>
<p>}</p>
<p>void loop(){</p>
<p>aggiornaTimer();  //aggiorna il primo timer</p>
<p>//se accade qualcosa parte il timer</p>
<p>if(A){</p>
<p>startTimer(1000);</p>
<p>}</p>
<p>if(B){ //se accade qualcosa blocco il timer</p>
<p>stopTimer();</p>
<p>}</p>
<p>}</p>
<p><strong>POLLED TIMERS SCHEDULATI</strong></p>
<p>Si possono realizzare timer <strong>anche</strong> a partire dalla <strong>base dei tempi</strong> misurata da uno <strong>schedulatore</strong> semplicemente <strong>contando i passi</strong> raggiunti finchè questi non arrivano ad un valore <strong>target</strong>, <strong>prima</strong> del quale, o <strong>dopo</strong> il quale, far <strong>accadere</strong> qualcosa.</p>
<p>Il codice di seguito fa partire un comando alla pressione di un pulsante solo se questo è stato <strong>premuto per un tempo minimo</strong>, in caso contrario non fa nulla (filtra i comandi)</p>
<p>unsigned long lastTime = 0;</p>
<p>unsigned long timerDelay = TBASE;  // send readings timer</p>
<p>unsigned step = 0;</p>
<p>bool start=false;</p>
<p>unsigned short  val;</p>
<p>byte precval=0; //switchdf e toggle</p>
<p>byte cmdin=2;</p>
<p>void loop() {</p>
<p>if ((millis() - lastTime) &gt; timerDelay) {</p>
<p>lastTime = millis();</p>
<p>step = (step + 1) % NSTEP;</p>
<p>btntime = (btntime + 1) % NSTEP;</p>
<p>val = !digitalRead(cmdin)); // pulsante pull up</p>
<p>if(switchdf(val)){ //rivelatore di fronte (salita e discesa)</p>
<p>Serial.println(“Ho una transizione dell’ingresso”);</p>
<p>if(val == HIGH){ // ho un fronte di salita</p>
<p>if(start==true){</p>
<p>start = false;</p>
<p>Serial.println(“Ho filtrato un comando”);</p>
<p>}else{</p>
<p>start = true;</p>
<p>Serial.println(“Ho abilitato un comando”);</p>
<p>}</p>
<p>}</p>
<p>btntime = 0;</p>
<p>}</p>
<p>// se premo il pulsante sufficientemente a lungo accendi il led</p>
<p>if(start &amp;&amp; (btntime &gt;= txtime)){</p>
<p>digitalWrite(led));</p>
<p>start = false;</p>
<p>}</p>
<p>}</p>
<p>//switch per un solo pulsante attivo su entrambi i fronti</p>
<p>bool switchdf(byte val){</p>
<p>bool changed = false;</p>
<p>changed = (val != precval); // campiona tutte le transizioni</p>
<p>precval = val;  // valore di val campionato al loop precedente</p>
<p>return changed;  // rivelatore di fronte (salita o discesa)</p>
<p>}</p>

