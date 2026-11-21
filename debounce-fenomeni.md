>[Torna all'indice](indexpulsanti.md)

# Che cosa deve filtrare un debouncer: quattro situazioni critiche e le ipotesi di progetto

Un debouncer non "pulisce" il segnale in generale: è progettato contro **situazioni precise**, sotto
**ipotesi precise** sulla loro durata. Se le ipotesi valgono, la protezione è certa; se cadono, nessuna
tecnica può distinguere un disturbo da un evento vero. Questa dispensa raccoglie le quattro situazioni
critiche, le ipotesi con cui si sceglie il tempo di antirimbalzo `T`, e quale tecnica copre che cosa.

> Tecniche: [pulsanti-bloccanti.md](pulsanti-bloccanti.md) (attese bloccanti), [pulsanti-rilascio.md](pulsanti-rilascio.md) (non bloccanti), [debouncerFSM.md](debouncerFSM.md) (livello stabile come macchina a stati). Parte elettrica: [interfacce-digitali.md](interfacce-digitali.md).

<p align="center">
  <img src="img/debounce-quattro-situazioni.png" alt="Le quattro situazioni critiche su un pulsante in pull-down: rimbalzi alla pressione, rimbalzi al rilascio, buco a pulsante premuto, spike a pulsante rilasciato" width="900">
</p>

## 1. Le quattro situazioni critiche

Le situazioni sono quattro, le cause fisiche tre: i **rimbalzi** del contatto si presentano due volte,
alla pressione e al rilascio. Si contano separatamente perché hanno effetti diversi e, nelle tecniche
più comuni, sono gestiti da parti diverse del codice (vedi la sezione 3).

| | Situazione | Quando | Causa |
| --- | --- | --- | --- |
| 1 | Rimbalzi alla pressione | il contatto si chiude | meccanica: il contatto rimbalza |
| 2 | Rimbalzi al rilascio | il contatto si apre | meccanica: il contatto rimbalza |
| 3 | Buco a pulsante premuto | durante la pressione | meccanica: contatto sporco o consumato, vibrazioni |
| 4 | Spike a pulsante rilasciato | a riposo | elettrica: disturbo indotto sui cavi |

### I rimbalzi: cosa sono e quanto durano

Quando i contatti di un interruttore meccanico si chiudono o si aprono, rimbalzano per un po' prima di
fermarsi: per qualche millisecondo il pin passa più volte fra 0 e 1. Avviene **a ogni pressione e a
ogni rilascio**, su pulsanti, finecorsa, microswitch e contatti di relè.

Jack Ganssle ha misurato una ventina di interruttori diversi: il rimbalzo medio è stato di circa 1,6 ms
e il massimo di 6,2 ms, con pochi casi anomali più lunghi. La durata cambia anche con il modo in cui si
preme: su uno degli interruttori ha trovato una differenza di tre volte a seconda della velocità del
colpo sulla leva.

Ogni rimbalzo sembra un fronte: un toggle scatta più volte, un contatore conta due pezzi. Con la logica
a livello ("finché è premuto, il motore gira") producono solo un glitch di pochi millisecondi.

### 1.1 Rimbalzi alla pressione

Arrivano subito dopo il primo fronte, quando il debouncer **ha già capito** che il pulsante è stato
premuto: basta che li ignori per qualche millisecondo. Sono la situazione più facile: quasi tutte le
tecniche li gestiscono, spesso con una semplice variabile di stato ("pulsante già premuto") o perché il
programma, dopo il primo fronte, sta già facendo altro.

### 1.2 Rimbalzi al rilascio

Sono più insidiosi, perché arrivano mentre il debouncer **sta decidendo** se il pulsante è stato
rilasciato. Se decide troppo presto, al primo livello basso, il rimbalzo successivo sembra una nuova
pressione: è la *pressione fantasma*. Per questo molte tecniche, e molti errori, riguardano soprattutto
il rilascio.

### 1.3 Buco a pulsante premuto

**Che cos'è.** Mentre il pulsante è tenuto premuto, il contatto si apre per un attimo: il pin scende a 0
per qualche millisecondo e poi torna a 1. È un fenomeno **isolato**, in mezzo a una pressione.

**Da dove viene.** Contatto sporco, ossidato o consumato; vibrazioni della macchina; dito che cambia
pressione. È raro con un pulsante buono, più frequente con uno scadente o vecchio.

**Che danno fa.** Se il debouncer lo prende per un rilascio, fa scattare l'azione del rilascio a
pulsante premuto, e subito dopo vede una nuova pressione: un toggle o un conteggio in più.

### 1.4 Spike a pulsante rilasciato

**Che cos'è.** A pulsante rilasciato, un disturbo porta il pin a 1 per un attimo.

**Da dove viene.** Da un accoppiamento elettrico: cavi lunghi vicino a motori, driver, inverter. È
probabile quando a riposo il pin è tenuto al suo livello solo da una resistenza grande (ingresso ad
**alta impedenza**): il pull-down da 10 kΩ di un Arduino, i 45 kΩ interni di un ESP32. È raro sugli
ingressi industriali a 24 V, come quelli di un robot UR, che assorbono milliampere.

**Che danno fa.** Se il debouncer lo prende per una pressione, esegue l'azione della pressione e, poco
dopo, quella del rilascio: un toggle o un conteggio fantasma, senza che nessuno abbia toccato niente.

### E lo sfarfallio dei sensori?

Lo **sfarfallio** dei sensori elettronici (una fotocellula sul bordo di un oggetto lucido) non è una
quinta situazione: ha la stessa forma dei rimbalzi, una raffica di impulsi brevi all'inizio o alla fine
di un evento vero, e si gestisce come le situazioni 1 e 2. Cambia solo la causa: il sensore che "esita"
sul bordo invece del contatto che rimbalza.

## 2. Le ipotesi su cui si dimensiona un debouncer

Un debouncer chiede che un livello resti **fermo** per un tempo `T` prima di accettarlo. Funziona se
valgono queste ipotesi:

| | Ipotesi | Valore tipico | Se non vale |
| --- | --- | --- | --- |
| **I1** | I rimbalzi finiscono entro un tempo massimo `Tr` | `Tr` ≤ 10 ms per un pulsante comune | rimbalzi più lunghi di `T` sembrano eventi veri (limite di fondo) |
| **I2** | Buchi e spike durano meno di un tempo massimo `Td` | `Td` di pochi ms | un disturbo più lungo di `T` è indistinguibile da un evento vero (limite di fondo) |
| **I3** | I disturbi sono **isolati**: fra uno e l'altro passa più di `T` | disturbi occasionali | se arrivano più spesso di `T`, il livello non resta mai fermo: il debouncer non decide mai (un'attesa bloccante non finisce più) |
| **I4** | Gli eventi veri (pressione e rilascio voluti) durano più di `T` | un dito non preme e rilascia in meno di ~100 ms | un evento più breve di `T` viene scartato come disturbo |
| **I5** | Il pin è letto spesso: l'intervallo `p` fra due letture è piccolo rispetto a `T` | `p` = 1 ms | con letture rade la decisione dipende da dove cadono, e la certezza diventa una probabilità |

Messe insieme danno la **finestra di progetto** per `T`:

```
  disturbo più lungo + p   <   T   <   evento vero più breve
     max(Tr, Td) + p
```

<p align="center">
  <img src="img/debounce-finestra.png" alt="Finestra di progetto per il tempo di antirimbalzo: fra il disturbo più lungo più l'intervallo di lettura e l'evento vero più breve" width="900">
</p>

Con un pulsante comune (rimbalzi fino a 10 ms, disturbi di pochi ms, una lettura al millisecondo) e
pressioni umane più lunghe di circa 100 ms, la finestra va da circa 11 a circa 100 ms. Il valore usato
in queste dispense, **50 ms**, sta nel mezzo: copre disturbi quattro volte più lunghi di quelli tipici
(un pulsante scadente o consumato) e resta a metà della pressione più rapida. Per questo `T` non si
abbassa sotto i 50 ms: si perde un po' di prontezza, che a mano non si nota, e si guadagna margine.

Per i **sensori** la finestra va ricalcolata: l'evento vero più breve può essere il passaggio di un
pezzo veloce davanti a una fotocellula, e allora `T` deve essere più corto di quel passaggio.

## 3. Quale tecnica copre quale situazione

| Tecnica | 1. Rimbalzi alla pressione | 2. Rimbalzi al rilascio | 3. Buco a pulsante premuto | 4. Spike a riposo |
| --- | --- | --- | --- | --- |
| Bloccante 1: una lettura ogni `T` | ✔ | ~ | ~ | ✘ |
| Bloccante 2: una lettura e un'attesa fissa dopo | ✔ | ✔ | ~ | ✘ |
| Bloccante 3: due letture | ✔ | ✔ | ~ | ✘ |
| Letture continue, solo sul rilascio: polling con `millis()`, DiffTimer, `waitUntilInputLowNB()`, bloccante 5 | ✔ | ✔ | ✔ | ✘ |
| Schedulatore con N letture uguali | ✔ | ✔ | ~ | ~ |
| Livello stabile in tutte e due le direzioni: `changed()`, `Debouncer`, bloccante 6, `waitUntilInputChange()` | ✔ | ✔ | ✔ | ✔ |

✔ certa, se valgono le ipotesi · ~ probabile · ✘ non filtrato

Le tecniche che filtrano **solo il rilascio** accettano la pressione alla prima lettura alta: la
risposta è immediata, ma lo spike a riposo passa. Le tecniche **a livello stabile** filtrano anche lo
spike, ma anche la pressione arriva `T` dopo, e una pressione più breve di `T` viene ignorata.

### Chi se ne occupa nel codice

Nelle tecniche che filtrano solo il rilascio, le quattro situazioni sono gestite da **meccanismi
diversi** del programma. Nelle tecniche a livello stabile, invece, una sola regola le tratta tutte
allo stesso modo.

| Situazione | Polling con `millis()` (`attivo` + `t0`) | Bloccante 5 (`waitUntilInputLow`) | Livello stabile (`changed()`) |
| --- | --- | --- | --- |
| 1. Rimbalzi alla pressione | la **variabile `attivo`**: `doOnRise()` parte solo al primo HIGH, i successivi trovano `attivo` già vero; i LOW fra un rimbalzo e l'altro non durano 50 ms | il **punto del programma**: dopo il primo HIGH il `loop()` è dentro l'attesa e non rilegge più l'`if`; l'attesa li assorbe perché nessun LOW dura 50 ms | la **regola unica**: ogni variazione fa ripartire il conteggio |
| 2. Rimbalzi al rilascio | il **timer che riparte**: `t0 = millis()` a ogni HIGH | il **timer che riparte**: `tl = millis()` a ogni HIGH dentro l'attesa | la stessa regola |
| 3. Buco a pulsante premuto | il **timer che riparte**, come i rimbalzi del rilascio | il **timer che riparte**, come i rimbalzi del rilascio | la stessa regola |
| 4. Spike a pulsante rilasciato | nessuno: diventa una pressione | nessuno: diventa una pressione | la stessa regola: lo spike non dura 50 ms e viene scartato |

Nelle tecniche asimmetriche il buco è gestito dallo **stesso codice** dei rimbalzi del rilascio: per il
programma sono la stessa cosa, un LOW che non dura abbastanza. I rimbalzi della pressione, invece, sono
gestiti da un **meccanismo diverso** (una variabile di stato o il punto in cui si trova il programma),
che però non sa riconoscere uno spike: per questo lo spike passa. La tecnica a livello stabile tratta
pressione e rilascio con lo stesso codice, e proprio per questo filtra anche lo spike.

## 4. Come scegliere

1. **Quali situazioni sono possibili?** I rimbalzi dipendono dalla **sorgente**: ci sono solo con i
   contatti meccanici. Gli spike dipendono dall'**ingresso** e dal **cablaggio**: sono probabili sui pin
   ad alta impedenza con cavi lunghi, rari sugli ingressi a 24 V. I buchi dipendono dalla **qualità** e
   dallo stato del contatto.
2. **Quali ipotesi valgono?** Stimate il rimbalzo più lungo, il disturbo più lungo e l'evento vero più
   breve, e verificate che la finestra per `T` esista. Se i disturbi non sono isolati (I3), il problema
   va risolto prima in hardware.
3. **Quanto costa un errore?** Con la logica a livello un disturbo produce solo un glitch transitorio;
   con toggle e contatori lascia un errore permanente.
4. **Scegliete la tecnica**: solo rilascio se gli spike sono improbabili e serve una risposta
   immediata; livello stabile se l'ingresso è disturbato o l'errore costa caro.

## Riferimenti

- [Jack Ganssle, A Guide to Debouncing](https://www.ganssle.com/debouncing.htm): misure dei rimbalzi su interruttori reali
- [Makeability Lab, Debouncing](https://makeabilitylab.github.io/physcomp/arduino/debouncing.html): sintesi delle misure e soluzioni per Arduino
- [pulsanti-bloccanti.md](pulsanti-bloccanti.md), [pulsanti-rilascio.md](pulsanti-rilascio.md), [debouncerFSM.md](debouncerFSM.md), [interfacce-digitali.md](interfacce-digitali.md)
