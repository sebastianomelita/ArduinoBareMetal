>[Torna all'indice](indexpulsanti.md) >[Consegne](consegneEserciziPulsanti.md) >[versione in C++](soluzioniEserciziPulsanti.md)


# Esercizi sui pulsanti - Soluzioni (MicroPython)

Le soluzioni usano un ESP32 con MicroPython (anche su Wokwi): pulsante sul pin 12 in **pull-down**, LED sui pin 13, 27, 26 e 25. Il codice è stato provato in un
simulatore con rimbalzi casuali da 1 a 10 ms, buchi di 2 ms a pulsante premuto e spike di 2 ms a
pulsante rilasciato. I risultati riportati con ogni soluzione sono quelli delle prove sulla versione C++, la più estesa. Anche la versione MicroPython è stata provata, con un numero minore di casi: 300 pressioni normali (errore massimo sulla durata 2 ms), 300 spike e 300 tocchi brevi (nessuna misura falsa) per l'esercizio 1, 100 sequenze per l'esercizio 2 (tutte giuste).

## 1. Tempo di pressione

[Consegna](consegneEserciziPulsanti.md#1-tempo-di-pressione)

### Scelta della tecnica

Serve il **livello stabile in due direzioni** (`changed()`, sezione 3 di
[pulsanti-rilascio-mpy.md](pulsanti-rilascio-mpy.md)), per tre motivi:

- conferma pressione e rilascio con **lo stesso ritardo** di 50 ms, quindi nella differenza fra i due
  istanti il ritardo si annulla;
- scarta da sola le pressioni più brevi di 50 ms, come chiede la consegna: un tocco non arriva
  nemmeno a `doOnRise()`, quindi non spegne i LED delle fasce;
- uno spike a riposo non produce una misura falsa, cioè una dose di zucchero non richiesta.

### Programma

```python
from machine import Pin
import time

P1  = Pin(12, Pin.IN)          # pulsante, pull-down esterno: premuto = 1
LED = Pin(13, Pin.OUT)         # LED di pressione
LED_VERDE  = Pin(27, Pin.OUT)  # poco zucchero
LED_GIALLO = Pin(26, Pin.OUT)  # medio
LED_ROSSO  = Pin(25, Pin.OUT)  # molto zucchero
val0  = 0                      # ultima lettura
val00 = 0                      # livello accettato
last  = 0                      # istante dell'ultima variazione
tPress = 0                     # istante della pressione confermata

def fascia(verde, giallo, rosso):
    LED_VERDE.value(verde)
    LED_GIALLO.value(giallo)
    LED_ROSSO.value(rosso)

def changed():
    # vero solo quando il pin ha un
    # nuovo livello fermo da più di 50 ms
    global val0, val00, last
    val = P1.value()
    if val != val0:
        val0 = val
        last = time.ticks_ms()
    elif val != val00 and time.ticks_diff(time.ticks_ms(), last) > 50:
        val00 = val
        return True
    return False

def doOnRise():
    # PRESSIONE confermata: si registra l'istante
    global tPress
    tPress = time.ticks_ms()
    LED.value(1)
    fascia(0, 0, 0)            # nuova misura: fasce spente

def doOnFall():
    # RILASCIO confermato: durata e fascia
    LED.value(0)
    durata = time.ticks_diff(time.ticks_ms(), tPress)
    if durata < 500:
        print("Durata:", durata, "ms, poco zucchero")
        fascia(1, 0, 0)
    elif durata <= 1500:
        print("Durata:", durata, "ms, medio")
        fascia(0, 1, 0)
    else:
        print("Durata:", durata, "ms, molto zucchero")
        fascia(0, 0, 1)

while True:
    if changed():
        if val00 == 1:
            doOnRise()
        else:
            doOnFall()
```

### Risposte

**1. Diagramma temporale.** La pressione vera inizia al primo rimbalzo. Il programma la conferma
50 ms dopo l'ultimo rimbalzo della pressione. Il rilascio vero inizia al primo rimbalzo del rilascio,
e il programma lo conferma 50 ms dopo l'ultimo. La durata vera va dall'inizio della pressione
all'inizio del rilascio.

**4. Errore di misura.**

| Tecnica | Pressione riconosciuta | Rilascio riconosciuto | Errore sulla durata |
| --- | --- | --- | --- |
| Pressione subito, rilascio confermato | al primo rimbalzo | 50 ms dopo l'ultimo rimbalzo del rilascio | **+50 ms** più la durata dei rimbalzi del rilascio (fino a 60 ms in simulazione) |
| Livello stabile (`changed()`) | 50 ms dopo l'ultimo rimbalzo della pressione | 50 ms dopo l'ultimo rimbalzo del rilascio | la differenza fra le durate dei due rimbalzi (al massimo 10 ms in simulazione) |

Con la prima tecnica l'errore si corregge togliendo 50 ms. In alternativa si registra l'istante
dell'ultima lettura alta e si calcola la durata fino a quello. Resta comunque un errore pari alla
durata dei rimbalzi. Con `changed()` non serve nessuna correzione.

**5. Situazioni critiche.**

| Situazione | Pressione subito, rilascio confermato | `changed()` |
| --- | --- | --- |
| Buco di 2 ms a pulsante premuto | nessun effetto: il buco dura meno di 50 ms | nessun effetto, ma se il buco cade nei primi 50 ms la pressione è confermata più tardi e la durata risulta più corta, fino a circa 50 ms |
| Spike di 2 ms a riposo | **una misura falsa** di circa 52 ms: "poco zucchero" e LED verde senza che nessuno abbia premuto (5000 casi su 5000 in simulazione) | nessuna misura |
| Tocco più breve di 50 ms | **viene misurato**, perché la durata calcolata include i 50 ms della conferma | nessuna misura, come chiede la consegna |

**6. Facoltativa: LED lampeggiante durante la pressione.** Il lampeggio usa un secondo tempo,
`tBlink`, e una variabile per lo stato del LED. Lo stato del LED si tiene in una variabile invece di rileggere il pin di
uscita: così il programma non dipende da come la scheda legge un pin configurato come uscita. La misura non cambia,
perché `tPress` è registrato solo in `doOnRise()`.

```python
tBlink = 0
ledAcceso = False

def doOnRise():
    global tPress, tBlink, ledAcceso
    tPress = time.ticks_ms()
    tBlink = time.ticks_ms()
    ledAcceso = True
    LED.value(1)
# doOnFall() spegne il LED e mette ledAcceso = False

while True:
    if changed():
        if val00 == 1:
            doOnRise()
        else:
            doOnFall()
    if val00 == 1 and time.ticks_diff(time.ticks_ms(), tBlink) >= 250:  # 2 Hz: 250 ms acceso, 250 spento
        tBlink = time.ticks_ms()
        ledAcceso = not ledAcceso
        LED.value(ledAcceso)
```

**Risultati in simulazione** (5000 prove per caso):

| Caso | Misure sbagliate o in più | Errore massimo sulla durata |
| --- | --- | --- |
| Pressioni normali con rimbalzi | 0 | 10 ms |
| Spike a riposo | 0 | — |
| Tocchi più brevi di 50 ms | 0 (nessuna misura, come richiesto) | — |
| Buco a pulsante premuto | 1 | 57 ms (buco nei primi 50 ms) |

L'unico caso sbagliato col buco è una pressione molto breve, con il buco proprio in mezzo: è il
limite discusso nell'esercizio 2, domanda 5.

## 2. Conteggio delle pressioni

[Consegna](consegneEserciziPulsanti.md#2-conteggio-delle-pressioni)

### Scelta della tecnica

Anche qui serve il **livello stabile in due direzioni**. Un contatore usa la logica a fronte, quindi
ogni evento falso diventa un errore permanente (vedi la risposta 1). La tecnica deve filtrare tutte e
quattro le situazioni critiche, compreso lo spike a riposo.

### Programma

```python
from machine import Pin
import time

P1 = Pin(12, Pin.IN)            # pulsante, pull-down esterno: premuto = 1
LED_VERDE = Pin(27, Pin.OUT)
LED_ROSSO = Pin(26, Pin.OUT)
CAPIENZA = 10
val0  = 0                       # ultima lettura
val00 = 0                       # livello accettato
last  = 0                       # istante dell'ultima variazione
tPress = 0
persone = 0

def changed():
    global val0, val00, last
    val = P1.value()
    if val != val0:
        val0 = val
        last = time.ticks_ms()
    elif val != val00 and time.ticks_diff(time.ticks_ms(), last) > 50:
        val00 = val
        return True
    return False

def aggiornaLed():
    LED_ROSSO.value(1 if persone >= CAPIENZA else 0)
    LED_VERDE.value(0 if persone >= CAPIENZA else 1)

def doOnRise():
    # PRESSIONE confermata: serve solo a misurare la durata
    global tPress
    tPress = time.ticks_ms()

def doOnFall():
    # RILASCIO confermato: azzeramento o conteggio
    global persone
    if time.ticks_diff(time.ticks_ms(), tPress) > 2000:
        persone = 0             # pressione lunga: azzera
    elif persone < CAPIENZA:
        persone += 1            # pressione breve: un ingresso
    print("Persone:", persone)
    aggiornaLed()

aggiornaLed()
while True:
    if changed():
        if val00 == 1:
            doOnRise()
        else:
            doOnFall()
```

### Risposte

**1. Livello e fronte.** Con la logica a **livello** (LED acceso finché il pulsante è premuto), un
evento falso produce un errore che dura quanto il disturbo: il LED si spegne per qualche millisecondo
e poi torna giusto. Con la logica a **fronte** (contatore, toggle), un evento falso lascia un errore
**permanente**: il contatore resta sbagliato finché qualcuno non lo corregge a mano. Per questo un
contatore ha bisogno della tecnica più robusta.

**4. Confronto con "pressione subito, rilascio confermato".** Le due versioni contano allo stesso modo
i rimbalzi e i buchi, che entrambe filtrano. Divergono con uno **spike a riposo**, la situazione
critica 4. Per esempio, il cavo del pulsante passa vicino a un motore o a un relè, oppure qualcuno
scarica elettricità statica sul pulsante. La versione a una direzione scambia lo spike per una
pressione, poi conferma il rilascio 50 ms dopo, e conta un ingresso che non c'è. La versione con
`changed()` non conta nulla.

**5. Pressioni rapide.** Ogni pressione deve restare alta per più di 50 ms dopo l'ultimo rimbalzo.
Poi il pulsante deve restare basso per altri 50 ms prima della pressione successiva. Il periodo
minimo è quindi di poco più di 100 ms, più la durata dei rimbalzi: circa **8-10 pressioni al
secondo**. Una pressione più breve di 50 ms viene scartata come un disturbo, e quindi **non viene
contata**: è il prezzo della robustezza.

Per lo stesso motivo una pressione corta con un buco proprio in mezzo può andare persa. Per esempio,
in una pressione di 100 ms con un buco a metà, nessuna delle due parti resta ferma 50 ms. In
simulazione è successo in 11 sequenze su 2000, sempre con pressioni di circa 100 ms. Per evitarlo,
la pressione deve durare almeno il doppio del tempo di debounce, oppure il tempo di debounce va
ridotto, ma mai sotto i 50 ms delle dispense.

**6. Facoltativa: secondo pulsante con `Debouncer`.** Ogni oggetto `Debouncer` conserva lo stato di
**un** pulsante: l'ultima lettura (`val0`), il livello accettato (`val00`) e l'istante dell'ultima
variazione (`last`). Con un solo oggetto per due pin, le letture di P1 e P2 si mescolerebbero: ogni
lettura di un pin farebbe ripartire il conteggio dell'altro, e nessuno dei due verrebbe mai
confermato.

```python
from machine import Pin
import time
from urutils import Debouncer

P1 = Pin(12, Pin.IN)            # entrate
P2 = Pin(14, Pin.IN)            # uscite
LED_VERDE = Pin(27, Pin.OUT)
LED_ROSSO = Pin(26, Pin.OUT)
CAPIENZA = 10
entrata = Debouncer(0)          # un oggetto per pulsante
uscita  = Debouncer(0)
tPress = 0
persone = 0

def aggiornaLed():
    LED_ROSSO.value(1 if persone >= CAPIENZA else 0)
    LED_VERDE.value(0 if persone >= CAPIENZA else 1)
    print("Persone:", persone)

aggiornaLed()
while True:
    if entrata.changed(P1, 50):
        if entrata.val00 == 1:
            tPress = time.ticks_ms()            # pressione
        else:                                   # rilascio
            if time.ticks_diff(time.ticks_ms(), tPress) > 2000:
                persone = 0
            elif persone < CAPIENZA:
                persone += 1
            aggiornaLed()
    if uscita.changed(P2, 50) and uscita.val00 == 0:
        if persone > 0:                         # rilascio di P2: un'uscita
            persone -= 1
        aggiornaLed()
```

**Risultati in simulazione.** 2000 sequenze, ciascuna di dodici pressioni brevi seguite da una
lunga. Rimbalzi sempre presenti, buchi e spike aggiunti a caso. Il contatore è arrivato a 10 e si è
fermato, poi la pressione lunga lo ha azzerato con il LED verde acceso, in tutte le sequenze tranne
11. Gli 11 errori sono le pressioni di circa 100 ms con il buco in mezzo, discusse nella risposta 5.

>[Torna all'indice](indexpulsanti.md) >[Consegne](consegneEserciziPulsanti.md) >[versione in C++](soluzioniEserciziPulsanti.md)
