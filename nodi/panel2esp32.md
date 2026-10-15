# ESP32 a energia solare + USB con batteria LiPo e load sharing

Alimentazione autonoma per un ESP32 con **pannello solare e/o alimentatore USB** che caricano una **batteria LiPo** tramite un modulo **TP4056**. Un semplice circuito di **load sharing** (un P-MOSFET e un diodo) fa sì che, quando c'è alimentazione esterna, l'ESP32 si alimenti direttamente da questa e la batteria si carichi senza carico addosso. Quando l'alimentazione esterna manca, il circuito passa da solo alla batteria, senza interruzioni.

Il progetto parte dal tutorial di Random Nerd Tutorials [*Power ESP32/ESP8266 with Solar Panels and Battery*](https://randomnerdtutorials.com/power-esp32-esp8266-solar-panels-battery-level-monitoring/) e aggiunge:

- **due sorgenti contemporanee** (pannello e USB) unite da diodi Schottky;
- **load sharing**, così il TP4056 termina correttamente la carica anche con l'ESP32 acceso;
- il regolatore **HT7833** (500 mA) al posto dell'MCP1700 (250 mA), più adatto ai picchi del WiFi;
- un partitore **100k/100k**, che porta la lettura nella zona più lineare dell'ADC.

Altre guide in questo repository:

- **[HELTEC.md](HELTEC.md)**: come usare il progetto con le schede Heltec WiFi Kit 32 V3 e Wireless Stick Lite V3;
- **[Variante con diodo ideale LTC4412](#variante-diodo-ideale-ltc4412-al-posto-di-d1)**: al posto di D1, meno caduta di tensione dal pannello e precedenza all'USB;
- **[Programmare l'ESP32 collegato alla scheda](#programmare-lesp32-collegato-alla-scheda)**: quando staccare il 3V3 durante il caricamento del firmware;
- **[Sensore pesante: SDS011](#sensore-pesante-sds011)**: come alimentare e collegare un sensore di particolato SDS011;
- **[MOSFET.md](MOSFET.md)**: dispensa su come scegliere e dimensionare P-MOSFET e N-MOSFET usati come interruttori;
- **[PCB.md](PCB.md)**: circuito stampato pronto da ordinare (Gerber in `pcb/gerber/`), configurabile per SDS011 o SCD41;
- **[MOSFET_TRIODO.md](MOSFET_TRIODO.md)**: approfondimento teorico sul modello quadratico e sulla zona di triodo, cioè il perché della regola 2–2,5×.

---

## Schema

<p align="center">
  <img src="img/schema.svg" alt="Schema elettrico" width="100%">
</p>

---

## Componenti

| Rif. | Componente | Quantità | Note |
|---|---|---|---|
| — | Modulo **TP4056 con protezione** (DW01 + 8205A, 6 piazzole con OUT+/OUT–) | 1 | La versione con protezione stacca la batteria se si scarica troppo |
| — | Batteria **LiPo 3,7 V** (es. 2000 mAh) | 1 | |
| — | **Pannello solare 6 V**, 2–3 W | 1 | Con il TP4056 i pannelli da 6 V funzionano meglio di quelli da 5 V |
| — | Ingresso **USB 5 V** | 1 | Un vecchio cavo USB tagliato (rosso = +5 V, nero = GND) o un breakout USB femmina |
| D1, D2, D3 | Diodo Schottky **1N5819** (o 1N5817, SS14, SS34) | 3 | |
| Q1 | P-MOSFET **AO3401A** (SOT-23) | 1 | Se non saldi SMD, prendilo su adattatore SOT-23 → DIP |
| R3 | Resistenza **100 kΩ** | 1 | Pull-down del gate |
| U1 | Regolatore LDO **HT7833** (3,3 V, 500 mA) | 1 | Alternative: HT7333, RT9013-33 |
| C1, C2 | Condensatore **10 µF** | 2 | Ingresso e uscita del regolatore |
| C3 | Condensatore elettrolitico **470 µF** (≥ 6,3 V) | 1 | Assorbe i picchi del WiFi. **Ha una polarità!** |
| C4 | Condensatore ceramico **100 nF** | 1 | |
| R1, R2 | Resistenza **100 kΩ** | 2 | Partitore per leggere la batteria |
| C5 | Condensatore ceramico **100 nF** | 1 | Stabilizza la lettura dell'ADC |
| — | **ESP32 DevKit** | 1 | |

---

## Come funziona

### 1. Due sorgenti, nessun conflitto (D1, D2)

Pannello e USB arrivano allo stesso nodo **VIN_EXT** attraverso un diodo ciascuno. In ogni momento conduce solo la sorgente a tensione più alta. I diodi impediscono che il pannello, che a vuoto arriva a 6–7 V, spinga corrente nell'alimentatore USB o nella porta di un PC, e che l'USB spinga corrente nel pannello.

Dopo la caduta sui diodi (circa 0,3–0,45 V) arrivano circa 4,6 V dall'USB e circa 5,5–6,5 V dal pannello: entrambi valori adatti al TP4056, che accetta da 4,5 a 8 V.

> ⚠️ Il connettore micro-USB **sul modulo TP4056** è collegato direttamente a IN+, senza diodo. Così com'è **non usarlo** con il pannello collegato: porta l'USB a VIN_EXT passando per D2, oppure modifica il modulo come descritto in [Alternativa: usare il micro-USB del TP4056](#alternativa-usare-il-micro-usb-del-tp4056).

### 2. Load sharing (Q1, D3, R3)

Il TP4056 non ha un "power path": se il carico sta su OUT+ mentre la batteria si carica, il chip vede anche la corrente dell'ESP32. Con il WiFi acceso può quindi non terminare mai la carica e tenere la batteria fissa a 4,2 V.

Il circuito di load sharing risolve il problema:

| Situazione | Gate di Q1 | Q1 | L'ESP32 è alimentato da | La batteria |
|---|---|---|---|---|
| Pannello o USB presenti | alto (VIN_EXT) | **spento** | VIN_EXT tramite D3 | si carica senza carico |
| Nessuna alimentazione esterna | basso (R3 verso GND) | **acceso** | batteria tramite Q1 | si scarica normalmente |

#### Il MOSFET Q1 nel dettaglio

**Cos'è.** Q1 è un **MOSFET a canale P**, in pratica un interruttore elettronico con tre terminali: **source (S)**, **drain (D)** e **gate (G)**. Un P-MOSFET conduce quando il gate è **più basso del source** di almeno la tensione di soglia (|V<sub>GS(th)</sub>| ≈ 0,5–1,3 V per l'AO3401A). Quando conduce, tra drain e source c'è una resistenza di poche decine di milliohm.

Ogni MOSFET contiene anche un **diodo interno (body diode)**, che nello schema è disegnato tratteggiato. In un P-MOSFET il body diode ha l'anodo sul drain e il catodo sul source.

**Perché è montato "al contrario".** Q1 ha il **drain verso la batteria** (OUT+) e il **source verso il carico** (VLOAD). Così il body diode conduce solo dalla batteria verso il carico, mai al contrario. Da spento, Q1 impedisce quindi che l'alimentazione esterna finisca nella batteria scavalcando il TP4056.

**Caso A: pannello o USB presenti**

| Punto | Tensione tipica (USB) | Tensione tipica (pannello al sole) |
|---|---|---|
| VIN_EXT = gate | ~4,6 V | ~5,5–6,5 V |
| VLOAD = source (VIN_EXT – D3) | ~4,2 V | ~5,1–6,1 V |
| V<sub>GS</sub> = gate – source | **+0,4 V** | **+0,4 V** |

V<sub>GS</sub> è positiva, quindi un P-MOSFET è **spento**. Anche il body diode è interdetto, perché la batteria (al massimo 4,2 V) non supera VLOAD. L'ESP32 è alimentato da VIN_EXT attraverso D3, e il TP4056 carica la batteria senza carico: la corrente che misura è solo quella di carica e la carica termina correttamente.

**Caso B: nessuna alimentazione esterna**

1. VIN_EXT scende a 0 e **R3 porta il gate a GND**.
2. Nel primo istante conduce il body diode: VLOAD ≈ VBAT – 0,6 V.
3. A quel punto V<sub>GS</sub> ≈ 0 – (VBAT – 0,6) ≈ **–3 V**, ben oltre la soglia: Q1 **si accende completamente**.
4. Il canale del MOSFET scavalca il body diode e VLOAD ≈ VBAT meno pochi millivolt.

**Il passaggio da una sorgente all'altra** non interrompe l'alimentazione. Quando l'esterno sparisce, il body diode alimenta subito il carico, mentre il gate scende in poche decine di microsecondi. Nel frattempo i condensatori C1 e C3 tengono su la tensione. Quando l'esterno torna, il gate si alza, Q1 si spegne e D3 riprende a fornire la corrente.

**Pannello debole (alba, tramonto, nuvole).** Se VIN_EXT è più bassa della batteria (per esempio 3,5 V con la batteria a 3,9 V), D3 è interdetto e Q1 conduce, almeno in parte, oppure conduce il suo body diode. L'ESP32 resta alimentato dalla batteria, mentre il pannello dà quel poco che può al TP4056.

#### Perché proprio questi componenti

- **Perché un MOSFET e non un semplice diodo dalla batteria?** Un diodo Schottky farebbe cadere 0,3–0,4 V. Con 200 mA, invece, Q1 fa cadere circa 200 mA × 0,08 Ω ≈ **16 mV**. L'HT7833 ha bisogno di circa 3,4–3,5 V in ingresso per dare 3,3 V, quindi ogni decimo di volt risparmiato è carica della batteria che puoi davvero usare.
- **Perché serve D3?** Senza D3, quando Q1 è acceso la batteria tornerebbe su VIN_EXT: alzerebbe il gate di Q1 e lo spegnerebbe, con un'oscillazione continua. Inoltre alimenterebbe all'indietro l'ingresso del TP4056. D3 lascia passare corrente solo da VIN_EXT verso il carico.
- **Perché serve R3?** Senza alimentazione esterna il gate resterebbe "flottante", cioè a una tensione indefinita, e Q1 potrebbe non accendersi. R3 lo porta con certezza a GND. Con 100 kΩ consuma circa 50 µA, e **solo quando c'è l'alimentazione esterna**: a batteria non consuma nulla.
- **Perché l'AO3401A?** È un MOSFET "logic level": si accende bene anche con V<sub>GS</sub> di soli –2,5/–3 V, cioè con la batteria quasi scarica. Regge –30 V e alcuni ampere, molto più del necessario. Vanno bene anche altri P-MOSFET con soglia bassa e R<sub>DS(on)</sub> bassa, come DMG2305UX o Si2301.

> ⚠️ Non usare P-MOSFET "classici" come IRF9540 o IRF9Z34: hanno una soglia troppo alta (2–4 V) e con la batteria a 3,5 V non si accenderebbero del tutto.

### 3. Regolatore a 3,3 V (U1, C1–C4)

L'HT7833 porta VLOAD (da circa 3,3 a 6,5 V) a 3,3 V stabili per il pin **3V3** dell'ESP32. Ha un dropout basso e una corrente a vuoto di pochi µA, quindi va bene anche con il deep sleep. C3 (470 µF), montato vicino all'ESP32, copre i picchi di corrente all'avvio del WiFi.

> ⚠️ Non collegare **mai** OUT+ della batteria direttamente al pin 3V3: una LiPo carica arriva a 4,2 V, mentre l'ESP32 regge al massimo circa 3,6 V.

### 4. Lettura della batteria (R1, R2, C5)

**A cosa serve.** La tensione di una LiPo indica, in modo approssimato, quanta carica le resta: circa 4,2 V da carica, circa 3,7 V a metà, sotto i 3,4 V quasi scarica. Leggendola, l'ESP32 può:

- inviare il livello della batteria insieme agli altri dati, così lo controlli da remoto;
- **risparmiare energia quando la batteria è bassa**, per esempio allungando il deep sleep, saltando il WiFi o non accendendo i sensori pesanti come l'SDS011;
- avvisarti se il pannello non riesce più a tenere carica la batteria (pannello sporco, ombreggiato, guasto).

**Perché serve un partitore.** L'ADC dell'ESP32 misura al massimo circa 3,1 V, e i pin reggono al massimo circa 3,6 V. La batteria arriva a 4,2 V: collegata direttamente, la lettura andrebbe fuori scala e il pin rischierebbe di danneggiarsi. Le due resistenze uguali R1 e R2 **dimezzano** la tensione:

```
V_ADC = VBAT × R2 / (R1 + R2) = VBAT / 2        →   VBAT = V_ADC × 2
```

A 4,2 V corrispondono 2,1 V sull'ADC, nella zona in cui l'ADC dell'ESP32 è più lineare e preciso.

**Perché si legge VBAT e non un altro punto.** Il partitore è collegato a **OUT+**, che sui moduli protetti coincide con B+, cioè con la batteria. Non avrebbe senso misurare il 3,3 V, che è regolato e resta fisso finché la batteria non è quasi vuota, né VLOAD, che con il sole riflette la tensione del pannello e non quella della batteria.

**Perché 100 kΩ.** Il partitore è sempre collegato alla batteria, quindi le resistenze devono essere alte per consumare poco: circa **21 µA** in modo continuo, circa 0,5 mAh al giorno. Valori ancora più alti renderebbero la lettura instabile, perché l'ADC durante la misura assorbe brevi impulsi di corrente. Il condensatore **C5** (100 nF) fa da serbatoio per quegli impulsi e stabilizza la lettura. Se vuoi azzerare anche quei 21 µA, vedi la [variante con partitore attivabile](#variante-partitore-attivabile-consumo-zero-a-riposo).

**Quale pin.** Si usa **GPIO33**, che fa parte dell'ADC1. I pin dell'ADC2 non funzionano quando il WiFi è attivo.

> ℹ️ Durante la carica il partitore legge la tensione imposta dal caricatore (fino a 4,2 V), più alta della tensione reale della batteria a riposo. La percentuale è affidabile solo quando non c'è alimentazione esterna.

---

## ⚠️ Attenzione alla massa

Sui moduli TP4056 con protezione, l'interruttore di protezione è sul **negativo**:

- **B–** → solo il polo negativo della batteria;
- **IN–** e **OUT–** → massa comune di tutto il circuito (pannello, USB, regolatore, ESP32, partitore).

Se colleghi la massa del circuito a B–, scavalchi la protezione dalla sovrascarica. **OUT+ coincide invece con B+**, per questo il partitore è collegato a OUT+.

---

## Montaggio passo passo

1. **Batteria.** Collega la LiPo a **B+ / B–** del TP4056, rispettando la polarità.
2. **Sorgenti.** Collega il + del pannello all'**anodo di D1** e il + dell'USB all'**anodo di D2**. I due **catodi** (lato con la fascetta) vanno uniti e collegati a **IN+**. Il – del pannello e il – dell'USB vanno a **IN–**.
3. **Prima prova.** Senza ESP32, controlla che il LED rosso del TP4056 si accenda (carica in corso) sia con il solo pannello sia con il solo USB.
4. **Load sharing.**
   - **Drain** di Q1 → **OUT+**
   - **Source** di Q1 → nodo **VLOAD**
   - **Gate** di Q1 → **VIN_EXT**, con R3 (100k) dal gate verso **GND**
   - **D3**: anodo su VIN_EXT, catodo su VLOAD
5. **Regolatore.** VLOAD → IN dell'HT7833, con C1 verso GND. GND dell'HT7833 → massa. OUT → C2, C3 e C4 verso GND.
6. **Verifica prima di collegare l'ESP32.** Con il tester:
   - OUT dell'HT7833 = **3,25–3,35 V**, sia con alimentazione esterna sia solo a batteria;
   - VLOAD ≈ VIN_EXT – 0,4 V con alimentazione esterna, ≈ VBAT senza.
7. **ESP32.** OUT dell'HT7833 → pin **3V3**, massa → **GND**.
8. **Partitore.** OUT+ → R1 (100k) → **GPIO33** → R2 (100k) → GND. C5 (100 nF) va tra GPIO33 e GND.

### Programmare l'ESP32 collegato alla scheda

Quando colleghi il cavo USB alla DevKit per caricare il firmware, sulla linea 3V3 ci sono **due regolatori in parallelo**: quello della DevKit (alimentato dall'USB del PC) e l'HT7833. Cosa succede dipende dalla tensione all'ingresso dell'HT7833 (VLOAD):

| Situazione | VLOAD | Cosa succede |
|---|---|---|
| Pannello o USB su J2 presenti, oppure batteria carica | > 3,5 V circa | **Nessun problema.** I due regolatori danno quasi la stessa tensione: quello leggermente più alto fornisce la corrente, l'altro smette di erogare. Un regolatore lineare non assorbe corrente dalla sua uscita, quindi non si "combattono". |
| Batteria scarica (sotto 3,4–3,5 V), niente sole e niente USB su J2 | < 3,3 V | **Da evitare.** L'uscita dell'HT7833 (3,3 V, imposti dalla DevKit) è più alta del suo ingresso. In molti regolatori LDO, in questa condizione, la corrente torna indietro dall'uscita all'ingresso. Da VLOAD passa per Q1, che è acceso perché manca VIN_EXT, e arriva alla batteria: **la DevKit carica la batteria** senza il controllo del TP4056, con una corrente limitata solo dal suo regolatore. Non c'è rischio di sovraccarica (la tensione non supera 3,3 V), ma la corrente può essere di centinaia di mA e scalda i due regolatori e la porta USB del PC. D3 invece non conduce, perché è polarizzato al contrario. |

**Cosa fare:**

- **Soluzione consigliata:** mentre programmi **stacca il filo 3V3** tra l'HT7833 e l'ESP32 (sullo stampato basta togliere il cappuccio di **JP3**, vedi [PCB.md](PCB.md)). La massa resta collegata: l'ESP32 è alimentato solo dall'USB del PC e il resto del circuito continua a funzionare. Finito il caricamento, ricollega il filo.
- **In alternativa:** collega il pannello al sole o l'USB a J2 mentre programmi. Con VIN_EXT presente Q1 è spento e VLOAD è circa 4,6 V, quindi si ricade nel primo caso della tabella.

> 💡 **E un regolatore con protezione dalla corrente inversa?** Esistono regolatori LDO che dichiarano nel datasheet la *reverse current protection*: bloccano la corrente dall'uscita all'ingresso e risolverebbero il problema alla radice. Non ho però trovato un modello verificato con la **stessa piedinatura dell'HT7833 in SOT-89**, da montare sullo stampato senza modifiche. Anche l'HT78R33 ha la stessa piedinatura, ma il datasheet non parla di questa protezione. Per questo la soluzione proposta resta il ponticello: costa pochi centesimi e si capisce a colpo d'occhio. Se usi un regolatore diverso, cerca nel datasheet le voci *reverse current protection* o *reverse current blocking* e controlla la piedinatura.

---

## Alternativa: usare il micro-USB del TP4056

Se preferisci collegare l'alimentatore USB direttamente al connettore micro-USB del modulo, invece del cavo tagliato, puoi trasformare quel connettore nell'ingresso protetto da diodo. Basta inserire un diodo Schottky tra il pin VBUS del connettore e la piazzola IN+: **il diodo saldato sul modulo prende il posto di D2**.

### Perché serve la modifica

Sul modulo originale il pin **VBUS** del micro-USB è collegato con una pista direttamente a **IN+**. Senza diodo, in pieno sole il pannello (circa 5,5–6,5 V dopo D1) spinge tensione verso l'alimentatore USB, che ne ha solo 5. Un caricatore da muro di solito lo tollera, ma la porta USB di un PC, un hub o un power bank potrebbero non gradire.

### Cosa serve

- 1 diodo Schottky **SMD SS14** (1 A, 40 V) oppure **SS34** (3 A, 40 V), contenitore SMA;
- cutter o bisturi, saldatore a punta fine, flussante, filo di rame sottile (eventualmente un reoforo tagliato);
- tester con la funzione continuità.

### Procedura

1. **Individua la pista.** Con il tester in continuità, trova la pista che collega il pin **VBUS** del micro-USB (il pin più esterno, dal lato del + del modulo) alla piazzola **IN+**. Spesso passa vicino al connettore o attraverso un via.
2. **Taglia la pista.** Incidi due volte con il cutter a circa 1 mm di distanza e asporta il rame tra i due tagli. Fallo in un tratto **senza altre piste vicine**.
3. **Verifica il taglio.** Tra VBUS del micro-USB e IN+ il tester **non deve più dare continuità**.
4. **Gratta la vernice** ai due lati del taglio, quanto basta per scoprire un po' di rame da stagnare.
5. **Salda il diodo** a cavallo del taglio:
   - **anodo** verso il connettore micro-USB (lato VBUS);
   - **catodo** (il lato con la fascetta) verso **IN+**.

   Se il diodo non arriva a coprire il taglio, completa il collegamento con un pezzetto di filo di rame.
6. **Controlla con il tester in modalità diodo.** Puntale rosso su VBUS, nero su IN+: devi leggere circa **0,2–0,4 V**. Al contrario deve risultare aperto (OL).
7. **Prova sotto tensione.** Collega un alimentatore USB senza batteria e senza ESP32. Su IN+ devi misurare circa **4,6–4,8 V**.

```
                PRIMA                              DOPO
  micro-USB VBUS ─────────── IN+     micro-USB VBUS ──►|── IN+
                                                     SS14
                                              (catodo verso IN+)
```

### Dopo la modifica

- **D2 e il cavo USB tagliato non servono più**: collega l'alimentatore direttamente al micro-USB del modulo. Il pannello resta collegato a IN+ tramite D1, come nello schema.
- Il **load sharing funziona allo stesso modo**: VIN_EXT è il nodo IN+, e da lì partono il gate di Q1 e D3.
- Puoi usare qualunque sorgente USB (caricatore da muro, PC, power bank) anche con il pannello collegato.
- Dall'USB arrivano circa 4,6–4,8 V, comunque sufficienti per il TP4056.

> ⚠️ Lavora con il modulo **scollegato** da batteria e alimentazioni. Se tagli la pista sbagliata o danneggi il connettore, il modulo costa poco: tienine uno di scorta.

---

## Codice di esempio (Arduino IDE)

Legge la tensione della batteria, la converte in una percentuale approssimata, la stampa e manda l'ESP32 in deep sleep per 10 minuti.

```cpp
#include <Arduino.h>

const int   PIN_BATT    = 33;     // GPIO33 (ADC1)
const float DIVIDER     = 2.0;    // R1 = R2 = 100k  ->  Vbat = Vadc * 2
const uint64_t SLEEP_US = 10ULL * 60ULL * 1000000ULL;  // 10 minuti

float readBatteryVoltage() {
  uint32_t sum = 0;
  for (int i = 0; i < 16; i++) {
    sum += analogReadMilliVolts(PIN_BATT);  // valore già calibrato in mV
    delay(2);
  }
  return (sum / 16.0f) * DIVIDER / 1000.0f;
}

// Curva di scarica LiPo approssimata (a riposo, a 25 °C)
int batteryPercent(float v) {
  const float volts[] = {3.30, 3.50, 3.60, 3.70, 3.75, 3.80, 3.85, 3.90, 4.00, 4.10, 4.20};
  const int   pct[]   = {   0,    5,   10,   20,   30,   40,   50,   60,   75,   90,  100};
  const int n = sizeof(volts) / sizeof(volts[0]);

  if (v <= volts[0])     return 0;
  if (v >= volts[n - 1]) return 100;
  for (int i = 1; i < n; i++) {
    if (v < volts[i]) {
      float t = (v - volts[i - 1]) / (volts[i] - volts[i - 1]);
      return pct[i - 1] + t * (pct[i] - pct[i - 1]);
    }
  }
  return 100;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  float vbat = readBatteryVoltage();
  Serial.printf("Batteria: %.2f V  (%d %%)\n", vbat, batteryPercent(vbat));

  // ... qui il tuo codice: WiFi, sensori, invio dati ...

  esp_sleep_enable_timer_wakeup(SLEEP_US);
  esp_deep_sleep_start();
}

void loop() {}
```

**Note sulla lettura**
- Durante la carica leggerai la tensione di carica (fino a circa 4,2 V), più alta di quella reale a riposo. Considera la percentuale affidabile solo senza alimentazione esterna.
- Per una lettura più precisa, misura VBAT con il tester e correggi `DIVIDER` di conseguenza (le resistenze hanno una tolleranza dell'1–5 %).

---

## Variante: diodo ideale LTC4412 al posto di D1

D1 ha due limiti:

- **Caduta di tensione.** Uno Schottky perde 0,3–0,45 V. Con poco sole il pannello scende verso i 5 V e il TP4056, che vuole almeno 4,5 V all'ingresso, smette di caricare prima del necessario.
- **Precedenza delle sorgenti.** Con i diodi vince la sorgente a tensione più alta. Quasi sempre è il pannello, che a vuoto arriva a 6–7 V: anche con l'USB collegato, la carica la fa il pannello. Se vuoi che **l'USB abbia la precedenza** e che il pannello venga staccato quando c'è l'USB, i diodi non bastano.

Un **diodo ideale** risolve entrambi i problemi. Al posto di D1 si usano due P-MOSFET comandati da un piccolo integrato, l'**LTC4412** di Analog Devices.

<p align="center">
  <img src="img/schema_ltc4412.svg" alt="Variante con diodo ideale LTC4412 al posto di D1" width="100%">
</p>

### Componenti aggiuntivi

| Rif. | Componente | Note |
|---|---|---|
| U2 | **LTC4412** | controllore di diodo ideale, contenitore TSOT-23-6 (ThinSOT) |
| Q6, Q7 | **AO3401A** | P-MOSFET, gli stessi di Q1 |
| R_CTL | 100 kΩ | dall'USB al pin CTL |

D1 si toglie. D2 sull'USB resta com'è.

### Come funziona

**Diodo ideale.** L'LTC4412 confronta la tensione del pannello (pin **VIN**) con quella di VIN_EXT (pin **SENSE**):

- se il pannello è più alto, abbassa il pin **GATE** e accende Q6 e Q7. Il pannello passa con una caduta di qualche decina di mV: l'integrato regola circa 20 mV a bassa corrente, e a correnti più alte la caduta è data dalla resistenza dei due MOSFET in serie (circa 0,1 Ω in tutto, quindi 50 mV a 0,5 A);
- se VIN_EXT è più alto del pannello (di notte, o con l'USB a tensione maggiore), spegne i MOSFET: è la funzione di "diodo" che blocca la corrente all'indietro.

**Perché due MOSFET contrapposti.** Ogni MOSFET ha un diodo interno (*body diode*) tra drain e source. Con un solo MOSFET quel diodo lascia comunque passare il pannello verso VIN_EXT anche a MOSFET spento. Con Q6 e Q7 montati **con le sorgenti in comune**, i due diodi interni sono in direzioni opposte: a MOSFET spenti la corrente non passa in nessuna delle due direzioni.

**Precedenza all'USB (R_CTL).** Il pin **CTL** spegne i MOSFET quando è a livello alto. Collegandolo all'USB attraverso R_CTL:

| Situazione | CTL | Q6, Q7 | Il TP4056 è alimentato da |
|---|---|---|---|
| Solo pannello | basso (pull-down interno) | accesi | pannello, con caduta di pochi mV |
| USB presente (con o senza pannello) | alto | **spenti** | solo USB, tramite D2 |
| Notte, nessuna sorgente | basso | spenti (VIN = 0) | nessuno: l'ESP32 va a batteria tramite Q1 |

R_CTL limita la corrente nel pin: il pin ha un pull-down interno di pochi µA e riconosce il livello alto già sotto 1 V, quindi 100 kΩ vanno benissimo.

**Consumo.** L'LTC4412 assorbe circa 11 µA, ma li prende dal pannello o da VIN_EXT, non dalla batteria: di notte è spento e non consuma nulla.

**Il pin STAT** (uscita open drain che segnala quale sorgente è attiva) qui non serve e resta scollegato.

> ⚠️ **Monta D1 oppure il gruppo LTC4412, non entrambi.** Con D1 montato il pannello passerebbe comunque, aggirando lo spegnimento comandato dall'USB. Lo stampato ([PCB.md](PCB.md)) ha le piazzole per tutte e due le soluzioni.

---

## Variante: partitore attivabile (consumo zero a riposo)

Il partitore R1/R2 dello schema base è sempre collegato alla batteria e consuma circa **21 µA**, giorno e notte. È poco, circa 0,5 mAh al giorno, ma se punti a mesi di autonomia conviene alimentarlo **solo nel momento della lettura**. Lo stesso principio è usato da schede come le Heltec, che hanno un pin `ADC_Ctrl` per attivare la lettura.

<p align="center">
  <img src="img/schema_partitore_attivabile.svg" alt="Partitore attivabile" width="100%">
</p>

### Componenti aggiuntivi

| Rif. | Componente | Note |
|---|---|---|
| Q2 | P-MOSFET **AO3401A** | Interruttore tra VBAT e R1 |
| Q3 | N-MOSFET **2N7002** o **BSS138** | Comandato dall'ESP32, pilota il gate di Q2 |
| R4 | Resistenza **100 kΩ** | Tra gate e source di Q2: tiene Q2 spento |
| R5 | Resistenza **100 kΩ** | Tra gate di Q3 e GND: tiene Q3 spento quando il GPIO è flottante (deep sleep, reset) |

### Come funziona

- **GPIO25 basso, oppure ESP32 in deep sleep o in reset:** R5 tiene spento Q3, R4 tiene il gate di Q2 a VBAT e quindi anche Q2 è spento. Il partitore è scollegato dalla batteria e consuma **praticamente zero**. GPIO33 resta a 0 V grazie a R2.
- **GPIO25 alto:** Q3 si accende e porta a GND il gate di Q2. V<sub>GS</sub> di Q2 diventa ≈ –VBAT e Q2 si accende: il partitore è alimentato e GPIO33 legge VBAT/2. Durante la lettura R4 consuma circa 42 µA, ma solo per qualche decina di millisecondi.
- Il body diode di Q2 è orientato in modo da non far passare corrente dalla batteria al partitore quando Q2 è spento.

> ⚠️ **Perché l'interruttore è sul lato batteria e non verso massa.** Verrebbe spontaneo mettere un solo N-MOSFET tra R2 e GND. Ma quando è spento il partitore resta "appeso" a VBAT attraverso R1, e su GPIO33 arriverebbero fino a **4,2 V**, oltre il massimo dell'ESP32. Con l'interruttore sul lato alto, a riposo GPIO33 è sempre a 0 V.

### Codice

Al posto della funzione `readBatteryVoltage()` dell'esempio precedente:

```cpp
const int PIN_BATT_EN = 25;   // comando del partitore (Q3)

float readBatteryVoltage() {
  pinMode(PIN_BATT_EN, OUTPUT);
  digitalWrite(PIN_BATT_EN, HIGH);   // accende il partitore
  delay(30);                          // attende che C5 si carichi (τ ≈ 5 ms)

  uint32_t sum = 0;
  for (int i = 0; i < 16; i++) {
    sum += analogReadMilliVolts(PIN_BATT);
    delay(2);
  }

  digitalWrite(PIN_BATT_EN, LOW);    // spegne il partitore
  return (sum / 16.0f) * DIVIDER / 1000.0f;
}
```

Al posto di GPIO25 puoi usare qualsiasi GPIO di uscita libero. Evita i pin di strapping (GPIO0, 2, 5, 12, 15) e quelli solo di ingresso (GPIO34–39).

---

## Sensore pesante: SDS011

L'SDS011 (sensore di particolato PM2.5/PM10) è "pesante" per un sistema a batteria per tre motivi:

1. **Vuole 5 V** (circa 4,7–5,3 V), mentre la batteria ne dà 3,0–4,2.
2. **Assorbe circa 70 mA** con ventola e laser accesi. Anche in modalità sleep consuma ancora qualche mA.
3. **Ha bisogno di circa 30 s di ventola** prima di dare letture affidabili.

Per questo il sensore va alimentato con un **boost 5 V** e **scollegato del tutto** tra una misura e l'altra.

<p align="center">
  <img src="img/sds011.svg" alt="Schema di collegamento SDS011" width="100%">
</p>

### Componenti aggiuntivi

| Rif. | Componente | Note |
|---|---|---|
| — | Modulo **boost 5 V**, ≥ 500 mA | Va bene il classico modulo da power bank con USB-A (ingresso 0,9–5 V), o un MT3608 tarato a 5,0 V. **Togli il LED** se c'è |
| Q4 | P-MOSFET **AO3401A** | Interruttore tra VBAT e l'ingresso del boost |
| Q5 | N-MOSFET **2N7002** o **BSS138** | Comandato da GPIO26 |
| R6 | Resistenza **100 kΩ** | Tra gate e source di Q4: tiene Q4 spento |
| R7 | Resistenza **100 kΩ** | Tra gate di Q5 e GND: tiene Q5 spento in deep sleep |
| R_S | Resistenza **10 kΩ** | Tra drain di Q5 e gate di Q4: soft-start |
| C_GD | Condensatore ceramico **1 µF**, ≥ 10 V | Tra gate e drain di Q4: soft-start, la tensione sul boost sale in circa 17 ms |
| C6 | Elettrolitico **470–1000 µF**, ≥ 10 V | Sui 5 V, vicino al sensore: assorbe lo spunto della ventola |

### Collegamenti

1. **VBAT (OUT+ del TP4056)** → source di **Q4**. Drain di Q4 → **IN+** del boost. IN– del boost → GND.
2. **Comando:** GPIO26 → gate di **Q5**, con **R7** verso GND. Drain di Q5 → **R_S** → gate di Q4, con **R6** tra gate di Q4 e VBAT. Source di Q5 → GND.
3. **Soft-start:** **C_GD** tra gate e drain di Q4. Il valore è calcolato nella [dispensa MOSFET](MOSFET.md#55-esempio-completo-q4-per-lsds011-con-soft-start).
4. **Uscita 5 V del boost** → pin **5V** dell'SDS011, con **C6** verso GND. Se il modulo ha l'USB-A, salda i fili sulle piazzole del connettore (i due pin esterni: VBUS e GND) o sui terminali del condensatore d'uscita.
5. **GND** dell'SDS011 → massa comune.
6. **Seriale** (livelli a 3,3 V, collegamento diretto):
   - **TXD** dell'SDS011 → **GPIO16** (RX dell'ESP32)
   - **RXD** dell'SDS011 ← **GPIO17** (TX dell'ESP32)

I nomi dei pin sono stampati sul connettore dell'SDS011: controlla la serigrafia del tuo esemplare.

> ⚠️ **Il boost va alimentato da VBAT, non da VLOAD.** Con il pannello al sole VLOAD arriva a circa 6 V. Un boost con l'ingresso più alto dell'uscita lascia passare quasi tutta la tensione, e l'SDS011 riceverebbe circa 5,6 V, oltre il suo massimo.

> ℹ️ Mentre l'SDS011 è acceso, il TP4056 vede un carico sulla batteria (come nel caso senza load sharing). Dura solo circa 40 s per misura, quindi non disturba la terminazione della carica.

### Bilancio energetico

Dalla batteria l'SDS011 con il boost assorbe circa **110 mA** (70 mA a 5 V con un'efficienza di circa l'85 %). Con circa 40 s di accensione per misura:

| Intervallo tra le misure | Consumo medio | Al giorno | Autonomia senza sole (2000 mAh, solo SDS011) |
|---|---|---|---|
| 10 min | ~7 mA | ~180 mAh | ~10 giorni |
| 30 min | ~2,5 mA | ~60 mAh | ~1 mese |
| 60 min | ~1,2 mA | ~30 mAh | ~2 mesi |

A questi valori va aggiunto il consumo dell'ESP32 (WiFi, invio dei dati).

- **Pannello:** con misure ogni 10 minuti, il pannello da 2–3 W d'inverno non basta: servono **5–6 W**. Con misure ogni 30–60 minuti il pannello da 2–3 W è in genere sufficiente.
- **Durata del laser:** il produttore indica circa 8000 ore di funzionamento. Accenderlo solo per le misure, come qui, ne allunga molto la vita.

### Codice di esempio

Accende il sensore, attende 30 s, legge e fa la media di alcuni pacchetti, spegne il sensore e manda l'ESP32 in deep sleep.

```cpp
#include <Arduino.h>

const int SDS_EN_PIN = 26;   // comando del boost (Q5)
const int SDS_RX_PIN = 16;   // RX dell'ESP32  <- TXD dell'SDS011
const int SDS_TX_PIN = 17;   // TX dell'ESP32  -> RXD dell'SDS011
const uint32_t WARMUP_MS = 30000;
const uint64_t SLEEP_US  = 30ULL * 60ULL * 1000000ULL;   // 30 minuti

HardwareSerial sds(2);

// Legge un pacchetto SDS011 (10 byte: AA C0 ... AB). Restituisce true se valido.
bool readSdsFrame(float &pm25, float &pm10, uint32_t timeoutMs = 2000) {
  uint8_t buf[10];
  uint32_t t0 = millis();
  int idx = 0;

  while (millis() - t0 < timeoutMs) {
    if (!sds.available()) { delay(5); continue; }
    uint8_t b = sds.read();

    if (idx == 0 && b != 0xAA) continue;          // attende l'header
    if (idx == 1 && b != 0xC0) { idx = 0; continue; }
    buf[idx++] = b;

    if (idx == 10) {
      uint8_t sum = 0;
      for (int i = 2; i <= 7; i++) sum += buf[i];
      if (buf[9] != 0xAB || buf[8] != sum) { idx = 0; continue; }
      pm25 = ((buf[3] << 8) | buf[2]) / 10.0f;
      pm10 = ((buf[5] << 8) | buf[4]) / 10.0f;
      return true;
    }
  }
  return false;
}

void sdsPowerOn() {
  pinMode(SDS_EN_PIN, OUTPUT);
  digitalWrite(SDS_EN_PIN, HIGH);
  sds.begin(9600, SERIAL_8N1, SDS_RX_PIN, SDS_TX_PIN);
}

// Comando "sleep" dell'SDS011: ferma ventola e laser.
// Con il soft-start Q4 si spegne lentamente (~0,3 s): il sensore a riposo
// assorbe poco e Q4 non scalda durante lo spegnimento.
void sdsSleep() {
  const uint8_t cmd[19] = {0xAA, 0xB4, 0x06, 0x01, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0xFF, 0xFF, 0x05, 0xAB};   // 0x05 = checksum
  sds.write(cmd, sizeof(cmd));
  sds.flush();
  delay(200);
}

void sdsPowerOff() {
  sdsSleep();                       // prima a riposo...
  sds.end();
  pinMode(SDS_TX_PIN, INPUT);       // non alimentare il sensore spento dalla linea TX
  digitalWrite(SDS_EN_PIN, LOW);    // ...poi via l'alimentazione
  delay(500);                       // attende lo spegnimento lento di Q4
}

void setup() {
  Serial.begin(115200);

  sdsPowerOn();
  delay(WARMUP_MS);                 // ventola accesa: lascia stabilizzare il flusso d'aria
  while (sds.available()) sds.read();   // scarta i pacchetti del riscaldamento

  float pm25Sum = 0, pm10Sum = 0;
  int n = 0;
  for (int i = 0; i < 5; i++) {     // l'SDS011 invia un pacchetto al secondo
    float pm25, pm10;
    if (readSdsFrame(pm25, pm10)) { pm25Sum += pm25; pm10Sum += pm10; n++; }
  }
  sdsPowerOff();

  if (n > 0) {
    Serial.printf("PM2.5: %.1f µg/m³  PM10: %.1f µg/m³  (%d letture)\n",
                  pm25Sum / n, pm10Sum / n, n);
  } else {
    Serial.println("SDS011: nessuna lettura valida");
  }

  // ... lettura batteria, WiFi, invio dati ...

  esp_sleep_enable_timer_wakeup(SLEEP_US);
  esp_deep_sleep_start();
}

void loop() {}
```

### Consigli per l'installazione all'esterno

- **Umidità:** sopra circa il 70 % di umidità relativa l'SDS011 tende a **sovrastimare** il particolato, perché le goccioline vengono contate come particelle. Affiancagli un sensore di umidità (per esempio un BME280) per scartare o correggere le letture nelle giornate umide.
- **Contenitore:** riparato da pioggia e sole diretto, ma con aria che circola. Il tubetto di aspirazione dell'SDS011 non deve essere ostruito.
- **Alimentazione:** il momento critico è l'accensione. Chiudendo Q4, C6 si carica di colpo attraverso il boost (che ha un percorso diretto tra ingresso e uscita), con un picco di corrente che può far scendere la batteria. Il soft-start (R_S e C_GD) serve proprio a questo. Se l'ESP32 si resetta comunque, **non aumentare C6**, che peggiorerebbe il picco: aumenta C_GD (per esempio a 2,2 µF), oppure riduci C6 a 220–470 µF.

---

## Protezione dalla scarica profonda

Una cella LiPo scaricata sotto circa **2,5 V** si danneggia in modo **irreversibile**: perde capacità, può non accettare più la carica e, nei casi peggiori, gonfiarsi alla ricarica successiva. In un'installazione all'aperto, dove nessuno vede il dispositivo, questa protezione è fondamentale.

La protezione lavora su **due livelli**.

### 1. Protezione hardware (già presente)

Il modulo **TP4056 con DW01 + 8205A** è già un PCM (Protection Circuit Module). Il DW01 controlla la tensione della cella e, tramite i MOSFET 8205A sul negativo (B– → OUT–), **stacca fisicamente il carico**:

- per **sovrascarica**: sotto circa **2,4 V** (valore tipico del DW01);
- per **sovracorrente e cortocircuito**: interviene in microsecondi.

Interviene anche se il firmware è bloccato, e quando la batteria riceve di nuovo carica (dal sole o dall'USB) **si ricollega da solo**.

Questa protezione è l'ultima difesa, e la sua soglia è bassa: protegge la cella dalla distruzione, non dall'usura. Esistono moduli con l'**FS312F** al posto del DW01, compatibile pin a pin, che stacca a circa **2,9 V** ed è preferibile. Molte celle LiPo hanno inoltre un loro PCM integrato nella guaina.

> ℹ️ Una DevKit consuma qualche mA anche in deep sleep, per il LED e il convertitore USB-seriale. In un lungo periodo senza sole è il DW01 a salvare la cella, non il firmware.

### 2. Protezione software

Il firmware legge la batteria **come prima cosa** al risveglio, prima di WiFi e sensori. Se la tensione è troppo bassa, entra in **modalità di emergenza**: niente trasmissioni, niente sensori, deep sleep lungo. Si ferma così molto prima che intervenga il DW01, e si evita il caso peggiore: ogni tentativo di trasmissione fa crollare la tensione, l'ESP32 si resetta, riprova, si resetta di nuovo, e la cella si scarica sempre più in fretta senza che arrivi nessun dato.

**Soglie per questo circuito**

| Soglia | Valore | Perché |
|---|---|---|
| Ingresso in emergenza | **3,30 V** | Sotto circa 3,4 V l'HT7833 non riesce più a tenere i 3,3 V e i picchi del WiFi rischiano il brown-out. A 3,3 V una LiPo ha ancora circa il 5 % di carica |
| Uscita dall'emergenza | **3,50 V** | 200 mV di **isteresi**: evitano che il dispositivo entri ed esca continuamente dall'emergenza quando la tensione è vicina alla soglia |
| Minimo per l'SDS011 | **3,50 V** | Il sensore assorbe circa 110 mA dalla batteria per 40 s: sotto questa soglia si salta la misura e si fa il resto |
| Sleep di emergenza | **6 ore** | Compromesso tra risparmio e ripresa rapida: se il pannello ricarica durante il giorno, il dispositivo torna operativo entro poche ore |

Se usi celle di qualità incerta o di recupero, alza le soglie di 0,1–0,2 V.

**Codice**

Lo stato di emergenza va **ricordato tra un risveglio e l'altro**, altrimenti l'isteresi non funziona. Si usa una variabile in memoria RTC (`RTC_DATA_ATTR`), che sopravvive al deep sleep:

```cpp
const uint32_t VBAT_EMERGENCY_MV  = 3300;   // sotto: emergenza
const uint32_t VBAT_RECOVERY_MV   = 3500;   // sopra: ripresa normale (isteresi 200 mV)
const uint32_t VBAT_SDS011_MIN_MV = 3500;   // sotto: niente misura SDS011
const uint64_t EMERGENCY_SLEEP_US = 6ULL * 3600ULL * 1000000ULL;   // 6 ore

RTC_DATA_ATTR bool emergency = false;       // conservata durante il deep sleep

void goToSleep(uint64_t us) {
  esp_sleep_enable_timer_wakeup(us);
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);

  // 1) Batteria come PRIMA cosa, prima di WiFi e sensori
  uint32_t vbat_mv = readBatteryVoltage() * 1000;

  if (vbat_mv > 500) {                      // ignora letture nulle o errate
    if (emergency && vbat_mv < VBAT_RECOVERY_MV) {
      Serial.printf("Emergenza: %u mV, torno a dormire\n", vbat_mv);
      goToSleep(EMERGENCY_SLEEP_US);
    }
    if (!emergency && vbat_mv < VBAT_EMERGENCY_MV) {
      Serial.printf("BATTERIA CRITICA: %u mV\n", vbat_mv);
      emergency = true;
      goToSleep(EMERGENCY_SLEEP_US);
    }
    emergency = false;                      // tensione sopra la soglia di ripresa
  }

  // 2) Sensori pesanti solo con la batteria in buone condizioni
  if (vbat_mv >= VBAT_SDS011_MIN_MV) {
    // ... misura SDS011 (vedi sezione SDS011) ...
  }

  // 3) WiFi, invio dati (incluso vbat_mv), deep sleep normale
  // ...
  goToSleep(SLEEP_US);
}
```

> ℹ️ **Interazione con il load sharing.** Con il sole o l'USB presenti, il partitore legge la tensione di carica (più alta di quella reale) e il dispositivo può uscire prima dall'emergenza. Non è un problema: in quella situazione l'ESP32 è alimentato da VIN_EXT tramite D3 e non preleva dalla batteria.

### Cosa resta scoperto

- **Firmware bloccato o con errori:** la protezione software non interviene, ma il DW01 sì.
- **Cortocircuito:** il firmware è troppo lento (millisecondi contro microsecondi). Interviene il DW01.
- **Batteria già troppo scarica al risveglio:** se l'ESP32 va in brown-out prima di leggere la batteria, il firmware non può fare nulla. Leggere la batteria come prima istruzione riduce molto questo rischio, e in ultima istanza stacca il DW01.

Per un'installazione permanente all'aperto le due protezioni vanno usate **insieme**: quella software allunga la vita della cella, quella hardware evita il peggio.

---

## Regolazioni facoltative

### Corrente di carica del TP4056

La corrente di carica del TP4056 dipende da una sola resistenza, **R_PROG**. Di fabbrica vale **1,2 kΩ**, cioè circa **1 A**.

#### Dove si trova

- Sui moduli blu con protezione (serigrafia "03962A") è la resistenza SMD siglata **R3**, vicina al chip TP4056.
- Sul componente è stampato **122**, cioè 1,2 kΩ.
- È collegata tra il pin **PROG** (pin 2 del TP4056) e GND.

Per esserne certo, a modulo scollegato misura col tester la resistenza tra il pin 2 del TP4056 e GND: devi leggere circa 1,2 kΩ. Su alcune varianti del modulo la numerazione delle resistenze cambia, quindi la misura vale più della sigla.

#### Come si calcola

Dal datasheet: **I_carica ≈ 1200 / R_PROG** (corrente in A, resistenza in Ω).

| R_PROG | Sigla SMD | Corrente di carica |
|---|---|---|
| 1,2 kΩ | 122 | ~1000 mA (default) |
| 2 kΩ | 202 | ~600 mA |
| 3 kΩ | 302 | ~400 mA |
| 4,7 kΩ | 472 | ~250 mA |
| 10 kΩ | 103 | ~120 mA |

#### Quale valore scegliere

- **Adatta la corrente al pannello.** Un pannello da 2–3 W a 6 V dà al massimo circa 330–500 mA in pieno sole. Se il TP4056 chiede 1 A, la tensione del pannello crolla e il caricatore lavora fuori specifica. Con questi pannelli un buon valore è **3 kΩ (circa 400 mA)**.
- **Meno calore.** A 1 A il TP4056 dissipa più di 1 W: in una scatola chiusa al sole d'estate diventa bollente ed entra in limitazione termica. A 400 mA il problema quasi scompare.
- **Più vita alla batteria.** Per una LiPo da 2000 mAh, 400 mA corrispondono a 0,2 C, una carica dolce. Fino a 1 A (0,5 C) va comunque bene.
- **Con l'USB.** Se ricarichi spesso da USB e vuoi fare in fretta, puoi lasciare 1,2 kΩ o scendere a 2 kΩ: è un compromesso tra velocità con l'USB e comportamento con il pannello.

#### Come si sostituisce

1. Scollega batteria, pannello e USB.
2. **Dissalda R3.** Metti un po' di stagno fresco su entrambi i lati, poi scaldali insieme con una punta larga, oppure alternando velocemente, e sposta la resistenza con una pinzetta. Con l'aria calda è ancora più semplice.
3. Pulisci le piazzole con la treccia dissaldante.
4. **Salda la nuova resistenza**, di solito formato 0603. Se non hai SMD, va bene anche una resistenza a filo saldata tra le due piazzole con reofori corti, oppure tra il pin 2 del TP4056 e un punto di GND.
5. Misura di nuovo col tester tra il pin 2 e GND per verificare il valore.
6. **Prova:** con la batteria parzialmente scarica, misura la corrente di carica con il tester in serie sul filo B+. Deve essere vicina al valore della tabella.

> ⚠️ **Non mettere una resistenza in parallelo a R3.** Due resistenze in parallelo hanno un valore **più basso**, quindi la corrente **aumenta**. Per ridurre la corrente R3 va sostituita, non affiancata.

### Stato di carica sull'ESP32

I pin **CHRG** e **STDBY** del TP4056 sono open-drain e pilotano i LED rosso e blu. Si possono collegare a due GPIO con `INPUT_PULLUP` per sapere dal software se la batteria è in carica o carica. I LED vanno rimossi.

### Consumi in deep sleep

Una DevKit ESP32 ha a bordo il LED di alimentazione e il convertitore USB-seriale, che consumano qualche mA anche in deep sleep. Per un'autonomia massima rimuovi il LED oppure usa un modulo ESP32 "nudo" o una scheda pensata per il basso consumo.

---

## Errori comuni da evitare

- ❌ **Alimentare il TP4056 dal pin 3V3 dell'ESP32**: 3,3 V non bastano per caricare una LiPo.
- ❌ **OUT+ direttamente sul 3V3**: fino a 4,2 V sull'ESP32.
- ❌ **Pannello e USB in parallelo senza diodi**: si alimentano a vicenda.
- ❌ **Regolatori della serie 78xx (es. 78L05)**: hanno un dropout troppo alto e una corrente a vuoto di qualche mA, quindi non sono adatti a una batteria.
- ❌ **Moduli boost 5 V "da power bank" per alimentare l'ESP32**: fanno una doppia conversione e hanno un consumo a vuoto alto.
- ❌ **Leggere la batteria su un pin non ADC** (es. GPIO23) o su un pin ADC2 con il WiFi attivo.

---

## Crediti

- Tutorial originale: [Random Nerd Tutorials, *Power ESP32/ESP8266 with Solar Panels and Battery*](https://randomnerdtutorials.com/power-esp32-esp8266-solar-panels-battery-level-monitoring/)
- Modifiche (doppia sorgente, load sharing, HT7833, partitore 100k/100k): questo repository.
