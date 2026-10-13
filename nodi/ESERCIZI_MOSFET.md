# Esercizi: dimensionamento di MOSFET come interruttori

Per svolgerli segui la procedura del capitolo 5 della [dispensa MOSFET](MOSFET.md#5-procedura-di-dimensionamento). Per ogni passo scrivi il calcolo, il risultato e se la verifica è superata. Se una verifica fallisce, spiega che cosa cambi e ripeti i passi interessati.

I parametri dei MOSFET vanno ricavati dai **datasheet** dei componenti che scegli: indica sempre il valore usato e la condizione a cui il datasheet lo garantisce (per esempio "R<sub>DS(on)</sub> massima a V<sub>GS</sub> = 2,5 V").

Le soluzioni sono nel capitolo 9 della [dispensa](MOSFET.md#9-esercizi-svolti).

---

## Esercizio 1 · Pompa per irrigazione a 12 V

Un ESP32 deve accendere e spegnere una pompa a 12 V DC per l'irrigazione.

| Dato | Valore |
|---|---|
| Alimentazione della pompa | 12 V, da un alimentatore separato con massa in comune con l'ESP32 |
| Corrente a regime | 1,2 A |
| Spunto all'avvio | 3 A per 100 ms |
| Comando | GPIO dell'ESP32 a 3,3 V |
| Temperatura ambiente massima | 40 °C |
| Caduta di tensione ammessa sull'interruttore | 0,2 V |
| Montaggio | Millefori, MOSFET in contenitore SOT-23 (R<sub>θJA</sub> = 200 °C/W) |

**Richieste**

1. Scegli la topologia (lato basso o lato alto) e motiva la scelta.
2. Ricava i limiti su V<sub>DS(max)</sub>, V<sub>GS(th)</sub> e R<sub>DS(on)</sub>, e scegli un MOSFET adatto.
3. Calcola la caduta di tensione, la potenza dissipata e la temperatura di giunzione a regime, nel caso peggiore.
4. Valuta lo spunto all'avvio.
5. Dimensiona le resistenze sul gate.
6. Disegna lo schema completo. Quali componenti servono oltre al MOSFET e alle resistenze? Perché?
7. Come vanno collegate le masse?

**Domanda di approfondimento.** Si vuole regolare la velocità della pompa in PWM a 20 kHz. Il circuito che hai progettato va ancora bene? Calcola le perdite di commutazione (Q<sub>g</sub> dal datasheet) e proponi una soluzione se necessario.

---

## Esercizio 2 · Modulo GSM SIM800L a batteria

Un ESP32 alimentato da una LiPo deve spegnere del tutto un modulo GSM SIM800L tra un invio e l'altro.

| Dato | Valore |
|---|---|
| Alimentazione | LiPo, da 3,6 a 4,2 V (sotto 3,6 V il firmware non usa il GSM) |
| Tensione minima richiesta dal SIM800L ai suoi pin | 3,4 V |
| Corrente del modulo | Picchi di 2 A per 577 µs ogni 4,6 ms durante la trasmissione, circa 100 mA a riposo |
| Condensatore sull'alimentazione del modulo | 1000 µF |
| Collegamento con l'ESP32 | UART (TX, RX) |
| Comando | GPIO dell'ESP32 a 3,3 V |
| Temperatura ambiente massima | 40 °C |
| Corrente di inrush ammessa all'accensione | 0,3 A |
| Montaggio | Millefori, MOSFET in contenitore SOT-23 (R<sub>θJA</sub> = 200 °C/W) |

**Richieste**

1. Scegli la topologia e motiva la scelta. Si può pilotare il MOSFET direttamente dal GPIO?
2. Ricava i limiti di tensione e scegli i MOSFET necessari.
3. Dimensiona le resistenze sul gate in modo che la |V<sub>GS</sub>| minima permetta di usare la R<sub>DS(on)</sub> garantita a 2,5 V.
4. Calcola il budget di caduta di tensione disponibile e verifica il MOSFET scelto. Se la verifica fallisce, proponi **almeno due** strade diverse per risolvere il problema.
5. Calcola la potenza dissipata e la temperatura di giunzione. Il fattore 1,5 sulla R<sub>DS(on)</sub> è giustificato in questo caso?
6. Dimensiona il soft-start per rispettare la corrente di inrush ammessa. Calcola l'energia dissipata nel MOSFET durante l'accensione.
7. Quali conseguenze ha il soft-start sullo spegnimento? Che cosa deve fare il software prima di togliere l'alimentazione al modulo?
8. Disegna lo schema completo.
