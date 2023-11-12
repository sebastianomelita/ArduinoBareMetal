>[Torna all'indice generale](index.md)

## **CONSIDERAZIONI GENERALI SUI PROBLEMI**

La logica degli algoritmi può essere pensata essenzialmente in due maniere: ad **eventi** e in modo **sequenziale**. 

Nei microcontrollori il **tempo** è un elemento portante della maggiorparte degli algoritmi. Il modo **sequenziale** di pensare il tempo in un algoritmo è sicuramente quello più intuitivo ed ha una sua naturale **corrispondenza** con le istruzioni di un programma. 

Infatti, in un listato sorgente, le istruzioni che **spazialmente** vengono dopo vengono sempre eseguite dopo anche **temporalmente** (se non ci sono stati salti). Quantificare il **dopo** nel tempo tra una istruzione e l'altra è proprio il compito assegnato all'istruzione ```delay(x```) che non fa altro che bloccare l'esecuzione del loop principale (come di un qualsiasi loop secondario basato sui thread) per ```x``` millisecondi.

Finchè un programma è consiste in un **unico task** con un flusso temporale unico va tutto bene. Il modello lineare di programmazione è chiaro, intuitivo ed adeguato. Quando però, in un algoritmo coesistono più task separati, ciascuno composto da un proprio filo temporale che si interseca con quello degli altri, allora, il problema di codificarlo in un programma si complica. In questo caso, è quindi dirimente sapere se i task di cui si compone l'algoritmo sono **sequenziali** o procedono in maniera **parallela**, e se si, stabilire se, quando si incrociano, un task deve **aspettarne** qualcun altro prima di proseguire.

In linea di principio, se i task sono **sequenziali** nulla osta a metterli tutti nello **stesso loop** principale eseguendoli uno **di seguito** all'altro, dato che il loro sviluppo fisico nel codice è anche il loro sviluppo temporale.

Se, invece, più task sono **paralleli** allora sviluppare la loro codifica in un unico flusso di esecuzione necessita il coordinamento dei ritardi di tutti. Per ottenere ciò una strategia potrebbe essere stabilire il **massimo comune divisore** dei tempi che compaiono nei ```delay()```. Se questo MCD esiste, diventa il **tempo base** di tutti i task mediante il quale è possibile **programmare** i tempi dei vari task **incastrandoli** l'uno nell'altro. 

Ad esempio, posso far accadere ogni 2 tempi base il blink di un led rosso e ogni 3 quello di un led verde semplicemente dividendo il tempo in **slot** tramite un delay che vale esattamente un tempo base e scrivendo, per ogni slot, l'istruzione che **compete** ad ogni task. Se un task in uno slot non fa nulla allora li non compare alcuna sua istruzione (vedi Es2).

Quando, però, i task concorrenti diventano **tanti** od oppure sono **complicati** allora potrebbe risultare tedioso progettarli e realizzarli tutti insieme interlacciandoli nel tempo. Un algoritmo siffatto può essere ancora più difficile da realizzare quando uno o più task in realtà non sono proprio del tutto indipendenti, ma si aspettano l'uno con l'altro.

In questo caso le opzioni sono: o cambiare radicalmente approccio adoperando uno stile di programmazione **ad eventi**, perdendo il benefit della intuitività della programmazione sequenziale, oppure mantenere lo stile **lineare e sequnziale** del programma rendendo i task **indipendenti**, non solo concettualmente ma anche nel loro **flusso di esecuzione**.

Nel secondo caso, task indipendenti vengono mandati in esecuzione su **loop indipendenti** che procedono separatamente ma contemporaneamente, **in parallelo** l'uno con l'altro. I flussi di esecuzione paralleli ed indipendenti vengono chiamati **thread** o **processi**, a seconda dei casi. Noi useremo **solamente thread**. Per una trattazione più completa della teoria dei thread e di come questi possono essere adoperati nel codice vedi [thread](threadsched.md).

Questo risultato è possibile ottenerlo se il microcontrollore ha la capacità di eseguire più flussi paralleli di codice e ciò è sempre possibile se a bordo è installato un **sistema operativo**. Il microcontrollore **ESP32** ha un su SO mentre **Arduino** no, per cui si possono usare le API dei thread nel codice di un programma solo su ESP32 e non su Arduino.

In **definitiva**, **non** adoperando, per adesso, tecniche di programmazione **ad eventi** dobbiamo limitarci a quelle **sequenziali** che hanno comunque il pregio di essere intrinsecamente più **intuitive** rispetto a quelle ad eventi per una larga categoria di problemi. Le **linee guida** per mantenere uno stile di programmazione **lineare** potrebbero essere:
- se i task procedono in **maniera sequenziale**, cioè, uno dopo l'altro, allora si possono pianificare **indipendentemente** mettendoli fisicamente uno di seguito all'altro all'**interno di uno stesso loop**.
- se i task procedono in maniera **parallela** e indipendente allora si possono pianificare in modo **indipendente** soltanto se si mandano in esecuzione su **thread separati**, all'inteno di un **loop() dedicato**, assegnato solamente a loro.
- se i task procedono in maniera **parallela** ma non del tutto indipendente, perchè capita che uno rimane in attesa del risultato dell'altro prima di andare avanti, allora bisogna trovare il modo di **sincronizzarli** con delle istruzioni di **attesa**, non fisse come le ```delay()```, ma dinamiche e legate all'accadere di un certo evento, segnalato dalla verità di una certa **condizione**, valutabile da una istruzione come ```waitUntil(c)```, dove ```c``` è la condizione.


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
I prossimi esercizi somo realizzati adoperando la libreria [urtils](urutils.h)

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

>[Torna all'indice generale](index.md)
