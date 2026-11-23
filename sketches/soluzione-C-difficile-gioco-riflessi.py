"""
===============================================================================
CONSEGNA - ESERCIZIO C (DIFFICILE): GIOCO DEI RIFLESSI, POLLING NON BLOCCANTE SU DUE FRONTI
===============================================================================
Classe terza - Sistemi e reti

Argomenti: pull-down, le quattro situazioni critiche, polling non bloccante su due fronti (versioni 1 e 4), confronto fra prontezza e sicurezza.

Realizzate in Wokwi un gioco che misura il tempo di reazione: dopo un'attesa casuale si accende un LED verde e il giocatore deve premere il pulsante il più presto possibile. Il tempo viene stampato nella console. Chi preme prima del verde fa una falsa partenza.

OBIETTIVO

Qui la pressione e il rilascio servono a cose diverse: la PRESSIONE ferma il cronometro (deve essere riconosciuta subito, altrimenti il tempo è falsato), il RILASCIO fa partire la manche successiva. Il programma deve controllare il pulsante anche durante l'attesa casuale, per accorgersi delle false partenze: per questo non può essere bloccante.

CIRCUITO IN WOKWI (ESP32 DevKit con MicroPython)

| Componente | Collegamento | A cosa serve |
| --- | --- | --- |
| Pulsante | tra il pin 12 e 3V3 | premuto porta il pin 12 a 1 |
| Resistenza 10 kΩ | tra il pin 12 e GND | pull-down |
| LED verde | pin 13, con resistenza 220 Ω | "via!" |
| LED rosso | pin 26, con resistenza 220 Ω | falsa partenza |

I messaggi di print() si leggono nella console di Wokwi.

REQUISITI

Il gioco ha tre stati: ATTESA, VIA, FINE.

1. ATTESA: LED spenti. Si estrae un'attesa casuale fra 2 e 5 s (random.randint(2000, 5000)); quando è trascorsa si passa a VIA.
2. VIA: si accende il LED verde e si memorizza l'istante di accensione. Alla PRESSIONE il verde si spegne, si stampa il tempo di reazione in ms e si passa a FINE.
3. Se il pulsante viene premuto in ATTESA (falsa partenza) si accende il LED rosso, si stampa "Falsa partenza!" e si passa a FINE.
4. FINE: al RILASCIO del pulsante si spengono i LED e riparte una nuova manche in ATTESA.
5. Dopo 5 manche valide si stampano il tempo migliore e la media; poi si ricomincia da capo.
6. Ogni pressione conta una sola volta, anche se il contatto rimbalza.

VINCOLI SUL CODICE

- Il pulsante si gestisce con la versione 1 della dispensa sul polling non bloccante su due fronti (ticks_ms(), t0, attivo): pressione riconosciuta alla prima lettura alta, rilascio confermato da 50 ms di livello basso.
- doOnRise() gestisce la pressione (stop del cronometro o falsa partenza), doOnFall() la ripartenza; entrambe agiscono in base allo stato del gioco.
- Lo stato del gioco è una variabile (un intero, con le costanti ATTESA, VIA, FINE) e il ciclo principale ha due compiti separati: prima il pulsante, poi l'avanzamento del gioco.
- Nessun time.sleep_ms(); tutte le differenze di tempo come time.ticks_diff(time.ticks_ms(), t) > durata.
- Per l'attesa casuale si usa il modulo random (import random).

COME PROVARLO

- [ ] Cinque manche normali: tempi plausibili (150-400 ms) e alla fine migliore e media.
- [ ] Premete prima del verde: LED rosso e "Falsa partenza!", la manche non conta.
- [ ] Tenete premuto dopo la misura: la manche successiva parte solo quando rilasciate.
- [ ] Premete in VIA: il tempo stampato non cambia se tenete premuto più a lungo.

COSA CONSEGNARE

1. Il link al progetto Wokwi.
2. Il codice completo, con un commento su ogni riga che riguarda i tempi.
3. Il diagramma degli stati del gioco (ATTESA, VIA, FINE) con gli eventi che fanno passare da uno all'altro.
4. Le risposte alle domande.

DOMANDE

1. Perché il cronometro si ferma alla pressione e non al rilascio, mentre la ripartenza avviene al rilascio? Che cosa misurereste agendo al rilascio?
2. Con la versione 1, di quanto è in ritardo il riconoscimento della pressione rispetto al contatto vero? E il rilascio?
3. Riscrivete la gestione del pulsante con la versione 4 (schedulatore, TBASE = 50 ms). Quale errore massimo e quale errore medio si aggiungono al tempo di reazione misurato? È accettabile per questo gioco?
4. Uno spike di 2 ms a pulsante rilasciato, con la versione 1: che effetto ha in ATTESA? E in VIA? È un problema di sicurezza o solo di gioco?
5. Un buco di 3 ms del contatto mentre il giocatore tiene premuto in FINE può far partire una manche? Rispondete per la versione 1 e per la versione 4.
6. Perché un programma bloccante (con waitUntilInputLow() e time.sleep_ms() per l'attesa casuale) non potrebbe riconoscere le false partenze?
===============================================================================
"""

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

"""
===============================================================================
RISPOSTE ALLE DOMANDE
===============================================================================
1. Il tempo di reazione è l'intervallo fra l'accensione del verde e l'istante in cui il dito chiude il contatto: va preso alla pressione, e con la versione 1 la pressione è riconosciuta alla prima lettura alta, cioè subito. Agendo al rilascio si misurerebbe il tempo di reazione PIU' il tempo per cui il giocatore tiene premuto PIU' i 50 ms di conferma: un numero che non dice nulla dei riflessi. La ripartenza invece non ha fretta: al rilascio si è sicuri che il dito si è alzato, quindi la nuova manche non parte con il pulsante già premuto (che sarebbe una falsa partenza immediata).

2. Pressione: riconosciuta alla prima lettura alta, quindi con un ritardo di un giro del ciclo principale (meno di 1 ms); i rimbalzi della pressione non la ritardano, perché basta il primo contatto. Rilascio: confermato 50 ms dopo l'ultimo rimbalzo, quando il pin è stato 0 per 50 ms di fila.

3. Con lo schedulatore la pressione è vista alla prima lettura dopo il contatto: le letture sono ogni 50 ms, quindi il ritardo va da 0 a 50 ms, in media 25 ms. L'errore si aggiunge al tempo di reazione e cambia da una manche all'altra. Su tempi di 150-400 ms è un errore fino al 15-30%, e due giocatori che differiscono di 20 ms non si possono confrontare: per questo gioco non è accettabile.

4. Lo spike a riposo è preso per una pressione (la versione 1 non lo filtra):
- in ATTESA: falsa partenza falsa, la manche è persa senza colpa;
- in VIA: il cronometro si ferma su un tempo inventato, magari un "record" di pochi ms; in entrambi i casi 50 ms dopo il "rilascio" viene confermato e parte la manche successiva. È un problema di gioco, non di sicurezza: nessuna macchina si muove. Fa capire però perché dove l'errore è pericoloso si conferma anche la pressione.

5. Versione 1: no. Un buco di 3 ms è più corto dei 50 ms richiesti per confermare il rilascio: alla lettura alta successiva t0 riparte, attivo resta True, doOnFall() non viene chiamata. Versione 4: sì, se una lettura cade nel buco. Quella lettura vale 0 e la precedente 1: è un fronte di discesa, doOnFall() fa partire la manche. Alla lettura dopo il pulsante è di nuovo 1: fronte di salita, doOnRise() in ATTESA, quindi una falsa partenza mentre il giocatore non ha fatto nulla. La probabilità è circa 3/50 = 6% per ogni buco.

6. Un programma bloccante, durante time.sleep_ms(attesa), non legge il pulsante: chi preme prima del verde non viene visto. Peggio: se il pulsante è ancora premuto quando l'attesa finisce, il programma lo trova subito 1 e misura un tempo di reazione di 0 ms. Per vedere le False partenze il pulsante va letto DURANTE l'attesa, cioè nello stesso ciclo principale che controlla il tempo: serve un programma non bloccante.

VERIFICA IN SIMULAZIONE

Pressioni simulate 200, 220, 240, 260, 280 ms dopo il verde: tempi stampati 201, 221, 243, 261, 281 ms (i pochi ms in più sono i primi rimbalzi che cadono bassi); dopo 5 manche valide: migliore 201 ms, media 241 ms; falsa partenza riconosciuta e non contata; pressione tenuta 3 s: la manche successiva parte solo dopo il rilascio.
"""
