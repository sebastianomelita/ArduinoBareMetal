>[Torna all'indice generale](index.md)

# **CONSIDERAZIONI GENERALI SULLA GESTIONE DEI TASK**

<img src="img\applicazMicro.jpg" alt="alt text" width="1000">

## **Logica sequenziale e logica ad eventi**

La **logica** degli algoritmi può essere pensata essenzialmente in due maniere: ad **eventi** e in modo **sequenziale**. 

Nei microcontrollori il **tempo** è un elemento portante della maggiorparte degli algoritmi. Il modo **sequenziale** di pensare il tempo in un algoritmo è sicuramente quello più intuitivo ed ha una sua naturale **corrispondenza** con le istruzioni di un programma. 

Infatti, in un listato sorgente, le istruzioni che **spazialmente** vengono dopo, sono eseguite dopo anche **temporalmente** (se non ci sono stati salti). Quantificare il **dopo** nel tempo tra una istruzione e l'altra è proprio il compito assegnato all'istruzione ```delay(x```), che non fa altro che bloccare l'esecuzione del loop principale (come di un qualsiasi loop secondario basato sui thread) per ```x``` millisecondi.

### **Task sequenziali su un unico flusso (loop o thread)**

Finchè un programma consiste in un **unico task** avente un flusso temporale unico va tutto bene. Il modello lineare di programmazione è chiaro, intuitivo ed adeguato. Quando però, in un algoritmo coesistono più task separati, ciascuno composto da un proprio filo temporale che si interseca con quello degli altri, allora, il problema di codificarlo in un programma si complica. In questo caso, è quindi dirimente sapere se i task di cui si compone l'algoritmo sono **sequenziali** o procedono in maniera **parallela**, e se si, stabilire se, quando si incrociano, un task deve **aspettarne** qualcun altro prima di proseguire.

In linea di principio, se i task sono **sequenziali** nulla osta a metterli tutti nello **stesso loop** principale eseguendoli uno **di seguito** all'altro, dato che il loro sviluppo fisico nel codice è anche il loro sviluppo temporale.

### **Task paralleli su un unico flusso (loop o thread)**

Se, invece, più task sono **paralleli**, allora sviluppare la loro codifica in un **unico flusso** di esecuzione necessita il coordinamento dei **ritardi** di tutti. Per ottenere ciò una strategia potrebbe essere stabilire il **massimo comune divisore** dei tempi che compaiono nei ```delay()```. Se questo MCD esiste, diventa il **tempo base** di tutti i task, cioè quello mediante il quale è possibile **programmare** i tempi dei vari task **incastrandoli** l'uno nell'altro. 

Ad esempio, posso far accadere ogni 2 tempi base il blink di un led rosso e ogni 3 quello di un led verde semplicemente dividendo il tempo in **slot** tramite un delay che vale esattamente un tempo base e scrivendo, per ogni slot, l'istruzione che **compete** ad ogni task. Se un task in uno slot non fa nulla allora li non compare alcuna sua istruzione (vedi Es2).

### **Task paralleli**

Quando, però, i task concorrenti diventano **tanti** od oppure sono **complicati** allora potrebbe risultare tedioso progettarli e realizzarli tutti insieme **interlacciandoli** nel tempo. Un algoritmo siffatto può essere ancora più difficile da realizzare se accade che uno o più task non sono proprio del tutto indipendenti, ma si aspettano l'uno con l'altro in certi istanti.

### **Task paralleli come eventi separati di un unico flusso di esecuzione**

In questo caso le opzioni sono: o cambiare radicalmente approccio adoperando uno stile di programmazione **ad eventi**, perdendo il benefit della intuitività della programmazione sequenziale, oppure mantenere lo stile **lineare e sequenziale** del programma rendendo i task **indipendenti**, non solo concettualmente ma anche nel loro **flusso di esecuzione**.

### **Task paralleli come flussi sequenziali separati (loop o thread)**

Se scegliamo la seconda opzione, task indipendenti vengono mandati in esecuzione su **loop indipendenti** che procedono separatamente ma contemporaneamente, **in parallelo** l'uno con l'altro. I flussi di esecuzione paralleli ed indipendenti vengono chiamati **thread** o **processi**, a seconda dei casi. Noi useremo **solamente i thread**. Per una trattazione più completa della teoria dei thread e di come questi possono essere adoperati nel codice vedi [thread](threadsched.md).

Questo risultato è possibile ottenerlo se il microcontrollore ha la capacità di eseguire più flussi paralleli di codice e ciò è sempre possibile se a bordo è installato un **sistema operativo**. Il microcontrollore **ESP32** ha un su SO mentre **Arduino** no, per cui si possono usare le API dei thread nel codice di un programma solo su ESP32 e non su Arduino.

### **In sintesi**

In **definitiva**, **non** adoperando, per adesso, tecniche di programmazione **ad eventi** dobbiamo limitarci a quelle **sequenziali** che hanno comunque il pregio di essere intrinsecamente più **intuitive** rispetto a quelle ad eventi per una larga categoria di problemi. Le **linee guida** per mantenere uno stile di programmazione **lineare** potrebbero essere:
- se i task procedono in **maniera sequenziale**, cioè, uno dopo l'altro, allora si possono pianificare **indipendentemente** mettendoli fisicamente uno di seguito all'altro all'**interno di uno stesso loop**.
- se i task procedono in maniera **parallela** e indipendente allora si possono pianificare in modo **indipendente** soltanto se si mandano in esecuzione su **thread separati**, all'interno di un **loop() dedicato**, assegnato solamente a loro.
- se i task procedono in maniera **parallela** ma non del tutto indipendente, perchè capita che uno rimane in attesa del risultato dell'altro prima di andare avanti, allora bisogna trovare il modo di **sincronizzarli** con delle istruzioni di **attesa**, non fisse come le ```delay()```, ma dinamiche e legate all'accadere di un certo evento. Un evento è sempre segnalato dalla verità di una certa **condizione**, valutabile da una istruzione come ```waitUntil(c)```, dove ```c``` è la condizione.

<img src="img\task.jpg" alt="alt text" width="1000">


1. [Funzioni di tempo](timefunc.md)
2. [Tecniche di schedulazione](timesched.md)
4. [Generazione di tempi assoluti](absolutetime.md)
5. [Schedulatore di compiti basato sui delay](delayschedulers.md)
6. [Schedulatore di compiti basato sul polling della millis()](tasksched.md)
7. [Schedulatore di compiti basato sui thread](threadsched.md)
8. [Schedulatore di compiti basato sui protothread](protothreadsched.md)
9. [Schedulatore di compiti basato su async/await](async_await.md)
10. [Schedulatore di compiti basato su timers HW](timersched.md)
11. [Eventi periodici contemporanei: esempio di SOS + blink](sosblink.md)
12. [Eventi periodici consecutivi: esempio con i blink ](chainedevents.md)
