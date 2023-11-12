>[Torna all'indice generale](index.md)

**FILA A**
1) Scrivere un programma Arduino che gestisca un sensore di transito di due sale che ospitano eventi artistici. Il sensore si attiva alla pressione di un pulsante sotto il pavimento. Il gradimento viene valutato contando i calpestii (pressioni). 
2) Un pulsante di reset in sala comando legge il risultato dei contatori e lo invia in rete tramite la seriale come messaggio JSON e azzera il conteggio subito dopo. 
3) Le luci delle due sale sono comandate a distanza tramite un messaggio, unico per entrambe, che arriva via seriale del tipo "luci":"on" e "luci":"off"
4) Se si preme per più di 5 secondi un pulsante in sala comando la luce di entrambe le sale si spegne.

**FILA B**
1) Scrivere un programma Arduino che gestisca un sensore di transito di due sale che ospitano eventi artistici. Il sensore si attiva alla pressione di un pulsante sotto il pavimento. Il gradimento viene valutato contando i calpestii (pressioni) su una pedana centrale. 
2) Realizzare una funzione che invia ogni minuto un messaggio col conteggio sulla seriale e che esegua il reset del conteggio subito dopo.
3) l'inserviente può essere allertato, per ciascuna delle due sale anche attraverso un comando via seriale in formato "salaA":"check" o "salaB":"check"
4) Se la pedana centrale rimane pressata per più di un minuto viene allertato (led) un inserviente che si presenta per un controllo visivo.

**FILA C**
1)	Scrivere un programma arduino che comandi il movimento motorizzato di una carrucola per alimenti (cestino per la spesa) in modo che quando si preme una prima volta parte e quando si preme una seconda volta si ferma. Simulare il motore con un led.
2)	Prevedere un pulsante che gestisca l’inversione della marcia corrente ogni volta che viene premuto. Indicare il verso con un secondo led. Inviare il messaggio di direzione sulla seriale verso una voce sintetizzata che lo legge.
3)	Prevedere di inviare gli stessi comandi via radio con un ricevitore che li trasmette al microcontrollore attraverso la seriale. Il formato tipo è “marcia”:”on” e “marcia”:”off”
4)	Se si preme il pulsante di comando per più di 5 secondi la carrucola torna in posizione tutto alto (motore acceso per 10 secondi.

**FILA D**
1) Scrivere un programma Arduino che gestisca un mezzo dei pompieri. Comandare l'accensione di fari e luci lampeggianti con un comando toggle per ciascuno di essi. 
2) Prevedere anche il comando via seriale proveniente da un telecomando.
3)  Creare un pulsante che comanda l'apertura della scala fintanto che è premuto. Inviare via seriale il conteggio dei metri aperti (uno ogni secondo).
4) Un pulsante comanda l'accensione di tre pompe antincendio, avanti, destra e sinistra, contando il numero delle pressioni (1:avanti, 2:destra, 3:sinistra).


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
1.Scrivere un programma in esp32 che realizzi un pulsante che accende un led solo finchè esso è premuto (pulsante senza memoria). 
    - https://wokwi.com/projects/348779417449792082
2. Al codice precedente aggiungere che alla pressione del tasto, oltre ad accendere un led, venga anche mantenuto il blink (lampeggio) continuo di un secondo led. Sia accensione continua del primo led che il blink del secondo, ovviamente, si interrompono non appena si rilascia il pulsante.
    - https://wokwi.com/projects/348779417449792082
  
  
