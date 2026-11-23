/*
===============================================================================
CONSEGNA - ESERCIZIO C (DIFFICILE) - Gioco dei riflessi, polling non bloccante su due fronti
===============================================================================
Classe terza - Sistemi e reti
Argomenti: pull-down, le quattro situazioni critiche, polling non bloccante su
due fronti (versioni 1 e 4), confronto fra prontezza e sicurezza.

Realizzate in Tinkercad un gioco che misura il tempo di reazione: dopo
un'attesa casuale si accende un LED verde e il giocatore deve premere il
pulsante il piu' presto possibile. Il tempo viene stampato sul Monitor
seriale. Chi preme prima del verde fa una falsa partenza.

OBIETTIVO
Qui la pressione e il rilascio servono a cose diverse: la PRESSIONE ferma il
cronometro (deve essere riconosciuta subito, altrimenti il tempo e' falsato),
il RILASCIO fa partire la manche successiva. Il programma deve controllare il
pulsante anche durante l'attesa casuale, per accorgersi delle false partenze:
per questo non puo' essere bloccante.

CIRCUITO IN TINKERCAD (Arduino Uno R3)
 Componente           Collegamento                     A cosa serve
 Pulsante             tra il pin 2 e 5V                premuto porta il pin 2 a HIGH
 Resistenza 10 kOhm   tra il pin 2 e GND               pull-down
 LED verde            pin 11, con resistenza 220 Ohm   "via!"
 LED rosso            pin 9,  con resistenza 220 Ohm   falsa partenza
Il Monitor seriale va aperto a 9600 baud.

REQUISITI
 Il gioco ha tre stati: ATTESA, VIA, FINE.
 1. ATTESA: LED spenti. Si estrae un'attesa casuale fra 2 e 5 s
    (random(2000, 5001)); quando e' trascorsa si passa a VIA.
 2. VIA: si accende il LED verde e si memorizza l'istante di accensione.
    Alla PRESSIONE il verde si spegne, si stampa il tempo di reazione in ms e
    si passa a FINE.
 3. Se il pulsante viene premuto in ATTESA (falsa partenza) si accende il LED
    rosso, si stampa "Falsa partenza!" e si passa a FINE.
 4. FINE: al RILASCIO del pulsante si spengono i LED e riparte una nuova
    manche in ATTESA.
 5. Dopo 5 manche valide si stampano il tempo migliore e la media; poi si
    ricomincia da capo.
 6. Ogni pressione conta una sola volta, anche se il contatto rimbalza.

VINCOLI SUL CODICE
 - Il pulsante si gestisce con la versione 1 della dispensa sul polling non
   bloccante su due fronti (millis(), t0, attivo): pressione riconosciuta
   alla prima lettura alta, rilascio confermato da 50 ms di livello basso.
 - doOnRise() gestisce la pressione (stop del cronometro o falsa partenza),
   doOnFall() la ripartenza; entrambe agiscono in base allo stato del gioco.
 - Lo stato del gioco e' una variabile (byte o enum) e il loop() ha due
   compiti separati: prima il pulsante, poi l'avanzamento del gioco.
 - Nessun delay(); tutte le differenze di tempo come millis() - t > durata,
   con variabili unsigned long.
 - In setup() randomSeed(analogRead(A0)).

COME PROVARLO
 [ ] Cinque manche normali: tempi plausibili (150-400 ms) e alla fine migliore
     e media.
 [ ] Premete prima del verde: LED rosso e "Falsa partenza!", la manche non
     conta.
 [ ] Tenete premuto dopo la misura: la manche successiva parte solo quando
     rilasciate.
 [ ] Premete in VIA: il tempo stampato non cambia se tenete premuto piu' a
     lungo.

COSA CONSEGNARE
 1. Il link al progetto Tinkercad, condiviso in sola lettura.
 2. Il codice completo, con un commento su ogni riga che riguarda i tempi.
 3. Il diagramma degli stati del gioco (ATTESA, VIA, FINE) con gli eventi che
    fanno passare da uno all'altro.
 4. Le risposte alle domande.

DOMANDE
 1. Perche' il cronometro si ferma alla pressione e non al rilascio, mentre
    la ripartenza avviene al rilascio? Che cosa misurereste agendo al
    rilascio?
 2. Con la versione 1, di quanto e' in ritardo il riconoscimento della
    pressione rispetto al contatto vero? E il rilascio?
 3. Riscrivete la gestione del pulsante con la versione 4 (schedulatore,
    TBASE = 50 ms). Quale errore massimo e quale errore medio si aggiungono al
    tempo di reazione misurato? E' accettabile per questo gioco?
 4. Uno spike di 2 ms a pulsante rilasciato, con la versione 1: che effetto ha
    in ATTESA? E in VIA? E' un problema di sicurezza o solo di gioco?
 5. Un buco di 3 ms del contatto mentre il giocatore tiene premuto in FINE
    puo' far partire una manche? Rispondete per la versione 1 e per la
    versione 4.
 6. Perche' un programma bloccante (con waitUntilInputLow() e delay() per
    l'attesa casuale) non potrebbe riconoscere le false partenze?
===============================================================================
*/

// ---------------------------------------------------------------------------
// SOLUZIONE - versione 1 su due fronti (millis(), t0, attivo)
// ---------------------------------------------------------------------------
const byte P1 = 2;                        // pull-down: premuto = HIGH
const byte VERDE = 11;
const byte ROSSO = 9;
const unsigned long TDEBOUNCE = 50;       // ms di basso continuo per il rilascio
const byte NMANCHE = 5;

enum { ATTESA, VIA, FINE };
byte statoGioco = ATTESA;

unsigned long t0 = 0;                     // ultima lettura alta del pulsante
bool attivo = false;                      // pulsante premuto (pressione riconosciuta)

unsigned long tStato = 0;                 // inizio dell'ATTESA o accensione del verde
unsigned long attesa = 0;                 // durata casuale dell'ATTESA (ms)
byte manche = 0;                          // manche valide giocate
unsigned long somma = 0;                  // somma dei tempi validi (ms)
unsigned long migliore = 0;               // tempo migliore (ms)

void nuovaManche() {
  digitalWrite(VERDE, LOW);
  digitalWrite(ROSSO, LOW);
  attesa = random(2000, 5001);            // fra 2 e 5 s
  tStato = millis();                      // l'attesa comincia adesso
  statoGioco = ATTESA;
}

void doOnRise() {
  // PRESSIONE: riconosciuta alla prima lettura alta, quindi subito
  if (statoGioco == ATTESA) {             // premuto prima del verde
    digitalWrite(ROSSO, HIGH);
    Serial.println("Falsa partenza!");
    statoGioco = FINE;
  } else if (statoGioco == VIA) {
    unsigned long tr = millis() - tStato; // tempo di reazione (ms)
    digitalWrite(VERDE, LOW);
    Serial.print("Tempo: ");
    Serial.print(tr);
    Serial.println(" ms");
    manche++;
    somma += tr;
    if (manche == 1 || tr < migliore) migliore = tr;
    if (manche == NMANCHE) {
      Serial.print("Migliore: ");
      Serial.print(migliore);
      Serial.print(" ms  Media: ");
      Serial.print(somma / NMANCHE);
      Serial.println(" ms");
      manche = 0;                         // si ricomincia da capo
      somma = 0;
    }
    statoGioco = FINE;
  }
  // in FINE la pressione non fa nulla
}

void doOnFall() {
  // RILASCIO: confermato da 50 ms di basso; in FINE fa partire la manche dopo
  if (statoGioco == FINE) nuovaManche();
}

void setup() {
  Serial.begin(9600);
  pinMode(P1, INPUT);
  pinMode(VERDE, OUTPUT);
  pinMode(ROSSO, OUTPUT);
  randomSeed(analogRead(A0));             // A0 scollegato: valore casuale
  nuovaManche();
}

void loop() {
  // --- compito 1: il pulsante
  if (digitalRead(P1) == HIGH) {
    if (!attivo) {                        // prima lettura alta: pressione
      attivo = true;
      doOnRise();
    }
    t0 = millis();                        // a ogni lettura alta il timer riparte
  } else if (attivo && millis() - t0 > TDEBOUNCE) {
    attivo = false;                       // basso da piu' di 50 ms: rilascio
    doOnFall();
  }

  // --- compito 2: il gioco
  if (statoGioco == ATTESA && millis() - tStato > attesa) {   // attesa finita
    digitalWrite(VERDE, HIGH);
    tStato = millis();                    // il cronometro parte adesso
    statoGioco = VIA;
  }
}

/*
===============================================================================
RISPOSTE ALLE DOMANDE
===============================================================================
1. Il tempo di reazione e' l'intervallo fra l'accensione del verde e
   l'istante in cui il dito chiude il contatto: va preso alla pressione, e
   con la versione 1 la pressione e' riconosciuta alla prima lettura alta,
   cioe' subito. Agendo al rilascio si misurerebbe il tempo di reazione PIU'
   il tempo per cui il giocatore tiene premuto PIU' i 50 ms di conferma: un
   numero che non dice nulla dei riflessi. La ripartenza invece non ha
   fretta: al rilascio si e' sicuri che il dito si e' alzato, quindi la
   nuova manche non parte con il pulsante gia' premuto (che sarebbe una
   falsa partenza immediata).

2. Pressione: riconosciuta alla prima lettura alta, quindi con un ritardo di
   un giro di loop() (meno di 1 ms); i rimbalzi della pressione non la
   ritardano, perche' basta il primo contatto. Rilascio: confermato 50 ms
   dopo l'ultimo rimbalzo, quando il pin e' stato LOW per 50 ms di fila.

3. Con lo schedulatore la pressione e' vista alla prima lettura dopo il
   contatto: le letture sono ogni 50 ms, quindi il ritardo va da 0 a 50 ms,
   in media 25 ms. L'errore si aggiunge al tempo di reazione e cambia da una
   manche all'altra. Su tempi di 150-400 ms e' un errore fino al 15-30%, e
   due giocatori che differiscono di 20 ms non si possono confrontare: per
   questo gioco non e' accettabile.

4. Lo spike a riposo e' preso per una pressione (la versione 1 non lo
   filtra):
   - in ATTESA: falsa partenza falsa, la manche e' persa senza colpa;
   - in VIA: il cronometro si ferma su un tempo inventato, magari un
     "record" di pochi ms;
   in entrambi i casi 50 ms dopo il "rilascio" viene confermato e parte la
   manche successiva. E' un problema di gioco, non di sicurezza: nessuna
   macchina si muove. Fa capire pero' perche' dove l'errore e' pericoloso si
   conferma anche la pressione.

5. Versione 1: no. Un buco di 3 ms e' piu' corto dei 50 ms richiesti per
   confermare il rilascio: alla lettura alta successiva t0 riparte, attivo
   resta true, doOnFall() non viene chiamata.
   Versione 4: si', se una lettura cade nel buco. Quella lettura vale LOW e
   la precedente HIGH: e' un fronte di discesa, doOnFall() fa partire la
   manche. Alla lettura dopo il pulsante e' di nuovo HIGH: fronte di salita,
   doOnRise() in ATTESA, quindi una falsa partenza mentre il giocatore non
   ha fatto nulla. La probabilita' e' circa 3/50 = 6% per ogni buco.

6. Un programma bloccante, durante delay(attesa), non legge il pulsante: chi
   preme prima del verde non viene visto. Peggio: se il pulsante e' ancora
   premuto quando l'attesa finisce, il programma lo trova subito HIGH e
   misura un tempo di reazione di 0 ms. Per vedere le false partenze il
   pulsante va letto DURANTE l'attesa, cioe' nello stesso loop() che
   controlla il tempo: serve un programma non bloccante.

VERIFICA IN SIMULAZIONE
Pressioni simulate 200, 220, 260, 280, 300 ms dopo il verde: tempi stampati
202, 220, 262, 280, 300 ms (i 2 ms in piu' sono il primo rimbalzo che
cade basso); falsa partenza riconosciuta e non contata; pressione tenuta 3 s:
la manche successiva parte solo dopo il rilascio; dopo 5 manche valide:
migliore 202 ms, media 252 ms.
*/
