import time


def waitUntil(cond, t):
    # attesa bloccante finché cond() non diventa vera, ricontrollando ogni t ms.
    # In Python un bool non si passa per riferimento: si passa una funzione,
    # ad esempio waitUntil(lambda: pronto, 10)
    while not cond():
        time.sleep_ms(t)


def waitUntilInputLow(btn, t):
    # attesa bloccante del rilascio (pull-down): ricontrolla ogni t ms
    # finché il pin è alto, poi attende altri t ms e ricontrolla.
    # Se il pin è tornato alto (rimbalzo o contatto che si è aperto
    # per un attimo) ricomincia: esce solo con un rilascio confermato.
    while True:
        while btn.value():
            time.sleep_ms(t)
        time.sleep_ms(t)  # attesa fine rimbalzi del rilascio
        if not btn.value():
            return


def waitUntilInputHigh(btn, t):
    # come sopra, per il rilascio in pull-up (pin che torna alto)
    while True:
        while not btn.value():
            time.sleep_ms(t)
        time.sleep_ms(t)  # attesa fine rimbalzi del rilascio
        if btn.value():
            return


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

    def reset(self):  # azzera il conteggio; se armato continua a contare
        self.elapsed = 0
        self.last = time.ticks_ms()

    def stop(self):  # disarma e accumula il tempo trascorso
        if self.timerState:
            self.timerState = False
            self.elapsed = self.elapsed + time.ticks_diff(time.ticks_ms(), self.last)

    def start(self):  # arma; non fa nulla se è già armato
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
    # contatore a passi fissi: ogni get() con timer armato aggiunge tbase
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
