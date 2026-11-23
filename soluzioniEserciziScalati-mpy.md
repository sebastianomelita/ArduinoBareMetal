>[Torna all'indice](indexpulsanti.md) >[consegne](consegneEserciziScalati-mpy.md) >[versione in C++](soluzioniEserciziScalati.md)

# Esercizi sui pulsanti a difficoltà scalata: soluzioni (MicroPython)

Per ogni esercizio: un riassunto della consegna, il programma MicroPython completo per ESP32 e le risposte alle domande. Ogni programma è stato provato in simulazione con rimbalzi, buchi e spike. Le consegne complete sono in [consegneEserciziScalati-mpy.md](consegneEserciziScalati-mpy.md).

## Prima serie: senza toggle

### Esercizio A (facile): Semaforo pedonale a chiamata, versione bloccante

Realizzate in Wokwi un semaforo per un attraversamento pedonale. Il semaforo delle auto resta verde finché un pedone non preme e rilascia il pulsante di chiamata; allora parte la sequenza che ferma le auto e fa passare il pedone.

[Consegna completa](consegneEserciziScalati-mpy.md#esercizio-a-facile-semaforo-pedonale-a-chiamata-versione-bloccante)

**Programma**

```python
# SOLUZIONE - tecnica 5 bloccante (letture continue con ticks_ms())
from machine import Pin
import time

P1 = Pin(12, Pin.IN)                # pulsante di chiamata, pull-down: premuto = 1
VERDE_AUTO  = Pin(13, Pin.OUT)
GIALLO_AUTO = Pin(27, Pin.OUT)
ROSSO_AUTO  = Pin(26, Pin.OUT)
VERDE_PED   = Pin(25, Pin.OUT)
ROSSO_PED   = Pin(33, Pin.OUT)

def luci(va, ga, ra, vp, rp):
    # imposta in un colpo solo tutti e cinque i LED
    VERDE_AUTO.value(va)
    GIALLO_AUTO.value(ga)
    ROSSO_AUTO.value(ra)
    VERDE_PED.value(vp)
    ROSSO_PED.value(rp)

def riposo():
    luci(1, 0, 0, 0, 1)             # auto: verde, pedoni: rosso

def waitUntilInputLow(btn, t):
    # attende t ms di livello basso continuo: ogni lettura alta
    # (rimbalzo, disturbo o pulsante ancora premuto) fa ripartire il conteggio
    tl = time.ticks_ms()                            # istante dell'ultima lettura alta
    while time.ticks_diff(time.ticks_ms(), tl) <= t:
        if btn.value():
            tl = time.ticks_ms()                    # il conteggio riparte
        time.sleep_ms(1)                            # una lettura al millisecondo

def doOnRise():
    # PRESSIONE (fronte di salita): qui non serve nulla
    pass

def doOnFall():
    # RILASCIO (fronte di discesa): sequenza di attraversamento
    luci(0, 1, 0, 0, 1)             # giallo auto, rosso pedoni
    time.sleep_ms(2000)             # fase gialla: 2 s
    luci(0, 0, 1, 1, 0)             # rosso auto, verde pedoni
    time.sleep_ms(5000)             # attraversamento: 5 s
    luci(0, 0, 1, 0, 1)             # tutto rosso (sgombero)
    time.sleep_ms(1000)             # sgombero: 1 s
    riposo()

riposo()
while True:
    if P1.value():                  # prima lettura alta: pressione
        doOnRise()
        waitUntilInputLow(P1, 50)   # rilascio: 50 ms di livello basso continuo
        doOnFall()
```

**Risposte alle domande**

1. Diagramma (da disegnare): il pin 12 vale 0 a riposo; alla pressione rimbalza per qualche ms e poi resta 1; al rilascio rimbalza di nuovo e poi resta 0. Il ciclo principale entra al primo 1 della pressione; waitUntilInputLow() termina 50 ms dopo l'ULTIMO rimbalzo alto del rilascio, perché ogni lettura alta fa ripartire il conteggio. Solo allora parte la sequenza.

2. Senza la resistenza da 10 kΩ, a pulsante rilasciato il pin 12 non è collegato a nulla (ingresso flottante): legge valori casuali, influenzati dai disturbi e perfino dalla mano vicina al cavo. Il semaforo partirebbe da solo. Il pull-down tiene il pin a 0 quando il pulsante è aperto.

3. La tecnica 1 aspetta t ms e legge una sola volta. Se quella lettura cade in un istante basso dei rimbalzi del rilascio, l'attesa finisce e parte la sequenza; ma il pulsante sta ancora rimbalzando, quindi al giro successivo il ciclo principale può leggere un 1 e prenderlo per una nuova pressione (pressione fantasma). In questo programma il secondo "giro" però arriva dopo gli 8 s della sequenza, quando i rimbalzi sono finiti da un pezzo: l'effetto peggiore è che la sequenza parte durante i rimbalzi del rilascio. Il danno vero della tecnica 1 si vede quando l'azione è breve (per esempio un toggle), perché allora il rimbalzo successivo viene letto subito.

4. Durante i time.sleep_ms() della sequenza il programma non legge il pin 12: le pressioni fatte in quel tempo non vengono viste. Però una pressione ANCORA IN CORSO quando la sequenza finisce viene vista: al primo giro del ciclo principale dopo la sequenza il pin vale 1, il programma attende il rilascio e fa partire una seconda sequenza. (Verificato in simulazione: pressione da 8,0 a 10,0 s, fine sequenza a 9,2 s, nuova sequenza al rilascio.)

5. Con un cavo lungo vicino a un motore diventano probabili gli SPIKE a pulsante rilasciato (e i buchi a pulsante premuto). La tecnica 5 filtra i buchi (servono 50 ms di livello basso continuo) ma NON gli spike: un impulso alto di 2 ms a riposo viene preso per una pressione e, 50 ms dopo, per un rilascio, quindi il semaforo ferma le auto senza nessun pedone (verificato in simulazione). Conviene la tecnica 6, che chiede 50 ms di livello alto fermo anche per la pressione: filtra tutte e quattro le situazioni. Il prezzo è una pressione riconosciuta 50 ms dopo, irrilevante per un semaforo.

### Esercizio B (medio): Pannello con selettore di modo e prova lampade

Molti pannelli di macchine hanno un tasto "prova lampade": finché è premuto tutte le spie si accendono, così l'operatore vede se qualcuna è bruciata. Realizzate in Wokwi un pannello con tre spie, un tasto che sceglie il modo di funzionamento e un tasto di prova lampade.

[Consegna completa](consegneEserciziScalati-mpy.md#esercizio-b-medio-pannello-con-selettore-di-modo-e-prova-lampade)

**Programma**

```python
# SOLUZIONE - P1 con schedulatore su un solo fronte (versione 4, pull-up)
#             P2 senza memoria (pull-down, nessun debounce)
from machine import Pin
import time

TBASE = 50                              # ms fra due letture di P1: > rimbalzi
P1 = Pin(14, Pin.IN, Pin.PULL_UP)       # SELEZIONE, pull-up interno: premuto = 0
P2 = Pin(12, Pin.IN)                    # PROVA LAMPADE, pull-down esterno: premuto = 1
LEDS = (Pin(13, Pin.OUT), Pin(27, Pin.OUT), Pin(26, Pin.OUT))   # verde, giallo, rosso

modo = 0                                # 0 manuale, 1 automatico, 2 manutenzione
precm = 0                               # istante dell'ultima lettura di P1
precval = 1                             # lettura precedente di P1 (riposo = 1)

def doOnRise():
    # RILASCIO di P1 (in pull-up è un fronte di SALITA): modo successivo
    global modo
    modo = (modo + 1) % 3

while True:
    # --- compito 1: P1, una lettura ogni TBASE ms
    if time.ticks_diff(time.ticks_ms(), precm) >= TBASE:
        precm = time.ticks_ms()         # istante di questa lettura
        val = P1.value()
        if precval == 0 and val == 1:   # prima premuto, ora rilasciato
            doOnRise()
        precval = val

    # --- compito 2: P2 senza memoria + uscita, in un solo punto
    prova = P2.value() == 1             # conta solo il livello di adesso
    for i in range(3):
        LEDS[i].value(1 if (prova or i == modo) else 0)
```

**Risposte alle domande**

1. Livelli e fronti dei due pulsanti:

   | | riposo | premuto | pressione | rilascio |
   | --- | --- | --- | --- | --- |
   | P1 (pull-up) | 1 | 0 | fronte di discesa | fronte di salita |
   | P2 (pull-down) | 0 | 1 | fronte di salita | fronte di discesa |

2. P2 è senza memoria: l'uscita dipende solo dal livello di ADESSO. Un rimbalzo di 5 ms fa accendere e spegnere i LED per qualche ms, un tempo che l'occhio non vede; finiti i rimbalzi l'uscita è giusta e non resta traccia dell'errore. P1 invece ha memoria: ogni fronte cambia la variabile modo, e un cambio sbagliato resta. Senza debounce ogni rimbalzo di P1 sarebbe un fronte in più: il modo avanzerebbe di un numero casuale di passi.

3. Il limite inferiore: TBASE deve essere più lungo dei rimbalzi (e dei disturbi), così due letture consecutive non cadono mai entrambe dentro lo stesso treno di rimbalzi: circa 11 ms secondo la finestra di progetto della dispensa. Il limite superiore: TBASE deve essere più corto dell'evento vero più breve (una pressione o una pausa fra due clic, circa 100 ms), altrimenti un clic rapido può cadere fra due letture.
- TBASE = 5 ms: due o tre letture cadono dentro i rimbalzi; una sequenza 0, 1, 0, 1 produce più fronti di salita: il modo avanza di più passi.
- TBASE = 300 ms: un clic di 150 ms può non essere mai letto, e il modo cambia fino a 300 ms dopo il rilascio, un ritardo che si vede.

4. Solo in certi casi. Con il pull-up lo spike a riposo è un impulso basso. Se nessuna lettura ci cade dentro, non succede nulla. Se una lettura lo vede, quella lettura vale 0 e la successiva 1: è un fronte di salita, il modo avanza. La probabilità è circa durata/TBASE = 2/50 = 4% (in simulazione: 41 avanzamenti su 1000 spike).

5. Versione 1 con il pull-up:

```python
t0 = 0
attivo = False
...
if P1.value() == 0:                     # premuto
    attivo = True
    t0 = time.ticks_ms()
elif attivo and time.ticks_diff(time.ticks_ms(), t0) > 50:
    attivo = False                      # alto da più di 50 ms: rilascio
    doOnRise()
```

Ritardo: con la versione 4 il modo cambia alla prima lettura dopo la fine dei rimbalzi, cioè da 0 a 50 ms dopo (in media 25), con la versione 1 sempre 50 ms dopo l'ultimo rimbalzo. Buco a pulsante premuto (impulso alto breve): la versione 1 lo filtra sempre, perché servono 50 ms di livello alto continuo; la versione 4 solo se nessuna lettura ci cade dentro, altrimenti lo prende per un rilascio e il modo avanza a metà pressione.

### Esercizio C (difficile): Gioco dei riflessi, polling non bloccante su due fronti

Realizzate in Wokwi un gioco che misura il tempo di reazione: dopo un'attesa casuale si accende un LED verde e il giocatore deve premere il pulsante il più presto possibile. Il tempo viene stampato nella console. Chi preme prima del verde fa una falsa partenza.

[Consegna completa](consegneEserciziScalati-mpy.md#esercizio-c-difficile-gioco-dei-riflessi-polling-non-bloccante-su-due-fronti)

**Programma**

```python
# SOLUZIONE - versione 1 su due fronti (ticks_ms(), t0, attivo)
from machine import Pin
import time, random

P1 = Pin(12, Pin.IN)                    # pull-down: premuto = 1
VERDE = Pin(13, Pin.OUT)
ROSSO = Pin(26, Pin.OUT)
TDEBOUNCE = 50                          # ms di livello basso continuo per il rilascio
NMANCHE = 5
ATTESA, VIA, FINE = 0, 1, 2
statoGioco = ATTESA

t0 = 0                                  # ultima lettura alta del pulsante
attivo = False                          # pulsante premuto (pressione riconosciuta)
tStato = 0                              # inizio dell'ATTESA o accensione del verde
attesa = 0                              # durata casuale dell'ATTESA (ms)
manche = 0                              # manche valide giocate
somma = 0                               # somma dei tempi validi (ms)
migliore = 0                            # tempo migliore (ms)

def nuovaManche():
    global attesa, tStato, statoGioco
    VERDE.value(0)
    ROSSO.value(0)
    attesa = random.randint(2000, 5000) # fra 2 e 5 s
    tStato = time.ticks_ms()            # l'attesa comincia adesso
    statoGioco = ATTESA

def doOnRise():
    # PRESSIONE: riconosciuta alla prima lettura alta, quindi subito
    global statoGioco, manche, somma, migliore
    if statoGioco == ATTESA:            # premuto prima del verde
        ROSSO.value(1)
        print("Falsa partenza!")
        statoGioco = FINE
    elif statoGioco == VIA:
        tr = time.ticks_diff(time.ticks_ms(), tStato)   # tempo di reazione (ms)
        VERDE.value(0)
        print("Tempo:", tr, "ms")
        manche += 1
        somma += tr
        if manche == 1 or tr < migliore:
            migliore = tr
        if manche == NMANCHE:
            print("Migliore:", migliore, "ms  Media:", somma // NMANCHE, "ms")
            manche = 0                  # si ricomincia da capo
            somma = 0
        statoGioco = FINE
    # in FINE la pressione non fa nulla

def doOnFall():
    # RILASCIO: confermato da 50 ms di basso; in FINE fa partire la manche dopo
    if statoGioco == FINE:
        nuovaManche()

nuovaManche()
while True:
    # --- compito 1: il pulsante
    if P1.value():
        if not attivo:                  # prima lettura alta: pressione
            attivo = True
            doOnRise()
        t0 = time.ticks_ms()            # a ogni lettura alta il timer riparte
    elif attivo and time.ticks_diff(time.ticks_ms(), t0) > TDEBOUNCE:
        attivo = False                  # basso da più di 50 ms: rilascio
        doOnFall()

    # --- compito 2: il gioco
    if statoGioco == ATTESA and time.ticks_diff(time.ticks_ms(), tStato) > attesa:
        VERDE.value(1)                  # attesa finita
        tStato = time.ticks_ms()        # il cronometro parte adesso
        statoGioco = VIA
```

**Risposte alle domande**

1. Il tempo di reazione è l'intervallo fra l'accensione del verde e l'istante in cui il dito chiude il contatto: va preso alla pressione, e con la versione 1 la pressione è riconosciuta alla prima lettura alta, cioè subito. Agendo al rilascio si misurerebbe il tempo di reazione PIU' il tempo per cui il giocatore tiene premuto PIU' i 50 ms di conferma: un numero che non dice nulla dei riflessi. La ripartenza invece non ha fretta: al rilascio si è sicuri che il dito si è alzato, quindi la nuova manche non parte con il pulsante già premuto (che sarebbe una falsa partenza immediata).

2. Pressione: riconosciuta alla prima lettura alta, quindi con un ritardo di un giro del ciclo principale (meno di 1 ms); i rimbalzi della pressione non la ritardano, perché basta il primo contatto. Rilascio: confermato 50 ms dopo l'ultimo rimbalzo, quando il pin è stato 0 per 50 ms di fila.

3. Con lo schedulatore la pressione è vista alla prima lettura dopo il contatto: le letture sono ogni 50 ms, quindi il ritardo va da 0 a 50 ms, in media 25 ms. L'errore si aggiunge al tempo di reazione e cambia da una manche all'altra. Su tempi di 150-400 ms è un errore fino al 15-30%, e due giocatori che differiscono di 20 ms non si possono confrontare: per questo gioco non è accettabile.

4. Lo spike a riposo è preso per una pressione (la versione 1 non lo filtra):
- in ATTESA: falsa partenza falsa, la manche è persa senza colpa;
- in VIA: il cronometro si ferma su un tempo inventato, magari un "record" di pochi ms; in entrambi i casi 50 ms dopo il "rilascio" viene confermato e parte la manche successiva. È un problema di gioco, non di sicurezza: nessuna macchina si muove. Fa capire però perché dove l'errore è pericoloso si conferma anche la pressione.

5. Versione 1: no. Un buco di 3 ms è più corto dei 50 ms richiesti per confermare il rilascio: alla lettura alta successiva t0 riparte, attivo resta True, doOnFall() non viene chiamata. Versione 4: sì, se una lettura cade nel buco. Quella lettura vale 0 e la precedente 1: è un fronte di discesa, doOnFall() fa partire la manche. Alla lettura dopo il pulsante è di nuovo 1: fronte di salita, doOnRise() in ATTESA, quindi una falsa partenza mentre il giocatore non ha fatto nulla. La probabilità è circa 3/50 = 6% per ogni buco.

6. Un programma bloccante, durante time.sleep_ms(attesa), non legge il pulsante: chi preme prima del verde non viene visto. Peggio: se il pulsante è ancora premuto quando l'attesa finisce, il programma lo trova subito 1 e misura un tempo di reazione di 0 ms. Per vedere le False partenze il pulsante va letto DURANTE l'attesa, cioè nello stesso ciclo principale che controlla il tempo: serve un programma non bloccante.

**Verifica in simulazione**

Pressioni simulate 200, 220, 240, 260, 280 ms dopo il verde: tempi stampati 201, 221, 243, 261, 281 ms (i pochi ms in più sono i primi rimbalzi che cadono bassi); dopo 5 manche valide: migliore 201 ms, media 241 ms; falsa partenza riconosciuta e non contata; pressione tenuta 3 s: la manche successiva parte solo dopo il rilascio.

## Seconda serie: con il toggle

### Esercizio D (facile): Lampada con toggle e campanello, versione bloccante

Realizzate in Wokwi l'ingresso di una casa: un pulsante accende e spegne la luce del portico (toggle), un secondo pulsante è il campanello, che suona solo finché è premuto.

[Consegna completa](consegneEserciziScalati-mpy.md#esercizio-d-facile-lampada-con-toggle-e-campanello-versione-bloccante)

**Programma**

```python
# SOLUZIONE - P1 toggle con tecnica 5 bloccante, P2 campanello senza memoria
from machine import Pin, PWM
import time

P1 = Pin(12, Pin.IN)                    # LUCE, pull-down esterno: premuto = 1
P2 = Pin(14, Pin.IN, Pin.PULL_UP)       # CAMPANELLO, pull-up interno: premuto = 0
LUCE = Pin(13, Pin.OUT)
BUZZER = PWM(Pin(27), freq=1000)     # buzzer a 1000 Hz
luce = False                            # stato del toggle

def waitUntilInputLow(btn, t):
    # attende t ms di livello basso continuo: ogni lettura alta fa ripartire il conteggio
    tl = time.ticks_ms()                # istante dell'ultima lettura alta
    while time.ticks_diff(time.ticks_ms(), tl) <= t:
        if btn.value():
            tl = time.ticks_ms()
        time.sleep_ms(1)                # una lettura al millisecondo

def doOnRise():
    # PRESSIONE di P1: qui non serve nulla
    pass

def doOnFall():
    # RILASCIO di P1: toggle della luce
    global luce
    luce = not luce
    LUCE.value(luce)

LUCE.value(0)
BUZZER.duty_u16(0)                      # all'inizio muto
while True:
    # --- P1: con memoria, debounce bloccante
    if P1.value():
        doOnRise()
        waitUntilInputLow(P1, 50)       # rilascio: 50 ms di livello basso continuo
        doOnFall()

    # --- P2: senza memoria, l'uscita segue il livello
    if P2.value() == 0:
        BUZZER.duty_u16(32768)          # premuto: suona (onda quadra al 50%)
    else:
        BUZZER.duty_u16(0)              # rilasciato: tace
```

**Risposte alle domande**

1. Livelli e fronti dei due pulsanti:

   | | riposo | premuto | pressione | rilascio |
   | --- | --- | --- | --- | --- |
   | P1 (pull-down) | 0 | 1 | fronte di salita | fronte di discesa |
   | P2 (Pin.PULL_UP) | 1 | 0 | fronte di discesa | fronte di salita |

2. Il campanello è un pulsante senza memoria: il buzzer segue il livello di adesso. Se P2 rimbalza per 5 ms, il suono si accende e spegne per 5 ms e nessuno se ne accorge; poi il livello si stabilizza e l'uscita è giusta. La luce ha memoria: ogni rilascio cambia lo stato, e un cambio in più resta. Senza debounce ogni rimbalzo sarebbe un rilascio in più e la luce cambierebbe un numero casuale di volte: a volte si accende, a volte no.

3. Con la tecnica 1 al rilascio una lettura può cadere in un istante basso dei rimbalzi: l'attesa finisce e la luce cambia. Il pulsante però sta ancora rimbalzando: al giro successivo il ciclo principale legge un 1, lo prende per una nuova pressione e alla fine la luce cambia di nuovo (pressione fantasma). Chi usa la luce vede che ogni tanto il clic "non funziona": la luce si accende e si rispegne subito.

4. No. Finché P1 è premuto il programma è fermo dentro waitUntilInputLow(): il pin 14 non viene letto, quindi il buzzer resta bloccato nello stato in cui era quando si è premuto P1 (se taceva, tace; se suonava, continua a suonare anche dopo aver lasciato P2). Si sblocca al rilascio di P1. È un limite di tutte le tecniche bloccanti: mentre aspettano un pulsante non fanno nient'altro. Con la tecnica 6 andrebbe peggio: il ciclo principale resta fermo anche in waitUntilInputHigh(), cioè sempre, a pulsante rilasciato, e il campanello non funzionerebbe mai.

5. Tecnica 5: sì. Il ciclo principale legge il pin a ogni giro, vede l'impulso alto, lo prende per una pressione e 50 ms dopo conferma il rilascio: la luce cambia (verificato in simulazione). Tecnica 6: no, perché la pressione deve restare 1 per 50 ms di fila e uno spike di 2 ms fa solo ripartire il conteggio. Con un cavo lungo e disturbato è più sicura la 6, ma in questo circuito bloccherebbe il campanello (risposta 4): servirebbe una tecnica non bloccante.

**Verifica in simulazione**

200 clic con rimbalzi: 200 cambi della luce; il buzzer suona finché P2 è premuto; uno spike di 2 ms sul pin 12 a riposo cambia la luce.

### Esercizio E (medio): Frecce di direzione: due toggle e un lampeggio

Realizzate in Wokwi le frecce di direzione di un veicolo: un pulsante accende e spegne la freccia sinistra, un altro la destra. Le frecce lampeggiano e non possono essere accese tutte e due insieme.

[Consegna completa](consegneEserciziScalati-mpy.md#esercizio-e-medio-frecce-di-direzione-due-toggle-e-un-lampeggio)

**Programma**

```python
# SOLUZIONE - due pulsanti con la versione 1 su un solo fronte + lampeggio
from machine import Pin
import time

PS = Pin(12, Pin.IN)                    # SINISTRA, pull-down: premuto = 1
PD = Pin(14, Pin.IN)                    # DESTRA,   pull-down: premuto = 1
LED_S = Pin(13, Pin.OUT)
LED_D = Pin(27, Pin.OUT)
TDEBOUNCE = 50                          # ms di livello basso continuo per il rilascio
TBLINK = 400                            # ms accesa e ms spenta

t0S = 0                                 # ultima lettura alta di PS
t0D = 0                                 # ultima lettura alta di PD
attivoS = False                         # PS premuto, rilascio da confermare
attivoD = False                         # PD premuto, rilascio da confermare
sx = False                              # frecce attive
dx = False
fase = False                            # fase del lampeggio, comune alle due frecce
tblink = 0                              # ultimo cambio di fase

def doOnFallS():
    # RILASCIO di PS: toggle della sinistra; se si accende, la destra si spegne
    global sx, dx
    sx = not sx
    if sx:
        dx = False

def doOnFallD():
    # RILASCIO di PD: toggle della destra; se si accende, la sinistra si spegne
    global sx, dx
    dx = not dx
    if dx:
        sx = False

while True:
    # --- compito 1: pulsante sinistro
    if PS.value():
        attivoS = True
        t0S = time.ticks_ms()           # a ogni lettura alta il timer riparte
    elif attivoS and time.ticks_diff(time.ticks_ms(), t0S) > TDEBOUNCE:
        attivoS = False                 # basso da più di 50 ms: rilascio
        doOnFallS()

    # --- compito 2: pulsante destro
    if PD.value():
        attivoD = True
        t0D = time.ticks_ms()
    elif attivoD and time.ticks_diff(time.ticks_ms(), t0D) > TDEBOUNCE:
        attivoD = False
        doOnFallD()

    # --- compito 3: lampeggio
    if time.ticks_diff(time.ticks_ms(), tblink) > TBLINK:   # passati 400 ms
        tblink = time.ticks_ms()
        fase = not fase
    LED_S.value(sx and fase)            # una freccia spenta resta spenta
    LED_D.value(dx and fase)
```

**Risposte alle domande**

1. t0 e attivo sono la memoria del debounce di UN pulsante: "è premuto, rilascio da confermare" e "l'ultima volta che l'ho visto premuto". Con una sola coppia i due pulsanti si scambiano la memoria. Esempio, con il blocco di PS scritto prima di quello di PD: si preme PD, il suo blocco mette attivo = True e aggiorna t0; 50 ms dopo il rilascio il primo blocco a trovare "pin basso, attivo vero, 50 ms passati" è quello di PS, che chiama doOnFallS(): premendo la freccia destra si accende la sinistra. E se si premono tutti e due, il rilascio di uno resta nascosto finché l'altro è premuto.

2. Nelle callback: è il momento in cui succede l'evento ed è l'unico punto in cui le variabili sx e dx cambiano, quindi lo stato è sempre coerente (mai sx e dx vere insieme). Il lampeggio si limita a mostrare lo stato (LED = freccia attiva E fase accesa) e non deve conoscere le regole. Se la regola stesse nel lampeggio, sx e dx potrebbero essere vere insieme e bisognerebbe decidere chi vince.

3. Sì: una sola lettura temporizzata legge tutti e due i pin.

```python
TBASE = 50
precm = 0
precS = 0
precD = 0
...
if time.ticks_diff(time.ticks_ms(), precm) >= TBASE:
    precm = time.ticks_ms()
    valS = PS.value()
    valD = PD.value()
    if precS == 1 and valS == 0:
        doOnFallS()
    if precD == 1 and valD == 0:
        doOnFallD()
    precS = valS
    precD = valD
```

Ritardo: con la versione 1 la freccia cambia 50 ms dopo l'ultimo rimbalzo del rilascio; con la versione 4 alla prima lettura dopo i rimbalzi, cioè fra 0 e 50 ms dopo (in media 25).

4. TBASE = 5 ms: due letture possono cadere dentro gli stessi rimbalzi del rilascio (che durano fino a circa 10 ms) e dare due fronti di discesa, cioè due toggle: la freccia non cambia. Il limite inferiore è circa 11 ms (rimbalzi + intervallo di lettura). TBASE = 300 ms: un clic rapido (pressione di 100-150 ms) può cadere tutto fra due letture e andare perso, e il cambio arriva fino a 300 ms dopo il rilascio. Il limite superiore è l'evento vero più breve, circa 100 ms.

5. Versione 1: no. Il buco dura 3 ms, meno dei 50 ms di basso richiesti; alla lettura alta successiva t0 riparte e il rilascio non viene confermato. Versione 4: dipende da dove cadono le letture. Se nessuna cade nel buco non succede nulla. Se una ci cade, vale 0 dopo un 1: fronte di discesa, la freccia cambia a metà pressione; al rilascio vero c'è un altro fronte di discesa e la freccia cambia di nuovo, quindi alla fine il clic sembra non aver fatto nulla. Probabilità circa 3/50 = 6%.

**Verifica in simulazione**

Clic su PS e PD alternati: una sola freccia attiva per volta; secondo clic sulla stessa freccia: tutto spento; PS tenuto premuto 3 s: la freccia già accesa continua a lampeggiare con mezzo periodo di circa 400 ms; tre clic rapidi: tre cambi.

### Esercizio F (difficile): Cronometro con start/stop a toggle e azzeramento

Realizzate in Wokwi un cronometro da gara: un pulsante fa partire e fermare il tempo (toggle), un altro lo azzera. Il tempo si legge nella console.

[Consegna completa](consegneEserciziScalati-mpy.md#esercizio-f-difficile-cronometro-con-startstop-a-toggle-e-azzeramento)

**Programma**

```python
# SOLUZIONE - due pulsanti con la versione 1 su due fronti
#   P1 pull-down: toggle start/stop alla PRESSIONE (fronte di salita)
#   P2 pull-up:   azzeramento al RILASCIO (fronte di salita)
from machine import Pin
import time

P1 = Pin(12, Pin.IN)                    # START/STOP, pull-down: premuto = 1
P2 = Pin(14, Pin.IN, Pin.PULL_UP)       # RESET, pull-up interno: premuto = 0
VERDE = Pin(13, Pin.OUT)
ROSSO = Pin(26, Pin.OUT)
TDEBOUNCE = 50                          # ms di livello di riposo per il rilascio

t01 = 0                                 # ultima lettura "premuto" di P1
t02 = 0                                 # ultima lettura "premuto" di P2
attivo1 = False                         # P1 premuto, rilascio da confermare
attivo2 = False                         # P2 premuto, rilascio da confermare

marcia = False                          # cronometro in marcia
tStart = 0                              # istante dell'ultima partenza
accumulato = 0                          # ms accumulati nelle marce precedenti
daStampare = False                      # tempo finale ancora da stampare
tPrint = 0                              # ultima stampa in marcia
tBlink = 0                              # ultimo cambio del LED rosso
rossoAcceso = False

def tempo():
    # tempo totale: accumulato + marcia in corso
    if marcia:
        return accumulato + time.ticks_diff(time.ticks_ms(), tStart)
    return accumulato

def doOnRise1():
    # PRESSIONE di P1: toggle start/stop, subito
    global marcia, tStart, accumulato, daStampare
    if not marcia:
        tStart = time.ticks_ms()        # partenza
        marcia = True
    else:
        accumulato += time.ticks_diff(time.ticks_ms(), tStart)   # arrivo
        marcia = False
        daStampare = True

def doOnFall1():
    # RILASCIO di P1: non serve
    pass

def doOnFall2():
    # PRESSIONE di P2 (in pull-up è un fronte di discesa): non serve
    pass

def doOnRise2():
    # RILASCIO di P2 (fronte di salita): azzera, ma solo da fermo
    global accumulato
    if not marcia:
        accumulato = 0
        print("Azzerato")

while True:
    # --- compito 1: P1 (premuto = 1)
    if P1.value():
        if not attivo1:
            attivo1 = True
            doOnRise1()                 # pressione alla prima lettura alta
        t01 = time.ticks_ms()
    elif attivo1 and time.ticks_diff(time.ticks_ms(), t01) > TDEBOUNCE:
        attivo1 = False
        doOnFall1()

    # --- compito 2: P2 (premuto = 0)
    if P2.value() == 0:
        if not attivo2:
            attivo2 = True
            doOnFall2()
        t02 = time.ticks_ms()
    elif attivo2 and time.ticks_diff(time.ticks_ms(), t02) > TDEBOUNCE:
        attivo2 = False                 # alto da più di 50 ms: rilascio
        doOnRise2()

    # --- compito 3: stampa
    if marcia:
        if time.ticks_diff(time.ticks_ms(), tPrint) >= 100:   # ogni 100 ms
            tPrint = time.ticks_ms()
            t = tempo()
            print("{},{}".format(t // 1000, (t % 1000) // 100))   # secondi,decimi
    elif daStampare:
        print("Tempo finale:", accumulato, "ms")
        daStampare = False

    # --- compito 4: LED
    VERDE.value(marcia)
    if not marcia and accumulato > 0:   # fermo con un tempo: rosso lampeggiante
        if time.ticks_diff(time.ticks_ms(), tBlink) > 250:
            tBlink = time.ticks_ms()
            rossoAcceso = not rossoAcceso
            ROSSO.value(rossoAcceso)
    else:
        rossoAcceso = False
        ROSSO.value(0)
```

**Risposte alle domande**

1. Il cronometro deve fermarsi nell'istante in cui il giudice preme: con la versione 1 la pressione è vista alla prima lettura alta, quindi subito. Con il toggle al rilascio partenza e arrivo sarebbero spostati del tempo per cui il giudice tiene premuto (100-300 ms, ogni volta diverso) più i 50 ms di conferma: errori di decimi di secondo, e diversi alla partenza e all'arrivo. L'azzeramento invece cancella il risultato: non c'è fretta, e al rilascio si agisce solo dopo un gesto completo (pressione e rilascio confermato). È la regola "fermarsi subito, ripartire con calma": l'azione urgente alla pressione, quella che si può pentire al rilascio.

2. Sì, da tutti e due. Se ne occupa la stessa parte di codice: la variabile attivo1 e il timer t01. Dopo la prima lettura alta attivo1 è True:
- i rimbalzi della pressione sono letture basse di pochi ms; il rilascio non viene confermato perché non passano 50 ms, e doOnRise1() non viene richiamata perché attivo1 è già True;
- i rimbalzi del rilascio sono letture alte che fanno ripartire t01: attivo1 resta True, quindi non sono prese per una nuova pressione. doOnRise1() può essere chiamata di nuovo solo dopo 50 ms di basso continuo.

3. È la situazione 4, spike a pulsante rilasciato. La versione 1 accetta la pressione alla prima lettura alta, quindi lo spike è preso per una pressione: il cronometro in marcia si FERMA (arrivo falso), quello fermo riparte. In una gara non è accettabile: il risultato è falso e non si può rifare. Servirebbe una tecnica che conferma anche la pressione (come la tecnica 6 bloccante o il livello stabile), che la ritarda di 50 ms. Per un cronometro quel ritardo è innocuo, perché è uguale alla partenza e all'arrivo e si annulla nella differenza: è un ritardo costante, non casuale.

4. Con lo schedulatore ogni pressione è vista alla prima lettura dopo il contatto: un ritardo fra 0 e 50 ms, ogni volta diverso. Il tempo misurato è (arrivo + e2) - (partenza + e1) = tempo vero + (e2 - e1). I due errori non si annullano, perché sono casuali e indipendenti: e2 - e1 va da -50 a +50 ms, quindi il tempo misurato può sbagliare fino a circa mezzo decimo di secondo, in più o in meno. Con la versione 1 l'errore è di un giro del ciclo principale, meno di 1 ms.

5. Limite inferiore: TBASE più lungo dei rimbalzi più l'intervallo di lettura, circa 10 + 1 = 11 ms, altrimenti due letture cadono negli stessi rimbalzi. Limite superiore: l'evento vero più breve, circa 100 ms. Per un cronometro conviene stare vicino al limite inferiore, per esempio 15-20 ms, perché l'errore massimo di ogni misura è proprio TBASE.

**Verifica in simulazione**

Partenza a 1000 ms, arrivo a 3500 ms: tempo finale 2498 ms (i ms mancanti sono rimbalzi bassi al primo contatto della partenza). Azzeramento in marcia: ignorato. Ripartenza senza azzerare: 3730 ms accumulati. Azzeramento da fermo: al rilascio, stampa "Azzerato".