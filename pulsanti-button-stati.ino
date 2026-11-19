/*
 * Due pulsanti gestiti con la struttura Button
 *
 *  - pulsante 1, rosso (GPIO 14), SENZA MEMORIA (momentaneo): il LED 1 rosso
 *    (GPIO 13) resta acceso finché il pulsante è premuto;
 *  - pulsante 2, verde (GPIO 23), CON MEMORIA (toggle): il LED 2 verde
 *    (GPIO 12) cambia stato a ogni pressione.
 *
 * Collegamento: pulsanti tra il pin e 3V3, resistenza di pull-down esterna
 * verso GND (pinMode INPUT). Il pin legge LOW a riposo e HIGH quando il
 * pulsante è premuto.
 */

#define BUTTON1_PIN 14    // pulsante momentaneo
#define BUTTON2_PIN 23    // pulsante toggle
#define LED1_PIN 13       // LED del pulsante momentaneo
#define LED2_PIN 12       // LED del pulsante toggle
#define DEBOUNCETIME 50   // ms: tempo di stabilità richiesto prima di accettare un cambio
                          // (va scelto in base alla qualità del pulsante: deve superare
                          //  la durata dei suoi rimbalzi e dei suoi disturbi)

/**
 * Struttura Button: debounce a LIVELLO STABILE, codifica "prima gli stati"
 *
 * Stessa macchina a stati del DEBOUNCER, ma lo stato è una variabile esplicita
 * (debState) e il codice esamina PRIMA lo stato e POI, dentro ogni stato,
 * gli ingressi:
 *  - STABILE (debState = false): il valore letto coincide con il livello accettato;
 *  - ATTESA DI CONFERMA (debState = true): è stato letto un livello diverso e
 *    se ne misura la durata.
 *
 * Un cambio viene segnalato solo quando il nuovo livello è rimasto costante per
 * debtime ms; ogni variazione letta (rimbalzo o disturbo) fa ripartire il
 * conteggio. Il prezzo è un ritardo di debtime ms, impercettibile per un
 * pulsante azionato a mano.
 *
 * Uso: chiamare changed() a ogni giro di loop(); dopo un true, val contiene
 * il nuovo livello stabile.
 */
struct Button {
  uint8_t pin;           // pin di Arduino collegato al pulsante
  uint8_t val0;          // ultimo valore letto (anche se instabile)
  unsigned long debtime; // tempo di stabilità richiesto, in millisecondi
  uint8_t val;           // valore letto nell'ultima chiamata
  uint8_t val00;         // ultimo livello stabile accettato
  unsigned long last;    // istante dell'ultima variazione letta
  bool debState;         // stato: false = STABILE, true = ATTESA DI CONFERMA

  bool changed() {
    bool chg = false;
    val = digitalRead(pin);

    if (!debState) {                  // STATO STABILE (val0 == val00)
      if (val != val0) {              // variazione: si attende la conferma
        val0 = val;
        last = millis();              // il conteggio della stabilità parte da qui
        debState = true;
      }
    } else {                          // STATO ATTESA DI CONFERMA (val0 != val00)
      if (val != val0) {              // rimbalzo o disturbo: il valore è tornato
        val0 = val;                   // uguale al livello accettato
        debState = false;             // si torna in STABILE senza segnalare nulla
                                      // (il timer ripartirà alla prossima variazione)
      } else if ((unsigned long)(millis() - last) > debtime) {
        val00 = val;                  // stabile da debtime ms: cambio vero
        debState = false;
        chg = true;
      }
    }
    return chg;
  }

  // Costruttore. Scelta: il livello iniziale si legge dal pin, così non va
  // indicato a mano e non si rischia un evento falso all'accensione
  // (vale sia con pull-down sia con pull-up).
  Button(uint8_t _pin, unsigned long _debtime = DEBOUNCETIME) {
    pin = _pin;
    debtime = _debtime;
    pinMode(pin, INPUT);              // prima si configura il pin come ingresso...
    val = digitalRead(pin);           // ...poi si legge il livello iniziale
    val0 = val;
    val00 = val;                      // il livello iniziale è quello accettato
    last = 0;
    debState = false;                 // si parte in STABILE
  }
};

Button buttonMomentary(BUTTON1_PIN);  // tempo di stabilità predefinito DEBOUNCETIME
Button buttonToggle(BUTTON2_PIN);

bool toggleState = false;   // stato memorizzato del pulsante con memoria

void setup() {
  Serial.begin(115200);
  // i pin dei pulsanti sono configurati dal costruttore (INPUT, pull-down esterno)
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
}

void loop() {
  // 1. PULSANTE SENZA MEMORIA (momentaneo)
  // Scelta: si reagisce a entrambi i cambi; il LED copia il livello stabile.
  if (buttonMomentary.changed()) {
    if (buttonMomentary.val == HIGH) {        // con pull-down, HIGH = premuto
      digitalWrite(LED1_PIN, HIGH);
      Serial.println("Pulsante 1 premuto - LED 1 acceso");
    } else {
      digitalWrite(LED1_PIN, LOW);
      Serial.println("Pulsante 1 rilasciato - LED 1 spento");
    }
  }

  // 2. PULSANTE CON MEMORIA (toggle)
  // Scelta: changed() sta prima dell'&& e viene chiamata a ogni giro, così il
  // debounce continua a lavorare anche sui rilasci; si agisce solo sulla pressione.
  if (buttonToggle.changed() && buttonToggle.val == HIGH) {
    toggleState = !toggleState;               // inverte lo stato memorizzato
    digitalWrite(LED2_PIN, toggleState);
    Serial.print("Pulsante 2 premuto - LED 2 ");
    Serial.println(toggleState ? "acceso" : "spento");
  }

  delay(1);  // solo per non bloccare il simulatore; il loop() deve restare veloce
}
