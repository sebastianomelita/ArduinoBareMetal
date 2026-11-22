>[Torna all'indice](indexpulsanti.md) >[Consegne](consegneEserciziPulsanti.md)


# Esercizi sui pulsanti - Soluzioni

Le soluzioni usano Arduino Uno con il pulsante in **pull-down**. Il codice è stato provato in un
simulatore con rimbalzi casuali da 1 a 10 ms, buchi di 2 ms a pulsante premuto e spike di 2 ms a
pulsante rilasciato. I risultati sono riportati con ogni soluzione.

## 1. Tempo di pressione

### Scelta della tecnica

Serve il **livello stabile in due direzioni** (`changed()`, sezione 5 di
[pulsanti-rilascio.md](pulsanti-rilascio.md)), per tre motivi:

- conferma pressione e rilascio con **lo stesso ritardo** di 50 ms, quindi nella differenza fra i due
  istanti il ritardo si annulla;
- scarta da sola le pressioni più brevi di 50 ms, come chiede la consegna: un tocco non arriva
  nemmeno a `doOnRise()`, quindi non spegne i LED delle fasce;
- uno spike a riposo non produce una misura falsa, cioè una dose di zucchero non richiesta.

### Programma

```cpp
const byte P1  = 2;
const byte LED = 13;         // LED di pressione
const byte LED_VERDE  = 11;  // poco zucchero
const byte LED_GIALLO = 10;  // medio
const byte LED_ROSSO  = 9;   // molto zucchero
byte val0  = LOW;        // ultima lettura
byte val00 = LOW;        // livello accettato
unsigned long last = 0;  // istante dell'ultima variazione
unsigned long tPress = 0;  // istante della pressione confermata

void fascia(byte verde, byte giallo, byte rosso) {
  digitalWrite(LED_VERDE, verde);
  digitalWrite(LED_GIALLO, giallo);
  digitalWrite(LED_ROSSO, rosso);
}

bool changed() {
  // vero solo quando il pin ha un
  // nuovo livello fermo da più di 50 ms
  byte val = digitalRead(P1);
  if (val != val0) {
    val0 = val;
    last = millis();
  } else if (val != val00 && millis() - last > 50) {
    val00 = val;
    return true;
  }
  return false;
}

void doOnRise() {
  // PRESSIONE confermata: si registra l'istante
  tPress = millis();
  digitalWrite(LED, HIGH);
  fascia(LOW, LOW, LOW);   // nuova misura: fasce spente
}

void doOnFall() {
  // RILASCIO confermato: durata e fascia
  digitalWrite(LED, LOW);
  unsigned long durata = millis() - tPress;
  Serial.print("Durata: ");
  Serial.print(durata);
  Serial.print(" ms, ");
  if (durata < 500) {
    Serial.println("poco zucchero");
    fascia(HIGH, LOW, LOW);
  } else if (durata <= 1500) {
    Serial.println("medio");
    fascia(LOW, HIGH, LOW);
  } else {
    Serial.println("molto zucchero");
    fascia(LOW, LOW, HIGH);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(P1, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_GIALLO, OUTPUT);
  pinMode(LED_ROSSO, OUTPUT);
}

void loop() {
  if (changed()) {
    if (val00 == HIGH) {
      doOnRise();
    } else {
      doOnFall();
    }
  }
}
```

### Risposte

**1. Diagramma temporale.** La pressione vera inizia al primo rimbalzo. Il programma la conferma
50 ms dopo l'ultimo rimbalzo della pressione. Il rilascio vero inizia al primo rimbalzo del rilascio,
e il programma lo conferma 50 ms dopo l'ultimo. La durata vera va dall'inizio della pressione
all'inizio del rilascio.

**4. Errore di misura.**

| Tecnica | Pressione riconosciuta | Rilascio riconosciuto | Errore sulla durata |
| --- | --- | --- | --- |
| Pressione subito, rilascio confermato | al primo rimbalzo | 50 ms dopo l'ultimo rimbalzo del rilascio | **+50 ms** più la durata dei rimbalzi del rilascio (fino a 60 ms in simulazione) |
| Livello stabile (`changed()`) | 50 ms dopo l'ultimo rimbalzo della pressione | 50 ms dopo l'ultimo rimbalzo del rilascio | la differenza fra le durate dei due rimbalzi (al massimo 10 ms in simulazione) |

Con la prima tecnica l'errore si corregge togliendo 50 ms. In alternativa si registra l'istante
dell'ultima lettura alta e si calcola la durata fino a quello. Resta comunque un errore pari alla
durata dei rimbalzi. Con `changed()` non serve nessuna correzione.

**5. Situazioni critiche.**

| Situazione | Pressione subito, rilascio confermato | `changed()` |
| --- | --- | --- |
| Buco di 2 ms a pulsante premuto | nessun effetto: il buco dura meno di 50 ms | nessun effetto, ma se il buco cade nei primi 50 ms la pressione è confermata più tardi e la durata risulta più corta, fino a circa 50 ms |
| Spike di 2 ms a riposo | **una misura falsa** di circa 52 ms: "poco zucchero" e LED verde senza che nessuno abbia premuto (5000 casi su 5000 in simulazione) | nessuna misura |
| Tocco più breve di 50 ms | **viene misurato**, perché la durata calcolata include i 50 ms della conferma | nessuna misura, come chiede la consegna |

**6. Facoltativa: LED lampeggiante durante la pressione.** Il lampeggio usa un secondo tempo,
`tBlink`, e una variabile per lo stato del LED. Non si legge il pin di uscita, perché su ESP32 un pin
configurato come `OUTPUT` letto con `digitalRead()` restituisce sempre 0. La misura non cambia,
perché `tPress` è registrato solo in `doOnRise()`.

```cpp
unsigned long tBlink = 0;
bool ledAcceso = false;

void doOnRise() {
  tPress = millis();
  tBlink = millis();
  ledAcceso = true;
  digitalWrite(LED, HIGH);
}
// doOnFall() spegne il LED e mette ledAcceso = false

void loop() {
  if (changed()) {
    if (val00 == HIGH) {
      doOnRise();
    } else {
      doOnFall();
    }
  }
  if (val00 == HIGH && millis() - tBlink >= 250) {  // 2 Hz: 250 ms acceso, 250 spento
    tBlink = millis();
    ledAcceso = !ledAcceso;
    digitalWrite(LED, ledAcceso);
  }
}
```

**Risultati in simulazione** (5000 prove per caso):

| Caso | Misure sbagliate o in più | Errore massimo sulla durata |
| --- | --- | --- |
| Pressioni normali con rimbalzi | 0 | 10 ms |
| Spike a riposo | 0 | — |
| Tocchi più brevi di 50 ms | 0 (nessuna misura, come richiesto) | — |
| Buco a pulsante premuto | 1 | 57 ms (buco nei primi 50 ms) |

L'unico caso sbagliato col buco è una pressione molto breve, con il buco proprio in mezzo: è il
limite discusso nell'esercizio 2, domanda 5.

## 2. Conteggio delle pressioni

### Scelta della tecnica

Anche qui serve il **livello stabile in due direzioni**. Un contatore usa la logica a fronte, quindi
ogni evento falso diventa un errore permanente (vedi la risposta 1). La tecnica deve filtrare tutte e
quattro le situazioni critiche, compreso lo spike a riposo.

### Programma

```cpp
const byte P1 = 2;
const byte LED_VERDE = 11;
const byte LED_ROSSO = 12;
const int CAPIENZA = 10;
byte val0  = LOW;        // ultima lettura
byte val00 = LOW;        // livello accettato
unsigned long last = 0;  // istante dell'ultima variazione
unsigned long tPress = 0;
int persone = 0;

bool changed() {
  byte val = digitalRead(P1);
  if (val != val0) {
    val0 = val;
    last = millis();
  } else if (val != val00 && millis() - last > 50) {
    val00 = val;
    return true;
  }
  return false;
}

void aggiornaLed() {
  digitalWrite(LED_ROSSO, persone >= CAPIENZA ? HIGH : LOW);
  digitalWrite(LED_VERDE, persone >= CAPIENZA ? LOW : HIGH);
}

void doOnRise() {
  // PRESSIONE confermata: serve solo a misurare la durata
  tPress = millis();
}

void doOnFall() {
  // RILASCIO confermato: azzeramento o conteggio
  if (millis() - tPress > 2000) {
    persone = 0;             // pressione lunga: azzera
  } else if (persone < CAPIENZA) {
    persone++;               // pressione breve: un ingresso
  }
  Serial.print("Persone: ");
  Serial.println(persone);
  aggiornaLed();
}

void setup() {
  Serial.begin(115200);
  pinMode(P1, INPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROSSO, OUTPUT);
  aggiornaLed();
}

void loop() {
  if (changed()) {
    if (val00 == HIGH) {
      doOnRise();
    } else {
      doOnFall();
    }
  }
}
```

### Risposte

**1. Livello e fronte.** Con la logica a **livello** (LED acceso finché il pulsante è premuto), un
evento falso produce un errore che dura quanto il disturbo: il LED si spegne per qualche millisecondo
e poi torna giusto. Con la logica a **fronte** (contatore, toggle), un evento falso lascia un errore
**permanente**: il contatore resta sbagliato finché qualcuno non lo corregge a mano. Per questo un
contatore ha bisogno della tecnica più robusta.

**4. Confronto con "pressione subito, rilascio confermato".** Le due versioni contano allo stesso modo
i rimbalzi e i buchi, che entrambe filtrano. Divergono con uno **spike a riposo**, la situazione
critica 4. Per esempio, il cavo del pulsante passa vicino a un motore o a un relè, oppure qualcuno
scarica elettricità statica sul pulsante. La versione a una direzione scambia lo spike per una
pressione, poi conferma il rilascio 50 ms dopo, e conta un ingresso che non c'è. La versione con
`changed()` non conta nulla.

**5. Pressioni rapide.** Ogni pressione deve restare alta per più di 50 ms dopo l'ultimo rimbalzo.
Poi il pulsante deve restare basso per altri 50 ms prima della pressione successiva. Il periodo
minimo è quindi di poco più di 100 ms, più la durata dei rimbalzi: circa **8-10 pressioni al
secondo**. Una pressione più breve di 50 ms viene scartata come un disturbo, e quindi **non viene
contata**: è il prezzo della robustezza.

Per lo stesso motivo una pressione corta con un buco proprio in mezzo può andare persa. Per esempio,
in una pressione di 100 ms con un buco a metà, nessuna delle due parti resta ferma 50 ms. In
simulazione è successo in 11 sequenze su 2000, sempre con pressioni di circa 100 ms. Per evitarlo,
la pressione deve durare almeno il doppio del tempo di debounce, oppure il tempo di debounce va
ridotto, ma mai sotto i 50 ms delle dispense.

**6. Facoltativa: secondo pulsante con `Debouncer`.** Ogni oggetto `Debouncer` conserva lo stato di
**un** pulsante: l'ultima lettura (`val0`), il livello accettato (`val00`) e l'istante dell'ultima
variazione (`last`). Con un solo oggetto per due pin, le letture di P1 e P2 si mescolerebbero: ogni
lettura di un pin farebbe ripartire il conteggio dell'altro, e nessuno dei due verrebbe mai
confermato.

```cpp
#include "urutils.h"
const byte P1 = 2;   // entrate
const byte P2 = 3;   // uscite
const byte LED_VERDE = 11;
const byte LED_ROSSO = 12;
const int CAPIENZA = 10;
Debouncer entrata(LOW);   // un oggetto per pulsante
Debouncer uscita(LOW);
unsigned long tPress = 0;
int persone = 0;

void aggiornaLed() {
  digitalWrite(LED_ROSSO, persone >= CAPIENZA ? HIGH : LOW);
  digitalWrite(LED_VERDE, persone >= CAPIENZA ? LOW : HIGH);
  Serial.print("Persone: ");
  Serial.println(persone);
}

void setup() {
  Serial.begin(115200);
  pinMode(P1, INPUT);
  pinMode(P2, INPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROSSO, OUTPUT);
  aggiornaLed();
}

void loop() {
  if (entrata.changed(P1, 50)) {
    if (entrata.val00 == HIGH) {
      tPress = millis();          // pressione
    } else {                      // rilascio
      if (millis() - tPress > 2000) {
        persone = 0;
      } else if (persone < CAPIENZA) {
        persone++;
      }
      aggiornaLed();
    }
  }
  if (uscita.changed(P2, 50) && uscita.val00 == LOW) {
    if (persone > 0) {            // rilascio di P2: un'uscita
      persone--;
    }
    aggiornaLed();
  }
}
```

**Risultati in simulazione.** 2000 sequenze, ciascuna di dodici pressioni brevi seguite da una
lunga. Rimbalzi sempre presenti, buchi e spike aggiunti a caso. Il contatore è arrivato a 10 e si è
fermato, poi la pressione lunga lo ha azzerato con il LED verde acceso, in tutte le sequenze tranne
11. Gli 11 errori sono le pressioni di circa 100 ms con il buco in mezzo, discusse nella risposta 5.

>[Torna all'indice](indexpulsanti.md) >[Consegne](consegneEserciziPulsanti.md)
