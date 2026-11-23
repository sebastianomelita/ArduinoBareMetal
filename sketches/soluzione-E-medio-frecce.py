"""
===============================================================================
CONSEGNA - ESERCIZIO E (MEDIO): FRECCE DI DIREZIONE: DUE TOGGLE E UN LAMPEGGIO
===============================================================================
Classe terza - Sistemi e reti

Argomenti: pull-down, rimbalzi e ipotesi di progetto, polling non bloccante su un solo fronte (versioni 1 e 4).

Realizzate in Wokwi le frecce di direzione di un veicolo: un pulsante accende e spegne la freccia sinistra, un altro la destra. Le frecce lampeggiano e non possono essere accese tutte e due insieme.

OBIETTIVO

Gestire due pulsanti toggle e un lampeggio nello stesso ciclo principale, senza che un compito blocchi gli altri. Ogni pulsante ha le sue variabili di debounce.

CIRCUITO IN WOKWI (ESP32 DevKit con MicroPython)

| Componente | Collegamento | A cosa serve |
| --- | --- | --- |
| Pulsante SINISTRA (PS) | tra il pin 12 e 3V3 | premuto porta il pin a 1 |
| Resistenza 10 kΩ | tra il pin 12 e GND | pull-down di PS |
| Pulsante DESTRA (PD) | tra il pin 14 e 3V3 | premuto porta il pin a 1 |
| Resistenza 10 kΩ | tra il pin 14 e GND | pull-down di PD |
| LED arancione SINISTRA | pin 13, con resistenza 220 Ω | freccia sinistra |
| LED arancione DESTRA | pin 27, con resistenza 220 Ω | freccia destra |

REQUISITI

1. All'accensione le frecce sono spente.
2. A ogni RILASCIO di PS la freccia sinistra cambia stato (toggle); lo stesso per PD con la freccia destra.
3. Accendere una freccia spegne l'altra: le due frecce non sono mai attive insieme.
4. La freccia attiva lampeggia: 400 ms accesa, 400 ms spenta. Quella spenta resta spenta.
5. Ogni pressione produce un solo cambio, anche se il contatto rimbalza.
6. Nessun pulsante blocca l'altro né il lampeggio.

VINCOLI SUL CODICE

- I due pulsanti si gestiscono con la versione 1 della dispensa sul polling su un solo fronte (ticks_ms(), t0, attivo, rilascio dopo 50 ms di basso): ogni pulsante ha le sue variabili (t0S, attivoS, t0D, attivoD).
- Le callback doOnFallS() e doOnFallD() cambiano solo le variabili bool delle frecce; è il compito del lampeggio a comandare i LED.
- Il ciclo principale con tre compiti separati: pulsante sinistro, pulsante destro, lampeggio.
- Nessun time.sleep_ms(); differenze di tempo come time.ticks_diff(time.ticks_ms(), t) > durata.

COME PROVARLO

- [ ] Clic su PS: lampeggia la sinistra; clic su PD: si spegne la sinistra e lampeggia la destra.
- [ ] Clic su PD con la destra accesa: tutto spento.
- [ ] Tenete premuto PS: non succede nulla finché non rilasciate, e intanto la freccia già accesa continua a lampeggiare regolare.
- [ ] Clic rapidi: ogni clic fa un solo cambio.

COSA CONSEGNARE

1. Il link al progetto Wokwi.
2. Il codice completo, con un commento su ogni riga che riguarda i tempi.
3. Le risposte alle domande.

DOMANDE

1. Perché ogni pulsante deve avere le sue variabili t0 e attivo? Cosa succederebbe usandone una sola coppia per tutti e due?
2. Dove avete messo la regola "mai due frecce insieme": nelle callback o nel lampeggio? Perché è meglio lì?
3. Riscrivete la gestione dei due pulsanti con la versione 4 (schedulatore, TBASE = 50 ms): bastano una sola lettura temporizzata e due variabili precval? Che cosa cambia nel ritardo fra il rilascio e il cambio della freccia?
4. Ipotesi di progetto: perché TBASE non può essere 5 ms? E perché non può essere 300 ms?
5. Un buco di 3 ms mentre PS è premuto: con la versione 1 cambia la freccia? E con la versione 4? Spiegate con un disegno del segnale e delle letture.
===============================================================================
"""

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

"""
===============================================================================
RISPOSTE ALLE DOMANDE
===============================================================================
1. t0 e attivo sono la memoria del debounce di UN pulsante: "è premuto, rilascio da confermare" e "l'ultima volta che l'ho visto premuto". Con una sola coppia i due pulsanti si scambiano la memoria. Esempio, con il blocco di PS scritto prima di quello di PD: si preme PD, il suo blocco mette attivo = True e aggiorna t0; 50 ms dopo il rilascio il primo blocco a trovare "pin basso, attivo vero, 50 ms passati" è quello di PS, che chiama doOnFallS(): premendo la freccia destra si accende la sinistra. E se si premono tutti e due, il rilascio di uno resta nascosto finché l'altro è premuto.

2. Nelle callback: è il momento in cui succede l'evento ed è l'unico punto in cui le variabili sx e dx cambiano, quindi lo stato è sempre coerente (mai sx e dx vere insieme). Il lampeggio si limita a mostrare lo stato (LED = freccia attiva E fase accesa) e non deve conoscere le regole. Se la regola stesse nel lampeggio, sx e dx potrebbero essere vere insieme e bisognerebbe decidere chi vince.

3. Sì: una sola lettura temporizzata legge tutti e due i pin.

python
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


Ritardo: con la versione 1 la freccia cambia 50 ms dopo l'ultimo rimbalzo del rilascio; con la versione 4 alla prima lettura dopo i rimbalzi, cioè fra 0 e 50 ms dopo (in media 25).

4. TBASE = 5 ms: due letture possono cadere dentro gli stessi rimbalzi del rilascio (che durano fino a circa 10 ms) e dare due fronti di discesa, cioè due toggle: la freccia non cambia. Il limite inferiore è circa 11 ms (rimbalzi + intervallo di lettura). TBASE = 300 ms: un clic rapido (pressione di 100-150 ms) può cadere tutto fra due letture e andare perso, e il cambio arriva fino a 300 ms dopo il rilascio. Il limite superiore è l'evento vero più breve, circa 100 ms.

5. Versione 1: no. Il buco dura 3 ms, meno dei 50 ms di basso richiesti; alla lettura alta successiva t0 riparte e il rilascio non viene confermato. Versione 4: dipende da dove cadono le letture. Se nessuna cade nel buco non succede nulla. Se una ci cade, vale 0 dopo un 1: fronte di discesa, la freccia cambia a metà pressione; al rilascio vero c'è un altro fronte di discesa e la freccia cambia di nuovo, quindi alla fine il clic sembra non aver fatto nulla. Probabilità circa 3/50 = 6%.

VERIFICA IN SIMULAZIONE

Clic su PS e PD alternati: una sola freccia attiva per volta; secondo clic sulla stessa freccia: tutto spento; PS tenuto premuto 3 s: la freccia già accesa continua a lampeggiare con mezzo periodo di circa 400 ms; tre clic rapidi: tre cambi.
"""
