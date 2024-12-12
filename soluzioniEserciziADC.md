
# Soluzioni Dettagliate Esercizi ADC

## 1. Cella di Carico

### 1.1 Calcolo del guadagno necessario
1. Calcolo tensione di uscita massima del sensore:
   - Sensibilità = 2 mV/V
   - Vexc = 5V
   - Tensione uscita per kg = 2 mV/V * 5V = 10 mV
   - Tensione massima (100 kg) = 10 mV * 100 = 1000 mV = 10 mV

2. Per utilizzare un ADC con Vref = 5V:
   - Guadagno = 5V / 10mV = 500

### 1.2 Calcolo numero di bit ADC
1. Calcolo numero di livelli richiesti:
   - Range = 100 kg
   - Risoluzione = 10 g = 0.01 kg
   - Numero livelli = 100/0.01 = 10000

2. Calcolo bit necessari:
   - 2^n ≥ 10000
   - n ≥ log2(10000) ≈ 13.3
   - Minimo 14 bit

### 1.3 Frequenza di campionamento
1. Considerazioni sul filtraggio:
   - Frequenza minima = 100 Hz
   - Per un buon filtraggio digitale: fs ≥ 4 * fmin
   - fs = 400 Hz consigliata

**Risposta finale: 500, 14, 400**

## 2. Sensore di Temperatura

### 2.1 Range di tensione in ingresso
1. Calcolo resistenza PT100:
   - R(t) = R0(1 + αt)
   - R(-50°C) = 100(1 + 0.385 * -50) = 80.75 Ω
   - R(150°C) = 100(1 + 0.385 * 150) = 157.75 Ω
   - ΔR = 77 Ω

2. Con corrente 1mA:
   - V = IR
   - Vmin = 0.001 * 80.75 = 80.75 mV
   - Vmax = 0.001 * 157.75 = 157.75 mV
   - Range = 77.44 mV

### 2.2 Guadagno necessario
- Per range ADC 5V:
- G = 5V / 0.07744V = 65

# Soluzioni Dettagliate Esercizi ADC

[Primi due esercizi come già mostrati...]

## 3. Acquisizione Audio

### 3.1 Frequenza di campionamento minima
1. Teorema di Nyquist:
   - Frequenza massima segnale = 8 kHz
   - fs_min = 2 * fmax = 16 kHz

### 3.2 Numero di bit necessari
1. Calcolo per range dinamico:
   - Range dinamico = 90 dB
   - Numero bit = Range/6.02
   - n = 90/6.02 ≈ 15 bit

### 3.3 Guadagno variabile
1. Calcolo range:
   - Vmax = 2V
   - Vmin = 1mV
   - Range = 2000:1
   - Guadagno massimo = 2000

### 3.4 Filtro anti-aliasing
- Frequenza di taglio consigliata:
  - ft = 8.5 kHz (leggermente sopra banda utile)

**Risposta finale: 16, 15, 2000, 8.5**

## 4. Sensore di Pressione Differenziale

### 4.1 Range di tensione considerando offset
1. Calcolo tensione sensore:
   - Sensibilità = 0.5 mV/kPa
   - Range = 500 kPa
   - V_signal = 0.5 mV * 500 = 250 mV
2. Considerando offset:
   - Voffset = ±10 mV
   - Vtot = 250 mV + 20 mV = 270 mV

### 4.2 Guadagno per ADC 3.3V
- G = 3300 mV / 270 mV ≈ 12

### 4.3 Numero bit ADC
1. Calcolo livelli necessari:
   - Range = 500 kPa
   - Risoluzione = 0.1 kPa
   - Livelli = 5000
2. Numero bit:
   - 2^n ≥ 5000
   - n ≥ log2(5000) ≈ 12.3
   - Minimo 13 bit

### 4.4 Frequenza di campionamento
- Tempo risposta = 10 ms
- fs ≥ 2 kHz per risposta adeguata

**Risposta finale: 270, 12, 13, 2000**

## 5. Sensore di Corrente Hall

### 5.1 Range tensione uscita
1. Calcolo tensione:
   - Sensibilità = 40 mV/A
   - Range = 50A
   - Vtot = 40 mV * 50 = 2V

### 5.2 Guadagno per ADC
- Per range 5V:
- G = 5V / 2V = 2.5

### 5.3 Frequenza campionamento
1. Per rilevare ripple:
   - Banda sensore = 100 kHz
   - fs ≥ 500 kHz

### 5.4 Numero bit
1. Calcolo per accuratezza:
   - Risoluzione = 0.1A
   - Range = 50A
   - Livelli = 500
   - 2^n ≥ 500
   - n ≥ 11 bit

### 5.5 Filtro anti-aliasing
- ft = 200 kHz (sopra banda sensore)

**Risposta finale: 2, 2.5, 500, 11, 200**

## 6. Accelerometro MEMS

### 6.1 Range tensione uscita
1. Calcolo range totale:
   - Range = ±8g = 16g totali
   - Con alimentazione 3.3V:
   - Vtot = 2.3V (tipico per MEMS)

### 6.2 Guadagno condizionamento
- G = 1.5 (per adattamento livelli)

### 6.3 Numero bit ADC
1. Risoluzione richiesta:
   - 10 mg con rumore 4 mg RMS
   - SNR richiesto ≈ 68 dB
   - n ≥ 14 bit

### 6.4 Frequenza campionamento
- fs = 5000 Hz (5x banda utile)

### 6.5 Frequenza filtro
- ft = 1000 Hz (banda passante)

**Risposta finale: 2.3, 1.5, 14, 5000, 1000**

## 7. Sensore Piezoelettrico

### 7.1 Carica e tensione massima
1. Calcolo carica massima:
   - Sensibilità = 100 pC/g
   - Range = ±50g = 100g totali
   - Qmax = 100 pC/g * 50g = 5000 pC = 5 nC
2. Calcolo tensione:
   - C = 1000 pF
   - V = Q/C = 5 nC / 1000 pF = 5 mV

### 7.2 Guadagno amplificatore di carica
1. Per range ADC standard:
   - Vout desiderata ≈ 0.5V
   - G = 500 mV / 5 mV = 100

### 7.3 Numero bit per SNR
1. SNR richiesto = 80 dB
   - N = SNR/6.02
   - N = 80/6.02 ≈ 13.3
   - Minimo 14 bit

### 7.4 Frequenza campionamento e filtro
1. Per banda 5 kHz:
   - fs = 20 kHz (4x banda)
   - ft = 5 kHz (banda utile)

**Risposta finale: 5, 5, 100, 14, 20, 5**

## 8. Microfono per Misure Acustiche

### 8.1 Tensione massima uscita
1. Calcolo con sensibilità:
   - -44 dB re 1V/Pa = 6.3 mV/Pa
   - 120 dB SPL = 20 Pa
   - Vout = 6.3 mV/Pa * 20 Pa = 63.1 mV

### 8.2 Guadagno necessario
1. Per ADC 5V:
   - G = 5V / 0.0631V = 79.2

### 8.3 Numero bit per range dinamico
1. Range dinamico 120 dB:
   - N = 120/6.02 = 20 bit

### 8.4 Frequenza campionamento e filtraggio
1. Per banda audio 20 kHz:
   - fs = 48 kHz (standard audio)
   - ft = 20 kHz (Nyquist)

**Risposta finale: 63.1, 79.2, 20, 48, 20**

## 9. Strain Gauge

### 9.1 Variazione resistenza e tensione
1. Calcolo ΔR:
   - R = 350Ω
   - ε = 1000 με = 0.001
   - GF = 2.1
   - ΔR = R * GF * ε = 350 * 2.1 * 0.001 = 1.47Ω

2. Calcolo tensione ponte:
   - Vexc = 5V
   - ΔV = Vexc * (ΔR/4R) = 5 * (1.47/1400) = 5.25 mV

### 9.2 Guadagno per ADC 3.3V
- G = 3300 mV / 5.25 mV = 628

### 9.3 Numero bit per stabilità termica
1. Risoluzione necessaria:
   - Range temperatura = 50°C
   - Stabilità = 0.1 με/°C
   - Variazione totale = 5 με
   - Livelli = 1000/5 = 200
   - 2^n ≥ 200
   - Minimo 16 bit considerando rumore

### 9.4 Frequenza e costante tempo
- fs = 100 Hz (applicazione statica)
- τ = 1s (filtro termico)

**Risposta finale: 1.47, 5.25, 628, 16, 100, 1**

## 10. Sensore pH

### 10.1 Range tensione totale
1. Calcolo tensione:
   - -59.2 mV/pH * 14 pH = 828.8 mV

### 10.2 Guadagno e impedenza
1. Per ADC 5V:
   - G = 6 (conservativo per offset)
2. Impedenza ingresso:
   - Zin ≥ 10 * Zsensore = 1000 MΩ

### 10.3 Banda passante per rumore
1. Con rumore 1 μV/√Hz:
   - Accuratezza = 0.01 pH = 0.592 mV
   - BW = (0.592mV / 1μV)² = 10 Hz

### 10.4 Numero bit ADC
1. Per accuratezza 0.01 pH:
   - Livelli = 14/0.01 = 1400
   - 2^n ≥ 1400
   - Minimo 14 bit

**Risposta finale: 828.8, 6, 1000, 10, 14**

## 11. Fotorivelatore

### 11.1 Corrente di uscita range completo
1. Calcolo corrente per illuminamento:
   - Conversione lux → W/m²: ≈ 1/683 W/m² per lux
   - Potenza ottica max = (100000/683) ≈ 146 W/m²
   - Con responsività 0.5 A/W:
   - Imax = 0.5 * 146 = 50 μA (approssimato)

### 11.2 Guadagno transimpedenza
1. Per conversione I→V:
   - Vout desiderata ≈ 5V
   - Rt = 5V / 50μA = 100 kΩ

### 11.3 Numero bit ADC
1. Range dinamico:
   - Range = 100000/0.1 = 10⁶
   - Bit = log2(10⁶) ≈ 20 bit

### 11.4 Frequenza campionamento e filtro
- fs = 5 kHz (5x banda)
- ft = 1 kHz (banda richiesta)

**Risposta finale: 50, 100, 20, 5, 1**

## 12. Sensore Campo Magnetico

### 12.1 Range tensione con offset
1. Calcolo tensione:
   - 25 mV/mT * 4 mT = 100 mV
   - Con offset ±5 mV:
   - Vtot = 100 mV + 2*5 mV = 110 mV

### 12.2 Guadagno per ADC 5V
- G = 5000 mV / 110 mV = 45

### 12.3 Numero bit per SNR
1. SNR = 72 dB:
   - N = 72/6.02 ≈ 12 bit

### 12.4 Frequenza campionamento e filtro
- fs = 5 kHz (5x banda)
- ft = 1 kHz (banda utile)

**Risposta finale: 110, 45, 12, 5, 1**

## 13. Sensore Capacitivo

### 13.1 Variazione capacità totale
1. Calcolo ΔC:
   - 0.2 pF/mm * 10 mm = 2 pF

### 13.2 Guadagno circuito interfaccia
1. Per conversione C→V:
   - Vout desiderata = 5V
   - G = 500 (tipico per interfacce capacitive)

### 13.3 Numero bit ADC
1. Risoluzione richiesta:
   - 0.05 mm → 0.01 pF
   - Livelli = 2 pF / 0.01 pF = 200
   - 2^n ≥ 200
   - Minimo 8 bit

### 13.4 Frequenza misura
- fmis = 100 Hz (applicazione statica)

**Risposta finale: 2, 500, 8, 100**

## 14. Cella di Carico Precisione

### 14.1 Range tensione e rumore
1. Calcolo tensione:
   - 3 mV/V * 10V = 30 mV/V
   - Rumore = 0.0005 mV

### 14.2 Guadagno ADC
1. Per range 5V:
   - G = 5000 mV / 30 mV = 167

### 14.3 Numero bit specifiche
1. Per SNR 96 dB:
   - N = 96/6.02 = 16 bit

### 14.4 Frequenza campionamento
- fs = 1000 Hz (applicazione statica di precisione)

**Risposta finale: 30, 0.0005, 167, 16, 1000**

## 15. Sensore Gas

### 15.1 Range corrente totale
1. Calcolo corrente:
   - 50 nA/ppm * 1000 ppm = 50 μA
   - Con corrente di buio:
   - Itot = 50 μA + 0.2 μA = 50.2 μA

### 15.2 Guadagno transimpedenza
- Rt = 100 MΩ (standard per sensori gas)

### 15.3 Numero bit ADC
1. Range dinamico:
   - Livelli = 1000
   - 2^n ≥ 1000
   - Minimo 12 bit

### 15.4 Frequenza campionamento
1. Da tempo risposta:
   - tr = 20 ms
   - fs = 200 Hz (10x tempo risposta)

**Risposta finale: 50.2, 100, 12, 200**

## 16. Sensore Vibrazioni

### 16.1 Range tensione uscita
1. Calcolo tensione:
   - 10 mV/g * ±50g = ±500 mV
   - Vpp = 1V

### 16.2 Guadagno amplificatore
1. Per ADC 5V:
   - G = 5V / 1V = 5

### 16.3 Numero bit ADC
1. Range dinamico:
   - 50g/0.1mg = 5*10⁵
   - N ≥ log2(5*10⁵) ≈ 14 bit

### 16.4 Frequenza campionamento
1. Per banda 10 kHz:
   - fs = 50 kHz (5x banda)

**Risposta finale: 1, 5, 14, 50**

