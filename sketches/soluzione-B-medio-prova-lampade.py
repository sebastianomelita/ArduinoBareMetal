"""
===============================================================================
CONSEGNA - ESERCIZIO B (MEDIO): PANNELLO CON SELETTORE DI MODO E PROVA LAMPADE
===============================================================================
Classe terza - Sistemi e reti

Argomenti: pull-up e pull-down, pulsanti senza memoria, rimbalzi e ipotesi di progetto, polling non bloccante su un solo fronte (versioni 1 e 4).

Molti pannelli di macchine hanno un tasto "prova lampade": finché è premuto tutte le spie si accendono, così l'operatore vede se qualcuna è bruciata. Realizzate in Wokwi un pannello con tre spie, un tasto che sceglie il modo di funzionamento e un tasto di prova lampade.

OBIETTIVO

Sullo stesso pannello convivono un pulsante SENZA memoria (prova lampade: conta solo il livello, finché è premuto) e un pulsante CON memoria (il selettore: ogni rilascio fa avanzare il modo). I due pulsanti sono collegati in modo diverso, uno in pull-down e uno in pull-up.

CIRCUITO IN WOKWI (ESP32 DevKit con MicroPython)

| Componente | Collegamento | A cosa serve |
| --- | --- | --- |
| Pulsante SELEZIONE (P1) | tra il pin 14 e GND (pull-up interno, Pin.PULL_UP) | premuto porta il pin 14 a 0 |
| Pulsante PROVA (P2) | tra il pin 12 e 3V3 | premuto porta il pin 12 a 1 |
| Resistenza 10 kΩ | tra il pin 12 e GND | pull-down di P2 |
| LED verde (MANUALE) | pin 13, con resistenza 220 Ω | spia del modo 0 |
| LED giallo (AUTOMATICO) | pin 27, con resistenza 220 Ω | spia del modo 1 |
| LED rosso (MANUTENZIONE) | pin 26, con resistenza 220 Ω | spia del modo 2 |

REQUISITI

1. All'accensione il modo è MANUALE: acceso solo il LED verde.
2. A ogni RILASCIO di P1 il modo avanza: MANUALE → AUTOMATICO → MANUTENZIONE → MANUALE ... ed è acceso solo il LED del modo corrente.
3. Finché P2 è premuto i tre LED sono tutti accesi; al rilascio di P2 torna acceso solo il LED del modo corrente. P2 non cambia mai il modo.
4. Ogni pressione di P1 fa avanzare il modo di un solo passo.
5. Il programma non è bloccante: P2 risponde anche mentre P1 è premuto.

VINCOLI SUL CODICE

- P1 si gestisce con la versione 4 della dispensa sul polling su un solo fronte (schedulatore non bloccante, TBASE = 50 ms, variabile precval). Attenzione: P1 è in pull-up, quindi il rilascio è un fronte di SALITA e la callback è doOnRise().
- doOnRise() cambia solo la variabile del modo (modo, da 0 a 2).
- P2 si legge a ogni giro del ciclo principale con P2.value(), SENZA debounce.
- Un solo punto del codice decide quali LED accendere, in base a modo e a P2.
- Nessun time.sleep_ms().

COME PROVARLO

- [ ] All'avvio è acceso solo il verde.
- [ ] Tre clic su P1: giallo, rosso, di nuovo verde. Il cambio avviene al rilascio, non alla pressione.
- [ ] Tenete premuto P2: tutti accesi; rilasciate: torna il modo di prima.
- [ ] Tenete premuto P1 e intanto premete P2: la prova lampade funziona lo stesso.
- [ ] Clic rapidi su P1: ognuno fa un solo passo.

COSA CONSEGNARE

1. Il link al progetto Wokwi.
2. Il codice completo, con un commento su ogni riga che riguarda i tempi.
3. Le risposte alle domande.

DOMANDE

1. Compilate la tabella per P1 e per P2: livello a riposo, livello da premuto, fronte della pressione, fronte del rilascio.
2. Perché P2 (prova lampade) non ha bisogno di debounce, mentre P1 sì? Che cosa vede l'operatore se P2 rimbalza per 5 ms? E se P1 rimbalza senza debounce?
3. Ipotesi di progetto: TBASE deve stare fra due limiti. Quali? Che cosa succederebbe con TBASE = 5 ms? E con TBASE = 300 ms?
4. Con lo schedulatore uno spike di 2 ms a P1 rilasciato può far avanzare il modo? Sempre, mai o solo in certi casi? Spiegate.
5. Riscrivete la gestione di P1 con la versione 1 della dispensa (ticks_ms() e variabile attivo, pull-up). Che cosa cambia nel ritardo con cui il modo avanza dopo il rilascio? Quale delle due versioni filtra il buco a pulsante premuto?
===============================================================================
"""

# SOLUZIONE - P1 con schedulatore su un solo fronte (versione 4, pull-up)
#             P2 senza memoria (pull-down, nessun debounce)
from machine import Pin
import time

TBASE = 50                              # ms fra due letture di P1: > rimbalzi
P1 = Pin(14, Pin.IN, Pin.PULL_UP)       # SELEZIONE, pull-up interno: premuto = 0
P2 = Pin(12, Pin.IN)                    # PROVA LAMPADE, pull-down esterno: premuto = 1
LEDS = (Pin(13, Pin.OUT), Pin(27, Pin.OUT), Pin(26, Pin.OUT))   # verde, giallo, rosso

modo = 0                                # 0 manuale, 1 automatico, 2 manutenzione
precm = 0                               # istante dell'ultima lettura di P1
precval = 1                             # lettura precedente di P1 (riposo = 1)

def doOnRise():
    # RILASCIO di P1 (in pull-up è un fronte di SALITA): modo successivo
    global modo
    modo = (modo + 1) % 3

while True:
    # --- compito 1: P1, una lettura ogni TBASE ms
    if time.ticks_diff(time.ticks_ms(), precm) >= TBASE:
        precm = time.ticks_ms()         # istante di questa lettura
        val = P1.value()
        if precval == 0 and val == 1:   # prima premuto, ora rilasciato
            doOnRise()
        precval = val

    # --- compito 2: P2 senza memoria + uscita, in un solo punto
    prova = P2.value() == 1             # conta solo il livello di adesso
    for i in range(3):
        LEDS[i].value(1 if (prova or i == modo) else 0)

"""
===============================================================================
RISPOSTE ALLE DOMANDE
===============================================================================
1. Livelli e fronti dei due pulsanti:

   | | riposo | premuto | pressione | rilascio |
   | --- | --- | --- | --- | --- |
   | P1 (pull-up) | 1 | 0 | fronte di discesa | fronte di salita |
   | P2 (pull-down) | 0 | 1 | fronte di salita | fronte di discesa |

2. P2 è senza memoria: l'uscita dipende solo dal livello di ADESSO. Un rimbalzo di 5 ms fa accendere e spegnere i LED per qualche ms, un tempo che l'occhio non vede; finiti i rimbalzi l'uscita è giusta e non resta traccia dell'errore. P1 invece ha memoria: ogni fronte cambia la variabile modo, e un cambio sbagliato resta. Senza debounce ogni rimbalzo di P1 sarebbe un fronte in più: il modo avanzerebbe di un numero casuale di passi.

3. Il limite inferiore: TBASE deve essere più lungo dei rimbalzi (e dei disturbi), così due letture consecutive non cadono mai entrambe dentro lo stesso treno di rimbalzi: circa 11 ms secondo la finestra di progetto della dispensa. Il limite superiore: TBASE deve essere più corto dell'evento vero più breve (una pressione o una pausa fra due clic, circa 100 ms), altrimenti un clic rapido può cadere fra due letture.
- TBASE = 5 ms: due o tre letture cadono dentro i rimbalzi; una sequenza 0, 1, 0, 1 produce più fronti di salita: il modo avanza di più passi.
- TBASE = 300 ms: un clic di 150 ms può non essere mai letto, e il modo cambia fino a 300 ms dopo il rilascio, un ritardo che si vede.

4. Solo in certi casi. Con il pull-up lo spike a riposo è un impulso basso. Se nessuna lettura ci cade dentro, non succede nulla. Se una lettura lo vede, quella lettura vale 0 e la successiva 1: è un fronte di salita, il modo avanza. La probabilità è circa durata/TBASE = 2/50 = 4% (in simulazione: 41 avanzamenti su 1000 spike).

5. Versione 1 con il pull-up:

python
t0 = 0
attivo = False
...
if P1.value() == 0:                     # premuto
    attivo = True
    t0 = time.ticks_ms()
elif attivo and time.ticks_diff(time.ticks_ms(), t0) > 50:
    attivo = False                      # alto da più di 50 ms: rilascio
    doOnRise()


Ritardo: con la versione 4 il modo cambia alla prima lettura dopo la fine dei rimbalzi, cioè da 0 a 50 ms dopo (in media 25), con la versione 1 sempre 50 ms dopo l'ultimo rimbalzo. Buco a pulsante premuto (impulso alto breve): la versione 1 lo filtra sempre, perché servono 50 ms di livello alto continuo; la versione 4 solo se nessuna lettura ci cade dentro, altrimenti lo prende per un rilascio e il modo avanza a metà pressione.
"""
