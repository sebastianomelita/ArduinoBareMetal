# Pulsanti in MicroPython: versioni corte, solo azione al rilascio

Queste versioni hanno solo la callback del **rilascio**: `doOnFall()` nel pull-down,
`doOnRise()` nel pull-up. Senza azione alla pressione il codice si accorcia.

> Versioni complete, con azione alla pressione e al rilascio: [pulsanti-rilascio-mpy.md](pulsanti-rilascio-mpy.md)
> Esempi con due task concorrenti, toggle e blink (pull-down): [toggle-blink-pulldown-mpy.md](toggle-blink-pulldown-mpy.md)
> Versione Arduino: [pulsanti-rilascio-corte.md](pulsanti-rilascio-corte.md)

Per riconoscere il rilascio bisogna comunque ricordare che prima il pulsante era premuto.
Nella maggior parte delle tecniche questa memoria c'è già, senza variabili booleane in più:

| Tecnica | Dove sta la memoria "era premuto" | Bool in più |
| --- | --- | --- |
| Bloccante | nel punto del programma: si è dentro l'`if` | no |
| ticks_ms() | variabile `armato` | sì |
| DiffTimer | `timerState`, dentro la classe | no |
| waitUntilNB() | `timerState`, dentro la classe (serve `premuto` per il reset) | no |
| Schedulatore | `precval`, il campione precedente | no |

Pin di esempio per ESP32: pulsante su GPIO 12, LED su GPIO 13. Su ESP32 si può usare anche
il pull-down interno, `Pin(12, Pin.IN, Pin.PULL_DOWN)`, al posto della resistenza esterna.
Al posto di `millis()` si usa `time.ticks_ms()`, e le differenze di tempo si calcolano con
`time.ticks_diff()`, che resta corretta anche quando il contatore ricomincia da capo.

## 1. Versione bloccante

<table>
<tr>
<th>Pull-down: azione sul fronte di discesa</th>
<th>Pull-up: azione sul fronte di salita</th>
</tr>
<tr>
<td>

```python
from machine import Pin
import time
from urutils import waitUntilInputLow

P1  = Pin(12, Pin.IN)   # pull-down esterno
LED = Pin(13, Pin.OUT)
stato = False

def doOnFall():
    # RILASCIO (fronte di discesa):
    # qui vanno le azioni al rilascio.
    # Esempio: toggle, inverte lo
    # stato del LED a ogni rilascio.
    global stato
    stato = not stato
    LED.value(stato)

while True:
    if P1.value():              # premuto
        waitUntilInputLow(P1, 50)
        doOnFall()
```

</td>
<td>

```python
from machine import Pin
import time
from urutils import waitUntilInputHigh

P1  = Pin(12, Pin.IN, Pin.PULL_UP)  # pull-up interno
LED = Pin(13, Pin.OUT)
stato = False

def doOnRise():
    # RILASCIO (fronte di salita):
    # qui vanno le azioni al rilascio.
    # Esempio: toggle, inverte lo
    # stato del LED a ogni rilascio.
    global stato
    stato = not stato
    LED.value(stato)

while True:
    if not P1.value():          # premuto
        waitUntilInputHigh(P1, 50)
        doOnRise()
```

</td>
</tr>
</table>

## 2. Versione non bloccante con ticks_ms()

Qui la variabile `armato` serve: senza, a pulsante rilasciato la differenza di tempo
resterebbe maggiore di 50 per sempre e l'azione scatterebbe a ogni giro.

<table>
<tr>
<th>Pull-down: azione sul fronte di discesa</th>
<th>Pull-up: azione sul fronte di salita</th>
</tr>
<tr>
<td>

```python
from machine import Pin
import time

P1  = Pin(12, Pin.IN)   # pull-down esterno
LED = Pin(13, Pin.OUT)
stato = False
t0 = 0
armato = False

def doOnFall():
    # RILASCIO (fronte di discesa):
    # qui vanno le azioni al rilascio.
    # Esempio: toggle, inverte lo
    # stato del LED a ogni rilascio.
    global stato
    stato = not stato
    LED.value(stato)

while True:
    if P1.value():              # premuto
        armato = True
        t0 = time.ticks_ms()
    elif armato and time.ticks_diff(
            time.ticks_ms(), t0) > 50:
        armato = False
        doOnFall()
```

</td>
<td>

```python
from machine import Pin
import time

P1  = Pin(12, Pin.IN, Pin.PULL_UP)  # pull-up interno
LED = Pin(13, Pin.OUT)
stato = False
t0 = 0
armato = False

def doOnRise():
    # RILASCIO (fronte di salita):
    # qui vanno le azioni al rilascio.
    # Esempio: toggle, inverte lo
    # stato del LED a ogni rilascio.
    global stato
    stato = not stato
    LED.value(stato)

while True:
    if not P1.value():          # premuto
        armato = True
        t0 = time.ticks_ms()
    elif armato and time.ticks_diff(
            time.ticks_ms(), t0) > 50:
        armato = False
        doOnRise()
```

</td>
</tr>
</table>

## 3. Versione non bloccante con DiffTimer

`start()` non fa nulla se il timer è già armato, quindi si può chiamare a ogni giro. A timer
fermo e azzerato `get()` restituisce 0: non serve controllare `timerState`.
Serve la versione corretta di `urutils.py` (vedi la sezione 3 di
[pulsanti-rilascio-mpy.md](pulsanti-rilascio-mpy.md)).

<table>
<tr>
<th>Pull-down: azione sul fronte di discesa</th>
<th>Pull-up: azione sul fronte di salita</th>
</tr>
<tr>
<td>

```python
from machine import Pin
import time
from urutils import DiffTimer

P1  = Pin(12, Pin.IN)   # pull-down esterno
LED = Pin(13, Pin.OUT)
stato = False
debt = DiffTimer()

def doOnFall():
    # RILASCIO (fronte di discesa):
    # qui vanno le azioni al rilascio.
    # Esempio: toggle, inverte lo
    # stato del LED a ogni rilascio.
    global stato
    stato = not stato
    LED.value(stato)

while True:
    if P1.value():              # premuto
        debt.start()            # niente se armato
        debt.reset()
    elif debt.get() > 50:
        debt.stop()
        debt.reset()
        doOnFall()
```

</td>
<td>

```python
from machine import Pin
import time
from urutils import DiffTimer

P1  = Pin(12, Pin.IN, Pin.PULL_UP)  # pull-up interno
LED = Pin(13, Pin.OUT)
stato = False
debt = DiffTimer()

def doOnRise():
    # RILASCIO (fronte di salita):
    # qui vanno le azioni al rilascio.
    # Esempio: toggle, inverte lo
    # stato del LED a ogni rilascio.
    global stato
    stato = not stato
    LED.value(stato)

while True:
    if not P1.value():          # premuto
        debt.start()            # niente se armato
        debt.reset()
    elif debt.get() > 50:
        debt.stop()
        debt.reset()
        doOnRise()
```

</td>
</tr>
</table>

## 4. Versione non bloccante con waitUntilNB()

`waitUntilNB()` della versione corretta di `urutils.py` fa ripartire il timer a ogni chiamata
con la condizione falsa, cioè a ogni lettura "premuto": scatta solo dopo 50 ms di pulsante
rilasciato senza interruzioni, quindi i rimbalzi del rilascio vengono ignorati.

<table>
<tr>
<th>Pull-down: azione sul fronte di discesa</th>
<th>Pull-up: azione sul fronte di salita</th>
</tr>
<tr>
<td>

```python
from machine import Pin
import time
from urutils import DiffTimer, waitUntilNB

P1  = Pin(12, Pin.IN)   # pull-down esterno
LED = Pin(13, Pin.OUT)
stato = False
debt = DiffTimer()

def doOnFall():
    # RILASCIO (fronte di discesa):
    # qui vanno le azioni al rilascio.
    # Esempio: toggle, inverte lo
    # stato del LED a ogni rilascio.
    global stato
    stato = not stato
    LED.value(stato)

while True:
    premuto = P1.value()
    if waitUntilNB(debt, not premuto, 50):
        doOnFall()
```

</td>
<td>

```python
from machine import Pin
import time
from urutils import DiffTimer, waitUntilNB

P1  = Pin(12, Pin.IN, Pin.PULL_UP)  # pull-up interno
LED = Pin(13, Pin.OUT)
stato = False
debt = DiffTimer()

def doOnRise():
    # RILASCIO (fronte di salita):
    # qui vanno le azioni al rilascio.
    # Esempio: toggle, inverte lo
    # stato del LED a ogni rilascio.
    global stato
    stato = not stato
    LED.value(stato)

while True:
    premuto = not P1.value()
    if waitUntilNB(debt, not premuto, 50):
        doOnRise()
```

</td>
</tr>
</table>

## 5. Versione con schedulatore non bloccante

Il rilascio viene riconosciuto con un ritardo fino a `TBASE` ms.

<table>
<tr>
<th>Pull-down: azione sul fronte di discesa</th>
<th>Pull-up: azione sul fronte di salita</th>
</tr>
<tr>
<td>

```python
from machine import Pin
import time

TBASE = 50
P1  = Pin(12, Pin.IN)   # pull-down esterno
LED = Pin(13, Pin.OUT)
stato = False
precm = 0
precval = 0

def doOnFall():
    # RILASCIO (fronte di discesa):
    # qui vanno le azioni al rilascio.
    # Esempio: toggle, inverte lo
    # stato del LED a ogni rilascio.
    global stato
    stato = not stato
    LED.value(stato)

while True:
    if time.ticks_diff(
            time.ticks_ms(), precm) >= TBASE:
        precm = time.ticks_ms()
        val = P1.value()
        if precval == 1 and val == 0:
            doOnFall()
        precval = val
```

</td>
<td>

```python
from machine import Pin
import time

TBASE = 50
P1  = Pin(12, Pin.IN, Pin.PULL_UP)  # pull-up interno
LED = Pin(13, Pin.OUT)
stato = False
precm = 0
precval = 1

def doOnRise():
    # RILASCIO (fronte di salita):
    # qui vanno le azioni al rilascio.
    # Esempio: toggle, inverte lo
    # stato del LED a ogni rilascio.
    global stato
    stato = not stato
    LED.value(stato)

while True:
    if time.ticks_diff(
            time.ticks_ms(), precm) >= TBASE:
        precm = time.ticks_ms()
        val = P1.value()
        if precval == 0 and val == 1:
            doOnRise()
        precval = val
```

</td>
</tr>
</table>

## Riferimenti

- [urutils.h](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/urutils.h): versione Arduino della libreria
- [pulsanti-rilascio-mpy.md](pulsanti-rilascio-mpy.md): versioni complete, con azione alla pressione e al rilascio
- [toggle-blink-pulldown-mpy.md](toggle-blink-pulldown-mpy.md): toggle e blink come due task concorrenti
