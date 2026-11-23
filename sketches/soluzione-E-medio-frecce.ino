/*
===============================================================================
CONSEGNA - ESERCIZIO E (MEDIO) - Frecce di direzione: due toggle e un lampeggio
===============================================================================
Classe terza - Sistemi e reti
Argomenti: pull-down, rimbalzi e ipotesi di progetto, polling non bloccante
su un solo fronte (versioni 1 e 4).

Realizzate in Tinkercad le frecce di direzione di un veicolo: un pulsante
accende e spegne la freccia sinistra, un altro la destra. Le frecce
lampeggiano e non possono essere accese tutte e due insieme.

OBIETTIVO
Gestire due pulsanti toggle e un lampeggio nello stesso loop(), senza che un
compito blocchi gli altri. Ogni pulsante ha le sue variabili di debounce.

CIRCUITO IN TINKERCAD (Arduino Uno R3)
 Componente               Collegamento                     A cosa serve
 Pulsante SINISTRA (PS)   tra il pin 2 e 5V                premuto porta il pin a HIGH
 Resistenza 10 kOhm       tra il pin 2 e GND               pull-down di PS
 Pulsante DESTRA (PD)     tra il pin 3 e 5V                premuto porta il pin a HIGH
 Resistenza 10 kOhm       tra il pin 3 e GND               pull-down di PD
 LED arancione SINISTRA   pin 11, con resistenza 220 Ohm   freccia sinistra
 LED arancione DESTRA     pin 10, con resistenza 220 Ohm   freccia destra

REQUISITI
 1. All'accensione le frecce sono spente.
 2. A ogni RILASCIO di PS la freccia sinistra cambia stato (toggle); lo
    stesso per PD con la freccia destra.
 3. Accendere una freccia spegne l'altra: le due frecce non sono mai attive
    insieme.
 4. La freccia attiva lampeggia: 400 ms accesa, 400 ms spenta. Quella spenta
    resta spenta.
 5. Ogni pressione produce un solo cambio, anche se il contatto rimbalza.
 6. Nessun pulsante blocca l'altro ne' il lampeggio.

VINCOLI SUL CODICE
 - I due pulsanti si gestiscono con la versione 1 della dispensa sul polling
   su un solo fronte (millis(), t0, attivo, rilascio dopo 50 ms di basso):
   ogni pulsante ha le sue variabili (t0S, attivoS, t0D, attivoD).
 - Le callback doOnFallS() e doOnFallD() cambiano solo le variabili bool
   delle frecce; e' il compito del lampeggio a comandare i LED.
 - loop() con tre compiti separati: pulsante sinistro, pulsante destro,
   lampeggio.
 - Nessun delay(); differenze di tempo come millis() - t > durata, con
   variabili unsigned long.

COME PROVARLO
 [ ] Clic su PS: lampeggia la sinistra; clic su PD: si spegne la sinistra e
     lampeggia la destra.
 [ ] Clic su PD con la destra accesa: tutto spento.
 [ ] Tenete premuto PS: non succede nulla finche' non rilasciate, e intanto
     la freccia gia' accesa continua a lampeggiare regolare.
 [ ] Clic rapidi: ogni clic fa un solo cambio.

COSA CONSEGNARE
 1. Il link al progetto Tinkercad, condiviso in sola lettura.
 2. Il codice completo, con un commento su ogni riga che riguarda i tempi.
 3. Le risposte alle domande.

DOMANDE
 1. Perche' ogni pulsante deve avere le sue variabili t0 e attivo? Cosa
    succederebbe usandone una sola coppia per tutti e due?
 2. Dove avete messo la regola "mai due frecce insieme": nelle callback o nel
    lampeggio? Perche' e' meglio li'?
 3. Riscrivete la gestione dei due pulsanti con la versione 4 (schedulatore,
    TBASE = 50 ms): bastano una sola lettura temporizzata e due variabili
    precval? Che cosa cambia nel ritardo fra il rilascio e il cambio della
    freccia?
 4. Ipotesi di progetto: perche' TBASE non puo' essere 5 ms? E perche' non
    puo' essere 300 ms?
 5. Un buco di 3 ms mentre PS e' premuto: con la versione 1 cambia la freccia?
    E con la versione 4? Spiegate con un disegno del segnale e delle letture.
===============================================================================
*/

// ---------------------------------------------------------------------------
// SOLUZIONE - due pulsanti con la versione 1 su un solo fronte + lampeggio
// ---------------------------------------------------------------------------
const byte PS = 2;                        // SINISTRA, pull-down: premuto = HIGH
const byte PD = 3;                        // DESTRA,   pull-down: premuto = HIGH
const byte LED_S = 11;
const byte LED_D = 10;
const unsigned long TDEBOUNCE = 50;       // ms di basso continuo per il rilascio
const unsigned long TBLINK = 400;         // ms accesa e ms spenta

unsigned long t0S = 0, t0D = 0;           // ultima lettura alta di ogni pulsante
bool attivoS = false, attivoD = false;    // pulsante premuto, rilascio da confermare
bool sx = false, dx = false;              // frecce attive
bool fase = LOW;                          // fase del lampeggio, comune alle due frecce
unsigned long tblink = 0;                 // ultimo cambio di fase

void doOnFallS() {
  // RILASCIO di PS: toggle della sinistra; se si accende, la destra si spegne
  sx = !sx;
  if (sx) dx = false;
}

void doOnFallD() {
  // RILASCIO di PD: toggle della destra; se si accende, la sinistra si spegne
  dx = !dx;
  if (dx) sx = false;
}

void setup() {
  pinMode(PS, INPUT);
  pinMode(PD, INPUT);
  pinMode(LED_S, OUTPUT);
  pinMode(LED_D, OUTPUT);
}

void loop() {
  // --- compito 1: pulsante sinistro
  if (digitalRead(PS) == HIGH) {
    attivoS = true;
    t0S = millis();                       // a ogni lettura alta il timer riparte
  } else if (attivoS && millis() - t0S > TDEBOUNCE) {
    attivoS = false;                      // basso da piu' di 50 ms: rilascio
    doOnFallS();
  }

  // --- compito 2: pulsante destro
  if (digitalRead(PD) == HIGH) {
    attivoD = true;
    t0D = millis();
  } else if (attivoD && millis() - t0D > TDEBOUNCE) {
    attivoD = false;
    doOnFallD();
  }

  // --- compito 3: lampeggio
  if (millis() - tblink > TBLINK) {       // passati 400 ms dall'ultimo cambio
    tblink = millis();
    fase = !fase;
  }
  digitalWrite(LED_S, sx && fase);        // una freccia spenta resta spenta
  digitalWrite(LED_D, dx && fase);
}

/*
===============================================================================
RISPOSTE ALLE DOMANDE
===============================================================================
1. t0 e attivo sono la memoria del debounce di UN pulsante: "e' premuto,
   rilascio da confermare" e "l'ultima volta che l'ho visto premuto". Con
   una sola coppia i due pulsanti si scambiano la memoria. Esempio, con il
   blocco di PS scritto prima di quello di PD: si preme PD, il suo blocco
   mette attivo = true e aggiorna t0; 50 ms dopo il rilascio il primo blocco
   a trovare "pin basso, attivo vero, 50 ms passati" e' quello di PS, che
   chiama doOnFallS(): premendo la freccia destra si accende la sinistra.
   E se si premono tutti e due, il rilascio di uno resta nascosto finche'
   l'altro e' premuto.

2. Nelle callback: e' il momento in cui succede l'evento ed e' l'unico
   punto in cui le variabili sx e dx cambiano, quindi lo stato e' sempre
   coerente (mai sx e dx vere insieme). Il lampeggio si limita a mostrare lo
   stato (LED = freccia attiva E fase accesa) e non deve conoscere le
   regole. Se la regola stesse nel lampeggio, sx e dx potrebbero essere
   vere insieme e bisognerebbe decidere chi vince.

3. Si': una sola lettura temporizzata legge tutti e due i pin.

     #define TBASE 50
     unsigned long precm = 0;
     byte precS = LOW, precD = LOW, valS, valD;
     ...
     if (millis() - precm >= TBASE) {
       precm = millis();
       valS = digitalRead(PS);
       valD = digitalRead(PD);
       if (precS == HIGH && valS == LOW) doOnFallS();
       if (precD == HIGH && valD == LOW) doOnFallD();
       precS = valS;
       precD = valD;
     }

   Ritardo: con la versione 1 la freccia cambia 50 ms dopo l'ultimo
   rimbalzo del rilascio; con la versione 4 alla prima lettura dopo i
   rimbalzi, cioe' fra 0 e 50 ms dopo (in media 25).

4. TBASE = 5 ms: due letture possono cadere dentro gli stessi rimbalzi del
   rilascio (che durano fino a circa 10 ms) e dare due fronti di discesa,
   cioe' due toggle: la freccia non cambia. Il limite inferiore e' circa
   11 ms (rimbalzi + intervallo di lettura). TBASE = 300 ms: un clic rapido
   (pressione di 100-150 ms) puo' cadere tutto fra due letture e andare
   perso, e il cambio arriva fino a 300 ms dopo il rilascio. Il limite
   superiore e' l'evento vero piu' breve, circa 100 ms.

5. Versione 1: no. Il buco dura 3 ms, meno dei 50 ms di basso richiesti; alla
   lettura alta successiva t0 riparte e il rilascio non viene confermato.
   Versione 4: dipende da dove cadono le letture. Se nessuna cade nel buco
   non succede nulla. Se una ci cade, vale LOW dopo un HIGH: fronte di
   discesa, la freccia cambia a meta' pressione; al rilascio vero c'e' un
   altro fronte di discesa e la freccia cambia di nuovo, quindi alla fine il
   clic sembra non aver fatto nulla. Probabilita' circa 3/50 = 6%.

VERIFICA IN SIMULAZIONE
Clic su PS e PD alternati: una sola freccia attiva per volta; secondo clic
sulla stessa freccia: tutto spento; PS tenuto premuto 3 s: la freccia gia'
accesa continua a lampeggiare con mezzo periodo di 401 ms; tre clic rapidi:
tre cambi.
*/
