"""
===============================================================================
CONSEGNA - ESERCIZIO A (FACILE): SEMAFORO PEDONALE A CHIAMATA, VERSIONE BLOCCANTE
===============================================================================
Classe terza - Sistemi e reti

Argomenti: pull-down, rimbalzi, le quattro situazioni critiche, gestione dei pulsanti con attese bloccanti (tecniche 1, 5 e 6).

Realizzate in Wokwi un semaforo per un attraversamento pedonale. Il semaforo delle auto resta verde finché un pedone non preme e rilascia il pulsante di chiamata; allora parte la sequenza che ferma le auto e fa passare il pedone.

OBIETTIVO

Il programma fa una cosa alla volta (aspetta il pedone, poi esegue la sequenza), quindi le attese bloccanti vanno bene: si usano time.sleep_ms() per le fasi del semaforo e waitUntilInputLow() della dispensa per il pulsante.

CIRCUITO IN WOKWI (ESP32 DevKit con MicroPython)

| Componente | Collegamento | A cosa serve |
| --- | --- | --- |
| Pulsante di chiamata | tra il pin 12 e 3V3 | premuto porta il pin 12 a 1 |
| Resistenza 10 kΩ | tra il pin 12 e GND | pull-down: a riposo il pin legge 0 |
| LED verde auto | pin 13, con resistenza 220 Ω | via libera alle auto |
| LED giallo auto | pin 27, con resistenza 220 Ω | attenzione |
| LED rosso auto | pin 26, con resistenza 220 Ω | auto ferme |
| LED verde pedoni | pin 25, con resistenza 220 Ω | il pedone può passare |
| LED rosso pedoni | pin 33, con resistenza 220 Ω | il pedone aspetta |

REQUISITI

1. A riposo: verde auto acceso, rosso pedoni acceso, gli altri spenti.
2. La chiamata vale al RILASCIO del pulsante.
3. Dopo il rilascio parte la sequenza: giallo auto 2 s → rosso auto + verde pedoni 5 s → rosso auto + rosso pedoni 1 s → di nuovo come al punto 1.
4. Ogni pressione produce una sola chiamata, anche se il contatto rimbalza.
5. Le pressioni fatte durante la sequenza vengono ignorate.

VINCOLI SUL CODICE

- Usate la tecnica 5 della dispensa sulle attese bloccanti ("Letture continue con ticks_ms()"): nel ciclo principale, se il pin vale 1 si chiama doOnRise(), poi waitUntilInputLow(P1, 50), poi doOnFall().
- doOnRise() è vuota; doOnFall() esegue la sequenza del semaforo.
- Ogni fase della sequenza in una funzione o in un blocco commentato.
- Il tempo di antirimbalzo è 50 ms.

COME PROVARLO

- [ ] All'avvio il semaforo è nello stato di riposo.
- [ ] Premete e tenete premuto 3 s: la sequenza parte solo quando rilasciate.
- [ ] Un clic breve: parte una sola sequenza, non due.
- [ ] Premete il pulsante mentre il pedone ha il verde: alla fine non riparte una seconda sequenza.

COSA CONSEGNARE

1. Il link al progetto Wokwi.
2. Il codice completo e commentato.
3. Le risposte alle domande.

DOMANDE

1. Disegnate il segnale del pin 12 durante una pressione, con i rimbalzi alla pressione e al rilascio. Segnate l'istante in cui waitUntilInputLow() termina.
2. Perché serve la resistenza da 10 kΩ? Cosa leggerebbe il pin 12, a pulsante rilasciato, senza di essa?
3. Se al posto della tecnica 5 usaste la tecnica 1 ("Una lettura ogni t ms"), cosa potrebbe succedere al rilascio? Che effetto avrebbe sul semaforo?
4. Il punto 5 dei requisiti è rispettato "gratis": perché? (Pensate a cosa fa il programma durante i time.sleep_ms().) Una pressione fatta durante la sequenza può comunque far ripartire il semaforo? Quando?
5. Il cavo del pulsante è lungo 10 m e passa vicino ai cavi di un motore. Quale delle quattro situazioni critiche diventa probabile? Quale tecnica della dispensa usereste e perché?
===============================================================================
"""

# SOLUZIONE - tecnica 5 bloccante (letture continue con ticks_ms())
from machine import Pin
import time

P1 = Pin(12, Pin.IN)                # pulsante di chiamata, pull-down: premuto = 1
VERDE_AUTO  = Pin(13, Pin.OUT)
GIALLO_AUTO = Pin(27, Pin.OUT)
ROSSO_AUTO  = Pin(26, Pin.OUT)
VERDE_PED   = Pin(25, Pin.OUT)
ROSSO_PED   = Pin(33, Pin.OUT)

def luci(va, ga, ra, vp, rp):
    # imposta in un colpo solo tutti e cinque i LED
    VERDE_AUTO.value(va)
    GIALLO_AUTO.value(ga)
    ROSSO_AUTO.value(ra)
    VERDE_PED.value(vp)
    ROSSO_PED.value(rp)

def riposo():
    luci(1, 0, 0, 0, 1)             # auto: verde, pedoni: rosso

def waitUntilInputLow(btn, t):
    # attende t ms di livello basso continuo: ogni lettura alta
    # (rimbalzo, disturbo o pulsante ancora premuto) fa ripartire il conteggio
    tl = time.ticks_ms()                            # istante dell'ultima lettura alta
    while time.ticks_diff(time.ticks_ms(), tl) <= t:
        if btn.value():
            tl = time.ticks_ms()                    # il conteggio riparte
        time.sleep_ms(1)                            # una lettura al millisecondo

def doOnRise():
    # PRESSIONE (fronte di salita): qui non serve nulla
    pass

def doOnFall():
    # RILASCIO (fronte di discesa): sequenza di attraversamento
    luci(0, 1, 0, 0, 1)             # giallo auto, rosso pedoni
    time.sleep_ms(2000)             # fase gialla: 2 s
    luci(0, 0, 1, 1, 0)             # rosso auto, verde pedoni
    time.sleep_ms(5000)             # attraversamento: 5 s
    luci(0, 0, 1, 0, 1)             # tutto rosso (sgombero)
    time.sleep_ms(1000)             # sgombero: 1 s
    riposo()

riposo()
while True:
    if P1.value():                  # prima lettura alta: pressione
        doOnRise()
        waitUntilInputLow(P1, 50)   # rilascio: 50 ms di livello basso continuo
        doOnFall()

"""
===============================================================================
RISPOSTE ALLE DOMANDE
===============================================================================
1. Diagramma (da disegnare): il pin 12 vale 0 a riposo; alla pressione rimbalza per qualche ms e poi resta 1; al rilascio rimbalza di nuovo e poi resta 0. Il ciclo principale entra al primo 1 della pressione; waitUntilInputLow() termina 50 ms dopo l'ULTIMO rimbalzo alto del rilascio, perché ogni lettura alta fa ripartire il conteggio. Solo allora parte la sequenza.

2. Senza la resistenza da 10 kΩ, a pulsante rilasciato il pin 12 non è collegato a nulla (ingresso flottante): legge valori casuali, influenzati dai disturbi e perfino dalla mano vicina al cavo. Il semaforo partirebbe da solo. Il pull-down tiene il pin a 0 quando il pulsante è aperto.

3. La tecnica 1 aspetta t ms e legge una sola volta. Se quella lettura cade in un istante basso dei rimbalzi del rilascio, l'attesa finisce e parte la sequenza; ma il pulsante sta ancora rimbalzando, quindi al giro successivo il ciclo principale può leggere un 1 e prenderlo per una nuova pressione (pressione fantasma). In questo programma il secondo "giro" però arriva dopo gli 8 s della sequenza, quando i rimbalzi sono finiti da un pezzo: l'effetto peggiore è che la sequenza parte durante i rimbalzi del rilascio. Il danno vero della tecnica 1 si vede quando l'azione è breve (per esempio un toggle), perché allora il rimbalzo successivo viene letto subito.

4. Durante i time.sleep_ms() della sequenza il programma non legge il pin 12: le pressioni fatte in quel tempo non vengono viste. Però una pressione ANCORA IN CORSO quando la sequenza finisce viene vista: al primo giro del ciclo principale dopo la sequenza il pin vale 1, il programma attende il rilascio e fa partire una seconda sequenza. (Verificato in simulazione: pressione da 8,0 a 10,0 s, fine sequenza a 9,2 s, nuova sequenza al rilascio.)

5. Con un cavo lungo vicino a un motore diventano probabili gli SPIKE a pulsante rilasciato (e i buchi a pulsante premuto). La tecnica 5 filtra i buchi (servono 50 ms di livello basso continuo) ma NON gli spike: un impulso alto di 2 ms a riposo viene preso per una pressione e, 50 ms dopo, per un rilascio, quindi il semaforo ferma le auto senza nessun pedone (verificato in simulazione). Conviene la tecnica 6, che chiede 50 ms di livello alto fermo anche per la pressione: filtra tutte e quattro le situazioni. Il prezzo è una pressione riconosciuta 50 ms dopo, irrilevante per un semaforo.
"""
