/*Realizzare un programma Arduino che dati due led, led1 e led2 ed un tasto, conta il numero di volte che 
si preme il tasto e si accende il led1 quando si raggiungono 10 pressioni e quando si raggiungono le 20 
si accende il led2, mentre quando si raggiungono le 30 si spegne tutto e si ricomincia da 0. Se è possibile, 
rendere, in qualche modo, le pressioni insensibili ai rimbalzi del tasto.
*/
#define TBASE  50               // periodo base in ms (> durata dei rimbalzi)
#define NBLINK (500 / TBASE)    // semiperiodo del lampeggio in tick (500 ms)
#define NSTEP  NBLINK           // lunghezza del ciclo: mcm dei periodi dei task

const byte led1 = 13;   // led associati alle porte
const byte led2 = 12;
const byte tasto = 2;   // pulsante in pull-down
int count = 0;          // numero di pressioni
bool blink1 = false;
bool blink2 = false;
unsigned long precm = 0;    // istante dell'ultimo tick
byte precval = LOW, val;    // campione precedente e attuale del tasto
byte step = 0;              // contatore circolare dei tick: 0 .. NSTEP-1

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
  if (millis() - precm >= TBASE) {      // un tick ogni TBASE ms
    precm = millis();
    step = (step + 1) % NSTEP;          // conteggio circolare, al massimo NSTEP-1

    // task 1: tasto, campionato a ogni tick (il campionamento fa da antirimbalzo)
    val = digitalRead(tasto);
    if (precval == LOW && val == HIGH) {         // fronte di salita: pressione
      doOnRise();
    } else if (precval == HIGH && val == LOW) {  // fronte di discesa: rilascio
      doOnFall();
    }
    precval = val;

    // task 2: lampeggio dei led ogni NBLINK tick (500 ms)
    if (step % NBLINK == 0) {
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
  }
  // qui altri compiti, mai bloccati
}
