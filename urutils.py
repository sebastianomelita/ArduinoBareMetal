import time


def waitUntil(cond, t):
    # attesa bloccante finché cond() non diventa vera, ricontrollando ogni t ms.
    # In Python un bool non si passa per riferimento: si passa una funzione,
    # ad esempio waitUntil(lambda: pronto, 10)
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
    tl = time.ticks_ms()
    while time.ticks_diff(time.ticks_ms(), tl) <= t:
        if btn.value():
            tl = time.ticks_ms()
        time.sleep_ms(1)


def waitUntilInputHigh(btn, t):
    # attende t ms di livello alto continuo (pressione in pull-down)
    tl = time.ticks_ms()
    while time.ticks_diff(time.ticks_ms(), tl) <= t:
        if not btn.value():
            tl = time.ticks_ms()
        time.sleep_ms(1)


def waitUntilInputChange(btn, livello, t):
    # attende un cambio di livello confermato: livello è il livello attuale
    # (quello accettato l'ultima volta); esce quando il pin è rimasto al livello
    # opposto per t ms di fila e restituisce il nuovo livello (0 o 1).
    if livello == 0:
        waitUntilInputHigh(btn, t)
        return 1
    waitUntilInputLow(btn, t)
    return 0


def waitUntilNB(d, c, t):
    # attesa non bloccante: restituisce True una sola volta, quando la
    # condizione c è rimasta vera per più di t ms di fila.
    # Finché c è falsa il timer riparte a ogni chiamata.
    ok = False
    if not c:
        d.start()
        d.reset()
    if d.get() > t and c:
        d.stop()
        d.reset()
        ok = True
    return ok


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
