# Dispensa: dimensionare P-MOSFET e N-MOSFET come interruttori

Questa dispensa riassume come scegliere e dimensionare un MOSFET usato come **interruttore** comandato da un microcontrollore (ESP32 a 3,3 V) in un circuito a batteria. Gli esempi numerici sono presi dal progetto descritto nel panel2esp32.md](panel2esp32.md): load sharing (Q1), partitore attivabile (Q2, Q3), alimentazione dell'SDS011 (Q4, Q5).

<p align="center">
  <img src="img/mosfet_interruttori.svg" alt="MOSFET come interruttori: low-side e high-side" width="100%">
</p>

---

## 1. Il MOSFET in breve

Un MOSFET ha tre terminali:

- **Gate (G):** il comando. Non assorbe corrente in modo continuo: si comporta come un piccolo condensatore che va caricato e scaricato.
- **Drain (D)** e **Source (S):** i due capi dell'interruttore. Quando il MOSFET è acceso, tra loro c'è una resistenza piccolissima, la **R<sub>DS(on)</sub>**.

A comandarlo è la tensione **tra gate e source (V<sub>GS</sub>)**, non la tensione del gate rispetto a massa. Questo è il punto da cui dipende tutto il resto.

| | N-MOSFET | P-MOSFET |
|---|---|---|
| Si accende quando | il gate è **più alto** del source (V<sub>GS</sub> > 0) | il gate è **più basso** del source (V<sub>GS</sub> < 0) |
| Source collegato tipicamente a | **GND** | **V+** (alimentazione) |
| Posizione tipica | **lato basso**: tra carico e massa | **lato alto**: tra alimentazione e carico |
| A parità di dimensioni | R<sub>DS(on)</sub> più bassa | R<sub>DS(on)</sub> un po' più alta |

Ogni MOSFET contiene anche un **diodo interno (body diode)** tra drain e source:
- nel **N-MOSFET** l'anodo è sul source e il catodo sul drain;
- nel **P-MOSFET** l'anodo è sul drain e il catodo sul source.

Anche con il MOSFET spento, questo diodo conduce se la tensione ha il verso "giusto". Bisogna sempre tenerne conto quando si sceglie l'orientamento.

---

## 2. Lato basso o lato alto?

### N-MOSFET lato basso (schema 1)

Il carico sta tra V+ e il drain, il source va a GND. Il GPIO comanda direttamente il gate.

- ✅ È il più semplice: basta un componente, perché V<sub>GS</sub> = tensione del GPIO.
- ✅ Gli N-MOSFET hanno R<sub>DS(on)</sub> più basse e costano meno.
- ❌ Interrompe la **massa** del carico. Quando è spento, il carico "galleggia" verso V+.

Va bene per carichi "stupidi": LED, relè, motori, ventole, buzzer.

### P-MOSFET lato alto con driver N (schema 2)

Il P-MOSFET sta tra V+ e il carico, che resta collegato a GND. Un piccolo N-MOSFET comandato dal GPIO porta il gate del P a massa per accenderlo.

- ✅ Il carico resta **sempre riferito a GND**. È indispensabile quando il carico ha linee di segnale verso l'ESP32 (UART, I²C, ADC).
- ✅ Il driver N permette di commutare tensioni più alte di quella del GPIO.
- ❌ Servono due transistor e una resistenza in più.

> ⚠️ **Perché non si può usare un N lato basso per un sensore con UART o I²C.** Con la massa interrotta, il sensore cerca un'altra strada verso GND e la trova attraverso le linee di segnale e i diodi di protezione dei pin dell'ESP32. Il risultato sono letture sbagliate, consumi anomali o pin danneggiati. È lo stesso motivo per cui nel partitore attivabile l'interruttore è sul lato batteria: un N verso massa lascerebbe GPIO33 "appeso" a VBAT attraverso R1, con fino a 4,2 V sul pin.

### Perché non pilotare il P direttamente dal GPIO?

Per spegnere un P-MOSFET il gate deve arrivare alla stessa tensione del source. Se il source è a VBAT (fino a 4,2 V) e il GPIO arriva solo a 3,3 V, resta V<sub>GS</sub> ≈ –0,9 V: abbastanza vicino alla soglia da lasciare il P **mezzo acceso**. Il driver N risolve il problema: da spento lascia che R<sub>GS</sub> porti il gate esattamente a V+.

Si può pilotare il P direttamente dal GPIO solo se V+ ≤ 3,3 V, cioè se l'alimentazione da commutare non supera quella dell'ESP32.

---

## 3. I parametri del datasheet, uno per uno

### 3.1 V<sub>DS(max)</sub>: tensione massima drain-source

È la tensione massima che il MOSFET regge da spento.
**Regola:** V<sub>DS(max)</sub> ≥ **2 ×** la tensione più alta del circuito, per assorbire picchi e transitori.
*Nel progetto:* al massimo circa 6,5 V (pannello a vuoto). Un MOSFET da 20–30 V va benissimo.

### 3.2 I<sub>D</sub>: corrente di drain

È la corrente continua massima.
**Regola:** I<sub>D</sub> ≥ **2–3 ×** la corrente del carico, **picchi compresi** (spunto di motori e ventole, carica dei condensatori, picchi del WiFi).
Il valore in prima pagina del datasheet è misurato con il contenitore tenuto a 25 °C. Su un SOT-23 saldato su millefori la corrente reale sostenibile è molto più bassa: conta la verifica della potenza (punto 3.5).

### 3.3 V<sub>GS(th)</sub>: tensione di soglia

È la tensione a cui il MOSFET **comincia** a condurre, tipicamente con una corrente minima di 250 µA.

⚠️ **Non è la tensione a cui il MOSFET è acceso del tutto.** A V<sub>GS(th)</sub> conduce appena. È l'errore più comune.

**Regola:** la |V<sub>GS</sub>| disponibile deve essere almeno **2–2,5 ×** la |V<sub>GS(th)</sub>| **massima** (non quella tipica). Con il GPIO a 3,3 V, oppure con una LiPo quasi scarica a circa 3,0 V, servono MOSFET **logic level**, con V<sub>GS(th)</sub> massima intorno a 1–1,5 V.

### 3.4 R<sub>DS(on)</sub>: resistenza da acceso

È la resistenza tra drain e source con il MOSFET acceso. Il datasheet la riporta **a diverse V<sub>GS</sub>**, per esempio a 10 V, 4,5 V e 2,5 V.

**Regola:** usa il valore misurato alla V<sub>GS</sub> **che hai davvero** nel circuito. Se il datasheet la specifica solo a 10 V, il MOSFET non è adatto a 3,3 V.

Da R<sub>DS(on)</sub> ricavi:

- **la caduta di tensione:** ΔV = I × R<sub>DS(on)</sub>
- **la potenza dissipata:** P = I² × R<sub>DS(on)</sub>

Tieni conto che R<sub>DS(on)</sub> **aumenta con la temperatura**: a 100 °C è circa 1,5 volte il valore a 25 °C.

### 3.5 Potenza e temperatura

Un SOT-23 su una scheda senza grandi aree di rame dissipa senza problemi **qualche centinaio di mW**.
**Regola pratica:** sotto **100 mW** non serve pensarci. Sopra i 200–300 mW conviene un contenitore più grande (SOT-89, SOT-223, DPAK) o un MOSFET con R<sub>DS(on)</sub> più bassa.

### 3.6 V<sub>GS(max)</sub>: tensione massima gate-source

Il gate è fragile: oltre questa tensione l'isolante si rompe. Tipicamente vale ±8, ±12 o ±20 V.
**Regola:** la |V<sub>GS</sub>| applicata, cioè la tensione di alimentazione nel caso del P lato alto, deve restare sotto V<sub>GS(max)</sub> con margine. Con la batteria a 4,2 V e il pannello a 6,5 V, ±12 V basta e avanza.

### 3.7 Body diode

Controlla sempre in che verso conduce e se può creare percorsi indesiderati:

- in un **interruttore lato alto** (P con source a V+), il body diode non conduce da V+ verso il carico: il MOSFET spento isola davvero il carico;
- nel **load sharing** (Q1) il P è montato **al contrario** (drain verso la batteria) proprio per sfruttare il body diode: conduce dalla batteria verso il carico e blocca il verso opposto.

---

## 4. Le resistenze attorno al gate

### Pull-down (N) o pull-up gate-source (P)

Durante il reset, durante il boot e in deep sleep i GPIO dell'ESP32 sono **flottanti**: senza una resistenza il gate prende una tensione casuale.

- **R<sub>PD</sub>** tra gate e source dell'N: lo tiene **spento** finché il GPIO non lo comanda.
- **R<sub>GS</sub>** tra gate e source del P: lo tiene **spento** finché il driver N non tira il gate a massa.

**Valore:** **10–100 kΩ**. Nei circuiti a batteria conviene 100 kΩ: quando il driver è acceso, R<sub>GS</sub> consuma I = V+/R, cioè circa 42 µA a 4,2 V con 100 kΩ, contro 420 µA con 10 kΩ.

### Resistenza in serie al gate (R<sub>G</sub>)

Una resistenza da **100–470 Ω** tra GPIO e gate:
- limita il picco di corrente con cui il GPIO carica la capacità del gate;
- smorza eventuali oscillazioni con fili lunghi.

Per interruttori lenti (pochi cambi al secondo) e fili corti è **facoltativa**. Nel driver N di uno schema high-side, che pilota solo un gate, si può omettere.

### Accensione lenta (soft-start)

Se il carico ha grossi condensatori, all'accensione il MOSFET li carica con un picco di corrente (**inrush**) che può far scendere la tensione della batteria e resettare l'ESP32. Attenzione al caso del **boost**: tra ingresso e uscita c'è un percorso diretto attraverso induttore e diodo, quindi anche il condensatore d'uscita (C6 per l'SDS011) si carica di colpo attraverso il MOSFET.

Il metodo classico per rallentare l'accensione nello schema high-side ha due componenti:

- una resistenza **R<sub>S</sub>** (circa 10 kΩ) tra il drain del driver N e il gate del P, così il gate non viene scaricato di colpo;
- un condensatore **C<sub>GD</sub>** tra **gate e drain** del P (non tra gate e source). Durante l'accensione la tensione del gate resta quasi ferma (il cosiddetto *plateau di Miller*) e la tensione sul carico sale con una pendenza fissata da R<sub>S</sub> e C<sub>GD</sub>.

> ⚠️ Un condensatore tra gate e source **non** serve allo scopo: il driver N lo scarica in pochi microsecondi, e il P si accende di colpo lo stesso.

Il calcolo dei valori è nella [procedura di dimensionamento](#5-procedura-di-dimensionamento), al passo 7. In alternativa esistono **load switch integrati** (per esempio la famiglia TPS22xxx di Texas Instruments): contengono MOSFET, driver e soft-start regolabile con un solo condensatore.

---

## 5. Procedura di dimensionamento

### 5.1 Chi comanda e chi segue

Nel dimensionamento le grandezze non sono tutte sullo stesso piano. Conviene dividerle in tre gruppi.

**Dati di progetto (imposti dal problema, non si scelgono):**

| Grandezza | Simbolo | Esempio (Q4, SDS011) |
|---|---|---|
| Tensione da commutare, minima e massima | V+<sub>min</sub>, V+<sub>max</sub> | 3,0–4,2 V (batteria) |
| Corrente del carico: continua, di picco, ciclo di lavoro | I<sub>L</sub>, I<sub>pk</sub>, D | 110 mA, per 40 s ogni 30 min |
| Tensione di comando disponibile | V<sub>drive</sub> | GPIO 3,3 V |
| Temperatura ambiente massima | T<sub>a</sub> | 60 °C (scatola al sole) |
| Caduta di tensione ammessa | ΔV<sub>max</sub> | 50 mV |
| Capacità del carico | C<sub>L</sub> | ~1000 µF (C6 attraverso il boost) |
| Consumo ammesso a riposo | I<sub>q,max</sub> | ~1 µA a interruttore spento |
| Il carico ha linee di segnale? | — | Sì (UART) |

**Variabili di progetto (le scegli tu: sono quelle che "comandano"):**

- la **topologia** (N lato basso o P lato alto);
- il **MOSFET**: scegliendo un modello scegli insieme *tutti* i suoi parametri (V<sub>DS(max)</sub>, V<sub>GS(th)</sub>, R<sub>DS(on)</sub>, R<sub>θJA</sub>, capacità…). Non li puoi regolare uno per uno: li prendi in blocco da un catalogo;
- le **resistenze di gate** (R<sub>GS</sub> o R<sub>PD</sub>, R<sub>S</sub>);
- l'eventuale **condensatore di soft-start** (C<sub>GD</sub>).

**Grandezze dipendenti (si calcolano e si verificano, non si impongono):**

- la V<sub>GS</sub> effettiva nel caso peggiore;
- la R<sub>DS(on)</sub> effettiva, che dipende da V<sub>GS</sub> e dalla temperatura;
- la caduta ΔV e la potenza dissipata P;
- la temperatura di giunzione T<sub>j</sub>;
- il consumo delle resistenze di gate, acceso e spento;
- la velocità di accensione e spegnimento, e quindi la corrente di inrush.

Il dimensionamento quindi **non è un calcolo diretto** dai dati al componente, ma un ciclo **scegli → verifica → correggi**. Dai dati si ricavano dei **limiti** sui parametri (per esempio "R<sub>DS(on)</sub> al massimo 0,3 Ω a V<sub>GS</sub> = –2,5 V"), si sceglie un MOSFET dal catalogo che li rispetti, e poi si verificano le grandezze dipendenti. Se una verifica fallisce si torna indietro e si cambia una scelta.

C'è un solo punto in cui due grandezze dipendenti si influenzano a vicenda: **R<sub>DS(on)</sub> cresce con la temperatura, e la temperatura cresce con la potenza**, che a sua volta dipende da R<sub>DS(on)</sub>. In pratica si risolve in uno dei due modi:
- si usa subito il valore peggiore (R<sub>DS(on)</sub> a 25 °C × 1,5), oppure
- si fa un'iterazione: si calcola T<sub>j</sub> con il valore a 25 °C, si corregge R<sub>DS(on)</sub> e si ricalcola. Di solito basta un passaggio.

### 5.2 Esistono procedure riconosciute?

Sì. Le *application note* dei produttori di MOSFET (Infineon, Nexperia, onsemi, Texas Instruments, Vishay e altri) seguono tutte, con piccole differenze, la stessa sequenza: **tensione → pilotaggio del gate → conduzione → temperatura → transitori (SOA) → commutazione**. Nell'industria si aggiungono due pratiche:

- il **derating**: non si usa mai un componente al suo limite. Per esempio si tiene la tensione sotto il 75–80 % di V<sub>DS(max)</sub> e la temperatura di giunzione sotto i 100–110 °C anche se il datasheet ammette 150 °C. Le percentuali precise variano da azienda ad azienda;
- l'**analisi del caso peggiore** (*worst-case analysis*): si usano sempre i valori **massimi** o **minimi** del datasheet, mai quelli tipici, e si combinano con le condizioni peggiori del circuito (batteria scarica, temperatura massima).

### 5.3 Formule esatte o stime a occhio?

Per un **interruttore statico** (si accende e si spegne raramente, come tutti quelli del progetto) le formule sono **semplici e chiuse**, tutte in continua. Le trovi nei passi qui sotto.

Il punto debole non sono le formule ma **i dati**: i parametri del datasheet hanno dispersioni ampie (V<sub>GS(th)</sub> può variare di un fattore 2–3 tra un esemplare e l'altro), e la resistenza termica dipende moltissimo da quanto rame c'è attorno al componente. Per questo:

- **lontano dai limiti**, le stime "a occhio" con un margine generoso (le regole 2× e 2,5× del capitolo 3) sono legittime e sono quello che fanno anche i professionisti. Se la potenza calcolata è il 2 % di quella sopportabile, una precisione del 30 % non cambia nulla;
- **vicino ai limiti** (potenza oltre un terzo del massimo, V<sub>GS</sub> appena sopra la soglia, grossi transitori) si fa il calcolo completo nel caso peggiore, si controllano i grafici del datasheet (R<sub>DS(on)</sub> in funzione di V<sub>GS</sub> e T<sub>j</sub>, SOA) e si **misura sul prototipo**. Per i transitori i produttori forniscono anche modelli SPICE.

Le formule in commutazione (perdite di switching, carica del gate) servono solo con il PWM, cioè con migliaia di commutazioni al secondo. Per un interruttore che si accende ogni mezz'ora sono trascurabili: P<sub>sw</sub> ≈ ½ · V · I · (t<sub>r</sub> + t<sub>f</sub>) · f, con f di circa 0,0006 Hz, dà valori di nanowatt.

### 5.4 La procedura passo per passo

**Passo 0 — Raccogli i dati di progetto** (tabella del punto 5.1), sempre nei valori peggiori.

**Passo 1 — Scegli la topologia.**
Il carico ha linee di segnale verso il microcontrollore? → P lato alto con driver N. Altrimenti → N lato basso.

**Passo 2 — Limiti di tensione** (vincoli sul MOSFET):

- V<sub>DS(max)</sub> ≥ 1,5–2 × V+<sub>max</sub>
- |V<sub>GS(max)</sub>| ≥ 1,5 × |V<sub>GS</sub> applicata|

**Passo 3 — Pilotaggio del gate.** Calcola la |V<sub>GS</sub>| **minima** che il circuito garantisce:

- N lato basso: V<sub>GS,min</sub> = V<sub>OH,min</sub> del GPIO. Per l'ESP32 vale almeno 0,8 × 3,3 ≈ 2,6 V con il pin sotto carico; con il solo gate da pilotare si resta vicini a 3,3 V.
- P lato alto con driver e R<sub>S</sub>: |V<sub>GS,min</sub>| = V+<sub>min</sub> · R<sub>GS</sub> / (R<sub>GS</sub> + R<sub>S</sub>)

Vincoli: |V<sub>GS,min</sub>| ≥ 2–2,5 × |V<sub>GS(th),max</sub>|, e il datasheet deve garantire la R<sub>DS(on)</sub> a una |V<sub>GS</sub>| **non superiore** a |V<sub>GS,min</sub>|.

**Passo 4 — Conduzione.** Dai dati ricavi il limite su R<sub>DS(on)</sub>:

- dalla caduta ammessa: R<sub>DS(on)</sub> ≤ ΔV<sub>max</sub> / I<sub>pk</sub>
- R<sub>DS(on)</sub> effettiva: R<sub>DS(on)</sub>(T<sub>j</sub>) ≈ R<sub>DS(on)</sub>(25 °C) × k<sub>T</sub>, con k<sub>T</sub> ≈ 1,3–1,5 a 100–125 °C (si legge sul grafico "R<sub>DS(on)</sub> normalizzata in funzione di T<sub>j</sub>")
- potenza: P = I<sub>rms</sub>² × R<sub>DS(on)</sub>(T<sub>j</sub>). Per impulsi rettangolari di ampiezza I<sub>pk</sub> e ciclo di lavoro D: I<sub>rms</sub> = I<sub>pk</sub> × √D. Per un interruttore che resta acceso per decine di secondi, però, va considerato acceso in modo continuo: la temperatura fa in tempo ad andare a regime.

**Passo 5 — Temperatura:**

T<sub>j</sub> = T<sub>a</sub> + P × R<sub>θJA</sub> ≤ T<sub>j,derating</sub> (per esempio 100 °C)

R<sub>θJA</sub> è sul datasheet, ma è misurata su una scheda di prova standard. Su millefori con poco rame usa un valore prudente: per un SOT-23, **200–250 °C/W**. Al contrario, la potenza massima ammessa è P<sub>max</sub> = (T<sub>j,derating</sub> – T<sub>a</sub>) / R<sub>θJA</sub>.

**Passo 6 — Resistenze di gate:**

- limite inferiore, dal consumo: R<sub>GS</sub> ≥ V+<sub>max</sub> / I<sub>q,max</sub>, dove I<sub>q,max</sub> è il consumo ammesso **mentre il driver è acceso**. A interruttore spento R<sub>GS</sub> non consuma nulla;
- limite superiore, dalle perdite del gate: I<sub>GSS,max</sub> × R<sub>GS</sub> ≪ |V<sub>GS(th),min</sub>|. Con I<sub>GSS</sub> = 100 nA e 100 kΩ si ha 10 mV: va bene. Con 10 MΩ si avrebbe 1 V: il MOSFET potrebbe accendersi da solo.

In pratica restano valori tra **10 kΩ e 1 MΩ**, e 100 kΩ è il compromesso abituale.

**Passo 7 — Transitori di accensione (inrush e SOA):**

- corrente di inrush: I<sub>inrush</sub> = C<sub>L</sub> × dV/dt
- fissata la corrente ammessa, la pendenza necessaria è dV/dt = I<sub>inrush,max</sub> / C<sub>L</sub>
- con il condensatore gate-drain: dV/dt ≈ I<sub>G</sub> / C<sub>GD</sub>, dove la corrente del gate durante il plateau è
  I<sub>G</sub> ≈ (V+ – |V<sub>pl</sub>|) / R<sub>S</sub> – |V<sub>pl</sub>| / R<sub>GS</sub>
  e |V<sub>pl</sub>| (tensione di plateau) è poco sopra |V<sub>GS(th)</sub>|, circa 1,5–2 V per i MOSFET logic level
- quindi C<sub>GD</sub> = I<sub>G</sub> / (dV/dt)
- **energia dissipata nel MOSFET** durante la carica: E ≈ ½ × C<sub>L</sub> × V+². È un risultato generale: caricando un condensatore da una sorgente di tensione si dissipa sempre questa energia nel percorso resistivo, qualunque sia la velocità. La potenza media sul tempo di salita è P ≈ E / t<sub>salita</sub>: va confrontata con il grafico **SOA** del datasheet per impulsi di quella durata.
- il condensatore rallenta anche lo **spegnimento**: t<sub>off</sub> ≈ C<sub>GD</sub> × V+ × R<sub>GS</sub> / |V<sub>pl</sub>|. Se in quel momento il carico assorbe ancora corrente, il MOSFET la porta in zona lineare e scalda: conviene **mettere a riposo il carico prima di togliergli l'alimentazione**.

**Passo 8 — Stato spento:**

I<sub>off</sub> ≈ I<sub>DSS</sub> (perdita del MOSFET, tipicamente < 1 µA) più eventuali percorsi attraverso le linee di segnale, che vanno messe in alta impedenza.

**Passo 9 — Verifica sul banco:**

- ΔV misurata con il tester ai capi del MOSFET sotto carico;
- temperatura al tatto o con una termocamera dopo qualche minuto;
- con l'oscilloscopio, la forma della tensione sul carico all'accensione e la tensione della batteria, per vedere l'inrush.

### 5.5 Esempio completo: Q4 per l'SDS011, con soft-start

Valori dell'AO3401A **indicativi**, da verificare sul datasheet del produttore esatto: V<sub>DS(max)</sub> = –30 V, V<sub>GS(max)</sub> = ±12 V, V<sub>GS(th),max</sub> = –1,3 V, R<sub>DS(on)</sub> ≈ 0,1 Ω a V<sub>GS</sub> = –2,5 V. R<sub>θJA</sub> = 200 °C/W (valore prudente su millefori).

| Passo | Calcolo | Esito |
|---|---|---|
| 2 · Tensione | 30 V ≥ 2 × 4,2 V; 12 V ≥ 1,5 × 4,2 V | ✅ |
| 3 · Gate | Con R<sub>GS</sub> = 100 kΩ e R<sub>S</sub> = 10 kΩ: \|V<sub>GS,min</sub>\| = 3,0 × 100/110 = 2,73 V. Rapporto 2,73 / 1,3 = 2,1; R<sub>DS(on)</sub> garantita a 2,5 V ≤ 2,73 V | ✅ (al limite inferiore della regola: accettabile perché la corrente è piccola) |
| 4 · Conduzione | Progetto con I<sub>pk</sub> = 150 mA (margine sui 110 mA). R<sub>DS(on)</sub> nel caso peggiore = 0,1 × 1,5 = 0,15 Ω. ΔV = 0,15 × 0,15 = 22 mV ≤ 50 mV. P = 0,15² × 0,15 = 3,4 mW | ✅ |
| 5 · Temperatura | T<sub>j</sub> = 60 + 0,0034 × 200 = 60,7 °C. Nessuna iterazione necessaria | ✅ ampio margine |
| 6 · Resistenze | Consumo con il driver acceso: 4,2 V / 100 kΩ = 42 µA (solo per 40 s). Perdite gate: 100 nA × 100 kΩ = 10 mV ≪ 0,5 V | ✅ |
| 7 · Inrush | Obiettivo I<sub>inrush</sub> ≤ 0,25 A con C<sub>L</sub> = 1000 µF → dV/dt = 0,25 V/ms. I<sub>G</sub> ≈ (4,2 – 1,5)/10 k – 1,5/100 k ≈ 0,26 mA. C<sub>GD</sub> = 0,26 mA / (0,25 V/ms) ≈ 1 µF. Salita in circa 17 ms. E = ½ × 1000 µF × 4,2² ≈ 8,8 mJ → P media ≈ 0,5 W per 17 ms | ✅ da verificare sul grafico SOA (curva 10 ms) |
| 7 · Spegnimento | t<sub>off</sub> ≈ 1 µF × 4,2 V × 100 kΩ / 1,5 V ≈ 0,3 s | ⚠️ Lento: prima di spegnere, metti l'SDS011 in sleep con il comando seriale |
| 8 · Spento | I<sub>DSS</sub> < 1 µA, linea TX in alta impedenza | ✅ |

**Cosa insegna l'esempio:**

- in **conduzione** l'AO3401A è sovradimensionato di due ordini di grandezza: qui la stima a occhio sarebbe bastata;
- il passo che **decide davvero** è il 7, il transitorio di accensione. Senza soft-start la corrente di inrush, limitata solo da poche decine di milliohm, arriverebbe a qualche ampere;
- ogni scelta ha un prezzo: C<sub>GD</sub> risolve l'inrush ma rallenta lo spegnimento, e quindi cambia il software (il sensore va messo in sleep prima di togliergli l'alimentazione).

---

## 6. Esempi dal progetto

### Q4 + Q5: alimentazione dell'SDS011

| Dato | Valore |
|---|---|
| Tensione | VBAT 3,0–4,2 V |
| Corrente | ~110 mA in funzionamento, picco di qualche centinaio di mA all'avvio del boost |

**Q4 (P, AO3401A)**
- V<sub>DS(max)</sub> 30 V ≥ 2 × 4,2 V ✅
- V<sub>GS</sub> disponibile: da –3,0 V (batteria scarica) a –4,2 V. V<sub>GS(th)</sub> massima circa –1,3 V → rapporto ≥ 2,3 ✅
- R<sub>DS(on)</sub> a –2,5 V circa 0,1 Ω, a –4,5 V circa 0,06 Ω (valori indicativi da datasheet)
- ΔV = 0,11 A × 0,1 Ω ≈ **11 mV**, P = 0,11² × 0,1 ≈ **1,2 mW** ✅

**Q5 (N, 2N7002)**: pilota solo il gate di Q4
- Corrente: quella di R6, 4,2 V / 100 kΩ = **42 µA**
- V<sub>GS</sub> = 3,3 V dal GPIO, V<sub>GS(th)</sub> massima 2,5 V → rapporto 1,3. È basso, ma con 42 µA anche una R<sub>DS(on)</sub> di qualche ohm dà una caduta di pochi decimi di millivolt ✅

> 💡 Un driver che deve solo "tirare" un gate porta correnti di microampere: va bene quasi qualsiasi piccolo N-MOSFET. Lo stesso 2N7002 **non** andrebbe bene per commutare direttamente l'SDS011: 110 mA × qualche ohm farebbe cadere alcuni decimi di volt, e la corrente sarebbe vicina al suo massimo.

### Q2 + Q3: partitore attivabile

| Dato | Valore |
|---|---|
| Corrente | VBAT / 200 kΩ ≈ **21 µA** |

Qualsiasi P-MOSFET logic level va bene: ΔV = 21 µA × 0,1 Ω ≈ 2 µV, del tutto trascurabile. Qui la scelta si fa sulla **soglia** (accensione piena con la batteria scarica) e sulla **corrente di perdita da spento** (I<sub>DSS</sub>, tipicamente sotto 1 µA), non su R<sub>DS(on)</sub>.

### Q1: load sharing

| Dato | Valore |
|---|---|
| Corrente | ~80 mA media con il WiFi, picchi di circa 400 mA all'avvio del WiFi |

- ΔV di picco = 0,4 A × 0,08 Ω ≈ **32 mV**, P di picco ≈ **13 mW** ✅
- Confronto con un diodo Schottky al suo posto: circa 0,35 V di caduta, cioè il 10 % della tensione della batteria buttato in calore. Ecco perché qui serve un MOSFET.

### Controesempio: IRF9540 al posto dell'AO3401A

- V<sub>GS(th)</sub> da –2 a –4 V, e R<sub>DS(on)</sub> specificata solo a –10 V.
- Con la batteria a 3,5 V, V<sub>GS</sub> ≈ –3,5 V è **sotto** la soglia massima: il MOSFET potrebbe non accendersi affatto, o restare mezzo acceso e scaldare.
- ❌ È un ottimo MOSFET per 12–24 V, ma non è logic level.

---

## 7. MOSFET consigliati (SOT-23, logic level)

Valori **indicativi**: verifica sempre sul datasheet del produttore esatto, perché lo stesso nome può avere specifiche leggermente diverse.

| Modello | Tipo | V<sub>DS</sub> | I<sub>D</sub> | V<sub>GS(th)</sub> | R<sub>DS(on)</sub> indicativa | Uso tipico |
|---|---|---|---|---|---|---|
| **AO3401A** | P | –30 V | ~–4 A | –0,5…–1,3 V | ~60 mΩ a –4,5 V, ~100 mΩ a –2,5 V | Interruttore lato alto, load sharing |
| **DMG2305UX** | P | –20 V | ~–4 A | –0,5…–0,9 V | ~50 mΩ a –4,5 V | Alternativa all'AO3401A |
| **AO3400A** | N | 30 V | ~5 A | 0,65…1,45 V | ~30 mΩ a 4,5 V, ~50 mΩ a 2,5 V | Interruttore lato basso per carichi fino a qualche ampere |
| **2N7002** | N | 60 V | ~0,1–0,3 A | 1…2,5 V | qualche Ω | **Solo** driver di gate o segnali |
| **BSS138** | N | 50 V | ~0,2 A | 0,8…1,5 V | qualche Ω | Driver di gate, traslatori di livello |

---

## 8. Checklist di dimensionamento

1. **Posizione:** il carico ha linee di segnale verso l'ESP32? → **P lato alto** con driver N. Altrimenti → **N lato basso**.
2. **V<sub>DS(max)</sub>** ≥ 2 × la tensione massima del circuito.
3. **I<sub>D</sub>** ≥ 2–3 × la corrente del carico, picchi compresi.
4. **Logic level:** la |V<sub>GS</sub>| che hai davvero (3,3 V dal GPIO, o la batteria scarica) è ≥ 2–2,5 × la |V<sub>GS(th)</sub>| massima.
5. **R<sub>DS(on)</sub>** letta alla V<sub>GS</sub> reale → calcola ΔV = I × R<sub>DS(on)</sub> e verifica che sia accettabile.
6. **Potenza** P = I² × R<sub>DS(on)</sub> → sotto 100 mW nessun problema in SOT-23.
7. **V<sub>GS(max)</sub>** > la tensione di alimentazione commutata, con margine.
8. **Body diode:** controlla in che verso conduce, e che non crei percorsi indesiderati.
9. **Pull-down / pull-up** sul gate da 100 kΩ, per uno stato definito in reset e deep sleep.
10. **Picco di accensione:** se il carico ha grossi condensatori, valuta il soft-start.
11. **Linee di segnale:** prima di spegnere un carico, metti in ingresso (alta impedenza) i GPIO collegati a lui, per non alimentarlo attraverso i segnali.

---

## 9. Esercizi svolti

Due esercizi complementari, uno per configurazione. Ognuno contiene una **trappola** che costringe a ragionare invece di applicare le formule a memoria. La sola consegna, da distribuire senza soluzioni, è in [ESERCIZI_MOSFET.md](ESERCIZI_MOSFET.md).

I valori dei MOSFET sono **indicativi**: in un'esercitazione vera vanno ricavati dal datasheet del produttore, e questo fa parte dell'esercizio.

### Esercizio 1 · Low-side: pompa per irrigazione a 12 V

<p align="center">
  <img src="img/es1_lowside_pompa.svg" alt="Esercizio 1: N-MOSFET lato basso per pompa 12 V" width="100%">
</p>

**Dati**

| Dato | Valore |
|---|---|
| Alimentazione della pompa | 12 V (alimentatore separato, masse in comune) |
| Corrente a regime | 1,2 A |
| Spunto all'avvio | 3 A per 100 ms |
| Comando | GPIO a 3,3 V |
| Temperatura ambiente massima | 40 °C |
| Caduta ammessa sul MOSFET | 0,2 V |

**Svolgimento**

| Passo | Calcolo | Esito |
|---|---|---|
| 1 · Topologia | La pompa non ha linee di segnale verso l'ESP32 → **N-MOSFET lato basso**, gate pilotato direttamente dal GPIO | — |
| 2 · Tensione | V<sub>DS(max)</sub> ≥ 2 × 12 = 24 V → MOSFET da 30 V | ✅ |
| 3 · Gate | V<sub>GS</sub> = 3,3 V → serve un logic level con V<sub>GS(th),max</sub> ≤ 1,5 V e R<sub>DS(on)</sub> garantita a 2,5 V. Scelta: **AO3400A** (indicativi: 30 V, V<sub>GS(th),max</sub> 1,45 V, R<sub>DS(on)</sub> ≈ 50 mΩ a 2,5 V). Rapporto 3,3 / 1,45 = 2,3 | ✅ |
| 4 · Conduzione | Caso peggiore R = 50 × 1,5 = 75 mΩ. ΔV = 1,2 × 0,075 = 90 mV. P = 1,2² × 0,075 = **108 mW** | ✅ |
| 5 · Temperatura | T<sub>j</sub> = 40 + 0,108 × 200 = **62 °C** | ✅ |
| 6 · Spunto | P = 3² × 0,075 = 0,68 W per 100 ms (68 mJ) → verifica sul grafico SOA o sull'impedenza termica transitoria per impulsi da 100 ms | ⚠️ al limite, in genere accettabile |
| 7 · Resistenze | R<sub>G</sub> = 220 Ω (picco dal GPIO: 3,3 / 220 = 15 mA). R<sub>PD</sub> = 100 kΩ, per tenere la pompa spenta al boot | ✅ |
| 8 · **Trappola** | Il carico è **induttivo**: allo spegnimento il motore genera una sovratensione che può superare i 30 V di Q1 e distruggerlo. Serve un **diodo di ricircolo** in antiparallelo alla pompa: Schottky da 3 A (SS34), catodo su +12 V. Con il diodo V<sub>DS</sub> resta a circa 12,5 V | ✅ con D1 |
| 9 · Cablaggio | I 3 A di spunto non devono passare per i fili di massa dell'ESP32 → **massa a stella** vicino al source di Q1 | — |

**Risultato:** AO3400A, R<sub>G</sub> 220 Ω, R<sub>PD</sub> 100 kΩ, diodo SS34 in antiparallelo alla pompa.

**Variante: velocità regolata in PWM a 20 kHz.** Ora contano anche le perdite di commutazione:

- tempo di commutazione: t ≈ Q<sub>g</sub> / I<sub>G</sub> ≈ 6 nC / [(3,3 – 2) V / 220 Ω] ≈ **1 µs** per fronte (Q<sub>g</sub> indicativa)
- P<sub>sw</sub> = ½ × 12 V × 1,2 A × 2 µs × 20 000 Hz ≈ **0,29 W**
- totale con la conduzione ≈ 0,4 W → T<sub>j</sub> ≈ 40 + 0,4 × 200 ≈ **120 °C** ❌

Le soluzioni sono due: abbassare la frequenza a circa 1 kHz (P<sub>sw</sub> ≈ 14 mW, ma il motore può fischiare), oppure aggiungere un **gate driver** che carichi il gate con correnti di centinaia di mA. È il caso in cui le formule in commutazione diventano indispensabili.

### Esercizio 2 · High-side: modulo GSM SIM800L a batteria

<p align="center">
  <img src="img/es2_highside_sim800l.svg" alt="Esercizio 2: P-MOSFET lato alto con soft-start per SIM800L" width="100%">
</p>

**Dati**

| Dato | Valore |
|---|---|
| Alimentazione | LiPo 3,6–4,2 V (sotto 3,6 V il firmware non usa il GSM) |
| Tensione minima del SIM800L | 3,4 V ai suoi pin |
| Corrente | Picchi di 2 A per 577 µs ogni 4,6 ms in trasmissione (ciclo di lavoro circa 1/8), circa 100 mA a riposo |
| Condensatore sul modulo | C<sub>L</sub> = 1000 µF |
| Linee di segnale | UART verso l'ESP32 |
| Temperatura ambiente massima | 40 °C |
| Inrush ammesso | 0,3 A |

**Svolgimento**

| Passo | Calcolo | Esito |
|---|---|---|
| 1 · Topologia | C'è l'UART → **P-MOSFET lato alto + driver N** (2N7002). Il P non si pilota direttamente dal GPIO, perché 4,2 V > 3,3 V | — |
| 2 · Tensione | 2 × 4,2 = 8,4 V → P da 20–30 V. V<sub>GS(max)</sub> ±12 V ≥ 1,5 × 4,2 V | ✅ |
| 3 · Gate | Per sfruttare la R<sub>DS(on)</sub> a 2,5 V: 3,6 × R<sub>GS</sub> / (R<sub>GS</sub> + R<sub>S</sub>) ≥ 2,5 → R<sub>S</sub> ≤ 0,44 × R<sub>GS</sub>. Con 100 kΩ e 10 kΩ: \|V<sub>GS,min</sub>\| = **3,27 V** | ✅ |
| 4 · **Trappola** | Budget: 3,6 – 3,4 = **0,2 V con 2 A** → resistenza totale ≤ 0,1 Ω, fili compresi. Al MOSFET si lasciano circa 40 mΩ. **Primo tentativo, AO3401A** (≈ 0,1 Ω a –2,5 V): ΔV = 0,2 V, che da sola esaurisce il budget | ❌ |
| 4 · Iterazione A | Si sceglie a catalogo un P con R<sub>DS(on),max</sub> ≤ 40 mΩ a V<sub>GS</sub> = –2,5 V → ΔV ≈ 2 × 0,04 = 80 mV | ✅ |
| 4 · Iterazione B | In alternativa si **rinegozia un dato**: soglia software da 3,6 a 3,75 V → budget 0,35 V, e l'AO3401A torna accettabile, al prezzo di sfruttare meno la batteria | ✅ |
| 5 · Temperatura | I<sub>rms</sub> ≈ 2 × √(1/8) ≈ 0,71 A → P ≈ 0,71² × 0,04 ≈ **20 mW** → T<sub>j</sub> ≈ 44 °C. A questa temperatura il fattore reale è circa 1,05: usare 1,5 alla cieca porterebbe a sovradimensionare | ✅ |
| 6 · Resistenze | R<sub>GS</sub> consuma 42 µA solo durante la sessione GSM; perdite del gate trascurabili | ✅ |
| 7 · Inrush | dV/dt = 0,3 A / 1000 µF = 0,3 V/ms. I<sub>G</sub> ≈ (4,2 – 1,5)/10 k – 1,5/100 k ≈ 0,26 mA → C<sub>GD</sub> = 0,26 / 0,3 ≈ 0,87 µF → **1 µF**, salita in circa 16 ms. E = ½ × 1000 µF × 4,2² ≈ 8,8 mJ → circa 0,55 W per 16 ms | ✅ da verificare sul grafico SOA |
| 8 · Spegnimento | t<sub>off</sub> ≈ 1 µF × 4,2 × 100 kΩ / 1,5 ≈ 0,3 s → spegnere prima il modulo con il comando AT di power-off, e mettere il TX dell'ESP32 in alta impedenza | ⚠️ vincolo sul software |

**Risultato:** P-MOSFET con R<sub>DS(on)</sub> ≤ 40 mΩ a –2,5 V (oppure AO3401A con soglia software a 3,75 V), driver 2N7002, R<sub>GS</sub> 100 kΩ, R<sub>S</sub> 10 kΩ, C<sub>GD</sub> 1 µF, R<sub>PD</sub> 100 kΩ.

### Cosa mettono a confronto i due esercizi

| | Esercizio 1 (low-side) | Esercizio 2 (high-side) |
|---|---|---|
| Passo critico | Temperatura e spunto, sovratensione induttiva | Caduta di tensione e inrush |
| Trappola | Il diodo di ricircolo; il PWM fa esplodere le perdite | Il primo MOSFET non basta; il fattore termico 1,5 è eccessivo |
| Lezione | Le formule in commutazione servono solo con il PWM | La procedura è un ciclo, e anche i dati di progetto si possono rinegoziare |
