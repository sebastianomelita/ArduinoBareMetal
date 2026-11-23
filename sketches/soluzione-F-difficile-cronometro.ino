/*
===============================================================================
CONSEGNA - ESERCIZIO F (DIFFICILE) - Cronometro con start/stop a toggle e azzeramento
===============================================================================
Classe terza - Sistemi e reti
Argomenti: pull-down e pull-up, le quattro situazioni critiche, polling non
bloccante su due fronti (versioni 1 e 4), prontezza e sicurezza.

Realizzate in Tinkercad un cronometro da gara: un pulsante fa partire e
fermare il tempo (toggle), un altro lo azzera. Il tempo si legge sul Monitor
seriale.

OBIETTIVO
In un cronometro il toggle di start/stop deve scattare alla PRESSIONE: il
giudice preme quando l'atleta taglia il traguardo, e un ritardo di 50 ms
sarebbe un errore di misura. L'azzeramento invece puo' aspettare il rilascio,
e anzi conviene. Servono quindi tutte e due le callback.

CIRCUITO IN TINKERCAD (Arduino Uno R3)
 Componente              Collegamento                      A cosa serve
 Pulsante START/STOP     tra il pin 2 e 5V                 premuto porta il pin a HIGH
 (P1)
 Resistenza 10 kOhm      tra il pin 2 e GND                pull-down di P1
 Pulsante RESET (P2)     tra il pin 3 e GND                premuto porta il pin a LOW
                         (pull-up interno, INPUT_PULLUP)
 LED verde               pin 11, con resistenza 220 Ohm    cronometro in marcia
 LED rosso               pin 9,  con resistenza 220 Ohm    cronometro fermo con un tempo
Il Monitor seriale va aperto a 9600 baud.

REQUISITI
 1. All'accensione il cronometro e' fermo e azzerato, LED spenti.
 2. Alla PRESSIONE di P1 il cronometro cambia stato (toggle): se era fermo
    riparte dal tempo accumulato, se era in marcia si ferma.
 3. In marcia il LED verde e' acceso e ogni 100 ms si stampa il tempo nel
    formato secondi,decimi (es. 12,3).
 4. Fermo con un tempo diverso da zero: LED rosso lampeggiante (250 ms) e si
    stampa una volta il tempo finale in millisecondi.
 5. Al RILASCIO di P2 il cronometro si azzera, ma solo se e' fermo; in marcia
    P2 non fa nulla.
 6. Ogni pressione produce una sola azione, anche se il contatto rimbalza.

VINCOLI SUL CODICE
 - I due pulsanti si gestiscono con la versione 1 della dispensa sul polling
   non bloccante su due fronti (millis(), t0, attivo), ognuno con le sue
   variabili.
 - P1: il toggle sta in doOnRise() (pressione); doOnFall() resta vuota.
 - P2 e' in pull-up: la pressione e' un fronte di discesa e il rilascio un
   fronte di salita. L'azzeramento sta nella callback del rilascio.
 - Il tempo si misura con due variabili unsigned long: l'istante di partenza
   e il tempo accumulato nelle marce precedenti.
 - loop() con compiti separati: P1, P2, stampa, LED. Nessun delay().

COME PROVARLO
 [ ] Premete P1: il tempo parte alla pressione, non al rilascio.
 [ ] Premete di nuovo: si ferma, il rosso lampeggia e si stampa il tempo.
 [ ] Premete ancora: riparte dal tempo accumulato, non da zero.
 [ ] P2 in marcia: nessun effetto. P2 da fermo: azzera al rilascio.
 [ ] Clic rapidi su P1: ognuno fa un solo cambio.

COSA CONSEGNARE
 1. Il link al progetto Tinkercad, condiviso in sola lettura.
 2. Il codice completo, con un commento su ogni riga che riguarda i tempi.
 3. Il diagramma degli stati del cronometro (AZZERATO, IN MARCIA, FERMO) con
    gli eventi che fanno passare da uno all'altro.
 4. Le risposte alle domande.

DOMANDE
 1. Perche' il toggle di start/stop sta alla pressione e l'azzeramento al
    rilascio? Che cosa misurerebbe il cronometro con il toggle al rilascio?
 2. Con la versione 1, il toggle alla pressione e' protetto dai rimbalzi alla
    pressione? E da quelli al rilascio? Spiegate quale parte del codice se ne
    occupa.
 3. Uno spike di 2 ms sul pin 2 a P1 rilasciato: che effetto ha sul
    cronometro in marcia? E' un rischio accettabile in una gara? Quale delle
    quattro situazioni critiche e'?
 4. Riscrivete la gestione di P1 con la versione 4 (schedulatore, TBASE = 50
    ms). Quale errore massimo si aggiunge a ogni partenza e a ogni arrivo?
    I due errori si annullano sempre nella differenza? Quale errore massimo
    puo' avere il tempo misurato?
 5. Proponete il valore minimo e massimo di TBASE per la versione 4, usando
    le ipotesi di progetto della dispensa.
===============================================================================
*/

// ---------------------------------------------------------------------------
// SOLUZIONE - due pulsanti con la versione 1 su due fronti
//   P1 pull-down: toggle start/stop alla PRESSIONE (fronte di salita)
//   P2 pull-up:   azzeramento al RILASCIO (fronte di salita)
// ---------------------------------------------------------------------------
const byte P1 = 2;                        // START/STOP, pull-down: premuto = HIGH
const byte P2 = 3;                        // RESET, INPUT_PULLUP: premuto = LOW
const byte VERDE = 11;
const byte ROSSO = 9;
const unsigned long TDEBOUNCE = 50;       // ms di livello di riposo per il rilascio

unsigned long t01 = 0, t02 = 0;           // ultima lettura "premuto" di P1 e P2
bool attivo1 = false, attivo2 = false;    // pulsante premuto, rilascio da confermare

bool marcia = false;                      // cronometro in marcia
unsigned long tStart = 0;                 // istante dell'ultima partenza
unsigned long accumulato = 0;             // ms accumulati nelle marce precedenti
bool daStampare = false;                  // tempo finale ancora da stampare
unsigned long tPrint = 0;                 // ultima stampa in marcia
unsigned long tBlink = 0;                 // ultimo cambio del LED rosso

unsigned long tempo() {
  // tempo totale: accumulato + marcia in corso
  return marcia ? accumulato + (millis() - tStart) : accumulato;
}

void doOnRise1() {
  // PRESSIONE di P1: toggle start/stop, subito
  if (!marcia) {
    tStart = millis();                    // partenza
    marcia = true;
  } else {
    accumulato += millis() - tStart;      // arrivo: si somma la marcia appena finita
    marcia = false;
    daStampare = true;
  }
}

void doOnFall1() {
  // RILASCIO di P1: non serve
}

void doOnFall2() {
  // PRESSIONE di P2 (in pull-up e' un fronte di discesa): non serve
}

void doOnRise2() {
  // RILASCIO di P2 (fronte di salita): azzera, ma solo da fermo
  if (!marcia) {
    accumulato = 0;
    Serial.println("Azzerato");
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(P1, INPUT);                     // pull-down esterno da 10 kOhm
  pinMode(P2, INPUT_PULLUP);
  pinMode(VERDE, OUTPUT);
  pinMode(ROSSO, OUTPUT);
}

void loop() {
  // --- compito 1: P1 (premuto = HIGH)
  if (digitalRead(P1) == HIGH) {
    if (!attivo1) {
      attivo1 = true;
      doOnRise1();                        // pressione alla prima lettura alta
    }
    t01 = millis();
  } else if (attivo1 && millis() - t01 > TDEBOUNCE) {
    attivo1 = false;
    doOnFall1();
  }

  // --- compito 2: P2 (premuto = LOW)
  if (digitalRead(P2) == LOW) {
    if (!attivo2) {
      attivo2 = true;
      doOnFall2();
    }
    t02 = millis();
  } else if (attivo2 && millis() - t02 > TDEBOUNCE) {
    attivo2 = false;                      // alto da piu' di 50 ms: rilascio
    doOnRise2();
  }

  // --- compito 3: stampa
  if (marcia) {
    if (millis() - tPrint >= 100) {       // ogni 100 ms
      tPrint = millis();
      unsigned long t = tempo();
      Serial.print(t / 1000);             // secondi
      Serial.print(",");
      Serial.println((t % 1000) / 100);   // decimi
    }
  } else if (daStampare) {
    Serial.print("Tempo finale: ");
    Serial.print(accumulato);
    Serial.println(" ms");
    daStampare = false;
  }

  // --- compito 4: LED
  digitalWrite(VERDE, marcia);
  if (!marcia && accumulato > 0) {        // fermo con un tempo: rosso lampeggiante
    if (millis() - tBlink > 250) {
      tBlink = millis();
      digitalWrite(ROSSO, !digitalRead(ROSSO));
    }
  } else {
    digitalWrite(ROSSO, LOW);
  }
}

/*
===============================================================================
RISPOSTE ALLE DOMANDE
===============================================================================
1. Il cronometro deve fermarsi nell'istante in cui il giudice preme: con la
   versione 1 la pressione e' vista alla prima lettura alta, quindi subito.
   Con il toggle al rilascio partenza e arrivo sarebbero spostati del tempo
   per cui il giudice tiene premuto (100-300 ms, ogni volta diverso) piu' i
   50 ms di conferma: errori di decimi di secondo, e diversi alla partenza e
   all'arrivo. L'azzeramento invece cancella il risultato: non c'e' fretta,
   e al rilascio si agisce solo dopo un gesto completo (pressione e
   rilascio confermato). E' la regola "fermarsi subito, ripartire con
   calma": l'azione urgente alla pressione, quella che si puo' pentire al
   rilascio.

2. Si', da tutti e due. Se ne occupa la stessa parte di codice: la variabile
   attivo1 e il timer t01. Dopo la prima lettura alta attivo1 e' true:
   - i rimbalzi della pressione sono letture basse di pochi ms; il rilascio
     non viene confermato perche' non passano 50 ms, e doOnRise1() non viene
     richiamata perche' attivo1 e' gia' true;
   - i rimbalzi del rilascio sono letture alte che fanno ripartire t01:
     attivo1 resta true, quindi non sono prese per una nuova pressione.
   doOnRise1() puo' essere chiamata di nuovo solo dopo 50 ms di basso
   continuo.

3. E' la situazione 4, spike a pulsante rilasciato. La versione 1 accetta la
   pressione alla prima lettura alta, quindi lo spike e' preso per una
   pressione: il cronometro in marcia si FERMA (arrivo falso), quello fermo
   riparte. In una gara non e' accettabile: il risultato e' falso e non si
   puo' rifare. Servirebbe una tecnica che conferma anche la pressione
   (come la tecnica 6 bloccante o il livello stabile), che la ritarda di
   50 ms. Per un cronometro quel ritardo e' innocuo, perche' e' uguale alla
   partenza e all'arrivo e si annulla nella differenza: e' un ritardo
   costante, non casuale.

4. Con lo schedulatore ogni pressione e' vista alla prima lettura dopo il
   contatto: un ritardo fra 0 e 50 ms, ogni volta diverso. Il tempo misurato
   e' (arrivo + e2) - (partenza + e1) = tempo vero + (e2 - e1). I due errori
   non si annullano, perche' sono casuali e indipendenti: e2 - e1 va da
   -50 a +50 ms, quindi il tempo misurato puo' sbagliare fino a circa
   mezzo decimo di secondo, in piu' o in meno. Con la versione 1 l'errore e'
   di un giro di loop(), meno di 1 ms.

5. Limite inferiore: TBASE piu' lungo dei rimbalzi piu' l'intervallo di
   lettura, circa 10 + 1 = 11 ms, altrimenti due letture cadono negli stessi
   rimbalzi. Limite superiore: l'evento vero piu' breve, circa 100 ms. Per
   un cronometro conviene stare vicino al limite inferiore, per esempio
   15-20 ms, perche' l'errore massimo di ogni misura e' proprio TBASE.

VERIFICA IN SIMULAZIONE
Partenza a 1000 ms, arrivo a 3500 ms: tempo finale 2499 ms (il ms mancante
e' un rimbalzo basso al primo contatto della partenza). Azzeramento in
marcia: ignorato. Ripartenza senza azzerare: 2499 + 1235 = 3734 ms
accumulati. Azzeramento da fermo: al rilascio, stampa "Azzerato".
*/
