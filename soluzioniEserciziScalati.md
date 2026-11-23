>[Torna all'indice](indexpulsanti.md)   >[consegne](consegneEserciziScalati.md) >[versione in Python](soluzioniEserciziScalati-mpy.md)

# Esercizi sui pulsanti a difficoltà scalata: soluzioni

Per ogni esercizio: un riassunto della consegna, il codice completo e le risposte alle domande. Ogni programma è stato provato in simulazione con rimbalzi, buchi e spike; i risultati delle prove sono in fondo alle risposte. Le consegne complete sono in [consegneEserciziScalati.md](consegneEserciziScalati.md).

## Prima serie: senza toggle

### Esercizio A (facile): Semaforo pedonale a chiamata, versione bloccante

Realizzate in Tinkercad un semaforo per un attraversamento pedonale. Il semaforo delle auto resta verde finché un pedone non preme e rilascia il pulsante di chiamata; allora parte la sequenza che ferma le auto e fa passare il pedone.

[Consegna completa](consegneEserciziScalati.md#esercizio-a-facile-semaforo-pedonale-a-chiamata-versione-bloccante)

**Codice**

```cpp
// ---------------------------------------------------------------------------
// SOLUZIONE - tecnica 5 bloccante (letture continue con millis())
// ---------------------------------------------------------------------------
const byte P1 = 2;            // pulsante di chiamata, pull-down: premuto = HIGH
const byte VERDE_AUTO  = 11;
const byte GIALLO_AUTO = 10;
const byte ROSSO_AUTO  = 9;
const byte VERDE_PED   = 6;
const byte ROSSO_PED   = 5;

void luci(byte va, byte ga, byte ra, byte vp, byte rp) {
  // imposta in un colpo solo tutti e cinque i LED
  digitalWrite(VERDE_AUTO, va);
  digitalWrite(GIALLO_AUTO, ga);
  digitalWrite(ROSSO_AUTO, ra);
  digitalWrite(VERDE_PED, vp);
  digitalWrite(ROSSO_PED, rp);
}

void riposo() {
  // auto: verde, pedoni: rosso
  luci(HIGH, LOW, LOW, LOW, HIGH);
}

void waitUntilInputLow(int btn, unsigned t) {
  // attende t ms di LOW continuo: ogni lettura HIGH (rimbalzo, disturbo o
  // pulsante ancora premuto) fa ripartire il conteggio
  unsigned long tl = millis();          // istante dell'ultima lettura HIGH
  while (millis() - tl <= t) {          // esce dopo t ms di LOW senza interruzioni
    if (digitalRead(btn) == HIGH) {
      tl = millis();                    // il conteggio riparte
    }
    delay(1);                           // una lettura al millisecondo
  }
}

void doOnRise() {
  // PRESSIONE (fronte di salita): qui non serve nulla
}

void doOnFall() {
  // RILASCIO (fronte di discesa): sequenza di attraversamento
  luci(LOW, HIGH, LOW, LOW, HIGH);      // giallo auto, rosso pedoni
  delay(2000);                          // fase gialla: 2 s
  luci(LOW, LOW, HIGH, HIGH, LOW);      // rosso auto, verde pedoni
  delay(5000);                          // attraversamento: 5 s
  luci(LOW, LOW, HIGH, LOW, HIGH);      // tutto rosso (sgombero)
  delay(1000);                          // sgombero: 1 s
  riposo();
}

void setup() {
  pinMode(P1, INPUT);                   // pull-down esterno da 10 kΩ
  pinMode(VERDE_AUTO, OUTPUT);
  pinMode(GIALLO_AUTO, OUTPUT);
  pinMode(ROSSO_AUTO, OUTPUT);
  pinMode(VERDE_PED, OUTPUT);
  pinMode(ROSSO_PED, OUTPUT);
  riposo();
}

void loop() {
  if (digitalRead(P1) == HIGH) {        // prima lettura alta: pressione
    doOnRise();
    waitUntilInputLow(P1, 50);          // rilascio: 50 ms di LOW continuo
    doOnFall();
  }
}
```

**Risposte alle domande**

1. Diagramma (da disegnare): il pin 2 è LOW a riposo; alla pressione rimbalza per qualche ms e poi resta HIGH; al rilascio rimbalza di nuovo e poi resta LOW. Il loop() entra al primo HIGH della pressione; waitUntilInputLow() termina 50 ms dopo l'ULTIMO rimbalzo alto del rilascio, perché ogni lettura HIGH fa ripartire il conteggio. Solo allora parte la sequenza.

2. Senza la resistenza da 10 kΩ, a pulsante rilasciato il pin 2 non è collegato a nulla (ingresso flottante): legge valori casuali, influenzati dai disturbi e perfino dalla mano vicina al cavo. Il semaforo partirebbe da solo. Il pull-down tiene il pin a LOW quando il pulsante è aperto.

3. La tecnica 1 aspetta t ms e legge una sola volta. Se quella lettura cade in un istante basso dei rimbalzi del rilascio, l'attesa finisce e parte la sequenza; ma il pulsante sta ancora rimbalzando, quindi al giro successivo il loop() può leggere un HIGH e prenderlo per una nuova pressione (pressione fantasma). In questo programma il secondo "giro" però arriva dopo gli 8 s della sequenza, quando i rimbalzi sono finiti da un pezzo: l'effetto peggiore è che la sequenza parte durante i rimbalzi del rilascio. Il danno vero della tecnica 1 si vede quando l'azione è breve (per esempio un toggle), perché allora il rimbalzo successivo viene letto subito.

4. Durante i delay() della sequenza il programma non legge il pin 2: le pressioni fatte in quel tempo non vengono viste. Però una pressione ANCORA IN CORSO quando la sequenza finisce viene vista: al primo giro di loop() dopo la sequenza il pin è HIGH, il programma attende il rilascio e fa partire una seconda sequenza. (Verificato in simulazione: pressione da 19,0 a 21,0 s, fine sequenza a 20,2 s, nuova sequenza al rilascio.)

5. Con un cavo lungo vicino a un motore diventano probabili gli SPIKE a pulsante rilasciato (e i buchi a pulsante premuto). La tecnica 5 filtra i buchi (servono 50 ms di LOW continuo) ma NON gli spike: un impulso HIGH di 2 ms a riposo viene preso per una pressione e, 50 ms dopo, per un rilascio, quindi il semaforo ferma le auto senza nessun pedone (verificato in simulazione). Conviene la tecnica 6, che chiede 50 ms di HIGH fermo anche per la pressione: filtra tutte e quattro le situazioni. Il prezzo è una pressione riconosciuta 50 ms dopo, irrilevante per un semaforo.

### Esercizio B (medio): Pannello con selettore di modo e prova lampade

Molti pannelli di macchine hanno un tasto "prova lampade": finché è premuto tutte le spie si accendono, così l'operatore vede se qualcuna è bruciata. Realizzate in Tinkercad un pannello con tre spie, un tasto che sceglie il modo di funzionamento e un tasto di prova lampade.

[Consegna completa](consegneEserciziScalati.md#esercizio-b-medio-pannello-con-selettore-di-modo-e-prova-lampade)

**Codice**

```cpp
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
  // RILASCIO di P1 (in pull-up è un fronte di SALITA): modo successivo
  modo = (modo + 1) % 3;
}

void setup() {
  pinMode(P1, INPUT_PULLUP);
  pinMode(P2, INPUT);                     // pull-down esterno da 10 kΩ
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
```

**Risposte alle domande**

1. Livelli e fronti dei due pulsanti:

   | | riposo | premuto | pressione | rilascio |
   | --- | --- | --- | --- | --- |
   | P1 (pull-up) | HIGH | LOW | fronte di discesa | fronte di salita |
   | P2 (pull-down) | LOW | HIGH | fronte di salita | fronte di discesa |

2. P2 è senza memoria: l'uscita dipende solo dal livello di ADESSO. Un rimbalzo di 5 ms fa accendere e spegnere i LED per qualche ms, un tempo che l'occhio non vede; finiti i rimbalzi l'uscita è giusta e non resta traccia dell'errore. P1 invece ha memoria: ogni fronte cambia la variabile modo, e un cambio sbagliato resta. Senza debounce ogni rimbalzo di P1 sarebbe un fronte in più: il modo avanzerebbe di un numero casuale di passi.

3. Il limite inferiore: TBASE deve essere più lungo dei rimbalzi (e dei disturbi), così due letture consecutive non cadono mai entrambe dentro lo stesso treno di rimbalzi: circa 11 ms secondo la finestra di progetto della dispensa. Il limite superiore: TBASE deve essere più corto dell'evento vero più breve (una pressione o una pausa fra due clic, circa 100 ms), altrimenti un clic rapido può cadere fra due letture.
- TBASE = 5 ms: due o tre letture cadono dentro i rimbalzi; una sequenza LOW, HIGH, LOW, HIGH produce più fronti di salita: il modo avanza di più passi.
- TBASE = 300 ms: un clic di 150 ms può non essere mai letto, e il modo cambia fino a 300 ms dopo il rilascio, un ritardo che si vede.

4. Solo in certi casi. Con il pull-up lo spike a riposo è un impulso LOW. Se nessuna lettura ci cade dentro, non succede nulla. Se una lettura lo vede, quella lettura vale LOW e la successiva HIGH: è un fronte di salita, il modo avanza. La probabilità è circa durata/TBASE = 2/50 = 4% (in simulazione: 45 avanzamenti su 1000 spike).

5. Versione 1 con il pull-up:

```cpp
unsigned long t0 = 0;
bool attivo = false;
...
if (digitalRead(P1) == LOW) {           // premuto
  attivo = true;
  t0 = millis();
} else if (attivo && millis() - t0 > 50) {
  attivo = false;                       // HIGH da più di 50 ms: rilascio
  doOnRise();
}
```

Ritardo: con la versione 4 il modo cambia alla prima lettura dopo la fine dei rimbalzi, cioè da 0 a 50 ms dopo (in media 25), con la versione 1 sempre 50 ms dopo l'ultimo rimbalzo. Buco a pulsante premuto (impulso HIGH breve): la versione 1 lo filtra sempre, perché servono 50 ms di HIGH continuo; la versione 4 solo se nessuna lettura ci cade dentro, altrimenti lo prende per un rilascio e il modo avanza a metà pressione.

### Esercizio C (difficile): Gioco dei riflessi, polling non bloccante su due fronti

Realizzate in Tinkercad un gioco che misura il tempo di reazione: dopo un'attesa casuale si accende un LED verde e il giocatore deve premere il pulsante il più presto possibile. Il tempo viene stampato sul Monitor seriale. Chi preme prima del verde fa una falsa partenza.

[Consegna completa](consegneEserciziScalati.md#esercizio-c-difficile-gioco-dei-riflessi-polling-non-bloccante-su-due-fronti)

**Codice**

```cpp
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
    attivo = false;                       // basso da più di 50 ms: rilascio
    doOnFall();
  }

  // --- compito 2: il gioco
  if (statoGioco == ATTESA && millis() - tStato > attesa) {   // attesa finita
    digitalWrite(VERDE, HIGH);
    tStato = millis();                    // il cronometro parte adesso
    statoGioco = VIA;
  }
}
```

**Risposte alle domande**

1. Il tempo di reazione è l'intervallo fra l'accensione del verde e l'istante in cui il dito chiude il contatto: va preso alla pressione, e con la versione 1 la pressione è riconosciuta alla prima lettura alta, cioè subito. Agendo al rilascio si misurerebbe il tempo di reazione PIU' il tempo per cui il giocatore tiene premuto PIU' i 50 ms di conferma: un numero che non dice nulla dei riflessi. La ripartenza invece non ha fretta: al rilascio si è sicuri che il dito si è alzato, quindi la nuova manche non parte con il pulsante già premuto (che sarebbe una falsa partenza immediata).

2. Pressione: riconosciuta alla prima lettura alta, quindi con un ritardo di un giro di loop() (meno di 1 ms); i rimbalzi della pressione non la ritardano, perché basta il primo contatto. Rilascio: confermato 50 ms dopo l'ultimo rimbalzo, quando il pin è stato LOW per 50 ms di fila.

3. Con lo schedulatore la pressione è vista alla prima lettura dopo il contatto: le letture sono ogni 50 ms, quindi il ritardo va da 0 a 50 ms, in media 25 ms. L'errore si aggiunge al tempo di reazione e cambia da una manche all'altra. Su tempi di 150-400 ms è un errore fino al 15-30%, e due giocatori che differiscono di 20 ms non si possono confrontare: per questo gioco non è accettabile.

4. Lo spike a riposo è preso per una pressione (la versione 1 non lo filtra):
- in ATTESA: falsa partenza falsa, la manche è persa senza colpa;
- in VIA: il cronometro si ferma su un tempo inventato, magari un "record" di pochi ms; in entrambi i casi 50 ms dopo il "rilascio" viene confermato e parte la manche successiva. È un problema di gioco, non di sicurezza: nessuna macchina si muove. Fa capire però perché dove l'errore è pericoloso si conferma anche la pressione.

5. Versione 1: no. Un buco di 3 ms è più corto dei 50 ms richiesti per confermare il rilascio: alla lettura alta successiva t0 riparte, attivo resta true, doOnFall() non viene chiamata. Versione 4: sì, se una lettura cade nel buco. Quella lettura vale LOW e la precedente HIGH: è un fronte di discesa, doOnFall() fa partire la manche. Alla lettura dopo il pulsante è di nuovo HIGH: fronte di salita, doOnRise() in ATTESA, quindi una falsa partenza mentre il giocatore non ha fatto nulla. La probabilità è circa 3/50 = 6% per ogni buco.

6. Un programma bloccante, durante delay(attesa), non legge il pulsante: chi preme prima del verde non viene visto. Peggio: se il pulsante è ancora premuto quando l'attesa finisce, il programma lo trova subito HIGH e misura un tempo di reazione di 0 ms. Per vedere le false partenze il pulsante va letto DURANTE l'attesa, cioè nello stesso loop() che controlla il tempo: serve un programma non bloccante.

**Verifica in simulazione**

Pressioni simulate 200, 220, 260, 280, 300 ms dopo il verde: tempi stampati 202, 220, 262, 280, 300 ms (i 2 ms in più sono il primo rimbalzo che cade basso); falsa partenza riconosciuta e non contata; pressione tenuta 3 s: la manche successiva parte solo dopo il rilascio; dopo 5 manche valide: migliore 202 ms, media 252 ms.

## Seconda serie: con il toggle

### Esercizio D (facile): Lampada con toggle e campanello, versione bloccante

Realizzate in Tinkercad l'ingresso di una casa: un pulsante accende e spegne la luce del portico (toggle), un secondo pulsante è il campanello, che suona solo finché è premuto.

[Consegna completa](consegneEserciziScalati.md#esercizio-d-facile-lampada-con-toggle-e-campanello-versione-bloccante)

**Codice**

```cpp
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
  pinMode(P1, INPUT);                     // pull-down esterno da 10 kΩ
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
```

**Risposte alle domande**

1. Livelli e fronti dei due pulsanti:

   | | riposo | premuto | pressione | rilascio |
   | --- | --- | --- | --- | --- |
   | P1 (pull-down) | LOW | HIGH | fronte di salita | fronte di discesa |
   | P2 (INPUT_PULLUP) | HIGH | LOW | fronte di discesa | fronte di salita |

2. Il campanello è un pulsante senza memoria: il buzzer segue il livello di adesso. Se P2 rimbalza per 5 ms, il suono si accende e spegne per 5 ms e nessuno se ne accorge; poi il livello si stabilizza e l'uscita è giusta. La luce ha memoria: ogni rilascio cambia lo stato, e un cambio in più resta. Senza debounce ogni rimbalzo sarebbe un rilascio in più e la luce cambierebbe un numero casuale di volte: a volte si accende, a volte no.

3. Con la tecnica 1 al rilascio una lettura può cadere in un istante basso dei rimbalzi: l'attesa finisce e la luce cambia. Il pulsante però sta ancora rimbalzando: al giro successivo il loop() legge un HIGH, lo prende per una nuova pressione e alla fine la luce cambia di nuovo (pressione fantasma). Chi usa la luce vede che ogni tanto il clic "non funziona": la luce si accende e si rispegne subito.

4. No. Finché P1 è premuto il programma è fermo dentro waitUntilInputLow(): il pin 3 non viene letto, quindi il buzzer resta bloccato nello stato in cui era quando si è premuto P1 (se taceva, tace; se suonava, continua a suonare anche dopo aver lasciato P2). Si sblocca al rilascio di P1. È un limite di tutte le tecniche bloccanti: mentre aspettano un pulsante non fanno nient'altro. Con la tecnica 6 andrebbe peggio: il loop() resta fermo anche in waitUntilInputHigh(), cioè sempre, a pulsante rilasciato, e il campanello non funzionerebbe mai.

5. Tecnica 5: sì. Il loop() legge il pin a ogni giro, vede l'impulso HIGH, lo prende per una pressione e 50 ms dopo conferma il rilascio: la luce cambia (verificato in simulazione). Tecnica 6: no, perché la pressione deve restare HIGH per 50 ms di fila e uno spike di 2 ms fa solo ripartire il conteggio. Con un cavo lungo e disturbato è più sicura la 6, ma in questo circuito bloccherebbe il campanello (risposta 4): servirebbe una tecnica non bloccante.

**Verifica in simulazione**

300 clic con rimbalzi: 300 cambi della luce; il buzzer suona finché P2 è premuto; uno spike di 2 ms sul pin 2 a riposo cambia la luce.

### Esercizio E (medio): Frecce di direzione: due toggle e un lampeggio

Realizzate in Tinkercad le frecce di direzione di un veicolo: un pulsante accende e spegne la freccia sinistra, un altro la destra. Le frecce lampeggiano e non possono essere accese tutte e due insieme.

[Consegna completa](consegneEserciziScalati.md#esercizio-e-medio-frecce-di-direzione-due-toggle-e-un-lampeggio)

**Codice**

```cpp
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
    attivoS = false;                      // basso da più di 50 ms: rilascio
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
```

**Risposte alle domande**

1. t0 e attivo sono la memoria del debounce di UN pulsante: "è premuto, rilascio da confermare" e "l'ultima volta che l'ho visto premuto". Con una sola coppia i due pulsanti si scambiano la memoria. Esempio, con il blocco di PS scritto prima di quello di PD: si preme PD, il suo blocco mette attivo = true e aggiorna t0; 50 ms dopo il rilascio il primo blocco a trovare "pin basso, attivo vero, 50 ms passati" è quello di PS, che chiama doOnFallS(): premendo la freccia destra si accende la sinistra. E se si premono tutti e due, il rilascio di uno resta nascosto finché l'altro è premuto.

2. Nelle callback: è il momento in cui succede l'evento ed è l'unico punto in cui le variabili sx e dx cambiano, quindi lo stato è sempre coerente (mai sx e dx vere insieme). Il lampeggio si limita a mostrare lo stato (LED = freccia attiva E fase accesa) e non deve conoscere le regole. Se la regola stesse nel lampeggio, sx e dx potrebbero essere vere insieme e bisognerebbe decidere chi vince.

3. Sì: una sola lettura temporizzata legge tutti e due i pin.

```cpp
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
```

Ritardo: con la versione 1 la freccia cambia 50 ms dopo l'ultimo rimbalzo del rilascio; con la versione 4 alla prima lettura dopo i rimbalzi, cioè fra 0 e 50 ms dopo (in media 25).

4. TBASE = 5 ms: due letture possono cadere dentro gli stessi rimbalzi del rilascio (che durano fino a circa 10 ms) e dare due fronti di discesa, cioè due toggle: la freccia non cambia. Il limite inferiore è circa 11 ms (rimbalzi + intervallo di lettura). TBASE = 300 ms: un clic rapido (pressione di 100-150 ms) può cadere tutto fra due letture e andare perso, e il cambio arriva fino a 300 ms dopo il rilascio. Il limite superiore è l'evento vero più breve, circa 100 ms.

5. Versione 1: no. Il buco dura 3 ms, meno dei 50 ms di basso richiesti; alla lettura alta successiva t0 riparte e il rilascio non viene confermato. Versione 4: dipende da dove cadono le letture. Se nessuna cade nel buco non succede nulla. Se una ci cade, vale LOW dopo un HIGH: fronte di discesa, la freccia cambia a metà pressione; al rilascio vero c'è un altro fronte di discesa e la freccia cambia di nuovo, quindi alla fine il clic sembra non aver fatto nulla. Probabilità circa 3/50 = 6%.

**Verifica in simulazione**

Clic su PS e PD alternati: una sola freccia attiva per volta; secondo clic sulla stessa freccia: tutto spento; PS tenuto premuto 3 s: la freccia già accesa continua a lampeggiare con mezzo periodo di 401 ms; tre clic rapidi: tre cambi.

### Esercizio F (difficile): Cronometro con start/stop a toggle e azzeramento

Realizzate in Tinkercad un cronometro da gara: un pulsante fa partire e fermare il tempo (toggle), un altro lo azzera. Il tempo si legge sul Monitor seriale.

[Consegna completa](consegneEserciziScalati.md#esercizio-f-difficile-cronometro-con-startstop-a-toggle-e-azzeramento)

**Codice**

```cpp
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
  // PRESSIONE di P2 (in pull-up è un fronte di discesa): non serve
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
  pinMode(P1, INPUT);                     // pull-down esterno da 10 kΩ
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
    attivo2 = false;                      // alto da più di 50 ms: rilascio
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
```

**Risposte alle domande**

1. Il cronometro deve fermarsi nell'istante in cui il giudice preme: con la versione 1 la pressione è vista alla prima lettura alta, quindi subito. Con il toggle al rilascio partenza e arrivo sarebbero spostati del tempo per cui il giudice tiene premuto (100-300 ms, ogni volta diverso) più i 50 ms di conferma: errori di decimi di secondo, e diversi alla partenza e all'arrivo. L'azzeramento invece cancella il risultato: non c'è fretta, e al rilascio si agisce solo dopo un gesto completo (pressione e rilascio confermato). È la regola "fermarsi subito, ripartire con calma": l'azione urgente alla pressione, quella che si può pentire al rilascio.

2. Sì, da tutti e due. Se ne occupa la stessa parte di codice: la variabile attivo1 e il timer t01. Dopo la prima lettura alta attivo1 è true:
- i rimbalzi della pressione sono letture basse di pochi ms; il rilascio non viene confermato perché non passano 50 ms, e doOnRise1() non viene richiamata perché attivo1 è già true;
- i rimbalzi del rilascio sono letture alte che fanno ripartire t01: attivo1 resta true, quindi non sono prese per una nuova pressione. doOnRise1() può essere chiamata di nuovo solo dopo 50 ms di basso continuo.

3. È la situazione 4, spike a pulsante rilasciato. La versione 1 accetta la pressione alla prima lettura alta, quindi lo spike è preso per una pressione: il cronometro in marcia si FERMA (arrivo falso), quello fermo riparte. In una gara non è accettabile: il risultato è falso e non si può rifare. Servirebbe una tecnica che conferma anche la pressione (come la tecnica 6 bloccante o il livello stabile), che la ritarda di 50 ms. Per un cronometro quel ritardo è innocuo, perché è uguale alla partenza e all'arrivo e si annulla nella differenza: è un ritardo costante, non casuale.

4. Con lo schedulatore ogni pressione è vista alla prima lettura dopo il contatto: un ritardo fra 0 e 50 ms, ogni volta diverso. Il tempo misurato è (arrivo + e2) - (partenza + e1) = tempo vero + (e2 - e1). I due errori non si annullano, perché sono casuali e indipendenti: e2 - e1 va da -50 a +50 ms, quindi il tempo misurato può sbagliare fino a circa mezzo decimo di secondo, in più o in meno. Con la versione 1 l'errore è di un giro di loop(), meno di 1 ms.

5. Limite inferiore: TBASE più lungo dei rimbalzi più l'intervallo di lettura, circa 10 + 1 = 11 ms, altrimenti due letture cadono negli stessi rimbalzi. Limite superiore: l'evento vero più breve, circa 100 ms. Per un cronometro conviene stare vicino al limite inferiore, per esempio 15-20 ms, perché l'errore massimo di ogni misura è proprio TBASE.

**Verifica in simulazione**

Partenza a 1000 ms, arrivo a 3500 ms: tempo finale 2499 ms (il ms mancante è un rimbalzo basso al primo contatto della partenza). Azzeramento in marcia: ignorato. Ripartenza senza azzerare: 2499 + 1235 = 3734 ms accumulati. Azzeramento da fermo: al rilascio, stampa "Azzerato".
