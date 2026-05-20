> [Torna a reti di sensori](sensornetworkshort.md#interfaccia-radio)>[Torna banda ISM](ism.md)

# Ortogonalità degli Spreading Factor in LoRa

Una delle proprietà che rende LoRa adatto a reti con molti dispositivi è la **quasi-ortogonalità tra segnali con Spreading Factor diversi**. Questa dispensa analizza cosa significa, perché esiste, quando funziona e quando si rompe.

---

## 1. Il senso matematico di "ortogonalità"

Due segnali $s_1(t)$ e $s_2(t)$ si dicono **ortogonali** su un intervallo di tempo $T$ se il loro prodotto scalare è nullo:

$$\int_0^T s_1(t) \cdot s_2^*(t)\, dt = 0$$

Se due segnali sono ortogonali, un ricevitore che applica un filtro adattato a $s_1$ produce **zero in uscita** quando in ingresso c'è $s_2$. Ovvero: può estrarre $s_1$ dalla somma $s_1 + s_2$ senza essere disturbato da $s_2$.

Questo è il principio alla base di **CDMA**, di **OFDM** e — con qualche stella accanto — anche di LoRa.

---

## 2. Come è fatto un simbolo LoRa

Un simbolo LoRa è un **chirp**: una sinusoide la cui frequenza varia linearmente nel tempo. Le caratteristiche dipendono da due parametri:

- **BW** (*BandWidth*): l'escursione di frequenza, tipicamente 125, 250 o 500 kHz.
- **SF** (*Spreading Factor*): un intero da 7 a 12. Determina la **durata** del chirp.

La relazione tra durata di un simbolo $T_s$, SF e BW è:

$$T_s = \frac{2^{SF}}{BW}$$

| SF | $T_s$ con BW = 125 kHz |
|---|---|
| 7 | 1,02 ms |
| 8 | 2,05 ms |
| 9 | 4,10 ms |
| 10 | 8,19 ms |
| 11 | 16,38 ms |
| 12 | 32,77 ms |

A ogni "scalino" di SF la durata raddoppia, e quindi anche il bit rate si dimezza (a parità di BW).

### Il chirp ideale

Un *up-chirp* base ha la frequenza istantanea che cresce linearmente da $-BW/2$ a $+BW/2$ nell'arco di $T_s$. Matematicamente:

$$s_{SF}(t) = e^{j 2\pi \left( -\frac{BW}{2} t + \frac{BW^2}{2 \cdot 2^{SF}} t^2 \right)}$$

L'informazione del simbolo (i bit) è codificata come **shift ciclico** di questo chirp di base: ci sono $2^{SF}$ possibili shift, quindi un simbolo trasporta SF bit.

### Il de-chirp al ricevitore

Il ricevitore moltiplica il segnale ricevuto per un **down-chirp** (chirp inverso) con lo **stesso SF**. Il risultato:

- per un chirp con il "proprio" SF: diventa un **tono fisso**, la cui frequenza dipende dallo shift e quindi dal simbolo trasmesso. Una FFT estrae il valore.
- per un chirp con un **SF diverso**: il prodotto resta una specie di chirp residuo, che non si concentra in nessuna singola frequenza. Si diffonde come rumore su tutta la banda.

È questa proprietà che produce la quasi-ortogonalità.

---

## 3. Le quattro situazioni di sovrapposizione

Quando due trasmissioni si sovrappongono nel tempo, le combinazioni rilevanti di **frequenza** e **SF** sono quattro:

| # | Frequenza | SF | Esito | Meccanismo |
|---|---|---|---|---|
| 1 | diverse | qualunque | ✅ separabili | separazione naturale in frequenza |
| 2 | uguali | diverse | ⚠️ quasi separabili | ortogonalità degli SF (questa dispensa) |
| 3 | uguali | uguali | ❌ collisione o capture | capture effect se uno è molto più forte |
| 4 | diverse | uguali | ✅ separabili | separazione in frequenza, demodulatori paralleli |

Il caso **#2** è quello davvero peculiare di LoRa: stessa frequenza, SF diversi, e i segnali coesistono perché un demodulatore configurato per SF=N "non vede" i chirp con SF≠N.

---

## 4. Perché è "quasi"-ortogonalità e non vera ortogonalità

In teoria il prodotto scalare di due chirp con SF diversi è zero. In pratica no, per tre motivi.

### 4.1 Imperfezioni hardware

- **Offset di frequenza** tra TX e RX: gli oscillatori al quarzo hanno un errore tipico di ±10–40 ppm. A 868 MHz, 20 ppm sono ~17 kHz di scostamento, che fa "scivolare" il chirp.
- **Drift temporale** durante il simbolo (jitter del clock).
- **Non-linearità** della catena RF (amplificatori, mixer, filtri).
- **Effetto Doppler** se TX o RX sono in movimento.

Tutte queste imperfezioni rompono leggermente l'ortogonalità matematica e introducono una "perdita" di energia residua nel demodulatore del SF sbagliato.

### 4.2 Co-channel rejection nei datasheet Semtech

I datasheet dei moduli SX1276 / SX1262 riportano una grandezza chiamata **co-channel rejection**, cioè il margine di potenza minimo che il segnale utile deve avere rispetto a un interferente con SF diverso per essere demodulato correttamente.

Valori tipici (segnale utile a SF=N, interferente a SF=M):

| SF utile | SF interferente | Rejection (dB) |
|---|---|---|
| 7 | 8 | ~ -8 |
| 7 | 9 | ~ -11 |
| 7 | 10 | ~ -16 |
| 7 | 11 | ~ -18 |
| 7 | 12 | ~ -19 |
| 12 | 7 | ~ -16 |
| 12 | 8 | ~ -16 |

Numeri negativi significano che **l'utile può essere più debole dell'interferente** e farcela comunque. Per esempio: un SF7 utile a -120 dBm può essere demodulato anche con un SF12 interferente fino a -101 dBm, perché ha un margine di -19 dB.

Per questo si parla di "quasi"-ortogonalità: l'isolamento c'è ma non è infinito. Tipicamente 15-20 dB.

### 4.3 Imperfetta ortogonalità tra SF "vicini"

I valori in tabella mostrano un dettaglio importante: **due SF molto vicini hanno meno isolamento** (es. SF7 vs SF8: ~8 dB) rispetto a due SF lontani (es. SF7 vs SF12: ~19 dB). Quindi la quasi-ortogonalità è migliore per coppie di SF distanti, e peggiore per coppie adiacenti.

---

## 5. Quando l'ortogonalità si rompe davvero

Se l'interferente con SF diverso è **molto più potente** dell'utile, il margine di rejection viene superato e l'utile viene perso.

### Esempio pratico

Un device A vicino al gateway trasmette a SF7 a -50 dBm.
Un device B lontano dal gateway trasmette a SF12 a -120 dBm (al limite della sensibilità).
Entrambi sulla **stessa frequenza**, nello **stesso istante**.

Differenza di potenza al gateway: 70 dB.

Il demodulatore SF12 del gateway, che dovrebbe demodulare B, vede A come interferente residuo. La rejection SF12 vs SF7 è ~-16 dB: significa che B può sopravvivere fino a quando A non lo supera di 16 dB. Ma in questo caso A supera B di 70 dB, ben oltre il margine. **Il pacchetto di B viene perso.**

Questo fenomeno si chiama **near-far problem** ed è uno dei limiti pratici di LoRaWAN.

### Mitigazione: l'ADR

LoRaWAN risponde al near-far problem con l'**Adaptive Data Rate**: il Network Server osserva nel tempo gli RSSI/SNR di ogni device e gli "consiglia" parametri ottimali:

- Device vicini al gateway → SF basso, potenza bassa.
- Device lontani → SF alto, potenza alta.

L'obiettivo è **appiattire la dinamica di potenza** ricevuta al gateway, in modo che tutti i device arrivino con potenze relativamente simili, sotto il margine di co-channel rejection.

Non è una soluzione perfetta (il device deve obbedire all'ADR, che richiede di accettare i MAC commands; molte librerie open source non lo fanno), ma su reti gestite riduce significativamente il problema.

---

## 6. Cosa succede a parità di SF e frequenza

Qui non c'è ortogonalità da sfruttare. Due chirp con lo **stesso SF e stessa frequenza** si sommano nel canale, e il ricevitore vede una somma indistinguibile.

### Capture effect

Se uno dei due segnali è significativamente più forte dell'altro (tipicamente **~6 dB di margine**), il demodulatore aggancia il preambolo del più forte e demodula quello. Il più debole è perso, ma il più forte arriva con successo.

Il capture effect è uno dei meccanismi che salvano LoRaWAN nelle reti dense, dove molti device si "azzuffano" sullo stesso canale.

### Collisione vera

Se i due segnali hanno potenza paragonabile (entro ~6 dB), il demodulatore non aggancia stabilmente nessuno dei due preamboli, e si perdono entrambi.

---

## 7. Implicazioni di sistema

### Capacità del gateway

Un concentrator SX1301 / SX1302 ha **8 demodulatori paralleli**. La quasi-ortogonalità tra SF permette in linea di principio di ricevere fino a:

- 8 device su frequenze diverse, qualunque SF
- più device sulla **stessa** frequenza ma con SF diversi (grazie all'ortogonalità)

In pratica, la capacità è limitata dal near-far problem e dal rumore termico, ma teoricamente il gateway può servire molte decine di TX simultanee.

### Ruolo della randomizzazione temporale

Anche tra device con stesso SF e stesso canale, l'evento "collisione" è raro perché LoRaWAN:

- distribuisce le trasmissioni su 8 canali EU868
- non sincronizza i device tra loro
- ha device che trasmettono raramente (uno ogni N minuti)

La probabilità che due device sulla stessa frequenza, stesso SF, e con preamboli sovrapposti coincidano è bassa. Quando invece la rete diventa densa, le collisioni crescono **rapidamente** (modello Aloha: capacità massima teorica ~18% del canale).

### Importanza dell'ADR e di più gateway

Per scalare oltre questi limiti, due strategie:

- **ADR aggressivo** per distribuire i device su SF differenti e ridurre near-far.
- **Più gateway** in zona: ogni gateway può fungere da "secondo ricevitore" per i device lontani, riducendo il rischio che un pacchetto debole venga ucciso da un interferente forte. Il Network Server poi fa deduplicazione.

---

## 8. Riassunto in una tabella

| Scenario | Ortogonalità? | Esito |
|---|---|---|
| Frequenze diverse, SF qualunque | sì, totale (canale diverso) | entrambi ricevuti |
| Stessa freq, **SF diversi** | sì, **imperfetta** (15-20 dB rejection) | entrambi ricevuti se la dinamica di potenza è ragionevole |
| Stessa freq, **SF uguali**, potenze diverse (>6 dB) | no | capture effect: vince il più forte |
| Stessa freq, **SF uguali**, potenze simili | no | collisione: entrambi persi |

### Frase da ricordare

> **L'ortogonalità in LoRa esiste solo tra Spreading Factor diversi, ed è quasi-ortogonalità: dà ~15-20 dB di isolamento sulla stessa frequenza. Tra segnali con stesso SF e stessa frequenza non c'è alcuna ortogonalità.**

---

## 9. Riferimenti

- Datasheet Semtech SX1276 e SX1262 (sezioni "Co-channel rejection")
- Vangelista L., "*Frequency Shift Chirp Modulation: The LoRa Modulation*", IEEE Signal Processing Letters, 2017
- Reynders B., Pollin S., "*Chirp spread spectrum as a modulation technique for long range communication*", IEEE 2016
- Croce D., Gucciardo M., Tinnirello I., Garlisi D., Mangione S., "*Impact of LoRa Imperfect Orthogonality: Analysis of Link-Level Performance*", IEEE Communications Letters, 2018 — articolo di riferimento che misura sperimentalmente la non-perfetta ortogonalità degli SF.
- LoRa Alliance, "*LoRaWAN Regional Parameters v1.0.3 rev A*"
