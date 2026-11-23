/*
===============================================================================
CONSEGNA - ESERCIZIO D (FACILE) - Lampada con toggle e campanello, versione bloccante
===============================================================================
Classe terza - Sistemi e reti
Argomenti: pull-up e pull-down, rimbalzi, pulsanti senza memoria, gestione
dei pulsanti con attese bloccanti (tecniche 1, 5 e 6).

Realizzate in Tinkercad l'ingresso di una casa: un pulsante accende e spegne
la luce del portico (toggle), un secondo pulsante e' il campanello, che suona
solo finche' e' premuto.

OBIETTIVO
Mettere a confronto un pulsante CON memoria (il toggle della luce, che ha
bisogno del debounce) e uno SENZA memoria (il campanello, che segue il
livello del pin), e scoprire il limite di una tecnica bloccante quando i
pulsanti sono due.

CIRCUITO IN TINKERCAD (Arduino Uno R3)
 Componente             Collegamento                      A cosa serve
 Pulsante LUCE (P1)     tra il pin 2 e 5V                 premuto porta il pin 2 a HIGH
 Resistenza 10 kOhm     tra il pin 2 e GND                pull-down di P1
 Pulsante CAMPANELLO    tra il pin 3 e GND                premuto porta il pin 3 a LOW
 (P2)                   (pull-up interno, INPUT_PULLUP)
 LED bianco o giallo    pin 12, con resistenza 220 Ohm    luce del portico
 Buzzer piezo           tra il pin 8 e GND                campanello

REQUISITI
 1. All'accensione la luce e' spenta e il campanello tace.
 2. A ogni RILASCIO di P1 la luce cambia stato (toggle).
 3. Ogni pressione di P1 produce un solo cambio, anche se il contatto rimbalza.
 4. Finche' P2 e' premuto il buzzer suona (tone(8, 1000)); quando viene
    rilasciato smette (noTone(8)).

VINCOLI SUL CODICE
 - P1 si gestisce con la tecnica 5 della dispensa sulle attese bloccanti
   (letture continue con millis()): se il pin e' HIGH, doOnRise(), poi
   waitUntilInputLow(P1, 50), poi doOnFall().
 - doOnRise() e' vuota; doOnFall() fa il toggle della luce.
 - P2 si legge con digitalRead() a ogni giro di loop(), senza debounce.

COME PROVARLO
 [ ] Clic su P1: la luce si accende al rilascio; secondo clic: si spegne.
 [ ] Clic rapidi su P1: ogni clic fa un solo cambio.
 [ ] Tenete premuto P2: il buzzer suona finche' non rilasciate.
 [ ] Tenete premuto P1 e intanto premete P2: cosa succede? Annotatelo, serve
     per la domanda 4.

COSA CONSEGNARE
 1. Il link al progetto Tinkercad, condiviso in sola lettura.
 2. Il codice completo e commentato.
 3. Le risposte alle domande.

DOMANDE
 1. Compilate la tabella per P1 e per P2: livello a riposo, livello da
    premuto, fronte della pressione, fronte del rilascio.
 2. Perche' il campanello non ha bisogno di debounce, mentre la luce si'? Che
    cosa succederebbe alla luce se P1 rimbalzasse e il programma non avesse
    debounce?
 3. Con la tecnica 1 ("Una lettura ogni t ms") al posto della 5, che errore
    puo' capitare al rilascio di P1? Che effetto vede chi usa la luce?
 4. Nella prova con P1 tenuto premuto, il campanello funziona? Perche'? E'
    un limite della tecnica 5 o di tutte le tecniche bloccanti?
 5. Uno spike di 2 ms sul pin 2, a P1 rilasciato, cambia lo stato della luce
    con la tecnica 5? E con la tecnica 6? Quale delle due scegliereste se il
    cavo del pulsante fosse lungo e disturbato?
===============================================================================
*/

// ---------------------------------------------------------------------------
// SOLUZIONE - P1 toggle con tecnica 5 bloccante, P2 campanello senza memoria
// ---------------------------------------------------------------------------
const byte P1 = 2;                        // LUCE, pull-down: premuto = HIGH
const byte P2 = 3;                        // CAMPANELLO, INPUT_PULLUP: premuto = LOW
const byte LUCE = 12;
const byte BUZZER = 8;
bool luce = false;                        // stato del toggle

void waitUntilInputLow(int btn, unsigned t) {
  // attende t ms di LOW continuo: ogni lettura HIGH fa ripartire il conteggio
  unsigned long tl = millis();            // istante dell'ultima lettura HIGH
  while (millis() - tl <= t) {            // esce dopo t ms di LOW senza interruzioni
    if (digitalRead(btn) == HIGH) {
      tl = millis();
    }
    delay(1);                             // una lettura al millisecondo
  }
}

void doOnRise() {
  // PRESSIONE di P1: qui non serve nulla
}

void doOnFall() {
  // RILASCIO di P1: toggle della luce
  luce = !luce;
  digitalWrite(LUCE, luce);
}

void setup() {
  pinMode(P1, INPUT);                     // pull-down esterno da 10 kOhm
  pinMode(P2, INPUT_PULLUP);
  pinMode(LUCE, OUTPUT);
  digitalWrite(LUCE, LOW);
}

void loop() {
  // --- P1: con memoria, debounce bloccante
  if (digitalRead(P1) == HIGH) {
    doOnRise();
    waitUntilInputLow(P1, 50);            // rilascio: 50 ms di LOW continuo
    doOnFall();
  }

  // --- P2: senza memoria, l'uscita segue il livello
  if (digitalRead(P2) == LOW) {
    tone(BUZZER, 1000);                   // premuto: suona
  } else {
    noTone(BUZZER);                       // rilasciato: tace
  }
}

/*
===============================================================================
RISPOSTE ALLE DOMANDE
===============================================================================
1.                     riposo   premuto   pressione          rilascio
   P1 (pull-down)       LOW      HIGH     fronte di salita   fronte di discesa
   P2 (INPUT_PULLUP)    HIGH     LOW      fronte di discesa  fronte di salita

2. Il campanello e' un pulsante senza memoria: il buzzer segue il livello di
   adesso. Se P2 rimbalza per 5 ms, il suono si accende e spegne per 5 ms e
   nessuno se ne accorge; poi il livello si stabilizza e l'uscita e' giusta.
   La luce ha memoria: ogni rilascio cambia lo stato, e un cambio in piu'
   resta. Senza debounce ogni rimbalzo sarebbe un rilascio in piu' e la luce
   cambierebbe un numero casuale di volte: a volte si accende, a volte no.

3. Con la tecnica 1 al rilascio una lettura puo' cadere in un istante basso
   dei rimbalzi: l'attesa finisce e la luce cambia. Il pulsante pero' sta
   ancora rimbalzando: al giro successivo il loop() legge un HIGH, lo prende
   per una nuova pressione e alla fine la luce cambia di nuovo (pressione
   fantasma). Chi usa la luce vede che ogni tanto il clic "non funziona": la
   luce si accende e si rispegne subito.

4. No. Finche' P1 e' premuto il programma e' fermo dentro
   waitUntilInputLow(): il pin 3 non viene letto, quindi il buzzer resta
   bloccato nello stato in cui era quando si e' premuto P1 (se taceva, tace;
   se suonava, continua a suonare anche dopo aver lasciato P2). Si sblocca al
   rilascio di P1. E' un limite di tutte le tecniche bloccanti: mentre
   aspettano un pulsante non fanno nient'altro. Con la tecnica 6 andrebbe
   peggio: il loop() resta fermo anche in waitUntilInputHigh(), cioe'
   sempre, a pulsante rilasciato, e il campanello non funzionerebbe mai.

5. Tecnica 5: si'. Il loop() legge il pin a ogni giro, vede l'impulso HIGH,
   lo prende per una pressione e 50 ms dopo conferma il rilascio: la luce
   cambia (verificato in simulazione). Tecnica 6: no, perche' la pressione
   deve restare HIGH per 50 ms di fila e uno spike di 2 ms fa solo ripartire
   il conteggio. Con un cavo lungo e disturbato e' piu' sicura la 6, ma in
   questo circuito bloccherebbe il campanello (risposta 4): servirebbe una
   tecnica non bloccante.

VERIFICA IN SIMULAZIONE
300 clic con rimbalzi: 300 cambi della luce; il buzzer suona finche' P2 e'
premuto; uno spike di 2 ms sul pin 2 a riposo cambia la luce.
*/
