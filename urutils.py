def waitUntilInputLow(self,btn,t):
        while btn.value():
            time.sleep_ms(t)
            
class DiffTimer(object):
    def __init__(self,elapsed):
        self.elapsed = elapsed
        self.timerState = False
        self.last = 0
    def __init__(self):
        self.elapsed = 0
        self.timerState = False
        self.last = 0
    def reset(self): # transizione di un pulsante
        self.elapsed = 0
        self.last = time.ticks_ms()
    def stop(self):
        if self.timerState:
            self.timerState = False
            self.elapsed = self.elapsed + time.ticks_ms() - self.last
    def start(self):
        if not self.timerState:
            self.timerState = True
            self.last = time.ticks_ms()
    def get(self):
        if self.timerState:
            return time.ticks_ms() - self.last + self.elapsed
        return self.elapsed
    def set(self, e):
        reset()
        self.elapsed = e

class DiffTimer2(object):
    def __init__(self,elapsed):
        self.elapsed = elapsed
        self.timerState = False
        self.tbase = 1
    def __init__(self):
        self.elapsed = 0
        self.timerState = False
    def reset(self): # transizione di un pulsante
        self.elapsed = 0
    def stop(self):
        if self.timerState:
            self.timerState = False
    def start(self):
        if not self.timerState:
            self.timerState = True
    def get(self):
        if self.timerState:
            self.elapsed = self.elapsed + self.tbase
        return self.elapsed
    def set(self, e):
        reset()
        self.elapsed = e
    def setBase(self, e):
        self.tbase = e
