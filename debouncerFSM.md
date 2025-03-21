

>[Torna all'indice](indexpulsanti.md) >[versione in Python](nastro_py.md)

#  **DEBOUNCER**

##  **Gestione di pulsanti con debouncer**

Questo esempio include due modalità di funzionamento:

### 1. Pulsante senza memoria (momentaneo)
- Il LED1 si accende solo quando il pulsante è premuto
- Quando il pulsante viene rilasciato, il LED si spegne
- Simile a un campanello o un pulsante di un citofono

### 2. Pulsante con memoria (toggle)
- Il LED2 cambia stato ad ogni pressione del pulsante
- Se il LED è spento, premendo si accende
- Se il LED è acceso, premendo si spegne
- Simile a un interruttore della luce in casa

Entrambe le modalità utilizzano lo stesso meccanismo di debounce avanzato che:
1. Rileva immediatamente il primo cambiamento
2. Inibisce successivi rilevamenti per il periodo di debounce
3. Gestisce correttamente gli overflow di `millis()`

Questo esempio presenta l'implementazione in modo più chiaro rispetto al codice del motore stepper, facilitando la comprensione del funzionamento del meccanismo di debounce.

###  **Esigenza**

Scrivere un programma che realizzi la gestione di un nastro traportatore attraverso la lettura di tre sensori di transito (barriere ad infrarossi) e il comando di un motore. 

I sensori permangono al livello alto finchè un oggetto ingombra l'area del sensore, dopodichè vanno a livello basso. I sensori sono due all'inizio del nastro (uno per i pezzi bassi ed un'altro per quelli alti) ed uno alla fine del nastro che rileva il pezzo pronto per essere prelevato. 

Esiste un timer di volo, così chiamato perchè dura esattamente per il tempo che impiegherebbe il pezzo più lontano dall'uscita, cioè quello sulla barriera di ingresso, per arrivare sulla barriera di uscita. 

All'attivazione di un qualsiasi sensore di ingresso parte il motore e si resetta e pure si blocca il timer di volo. All'attivazione del sensore di uscita si blocca il nastro, alla sua disattivazione riparte il nastro e parte il timer di volo. Allo scadere del timer di volo si spegne il motore.

## **Stati**

- **RIPOSO**: Il nastro è vuoto e fermo, in attesa di nuovi pezzi.
- **TRASPORTO_CERTO**: C'è almeno un pezzo sul nastro, confermato dal rilevamento di un sensore all'ingresso. Il timer è disattivato perché abbiamo certezza della presenza del pezzo.
- **PEZZO_PRONTO**: Un pezzo è arrivato all'uscita ed è pronto per essere prelevato. Il nome riflette perfettamente lo stato del sistema piuttosto che l'azione esterna.
- **TRASPORTO_STIMATO**: Potrebbero esserci altri pezzi sul nastro, ma non ne abbiamo conferma diretta dai sensori. Il timer è attivo per stimare quando il nastro sarà vuoto.


## Tabella di Transizione del Nastro Trasportatore

# Tabella delle Transizioni del Debouncer

## Definizione delle Variabili
- **pin**: Pin di Arduino collegato al pulsante
- **val0**: Ultimo valore letto (per rilevare cambiamenti)
- **val**: Valore attuale letto dal pin
- **val00**: Ultimo stato stabile (dopo il debounce)
- **last**: Timestamp dell'ultimo cambiamento
- **debState**: Flag stato del debounce (true = in debounce, false = rilevamento abilitato)
- **debtime**: Tempo di debounce in millisecondi
- **chg**: Flag che indica se è stato rilevato un cambiamento valido

## Tabella delle Transizioni

| Stato Attuale | Condizione | Azione | Prossimo Stato | Output (chg) |
|---------------|------------|--------|---------------|--------------|
| IDLE<br>(debState = false) | val == val0 | val0 = val<br>val00 = val | IDLE | false |
| IDLE<br>(debState = false) | val != val0 | debState = true<br>last = millis()<br>val0 = val<br>val00 = val | DEBOUNCE | true |
| DEBOUNCE<br>(debState = true) | val != val0 | last = millis()<br>val0 = val | DEBOUNCE | false |
| DEBOUNCE<br>(debState = true) | val == val0 | val0 = val | DEBOUNCE | false |
| DEBOUNCE<br>(debState = true) | (millis() - last) > debtime | debState = false<br>val0 = val00 | IDLE | false |

## Caratteristiche della Macchina a Stati
1. **Rilevamento immediato**: Il primo cambiamento di stato viene rilevato immediatamente (chg = true)
2. **Periodo di inibizione**: Dopo il primo cambio, tutti i fronti successivi vengono ignorati per un periodo configurabile
3. **Reset del timer**: Se durante il periodo di debounce lo stato continua a cambiare, il timer viene resettato
4. **Ripristino stato di riferimento**: Al termine del debounce, lo stato di riferimento viene reimpostato all'ultimo stato stabile

## Comportamento agli eventi
- **Pulsante premuto/rilasciato**: Viene rilevato immediatamente e poi viene attivato il periodo di inibizione
- **Rimbalzi del contatto**: Vengono filtrati durante il periodo di debounce
- **Nuova pressione dopo debounce**: Viene rilevata come nuovo evento

Questa implementazione di debouncing rileva subito il primo fronte ma ignora i successivi cambiamenti per un periodo definito, combinando reattività e stabilità.

##  **Diagramma degli stati**

```mermaid
%%{init: {'theme': 'default', 'themeVariables': { 'primaryColor': '#ffffff', 'primaryTextColor': '#000000', 'primaryBorderColor': '#000000', 'lineColor': '#000000', 'secondaryColor': '#f4f4f4', 'tertiaryColor': '#ffffff' }}}%%
stateDiagram-v2
    direction LR
    
    IDLE: Stato Stabile (IDLE)
    DEBOUNCE: Stato di Debounce
    
    note right of IDLE
        debState = false
        In attesa di un cambiamento
    end note
    
    note right of DEBOUNCE
        debState = true
        Inibizione temporanea
        dei fronti successivi
    end note
    
    [*] --> IDLE
    
    IDLE --> IDLE: val == val0 / Nessun cambiamento
    IDLE --> DEBOUNCE: val != val0 / Rileva cambiamento + Avvia timer
    
    DEBOUNCE --> DEBOUNCE: (millis() - last) <= debtime / Aggiorna last se val cambia
    DEBOUNCE --> IDLE: (millis() - last) > debtime / Ripristina stato di riferimento
```

##  **Soluzione in logica "prima gli stati"**

```C++
Ecco un esempio semplificato che utilizza la tua struttura di debounce per controllare LED con e senza memoria:

```cpp
#define BUTTON1_PIN 2     // Pulsante momentaneo (senza memoria)
#define BUTTON2_PIN 3     // Pulsante toggle (con memoria)
#define LED1_PIN 8        // LED controllato senza memoria
#define LED2_PIN 9        // LED controllato con memoria
#define DEBOUNCETIME 50   // Tempo di debounce in ms

/**
 * Struttura Button per gestione avanzata del debounce
 * 
 * Rileva immediatamente cambiamenti di stato e inibisce 
 * temporaneamente il rilevamento dei fronti successivi
 */
struct Button {
  uint8_t pin;           // Pin di Arduino collegato al pulsante
  uint8_t val0;          // Ultimo valore letto
  unsigned long debtime; // Tempo di debounce in millisecondi
  uint8_t val;           // Valore attuale letto
  uint8_t val00;         // Ultimo stato stabile
  unsigned long last;    // Timestamp ultimo cambiamento
  bool debState;         // Flag per stato debounce (true = in debounce)
  
  /**
   * Rileva cambiamenti di stato con debounce
   * @return true se è stato rilevato un cambiamento stabile
   */
  bool changed() {
    bool chg;

    val = digitalRead(pin);
    if(!debState) {
      // Stato normale: attesa di un cambiamento
      if(val != val0) {
        // Primo cambiamento rilevato - attiva subito
        debState = true;
        last = millis();
        chg = true;
      } else {
        chg = false;
      }
      val0 = val;
      val00 = val;  // Memorizza ultimo stato valido
    } else {
      // In debounce: ignora cambiamenti
      if(val != val0) {
        // Resetta timer se continua a cambiare
        last = millis();
      }
      val0 = val;
      chg = false;
      
      // Gestione sicura dell'overflow di millis()
      if((unsigned long)(millis() - last) > debtime) {
        debState = false;  // Fine debounce
        val0 = val00;      // Ripristina stato di riferimento
      }
    }
        
    return chg;     
  }
};

// Inizializzazione pulsanti
Button buttonMomentary = {BUTTON1_PIN, LOW, 50};  // Pulsante senza memoria
Button buttonToggle = {BUTTON2_PIN, LOW, 50};     // Pulsante con memoria

// Stato per il pulsante con memoria
bool toggleState = false;

void setup() {
  Serial.begin(115200);
  
  // Configurazione I/O
  pinMode(BUTTON1_PIN, INPUT_PULLUP);  // Pulsanti con resistenza pullup
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(LED1_PIN, OUTPUT);           // LED 
  pinMode(LED2_PIN, OUTPUT);
  
  // Stato iniziale LED
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  
  Serial.println("Sistema di controllo LED inizializzato");
  Serial.println("- Pulsante 1: Controllo momentaneo (senza memoria)");
  Serial.println("- Pulsante 2: Controllo toggle (con memoria)");
}

void loop() {
  // 1. PULSANTE SENZA MEMORIA (momentaneo)
  // Accende il LED solo quando il pulsante è premuto
  if(buttonMomentary.changed()) {
    // Il pulsante ha cambiato stato
    if(buttonMomentary.val == LOW) {  // Con pullup, LOW = premuto
      digitalWrite(LED1_PIN, HIGH);
      Serial.println("Pulsante 1 premuto - LED 1 acceso");
    } else {
      digitalWrite(LED1_PIN, LOW);
      Serial.println("Pulsante 1 rilasciato - LED 1 spento");
    }
  }
  
  // 2. PULSANTE CON MEMORIA (toggle)
  // Cambia lo stato del LED ad ogni pressione
  if(buttonToggle.changed()) {
    if(buttonToggle.val == LOW) {  // Solo sulla pressione (non sul rilascio)
      toggleState = !toggleState;  // Inverte lo stato
      digitalWrite(LED2_PIN, toggleState);
      Serial.print("Pulsante 2 premuto - LED 2 ");
      Serial.println(toggleState ? "acceso" : "spento");
    }
  }
  
  delay(1);  // Breve pausa per stabilità
}
```
Simulazione su Esp32 con Wowki: https://wokwi.com/projects/349645533881565780

>[Torna all'indice](indexpulsanti.md) >[versione in Python](nastro_py.md)
