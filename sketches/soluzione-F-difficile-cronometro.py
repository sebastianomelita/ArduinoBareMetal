"""
===============================================================================
CONSEGNA - ESERCIZIO F (DIFFICILE): CRONOMETRO CON START/STOP A TOGGLE E AZZERAMENTO
===============================================================================
Classe terza - Sistemi e reti

Argomenti: pull-down e pull-up, le quattro situazioni critiche, polling non bloccante su due fronti (versioni 1 e 4), prontezza e sicurezza.

Realizzate in Wokwi un cronometro da gara: un pulsante fa partire e fermare il tempo (toggle), un altro lo azzera. Il tempo si legge nella console.

OBIETTIVO

In un cronometro il toggle di start/stop deve scattare alla PRESSIONE: il giudice preme quando l'atleta taglia il traguardo, e un ritardo di 50 ms sarebbe un errore di misura. L'azzeramento invece può aspettare il rilascio, e anzi conviene. Servono quindi tutte e due le callback.

CIRCUITO IN WOKWI (ESP32 DevKit con MicroPython)

| Componente | Collegamento | A cosa serve |
| --- | --- | --- |
| Pulsante START/STOP (P1) | tra il pin 12 e 3V3 | premuto porta il pin a 1 |
| Resistenza 10 kΩ | tra il pin 12 e GND | pull-down di P1 |
| Pulsante RESET (P2) | tra il pin 14 e GND (pull-up interno, Pin.PULL_UP) | premuto porta il pin a 0 |
| LED verde | pin 13, con resistenza 220 Ω | cronometro in marcia |
| LED rosso | pin 26, con resistenza 220 Ω | cronometro fermo con un tempo |

I messaggi di print() si leggono nella console di Wokwi.

REQUISITI

1. All'accensione il cronometro è fermo e azzerato, LED spenti.
2. Alla PRESSIONE di P1 il cronometro cambia stato (toggle): se era fermo riparte dal tempo accumulato, se era in marcia si ferma.
3. In marcia il LED verde è acceso e ogni 100 ms si stampa il tempo nel formato secondi,decimi (es. 12,3).
4. Fermo con un tempo diverso da zero: LED rosso lampeggiante (250 ms) e si stampa una volta il tempo finale in millisecondi.
5. Al RILASCIO di P2 il cronometro si azzera, ma solo se è fermo; in marcia P2 non fa nulla.
6. Ogni pressione produce una sola azione, anche se il contatto rimbalza.

VINCOLI SUL CODICE

- I due pulsanti si gestiscono con la versione 1 della dispensa sul polling non bloccante su due fronti (ticks_ms(), t0, attivo), ognuno con le sue variabili.
- P1: il toggle sta in doOnRise() (pressione); doOnFall() resta vuota.
- P2 è in pull-up: la pressione è un fronte di discesa e il rilascio un fronte di salita. L'azzeramento sta nella callback del rilascio.
- Il tempo si misura con due variabili: l'istante di partenza e il tempo accumulato nelle marce precedenti.
- Il ciclo principale con compiti separati: P1, P2, stampa, LED. Nessun time.sleep_ms().

COME PROVARLO

- [ ] Premete P1: il tempo parte alla pressione, non al rilascio.
- [ ] Premete di nuovo: si ferma, il rosso lampeggia e si stampa il tempo.
- [ ] Premete ancora: riparte dal tempo accumulato, non da zero.
- [ ] P2 in marcia: nessun effetto. P2 da fermo: azzera al rilascio.
- [ ] Clic rapidi su P1: ognuno fa un solo cambio.

COSA CONSEGNARE

1. Il link al progetto Wokwi.
2. Il codice completo, con un commento su ogni riga che riguarda i tempi.
3. Il diagramma degli stati del cronometro (AZZERATO, IN MARCIA, FERMO) con gli eventi che fanno passare da uno all'altro.
4. Le risposte alle domande.

DOMANDE

1. Perché il toggle di start/stop sta alla pressione e l'azzeramento al rilascio? Che cosa misurerebbe il cronometro con il toggle al rilascio?
2. Con la versione 1, il toggle alla pressione è protetto dai rimbalzi alla pressione? E da quelli al rilascio? Spiegate quale parte del codice se ne occupa.
3. Uno spike di 2 ms sul pin 12 a P1 rilasciato: che effetto ha sul cronometro in marcia? È un rischio accettabile in una gara? Quale delle quattro situazioni critiche è?
4. Riscrivete la gestione di P1 con la versione 4 (schedulatore, TBASE = 50 ms). Quale errore massimo si aggiunge a ogni partenza e a ogni arrivo? I due errori si annullano sempre nella differenza? Quale errore massimo può avere il tempo misurato?
5. Proponete il valore minimo e massimo di TBASE per la versione 4, usando le ipotesi di progetto della dispensa.
===============================================================================
"""

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

"""
===============================================================================
RISPOSTE ALLE DOMANDE
===============================================================================
1. Il cronometro deve fermarsi nell'istante in cui il giudice preme: con la versione 1 la pressione è vista alla prima lettura alta, quindi subito. Con il toggle al rilascio partenza e arrivo sarebbero spostati del tempo per cui il giudice tiene premuto (100-300 ms, ogni volta diverso) più i 50 ms di conferma: errori di decimi di secondo, e diversi alla partenza e all'arrivo. L'azzeramento invece cancella il risultato: non c'è fretta, e al rilascio si agisce solo dopo un gesto completo (pressione e rilascio confermato). È la regola "fermarsi subito, ripartire con calma": l'azione urgente alla pressione, quella che si può pentire al rilascio.

2. Sì, da tutti e due. Se ne occupa la stessa parte di codice: la variabile attivo1 e il timer t01. Dopo la prima lettura alta attivo1 è True:
- i rimbalzi della pressione sono letture basse di pochi ms; il rilascio non viene confermato perché non passano 50 ms, e doOnRise1() non viene richiamata perché attivo1 è già True;
- i rimbalzi del rilascio sono letture alte che fanno ripartire t01: attivo1 resta True, quindi non sono prese per una nuova pressione. doOnRise1() può essere chiamata di nuovo solo dopo 50 ms di basso continuo.

3. È la situazione 4, spike a pulsante rilasciato. La versione 1 accetta la pressione alla prima lettura alta, quindi lo spike è preso per una pressione: il cronometro in marcia si FERMA (arrivo falso), quello fermo riparte. In una gara non è accettabile: il risultato è falso e non si può rifare. Servirebbe una tecnica che conferma anche la pressione (come la tecnica 6 bloccante o il livello stabile), che la ritarda di 50 ms. Per un cronometro quel ritardo è innocuo, perché è uguale alla partenza e all'arrivo e si annulla nella differenza: è un ritardo costante, non casuale.

4. Con lo schedulatore ogni pressione è vista alla prima lettura dopo il contatto: un ritardo fra 0 e 50 ms, ogni volta diverso. Il tempo misurato è (arrivo + e2) - (partenza + e1) = tempo vero + (e2 - e1). I due errori non si annullano, perché sono casuali e indipendenti: e2 - e1 va da -50 a +50 ms, quindi il tempo misurato può sbagliare fino a circa mezzo decimo di secondo, in più o in meno. Con la versione 1 l'errore è di un giro del ciclo principale, meno di 1 ms.

5. Limite inferiore: TBASE più lungo dei rimbalzi più l'intervallo di lettura, circa 10 + 1 = 11 ms, altrimenti due letture cadono negli stessi rimbalzi. Limite superiore: l'evento vero più breve, circa 100 ms. Per un cronometro conviene stare vicino al limite inferiore, per esempio 15-20 ms, perché l'errore massimo di ogni misura è proprio TBASE.

VERIFICA IN SIMULAZIONE

Partenza a 1000 ms, arrivo a 3500 ms: tempo finale 2498 ms (i ms mancanti sono rimbalzi bassi al primo contatto della partenza). Azzeramento in marcia: ignorato. Ripartenza senza azzerare: 3730 ms accumulati. Azzeramento da fermo: al rilascio, stampa "Azzerato".
"""
