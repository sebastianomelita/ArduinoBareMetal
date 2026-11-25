>[Torna all'indice](indexpulsanti.md) >[soluzioni](soluzioniEserciziScalati-mpy.md) >[versione in C++](consegneEserciziScalati.md)

# Esercizi sui pulsanti a difficoltà scalata: facile, medio, difficile (MicroPython)

Sei esercizi per una classe terza, in due serie da tre (facile, medio, difficile). Usano solo questi argomenti:
pull-up e pull-down, rimbalzi, le quattro situazioni critiche e le ipotesi di progetto ([debounce-fenomeni.md](debounce-fenomeni.md)),
pulsanti senza memoria, attese bloccanti (tecniche 1, 5 e 6 di [pulsanti-bloccanti.md](pulsanti-bloccanti-mpy.md)),
polling non bloccante su due fronti (versioni 1 e 2 di [pulsanti-rilascio.md](pulsanti-rilascio-mpy.md)) e su un solo fronte
(versioni 1 e 2 di [pulsanti-rilascio-corte.md](pulsanti-rilascio-corte-mpy.md)). La prima serie non usa il toggle, la seconda sì.

Le consegne sono quelle della [versione C++](consegneEserciziScalati.md), adattate a ESP32 con MicroPython e al simulatore [Wokwi](https://wokwi.com): pin dell'ESP32, alimentazione a 3,3 V, `time.ticks_ms()` al posto di `ticks_ms()`, `print()` al posto del Monitor seriale.

| Livello | Esercizio | Soluzione |
| --- | --- | --- |
| facile | [Esercizio A (facile): Semaforo pedonale a chiamata, versione bloccante](#esercizio-a-facile-semaforo-pedonale-a-chiamata-versione-bloccante) | [soluzione](soluzioniEserciziScalati-mpy.md#esercizio-a-facile-semaforo-pedonale-a-chiamata-versione-bloccante) |
| medio | [Esercizio B (medio): Pannello con selettore di modo e prova lampade](#esercizio-b-medio-pannello-con-selettore-di-modo-e-prova-lampade) | [soluzione](soluzioniEserciziScalati-mpy.md#esercizio-b-medio-pannello-con-selettore-di-modo-e-prova-lampade) |
| difficile | [Esercizio C (difficile): Gioco dei riflessi, polling non bloccante su due fronti](#esercizio-c-difficile-gioco-dei-riflessi-polling-non-bloccante-su-due-fronti) | [soluzione](soluzioniEserciziScalati-mpy.md#esercizio-c-difficile-gioco-dei-riflessi-polling-non-bloccante-su-due-fronti) |
| facile | [Esercizio D (facile): Lampada con toggle e campanello, versione bloccante](#esercizio-d-facile-lampada-con-toggle-e-campanello-versione-bloccante) | [soluzione](soluzioniEserciziScalati-mpy.md#esercizio-d-facile-lampada-con-toggle-e-campanello-versione-bloccante) |
| medio | [Esercizio E (medio): Frecce di direzione: due toggle e un lampeggio](#esercizio-e-medio-frecce-di-direzione-due-toggle-e-un-lampeggio) | [soluzione](soluzioniEserciziScalati-mpy.md#esercizio-e-medio-frecce-di-direzione-due-toggle-e-un-lampeggio) |
| difficile | [Esercizio F (difficile): Cronometro con start/stop a toggle e azzeramento](#esercizio-f-difficile-cronometro-con-startstop-a-toggle-e-azzeramento) | [soluzione](soluzioniEserciziScalati-mpy.md#esercizio-f-difficile-cronometro-con-startstop-a-toggle-e-azzeramento) |

## Prima serie: senza toggle

### Esercizio A (facile): Semaforo pedonale a chiamata, versione bloccante

[Soluzione](soluzioniEserciziScalati-mpy.md#esercizio-a-facile-semaforo-pedonale-a-chiamata-versione-bloccante)

*Classe terza - Sistemi e reti*

*Argomenti: pull-down, rimbalzi, le quattro situazioni critiche, gestione dei pulsanti con attese bloccanti (tecniche 1, 5 e 6).*

Realizzate in Wokwi un semaforo per un attraversamento pedonale. Il semaforo delle auto resta verde finché un pedone non preme e rilascia il pulsante di chiamata; allora parte la sequenza che ferma le auto e fa passare il pedone.

**Obiettivo**

Il programma fa una cosa alla volta (aspetta il pedone, poi esegue la sequenza), quindi le attese bloccanti vanno bene: si usano time.sleep_ms() per le fasi del semaforo e waitUntilInputLow() della dispensa per il pulsante.

**Circuito in Wokwi** (ESP32 DevKit con MicroPython)

| Componente | Collegamento | A cosa serve |
| --- | --- | --- |
| Pulsante di chiamata | tra il pin 12 e 3V3 | premuto porta il pin 12 a 1 |
| Resistenza 10 kΩ | tra il pin 12 e GND | pull-down: a riposo il pin legge 0 |
| LED verde auto | pin 13, con resistenza 220 Ω | via libera alle auto |
| LED giallo auto | pin 27, con resistenza 220 Ω | attenzione |
| LED rosso auto | pin 26, con resistenza 220 Ω | auto ferme |
| LED verde pedoni | pin 25, con resistenza 220 Ω | il pedone può passare |
| LED rosso pedoni | pin 33, con resistenza 220 Ω | il pedone aspetta |

**Requisiti**

1. A riposo: verde auto acceso, rosso pedoni acceso, gli altri spenti.
2. La chiamata vale al RILASCIO del pulsante.
3. Dopo il rilascio parte la sequenza: giallo auto 2 s → rosso auto + verde pedoni 5 s → rosso auto + rosso pedoni 1 s → di nuovo come al punto 1.
4. Ogni pressione produce una sola chiamata, anche se il contatto rimbalza.
5. Le pressioni fatte durante la sequenza vengono ignorate.

**Vincoli sul codice**

- Usate la tecnica 5 della dispensa sulle attese bloccanti ("Letture continue con ticks_ms()"): nel ciclo principale, se il pin vale 1 si chiama doOnRise(), poi waitUntilInputLow(P1, 50), poi doOnFall().
- doOnRise() è vuota; doOnFall() esegue la sequenza del semaforo.
- Ogni fase della sequenza in una funzione o in un blocco commentato.
- Il tempo di antirimbalzo è 50 ms.

**Come provarlo**

- [ ] All'avvio il semaforo è nello stato di riposo.
- [ ] Premete e tenete premuto 3 s: la sequenza parte solo quando rilasciate.
- [ ] Un clic breve: parte una sola sequenza, non due.
- [ ] Premete il pulsante mentre il pedone ha il verde: alla fine non riparte una seconda sequenza.

**Cosa consegnare**

1. Il link al progetto Wokwi.
2. Il codice completo e commentato.
3. Le risposte alle domande.

**Domande**

1. Disegnate il segnale del pin 12 durante una pressione, con i rimbalzi alla pressione e al rilascio. Segnate l'istante in cui waitUntilInputLow() termina.
2. Perché serve la resistenza da 10 kΩ? Cosa leggerebbe il pin 12, a pulsante rilasciato, senza di essa?
3. Se al posto della tecnica 5 usaste la tecnica 1 ("Una lettura ogni t ms"), cosa potrebbe succedere al rilascio? Che effetto avrebbe sul semaforo?
4. Il punto 5 dei requisiti è rispettato "gratis": perché? (Pensate a cosa fa il programma durante i time.sleep_ms().) Una pressione fatta durante la sequenza può comunque far ripartire il semaforo? Quando?
5. Il cavo del pulsante è lungo 10 m e passa vicino ai cavi di un motore. Quale delle quattro situazioni critiche diventa probabile? Quale tecnica della dispensa usereste e perché?

### Esercizio B (medio): Pannello con selettore di modo e prova lampade

[Soluzione](soluzioniEserciziScalati-mpy.md#esercizio-b-medio-pannello-con-selettore-di-modo-e-prova-lampade)

*Classe terza - Sistemi e reti*

*Argomenti: pull-up e pull-down, pulsanti senza memoria, rimbalzi e ipotesi di progetto, polling non bloccante su un solo fronte (versioni 1 e 2).*

Molti pannelli di macchine hanno un tasto "prova lampade": finché è premuto tutte le spie si accendono, così l'operatore vede se qualcuna è bruciata. Realizzate in Wokwi un pannello con tre spie, un tasto che sceglie il modo di funzionamento e un tasto di prova lampade.

**Obiettivo**

Sullo stesso pannello convivono un pulsante SENZA memoria (prova lampade: conta solo il livello, finché è premuto) e un pulsante CON memoria (il selettore: ogni rilascio fa avanzare il modo). I due pulsanti sono collegati in modo diverso, uno in pull-down e uno in pull-up.

**Circuito in Wokwi** (ESP32 DevKit con MicroPython)

| Componente | Collegamento | A cosa serve |
| --- | --- | --- |
| Pulsante SELEZIONE (P1) | tra il pin 14 e GND (pull-up interno, Pin.PULL_UP) | premuto porta il pin 14 a 0 |
| Pulsante PROVA (P2) | tra il pin 12 e 3V3 | premuto porta il pin 12 a 1 |
| Resistenza 10 kΩ | tra il pin 12 e GND | pull-down di P2 |
| LED verde (MANUALE) | pin 13, con resistenza 220 Ω | spia del modo 0 |
| LED giallo (AUTOMATICO) | pin 27, con resistenza 220 Ω | spia del modo 1 |
| LED rosso (MANUTENZIONE) | pin 26, con resistenza 220 Ω | spia del modo 2 |

**Requisiti**

1. All'accensione il modo è MANUALE: acceso solo il LED verde.
2. A ogni RILASCIO di P1 il modo avanza: MANUALE → AUTOMATICO → MANUTENZIONE → MANUALE ... ed è acceso solo il LED del modo corrente.
3. Finché P2 è premuto i tre LED sono tutti accesi; al rilascio di P2 torna acceso solo il LED del modo corrente. P2 non cambia mai il modo.
4. Ogni pressione di P1 fa avanzare il modo di un solo passo.
5. Il programma non è bloccante: P2 risponde anche mentre P1 è premuto.

**Vincoli sul codice**

- P1 si gestisce con la versione 2 della dispensa sul polling su un solo fronte (schedulatore non bloccante, TBASE = 50 ms, variabile precval). Attenzione: P1 è in pull-up, quindi il rilascio è un fronte di SALITA e la callback è doOnRise().
- doOnRise() cambia solo la variabile del modo (modo, da 0 a 2).
- P2 si legge a ogni giro del ciclo principale con P2.value(), SENZA debounce.
- Un solo punto del codice decide quali LED accendere, in base a modo e a P2.
- Nessun time.sleep_ms().

**Come provarlo**

- [ ] All'avvio è acceso solo il verde.
- [ ] Tre clic su P1: giallo, rosso, di nuovo verde. Il cambio avviene al rilascio, non alla pressione.
- [ ] Tenete premuto P2: tutti accesi; rilasciate: torna il modo di prima.
- [ ] Tenete premuto P1 e intanto premete P2: la prova lampade funziona lo stesso.
- [ ] Clic rapidi su P1: ognuno fa un solo passo.

**Cosa consegnare**

1. Il link al progetto Wokwi.
2. Il codice completo, con un commento su ogni riga che riguarda i tempi.
3. Le risposte alle domande.

**Domande**

1. Compilate la tabella per P1 e per P2: livello a riposo, livello da premuto, fronte della pressione, fronte del rilascio.
2. Perché P2 (prova lampade) non ha bisogno di debounce, mentre P1 sì? Che cosa vede l'operatore se P2 rimbalza per 5 ms? E se P1 rimbalza senza debounce?
3. Ipotesi di progetto: TBASE deve stare fra due limiti. Quali? Che cosa succederebbe con TBASE = 5 ms? E con TBASE = 300 ms?
4. Con lo schedulatore uno spike di 2 ms a P1 rilasciato può far avanzare il modo? Sempre, mai o solo in certi casi? Spiegate.
5. Riscrivete la gestione di P1 con la versione 1 della dispensa (ticks_ms() e variabile attivo, pull-up). Che cosa cambia nel ritardo con cui il modo avanza dopo il rilascio? Quale delle due versioni filtra il buco a pulsante premuto?

### Esercizio C (difficile): Gioco dei riflessi, polling non bloccante su due fronti

[Soluzione](soluzioniEserciziScalati-mpy.md#esercizio-c-difficile-gioco-dei-riflessi-polling-non-bloccante-su-due-fronti)

*Classe terza - Sistemi e reti*

*Argomenti: pull-down, le quattro situazioni critiche, polling non bloccante su due fronti (versioni 1 e 2), confronto fra prontezza e sicurezza.*

Realizzate in Wokwi un gioco che misura il tempo di reazione: dopo un'attesa casuale si accende un LED verde e il giocatore deve premere il pulsante il più presto possibile. Il tempo viene stampato nella console. Chi preme prima del verde fa una falsa partenza.

**Obiettivo**

Qui la pressione e il rilascio servono a cose diverse: la PRESSIONE ferma il cronometro (deve essere riconosciuta subito, altrimenti il tempo è falsato), il RILASCIO fa partire la manche successiva. Il programma deve controllare il pulsante anche durante l'attesa casuale, per accorgersi delle false partenze: per questo non può essere bloccante.

**Circuito in Wokwi** (ESP32 DevKit con MicroPython)

| Componente | Collegamento | A cosa serve |
| --- | --- | --- |
| Pulsante | tra il pin 12 e 3V3 | premuto porta il pin 12 a 1 |
| Resistenza 10 kΩ | tra il pin 12 e GND | pull-down |
| LED verde | pin 13, con resistenza 220 Ω | "via!" |
| LED rosso | pin 26, con resistenza 220 Ω | falsa partenza |

I messaggi di print() si leggono nella console di Wokwi.

**Requisiti**

Il gioco ha tre stati: ATTESA, VIA, FINE.

1. ATTESA: LED spenti. Si estrae un'attesa casuale fra 2 e 5 s (random.randint(2000, 5000)); quando è trascorsa si passa a VIA.
2. VIA: si accende il LED verde e si memorizza l'istante di accensione. Alla PRESSIONE il verde si spegne, si stampa il tempo di reazione in ms e si passa a FINE.
3. Se il pulsante viene premuto in ATTESA (falsa partenza) si accende il LED rosso, si stampa "Falsa partenza!" e si passa a FINE.
4. FINE: al RILASCIO del pulsante si spengono i LED e riparte una nuova manche in ATTESA.
5. Dopo 5 manche valide si stampano il tempo migliore e la media; poi si ricomincia da capo.
6. Ogni pressione conta una sola volta, anche se il contatto rimbalza.

**Vincoli sul codice**

- Il pulsante si gestisce con la versione 1 della dispensa sul polling non bloccante su due fronti (ticks_ms(), t0, attivo): pressione riconosciuta alla prima lettura alta, rilascio confermato da 50 ms di livello basso.
- doOnRise() gestisce la pressione (stop del cronometro o falsa partenza), doOnFall() la ripartenza; entrambe agiscono in base allo stato del gioco.
- Lo stato del gioco è una variabile (un intero, con le costanti ATTESA, VIA, FINE) e il ciclo principale ha due compiti separati: prima il pulsante, poi l'avanzamento del gioco.
- Nessun time.sleep_ms(); tutte le differenze di tempo come time.ticks_diff(time.ticks_ms(), t) > durata.
- Per l'attesa casuale si usa il modulo random (import random).

**Come provarlo**

- [ ] Cinque manche normali: tempi plausibili (150-400 ms) e alla fine migliore e media.
- [ ] Premete prima del verde: LED rosso e "Falsa partenza!", la manche non conta.
- [ ] Tenete premuto dopo la misura: la manche successiva parte solo quando rilasciate.
- [ ] Premete in VIA: il tempo stampato non cambia se tenete premuto più a lungo.

**Cosa consegnare**

1. Il link al progetto Wokwi.
2. Il codice completo, con un commento su ogni riga che riguarda i tempi.
3. Il diagramma degli stati del gioco (ATTESA, VIA, FINE) con gli eventi che fanno passare da uno all'altro.
4. Le risposte alle domande.

**Domande**

1. Perché il cronometro si ferma alla pressione e non al rilascio, mentre la ripartenza avviene al rilascio? Che cosa misurereste agendo al rilascio?
2. Con la versione 1, di quanto è in ritardo il riconoscimento della pressione rispetto al contatto vero? E il rilascio?
3. Riscrivete la gestione del pulsante con la versione 2 (schedulatore, TBASE = 50 ms). Quale errore massimo e quale errore medio si aggiungono al tempo di reazione misurato? È accettabile per questo gioco?
4. Uno spike di 2 ms a pulsante rilasciato, con la versione 1: che effetto ha in ATTESA? E in VIA? È un problema di sicurezza o solo di gioco?
5. Un buco di 3 ms del contatto mentre il giocatore tiene premuto in FINE può far partire una manche? Rispondete per la versione 1 e per la versione 2.
6. Perché un programma bloccante (con waitUntilInputLow() e time.sleep_ms() per l'attesa casuale) non potrebbe riconoscere le false partenze?

## Seconda serie: con il toggle

### Esercizio D (facile): Lampada con toggle e campanello, versione bloccante

[Soluzione](soluzioniEserciziScalati-mpy.md#esercizio-d-facile-lampada-con-toggle-e-campanello-versione-bloccante)

*Classe terza - Sistemi e reti*

*Argomenti: pull-up e pull-down, rimbalzi, pulsanti senza memoria, gestione dei pulsanti con attese bloccanti (tecniche 1, 5 e 6).*

Realizzate in Wokwi l'ingresso di una casa: un pulsante accende e spegne la luce del portico (toggle), un secondo pulsante è il campanello, che suona solo finché è premuto.

**Obiettivo**

Mettere a confronto un pulsante CON memoria (il toggle della luce, che ha bisogno del debounce) e uno SENZA memoria (il campanello, che segue il livello del pin), e scoprire il limite di una tecnica bloccante quando i pulsanti sono due.

**Circuito in Wokwi** (ESP32 DevKit con MicroPython)

| Componente | Collegamento | A cosa serve |
| --- | --- | --- |
| Pulsante LUCE (P1) | tra il pin 12 e 3V3 | premuto porta il pin 12 a 1 |
| Resistenza 10 kΩ | tra il pin 12 e GND | pull-down di P1 |
| Pulsante CAMPANELLO (P2) | tra il pin 14 e GND (pull-up interno, Pin.PULL_UP) | premuto porta il pin 14 a 0 |
| LED bianco o giallo | pin 13, con resistenza 220 Ω | luce del portico |
| Buzzer piezo | tra il pin 27 e GND, comandato in PWM | campanello |

**Requisiti**

1. All'accensione la luce è spenta e il campanello tace.
2. A ogni RILASCIO di P1 la luce cambia stato (toggle).
3. Ogni pressione di P1 produce un solo cambio, anche se il contatto rimbalza.
4. Finché P2 è premuto il buzzer suona (PWM a 1000 Hz, duty_u16(32768)); quando viene rilasciato smette (duty_u16(0)).

**Vincoli sul codice**

- P1 si gestisce con la tecnica 5 della dispensa sulle attese bloccanti (letture continue con ticks_ms()): se il pin vale 1, doOnRise(), poi waitUntilInputLow(P1, 50), poi doOnFall().
- doOnRise() è vuota; doOnFall() fa il toggle della luce.
- P2 si legge con P2.value() a ogni giro del ciclo principale, senza debounce.

**Come provarlo**

- [ ] Clic su P1: la luce si accende al rilascio; secondo clic: si spegne.
- [ ] Clic rapidi su P1: ogni clic fa un solo cambio.
- [ ] Tenete premuto P2: il buzzer suona finché non rilasciate.
- [ ] Tenete premuto P1 e intanto premete P2: cosa succede? Annotatelo, serve per la domanda 4.

**Cosa consegnare**

1. Il link al progetto Wokwi.
2. Il codice completo e commentato.
3. Le risposte alle domande.

**Domande**

1. Compilate la tabella per P1 e per P2: livello a riposo, livello da premuto, fronte della pressione, fronte del rilascio.
2. Perché il campanello non ha bisogno di debounce, mentre la luce sì? Che cosa succederebbe alla luce se P1 rimbalzasse e il programma non avesse debounce?
3. Con la tecnica 1 ("Una lettura ogni t ms") al posto della 5, che errore può capitare al rilascio di P1? Che effetto vede chi usa la luce?
4. Nella prova con P1 tenuto premuto, il campanello funziona? Perché? È un limite della tecnica 5 o di tutte le tecniche bloccanti?
5. Uno spike di 2 ms sul pin 12, a P1 rilasciato, cambia lo stato della luce con la tecnica 5? E con la tecnica 6? Quale delle due scegliereste se il cavo del pulsante fosse lungo e disturbato?

### Esercizio E (medio): Frecce di direzione: due toggle e un lampeggio

[Soluzione](soluzioniEserciziScalati-mpy.md#esercizio-e-medio-frecce-di-direzione-due-toggle-e-un-lampeggio)

*Classe terza - Sistemi e reti*

*Argomenti: pull-down, rimbalzi e ipotesi di progetto, polling non bloccante su un solo fronte (versioni 1 e 2).*

Realizzate in Wokwi le frecce di direzione di un veicolo: un pulsante accende e spegne la freccia sinistra, un altro la destra. Le frecce lampeggiano e non possono essere accese tutte e due insieme.

**Obiettivo**

Gestire due pulsanti toggle e un lampeggio nello stesso ciclo principale, senza che un compito blocchi gli altri. Ogni pulsante ha le sue variabili di debounce.

**Circuito in Wokwi** (ESP32 DevKit con MicroPython)

| Componente | Collegamento | A cosa serve |
| --- | --- | --- |
| Pulsante SINISTRA (PS) | tra il pin 12 e 3V3 | premuto porta il pin a 1 |
| Resistenza 10 kΩ | tra il pin 12 e GND | pull-down di PS |
| Pulsante DESTRA (PD) | tra il pin 14 e 3V3 | premuto porta il pin a 1 |
| Resistenza 10 kΩ | tra il pin 14 e GND | pull-down di PD |
| LED arancione SINISTRA | pin 13, con resistenza 220 Ω | freccia sinistra |
| LED arancione DESTRA | pin 27, con resistenza 220 Ω | freccia destra |

**Requisiti**

1. All'accensione le frecce sono spente.
2. A ogni RILASCIO di PS la freccia sinistra cambia stato (toggle); lo stesso per PD con la freccia destra.
3. Accendere una freccia spegne l'altra: le due frecce non sono mai attive insieme.
4. La freccia attiva lampeggia: 400 ms accesa, 400 ms spenta. Quella spenta resta spenta.
5. Ogni pressione produce un solo cambio, anche se il contatto rimbalza.
6. Nessun pulsante blocca l'altro né il lampeggio.

**Vincoli sul codice**

- I due pulsanti si gestiscono con la versione 1 della dispensa sul polling su un solo fronte (ticks_ms(), t0, attivo, rilascio dopo 50 ms di basso): ogni pulsante ha le sue variabili (t0S, attivoS, t0D, attivoD).
- Le callback doOnFallS() e doOnFallD() cambiano solo le variabili bool delle frecce; è il compito del lampeggio a comandare i LED.
- Il ciclo principale con tre compiti separati: pulsante sinistro, pulsante destro, lampeggio.
- Nessun time.sleep_ms(); differenze di tempo come time.ticks_diff(time.ticks_ms(), t) > durata.

**Come provarlo**

- [ ] Clic su PS: lampeggia la sinistra; clic su PD: si spegne la sinistra e lampeggia la destra.
- [ ] Clic su PD con la destra accesa: tutto spento.
- [ ] Tenete premuto PS: non succede nulla finché non rilasciate, e intanto la freccia già accesa continua a lampeggiare regolare.
- [ ] Clic rapidi: ogni clic fa un solo cambio.

**Cosa consegnare**

1. Il link al progetto Wokwi.
2. Il codice completo, con un commento su ogni riga che riguarda i tempi.
3. Le risposte alle domande.

**Domande**

1. Perché ogni pulsante deve avere le sue variabili t0 e attivo? Cosa succederebbe usandone una sola coppia per tutti e due?
2. Dove avete messo la regola "mai due frecce insieme": nelle callback o nel lampeggio? Perché è meglio lì?
3. Riscrivete la gestione dei due pulsanti con la versione 2 (schedulatore, TBASE = 50 ms): bastano una sola lettura temporizzata e due variabili precval? Che cosa cambia nel ritardo fra il rilascio e il cambio della freccia?
4. Ipotesi di progetto: perché TBASE non può essere 5 ms? E perché non può essere 300 ms?
5. Un buco di 3 ms mentre PS è premuto: con la versione 1 cambia la freccia? E con la versione 2? Spiegate con un disegno del segnale e delle letture.

### Esercizio F (difficile): Cronometro con start/stop a toggle e azzeramento

[Soluzione](soluzioniEserciziScalati-mpy.md#esercizio-f-difficile-cronometro-con-startstop-a-toggle-e-azzeramento)

*Classe terza - Sistemi e reti*

*Argomenti: pull-down e pull-up, le quattro situazioni critiche, polling non bloccante su due fronti (versioni 1 e 2), prontezza e sicurezza.*

Realizzate in Wokwi un cronometro da gara: un pulsante fa partire e fermare il tempo (toggle), un altro lo azzera. Il tempo si legge nella console.

**Obiettivo**

In un cronometro il toggle di start/stop deve scattare alla PRESSIONE: il giudice preme quando l'atleta taglia il traguardo, e un ritardo di 50 ms sarebbe un errore di misura. L'azzeramento invece può aspettare il rilascio, e anzi conviene. Servono quindi tutte e due le callback.

**Circuito in Wokwi** (ESP32 DevKit con MicroPython)

| Componente | Collegamento | A cosa serve |
| --- | --- | --- |
| Pulsante START/STOP (P1) | tra il pin 12 e 3V3 | premuto porta il pin a 1 |
| Resistenza 10 kΩ | tra il pin 12 e GND | pull-down di P1 |
| Pulsante RESET (P2) | tra il pin 14 e GND (pull-up interno, Pin.PULL_UP) | premuto porta il pin a 0 |
| LED verde | pin 13, con resistenza 220 Ω | cronometro in marcia |
| LED rosso | pin 26, con resistenza 220 Ω | cronometro fermo con un tempo |

I messaggi di print() si leggono nella console di Wokwi.

**Requisiti**

1. All'accensione il cronometro è fermo e azzerato, LED spenti.
2. Alla PRESSIONE di P1 il cronometro cambia stato (toggle): se era fermo riparte dal tempo accumulato, se era in marcia si ferma.
3. In marcia il LED verde è acceso e ogni 100 ms si stampa il tempo nel formato secondi,decimi (es. 12,3).
4. Fermo con un tempo diverso da zero: LED rosso lampeggiante (250 ms) e si stampa una volta il tempo finale in millisecondi.
5. Al RILASCIO di P2 il cronometro si azzera, ma solo se è fermo; in marcia P2 non fa nulla.
6. Ogni pressione produce una sola azione, anche se il contatto rimbalza.

**Vincoli sul codice**

- I due pulsanti si gestiscono con la versione 1 della dispensa sul polling non bloccante su due fronti (ticks_ms(), t0, attivo), ognuno con le sue variabili.
- P1: il toggle sta in doOnRise() (pressione); doOnFall() resta vuota.
- P2 è in pull-up: la pressione è un fronte di discesa e il rilascio un fronte di salita. L'azzeramento sta nella callback del rilascio.
- Il tempo si misura con due variabili: l'istante di partenza e il tempo accumulato nelle marce precedenti.
- Il ciclo principale con compiti separati: P1, P2, stampa, LED. Nessun time.sleep_ms().

**Come provarlo**

- [ ] Premete P1: il tempo parte alla pressione, non al rilascio.
- [ ] Premete di nuovo: si ferma, il rosso lampeggia e si stampa il tempo.
- [ ] Premete ancora: riparte dal tempo accumulato, non da zero.
- [ ] P2 in marcia: nessun effetto. P2 da fermo: azzera al rilascio.
- [ ] Clic rapidi su P1: ognuno fa un solo cambio.

**Cosa consegnare**

1. Il link al progetto Wokwi.
2. Il codice completo, con un commento su ogni riga che riguarda i tempi.
3. Il diagramma degli stati del cronometro (AZZERATO, IN MARCIA, FERMO) con gli eventi che fanno passare da uno all'altro.
4. Le risposte alle domande.

**Domande**

1. Perché il toggle di start/stop sta alla pressione e l'azzeramento al rilascio? Che cosa misurerebbe il cronometro con il toggle al rilascio?
2. Con la versione 1, il toggle alla pressione è protetto dai rimbalzi alla pressione? E da quelli al rilascio? Spiegate quale parte del codice se ne occupa.
3. Uno spike di 2 ms sul pin 12 a P1 rilasciato: che effetto ha sul cronometro in marcia? È un rischio accettabile in una gara? Quale delle quattro situazioni critiche è?
4. Riscrivete la gestione di P1 con la versione 2 (schedulatore, TBASE = 50 ms). Quale errore massimo si aggiunge a ogni partenza e a ogni arrivo? I due errori si annullano sempre nella differenza? Quale errore massimo può avere il tempo misurato?
5. Proponete il valore minimo e massimo di TBASE per la versione 2, usando le ipotesi di progetto della dispensa.

