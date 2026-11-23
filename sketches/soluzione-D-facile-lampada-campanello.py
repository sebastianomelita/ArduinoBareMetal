"""
===============================================================================
CONSEGNA - ESERCIZIO D (FACILE): LAMPADA CON TOGGLE E CAMPANELLO, VERSIONE BLOCCANTE
===============================================================================
Classe terza - Sistemi e reti

Argomenti: pull-up e pull-down, rimbalzi, pulsanti senza memoria, gestione dei pulsanti con attese bloccanti (tecniche 1, 5 e 6).

Realizzate in Wokwi l'ingresso di una casa: un pulsante accende e spegne la luce del portico (toggle), un secondo pulsante è il campanello, che suona solo finché è premuto.

OBIETTIVO

Mettere a confronto un pulsante CON memoria (il toggle della luce, che ha bisogno del debounce) e uno SENZA memoria (il campanello, che segue il livello del pin), e scoprire il limite di una tecnica bloccante quando i pulsanti sono due.

CIRCUITO IN WOKWI (ESP32 DevKit con MicroPython)

| Componente | Collegamento | A cosa serve |
| --- | --- | --- |
| Pulsante LUCE (P1) | tra il pin 12 e 3V3 | premuto porta il pin 12 a 1 |
| Resistenza 10 kΩ | tra il pin 12 e GND | pull-down di P1 |
| Pulsante CAMPANELLO (P2) | tra il pin 14 e GND (pull-up interno, Pin.PULL_UP) | premuto porta il pin 14 a 0 |
| LED bianco o giallo | pin 13, con resistenza 220 Ω | luce del portico |
| Buzzer piezo | tra il pin 27 e GND, comandato in PWM | campanello |

REQUISITI

1. All'accensione la luce è spenta e il campanello tace.
2. A ogni RILASCIO di P1 la luce cambia stato (toggle).
3. Ogni pressione di P1 produce un solo cambio, anche se il contatto rimbalza.
4. Finché P2 è premuto il buzzer suona (PWM a 1000 Hz, duty_u16(32768)); quando viene rilasciato smette (duty_u16(0)).

VINCOLI SUL CODICE

- P1 si gestisce con la tecnica 5 della dispensa sulle attese bloccanti (letture continue con ticks_ms()): se il pin vale 1, doOnRise(), poi waitUntilInputLow(P1, 50), poi doOnFall().
- doOnRise() è vuota; doOnFall() fa il toggle della luce.
- P2 si legge con P2.value() a ogni giro del ciclo principale, senza debounce.

COME PROVARLO

- [ ] Clic su P1: la luce si accende al rilascio; secondo clic: si spegne.
- [ ] Clic rapidi su P1: ogni clic fa un solo cambio.
- [ ] Tenete premuto P2: il buzzer suona finché non rilasciate.
- [ ] Tenete premuto P1 e intanto premete P2: cosa succede? Annotatelo, serve per la domanda 4.

COSA CONSEGNARE

1. Il link al progetto Wokwi.
2. Il codice completo e commentato.
3. Le risposte alle domande.

DOMANDE

1. Compilate la tabella per P1 e per P2: livello a riposo, livello da premuto, fronte della pressione, fronte del rilascio.
2. Perché il campanello non ha bisogno di debounce, mentre la luce sì? Che cosa succederebbe alla luce se P1 rimbalzasse e il programma non avesse debounce?
3. Con la tecnica 1 ("Una lettura ogni t ms") al posto della 5, che errore può capitare al rilascio di P1? Che effetto vede chi usa la luce?
4. Nella prova con P1 tenuto premuto, il campanello funziona? Perché? È un limite della tecnica 5 o di tutte le tecniche bloccanti?
5. Uno spike di 2 ms sul pin 12, a P1 rilasciato, cambia lo stato della luce con la tecnica 5? E con la tecnica 6? Quale delle due scegliereste se il cavo del pulsante fosse lungo e disturbato?
===============================================================================
"""

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

"""
===============================================================================
RISPOSTE ALLE DOMANDE
===============================================================================
1. Livelli e fronti dei due pulsanti:

   | | riposo | premuto | pressione | rilascio |
   | --- | --- | --- | --- | --- |
   | P1 (pull-down) | 0 | 1 | fronte di salita | fronte di discesa |
   | P2 (Pin.PULL_UP) | 1 | 0 | fronte di discesa | fronte di salita |

2. Il campanello è un pulsante senza memoria: il buzzer segue il livello di adesso. Se P2 rimbalza per 5 ms, il suono si accende e spegne per 5 ms e nessuno se ne accorge; poi il livello si stabilizza e l'uscita è giusta. La luce ha memoria: ogni rilascio cambia lo stato, e un cambio in più resta. Senza debounce ogni rimbalzo sarebbe un rilascio in più e la luce cambierebbe un numero casuale di volte: a volte si accende, a volte no.

3. Con la tecnica 1 al rilascio una lettura può cadere in un istante basso dei rimbalzi: l'attesa finisce e la luce cambia. Il pulsante però sta ancora rimbalzando: al giro successivo il ciclo principale legge un 1, lo prende per una nuova pressione e alla fine la luce cambia di nuovo (pressione fantasma). Chi usa la luce vede che ogni tanto il clic "non funziona": la luce si accende e si rispegne subito.

4. No. Finché P1 è premuto il programma è fermo dentro waitUntilInputLow(): il pin 14 non viene letto, quindi il buzzer resta bloccato nello stato in cui era quando si è premuto P1 (se taceva, tace; se suonava, continua a suonare anche dopo aver lasciato P2). Si sblocca al rilascio di P1. È un limite di tutte le tecniche bloccanti: mentre aspettano un pulsante non fanno nient'altro. Con la tecnica 6 andrebbe peggio: il ciclo principale resta fermo anche in waitUntilInputHigh(), cioè sempre, a pulsante rilasciato, e il campanello non funzionerebbe mai.

5. Tecnica 5: sì. Il ciclo principale legge il pin a ogni giro, vede l'impulso alto, lo prende per una pressione e 50 ms dopo conferma il rilascio: la luce cambia (verificato in simulazione). Tecnica 6: no, perché la pressione deve restare 1 per 50 ms di fila e uno spike di 2 ms fa solo ripartire il conteggio. Con un cavo lungo e disturbato è più sicura la 6, ma in questo circuito bloccherebbe il campanello (risposta 4): servirebbe una tecnica non bloccante.

VERIFICA IN SIMULAZIONE

200 clic con rimbalzi: 200 cambi della luce; il buzzer suona finché P2 è premuto; uno spike di 2 ms sul pin 12 a riposo cambia la luce.
"""
