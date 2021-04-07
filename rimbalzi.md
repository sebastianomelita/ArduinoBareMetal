---


---

<h2 id="il-problema-dei-rimbalzi-meccanici"><strong>IL PROBLEMA DEI RIMBALZI MECCANICI</strong></h2>
<p>Un <strong>pulsante reale</strong> (non semplicemente simulato) non si comporta come ci aspetteremmo, cioè ad una pressione non si limita semplicemente a chiudere il circuito dei contatti in esso contenuti. Di seguito è riportato graficamente <strong>l’andamento nel tempo</strong> del suo <strong>comportamento</strong>:</p>
<p><img src="https://db3pap001files.storage.live.com/y4pbZbxldjiU0x6ZjHykCIOISrWHKAh-U76AxcNrqlHxy9BmhHkc-k5q3sWzZ42xlaVT6VQXLAQUoJhXJoRFQ0fVReSzayKnxDaU8AV0dcSg-91HcpOZJUrogSgrKiN8cX-v4EKgMIlnLuizJHuUxZ0xpFU3y4mtgnPCFdzboq32fsSqVkYg-s1beUBjmy6Ty9w/rimbalzi.jpg?psid=1&amp;width=787&amp;height=413" alt="rimbalzi"></p>
<p>Il fenomeno inatteso è la sequenza di <strong>rimbalzi</strong> meccanici che seguono la <strong>pressione</strong> del pulsante che si estingue dopo un certo tempo dipendente dalla qualità e dall’usura del contatto.</p>
<p>I rimbalzi sono molto veloci ma comunque sufficientemente lenti da essere <strong>campionati</strong> dalle frequenti letture delle porte condotte alla <strong>massima velocità</strong> della CPU. Il campionamento dei rimbalzi genera una <strong>sequenza di stati logici</strong> opposti il cui <strong>numero</strong> non è prevedibile che, dopo un intervallo transitorio, si <strong>stabilizza</strong> sul valore finale della lettura a tasto premuto (HIGH in pulldown, LOW in pullup).</p>
<p>In misura minore esiste lo stesso fenomeno <strong>al rilascio</strong> del pulsante, cioè la fase di rilascio avviene con oscillazioni che conducono al valore finale rilasciato (LOW in pulldown, HIGH in pullup).</p>
<p><strong>E’ sempre dannoso</strong> questo fenomeno? La risposta giusta è <strong>dipende</strong>. Nel caso dell’ultimo esempio di un semplice pulsante in pratica no. Questo perché le incertezze sono così veloci e cosi limitate del tempo che non si è in grado di apprezzarle percettivamente con i nostri sensi. Alla vista il fenomeno della persistenza delle immagini sulla retina (inerzia del nostro sensore naturale) ci impedirà di apprezzare la rapida variazione di luminosità conseguente alla corrispondente catena di accensioni e spegnimenti. Ma anche se non le <strong>apprezziamo percettivamente</strong>, queste esistono e potrebbero essere <strong>apprezzate strumentalmente</strong>.</p>
<p>La CPU è in grado di <strong>campionarle</strong>, rilevarle e <strong>includerle</strong> nella normale elaborazione degli input. Se questa elaborazione è ad esempio, sensibile al <strong>numero dei cambiamenti</strong> di stato dell’ingresso, essendo questo oscillante in maniera non prevedibile, può generare <strong>risultati altrettanto imprevedibili.</strong></p>
<p><strong>In sostanza</strong>, in tutte quelle situazioni in cui è importante elaborare <strong>il numero delle transizioni</strong> da acceso a spento e viceversa il fenomeno dei rimbalzi <strong>è sicuramente dannoso.</strong></p>
<p><strong>Come si può risolvere il problema dei rimbalzi?</strong> Esistono due tipologie di soluzioni:</p>
<ul>
<li>
<p>HW con dei circuiti detti debouncer realizzabili sia con elettronica analogica che digitale</p>
</li>
<li>
<p>SW con un algoritmo che ottiene lo stesso scopo del circuito HW <strong>filtrando</strong> opportunamente i campionamenti.</p>
</li>
</ul>
<p>Noi tratteremo solo i <strong>debouncer di tipo SW</strong>. Questi realizzano un filtraggio digitale degli ingressi sostanzialmente ottenibile:</p>
<ul>
<li>
<p>con operazioni di <strong>media</strong> dei valori misurati nel tempo.</p>
</li>
<li>
<p>Con il <strong>filtraggio dei campionamenti</strong>, ottenuto facendo in modo che la distanza temporale tra l’uno e l’altro sia così grande da estinguere al suo interno i transitori delle oscillazioni, ciò garantisce che <strong>due misure consecutive</strong> non possono essere entrambe la misura di una oscillazione. Il <strong>filtraggio</strong> può essere:</p>
<ul>
<li>
<p><strong>Periodico</strong> schedulando ad <strong>intervalli regolari</strong> i campionamenti. Ciò si ottiene utilizzando un generico <strong>algoritmo di schedulazione</strong> (schedulatore SW) dei task tarato per il debouncing.</p>
</li>
<li>
<p><strong>Aperiodico</strong> registrando in una variabile il <strong>momento</strong> in cui si verificano <strong>i cambiamenti di stato degli ingressi</strong>, e ignorando tutti quelli successivi che stanno all’interno di un <strong>tempo limite</strong> che marca la <strong>zona di inaffidabilità</strong> suscettibile ai rimbalzi. Ciò si ottiene impostando un <strong>timer</strong> che si attiva ad ogni transizione.</p>
</li>
</ul>
</li>
</ul>
<p>I <strong>timer</strong> si possono realizzare in due maniere:</p>
<ul>
<li>
<p><strong>Polling della funzione millis().</strong> Viene campionato l’istante di attivazione del timer e da questo momento viene misurato, ad ogni loop(), il valore corrente della funzione millis() se la differenza tra l’ultimo istante campionato è minore del timeout non accade nulla, se invece lo è si esegue il codice previsto per lo scadere del timer.</p>
</li>
<li>
<p><strong>Interrupt generato da un timer HW.</strong> Si imposta il timer, questo è un circuito HW che procede parallelamente alla funzione loop() e in maniera indipendente da essa. Allo scadere del timer viene inviato un segnale di interrupt che può essere raccolto da una ISR (Interrupt Service Routine) che esegue al suo interno il codice previsto allo scadere del timer.</p>
</li>
</ul>
<p>Nel seguito introdurremo due tecniche basate su <strong>un filtraggio periodico</strong> dei valori, uno basato sui delay() e uno basato sulla millis().</p>

