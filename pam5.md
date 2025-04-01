# Modulazione PAM-5: Guida Completa

## Indice

- [Introduzione](#introduzione)
- [Cos'è PAM-5?](#cosè-pam-5)
- [Come funziona PAM-5](#come-funziona-pam-5)
- [Implementazione in Gigabit Ethernet](#implementazione-in-gigabit-ethernet)
- [Vantaggi e sfide](#vantaggi-e-sfide)
- [Diagramma ad occhio](#diagramma-ad-occhio)
- [Confronto con altre modulazioni](#confronto-con-altre-modulazioni)
- [Applicazioni oltre Gigabit Ethernet](#applicazioni-oltre-gigabit-ethernet)
- [Riferimenti e risorse](#riferimenti-e-risorse)

## Introduzione

La **Modulazione di Ampiezza dell'Impulso** (PAM - Pulse Amplitude Modulation) è una tecnica di modulazione che trasmette dati variando l'ampiezza (altezza) di una serie di impulsi in base ai dati da trasmettere. Il numero che segue l'acronimo PAM (es. PAM-5) indica il numero di livelli di ampiezza distinti utilizzati nella modulazione.

PAM è una famiglia di tecniche di modulazione digitale ampiamente utilizzata nelle comunicazioni moderne, che offre un buon compromesso tra efficienza spettrale e complessità di implementazione.

## Cos'è PAM-5?

**PAM-5** è una forma di modulazione che utilizza **cinque livelli di tensione distinti** per trasmettere dati. È ampiamente utilizzata nelle comunicazioni digitali ad alta velocità, in particolare nelle reti Gigabit Ethernet (1000BASE-T).

I cinque livelli di tensione in PAM-5 tipicamente sono:
- -2 volts
- -1 volt
- 0 volt
- +1 volt
- +2 volts

```svg
<svg width="500" height="300" xmlns="http://www.w3.org/2000/svg">
  <!-- Background -->
  <rect width="500" height="300" fill="#f8f9fa"/>
  
  <!-- Title -->
  <text x="250" y="30" font-family="Arial, sans-serif" font-size="20" text-anchor="middle" font-weight="bold">Livelli di tensione PAM-5</text>
  
  <!-- Axes -->
  <line x1="50" y1="250" x2="450" y2="250" stroke="#333" stroke-width="2"/>
  <line x1="50" y1="70" x2="50" y2="250" stroke="#333" stroke-width="2"/>
  
  <!-- Voltage levels -->
  <line x1="50" y1="90" x2="450" y2="90" stroke="#ddd" stroke-width="1" stroke-dasharray="5,5"/>
  <line x1="50" y1="130" x2="450" y2="130" stroke="#ddd" stroke-width="1" stroke-dasharray="5,5"/>
  <line x1="50" y1="170" x2="450" y2="170" stroke="#ddd" stroke-width="1" stroke-dasharray="5,5"/>
  <line x1="50" y1="210" x2="450" y2="210" stroke="#ddd" stroke-width="1" stroke-dasharray="5,5"/>
  
  <!-- Level labels -->
  <text x="40" y="90" font-family="Arial, sans-serif" font-size="14" text-anchor="end">+2V</text>
  <text x="40" y="130" font-family="Arial, sans-serif" font-size="14" text-anchor="end">+1V</text>
  <text x="40" y="170" font-family="Arial, sans-serif" font-size="14" text-anchor="end">0V</text>
  <text x="40" y="210" font-family="Arial, sans-serif" font-size="14" text-anchor="end">-1V</text>
  <text x="40" y="250" font-family="Arial, sans-serif" font-size="14" text-anchor="end">-2V</text>
  
  <!-- Sample waveform -->
  <polyline points="50,170 80,170 80,130 120,130 120,90 160,90 160,130 200,130 200,170 240,170 240,210 280,210 280,250 320,250 320,210 360,210 360,170 400,170 400,130 440,130 440,90" 
           fill="none" stroke="#3498db" stroke-width="3"/>
  
  <!-- Voltage markers -->
  <circle cx="50" cy="170" r="5" fill="#3498db"/>
  <circle cx="80" cy="130" r="5" fill="#3498db"/>
  <circle cx="120" cy="90" r="5" fill="#3498db"/>
  <circle cx="160" cy="130" r="5" fill="#3498db"/>
  <circle cx="200" cy="170" r="5" fill="#3498db"/>
  <circle cx="240" cy="210" r="5" fill="#3498db"/>
  <circle cx="280" cy="250" r="5" fill="#3498db"/>
  <circle cx="320" cy="210" r="5" fill="#3498db"/>
  <circle cx="360" cy="170" r="5" fill="#3498db"/>
  <circle cx="400" cy="130" r="5" fill="#3498db"/>
  <circle cx="440" cy="90" r="5" fill="#3498db"/>
  
  <!-- Time markers -->
  <text x="50" y="270" font-family="Arial, sans-serif" font-size="12" text-anchor="middle">0</text>
  <text x="120" y="270" font-family="Arial, sans-serif" font-size="12" text-anchor="middle">T1</text>
  <text x="200" y="270" font-family="Arial, sans-serif" font-size="12" text-anchor="middle">T2</text>
  <text x="280" y="270" font-family="Arial, sans-serif" font-size="12" text-anchor="middle">T3</text>
  <text x="360" y="270" font-family="Arial, sans-serif" font-size="12" text-anchor="middle">T4</text>
  <text x="440" y="270" font-family="Arial, sans-serif" font-size="12" text-anchor="middle">T5</text>
</svg>
```

## Come funziona PAM-5

### Efficienza di codifica

Con 5 livelli di segnale, ogni simbolo PAM-5 può rappresentare log₂(5) ≈ 2,32 bit di informazione. Poiché non possiamo trasmettere una frazione di bit, questa capacità viene sfruttata attraverso schemi di codifica speciali.

### Mappatura bit-livelli

In Gigabit Ethernet, per esempio, viene utilizzato uno schema particolare:

- 8 bit vengono codificati in 4 simboli PAM-5 (uno per coppia di cavi)
- La capacità teorica di 4 simboli PAM-5 è 4 × 2,32 = 9,28 bit, quindi c'è sufficiente "spazio" per codificare 8 bit più alcune informazioni aggiuntive utilizzate per il controllo degli errori

### Differenza con codifiche precedenti

A differenza della codifica 4B5B utilizzata in Fast Ethernet (dove 4 bit di dati vengono codificati in blocchi di 5 bit), PAM-5 sfrutta i diversi livelli di ampiezza per trasmettere più bit per simbolo. Questo approccio permette di raggiungere velocità più elevate sullo stesso mezzo fisico.

## Implementazione in Gigabit Ethernet

Nel Gigabit Ethernet su cavo in rame (1000BASE-T), il PAM-5 viene implementato in modo particolarmente intelligente:

```svg
<svg width="500" height="250" xmlns="http://www.w3.org/2000/svg">
  <!-- Background -->
  <rect width="500" height="250" fill="#f8f9fa"/>
  
  <!-- Title -->
  <text x="250" y="30" font-family="Arial, sans-serif" font-size="20" text-anchor="middle" font-weight="bold">Implementazione in Gigabit Ethernet</text>
  
  <!-- Cable pairs illustration -->
  <rect x="50" y="70" width="400" height="30" fill="#e74c3c" fill-opacity="0.2" stroke="#e74c3c"/>
  <text x="250" y="90" font-family="Arial, sans-serif" font-size="14" text-anchor="middle">Coppia 1: 250 Mbps (PAM-5 @ 125 Mbaud)</text>
  
  <rect x="50" y="110" width="400" height="30" fill="#e67e22" fill-opacity="0.2" stroke="#e67e22"/>
  <text x="250" y="130" font-family="Arial, sans-serif" font-size="14" text-anchor="middle">Coppia 2: 250 Mbps (PAM-5 @ 125 Mbaud)</text>
  
  <rect x="50" y="150" width="400" height="30" fill="#f1c40f" fill-opacity="0.2" stroke="#f1c40f"/>
  <text x="250" y="170" font-family="Arial, sans-serif" font-size="14" text-anchor="middle">Coppia 3: 250 Mbps (PAM-5 @ 125 Mbaud)</text>
  
  <rect x="50" y="190" width="400" height="30" fill="#2ecc71" fill-opacity="0.2" stroke="#2ecc71"/>
  <text x="250" y="210" font-family="Arial, sans-serif" font-size="14" text-anchor="middle">Coppia 4: 250 Mbps (PAM-5 @ 125 Mbaud)</text>
  
  <!-- Total -->
  <text x="250" y="240" font-family="Arial, sans-serif" font-size="16" text-anchor="middle" font-weight="bold">Totale: 1000 Mbps (1 Gbps)</text>
</svg>
```

1. Si utilizzano **4 coppie di cavi** (gli stessi presenti nei normali cavi Ethernet Cat5e/Cat6)
2. Su ciascuna coppia, viene trasmesso un segnale PAM-5 a 125 megabaud (125 milioni di simboli al secondo)
3. Ogni simbolo PAM-5 trasporta 2 bit di informazione utile
4. Calcolo totale del throughput: 4 coppie × 125 Mbaud × 2 bit/simbolo = 1000 Mbps (1 Gbps)

### Codifica 4D-PAM5

In realtà, Gigabit Ethernet utilizza una versione più sofisticata chiamata 4D-PAM5, dove:
- I simboli sulle 4 coppie sono considerati insieme (4 dimensioni)
- Viene utilizzata una codifica trellis per il controllo degli errori
- I 4 simboli PAM-5 trasmessi simultaneamente codificano 8 bit di dati

### Trasmissione full-duplex

Un'altra caratteristica importante di 1000BASE-T è la trasmissione full-duplex:
- Ogni coppia di cavi trasmette in entrambe le direzioni simultaneamente
- I circuiti di cancellazione dell'eco permettono di separare i segnali in arrivo e in partenza sulla stessa coppia

## Vantaggi e sfide

### Vantaggi di PAM-5

1. **Efficienza spettrale**: Trasporta più bit per simbolo rispetto a tecniche più semplici come NRZ (Non Return to Zero)
2. **Ridotta larghezza di banda**: Per trasmettere la stessa quantità di dati, richiede una minore larghezza di banda rispetto a tecniche a 2 livelli
3. **Velocità elevate**: Permette di raggiungere velocità di 1 Gbps su cavi in rame standard

### Sfide di PAM-5

1. **Maggiore sensibilità al rumore**: Poiché i livelli di segnale sono più vicini tra loro rispetto a modulazioni con meno livelli, il segnale è più suscettibile al rumore
2. **Complessità del circuito**: I circuiti per generare e interpretare 5 livelli distinti sono più complessi
3. **Maggiore potenza richiesta**: La necessità di generare e rilevare più livelli di tensione richiede maggiore potenza

## Diagramma ad occhio

Il "diagramma ad occhio" è un metodo visivo per valutare la qualità di un segnale digitale. Per PAM-5, il diagramma mostra 4 "occhi" distinti tra i 5 livelli di segnale.

```svg
<svg width="500" height="300" xmlns="http://www.w3.org/2000/svg">
  <!-- Background -->
  <rect width="500" height="300" fill="#f8f9fa"/>
  
  <!-- Title -->
  <text x="250" y="30" font-family="Arial, sans-serif" font-size="20" text-anchor="middle" font-weight="bold">Diagramma ad occhio PAM-5</text>
  
  <!-- Axes -->
  <line x1="50" y1="250" x2="450" y2="250" stroke="#333" stroke-width="2"/>
  <line x1="50" y1="90" x2="50" y2="250" stroke="#333" stroke-width="2"/>
  
  <!-- Voltage levels -->
  <line x1="50" y1="90" x2="450" y2="90" stroke="#ddd" stroke-width="1" stroke-dasharray="5,5"/>
  <line x1="50" y1="130" x2="450" y2="130" stroke="#ddd" stroke-width="1" stroke-dasharray="5,5"/>
  <line x1="50" y1="170" x2="450" y2="170" stroke="#ddd" stroke-width="1" stroke-dasharray="5,5"/>
  <line x1="50" y1="210" x2="450" y2="210" stroke="#ddd" stroke-width="1" stroke-dasharray="5,5"/>
  
  <!-- Level labels -->
  <text x="40" y="90" font-family="Arial, sans-serif" font-size="14" text-anchor="end">+2V</text>
  <text x="40" y="130" font-family="Arial, sans-serif" font-size="14" text-anchor="end">+1V</text>
  <text x="40" y="170" font-family="Arial, sans-serif" font-size="14" text-anchor="end">0V</text>
  <text x="40" y="210" font-family="Arial, sans-serif" font-size="14" text-anchor="end">-1V</text>
  <text x="40" y="250" font-family="Arial, sans-serif" font-size="14" text-anchor="end">-2V</text>
  
  <!-- Eye pattern -->
  <!-- First eye (between -2V and -1V) -->
  <path d="M50,250 C100,250 150,210 250,210 C350,210 400,250 450,250 C400,250 350,210 250,210 C150,210 100,250 50,250" 
        fill="#3498db" fill-opacity="0.1" stroke="#2980b9" stroke-width="1"/>
  
  <!-- Second eye (between -1V and 0V) -->
  <path d="M50,210 C100,210 150,170 250,170 C350,170 400,210 450,210 C400,210 350,170 250,170 C150,170 100,210 50,210" 
        fill="#3498db" fill-opacity="0.1" stroke="#2980b9" stroke-width="1"/>
  
  <!-- Third eye (between 0V and +1V) -->
  <path d="M50,170 C100,170 150,130 250,130 C350,130 400,170 450,170 C400,170 350,130 250,130 C150,130 100,170 50,170" 
        fill="#3498db" fill-opacity="0.1" stroke="#2980b9" stroke-width="1"/>
  
  <!-- Fourth eye (between +1V and +2V) -->
  <path d="M50,130 C100,130 150,90 250,90 C350,90 400,130 450,130 C400,130 350,90 250,90 C150,90 100,130 50,130" 
        fill="#3498db" fill-opacity="0.1" stroke="#2980b9" stroke-width="1"/>
  
  <!-- Add some transition lines to make the eye diagram more realistic -->
  <path d="M50,90 C75,90 100,210 125,210" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M50,90 C75,90 100,250 125,250" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M50,130 C75,130 100,90 125,90" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M50,130 C75,130 100,170 125,170" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M50,170 C75,170 100,130 125,130" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M50,170 C75,170 100,210 125,210" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M50,210 C75,210 100,90 125,90" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M50,210 C75,210 100,170 125,170" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M50,250 C75,250 100,130 125,130" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M50,250 C75,250 100,210 125,210" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  
  <!-- Mirror the transitions on the right side -->
  <path d="M375,90 C400,90 425,210 450,210" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M375,90 C400,90 425,250 450,250" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M375,130 C400,130 425,90 450,90" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M375,130 C400,130 425,170 450,170" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M375,170 C400,170 425,130 450,130" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M375,170 C400,170 425,210 450,210" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M375,210 C400,210 425,90 450,90" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M375,210 C400,210 425,170 450,170" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M375,250 C400,250 425,130 450,130" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  <path d="M375,250 C400,250 425,210 450,210" stroke="#2980b9" stroke-width="0.5" opacity="0.6"/>
  
  <!-- Sample point markers -->
  <line x1="250" y1="85" x2="250" y2="255" stroke="#e74c3c" stroke-width="1" stroke-dasharray="5,5"/>
  <text x="250" y="270" font-family="Arial, sans-serif" font-size="12" text-anchor="middle">Punto di campionamento</text>
  
  <!-- Eye labels -->
  <text x="150" y="150" font-family="Arial, sans-serif" font-size="12" fill="#2980b9" text-anchor="middle">Occhi</text>
  <text x="350" y="150" font-family="Arial, sans-serif" font-size="12" fill="#2980b9" text-anchor="middle">Occhi</text>
</svg>
```

### Interpretazione del diagramma ad occhio

- Se gli occhi appaiono "aperti" e chiari (come nel diagramma sopra), il segnale è di buona qualità e facilmente decodificabile.
- Se gli occhi sono "chiusi" o indistinti, c'è troppa interferenza o distorsione nel segnale, rendendo difficile la decodifica accurata.

### Importanza del diagramma ad occhio

Il diagramma ad occhio è uno strumento diagnostico fondamentale per:
- Valutare la qualità complessiva di un sistema di trasmissione
- Individuare problemi come interferenze, rumore eccessivo, jitter
- Verificare il corretto funzionamento dei circuiti di equalizzazione
- Determinare il punto ottimale di campionamento per la decodifica del segnale

## Confronto con altre modulazioni

```svg
<svg width="500" height="400" xmlns="http://www.w3.org/2000/svg">
  <!-- Background -->
  <rect width="500" height="400" fill="#f8f9fa"/>
  
  <!-- Title -->
  <text x="250" y="30" font-family="Arial, sans-serif" font-size="20" text-anchor="middle" font-weight="bold">Confronto tra modulazioni</text>
  
  <!-- NRZ (2 levels) -->
  <text x="60" y="70" font-family="Arial, sans-serif" font-size="16" font-weight="bold">NRZ (2 livelli)</text>
  <line x1="50" y1="90" x2="450" y2="90" stroke="#ddd" stroke-width="1"/>
  <line x1="50" y1="120" x2="450" y2="120" stroke="#ddd" stroke-width="1"/>
  
  <!-- NRZ Signal -->
  <polyline points="50,90 100,90 100,120 200,120 200,90 300,90 300,120 400,120 400,90 450,90" 
           fill="none" stroke="#e74c3c" stroke-width="2"/>
  <text x="465" y="105" font-family="Arial, sans-serif" font-size="12">1 bit/simbolo</text>
  
  <!-- PAM-3 (3 levels) -->
  <text x="60" y="170" font-family="Arial, sans-serif" font-size="16" font-weight="bold">PAM-3 (3 livelli)</text>
  <line x1="50" y1="190" x2="450" y2="190" stroke="#ddd" stroke-width="1"/>
  <line x1="50" y1="220" x2="450" y2="220" stroke="#ddd" stroke-width="1"/>
  <line x1="50" y1="250" x2="450" y2="250" stroke="#ddd" stroke-width="1"/>
  
  <!-- PAM-3 Signal -->
  <polyline points="50,190 80,190 80,220 140,220 140,250 200,250 200,220 260,220 260,190 320,190 320,250 380,250 380,220 450,220" 
           fill="none" stroke="#e67e22" stroke-width="2"/>
  <text x="465" y="220" font-family="Arial, sans-serif" font-size="12">~1,58 bit/simbolo</text>
  
  <!-- PAM-5 (5 levels) -->
  <text x="60" y="300" font-family="Arial, sans-serif" font-size="16" font-weight="bold">PAM-5 (5 livelli)</text>
  <line x1="50" y1="320" x2="450" y2="320" stroke="#ddd" stroke-width="1"/>
  <line x1="50" y1="340" x2="450" y2="340" stroke="#ddd" stroke-width="1"/>
  <line x1="50" y1="360" x2="450" y2="360" stroke="#ddd" stroke-width="1"/>
  <line x1="50" y1="380" x2="450" y2="380" stroke="#ddd" stroke-width="1"/>
  <line x1="50" y1="400" x2="450" y2="400" stroke="#ddd" stroke-width="1"/>
  
  <!-- PAM-5 Signal -->
  <polyline points="50,320 70,320 70,360 100,360 100,400 130,400 130,380 160,380 160,340 190,340 190,320 220,320 220,380 250,380 250,340 280,340 280,360 310,360 310,400 340,400 340,320 370,320 370,360 400,360 400,380 430,380 430,320 450,320" 
           fill="none" stroke="#3498db" stroke-width="2"/>
  <text x="465" y="360" font-family="Arial, sans-serif" font-size="12">~2,32 bit/simbolo</text>
</svg>
```

### NRZ (Non-Return-to-Zero)

- Utilizza solo 2 livelli di tensione (tipicamente +V e -V o +V e 0)
- Trasporta 1 bit per simbolo
- È la modulazione più semplice e robusta
- Utilizzata in Ethernet 10BASE-T e molti standard di comunicazione seriale
- Richiede una larghezza di banda maggiore per velocità elevate

### Manchester (utilizzato in 10BASE-T)

- Una variante di NRZ con transizione a metà bit
- Offre auto-sincronizzazione (il clock è incorporato nel segnale)
- Richiede il doppio della larghezza di banda rispetto a NRZ semplice
- Utilizzato in Ethernet 10BASE-T e in altri standard più vecchi

### MLT-3 con codifica 4B5B (utilizzato in 100BASE-TX)

- Una tecnica che usa 3 livelli di tensione (+V, 0, -V)
- Codifica 4B5B: mappa 4 bit di dati su blocchi di 5 bit
- Richiede una velocità di 125 Mbaud per trasmettere 100 Mbps
- Il Fast Ethernet (100BASE-TX) utilizza questa combinazione

### PAM-5 (utilizzato in 1000BASE-T)

- Utilizza 5 livelli di tensione
- Trasporta circa 2,32 bit per simbolo
- È più efficiente in termini di spettro ma più sensibile al rumore
- Utilizzato in Gigabit Ethernet (1000BASE-T)

## Applicazioni oltre Gigabit Ethernet

### 10GBASE-T (10 Gigabit Ethernet)

Per raggiungere 10 Gbps su cavi in rame, viene utilizzata una versione più avanzata:

- DSQ128 (modulazione a 128 stati)
- Trasmissione su 4 coppie di cavi a 800 Msimb/sec
- Codifica LDPC (Low-Density Parity Check) per il controllo degli errori

### 2.5G/5G Ethernet

Gli standard 2.5GBASE-T e 5GBASE-T utilizzano ancora PAM-5, ma con frequenze di simbolo più elevate:

- 2.5GBASE-T: PAM-5 a 312.5 Mbaud
- 5GBASE-T: PAM-5 a 625 Mbaud

### Standards in evoluzione

Le tecnologie più recenti stanno adottando modulazioni con un numero ancora maggiore di livelli:

- **PAM-8** (8 livelli): 3 bit/simbolo, utilizzato in alcune implementazioni di 25G/50G Ethernet
- **PAM-16** (16 livelli): 4 bit/simbolo, in fase di sviluppo per future tecnologie
- **PAM-32** (32 livelli): 5 bit/simbolo, in fase di ricerca per applicazioni future

## Riferimenti e risorse

### Standard IEEE

- IEEE 802.3ab (1000BASE-T): definisce l'uso di PAM-5 per Gigabit Ethernet su cavi in rame
- IEEE 802.3an (10GBASE-T): definisce le tecniche per 10 Gigabit Ethernet su cavi in rame
- IEEE 802.3bz (2.5GBASE-T e 5GBASE-T): definisce le tecniche per 2.5 e 5 Gigabit Ethernet

### Risorse online

- [IEEE 802.3 Standards](https://www.ieee802.org/3/)
- [Ethernet Alliance](https://ethernetalliance.org/)
- [Charles Spurgeon's Ethernet Web Site](http://www.ethermanage.com/ethernet/)

### Libri consigliati

- "Gigabit Ethernet Technology and Applications" di Mark Norris
- "Digital Communications: Fundamentals and Applications" di Bernard Sklar
- "Ethernet: The Definitive Guide" di Charles E. Spurgeon e Joann Zimmerman
