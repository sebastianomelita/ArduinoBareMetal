> [Torna ISM](../ism.md#2-parallelismo-del-gateway-ricezione-concorrente)

# **Accesso multiplo in LoRaWAN: FDM reale + CDM**

In una rete LoRaWAN il gateway deve ricevere contemporaneamente i messaggi di migliaia di dispositivi IoT distribuiti sul territorio. Per farlo sfrutta una combinazione di due tecniche di multiplazione: una **FDM reale** tra canali in frequenza e una **CDM basata sullo Spreading Factor** all'interno di ciascun canale.

---

## **Lato device (end-node): un canale alla volta**

Un singolo end-device LoRa dispone di **un solo trasmettitore radio**. Questo significa che in ogni istante può trasmettere su **una sola frequenza** con **un solo SF**. Non è capace di trasmettere bit in parallelo su più canali simultaneamente.

Tra un pacchetto e il successivo il device cambia frequenza in modo pseudo-casuale tra quelle disponibili (**frequency hopping tra pacchetti**), ma si tratta di una trasmissione sequenziale, non parallela. L'obiettivo del frequency hopping è distribuire il carico sui canali disponibili e ridurre le collisioni, non aumentare il throughput di un singolo device.

---

## **Lato gateway: 8 radio fisiche in parallelo**

Il gateway LoRaWAN non ha una sola radio che salta a round robin tra le 8 frequenze: ha **8 ricevitori fisici distinti e simultanei**. Questo è reso possibile dal chip concentratore **Semtech SX1301** (e successori), che integra in un singolo IC 8 canali di ricezione in parallelo, ciascuno sintonizzato su una frequenza diversa.

Un gateway che facesse round robin su una sola radio perderebbe tutti i pacchetti che arrivano mentre sta ascoltando le altre frequenze — inaccettabile con migliaia di device che trasmettono in modo asincrono e imprevedibile.

Il costo di 8 frontend radio non è proibitivo perché la complessità è interamente integrata nel chip concentratore, che ha un prezzo paragonabile a quello di un modulo radio singolo.

---

## **FDM: separazione in frequenza tra gli 8 canali**

I 8 canali uplink (regione EU868) occupano frequenze fisiche distinte, ad esempio:

| Canale | Frequenza  |
|:------:|:----------:|
| 0      | 868.1 MHz  |
| 1      | 868.3 MHz  |
| 2      | 868.5 MHz  |
| 3      | 867.1 MHz  |
| 4      | 867.3 MHz  |
| 5      | 867.5 MHz  |
| 6      | 867.7 MHz  |
| 7      | 867.9 MHz  |

La separazione tra questi canali è una vera **multiplazione FDM**: ogni canale occupa una porzione diversa dello spettro e le trasmissioni su frequenze diverse non si interferiscono fisicamente, esattamente come i canali radio tradizionali.

Ogni device sceglie pseudo-casualmente una di queste frequenze per ogni pacchetto trasmesso. Il gateway le riceve tutte contemporaneamente grazie agli 8 ricevitori fisici.

---

## **CDM: separazione per Spreading Factor all'interno di ogni canale**

All'interno di **ciascuno** degli 8 canali in frequenza, più dispositivi possono trasmettere **contemporaneamente sulla stessa frequenza** usando Spreading Factor diversi. Come visto nella modulazione CSS, segnali con SF differente sono **ortogonali** tra loro: il ricevitore riesce a separarli e decodificarli indipendentemente, anche se sovrapposti nel tempo e nella frequenza.

Questo è concettualmente analogo al **CDM (Code Division Multiplexing)**: la proprietà di separazione non è né il tempo né la frequenza, ma il **codice di spreading** (in questo caso lo SF). Ogni ricevitore fisico del gateway è quindi un **demodulatore multi-SF**, capace di decodificare in parallelo fino a 6 trasmissioni contemporanee sulla stessa frequenza (una per ogni SF da 7 a 12).

---

## **Capacità totale del gateway**

La combinazione delle due tecniche porta a una capacità di ricezione simultanea pari a:

```
8 canali (FDM) × 6 SF (CDM) = 48 demodulatori logici in parallelo
```

| | FDM (frequenza) | CDM (SF) | Demodulatori logici |
|:---:|:---:|:---:|:---:|
| 1 canale | 1 | 6 | 6 |
| 8 canali | 8 | 6 | **48** |

Ogni demodulatore logico può ricevere indipendentemente un pacchetto da un device diverso. In pratica la capacità reale è limitata anche dal **duty cycle** imposto dalla regolamentazione ISM (1% in EU868), che limita quanto spesso un device può trasmettere, non dalla capacità di ricezione del gateway.

---

## **Riepilogo: chi fa cosa**

| | End-device | Gateway |
|:---|:---:|:---:|
| Radio fisiche | **1** | **8** |
| Canali simultanei in TX/RX | 1 | 8 |
| SF simultanei per canale | 1 | fino a 6 |
| Demodulatori logici totali | 1 | fino a 48 |
| Frequency hopping | tra pacchetti (sequenziale) | — (ascolta tutto sempre) |

La **asimmetria** tra device e gateway è una scelta progettuale precisa: il device deve essere economico, a basso consumo e semplice; il gateway, che serve migliaia di device, può permettersi maggiore complessità hardware concentrata nel chip SX1301.

---

## **Fonti**

- **Semtech SX1301 datasheet** — chip concentratore con 8 canali paralleli integrati:
  <https://www.semtech.com/products/wireless-rf/lora-core/sx1301>

- **RF Essentials — LoRa SF orthogonality and 48 virtual channels**:
  <https://rfessentials.com/rf-knowledge-base/what-are-the-rf-requirements-for-a-lora-device-and-how-does-the-spreading-factor/>

- **LoRa Alliance — Regional Parameters RP002-1.0.2** (specifiche ufficiali EU868, frequenze obbligatorie, piano canali):
  <https://lora-alliance.org/wp-content/uploads/2020/11/RP_2-1.0.2.pdf>

- **The Things Network — EU868 regional parameters** (frequenze, DR, SF):
  <https://www.thethingsnetwork.org/docs/lorawan/regional-parameters/eu868/>

- **EdgePilot — EU868 frequency bands guide** (sotto-bande ETSI, duty cycle, canali mandatory):
  <https://edgepilot.io/news/lorawan-frequency-bands-europe-eu868-guide>

- **arXiv:1902.02866 — Analysis of LoRaWAN Uplink with Multiple Demodulating Paths** (analisi della capacità SX1301 con Nc canali paralleli):
  <https://arxiv.org/pdf/1902.02866>

- **arXiv:1912.01285 — A Complete LoRaWAN Model for Single-Gateway Scenarios** (ortogonalità degli SF, decodifica simultanea):
  <https://arxiv.org/pdf/1912.01285>

- **NCBI/PMC — Energy Constrained Optimization for SF Allocation in LoRaWAN** (ortogonalità imperfetta degli SF, gestione interferenze):
  <https://www.ncbi.nlm.nih.gov/pmc/articles/PMC7472179/>

- **Tektelic — The benefit of a 16-channel deployment in EU868**:
  <https://tektelic.com/expertise/the-benefit-of-a-16-channel-deployment/>

- **The Stunning Efficiency and Beauty of the Polyphase Channelizer**
  https://tomverbeure.github.io/2026/02/16/Polyphase-Channelizer.html

  - **Datasheet SX1301 v2.4 (PDF ufficiale Semtech, estratti verificati) — descrive i canali IF, la decimazione e l'architettura a livello funzionale:**
    https://docs.rakwireless.com/product-categories/wislink/rak831/datasheet/
- **FAQ Semtech — architettura 2×SX1257 + SX1301:**
  https://www.semtech.com/design-support/faq/faq-gateway
- Teoria del digital channelizer (il processo DSP sottostante, indipendente da LoRa) — fonte accademica classica: Crochiere & Rabiner, Multirate Digital Signal Processing, Prentice Hall 1983 — citato anche nel brevetto USPTO citato sopra.

- https://bxbsp.com/Tutorials/Tutorial1/Channelizer_Derivation.html
---
> [Torna ISM](../ism.md#2-parallelismo-del-gateway-ricezione-concorrente)
