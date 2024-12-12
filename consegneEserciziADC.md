>[Torna all'indice eserczi ADC](esercizi.md)


# Esercizi di Progettazione ADC - Consegne

## 1. Cella di Carico Base
Si vuole progettare un sistema di acquisizione per una cella di carico con le seguenti specifiche:
- Sensibilità: 2 mV/V
- Tensione di alimentazione: 5V
- Range di misura: 0-100 kg
- Risoluzione richiesta: 10 g
- Frequenza di campionamento minima: 100 Hz
- Rapporto segnale/rumore desiderato: 60 dB

Determinare:
1. Il guadagno necessario dello stadio di amplificazione
2. Il numero minimo di bit necessari per l'ADC
3. La frequenza di campionamento consigliata

## 2. Sensore di Temperatura
Si vuole realizzare un sistema di monitoraggio della temperatura con:
- Sensore PT100 (0.385 Ω/°C)
- Range di temperatura: -50°C a +150°C
- Risoluzione richiesta: 0.1°C
- Corrente di eccitazione: 1 mA
- Frequenza di aggiornamento minima: 1 Hz
- Accuratezza richiesta: ±0.5°C

Determinare:
1. Il range di tensione in ingresso in mV
2. Il guadagno necessario per utilizzare l'intero range dell'ADC (0-5V)
3. Il numero minimo di bit dell'ADC
4. La frequenza di campionamento minima considerando il filtraggio

## 3. Acquisizione Audio
Si deve progettare un sistema di acquisizione audio per applicazioni di analisi vocale:
- Banda del segnale: 100 Hz - 8 kHz
- Range dinamico richiesto: 90 dB
- Tensione picco-picco massima in ingresso: 2V
- Livello minimo del segnale da rilevare: 1 mV
- Impedenza di ingresso minima: 10 kΩ

Determinare:
1. La frequenza di campionamento minima secondo il teorema di Nyquist in kHz
2. Il numero di bit necessari per garantire il range dinamico richiesto
3. Il guadagno variabile necessario per gestire l'intero range di ingresso
4. La frequenza di taglio del filtro anti-aliasing in kHz

## 4. Sensore di Pressione
Si vuole progettare un sistema di acquisizione per un sensore di pressione differenziale:
- Sensibilità: 0.5 mV/kPa
- Range di misura: 0-500 kPa
- Tensione di offset: ±10 mV
- Risoluzione richiesta: 0.1 kPa
- Tempo di risposta massimo: 10 ms
- Errore massimo ammesso: 1%
- Temperatura operativa: -20°C a +60°C

Determinare:
1. Il range di tensione di ingresso in mV considerando l'offset
2. Il guadagno necessario per interfacciarsi con un ADC 0-3.3V
3. Il numero minimo di bit dell'ADC per la risoluzione richiesta
4. La frequenza di campionamento minima in Hz considerando il tempo di risposta

## 5. Sensore di Corrente
Si deve progettare un sistema di misura per un sensore di corrente ad effetto Hall:
- Sensibilità: 40 mV/A
- Range di misura: 0-50A
- Banda passante del sensore: 100 kHz
- Accuratezza richiesta: ±0.1A
- Ripple massimo ammesso: 50 mA
- Alimentazione disponibile: ±12V

Determinare:
1. Il range di tensione di uscita del sensore in V
2. Il guadagno necessario per ottimizzare il range dell'ADC
3. La frequenza di campionamento minima in kHz per rilevare il ripple
4. Il numero di bit necessari per garantire l'accuratezza richiesta
5. La frequenza di taglio del filtro anti-aliasing in kHz

## 6. Accelerometro
Si vuole realizzare un sistema di acquisizione per un accelerometro MEMS:
- Sensibilità: 250 mg/digit
- Range di misura: ±8g
- Banda passante utile: 0-1000 Hz
- Rumore RMS: 4 mg
- Tensione di alimentazione: 3.3V
- Risoluzione minima richiesta: 10 mg
- Interfaccia analogica (non digitale)

Determinare:
1. Il range di tensione di uscita da gestire in V
2. Il guadagno dello stadio di condizionamento
3. Il numero di bit necessari dell'ADC
4. La frequenza di campionamento in Hz
5. La frequenza di taglio del filtro in Hz

## 7. Sensore Piezoelettrico
Si vuole progettare un sistema di acquisizione per un sensore piezoelettrico:
- Sensibilità: 100 pC/g
- Range di misura: ±50g
- Capacità del sensore: 1000 pF
- Rumore di fondo: 0.1 pC RMS
- Banda passante richiesta: 0-5 kHz
- SNR minimo richiesto: 80 dB

Determinare:
1. La carica massima in nC e la tensione corrispondente in mV
2. Il guadagno dell'amplificatore di carica
3. Il numero minimo di bit dell'ADC per soddisfare il SNR
4. La frequenza di campionamento in kHz e la frequenza di taglio del filtro in kHz

## 8. Microfono per Misure Acustiche
Si vuole progettare un sistema per misure acustiche con microfono:
- Sensibilità: -44 dB re 1V/Pa
- Range dinamico: 120 dB SPL
- Rumore di fondo: 20 dB SPL
- Banda passante: 20 Hz - 20 kHz
- THD massima ammessa: 0.1%

Determinare:
1. La tensione di uscita per il massimo SPL in mV
2. Il guadagno necessario per l'ADC
3. Il numero di bit necessari per il range dinamico
4. La frequenza di campionamento in kHz e le specifiche di filtraggio

## 9. Strain Gauge
Si vuole realizzare un sistema di acquisizione per uno strain gauge:
- Fattore di gauge: 2.1
- Deformazione massima: ±1000 με
- Resistenza nominale: 350Ω
- Tensione di eccitazione: 5V
- Stabilità termica richiesta: 0.1 με/°C
- Range temperatura: 0-50°C

Determinare:
1. La variazione di resistenza in Ω e tensione di uscita in mV
2. Il guadagno per interfaccia con ADC 3.3V
3. Numero di bit ADC considerando la stabilità termica
4. Frequenza di campionamento in Hz e costante di tempo del filtro termico in secondi

## 10. Sensore pH
Si vuole progettare un sistema di acquisizione per un sensore di pH:
- Range: 0-14 pH
- Sensibilità nominale: -59.2 mV/pH a 25°C
- Impedenza di uscita: 100 MΩ
- Rumore Johnson: 1 μV/√Hz
- Accuratezza richiesta: ±0.01 pH

Determinare:
1. Il range di tensione di uscita totale in mV
2. Il guadagno e impedenza di ingresso minima dell'amplificatore in MΩ
3. La banda passante massima in Hz per il rumore specificato
4. Numero di bit ADC per l'accuratezza richiesta

## 11. Fotorivelatore
Si vuole progettare un sistema di acquisizione per un fotorivelatore:
- Responsività: 0.5 A/W
- Range di illuminamento: 0.1-100000 lux
- Rumore shot: 1 pA/√Hz
- NEP (Noise Equivalent Power): 10 pW/√Hz
- Banda richiesta: 0-1 kHz

Determinare:
1. La corrente di uscita per range completo in μA
2. Il guadagno del transimpedenza in kΩ
3. Numero di bit ADC per range dinamico
4. Frequenza campionamento in kHz e frequenza taglio filtro in kHz

## 12. Sensore Campo Magnetico
Si vuole progettare un sistema per misure di campo magnetico:
- Sensibilità: 25 mV/mT
- Range: ±2 mT
- Offset: ±5 mV
- Rumore: 1 μT/√Hz
- SNR minimo: 72 dB
- Banda DC-1kHz

Determinare:
1. Range tensione con offset in mV
2. Guadagno per ADC 5V
3. Numero bit per SNR richiesto
4. Frequenza campionamento in kHz e frequenza taglio filtro in kHz

## 13. Sensore Capacitivo
Si vuole progettare un sistema per un sensore capacitivo di prossimità:
- Sensibilità: 0.2 pF/mm
- Range: 0-10 mm
- Capacità parassita: 5 pF
- Risoluzione richiesta: 0.05 mm
- Rumore ambientale: 0.01 pF RMS

Determinare:
1. Variazione capacità totale in pF
2. Guadagno del circuito di interfaccia
3. Numero bit ADC per risoluzione
4. Frequenza misura in Hz

## 14. Cella di Carico Precisione
Si vuole progettare un sistema per una cella di carico ad alta precisione:
- Sensibilità: 3 mV/V
- Alimentazione: 10V
- Range: 0-500 kg
- Rumore: 0.5 μV RMS
- SNR richiesto: 96 dB
- Stabilità: 0.02% FS

Determinare:
1. Range tensione e rumore riferito in mV
2. Guadagno per ADC
3. Numero bit per specifiche
4. Frequenza campionamento in Hz

## 15. Sensore Gas
Si vuole progettare un sistema per un sensore di gas:
- Sensibilità: 50 nA/ppm
- Range: 0-1000 ppm
- Corrente di buio: 200 pA
- Rumore corrente: 10 pA RMS
- Tempo risposta: 20 ms

Determinare:
1. Range corrente totale in μA
2. Guadagno transimpedenza in MΩ
3. Numero bit ADC
4. Frequenza campionamento in Hz

## 16. Sensore Vibrazioni
Si vuole progettare un sistema per un sensore di vibrazioni:
- Sensibilità: 10 mV/g
- Range: ±50g
- Frequenza risonanza: 25 kHz
- Rumore di fondo: 0.1 mg/√Hz
- Banda utile: 0-10 kHz

Determinare:
1. Range tensione uscita in V
2. Guadagno amplificatore
3. Numero bit ADC
4. Frequenza campionamento in kHz

>[Torna all'indice eserczi ADC](esercizi.md)
