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
>[Torna all'indice generale](index.md)

