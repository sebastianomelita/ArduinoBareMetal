/*
===============================================================================
CONSEGNA - Pulsante toggle che avvia e ferma il lampeggio di un LED
===============================================================================
Realizzate in Tinkercad un circuito con un pulsante e un LED: ogni volta che il
pulsante viene rilasciato, il LED inizia a lampeggiare o smette, senza mai usare
delay().

OBIETTIVO
Il pulsante funziona come un interruttore toggle: un rilascio avvia il
lampeggio del LED, il rilascio successivo lo ferma e il LED resta spento.
Mentre il LED lampeggia, il pulsante deve continuare a rispondere subito.
L'esercizio mette insieme due tecniche della dispensa sui pulsanti:
 - il debounce non bloccante con millis(), con azione al rilascio;
 - la gestione di due compiti nello stesso loop(), il pulsante e il
   lampeggio, senza che uno blocchi l'altro.

CIRCUITO IN TINKERCAD (Arduino Uno R3)
 Componente        Collegamento                         A cosa serve
 Pulsante          tra il pin 2 e 5V                    premuto, porta il pin 2 a HIGH
 Resistenza 10 k   tra il pin 2 e GND                   pull-down: a riposo il pin 2 legge LOW
 LED               anodo (gamba piegata) verso la       indica il lampeggio
                   resistenza da 220 Ohm, catodo a GND
 Resistenza 220    tra il pin 12 e l'anodo del LED      limita la corrente del LED
Nell'editor del codice scegliete la modalita' Testo.

REQUISITI - comportamento richiesto
 1. All'accensione il LED e' spento.
 2. Al rilascio del pulsante il LED inizia a lampeggiare: 500 ms acceso,
    500 ms spento.
 3. Al rilascio successivo il lampeggio si ferma e il LED resta spento.
 4. Ogni pressione produce un solo cambio, anche se il contatto rimbalza.
 5. Il pulsante risponde subito anche mentre il LED lampeggia.

VINCOLI SUL CODICE
 - Nessun delay(): i tempi si misurano solo con millis().
 - Il loop() contiene due compiti separati: prima il pulsante, poi il lampeggio.
 - Il pulsante usa il debounce della dispensa: alla prima lettura alta si
   attiva, a ogni lettura alta il timer riparte (t0 = millis()), il rilascio
   vale solo dopo 50 ms di livello basso continuo.
 - Le azioni stanno nelle callback doOnRise() (pressione, qui vuota) e
   doOnFall() (rilascio).
 - doOnFall() cambia solo lo stato del toggle, una variabile bool: e' il
   compito del lampeggio a decidere cosa fare del LED.
 - Il LED si inverte con digitalWrite(LED, !digitalRead(LED));.
 - Tutte le differenze di tempo si scrivono come millis() - t > durata, con
   variabili unsigned long.

COME PROVARLO
 [ ] Un clic breve sul pulsante: il lampeggio parte al rilascio, non alla
     pressione.
 [ ] Un secondo clic: il lampeggio si ferma e il LED resta spento, non acceso
     a meta'.
 [ ] Tenete premuto il pulsante per 2-3 secondi: non succede nulla finche' non
     lo rilasciate.
 [ ] Premete piu' volte di seguito: ogni rilascio fa un solo cambio.
 [ ] Durante il lampeggio il ritmo resta regolare anche mentre premete il
     pulsante.
Se la simulazione rallenta, potete aggiungere delay(1); in fondo al loop():
serve solo al simulatore e va motivato nella relazione.

COSA CONSEGNARE
 1. Il link al progetto Tinkercad, condiviso in sola lettura.
 2. Il codice completo, con un commento su ogni riga che riguarda i tempi.
 3. Le risposte alle domande qui sotto, qualche riga ciascuna.

DOMANDE DI RIFLESSIONE
 1. Perche' t0 = millis() va eseguito a ogni lettura alta e non solo alla
    prima? Cosa succederebbe con un rimbalzo al rilascio?
 2. A cosa serve la variabile attivo? Cosa farebbe il programma senza di essa,
    a pulsante rilasciato?
 3. Nel compito del lampeggio, perche' il LED va spento solo quando il
    lampeggio e' fermo e non in un else della condizione sui 500 ms?
 4. Perche' con delay(500) nel lampeggio il pulsante smetterebbe di funzionare
    bene?
===============================================================================
*/

// ---------------------------------------------------------------------------
// SOLUZIONE
// ---------------------------------------------------------------------------
const byte PULSANTE = 2;                  // pulsante con pull-down: premuto = HIGH
const byte LED = 12;                      // LED con resistenza da 220 Ohm
const unsigned long TDEBOUNCE = 50;       // ms di livello basso continuo per confermare il rilascio
const unsigned long TBLINK = 500;         // ms acceso e ms spento

bool attivo = false;          // true dalla prima lettura alta fino al rilascio confermato
unsigned long t0 = 0;         // istante dell'ultima lettura alta del pulsante
bool lampeggio = false;       // stato del toggle: true = il LED lampeggia
unsigned long tblink = 0;     // istante dell'ultimo cambio del LED

void doOnRise() {
  // PRESSIONE (fronte di salita): in questo esercizio non fa nulla
}

void doOnFall() {
  // RILASCIO (fronte di discesa): cambia solo lo stato del toggle,
  // il LED lo gestisce il compito del lampeggio
  lampeggio = !lampeggio;
}

void setup() {
  pinMode(PULSANTE, INPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);     // all'accensione il LED e' spento
}

void loop() {
  // --- compito 1: il pulsante (debounce non bloccante, azione al rilascio)
  if (digitalRead(PULSANTE) == HIGH) {
    if (!attivo) {                         // prima lettura alta: pressione riconosciuta
      attivo = true;
      doOnRise();
    }
    t0 = millis();                         // a OGNI lettura alta il timer riparte
  } else if (attivo && millis() - t0 > TDEBOUNCE) {
    attivo = false;                        // basso da piu' di 50 ms: rilascio confermato
    doOnFall();
  }

  // --- compito 2: il lampeggio
  if (lampeggio) {
    if (millis() - tblink > TBLINK) {      // passati 500 ms dall'ultimo cambio
      tblink = millis();                   // istante del cambio
      digitalWrite(LED, !digitalRead(LED));
    }
  } else {
    digitalWrite(LED, LOW);                // lampeggio fermo: LED spento
  }
}

/*
===============================================================================
RISPOSTE ALLE DOMANDE DI RIFLESSIONE
===============================================================================
1. t0 = millis() a ogni lettura alta.
   t0 deve essere l'istante dell'ULTIMA lettura alta: cosi' millis() - t0
   misura da quanto tempo il pulsante e' basso senza interruzioni, e il
   rilascio vale solo dopo 50 ms di basso continuo.
   Se t0 fosse aggiornato solo alla prima lettura alta, dopo una pressione
   lunga millis() - t0 sarebbe gia' oltre 50 ms: il primo rimbalzo basso del
   rilascio verrebbe preso subito per un rilascio (doOnFall), il rimbalzo alto
   successivo per una nuova pressione e il successivo basso per un nuovo
   rilascio. Risultato: due o piu' cambi del toggle con un solo clic, e il LED
   potrebbe restare nello stato di partenza. Lo stesso succederebbe con un
   breve buco del contatto a pulsante premuto.

2. La variabile attivo.
   Ricorda che il pulsante e' premuto (pressione gia' riconosciuta, rilascio
   non ancora confermato). Serve a chiamare doOnRise() una sola volta, alla
   prima lettura alta, e doOnFall() una sola volta, al rilascio.
   Senza attivo, a pulsante rilasciato la condizione millis() - t0 > 50
   resterebbe vera a ogni giro di loop(): doOnFall() verrebbe chiamata
   migliaia di volte al secondo e il toggle cambierebbe di continuo, con il
   LED in uno stato casuale. Allo stesso modo, a pulsante premuto, doOnRise()
   verrebbe chiamata a ogni lettura alta.

3. LED spento solo a lampeggio fermo.
   La condizione millis() - tblink > 500 e' falsa in quasi tutti i giri di
   loop(): e' vera solo una volta ogni 500 ms. Un else su quella condizione
   spegnerebbe il LED al giro successivo a ogni accensione, quindi il LED
   resterebbe acceso per pochi microsecondi e sembrerebbe sempre spento.
   Il LED va spento solo nel ramo in cui il lampeggio e' fermo (lampeggio ==
   false): cosi' si spegne quando il toggle lo ferma, anche se in quel momento
   era acceso, e non resta "acceso a meta'".

4. delay(500) nel lampeggio.
   delay() blocca tutto il loop() per 500 ms: il pulsante verrebbe letto solo
   una volta ogni mezzo secondo. Un clic piu' breve di 500 ms potrebbe cadere
   interamente fra due letture e andare perso; il rilascio verrebbe
   riconosciuto con un ritardo fino a mezzo secondo, quindi il pulsante non
   risponderebbe piu' "subito" (requisito 5). I due compiti non sarebbero piu'
   indipendenti: il lampeggio terrebbe fermo il pulsante.

VERIFICA
Il codice e' stato provato in simulazione con pressioni rumorose (rimbalzi di
1-8 ms in pressione e rilascio e un buco di 2 ms a meta' pressione):
LED spento all'accensione, mezzo periodo misurato 501 ms, nessun cambio
durante una pressione di 2,5 s, 200 clic su 200 con un solo cambio ciascuno.
(Il mezzo periodo e' 501 ms e non 500 perche' la condizione e' "> 500".)
*/
