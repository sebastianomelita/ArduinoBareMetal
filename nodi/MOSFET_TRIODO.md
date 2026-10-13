# Approfondimento: perché la regola 2–2,5× — il modello quadratico e la zona di triodo

Nella [dispensa MOSFET](MOSFET.md) la regola "|V<sub>GS</sub>| ≥ 2–2,5 × |V<sub>GS(th)</sub>|" è presentata come regola pratica. Questo approfondimento ne mostra la **base teorica**: con il modello quadratico del MOSFET (quello "a canale lungo" dei libri di testo) si ricava la V<sub>GS</sub> minima che tiene il MOSFET in **zona di triodo**, cioè acceso come un interruttore chiuso, con una certa corrente. Si vede anche perché, nella pratica, a quel valore si aggiunge un margine.

Le formule si riferiscono a un **N-MOSFET ad arricchimento**. Per un P-MOSFET valgono le stesse relazioni con tensioni e correnti cambiate di segno.

---

## 1. Le tre zone di funzionamento

Si definisce la **tensione di overdrive** V<sub>ov</sub> = V<sub>GS</sub> – V<sub>T</sub>, cioè di quanto la tensione di gate supera la soglia, e la costante del dispositivo:

k = ½ · μ<sub>n</sub> · C<sub>ox</sub> · W / L   [A/V²]

| Zona | Condizioni | Corrente di drain | Stato dell'interruttore ([punto 3.0](MOSFET.md#30-i-tre-stati-dellinterruttore-quando-si-raggiungono-i-limiti)) |
|---|---|---|---|
| **Interdizione** | V<sub>GS</sub> < V<sub>T</sub> | I<sub>D</sub> ≈ 0 | **Spento** |
| **Triodo** | V<sub>GS</sub> > V<sub>T</sub> e V<sub>DS</sub> < V<sub>ov</sub> | I<sub>D</sub> = k · (2 V<sub>ov</sub> V<sub>DS</sub> – V<sub>DS</sub>²) | **Acceso** (interruttore chiuso) |
| **Saturazione** | V<sub>GS</sub> > V<sub>T</sub> e V<sub>DS</sub> ≥ V<sub>ov</sub> | I<sub>D</sub> = k · V<sub>ov</sub>² (non dipende da V<sub>DS</sub>) | **Transizione**, o MOSFET pilotato male. È invece la zona di lavoro degli **amplificatori** (vedi sotto) |

Il confine tra triodo e saturazione è la curva **V<sub>DS,sat</sub> = V<sub>GS</sub> – V<sub>T</sub>**. Sostituita nella formula della saturazione, dà la parabola I<sub>D</sub> = k · V<sub>DS</sub>², tratteggiata nella figura.

> ⚠️ **Trappola di terminologia.** La stessa parola indica cose diverse a seconda del contesto:
>
> | Nella teoria del MOSFET | Nei datasheet e nelle application note di potenza | Nel BJT |
> |---|---|---|
> | **Triodo** | *Ohmic region* | — |
> | **Saturazione** (corrente costante) | ***Linear region*** | Zona attiva |
> | — | — | **Saturazione** = interruttore **chiuso** |
>
> Un MOSFET "in saturazione" è quindi un interruttore **non** completamente chiuso, mentre un BJT "in saturazione" è un interruttore chiuso. Quando i datasheet parlano di *linear mode* (per esempio nelle curve SOA) intendono la saturazione della teoria.

### Che cosa succede fisicamente in saturazione: il generatore di corrente

**Il canale si strozza.** In triodo il canale invertito collega source e drain per tutta la sua lunghezza, e il MOSFET si comporta come una resistenza comandata da V<sub>GS</sub>. Quando V<sub>DS</sub> raggiunge V<sub>GS</sub> – V<sub>T</sub>, la tensione tra gate e drain scende alla soglia (V<sub>GD</sub> = V<sub>T</sub>): vicino al drain il canale non è più invertito e si **strozza** (*pinch-off*).

**La tensione in più cade sulla zona di svuotamento.** Se V<sub>DS</sub> cresce ancora, l'aumento cade quasi tutto sulla piccola **zona di svuotamento** tra la fine del canale e il drain. Gli elettroni che arrivano al bordo del canale vengono trascinati al drain dal campo elettrico intenso di quella zona. La corrente è quindi fissata dalla carica che il gate induce nel canale, cioè da V<sub>GS</sub>, e quasi non dipende più da V<sub>DS</sub>: le curve della figura diventano orizzontali.

**Il risultato è un generatore di corrente pilotato in tensione:**

- per grandi segnali, I<sub>D</sub> = k · (V<sub>GS</sub> – V<sub>T</sub>)²;
- per piccoli segnali attorno a un punto di lavoro, i<sub>d</sub> = g<sub>m</sub> · v<sub>gs</sub>, con **g<sub>m</sub> = 2k · V<sub>ov</sub>** (transconduttanza).

È la zona usata negli **amplificatori**, analoga alla **zona attiva del BJT**, dove I<sub>C</sub> = β · I<sub>B</sub> è un generatore di corrente pilotato in corrente.

**Il generatore non è ideale.** Aumentando V<sub>DS</sub>, la zona strozzata si allarga e il canale efficace si accorcia (*modulazione della lunghezza di canale*). La corrente cresce leggermente:

I<sub>D</sub> ≈ k · V<sub>ov</sub>² · (1 + λ · V<sub>DS</sub>)

con λ parametro del dispositivo. Ne risulta una resistenza d'uscita finita r<sub>o</sub> ≈ 1 / (λ · I<sub>D</sub>). È l'analogo dell'**effetto Early** nel BJT.

**"Lineare" vale per i piccoli segnali.** Per grandi segnali la legge è quadratica, non lineare. Il nome *linear region* usato nei datasheet di potenza viene dall'impiego come amplificatore o come regolatore lineare, non da una proporzionalità tra V<sub>GS</sub> e I<sub>D</sub>.

### Interruttore o amplificatore: le zone "incrociate"

| | Funzionamento da amplificatore (generatore di corrente) | Funzionamento da interruttore chiuso |
|---|---|---|
| **BJT** | Zona attiva | **Saturazione** |
| **MOSFET** | **Saturazione** | Triodo (*ohmic region*) |

Per l'interruttore, la saturazione del MOSFET è la zona da **attraversare in fretta** (la transizione) o da **evitare** (V<sub>GS</sub> insufficiente, punto B della figura). Proprio perché lì il MOSFET si comporta da generatore di corrente, limita la corrente del carico e "assorbe" la differenza di tensione, dissipando V<sub>DS</sub> · I. Per l'amplificatore è invece la zona di lavoro, in cui si fissa il punto di riposo con la polarizzazione.

---

## 2. La figura: caratteristiche d'uscita e retta di carico

<p align="center">
  <img src="img/mosfet_triodo_caratteristiche.svg" alt="Caratteristiche d'uscita di un N-MOSFET con retta di carico e punti di lavoro" width="100%">
</p>

La figura usa valori scelti per essere leggibili (k = 1,5 A/V², V<sub>T</sub> = 1 V): **non** rappresenta un MOSFET reale, che a parità di scala avrebbe le curve di triodo quasi verticali.

Il MOSFET e il carico R<sub>L</sub> sono in serie su V<sub>DD</sub>, quindi il punto di lavoro deve stare sia su una curva del MOSFET sia sulla **retta di carico**:

I<sub>D</sub> = (V<sub>DD</sub> – V<sub>DS</sub>) / R<sub>L</sub>

che va da (0 V; V<sub>DD</sub>/R<sub>L</sub>) a (V<sub>DD</sub>; 0 A). I tre punti segnati corrispondono ai tre stati dell'interruttore:

- **C · spento:** V<sub>GS</sub> < V<sub>T</sub>, corrente nulla, V<sub>DS</sub> = V<sub>DD</sub>. Tutta la tensione è ai capi del MOSFET, ed è qui che conta V<sub>DS(max)</sub>.
- **A · acceso, in triodo (V<sub>GS</sub> = 4 V):** la curva incontra la retta molto vicino all'asse verticale. V<sub>DS</sub> ≈ 0,22 V e il MOSFET dissipa circa 0,42 W, mentre quasi tutta la tensione va al carico.
- **B · V<sub>GS</sub> insufficiente (1,8 V):** la corrente di saturazione a quella V<sub>GS</sub>, k · 0,8² ≈ 0,96 A, è **minore** di quella che il carico vorrebbe (fino a 2 A). Il punto di lavoro finisce in saturazione: il MOSFET limita la corrente, V<sub>DS</sub> ≈ 2,6 V e dissipa circa **2,5 W**, sei volte più che nel punto A, e il carico riceve meno corrente del previsto.

Durante l'accensione e lo spegnimento il punto di lavoro percorre tutta la retta, da C ad A e ritorno, attraversando la saturazione: è per questo che la **transizione** è lo stato in cui tensione e corrente sono alte insieme.

---

## 3. La condizione per restare in triodo

Il punto B mostra il rischio principale. La corrente che il circuito "chiede" è

I = V<sub>DD</sub> / (R<sub>L</sub> + R<sub>on</sub>)

e il MOSFET resta in triodo solo se è in grado di erogarla, cioè se la corrente di saturazione alla V<sub>GS</sub> applicata è maggiore:

I < I<sub>D,sat</sub>(V<sub>GS</sub>) = k · (V<sub>GS</sub> – V<sub>T</sub>)²

Se la condizione non è rispettata il MOSFET **esce dal triodo da solo**: limita la corrente, V<sub>DS</sub> sale e la potenza V<sub>DS</sub> · I diventa grande. È il motivo per cui un MOSFET pilotato con una V<sub>GS</sub> troppo bassa scalda, anche se "è acceso".

Risolvendo la condizione rispetto a V<sub>GS</sub> si ottiene la **V<sub>GS</sub> minima** per una corrente I:

**V<sub>GS,min</sub> = V<sub>T</sub> + √(I / k)**

Al limite (V<sub>GS</sub> = V<sub>GS,min</sub>) il MOSFET sta proprio sul confine con la saturazione, e la sua V<sub>DS</sub> vale già V<sub>ov</sub>: non è ancora un buon interruttore. Per avere una V<sub>DS</sub> piccola bisogna stare **ben sopra** questo valore, cioè V<sub>GS</sub> ≫ V<sub>GS,min</sub>.

---

## 4. La resistenza da acceso nel modello

In triodo profondo, con V<sub>DS</sub> ≪ V<sub>ov</sub>, il termine V<sub>DS</sub>² è trascurabile e il MOSFET si comporta come una resistenza:

I<sub>D</sub> ≈ 2k · V<sub>ov</sub> · V<sub>DS</sub>   →   **R<sub>DS(on)</sub> = 1 / [2k · (V<sub>GS</sub> – V<sub>T</sub>)]**

La formula conferma due fatti usati nella dispensa:

- R<sub>DS(on)</sub> **diminuisce** all'aumentare di V<sub>GS</sub>: un valore del datasheet garantito a una V<sub>GS</sub> più bassa della tua è un limite superiore sicuro ([passo 3, vincolo 2](MOSFET.md#54-la-procedura-passo-per-passo));
- R<sub>DS(on)</sub> cresce molto quando V<sub>GS</sub> si avvicina alla soglia: con V<sub>ov</sub> che tende a zero, la resistenza tende all'infinito.

---

## 5. Come ricavare k dal datasheet

k non compare nel datasheet: si ricava da altri dati, in tre modi. **Ogni metodo dà un valore diverso**, perché il modello quadratico descrive i MOSFET di potenza solo in modo approssimato (punto 7).

| Metodo | Dato del datasheet | Formula | Note |
|---|---|---|---|
| 1 · Caratteristica di trasferimento | Grafico *"Transfer Characteristics"* (I<sub>D</sub> in funzione di V<sub>GS</sub>): un punto (V<sub>GS,on</sub>; I<sub>D,on</sub>) in saturazione | k = I<sub>D,on</sub> / (V<sub>GS,on</sub> – V<sub>T</sub>)² | Il più diretto: usa la zona che interessa per la condizione del punto 3 |
| 2 · Resistenza da acceso | R<sub>DS(on)</sub> a una certa V<sub>GS</sub>, dalla tabella | k = 1 / [2 · R<sub>DS(on)</sub> · (V<sub>GS</sub> – V<sub>T</sub>)] | Valido perché R<sub>DS(on)</sub> si misura in triodo con V<sub>DS</sub> piccola |
| 3 · Transconduttanza | g<sub>fs</sub> a una corrente I<sub>D</sub> | k = g<sub>fs</sub>² / (4 · I<sub>D</sub>) | Da g<sub>m</sub> = 2k V<sub>ov</sub> e I<sub>D</sub> = k V<sub>ov</sub>² |

Nel modello vale anche k = 1 / (4 · I<sub>D</sub> · R<sub>DS</sub>²), perché la transconduttanza in saturazione (2k V<sub>ov</sub>) e la conduttanza in triodo (1/R<sub>DS</sub> = 2k V<sub>ov</sub>) coincidono alla stessa V<sub>GS</sub>. Su un MOSFET reale questa uguaglianza è solo approssimata.

---

## 6. Forme equivalenti di V<sub>GS,min</sub>, e una da evitare

**V<sub>GS,min</sub> = V<sub>T</sub> + √(I / k)** è la forma di riferimento.

**V<sub>GS,min</sub> = V<sub>T</sub> + 2 · R<sub>DS</sub> · I** è equivalente: con R<sub>DS</sub> = 1/(2k V<sub>ov</sub>) si ha 2 R<sub>DS</sub> I = I/(k V<sub>ov</sub>), che uguagliato a V<sub>ov</sub> riporta a V<sub>ov</sub>² = I/k. Però è **circolare**: R<sub>DS</sub> dipende dalla V<sub>GS</sub> che si sta cercando, e la formula vale solo con R<sub>DS</sub> valutata alla V<sub>GS,min</sub> stessa. Se ci si mette la R<sub>DS(on)</sub> del datasheet, misurata a 4,5 V o 10 V, cioè con una resistenza più bassa, si ottiene una V<sub>GS,min</sub> **troppo ottimistica**.

**Da evitare: I<sub>D</sub> = I<sub>DSS</sub> · (1 – V<sub>GS</sub>/V<sub>T</sub>)².** È l'equazione di Shockley dei **JFET e dei MOSFET a svuotamento**, dove I<sub>DSS</sub> è la corrente con V<sub>GS</sub> = 0. In un MOSFET ad arricchimento, con V<sub>GS</sub> = 0 la corrente è nulla. L'uguaglianza regge solo definendo I<sub>DSS</sub> ≡ k · V<sub>T</sub>², che non ha un significato fisico. Inoltre **nei datasheet dei MOSFET I<sub>DSS</sub> indica la corrente di perdita da spento**, dell'ordine di nA–µA: usare lo stesso simbolo con due significati genera confusione. Se si vuole questa forma, conviene chiamare la costante in un altro modo, per esempio I<sub>0</sub> = k · V<sub>T</sub>², da cui V<sub>GS,min</sub> = V<sub>T</sub> · (1 + √(I / I<sub>0</sub>)).

---

## 7. Dal modello alla regola 2–2,5×

Il modello dà una V<sub>GS,min</sub> precisa, ma nella pratica si aggiunge un margine generoso, per quattro motivi:

1. **Il modello quadratico descrive male i MOSFET di potenza.** Nei componenti verticali, trench e a canale corto la corrente cresce in modo più vicino a (V<sub>GS</sub> – V<sub>T</sub>)<sup>α</sup>, con α tra 1 e 2. Le formule danno ordini di grandezza, non valori di progetto.
2. **V<sub>T</sub> del modello non è la V<sub>GS(th)</sub> del datasheet.** Il datasheet misura la soglia a una corrente minima (tipicamente 250 µA), mentre nel modello V<sub>T</sub> è una soglia estrapolata.
3. **La soglia ha una dispersione ampia** (un fattore 2–3 tra esemplari) e **diminuisce con la temperatura**.
4. **Stare al confine non basta:** a V<sub>GS,min</sub> il MOSFET è appena in triodo, con V<sub>DS</sub> = V<sub>ov</sub>. Per essere un buon interruttore deve stare in triodo **profondo**.

La regola "|V<sub>GS</sub>| ≥ 2–2,5 × |V<sub>GS(th),max</sub>|" copre tutti e quattro gli aspetti senza doverli calcolare. Per il dimensionamento vero restano più affidabili i grafici del datasheet: *R<sub>DS(on)</sub> vs V<sub>GS</sub>* e *Output Characteristics*.

---

## 8. Esempio: AO3400A pilotato da un GPIO a 3,3 V

Valori **indicativi**: V<sub>GS(th),max</sub> ≈ 1,45 V, usata come V<sub>T</sub>; R<sub>DS(on)</sub> ≈ 50 mΩ a V<sub>GS</sub> = 2,5 V.

| Passo | Calcolo | Risultato |
|---|---|---|
| k con il metodo 2 | 1 / [2 × 0,05 × (2,5 – 1,45)] | ≈ **9,5 A/V²** |
| Corrente di saturazione a 3,3 V | 9,5 × (3,3 – 1,45)² | ≈ **32 A** |
| Condizione di triodo con la pompa dell'[esercizio 1](MOSFET.md#esercizio-1--low-side-pompa-per-irrigazione-a-12-v) (spunto 3 A) | 3 A < 32 A | ✅ ben dentro il triodo |
| V<sub>GS,min</sub> per 3 A | 1,45 + √(3 / 9,5) | ≈ **2,0 V**, circa 1,4 volte la soglia |

Il GPIO dà 3,3 V, circa 2,3 volte la soglia massima: rispetta la regola pratica e sta ben sopra la V<sub>GS,min</sub> teorica. Il margine tra 2,0 V e 3,3 V copre i limiti del modello elencati al punto 7.

---

## 9. Il confronto con il BJT

Nel BJT usato come interruttore la condizione analoga è la **sovrasaturazione**: la corrente di base deve essere abbondantemente superiore a I<sub>C</sub>/β, così il transistor resta saturo (cioè chiuso) anche con β minimo. Nel MOSFET il ruolo della corrente di base lo ha la tensione V<sub>GS</sub>: deve superare abbondantemente V<sub>GS,min</sub>.

Il vantaggio del MOSFET è che, una volta carico il gate, per restare acceso **non serve corrente continua**: il GPIO non deve fornire alcuna corrente di pilotaggio a regime, mentre la base di un BJT assorbe corrente per tutto il tempo in cui è acceso.
