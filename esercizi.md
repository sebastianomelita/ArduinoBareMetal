>[Torna all'indice generale](index.md)

## **CONSIDERAZIONI GENERALI SUI PROBLEMI**

La logica degli algoritmi può essere pensata essenzialmente in due maniere: ad **eventi** e in modo **sequenziale**. 

Nei microcontrollori il **tempo** è un elemento portante della maggiorparte degli algoritmi. Il modo **sequenziale** di pensare il tempo in un algoritmo è sicuramente quello più intuitivo ed ha una sua naturale **corrispondenza** con le istruzioni di un programma. 

Infatti, in un listato sorgente, le istruzioni che **spazialmente** vengono dopo, sono eseguite dopo anche **temporalmente** (se non ci sono stati salti). Quantificare il **dopo** nel tempo tra una istruzione e l'altra è proprio il compito assegnato all'istruzione ```delay(x```), che non fa altro che bloccare l'esecuzione del loop principale (come di un qualsiasi loop secondario basato sui thread) per ```x``` millisecondi.

Finchè un programma consiste in un **unico task** avente un flusso temporale unico va tutto bene. Il modello lineare di programmazione è chiaro, intuitivo ed adeguato. Quando però, in un algoritmo coesistono più task separati, ciascuno composto da un proprio filo temporale che si interseca con quello degli altri, allora, il problema di codificarlo in un programma si complica. In questo caso, è quindi dirimente sapere se i task di cui si compone l'algoritmo sono **sequenziali** o procedono in maniera **parallela**, e se si, stabilire se, quando si incrociano, un task deve **aspettarne** qualcun altro prima di proseguire.

In linea di principio, se i task sono **sequenziali** nulla osta a metterli tutti nello **stesso loop** principale eseguendoli uno **di seguito** all'altro, dato che il loro sviluppo fisico nel codice è anche il loro sviluppo temporale.

Se, invece, più task sono **paralleli**, allora sviluppare la loro codifica in un unico flusso di esecuzione necessita il coordinamento dei ritardi di tutti. Per ottenere ciò una strategia potrebbe essere stabilire il **massimo comune divisore** dei tempi che compaiono nei ```delay()```. Se questo MCD esiste, diventa il **tempo base** di tutti i task, cioè quello mediante il quale è possibile **programmare** i tempi dei vari task **incastrandoli** l'uno nell'altro. 

Ad esempio, posso far accadere ogni 2 tempi base il blink di un led rosso e ogni 3 quello di un led verde semplicemente dividendo il tempo in **slot** tramite un delay che vale esattamente un tempo base e scrivendo, per ogni slot, l'istruzione che **compete** ad ogni task. Se un task in uno slot non fa nulla allora li non compare alcuna sua istruzione (vedi Es2).

Quando, però, i task concorrenti diventano **tanti** od oppure sono **complicati** allora potrebbe risultare tedioso progettarli e realizzarli tutti insieme interlacciandoli nel tempo. Un algoritmo siffatto può essere ancora più difficile da realizzare se accade che uno o più task non sono proprio del tutto indipendenti, ma si aspettano l'uno con l'altro in certi istanti.

In questo caso le opzioni sono: o cambiare radicalmente approccio adoperando uno stile di programmazione **ad eventi**, perdendo il benefit della intuitività della programmazione sequenziale, oppure mantenere lo stile **lineare e sequenziale** del programma rendendo i task **indipendenti**, non solo concettualmente ma anche nel loro **flusso di esecuzione**.

Se scegliamo la seconda opzione, task indipendenti vengono mandati in esecuzione su **loop indipendenti** che procedono separatamente ma contemporaneamente, **in parallelo** l'uno con l'altro. I flussi di esecuzione paralleli ed indipendenti vengono chiamati **thread** o **processi**, a seconda dei casi. Noi useremo **solamente i thread**. Per una trattazione più completa della teoria dei thread e di come questi possono essere adoperati nel codice vedi [thread](threadsched.md).

Questo risultato è possibile ottenerlo se il microcontrollore ha la capacità di eseguire più flussi paralleli di codice e ciò è sempre possibile se a bordo è installato un **sistema operativo**. Il microcontrollore **ESP32** ha un su SO mentre **Arduino** no, per cui si possono usare le API dei thread nel codice di un programma solo su ESP32 e non su Arduino.

In **definitiva**, **non** adoperando, per adesso, tecniche di programmazione **ad eventi** dobbiamo limitarci a quelle **sequenziali** che hanno comunque il pregio di essere intrinsecamente più **intuitive** rispetto a quelle ad eventi per una larga categoria di problemi. Le **linee guida** per mantenere uno stile di programmazione **lineare** potrebbero essere:
- se i task procedono in **maniera sequenziale**, cioè, uno dopo l'altro, allora si possono pianificare **indipendentemente** mettendoli fisicamente uno di seguito all'altro all'**interno di uno stesso loop**.
- se i task procedono in maniera **parallela** e indipendente allora si possono pianificare in modo **indipendente** soltanto se si mandano in esecuzione su **thread separati**, all'interno di un **loop() dedicato**, assegnato solamente a loro.
- se i task procedono in maniera **parallela** ma non del tutto indipendente, perchè capita che uno rimane in attesa del risultato dell'altro prima di andare avanti, allora bisogna trovare il modo di **sincronizzarli** con delle istruzioni di **attesa**, non fisse come le ```delay()```, ma dinamiche e legate all'accadere di un certo evento. Un evento è sempre segnalato dalla verità di una certa **condizione**, valutabile da una istruzione come ```waitUntil(c)```, dove ```c``` è la condizione.

## **TIMER UR**

E’ possibile realizzare dei timers, con cui programmare **nel futuro** lo stesso evento o una sequenza di eventi diversi, eseguendo un polling del **tempo di conteggio**. 

Un **polling** è l'interrogazione periodica di una **variabile** o di un **ingresso** per leggerne il **valore**. Se il valore è maggiore di un **tempo massimo** allora si considera avvenuto un **timeout** e si esegue il **blocco** di codice associato ad esso.

Il polling serve per verificare, ad ogni loop(), che il tempo trascorso (**elapsed**) abbia raggiunto il **valore finale** (timeout) stabilito come obiettivo (target). Ciò si può ottenere eseguendo il **test** periodico di una **condizione** su una funzione di **misura** del tempo, nello specifico una ```get()```, che, ad ogni ```loop()```, restituisce il **tempo di conteggio**. Se la **condizione di uscita** dal conteggio è **vera** allora il **blocco** di istruzioni associato al **timeout del timer** viene **eseguito**.

Il **timeout** si controlla, in definitiva, valutando la **condizione di uscita** sulla funzione ```get()``` mediante una istruzione di selezione **if**. 

I **timer sequenziali** sono più **intuitivi** e **semplici** da usare rispetto ad i **timer ad eventi** perchè possono essere adoperati usando la stessa logica **sequenziale** e **lineare** che si usa in un normale algoritmo sincrono. Un'**algoritmo sincrono** pianifica le azioni in base alla **posizione** nel codice delle sue istruzioni e in base ai tempi stabiliti da eventuali **ritardi fissi** posti tra un'azione e l'altra. Ma le azioni si possono programmare anche in base ad eventuali **ritardi variabili**, cioè non prestabiliti, che bloccano l'esecuzione in attesa del termine del **polling di un timer** (test di avvenuto timeout) o del **polling di un ingresso** (test di avvenuta consegna di un dato) o, ancora, in attesa del **polling di una variabile flag** che segnali l'accadere di un qualunque altro evento.

Si tratta di un **pattern** (tipo di API) per la realizzazione di timers **molto comune** nella programmazione di **bracci robotici** per uso industriale (vedi bracci Universal Robots) che si adatta bene ad essere impiegato sia in un contesto in cui la logica dell'algoritmo è realizzata in maniera sequenziale sia in quello in cui gli input sono gestiti con un modello ad eventi.

**In** **generale**, possiamo individuare alcune **componenti del timer**:

- **reset** del timer. Accade al verificarsi di una certa **condizione**. Determina sia l'azzeramento del **tempo di conteggio** che l'azzeramento della misura del ritardo dall'ultimo riavvio. L'azzeramento **non** è mai automatico ed è necessario impostarlo prima del riutilizzo del timer dopo uno o più comandi ``stop()`` (tipicamente al momento della valutazione della condizione di timeout) altrimenti è sufficiente richiamare la funzione ```start()```.  
- **start**. Avvia o riavvia il timer quando si avvera la **condizione di attivazione** posta in punto qualsiasi del loop. Imposta il flag di stato ```timerState```. 
- **stop** del timer. Accade al verificarsi di una certa **condizione** di sospensione del timer posta in punto qualsiasi del loop. In questa occasione il timer **campiona** il ritardo accumulato dall'ultimo riavvio e lo somma al **tempo di conteggio** cumulato in corrispondenza della sospensione precedente. In questo modo il tempo trascorso è memorizzato e può essere valutato in un momento successivo nel codice tramite la funzione ```get()```.
- **get** del tempo **tempo di conteggio** (elapsed) dal momento del **primo avvio** del timer. Viene fatto ad ogni ciclo di **loop** (meno se filtrato) e serve a realizzare il confronto con un tempo di timeout mediante un operatore di confronto (```var.get() > timeout```). Il valore restituito dipende dallo stato del timer:
     - se **```timerState == false```** allora restituisce l'ultimo **tempo di conteggio** misurato e memorizzato al momento della chiamata a stop() .
     - se **```timerState == true```** allora calcola il **tempo di conteggio** corrente campionando il ritardo attuale dall'ultimo riavvio e sommandolo ai ritardo cumulato prima dell'ultima sospensione.
- **istruzioni triggerate** (scatenate) dal timer. Vengono eseguite in base al **tempo di conteggio misurato** dal timer. Vengono eseguite in maniera sequenziale in un punto stabilito del loop (istruzioni sincrone) in corrispondenza della **verità** di una certa **condizione** sul tempo di conteggio che coinvolge la funzione ```get()``` come operando. In corrispondenza si dovrebbero eseguire:
  -    **reset()**  per azzerare il timer.
  -    **stop()** per bloccare il timer.
 
Per consultare dettagli sulla sua implementazione vedi [timer sequenziali](polledtimer_seq.md).

## ESERCIZI SU PULSANTI (NORMALI E TOGGLE) E TASK CONCORRENTI

### **Es1**
1) Creare un programma che realizzi l'accesione di due led. Uno ogni mezzo secondo in modalità "normale" ed uno ogni scendo in modalità "drain".
2) Svolgere lo stesso esercizio di cui sopra ma fare in modo che il blink di uno dei led duri soltanto 10 lampeggi.
- https://wokwi.com/projects/378094177285037057
- https://wokwi.com/projects/378093455257452545

### **Es2**
Creare un programma che gestisca l'accensione di due led (uno in modo drain e l'altro normale) in maniera tale che:
    - led1 blinki mezzo secondo per 5 volte dopo di che blinki ogni secondo
    - led2 blinki un secondo per 5 volte dopo di che blinki ogni mezzo secondo
  
  - https://wokwi.com/projects/381214153548600321

### **Es3**  
Creare un programma che gestisca l'accensione di due led (uno in modo drain e l'altro normale) in maniera tale che:
  - il led1 blinki ogni mezzo secondo
  - il led2 blinki ogni secondo
  - il led2 parta dopo che il led1 abbia blinkato per 3 volte e si sia spento
  - il led1 si riaccenda dopo che il led2 abbia blinkato per due volte e si sia spento
    
https://wokwi.com/projects/381214508523057153

### **Es4**
Realizzare un programma arduino che faccia lampeggiare un led mentre un altri tre led (rosso, verde e giallo) realizzano la tempistica di un semaforo.
- https://wokwi.com/projects/379451315100484609
- https://wokwi.com/projects/379449185988576257
- https://wokwi.com/projects/379393885414092801
  
### **Es5**
Scrivere e simulare un programma (arduino o esp32) che realizzi l'accensione di ul led funchè si preme un pulsante e contemporaneamente il blink di un'altro.
- https://wokwi.com/projects/379577438032393217

### **Es6**
Scrivere un programma (arduino o esp32) che realizzi un blink di un led giallo fino a che si tiene premuto un pulsante e realizzi pure il blink di un led verde quando viene premuto un altro pulsante.
- https://wokwi.com/projects/381194244081573889
- https://wokwi.com/projects/381194871814782977

### **Es7**
Scrivere e simulare un programma (arduino o esp32) che realizzi l'accensione di un led finchè si preme un pulsante e, contemporaneamente, il blink di un altro led.
- https://wokwi.com/projects/379577438032393217
- https://wokwi.com/projects/381114962105698305

### **Es8**
Compilare un progetto con esp32 che realizzi il comando di accensione/spegnimento di un led in modo toggle con una pressione sullo stesso tasto (una pressione accende, la successiva spegne).
- https://wokwi.com/projects/381115546873501697
  
### **Es9**
Come sopra, ma con la variante che, solo quando il primo led è accesso, la pressione di un secondo pulsante accende/spegne in modo toggle il blink di un secondo led. Se il primo led è spento il secondo pulsante non fa nulla.
- https://wokwi.com/projects/381120076068796417
- https://wokwi.com/projects/381116393250548737

## **URUTILS**
I prossimi esercizi sono realizzati adoperando la libreria [urtils](urutils.h)

### **Es10**
Creare un programma per esp32 che realizzi con 4 led l'effetto supercar inserbile e disinseribile in modo toggle con un pulsante.
- https://wokwi.com/projects/381192229624334337
  
### **Es11**
Lo stesso di prima ma con un blink di un led a parte che deve rimanere insensibile ai comandi che agiscono sui led supercar (nessun ritardo).
- https://wokwi.com/projects/381192787395509249
  
### **Es12**
Fare un toggle che accende un led dopo 4 volte che si preme un pulsante e lo spegne dopo 2 (suggerimento: prevedere un tempo massimo complessivo per le pressioni di 1 sec).
- https://wokwi.com/projects/381191721792664577

### **Es13**
Creare un programma per esp32 che realizzi l'accensione di un led su 4 alla prima pressione, di un secondo led alla seconda, di un terzo alla terza. 
- https://wokwi.com/projects/381146888228329473

### **Es14**
1. Scrivere un programma in esp32 che realizzi un pulsante che accende un led solo finchè esso è premuto (pulsante senza memoria). 
    - https://wokwi.com/projects/348779417449792082
2. Al codice precedente aggiungere che alla pressione del tasto, oltre ad accendere un led, venga anche mantenuto il blink (lampeggio) continuo di un secondo led. Sia accensione continua del primo led che il blink del secondo, ovviamente, si interrompono non appena si rilascia il pulsante.
    - https://wokwi.com/projects/348779417449792082
3.  Al codice di cui sopra, aggiungere un secondo tasto che realizzi, in modo toggle (tasto con memoria), l'accensione di un terzo led.
    - https://wokwi.com/projects/381144987021586433
    - https://wokwi.com/projects/381122295298210817
4.  Al codice precedente aggiungere che il toggle inibisca completamente l'effetto del primo tasto, lasciando tutto spento, quando lo stato del toggle è false mentre lo riabilita quando lo stato del toggle è true.
    - https://wokwi.com/projects/381145527357009921

### **Es15**
1. Scrivere un programma in esp32 che realizzi un pulsante che quando esso è premuto accenda un led verde e ne spenga uno rosso. Quando si rilascia deve accadere esattamente il viceversa.
    - https://wokwi.com/projects/381379298406892545
2. Al codice di cui sopra, aggiungere un secondo tasto che realizzi, in modo toggle (tasto con memoria), il blink di un terzo led.
    - https://wokwi.com/projects/381379835699414017
3. Al codice precedente aggiungere che alla pressione del primo tasto, parta pure un contatore di secondi. Dopo 10 secondi si deve attivare il toggle del terzo led.
    - https://wokwi.com/projects/381380050178326529
    - https://wokwi.com/projects/381380815530919937

### **Es16**
1. Scrivere un programma in esp32 che realizzi un pulsante che quando esso è premuto accenda il blink alternato di due led giallo e rosso. Quando si rilascia deve rimanere attivo solo il led rosso.
   -    https://wokwi.com/projects/381412873080576001
3. Al codice di cui sopra, aggiungere un secondo tasto che realizzi, in modo toggle (tasto con memoria), il blink di un terzo led ma solo se il pulsante precedente è rilasciato.
   -    https://wokwi.com/projects/381413194370570241
4. Al codice precedente aggiungere che alla pressione del primo tasto, parta pure un contatore di secondi. Dopo 5 secondi si deve attivare il toggle del terzo led.
   -    https://wokwi.com/projects/381413625688702977

### **Es17**
1. Scrivere un programma che accenda due led con un tasto senza memoria.  Quando il tasto è premuto, il primo  led lampeggia due volte dopodichè il secondo lampeggia tre volte, dopodichè entrambi lampeggiano indefinitivamente.
   -    https://wokwi.com/projects/381751605001285633
2. Dato l'esercizio al punto precedente, aggiungere un secondo tasto e un terzo led. Il nuovo tasto azionato in modo Toggle, fa lampeggiare il terzo led oppure lo spegne. Fare in modo che quando i primi due led lampeggiano insieme il secondo tasto cambi stato una sola volta.
   -    https://wokwi.com/projects/381413194370570241
3. Al codice precedente aggiungere la funzione che spegne il lampeggio sul terzo led dopo un tempo di 10 secondi (timer UR in dispensa).
   -    https://wokwi.com/projects/381754353576022017

### **Es18**
1. Scrivere un programma che accenda due led con un tasto senza memoria. Quando il tasto è premuto, il primo  led lampeggia dieci volte dopodichè entrambi, dopodichè entrambi lampeggiano alternativamente indefinitivamente.

2. Dato l'esercizio al punto precedente, aggiungere un secondo tasto e un terzo led. Il nuovo tasto azionato in modo Toggle, fa lampeggiare il terzo led con il codice morse delll'SOS (tre impulsi brevi, tre lunghi e di nuovo tre brevi). Una pressione successiva del tasto lo fa blinkare normalmente.

3. Al codice precedente aggiungere la funzione che spegne l'esecuzione dell'SOS dopo 10 secondi. Dopo 20 secondi il led SOS smette pure di blinkare (timer UR in dispensa).

### **Es19**
1. Scrivere un programma che accenda due led con un tasto senza memoria.  Quando il tasto è premuto, il led lampeggiano 5 volte alternativamente in maniera rapida, dopodichè entrambi lampeggiano alternativamente in maniera più lenta per un tempo indefinito.

2. Dato l'esercizio al punto precedente, aggiungere un secondo tasto e un terzo led. Il nuovo tasto azionato in modo Toggle, fa lampeggiare i primi due led in maniera alternata e rapida per un tempo indefinito. Una seconda pressione li fa lampeggiare indefinitivamente in maniera più lenta. 

3. Al codice precedente aggiungere la funzione che misura il tempo x in cui si preme il tasto senza memoria e al suo rilascio fa lampeggiare periodicamente un terzo led con un periodo pari ad x  (timer UR in dispensa).

### **Es20**
1. Scrivere un programma che accenda insieme led con un tasto senza memoria.  Quando il tasto è premuto, il led lampeggiano 4 volte insieme, dopodichè entrambi lampeggiano alternativamente uno alla volta 4 volte per un tempo indefinito.

2. Dato l'esercizio al punto precedente, aggiungere un secondo tasto e un terzo led. Il nuovo tasto azionato in modo Toggle, fa lampeggiare il secondo led secondo la logica al punto 1) se lo stato è true, altrimenti sta spento.

3. Al codice precedente aggiungere la funzione che misura il tempo x in cui si preme il primo tasto senza memoria e al suo rilascio fa accendere il terzo led dopo x secondi per mezzo secondo, poi lo spegne e lo riaccende dopo x=x/2 secondi, poi lo spegne e lo riaccende dopo x=x/2 secondi, e così via. Quando il tempo è sotto 100 ms il led rimane perennemente acceso. Il processo ricomincia ad una nuova pressione del primo tasto ed un suo nuovo rilascio (timer UR in dispensa).

### **Es21**
Scrivere un programma che realizzi l'accensione di un led tramite un pulsante temporizzato che spegne il led 
dopo un numero di ms impostati da setup. La logica del tasto deve essere senza stato e deve essere sensibile 
al fronte di salita del segnale. Immaginare un collegamento pull down del tasto.
- https://www.tinkercad.com/things/fCpauVnNUZh-accensione-led-monostabile/editel

### **Es22**
Scrivere un programma che realizzi l'accensione di un led, due led o tre led tramite la pressione consecutiva di un pulsante 
una, due o tre volte all'interno di un intervallo temporale di un secondo.
Col la pressione prolungata dello stesso pulsante si spengono tutti e tre contemporaneamente.
(Realizzazione del timer senza schedulatore)


>[Torna all'indice generale](index.md)
