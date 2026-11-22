import time

# ===========================================================================
# LEGENDA: le quattro situazioni critiche (vedi debounce-fenomeni.md)
#   [1] rimbalzi alla pressione   [2] rimbalzi al rilascio
#   [3] buco a pulsante premuto   [4] spike a pulsante rilasciato
#   OK  = filtrato (nessun evento falso), se t > rimbalzo più lungo + passo
#         di lettura e il disturbo dura meno di t - 1 ms
#   NO  = passa: produce un evento falso
# Le indicazioni sono date per il pull-down; in pull-up si scambiano 0 e 1.
# ===========================================================================


def waitUntil(cond, t):
    # waitUntil(cond, t): attesa BLOCCANTE di una condizione.
    # Ferma il programma finché cond() non restituisce True, ricontrollandola ogni t ms.
    # In Python un bool non si passa per riferimento: si passa una funzione.
    #   Esempio: un interrupt sul pulsante mette premuto = True;
    #            nel ciclo: waitUntil(lambda: premuto, 10)  # aspetta la pressione
    # Debounce: se cond() legge una variabile, la funzione non filtra nessuna delle
    # quattro situazioni (basta un rimbalzo o uno spike nell'interrupt per farla uscire).
    # Se cond() legge il pin, per esempio lambda: P1.value() == 0, fa una lettura
    # ogni t ms: letture discontinue, come la tecnica 1 di pulsanti-bloccanti-mpy.md.
    while not cond():
        time.sleep_ms(t)


# ---------------------------------------------------------------------------
# Attese BLOCCANTI a letture continue (tecnica 5 di pulsanti-bloccanti-mpy.md).
# Escono solo dopo t ms in cui TUTTE le letture, una ogni ms, hanno dato il
# livello atteso: ogni lettura diversa fa ripartire il conteggio. Certe se
# rimbalzi e disturbi durano meno di t - 1 ms. sleep_ms(1) cede la CPU agli
# altri thread (non ai task di asyncio).
# ---------------------------------------------------------------------------

def waitUntilInputLow(btn, t):
    # attende t ms di livello basso continuo (rilascio in pull-down)
    # Usata come attesa del rilascio dopo "if P1.value():" (tecnica 5):
    #   [1] rimbalzi alla pressione : OK (il programma resta qui dentro, non torna all'if)
    #   [2] rimbalzi al rilascio    : OK (ogni rimbalzo alto azzera il conteggio)
    #   [3] buco a pulsante premuto : OK (la lettura alta dopo il buco azzera il conteggio)
    #   [4] spike a riposo          : NO (l'if crede al primo 1; per filtrarlo
    #                                    precedetela con waitUntilInputHigh: tecnica 6)
    tl = time.ticks_ms()
    while time.ticks_diff(time.ticks_ms(), tl) <= t:
        if btn.value():
            tl = time.ticks_ms()
        time.sleep_ms(1)


def waitUntilInputHigh(btn, t):
    # attende t ms di livello alto continuo (pressione in pull-down)
    # Usata come attesa della pressione, seguita da waitUntilInputLow (tecnica 6):
    #   [1] rimbalzi alla pressione : OK (la pressione vale solo dopo t ms di livello alto)
    #   [2] rimbalzi al rilascio    : OK (ci pensa la waitUntilInputLow che segue)
    #   [3] buco a pulsante premuto : OK (ci pensa la waitUntilInputLow che segue)
    #   [4] spike a riposo          : OK (uno spike non resta alto per t ms)
    tl = time.ticks_ms()
    while time.ticks_diff(time.ticks_ms(), tl) <= t:
        if not btn.value():
            tl = time.ticks_ms()
        time.sleep_ms(1)


def waitUntilInputChange(btn, livello, t):
    # attende un cambio di livello confermato: livello è il livello attuale
    # (quello accettato l'ultima volta); esce quando il pin è rimasto al livello
    # opposto per t ms di fila e restituisce il nuovo livello (0 o 1).
    #   [1] rimbalzi alla pressione : OK
    #   [2] rimbalzi al rilascio    : OK
    #   [3] buco a pulsante premuto : OK
    #   [4] spike a riposo          : OK
    if livello == 0:
        waitUntilInputHigh(btn, t)
        return 1
    waitUntilInputLow(btn, t)
    return 0


def waitUntilNB(d, c, t):
    # attesa non bloccante: restituisce True una sola volta, quando la
    # condizione c è rimasta vera per più di t ms di fila.
    # Finché c è falsa il timer riparte a ogni chiamata.
    # Con c = (P1.value() == 0) in pull-down equivale a waitUntilInputLowNB:
    #   [1] rimbalzi alla pressione : OK (dà un solo evento, al rilascio)
    #   [2] rimbalzi al rilascio    : OK
    #   [3] buco a pulsante premuto : OK
    #   [4] spike a riposo          : NO (lo spike fa partire il timer: t ms dopo
    #                                    arriva un rilascio falso)
    ok = False
    if not c:
        d.start()
        d.reset()
    if d.get() > t and c:
        d.stop()
        d.reset()
        ok = True
    return ok


# DiffTimer e DiffTimer2 sono cronometri: non leggono pin e non filtrano nulla da soli.
class DiffTimer(object):
    # cronometro basato su time.ticks_ms()
    def __init__(self, elapsed=0):
        self.elapsed = elapsed
        self.timerState = False
        self.last = 0

    def reset(self):  # azzera il conteggio; se attivo continua a contare
        self.elapsed = 0
        self.last = time.ticks_ms()

    def stop(self):  # disattiva e accumula il tempo trascorso
        if self.timerState:
            self.timerState = False
            self.elapsed = self.elapsed + time.ticks_diff(time.ticks_ms(), self.last)

    def start(self):  # attiva; non fa nulla se è già attivo
        if not self.timerState:
            self.timerState = True
            self.last = time.ticks_ms()

    def toggle(self):
        if self.timerState:
            self.stop()
        else:
            self.start()

    def get(self):  # ms contati: accumulati + dall'ultimo start()/reset()
        if self.timerState:
            return time.ticks_diff(time.ticks_ms(), self.last) + self.elapsed
        return self.elapsed

    def set(self, e):
        self.reset()
        self.elapsed = e


class DiffTimer2(object):
    # contatore a passi fissi: ogni get() con timer attivo aggiunge tbase
    def __init__(self, elapsed=0):
        self.elapsed = elapsed
        self.timerState = False
        self.tbase = 1

    def reset(self):
        self.elapsed = 0

    def stop(self):
        if self.timerState:
            self.timerState = False

    def start(self):
        if not self.timerState:
            self.timerState = True

    def toggle(self):
        if self.timerState:
            self.stop()
        else:
            self.start()

    def get(self):
        if self.timerState:
            self.elapsed = self.elapsed + self.tbase
        return self.elapsed

    def peek(self):  # legge senza far avanzare il conteggio
        return self.elapsed

    def set(self, e):
        self.reset()
        self.elapsed = e

    def setBase(self, e):
        self.tbase = e


def waitUntilInputLowNB(d, btn, t):
    # attesa NON bloccante del rilascio in pull-down: va chiamata a ogni
    # giro del ciclo principale. Restituisce True una sola volta, quando il
    # pin, dopo essere stato alto, è rimasto basso per più di t ms di fila.
    # Ogni lettura alta (pulsante premuto, rimbalzo o disturbo) fa ripartire
    # il conteggio. Il DiffTimer d (uno per pulsante) misura il tempo e
    # ricorda che il pulsante era premuto: d.timerState è vero dalla
    # pressione al rilascio.
    #   [1] rimbalzi alla pressione : OK (dà un solo evento, al rilascio)
    #   [2] rimbalzi al rilascio    : OK (ogni rimbalzo alto azzera il conteggio)
    #   [3] buco a pulsante premuto : OK (la lettura alta dopo il buco azzera il conteggio)
    #   [4] spike a riposo          : NO (lo spike vale come pressione: t ms dopo
    #                                    arriva un rilascio falso)
    if btn.value():
        d.start()  # non fa nulla se già attivo
        d.reset()  # il conteggio riparte
    elif d.timerState and d.get() > t:
        d.stop()   # rilascio confermato
        d.reset()
        return True
    return False


def waitUntilInputHighNB(d, btn, t):
    # come sopra, per il rilascio in pull-up: t ms di livello alto continuo
    # Stesse situazioni di waitUntilInputLowNB (spike in pull-up = 0 breve): [1][2][3] OK, [4] NO
    if not btn.value():
        d.start()
        d.reset()
    elif d.timerState and d.get() > t:
        d.stop()
        d.reset()
        return True
    return False



class Debouncer(object):
    # debounce a livello stabile (macchina a stati "prima gli ingressi", vedi debouncerFSM_py.md):
    # accetta un nuovo livello, in TUTTE E DUE le direzioni, solo quando è rimasto fermo per
    # più di t ms. Filtra rimbalzi, buchi a pulsante premuto e spike a pulsante rilasciato.
    # Un oggetto per pulsante, creato con il livello di riposo (0 in pull-down, 1 in pull-up).
    #   val0  = ultima lettura: ogni sua variazione fa ripartire il conteggio da last
    #   val00 = livello accettato
    # changed() va chiamata a ogni giro del ciclo e restituisce True una sola volta, nel giro
    # in cui il nuovo livello viene accettato: guardando val00 si sa se è pressione o rilascio.
    #   [1] rimbalzi alla pressione : OK
    #   [2] rimbalzi al rilascio    : OK
    #   [3] buco a pulsante premuto : OK (il livello basso non resta fermo t ms)
    #   [4] spike a riposo          : OK (il livello alto non resta fermo t ms)
    # Esempio (toggle del LED al rilascio, pull-down):
    #   deb = Debouncer(0)                 # un oggetto per pulsante; pull-up: Debouncer(1)
    #   while True:
    #       if deb.changed(P1, 50):        # nuovo livello fermo da più di 50 ms
    #           if deb.val00 == 1:
    #               pass                   # pressione confermata
    #           else:
    #               LED.value(not LED.value())   # rilascio confermato: toggle
    def __init__(self, riposo=0):
        self.val0 = riposo
        self.val00 = riposo
        self.last = 0

    def changed(self, btn, t):
        val = btn.value()
        if val != self.val0:          # variazione: il conteggio riparte
            self.val0 = val
            self.last = time.ticks_ms()
        elif val != self.val00 and time.ticks_diff(time.ticks_ms(), self.last) > t:
            self.val00 = val          # livello fermo da più di t ms: accettato
            return True
        return False

class Lockout(object):
    # debounce senza conferma (lockout, "trust the first change"): il primo cambio di livello
    # è creduto SUBITO, poi per t ms ogni altro cambio è ignorato (i rimbalzi cadono lì dentro).
    # È la tecnica più pronta, ma NON filtra buchi a pulsante premuto e spike a pulsante
    # rilasciato: un disturbo isolato viene preso per un evento vero. Va bene dove conta la
    # prontezza e un errore raro è tollerato (videogiochi, tastierini); per toggle e contatori
    # meglio Debouncer. Un oggetto per pulsante, creato con il livello di riposo.
    #   stato = livello creduto
    #   t0    = istante dell'ultimo evento: per t ms dopo t0 la lettura non conta
    # changed() va chiamata a ogni giro del ciclo e restituisce True nel giro in cui il cambio
    # viene creduto: guardando stato si sa se è una pressione o un rilascio.
    #   [1] rimbalzi alla pressione : OK (cadono nella finestra di blocco)
    #   [2] rimbalzi al rilascio    : OK (cadono nella finestra di blocco)
    #   [3] buco a pulsante premuto : NO (creduto come rilascio, poi pressione falsa)
    #   [4] spike a riposo          : NO (creduto come pressione, poi rilascio)
    # Esempio (tasto di un gioco: azione subito alla pressione, pull-down):
    #   lk = Lockout(0)                    # un oggetto per pulsante; pull-up: Lockout(1)
    #   while True:
    #       if lk.changed(P1, 50):         # cambio creduto subito, poi 50 ms di blocco
    #           if lk.stato == 1:
    #               salta()                # pressione: nessun ritardo
    #           else:
    #               pass                   # rilascio
    def __init__(self, riposo=0):
        self.stato = riposo
        self.t0 = time.ticks_ms()

    def changed(self, btn, t):
        val = btn.value()
        if val != self.stato and time.ticks_diff(time.ticks_ms(), self.t0) > t:
            self.stato = val              # creduto subito
            self.t0 = time.ticks_ms()     # da qui, t ms di blocco
            return True
        return False
