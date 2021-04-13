>[Torna all'indice](indexinterrupts.md)
## **SEZIONI CRITICHE**

Comandi per abilitare e disabilitare interrupts:

**Abilitazione:**
```C++
interrupts ();  // or ...
sei ();         // set interrupts flag
```

**Disabilitazione:**
```C++
noInterrupts ();  // or ...
cli ();           // clear interrupts flag
```


**Sezioni critiche**

Le variabili condivise tra ISR e programma principale devono essere protette da accessi concorrenti, cioè contemporanei tra ISR e altre istruzioni (in genere di scrittura).

Il problema deriva dal fatto che alcune istruzioni di accesso alle variabili, come le assegnazioni, non sono atomiche, questo significa che sono scomponibili in due o più istruzioni assembly che sono suscettibili di essere separate da una chiamata di interrupt: se la chiamata ISR legge una variabile che è stata scritta solo parzialmente, il risultato può essere impredicibile e causare malfunzionamenti. 

La soluzione è aver cura, nel programma principale, di effettuare tutte le operazioni di scrittura (modifica del valore) delle variabili condivise tra loop() e ISR in maniera strettamente atomica, cioè le istruzioni assembly sottostanti devono essere svolte o tutte o nessuna.

Per ottenere, dentro il loop(), l’atomicità delle istruzioni su una variabile è sufficiente che queste siano rese non interrompibili, disabilitando gli interrupt immediatamente prima di esse (CLI) e riabilitandoli immediatamente dopo di esse (STI). 

L’equivalente ad alto livello di una istruzione assembly CLI è il comando noInterrupts(), mentre l’equivalente ad alto livello di una istruzione assembly STI è il comando interrupts(). 

Il blocco delimitato da due chiamate successive noInterrupts() e interrupts() viene detto, in gergo, sezione critica. 
```C++
noInterrupts ();
long myCounter = isrCounter;  // get value set by ISR
interrupts ()
```
Una sezione critica delimita quelle porzioni del codice che devono essere eseguite in maniera strettamente seriale, senza parallelizzazioni.

Le parallelizzazioni, nel programma principale, possono incrementare le prestazioni di un programma ma, per essere eseguite in maniera safe (sicura), devono riguardare le parti del codice:

-	non condivise tra programma principale e ISR
-	essere di sola lettura se riguardano informazioni (variabili) condivise tra programma principale e ISR



**Sicurezza delle letture**

Le operazioni di lettura se avvengono su variabili non condivise (locali al main), anche se interrotte, sono ritenute safe (sicure) perché le interruzioni non danneggiano il loro valore e quindi non creano inconsistenze (valori ambigui, ormai privi di significato): benchè interrotte, in qualunque punto del loop le variabili assumono sempre lo stesso valore se nessuno nel loop le modifica (ed essendo non condivise, nessuno oltre il loop può farlo).

Le operazioni di lettura su variabili condivise tra loop e ISR, se nel loop avvengono in concomitanza di una scrittura di un task concorrente, cioè dell’ISR, potrebbero portare ad un **risultato inconsistente** (si potrebbe leggere un misto tra il valore prima e quello dopo la scrittura) che chiaramente rappresenta un valore **corrotto** e **privo di significato**. 

In ogni caso, pur in assenza di valori inconsistenti, può sempre capitare che, a seguito di letture in punti diversi del loop(), la stessa variabile condivisa potrebbe assumere valori diversi se, tra le due letture successive, è avvenuta una interruzione che ne ha modificato il valore.

Per evitare questo tipo di anomalie in lettura, le soluzioni si potrebbero realizzare:
-	**mantenendo le interruzioni**, facendo in modo che:
    -	**all’inizio del codice** del loop, copiare la variabile condivisa su una variabile locale mediante un’operazione di scrittura (assegnamento) che dovrebbe avvenire all’interno di una corsa critica per proteggerla da eventuali interruzioni.
    -	nel **resto del codice**, accedere in lettura alla sola variabile locale che, anche se viene interrotta, manterrà comunque, in ogni parte del codice, il suo valore originale che non verrà aggiornato prima del loop successivo.
-	**eliminando le interruzioni**. E’ la soluzione più drastica, basta trattare tutte le operazioni in lettura sulle variabili condivise come se fossero in scrittura e quindi proteggere ogni singolo accesso con una corsa critica che, disabilitando gli interrupt, impedisce le interruzioni. La soluzione può essere macchinosa in presenza di molti accessi o in presenza di istruzioni condizionali, cicli, ecc.

**Quale codice proteggere?**

Il codice da racchiudere in una sezione critica dovrebbe includere tutte le istruzioni suscettibili di essere svolte in maniera non atomica. Alcune, però in Arduino sono, per loro natura, atomiche e non è necessario proteggerle: sono quelle che accedono (in lettura o scrittura) a variabili ad 8bit. 

Una modifica (scrittura)a una variabile **a 8 bit** è atomica. Può essere usata in maniera safe sia dentro che fuori un ISR.
A maggior ragione, le variabili ad 8bit in Arduino sono sicure anche in lettura pur se condivise con una ISR.
Le modifiche a valori maggiori non lo sono, pertanto le variabili a 16 o 32bit andrebbero gestite con gli interrupt disabilitati (sezione critica). Tuttavia, gli interrupt vengono disabilitati durante una routine di servizio di interrupt, quindi non si verificherà il danneggiamento di una variabile multibyte nell'ISR. 

Quindi, riassumendo, per variabili multibyte:
-	dentro l’ISR. il valore di una variabile multibyte non può cambiare perché di default gli interrupt sono disabilitati. Non è necessario usare un blocco noInterrupts()-interrupts().
-	al di fuori dell'ISR . Il valore in una variabile multibyte può cambiare durante un'operazione di lettura/scrittura che deve essere protetta disabilitando gli interrupt durante la lettura/scrittura e quindi riabilitandoli subito dopo. E’ necessario usare un blocco noInterrupts()-interrupts().

**Salvataggio stato corrente interrupts**

Talvolta si vogliono realizzare corse critiche che non alterino lo stato iniziale degli interrupt, cioè che realizzino questo risultato:
-	se erano disattivati prima del blocco da proteggere lo devono rimanere anche dopo
-	se erano attivati prima del blocco da proteggere lo devono rimanere anche dopo

Una funzione, non sapendo lo stato effettivo degli interrupt al momento della sua invocazione e non volendo alterarli:
 1.	memorizza lo stato corrente degli interrupts (attivati/disattivati) su una variabile di appoggio oldSREG
 2.	disattiva gli interrupts con CLI() oppure noInterrupts()
 3.	esegue il codice proprio della funzione
 4.	ripristina lo stato precedente degli interrupts (attivati/disattivati) memorizzato in oldSREG

Esempio che mostra come viene realizzata una corsa critica dentro la funzione millis():

```C++
unsigned long millis()
{
  unsigned long m;
  uint8_t oldSREG = SREG;

  // disabilita gli interrupts per evitare valori inconsistenti
  cli(); // inizio corsa critica

  m = timer0_millis;  // scrittura variabile long NON atomica!

  SREG = oldSREG;  //fine corsa critica

  return m;
}
```
>[Torna all'indice](indexinterrupts.md)
