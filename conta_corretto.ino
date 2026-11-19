/*Realizzare un programma Arduino che dati due led, led1 e led2 ed un tasto, conta il numero di volte che 
si preme il tasto e si accende il led1 quando si raggiungono 10 pressioni e quando si raggiungono le 20 
si accende il led2, mentre quando si raggiungono le 30 si spegne tutto e si ricomincia da 0. Se è possibile, 
rendere, in qualche modo, le pressioni insensibili ai rimbalzi del tasto.
*/
const byte led1 = 13;   // led associati alle porte
const byte led2 = 12;
const byte tasto = 2;   // pulsante in pull-down
int count = 0;          // numero di pressioni
bool blink1 = false;
bool blink2 = false;
unsigned long elapsed = 0;  // ultimo cambio dei led lampeggianti
bool armato = false;        // true = pressione in corso
unsigned long t0 = 0;       // ultima lettura "premuto"

void doOnRise() {
  // PRESSIONE (fronte di salita): qui vanno le azioni immediate alla pressione.
  // Esempio: conteggio delle pressioni.
  count++;
  if (count == 10) {
    blink1 = true;
    blink2 = false;
  } else if (count == 20) {
    blink1 = false;
    blink2 = true;
  } else if (count == 30) {
    blink1 = false;
    blink2 = false;
    count = 0;
  }
}

void doOnFall() {
  // RILASCIO (fronte di discesa): qui vanno le azioni al rilascio.
  // Lasciatela vuota se non serve.
}

void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(tasto, INPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
}

void loop() {
  // task 1: pulsante
  if (digitalRead(tasto) == HIGH) {
    if (!armato) {
      armato = true;
      doOnRise();
    }
    t0 = millis();              // ancora premuto: il conteggio riparte
  } else if (armato && millis() - t0 > 50) {
    armato = false;             // rilasciato da più di 50 ms
    doOnFall();
  }

  // task 2: lampeggio dei led ogni 500 ms
  if (millis() - elapsed > 500) {
    elapsed = millis();
    if (blink1) {
      digitalWrite(led1, !digitalRead(led1));
    } else {
      digitalWrite(led1, LOW);
    }
    if (blink2) {
      digitalWrite(led2, !digitalRead(led2));
    } else {
      digitalWrite(led2, LOW);
    }
  }
  delay(10);  // solo per il simulatore: si può togliere
}
