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
<p>In un microcontrollore le tre fasi sono eseguite in quell’ordine ma, se inserite dentro la funzione loop(), sono eseguite <strong>periodicamente</strong>, all’infinito, fino allo spegnimento della macchina.</p>
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
<p>Se le metto <strong>“nude e crude”</strong> dentro il blocco loop() vengono eseguite molto spesso, anzi più spesso che si può. Il numero di volte al secondo dipende dalla velocità della CPU e dalla velocità delle singole operazioni.</p>
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
<p><strong>Filtro delle esecuzioni</strong>. Ovviamente blocchi di codice possono essere filtrati tramite istruzioni di selezione, quindi inserendoli nel blocco then o in quello else di un <strong>costrutto if-then-else</strong>. La condizione di selezione può valutare il <strong>tempo</strong> (mediante millis()) oppure altri <strong>ingressi</strong>, oppure lo <strong>stato</strong> di altre variabili del programma.</p>
<p><strong>Memoria</strong>. In genere si fanno frequentemente due cose:</p>
<ul>
<li>
<p>Tenere memoria degli <strong>ingressi</strong> al loop precedente. Cioè conservare il valore corrente di uno o più ingressi in una variabile per poi poterlo “consumare” cioè leggere ed utilizzarlo durante l’esecuzione del loop successivo.</p>
</li>
<li>
<p>Tenere traccia dello <strong>stato</strong> del mio algoritmo, cioè memoria di informazioni importanti (dedotte dalla storia di ingressi e da quella di altre variabili di stato) conservandole all’interno di una o più <strong>variabili</strong>.</p>
</li>
</ul>
<p>In <strong>entrambi</strong> i casi precedenti le informazioni devono “<strong>sopravvivere</strong>” tra un <strong>loop e l’altro</strong>, cioè il loro valore non deve essere cancellato al termine dell’esecuzione della funzione loop() e ciò può essere ottenuto dichiarando le <strong>variabili di memoria globali</strong>, cioè dichiarandole <strong>all’esterno</strong> di tutte le funzioni del sistema, compresa la funzione loop().</p>
<p>**In conclusione, ** quando vogliamo gestire <strong>l’evento di un pulsante</strong> dobbiamo chiederci che <strong>caratteristiche</strong> ha l’evento alla luce delle considerazioni precedenti per capire quale è la <strong>maniera più appropriata</strong> per gestirlo.</p>

