/*Realizzare un programma Arduino che dati due led, led1 e led2 ed un tasto, conta il numero di volte che 
si preme il tasto e si accende il led1 quando si raggiungono 10 pressioni e quando si raggiungono le 20 
si accende il led2, mentre quando si raggiungono le 30 si spegne tutto e si ricomincia da 0. Se è possibile, 
rendere, in qualche modo, le pressioni insensibili ai rimbalzi del tasto.
*/
const byte led1 = 13;   // led associati alle porte
const byte led2 = 12;
const byte tasto = 2;   // pulsante in pull-down
int count = 0;          // numero di pressioni
bool blinkon1 = false;
bool blinkon2 = false;

// attesa bloccante del rilascio, come in urutils.h: dopo il primo LOW attende
// altri t ms e ricontrolla, quindi esce solo con un rilascio confermato
void waitUntilInputLow(int btn, unsigned t) {
  do {
    do {
      delay(t);
    } while (digitalRead(btn) != LOW);
    delay(t);  // fine rimbalzi
  } while (digitalRead(btn) != LOW);
}

void doOnRise() {
  // PRESSIONE (fronte di salita): qui vanno le azioni immediate alla pressione.
  // Lasciatela vuota se non serve.
}

void doOnFall() {
  // RILASCIO (fronte di discesa): qui vanno le azioni al rilascio.
  // Esempio: conteggio delle pressioni.
  count++;
  if (count == 10) {
    blinkon1 = true;
  } else if (count == 20) {
    blinkon1 = false;
    blinkon2 = true;
  } else if (count == 30) {
    blinkon1 = false;
    blinkon2 = false;
    count = 0;
  }
}

void setup() {
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(tasto, INPUT);
  digitalWrite(led1, LOW);  // nel pull-down è LOW
  digitalWrite(led2, LOW);
}

void loop() {
  // task 1: tasto (bloccante finché è premuto)
  if (digitalRead(tasto) == HIGH) {   // fronte di salita
    doOnRise();
    waitUntilInputLow(tasto, 50);     // attesa del rilascio confermato
    doOnFall();                       // fronte di discesa
  }

  // task 2: lampeggio (bloccante: con un led acceso il loop() si ferma 500 ms)
  if (blinkon1) {
    digitalWrite(led1, !digitalRead(led1));
    delay(500);
  } else {
    digitalWrite(led1, LOW);
  }
  if (blinkon2) {
    digitalWrite(led2, !digitalRead(led2));
    delay(500);
  } else {
    digitalWrite(led2, LOW);
  }
  delay(10);  // solo per il simulatore: si può togliere
}
