/*
===============================================================================
CONSEGNA - ESERCIZIO B (MEDIO) - Pannello con selettore di modo e prova lampade
===============================================================================
Classe terza - Sistemi e reti
Argomenti: pull-up e pull-down, pulsanti senza memoria, rimbalzi e ipotesi di
progetto, polling non bloccante su un solo fronte (versioni 1 e 4).

Molti pannelli di macchine hanno un tasto "prova lampade": finche' e' premuto
tutte le spie si accendono, cosi' l'operatore vede se qualcuna e' bruciata.
Realizzate in Tinkercad un pannello con tre spie, un tasto che sceglie il modo
di funzionamento e un tasto di prova lampade.

OBIETTIVO
Sullo stesso pannello convivono un pulsante SENZA memoria (prova lampade:
conta solo il livello, finche' e' premuto) e un pulsante CON memoria (il
selettore: ogni rilascio fa avanzare il modo). I due pulsanti sono collegati
in modo diverso, uno in pull-down e uno in pull-up.

CIRCUITO IN TINKERCAD (Arduino Uno R3)
 Componente               Collegamento                     A cosa serve
 Pulsante SELEZIONE (P1)  tra il pin 2 e GND               premuto porta il pin 2 a LOW
                          (pull-up interno, INPUT_PULLUP)
 Pulsante PROVA (P2)      tra il pin 3 e 5V                premuto porta il pin 3 a HIGH
 Resistenza 10 kOhm       tra il pin 3 e GND               pull-down di P2
 LED verde  (MANUALE)     pin 11, con resistenza 220 Ohm   spia del modo 0
 LED giallo (AUTOMATICO)  pin 10, con resistenza 220 Ohm   spia del modo 1
 LED rosso  (MANUTENZ.)   pin 9,  con resistenza 220 Ohm   spia del modo 2

REQUISITI
 1. All'accensione il modo e' MANUALE: acceso solo il LED verde.
 2. A ogni RILASCIO di P1 il modo avanza: MANUALE -> AUTOMATICO ->
    MANUTENZIONE -> MANUALE ... ed e' acceso solo il LED del modo corrente.
 3. Finche' P2 e' premuto i tre LED sono tutti accesi; al rilascio di P2 torna
    acceso solo il LED del modo corrente. P2 non cambia mai il modo.
 4. Ogni pressione di P1 fa avanzare il modo di un solo passo.
 5. Il programma non e' bloccante: P2 risponde anche mentre P1 e' premuto.

VINCOLI SUL CODICE
 - P1 si gestisce con la versione 4 della dispensa sul polling su un solo
   fronte (schedulatore non bloccante, TBASE = 50 ms, variabile precval).
   Attenzione: P1 e' in pull-up, quindi il rilascio e' un fronte di SALITA e
   la callback e' doOnRise().
 - doOnRise() cambia solo la variabile del modo (byte modo, 0..2).
 - P2 si legge a ogni giro di loop() con digitalRead(), SENZA debounce.
 - Un solo punto del codice decide quali LED accendere, in base a modo e a P2.
 - Nessun delay().

COME PROVARLO
 [ ] All'avvio e' acceso solo il verde.
 [ ] Tre clic su P1: giallo, rosso, di nuovo verde. Il cambio avviene al
     rilascio, non alla pressione.
 [ ] Tenete premuto P2: tutti accesi; rilasciate: torna il modo di prima.
 [ ] Tenete premuto P1 e intanto premete P2: la prova lampade funziona lo
     stesso.
 [ ] Clic rapidi su P1: ognuno fa un solo passo.

COSA CONSEGNARE
 1. Il link al progetto Tinkercad, condiviso in sola lettura.
 2. Il codice completo, con un commento su ogni riga che riguarda i tempi.
 3. Le risposte alle domande.

DOMANDE
 1. Compilate la tabella per P1 e per P2: livello a riposo, livello da
    premuto, fronte della pressione, fronte del rilascio.
 2. Perche' P2 (prova lampade) non ha bisogno di debounce, mentre P1 si'? Che
    cosa vede l'operatore se P2 rimbalza per 5 ms? E se P1 rimbalza senza
    debounce?
 3. Ipotesi di progetto: TBASE deve stare fra due limiti. Quali? Che cosa
    succederebbe con TBASE = 5 ms? E con TBASE = 300 ms?
 4. Con lo schedulatore uno spike di 2 ms a P1 rilasciato puo' far avanzare il
    modo? Sempre, mai o solo in certi casi? Spiegate.
 5. Riscrivete la gestione di P1 con la versione 1 della dispensa (millis() e
    variabile attivo, pull-up). Che cosa cambia nel ritardo con cui il modo
    avanza dopo il rilascio? Quale delle due versioni filtra il buco a
    pulsante premuto?
===============================================================================
*/

// ---------------------------------------------------------------------------
// SOLUZIONE - P1 con schedulatore su un solo fronte (versione 4, pull-up)
//             P2 senza memoria (pull-down, nessun debounce)
// ---------------------------------------------------------------------------
#define TBASE 50                          // ms fra due letture di P1: > rimbalzi

const byte P1 = 2;                        // SELEZIONE, INPUT_PULLUP: premuto = LOW
const byte P2 = 3;                        // PROVA LAMPADE, pull-down: premuto = HIGH
const byte LEDS[3] = {11, 10, 9};         // verde MANUALE, giallo AUTOMATICO, rosso MANUTENZIONE

byte modo = 0;                            // 0 manuale, 1 automatico, 2 manutenzione
unsigned long precm = 0;                  // istante dell'ultima lettura di P1
byte precval = HIGH;                      // lettura precedente di P1 (riposo = HIGH)
byte val;

void doOnRise() {
  // RILASCIO di P1 (in pull-up e' un fronte di SALITA): modo successivo
  modo = (modo + 1) % 3;
}

void setup() {
  pinMode(P1, INPUT_PULLUP);
  pinMode(P2, INPUT);                     // pull-down esterno da 10 kOhm
  for (byte i = 0; i < 3; i++) pinMode(LEDS[i], OUTPUT);
}

void loop() {
  // --- compito 1: P1, una lettura ogni TBASE ms
  if (millis() - precm >= TBASE) {        // passati TBASE ms dall'ultima lettura
    precm = millis();                     // istante di questa lettura
    val = digitalRead(P1);
    if (precval == LOW && val == HIGH) {  // prima premuto, ora rilasciato
      doOnRise();
    }
    precval = val;
  }

  // --- compito 2: P2 senza memoria + uscita, in un solo punto
  bool prova = (digitalRead(P2) == HIGH); // conta solo il livello di adesso
  for (byte i = 0; i < 3; i++) {
    digitalWrite(LEDS[i], (prova || i == modo) ? HIGH : LOW);
  }
}

/*
===============================================================================
RISPOSTE ALLE DOMANDE
===============================================================================
1.               riposo   premuto   pressione         rilascio
   P1 (pull-up)   HIGH     LOW      fronte di discesa  fronte di salita
   P2 (pull-down) LOW      HIGH     fronte di salita   fronte di discesa

2. P2 e' senza memoria: l'uscita dipende solo dal livello di ADESSO. Un
   rimbalzo di 5 ms fa accendere e spegnere i LED per qualche ms, un tempo
   che l'occhio non vede; finiti i rimbalzi l'uscita e' giusta e non resta
   traccia dell'errore. P1 invece ha memoria: ogni fronte cambia la
   variabile modo, e un cambio sbagliato resta. Senza debounce ogni
   rimbalzo di P1 sarebbe un fronte in piu': il modo avanzerebbe di un
   numero casuale di passi.

3. Il limite inferiore: TBASE deve essere piu' lungo dei rimbalzi (e dei
   disturbi), cosi' due letture consecutive non cadono mai entrambe dentro
   lo stesso treno di rimbalzi: circa 11 ms secondo la finestra di progetto
   della dispensa. Il limite superiore: TBASE deve essere piu' corto
   dell'evento vero piu' breve (una pressione o una pausa fra due clic,
   circa 100 ms), altrimenti un clic rapido puo' cadere fra due letture.
   - TBASE = 5 ms: due o tre letture cadono dentro i rimbalzi; una sequenza
     LOW, HIGH, LOW, HIGH produce piu' fronti di salita: il modo avanza di
     piu' passi.
   - TBASE = 300 ms: un clic di 150 ms puo' non essere mai letto, e il modo
     cambia fino a 300 ms dopo il rilascio, un ritardo che si vede.

4. Solo in certi casi. Con il pull-up lo spike a riposo e' un impulso LOW.
   Se nessuna lettura ci cade dentro, non succede nulla. Se una lettura lo
   vede, quella lettura vale LOW e la successiva HIGH: e' un fronte di
   salita, il modo avanza. La probabilita' e' circa durata/TBASE = 2/50 = 4%
   (in simulazione: 45 avanzamenti su 1000 spike).

5. Versione 1 con il pull-up:

     unsigned long t0 = 0;
     bool attivo = false;
     ...
     if (digitalRead(P1) == LOW) {           // premuto
       attivo = true;
       t0 = millis();
     } else if (attivo && millis() - t0 > 50) {
       attivo = false;                       // HIGH da piu' di 50 ms: rilascio
       doOnRise();
     }

   Ritardo: con la versione 4 il modo cambia alla prima lettura dopo la fine
   dei rimbalzi, cioe' da 0 a 50 ms dopo (in media 25), con la versione 1
   sempre 50 ms dopo l'ultimo rimbalzo.
   Buco a pulsante premuto (impulso HIGH breve): la versione 1 lo filtra
   sempre, perche' servono 50 ms di HIGH continuo; la versione 4 solo se
   nessuna lettura ci cade dentro, altrimenti lo prende per un rilascio e il
   modo avanza a meta' pressione.
*/
