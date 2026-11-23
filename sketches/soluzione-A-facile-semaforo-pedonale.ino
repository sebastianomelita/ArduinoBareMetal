/*
===============================================================================
CONSEGNA - ESERCIZIO A (FACILE) - Semaforo pedonale a chiamata, versione bloccante
===============================================================================
Classe terza - Sistemi e reti
Argomenti: pull-down, rimbalzi, le quattro situazioni critiche, gestione dei
pulsanti con attese bloccanti (tecniche 1, 5 e 6).

Realizzate in Tinkercad un semaforo per un attraversamento pedonale. Il
semaforo delle auto resta verde finche' un pedone non preme e rilascia il
pulsante di chiamata; allora parte la sequenza che ferma le auto e fa passare
il pedone.

OBIETTIVO
Il programma fa una cosa alla volta (aspetta il pedone, poi esegue la
sequenza), quindi le attese bloccanti vanno bene: si usano delay() per le
fasi del semaforo e waitUntilInputLow() della dispensa per il pulsante.

CIRCUITO IN TINKERCAD (Arduino Uno R3)
 Componente               Collegamento                     A cosa serve
 Pulsante di chiamata     tra il pin 2 e 5V                premuto porta il pin 2 a HIGH
 Resistenza 10 kOhm       tra il pin 2 e GND               pull-down: a riposo il pin legge LOW
 LED verde auto           pin 11, con resistenza 220 Ohm   via libera alle auto
 LED giallo auto          pin 10, con resistenza 220 Ohm   attenzione
 LED rosso auto           pin 9,  con resistenza 220 Ohm   auto ferme
 LED verde pedoni         pin 6,  con resistenza 220 Ohm   il pedone puo' passare
 LED rosso pedoni         pin 5,  con resistenza 220 Ohm   il pedone aspetta
Nell'editor del codice scegliete la modalita' Testo.

REQUISITI
 1. A riposo: verde auto acceso, rosso pedoni acceso, gli altri spenti.
 2. La chiamata vale al RILASCIO del pulsante.
 3. Dopo il rilascio parte la sequenza:
      giallo auto 2 s  ->  rosso auto + verde pedoni 5 s  ->
      rosso auto + rosso pedoni 1 s  ->  di nuovo come al punto 1.
 4. Ogni pressione produce una sola chiamata, anche se il contatto rimbalza.
 5. Le pressioni fatte durante la sequenza vengono ignorate.

VINCOLI SUL CODICE
 - Usate la tecnica 5 della dispensa sulle attese bloccanti ("Letture continue
   con millis()"): nel loop(), se il pin e' HIGH si chiama doOnRise(), poi
   waitUntilInputLow(P1, 50), poi doOnFall().
 - doOnRise() e' vuota; doOnFall() esegue la sequenza del semaforo.
 - Ogni fase della sequenza in una funzione o in un blocco commentato.
 - Il tempo di antirimbalzo e' 50 ms.

COME PROVARLO
 [ ] All'avvio il semaforo e' nello stato di riposo.
 [ ] Premete e tenete premuto 3 s: la sequenza parte solo quando rilasciate.
 [ ] Un clic breve: parte una sola sequenza, non due.
 [ ] Premete il pulsante mentre il pedone ha il verde: alla fine non riparte
     una seconda sequenza.

COSA CONSEGNARE
 1. Il link al progetto Tinkercad, condiviso in sola lettura.
 2. Il codice completo e commentato.
 3. Le risposte alle domande.

DOMANDE
 1. Disegnate il segnale del pin 2 durante una pressione, con i rimbalzi alla
    pressione e al rilascio. Segnate l'istante in cui waitUntilInputLow()
    termina.
 2. Perche' serve la resistenza da 10 kOhm? Cosa leggerebbe il pin 2, a
    pulsante rilasciato, senza di essa?
 3. Se al posto della tecnica 5 usaste la tecnica 1 ("Una lettura ogni t ms"),
    cosa potrebbe succedere al rilascio? Che effetto avrebbe sul semaforo?
 4. Il punto 5 dei requisiti e' rispettato "gratis": perche'? (Pensate a cosa
    fa il programma durante i delay().) Una pressione fatta durante la
    sequenza puo' comunque far ripartire il semaforo? Quando?
 5. Il cavo del pulsante e' lungo 10 m e passa vicino ai cavi di un motore.
    Quale delle quattro situazioni critiche diventa probabile? Quale tecnica
    della dispensa usereste e perche'?
===============================================================================
*/

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
  pinMode(P1, INPUT);                   // pull-down esterno da 10 kOhm
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

/*
===============================================================================
RISPOSTE ALLE DOMANDE
===============================================================================
1. Diagramma (da disegnare): il pin 2 e' LOW a riposo; alla pressione
   rimbalza per qualche ms e poi resta HIGH; al rilascio rimbalza di nuovo e
   poi resta LOW. Il loop() entra al primo HIGH della pressione;
   waitUntilInputLow() termina 50 ms dopo l'ULTIMO rimbalzo alto del
   rilascio, perche' ogni lettura HIGH fa ripartire il conteggio. Solo allora
   parte la sequenza.

2. Senza la resistenza da 10 kOhm, a pulsante rilasciato il pin 2 non e'
   collegato a nulla (ingresso flottante): legge valori casuali, influenzati
   dai disturbi e perfino dalla mano vicina al cavo. Il semaforo partirebbe
   da solo. Il pull-down tiene il pin a LOW quando il pulsante e' aperto.

3. La tecnica 1 aspetta t ms e legge una sola volta. Se quella lettura cade
   in un istante basso dei rimbalzi del rilascio, l'attesa finisce e parte la
   sequenza; ma il pulsante sta ancora rimbalzando, quindi al giro successivo
   il loop() puo' leggere un HIGH e prenderlo per una nuova pressione
   (pressione fantasma). In questo programma il secondo "giro" pero' arriva
   dopo gli 8 s della sequenza, quando i rimbalzi sono finiti da un pezzo:
   l'effetto peggiore e' che la sequenza parte durante i rimbalzi del
   rilascio. Il danno vero della tecnica 1 si vede quando l'azione e' breve
   (per esempio un toggle), perche' allora il rimbalzo successivo viene
   letto subito.

4. Durante i delay() della sequenza il programma non legge il pin 2: le
   pressioni fatte in quel tempo non vengono viste. Pero' una pressione
   ANCORA IN CORSO quando la sequenza finisce viene vista: al primo giro di
   loop() dopo la sequenza il pin e' HIGH, il programma attende il rilascio e
   fa partire una seconda sequenza. (Verificato in simulazione: pressione da
   19,0 a 21,0 s, fine sequenza a 20,2 s, nuova sequenza al rilascio.)

5. Con un cavo lungo vicino a un motore diventano probabili gli SPIKE a
   pulsante rilasciato (e i buchi a pulsante premuto). La tecnica 5 filtra i
   buchi (servono 50 ms di LOW continuo) ma NON gli spike: un impulso HIGH di
   2 ms a riposo viene preso per una pressione e, 50 ms dopo, per un rilascio,
   quindi il semaforo ferma le auto senza nessun pedone (verificato in
   simulazione). Conviene la tecnica 6, che chiede 50 ms di HIGH fermo anche
   per la pressione: filtra tutte e quattro le situazioni. Il prezzo e' una
   pressione riconosciuta 50 ms dopo, irrilevante per un semaforo.
*/
