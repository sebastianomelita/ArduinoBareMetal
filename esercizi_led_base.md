
# Esercizi LED - Consegne

## 1. LED Sequenziali Base
Scrivere un programma che accenda due led con un tasto senza memoria. Quando il tasto è premuto, il primo led lampeggia dieci volte, dopodichè entrambi lampeggiano alternativamente indefinitamente.

## 2. LED Sequenziali con Temporizzazione
Scrivere un programma che accenda due led con un tasto senza memoria. Quando il tasto è premuto, i led lampeggiano 5 volte alternativamente in maniera rapida (100ms on/off), dopodichè entrambi lampeggiano alternativamente in maniera più lenta (500ms on/off) per un tempo indefinito.

## 3. LED Concorrenti
Scrivere e simulare un programma (Arduino o ESP32) che realizzi l'accensione di un led finché si preme un pulsante e, contemporaneamente, il blink di un altro led a frequenza costante (500ms on/off), indipendentemente dallo stato del pulsante.

## 4. LED Semaforo
Realizzare un programma Arduino che faccia lampeggiare un led (frequenza 1Hz) mentre altri tre led (rosso, verde e giallo) realizzano la tempistica di un semaforo (rosso 5s, verde 5s, giallo 2s, ciclicamente).

## 5. LED Pattern Complesso
Scrivere un programma per la gestione di tre LED. Alla pressione di un pulsante, i LED devono:
- LED1: lampeggiare 3 volte velocemente (200ms on/off)
- LED2 e LED3: dopo i lampeggi del LED1, lampeggiare alternativamente 4 volte (400ms per stato)
- Tutti i LED: lampeggiare simultaneamente 2 volte (800ms on/off)
- Tornare allo stato iniziale (tutti spenti)

## 6. LED Morse Code
Implementare un sistema che, alla pressione di un pulsante, faccia lampeggiare un LED secondo il codice Morse per la sequenza "SOS", con:
- Punto: LED acceso 200ms
- Linea: LED acceso 600ms
- Pausa tra elementi: 200ms
- Pausa tra lettere: 600ms
- Al termine della sequenza, attendere 2 secondi e ripetere

## 7. LED Base Pull-up/Pull-down
Realizzare un programma per ESP32 che utilizzi:
- Un pulsante configurato con pull-up interno
- Un pulsante configurato con pull-down esterno (10kΩ)
- Due LED in uscita standard
Quando si preme un pulsante, il LED associato deve accendersi per 2 secondi e poi spegnersi.

## 8. LED Open-Drain Base
Implementare un sistema con:
- Un pulsante con pull-up interno
- Due LED: uno in configurazione standard, uno in modalità open-drain con pull-up esterno (10kΩ)
Alla pressione del pulsante i LED devono lampeggiare alternativamente 5 volte (500ms per stato).

## 9. LED Sequenza con Pull
Progettare un sistema con:
- Un pulsante con pull-down esterno (10kΩ)
- Due LED: uno standard e uno open-drain con pull-up
Alla pressione del pulsante:
- Il primo LED deve lampeggiare 3 volte (200ms on/off)
- Il secondo LED deve accendersi per 2 secondi
- Entrambi lampeggiano insieme 2 volte (1s on/off)

## 10. LED Controllo Doppio
Realizzare un sistema con:
- Due pulsanti: uno pull-up interno, uno pull-down esterno (10kΩ)
- Due LED: uno standard, uno open-drain
Funzionamento:
- Primo pulsante: il LED standard lampeggia 3 volte velocemente (200ms)
- Secondo pulsante: il LED open-drain lampeggia 2 volte lentamente (1s)
- Se entrambi i pulsanti sono premuti: i LED lampeggiano alternativamente

## 11. LED Stati Multipli
Implementare un sistema con:
- Un pulsante con pull-up interno
- Un pulsante con pull-down esterno (10kΩ)
- Due LED: uno in configurazione standard, uno open-drain
Funzionamento:
- Primo pulsante premuto: LED standard lampeggia (500ms on/off)
- Secondo pulsante premuto: LED open-drain rimane acceso
- Entrambi i pulsanti premuti: i LED si accendono e spengono in sequenza

## 12. LED Sequenze Miste
Progettare un sistema che utilizzi:
- Un pulsante con pull-up interno
- Un pulsante con pull-down esterno (10kΩ)
- Due LED in modalità standard
- Un LED in modalità open-drain
Realizzare:
1. Primo pulsante premuto:
   - LED standard 1: tre lampeggi veloci (200ms)
   - LED standard 2: due lampeggi lenti (1s)
2. Secondo pulsante premuto:
   - LED open-drain: lampeggio continuo (500ms)
3. Entrambi i pulsanti premuti:
   - Tutti i LED lampeggiano in sequenza circolare
