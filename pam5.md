### Trasmissione full-duplex e cancellazione dell'eco

#### Il principio della trasmissione full-duplex

Una caratteristica chiave di 1000BASE-T è la comunicazione full-duplex su ogni coppia di cavi:
- Ogni coppia trasporta contemporaneamente dati in entrambe le direzioni
- I circuiti di cancellazione dell'eco (echo cancellation) separano i segnali nelle due direzioni
- Questa tecnica raddoppia l'efficienza dell'utilizzo della banda disponibile

#### Problema dell'eco nei sistemi full-duplex

Quando un dispositivo trasmette e riceve simultaneamente sulla stessa coppia di cavi, si verificano due fenomeni problematici:

1. **Eco di ritorno** (Return Loss): Una parte del segnale trasmesso si riflette e ritorna al trasmettitore a causa di disadattamenti di impedenza nei connettori, nelle giunzioni e nei terminatori.

2. **Eco locale** (Local Echo): Il segnale trasmesso si accoppia direttamente al circuito di ricezione all'interno del transceiver stesso.

Senza un meccanismo di cancellazione, questi echi renderebbero impossibile distinguere il segnale ricevuto dalla stazione remota dal proprio segnale riflesso.

```
            ┌──────────────┐
            │  Transceiver │
            │              │
   ┌────────►  Trasmettitore
   │        │              │
   │        │              │   Eco di        Segnale
   │        │              │   ritorno       trasmesso
   │        │              │     ◄───┐         ───►
   │        │              │         │           │
Segnale da  │              ├─────────┴───────────┴─────
trasmettere │              │   Cavo Ethernet (1 coppia)
            │              ├─────────┬───────────┬─────
   ▲        │              │         │           │
   │        │              │     ┌───┘         ◄───┐
   │        │              │   Eco            Segnale
   │        │ Ricevitore   │   locale        ricevuto
   └────────┤              │
            │              │
            └──────────────┘
```

#### Funzionamento della cancellazione dell'eco

Il processo di cancellazione dell'eco in un transceiver 1000BASE-T funziona secondo questi principi:

1. **Modellazione dell'eco**: Il cancellatore crea un modello preciso di come il segnale trasmesso viene riflesso nel canale di ricezione.

2. **Generazione del segnale di cancellazione**: In base al modello e al segnale trasmesso, viene generata una "replica" dell'eco atteso.

3. **Sottrazione del segnale**: Questa replica viene sottratta dal segnale ricevuto, eliminando la componente di eco.

4. **Adattamento continuo**: I parametri del modello vengono continuamente aggiornati per adattarsi alle variazioni del canale.

```
     Segnale da   ┌───────────┐
     trasmettere  │Trasmettitore│ ───────────────────────►
         │        └───────────┘ 
         │                                    Canale di
         │                                   trasmissione
         │                                       │
         ▼                                       │
   ┌────────────┐                                │
   │ Modello di │                                │
   │    eco     │                                │
   └─────┬──────┘                                │
         │                                       │
         │  Eco predetto                         │
         ▼                                       ▼
   ┌────────────┐  Eco       ┌───────────┐  Segnale + Eco
   │ Sottrazione│◄───────────┤Ricevitore │◄──────────────
   └─────┬──────┘  cancellato└───────────┘
         │
         ▼
    Segnale pulito
    (solo segnale
     ricevuto)
```

#### Implementazione digitale

Nei transceiver moderni, la cancellazione dell'eco è implementata digitalmente:

1. **Convertitori A/D e D/A**: Il segnale ricevuto viene digitalizzato, e il segnale di cancellazione viene riconvertito in analogico prima della sottrazione.

2. **Filtri adattivi FIR**: Vengono utilizzati filtri digitali a risposta impulsiva finita (FIR) per modellare l'eco. I coefficienti di questi filtri vengono aggiornati in tempo reale.

3. **Algoritmi adattivi**: Algoritmi come LMS (Least Mean Square) o RLS (Recursive Least Squares) aggiornano continuamente i coefficienti del filtro per minimizzare l'errore residuo.

#### Prestazioni e limiti

La qualità della cancellazione dell'eco è misurata in decibel (dB) di attenuazione:

- I transceiver 1000BASE-T tipicamente forniscono 40-60 dB di cancellazione dell'eco
- Questo significa che il segnale di eco viene ridotto di un fattore 10,000-1,000,000 volte
- Nonostante ciò, una piccola componente residua dell'eco rimane sempre nel segnale ricevuto
- La precisione dei componenti analogici, la risoluzione dei convertitori A/D e D/A, e la potenza di calcolo disponibile limitano le prestazioni del sistema

#### Differenza con Half-Duplex e altri sistemi

Per comprendere meglio l'importanza della cancellazione dell'eco:

- **Sistemi half-duplex**: Alternano trasmissione e ricezione nel tempo, evitando del tutto il problema dell'eco ma dimezzando l'efficienza del canale.

- **Sistemi a frequenze separate** (FDD): Utilizzano bande di frequenza diverse per trasmissione e ricezione, evitando l'interferenza ma richiedendo più spettro.

- **Sistemi full-duplex con cancellazione dell'eco**: Ottengono la massima efficienza spettrale permettendo comunicazioni simultanee bidirezionali sulla stessa banda di frequenza e sullo stesso mezzo fisico.
