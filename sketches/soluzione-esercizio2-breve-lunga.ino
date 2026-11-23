/*
===============================================================================
CONSEGNA - ESERCIZIO 2: pressione breve e pressione lunga sullo stesso pulsante
===============================================================================
Con lo stesso circuito dell'esercizio 1 (pulsante sul pin 2 con pull-down da
10 kOhm, LED sul pin 12 con resistenza da 220 Ohm) lo stesso pulsante deve
distinguere due gesti, riconosciuti entrambi al RILASCIO:
 - pressione breve (meno di 1 s): avvia o ferma il lampeggio (toggle);
 - pressione lunga (1 s o piu'): cambia la velocita' del lampeggio e NON
   cambia lo stato acceso/fermo.

REQUISITI
 1. All'accensione il LED e' spento e la velocita' e' la piu' lenta.
 2. Le velocita' sono tre, in ciclo: lenta, media, veloce, poi di nuovo lenta.
    Mezzo periodo in ms: const unsigned long TBLINK[3] = {1000, 500, 200};
    la velocita' successiva si ottiene con vel = (vel + 1) % 3;
 3. A lampeggio fermo il LED resta spento.
 4. Ogni pressione produce una sola azione, anche se il contatto rimbalza o
    ha un breve disturbo mentre e' premuto.
 5. Il pulsante risponde subito anche mentre il LED lampeggia.

VINCOLI SUL CODICE
 - Nessun delay(): tempi solo con millis() e variabili unsigned long.
 - Stesso debounce dell'esercizio 1 (t0 = millis() a ogni lettura alta,
   rilascio valido dopo 50 ms di livello basso continuo).
 - doOnRise() memorizza l'istante della pressione in tPress.
 - doOnFall() calcola la durata come t0 - tPress e la confronta con la
   costante TLUNGA (1000 ms) per scegliere l'azione.
 - loop() con due compiti separati: prima il pulsante, poi il lampeggio.

DOMANDE DI RIFLESSIONE
 1. Perche' la durata si calcola con t0 - tPress e non con millis() - tPress?
    Di quanto sbaglierebbe la seconda formula?
 2. Perche' un breve disturbo (contatto che si apre per pochi ms) mentre il
    pulsante e' premuto non spezza la pressione in due?
 3. Come cambiereste il programma perche' la velocita' cambi gia' dopo 1 s di
    pressione, senza aspettare il rilascio, e una sola volta anche se si tiene
    premuto a lungo? (Suggerimento: una variabile bool in piu'.)
===============================================================================
*/

// ---------------------------------------------------------------------------
// SOLUZIONE
// ---------------------------------------------------------------------------
const byte PULSANTE = 2;                  // pulsante con pull-down: premuto = HIGH
const byte LED = 12;                      // LED con resistenza da 220 Ohm
const unsigned long TDEBOUNCE = 50;       // ms di livello basso continuo per confermare il rilascio
const unsigned long TLUNGA = 1000;        // soglia fra pressione breve e lunga (ms)
const unsigned long TBLINK[3] = {1000, 500, 200};   // mezzo periodo: lenta, media, veloce

bool attivo = false;          // true dalla prima lettura alta fino al rilascio confermato
unsigned long t0 = 0;         // istante dell'ultima lettura alta del pulsante
unsigned long tPress = 0;     // istante della pressione
bool lampeggio = false;       // true = il LED lampeggia
byte vel = 0;                 // indice della velocita': 0 lenta, 1 media, 2 veloce
unsigned long tblink = 0;     // istante dell'ultimo cambio del LED

void doOnRise() {
  // PRESSIONE: si memorizza l'istante in cui e' cominciata
  tPress = millis();
}

void doOnFall() {
  // RILASCIO: durata = ultimo istante in cui il pulsante era premuto - inizio
  unsigned long durata = t0 - tPress;
  if (durata >= TLUNGA) {
    vel = (vel + 1) % 3;        // pressione lunga: velocita' successiva
  } else {
    lampeggio = !lampeggio;     // pressione breve: avvia o ferma il lampeggio
  }
}

void setup() {
  pinMode(PULSANTE, INPUT);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);       // all'accensione LED spento, velocita' lenta (vel = 0)
}

void loop() {
  // --- compito 1: il pulsante (debounce non bloccante)
  if (digitalRead(PULSANTE) == HIGH) {
    if (!attivo) {                         // prima lettura alta: pressione riconosciuta
      attivo = true;
      doOnRise();
    }
    t0 = millis();                         // a ogni lettura alta il timer riparte
  } else if (attivo && millis() - t0 > TDEBOUNCE) {
    attivo = false;                        // basso da piu' di 50 ms: rilascio confermato
    doOnFall();
  }

  // --- compito 2: il lampeggio
  if (lampeggio) {
    if (millis() - tblink > TBLINK[vel]) { // passato il mezzo periodo della velocita' scelta
      tblink = millis();
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
1. t0 - tPress invece di millis() - tPress.
   doOnFall() viene chiamata quando il rilascio e' CONFERMATO, cioe' dopo piu'
   di 50 ms di livello basso. In quel momento millis() vale circa t0 + 51:
   millis() - tPress sovrastimerebbe la durata di circa 50 ms (la finestra di
   debounce). t0 invece e' l'ultimo istante in cui il pulsante era premuto,
   quindi t0 - tPress e' la durata reale della pressione.
   Con la formula sbagliata una pressione di circa 950 ms verrebbe gia'
   classificata come lunga.

2. Il disturbo non spezza la pressione.
   Un buco di pochi ms rende il pin basso per meno di 50 ms: alla lettura alta
   successiva t0 riparte, quindi la condizione millis() - t0 > 50 non diventa
   mai vera e il rilascio non viene confermato. attivo resta true, doOnFall()
   non viene chiamata e doOnRise() non viene richiamata: tPress resta quello
   dell'inizio vero della pressione e la durata misurata e' quella intera.

3. Cambio di velocita' durante la pressione, una sola volta.
   Si aggiunge bool lungaFatta, azzerata alla pressione. Nel loop(), dopo il
   compito del pulsante, se il pulsante e' ancora premuto ed e' passato
   TLUNGA, si cambia velocita' e si alza lungaFatta; al rilascio il toggle
   avviene solo se la pressione lunga non c'e' stata:

     bool lungaFatta = false;

     void doOnRise() {
       tPress = millis();
       lungaFatta = false;               // nuova pressione: nessuna azione lunga ancora
     }

     void doOnFall() {
       if (!lungaFatta) lampeggio = !lampeggio;   // solo le pressioni brevi fanno toggle
     }

     // nel loop(), dopo il compito del pulsante:
     if (attivo && !lungaFatta && t0 - tPress >= TLUNGA) {
       vel = (vel + 1) % 3;              // cambia una sola volta per pressione
       lungaFatta = true;
     }

   Anche qui si usa t0 - tPress e non millis() - tPress: durante un buco del
   contatto t0 resta fermo, quindi il tempo non avanza mentre il pulsante e'
   aperto. La pressione lunga diventa pronta (azione dopo 1 s), quella breve
   resta sul rilascio.

VERIFICA
Il codice e' stato provato in simulazione con pressioni rumorose (rimbalzi di
1-8 ms e un buco di 2 ms a meta' pressione):
 - clic breve: parte il lampeggio a 1001 ms di mezzo periodo;
 - tre pressioni lunghe: 501, 201, poi di nuovo 1001 ms, senza fermare il
   lampeggio;
 - clic breve: lampeggio fermo, LED spento;
 - pressione lunga da fermo: cambia la velocita' ma il LED resta spento;
 - pressione di 1000 ms: lunga; pressione di 990 ms: breve.
Anche la variante della domanda 3 e' stata provata: la velocita' cambia dopo
1 s di pressione, una sola volta in 3 s, e al rilascio non c'e' toggle.
*/
