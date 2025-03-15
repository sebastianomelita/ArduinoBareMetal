>[Torna all'indice generale](indexstatifiniti.md)

# **PULSANTE LUCI**

Scrivere un programma che realizzi una lampada intelligente
che funzioni in questo modo:
- Si accenda premendo il pulsante P1, alla cui pressione si attiva la luce a bassa intensità (stato 1) e si accende il LED indicatore L1.
- Una seconda pressione del pulsante P1 mentre la lampada è accesa a bassa intensità la porta a media intensità (stato 2), spegnendo L1 e accendendo L2.
- Una terza pressione del pulsante P1 porta la lampada ad alta intensità (stato 3), spegnendo L2 e accendendo L3.
- Una quarta pressione del pulsante P1 spegne completamente la lampada (stato 0), spegnendo tutti i LED indicatori.
- Se la lampada rileva che non ci sono movimenti per più di 5 minuti (tramite un sensore di movimento PIR), si spegne automaticamente passando allo stato 0.

Gli studenti dovranno:     
- Identificare gli stati necessari per descrivere il funzionamento del sistema.
- Definire le transizioni tra gli stati in base agli eventi rilevati dai sensori e alle azioni richieste dal sistema.
- Individuare quali siano gli ingressi (input) e le uscite (output) che il sistema deve gestire.
- Implementare la logica della FSM in un microcontrollore (Arduino, ESP32 o altro) utilizzando un linguaggio di programmazione adeguato.
- Documentare il lavoro svolto con un diagramma a stati e una tabella delle  transizioni.

## Tabella di Transizione della Lampada Intelligente

Hai ragione, l'ingresso dal sensore PIR (rilevamento di movimento) non è esplicitamente rappresentato nella tabella. Ecco la tabella aggiornata con l'ingresso PIR:

## Tabella di Transizione della Lampada Intelligente
| Stato attuale | Input | Stato prossimo | Output |
|---------------|-------|----------------|--------|
| SPENTO | Pressione pulsante P1 | BASSA_INTENSITA | Lampada accesa a bassa intensità, LED L1 acceso |
| SPENTO | Rilevamento movimento (PIR) | SPENTO | Nessun cambiamento |
| BASSA_INTENSITA | Pressione pulsante P1 | MEDIA_INTENSITA | Lampada accesa a media intensità, LED L1 spento, LED L2 acceso |
| BASSA_INTENSITA | Inattività > 5 minuti (PIR) | SPENTO | Lampada spenta, LED L1 spento |
| BASSA_INTENSITA | Rilevamento movimento (PIR) | BASSA_INTENSITA | Reset timer inattività |
| MEDIA_INTENSITA | Pressione pulsante P1 | ALTA_INTENSITA | Lampada accesa ad alta intensità, LED L2 spento, LED L3 acceso |
| MEDIA_INTENSITA | Inattività > 5 minuti (PIR) | SPENTO | Lampada spenta, LED L2 spento |
| MEDIA_INTENSITA | Rilevamento movimento (PIR) | MEDIA_INTENSITA | Reset timer inattività |
| ALTA_INTENSITA | Pressione pulsante P1 | SPENTO | Lampada spenta, LED L3 spento |
| ALTA_INTENSITA | Inattività > 5 minuti (PIR) | SPENTO | Lampada spenta, LED L3 spento |
| ALTA_INTENSITA | Rilevamento movimento (PIR) | ALTA_INTENSITA | Reset timer inattività |

Ho aggiunto sia le transizioni per il rilevamento del movimento (che resettano il timer di inattività quando la lampada è accesa) che il comportamento quando il PIR rileva movimento mentre la lampada è spenta (in quel caso non succede nulla, poiché l'accensione avviene solo tramite pressione del pulsante P1).
## Ingressi (Input)
- **Pulsante P1**: Utilizzato per cambiare l'intensità della lampada
- **Sensore PIR**: Sensore di movimento per rilevare l'inattività

## Uscite (Output)
- **Lampada**: Con tre livelli di intensità (bassa, media, alta)
- **LED L1**: Indicatore di bassa intensità 
- **LED L2**: Indicatore di media intensità
- **LED L3**: Indicatore di alta intensità

## Note
- Il sistema rileva l'inattività tramite il sensore PIR e avvia un timer di 5 minuti
- Ogni rilevamento di movimento resetta il timer di inattività
- In ogni stato di accensione, solo uno dei LED indicatori è acceso

## **Diagramma degli stati**

```mermaid
%%{init: {'theme': 'default', 'themeVariables': { 'primaryColor': '#ffffff', 'primaryTextColor': '#000000', 'primaryBorderColor': '#000000', 'lineColor': '#000000', 'secondaryColor': '#f4f4f4', 'tertiaryColor': '#ffffff' }}}%%
stateDiagram-v2
    [*] --> SPENTO
    
    SPENTO --> BASSA_INTENSITA: Pressione P1
    BASSA_INTENSITA --> MEDIA_INTENSITA: Pressione P1
    MEDIA_INTENSITA --> ALTA_INTENSITA: Pressione P1
    ALTA_INTENSITA --> SPENTO: Pressione P1
    
    BASSA_INTENSITA --> SPENTO: Inattività > 5 min
    MEDIA_INTENSITA --> SPENTO: Inattività > 5 min
    ALTA_INTENSITA --> SPENTO: Inattività > 5 min
    
    BASSA_INTENSITA --> BASSA_INTENSITA: Movimento PIR / Reset timer
    MEDIA_INTENSITA --> MEDIA_INTENSITA: Movimento PIR / Reset timer
    ALTA_INTENSITA --> ALTA_INTENSITA: Movimento PIR / Reset timer
    
    note right of SPENTO
        Lampada spenta
        Tutti i LED spenti
    end note
    
    note right of BASSA_INTENSITA
        Lampada a bassa intensità
        LED L1 acceso
    end note
    
    note right of MEDIA_INTENSITA
        Lampada a media intensità
        LED L2 acceso
    end note
    
    note right of ALTA_INTENSITA
        Lampada ad alta intensità
        LED L3 acceso
    end note
```

## **Codice Arduino "prima gli stati e poi gli ingressi"**

Per lo sviluppo fare riferimento alla metodologia esposta in: [priorità-statoingresso](statifinitisviluppo.md#priorità-statoingresso)

```C++
//##### urutils.h #####
void waitUntilInputLow(int btn, unsigned t)
{
   do{
     delay(t);
   }while(digitalRead(btn)!=LOW);
}

struct DiffTimer
{
	unsigned long elapsed, last;
	bool timerstate=false;
	byte state = 0;
	byte count = 0;
	void reset(){
		elapsed = 0;
		last = millis();
	}
	void toggle(){
		if(timerstate){
    	    stop();
		}else{
			start();
		}	
	}
	void stop(){
		if(timerstate){
			timerstate = false;
    	    elapsed += millis() - last;
		}	
	}
	void start(){
		if(!timerstate){
			timerstate = true;
			last = millis();
		}
	}
	unsigned long get(){
		if(timerstate){
			return millis() - last + elapsed;
		}
		return elapsed;
	}
	void set(unsigned long e){
		reset();
		elapsed = e;
	}
};
//##### urutils.h #####

// Definizione dei pin
const int pulsanteP1 = 2;     // Pin per il pulsante P1
const int pirSensor = 3;      // Pin per il sensore PIR
const int ledL1 = 4;          // LED indicatore bassa intensità
const int ledL2 = 5;          // LED indicatore media intensità
const int ledL3 = 6;          // LED indicatore alta intensità
const int outputLampada = 9;  // Pin PWM per controllare l'intensità della lampada

// Valori di intensità della lampada
const int OFF = 0;    // 0% di 255
const int INTENSITA_BASSA = 85;    // ~33% di 255
const int INTENSITA_MEDIA = 170;   // ~66% di 255
const int INTENSITA_ALTA = 255;    // 100% di 255

// Timer per l'inattività
DiffTimer timerInattivita;
const unsigned long TEMPO_INATTIVITA = 300000; // 5 minuti in millisecondi

// Definizione stati
enum Stati {
  SPENTO = 0,
  BASSA_INTENSITA = 1,
  MEDIA_INTENSITA = 2,
  ALTA_INTENSITA = 3
};

// Variabile di stato
uint8_t statoCorrente;

void updateOutputs(uint8_t l1, uint8_t l2, uint8_t l3, uint8_t al){
    digitalWrite(ledL1, LOW);
    digitalWrite(ledL2, LOW);
    digitalWrite(ledL3, LOW);
    analogWrite(outputLampada, al);   
}

void setup() {
  // Inizializzazione pin
  pinMode(pulsanteP1, INPUT);    // Pulsante con resistenza di pull-down esterna
  pinMode(pirSensor, INPUT);     // Sensore PIR
  pinMode(ledL1, OUTPUT);
  pinMode(ledL2, OUTPUT);
  pinMode(ledL3, OUTPUT);
  pinMode(outputLampada, OUTPUT);
  
  // Inizializzazione stato
  statoCorrente = SPENTO;  
  // Inizializzazione seriale per debug
  Serial.begin(115200);  
  // Spegni tutti i LED e la lampada inizialmente
  updateOutputs(LOW, LOW, LOW, SPENTO);  
  Serial.println("Sistema Lampada Intelligente inizializzato");
}

void loop() {
  // Macchina a stati
  switch (statoCorrente) {
    case SPENTO:   
      // Stato SPENTO: tutti i LED e lampada spenti
      // Controllo pressione pulsante P1 (HIGH con pull-down quando premuto)
      if (digitalRead(pulsanteP1) == HIGH) {
        waitUntilInputLow(pulsanteP1, 50); // Debounce tramite waitUntilInputLow
        statoCorrente = BASSA_INTENSITA;
	Serial.println("Stato: BASSA_INTENSITA");
	// impostazione valore uscite
	updateOutputs(LOW, HIGH, LOW, INTENSITA_BASSA);
	// inizializzazione stato successivo
        timerInattivita.reset();
        timerInattivita.start();
      }
      break;
      
    case BASSA_INTENSITA:
      // Stato BASSA_INTENSITA: LED L1 acceso, altri spenti, lampada a bassa intensità     
      Serial.println("Stato: BASSA_INTENSITA");
       
      if (digitalRead(pulsanteP1) == HIGH) {// Controllo pressione pulsante P1
        waitUntilInputLow(pulsanteP1, 50);
        statoCorrente = MEDIA_INTENSITA;
	Serial.println("Stato: MEDIA_INTENSITA");
	// impostazione valore uscite
	updateOutputs(LOW, HIGH, LOW, INTENSITA_MEDIA);
	// inizializzazione stato successivo
	timerInattivita.reset(); // Reset del timer di inattività
      }else if (digitalRead(pirSensor) == HIGH) {// Controllo movimento (rilevato = HIGH)
	// inizializzazione stato successivo
        timerInattivita.reset(); // Reset del timer di inattività
        Serial.println("Movimento rilevato - Timer resettato");
      }else if( timerInattivita.get() > TEMPO_INATTIVITA) {// Verifica inattività
        Serial.println("Inattività rilevata - Spegnimento automatico");
        statoCorrente = SPENTO;
	Serial.println("Stato: SPENTO");
	// impostazione valore uscite
	updateOutputs(LOW, LOW, LOW, OFF);
	// inizializzazione stato successivo
	timerInattivita.stop();
      }
      break;
      
    case MEDIA_INTENSITA:
      // Stato MEDIA_INTENSITA: LED L2 acceso, altri spenti, lampada a media intensità
      // Controllo pressione pulsante P1
      if(digitalRead(pulsanteP1) == HIGH) {
        waitUntilInputLow(pulsanteP1, 50);
        statoCorrente = ALTA_INTENSITA;
	Serial.println("Stato: ALTA_INTENSITA");
	// impostazione valore uscite
	updateOutputs(LOW, LOW, HIGH, INTENSITA_ALTA);
	// inizializzazione stato successivo
        timerInattivita.reset(); // Reset del timer di inattività
      }else if (digitalRead(pirSensor) == HIGH) {// Controllo movimento
	// inizializzazione stato successivo
        timerInattivita.reset(); // Reset del timer di inattività
        Serial.println("Movimento rilevato - Timer resettato");
      }else if (timerInattivita.get() > TEMPO_INATTIVITA) {// Verifica inattività
        Serial.println("Inattività rilevata - Spegnimento automatico");
        statoCorrente = SPENTO;
	Serial.println("Stato: SPENTO");
	// impostazione valore uscite
	updateOutputs(LOW, LOW, LOW, OFF);
	// inizializzazione stato successivo
	timerInattivita.reset();
      }
      break;
      
    case ALTA_INTENSITA:
      // Stato ALTA_INTENSITA: LED L3 acceso, altri spenti, lampada ad alta intensità           
      // Controllo pressione pulsante P1
      if(digitalRead(pulsanteP1) == HIGH) {
        waitUntilInputLow(pulsanteP1, 50);
        statoCorrente = SPENTO;
	// impostazione valore uscite
	updateOutputs(LOW, LOW, HIGH, INTENSITA_ALTA);
	// inizializzazione stato successivo
        timerInattivita.stop(); // Ferma il timer di inattività
      }else if(digitalRead(pirSensor) == HIGH) { // controllo movimento
	// inizializzazione stato successivo
        timerInattivita.reset(); // Reset del timer di inattività
        Serial.println("Movimento rilevato - Timer resettato");
      }else if(timerInattivita.get() > TEMPO_INATTIVITA) {// Verifica inattività
        Serial.println("Inattività rilevata - Spegnimento automatico");
        statoCorrente = SPENTO;
	Serial.println("Stato: SPENTO");
	// impostazione valore uscite
	updateOutputs(LOW, LOW, LOW, OFF);
	// inizializzazione stato successivo
	timerInattivita.stop();
      }
      break;
  }
  delay(10); // Piccolo delay per stabilità
}
```

Simulazione con Arduino su Tinkercad: https://www.tinkercad.com/things/ixDZp3lQSwo-lampada-intelligente

## **Codice Arduino "prima gli ingressi e poi gli stati"**

Per lo sviluppo fare riferimento alla metodologia esposta in: [priorità-statoingresso](statifinitisviluppo.md#priorità-statoingresso)

```C++
//##### urutils.h #####
void waitUntilInputLow(int btn, unsigned t)
{
   do{
     delay(t);
   }while(digitalRead(btn)!=LOW);
}

struct DiffTimer
{
	unsigned long elapsed, last;
	bool timerstate=false;
	byte state = 0;
	byte count = 0;
	void reset(){
		elapsed = 0;
		last = millis();
	}
	void toggle(){
		if(timerstate){
    	    stop();
		}else{
			start();
		}	
	}
	void stop(){
		if(timerstate){
			timerstate = false;
    	    elapsed += millis() - last;
		}	
	}
	void start(){
		if(!timerstate){
			timerstate = true;
			last = millis();
		}
	}
	unsigned long get(){
		if(timerstate){
			return millis() - last + elapsed;
		}
		return elapsed;
	}
	void set(unsigned long e){
		reset();
		elapsed = e;
	}
};
//##### urutils.h #####

// Definizione dei pin
const int pulsanteP1 = 2;     // Pin per il pulsante P1
const int pirSensor = 3;      // Pin per il sensore PIR
const int ledL1 = 4;          // LED indicatore bassa intensità
const int ledL2 = 5;          // LED indicatore media intensità
const int ledL3 = 6;          // LED indicatore alta intensità
const int outputLampada = 9;  // Pin PWM per controllare l'intensità della lampada

// Valori di intensità della lampada
const int OFF = 0;    // 0% di 255
const int INTENSITA_BASSA = 85;    // ~33% di 255
const int INTENSITA_MEDIA = 170;   // ~66% di 255
const int INTENSITA_ALTA = 255;    // 100% di 255

// Timer per l'inattività
DiffTimer timerInattivita;
const unsigned long TEMPO_INATTIVITA = 300000; // 5 minuti in millisecondi

// Definizione stati
enum Stati {
  SPENTO = 0,
  BASSA_INTENSITA = 1,
  MEDIA_INTENSITA = 2,
  ALTA_INTENSITA = 3
};

// Variabile di stato
uint8_t statoCorrente;

void updateOutputs(uint8_t l1, uint8_t l2, uint8_t l3, uint8_t al){
    digitalWrite(ledL1, LOW);
    digitalWrite(ledL2, LOW);
    digitalWrite(ledL3, LOW);
    analogWrite(outputLampada, al);   
}

void setup() {
  // Inizializzazione pin
  pinMode(pulsanteP1, INPUT);    // Pulsante con resistenza di pull-down esterna
  pinMode(pirSensor, INPUT);     // Sensore PIR
  pinMode(ledL1, OUTPUT);
  pinMode(ledL2, OUTPUT);
  pinMode(ledL3, OUTPUT);
  pinMode(outputLampada, OUTPUT);
  
  // Inizializzazione stato
  statoCorrente = SPENTO;
  
  // Inizializzazione seriale per debug
  Serial.begin(115200);
  
  // Spegni tutti i LED e la lampada inizialmente
  updateOutputs(LOW, LOW, LOW, SPENTO);
  
  Serial.println("Sistema Lampada Intelligente inizializzato");
}

void loop() {
  // Macchina a stati con priorità agli ingressi
  
  // INGRESSO 1: Pulsante P1 premuto
  if (digitalRead(pulsanteP1) == HIGH) {
    waitUntilInputLow(pulsanteP1, 50); // Debounce tramite waitUntilInputLow
    // Gestione pressione del pulsante in base allo stato attuale
    switch (statoCorrente) {
      case SPENTO:
        Serial.println("Passaggio a BASSA_INTENSITA");
        statoCorrente = BASSA_INTENSITA;
        // impostazione valore uscite
        updateOutputs(HIGH, LOW, LOW, SPENTO);
        analogWrite(outputLampada, INTENSITA_BASSA);
	// inizializzazione stato successivo
        timerInattivita.reset();
        timerInattivita.start();// Avvio timer inattività
        break;
        
      case BASSA_INTENSITA:
        Serial.println("Passaggio a MEDIA_INTENSITA");
        statoCorrente = MEDIA_INTENSITA;
        // impostazione valore uscite
	updateOutputs(LOW, HIGH, LOW, INTENSITA_MEDIA);
        // inizializzazione stato successivo
        timerInattivita.reset();// Reset timer inattività
        break;
        
      case MEDIA_INTENSITA:
        Serial.println("Passaggio a ALTA_INTENSITA");
        statoCorrente = ALTA_INTENSITA;
        // impostazione valore uscite
	updateOutputs(LOW, LOW, HIGH, INTENSITA_ALTA);
	// inizializzazione stato successivo
        timerInattivita.reset();// Reset timer inattività
        break;
        
      case ALTA_INTENSITA:
        Serial.println("Passaggio a SPENTO");
        statoCorrente = SPENTO;
        // impostazione valore uscite
        updateOutputs(LOW, LOW, LOW, OFF);
	// inizializzazione stato successivo
        timerInattivita.stop();// Stop timer inattività
        break;
    }
  }
  
  // INGRESSO 2: Sensore di movimento PIR
  else if (digitalRead(pirSensor) == HIGH) {
    // Gestione rilevamento movimento in base allo stato attuale
    switch (statoCorrente) {
      case SPENTO:
        // Non fa nulla quando è spento
        break;
        
      case BASSA_INTENSITA:
      case MEDIA_INTENSITA:
      case ALTA_INTENSITA:
        // inizializzazione stato successivo
        timerInattivita.reset();// Reset del timer di inattività per tutti gli stati accesi
        Serial.println("Movimento rilevato - Timer resettato");
        break;
    }
  }
  
  // INGRESSO 3: Timer di inattività scaduto
  else if (timerInattivita.get() > TEMPO_INATTIVITA) {
    // Gestione timeout inattività in base allo stato attuale
    switch (statoCorrente) {
      case SPENTO:
        // Già spento, non fa nulla
        break;
        
      case BASSA_INTENSITA:
      case MEDIA_INTENSITA:
      case ALTA_INTENSITA:
        // Spegnimento automatico per tutti gli stati accesi
        Serial.println("Inattività rilevata - Spegnimento automatico");
        statoCorrente = SPENTO;
        // impostazione valore uscite
        updateOutputs(LOW, LOW, LOW, OFF);
        // inizializzazione stato successivo
        timerInattivita.stop();// Stop timer inattività
        break;
    }
  }
  delay(10); // Piccolo delay per stabilità
}
```

## **Codice Arduino "prima gli ingressi e poi gli stati" rappresentati come contatori**

Per lo sviluppo fare riferimento alla metodologia esposta in: [priorità-statoingresso](statifinitisviluppo.md#priorità-statoingresso)

```C++
//##### urutils.h #####
void waitUntilInputLow(int btn, unsigned t)
{
   do{
     delay(t);
   }while(digitalRead(btn)!=LOW);
}

struct DiffTimer
{
	unsigned long elapsed, last;
	bool timerstate=false;
	byte state = 0;
	byte count = 0;
	void reset(){
		elapsed = 0;
		last = millis();
	}
	void toggle(){
		if(timerstate){
    	    stop();
		}else{
			start();
		}	
	}
	void stop(){
		if(timerstate){
			timerstate = false;
    	    elapsed += millis() - last;
		}	
	}
	void start(){
		if(!timerstate){
			timerstate = true;
			last = millis();
		}
	}
	unsigned long get(){
		if(timerstate){
			return millis() - last + elapsed;
		}
		return elapsed;
	}
	void set(unsigned long e){
		reset();
		elapsed = e;
	}
};
//##### urutils.h #####

// Definizione dei pin
const int pulsanteP1 = 2;     // Pin per il pulsante P1
const int pirSensor = 3;      // Pin per il sensore PIR
const int outputLampada = 9;  // Pin PWM per controllare l'intensità della lampada

// Array dei pin LED - ora possono essere non consecutivi
const int ledPins[] = {4, 5, 6};  // ledL1, ledL2, ledL3 - possono essere qualsiasi pin
const int NUM_LED = sizeof(ledPins) / sizeof(ledPins[0]);

// Valori di intensità della lampada
const int INTENSITA[] = {0, 85, 170, 255};  // Valori di intensità per ogni stato
const int NUM_STATI = sizeof(INTENSITA) / sizeof(INTENSITA[0]);  // Conta automaticamente gli stati

// Timer per l'inattività
DiffTimer timerInattivita;
const unsigned long TEMPO_INATTIVITA = 300000; // 5 minuti in millisecondi

// Variabile di stato come contatore
uint8_t statoCorrente = 0;  // 0=SPENTO, 1=BASSA, 2=MEDIA, 3=ALTA

// Nomi degli stati per i messaggi di debug
const char* NOMI_STATI[] = {"SPENTO", "BASSA_INTENSITA", "MEDIA_INTENSITA", "ALTA_INTENSITA"};

void setup() {
  // Inizializzazione pin
  pinMode(pulsanteP1, INPUT);    // Pulsante con resistenza di pull-down esterna
  pinMode(pirSensor, INPUT);     // Sensore PIR
  pinMode(outputLampada, OUTPUT);
  
  // Inizializzazione dei pin LED
  for (int i = 0; i < NUM_LED; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);  // Tutti i LED inizialmente spenti
  }
  
  // Inizializzazione seriale per debug
  Serial.begin(115200);
  
  // Imposta l'output iniziale
  analogWrite(outputLampada, INTENSITA[statoCorrente]);
  
  Serial.println("Sistema Lampada Intelligente inizializzato");
}

void loop() {
  // Macchina a stati con priorità agli ingressi
  
  // INGRESSO 1: Pulsante P1 premuto
  if (digitalRead(pulsanteP1) == HIGH) {
    waitUntilInputLow(pulsanteP1, 50); // Debounce tramite waitUntilInputLow

    // inizializzazione stato successivo
    // Se era spento, avvia il timer di inattività
    if (statoCorrente == 0) {
      timerInattivita.reset();
      timerInattivita.start();
    }
    // Se passa a spento, ferma il timer
    else if (statoCorrente == NUM_STATI - 1) {
      timerInattivita.stop();
    } 
    // Altrimenti resetta il timer in corso
    else {
      timerInattivita.reset();
    }
    
    // Incrementa lo stato in modo ciclico
    statoCorrente = (statoCorrente + 1) % NUM_STATI;
    Serial.print("Passaggio a stato: ");
    Serial.println(NOMI_STATI[statoCorrente]);
    
    // Aggiorna output in base al nuovo stato
    aggiornaOutput();
  }
  
  // INGRESSO 2: Sensore di movimento PIR
  else if (digitalRead(pirSensor) == HIGH) {
    // Reset del timer di inattività solo se la lampada è accesa
    if (statoCorrente > 0) {
      timerInattivita.reset();
      Serial.println("Movimento rilevato - Timer resettato");
    }
  }
  
  // INGRESSO 3: Timer di inattività scaduto
  else if (statoCorrente > 0 && timerInattivita.get() > TEMPO_INATTIVITA) {
    // Spegnimento automatico
    Serial.println("Inattività rilevata - Spegnimento automatico");
    statoCorrente = 0;  // Torna allo stato SPENTO
    // inizializzazione stato successivo
    timerInattivita.stop();
    
    // Aggiorna output
    aggiornaOutput();
  }

  delay(10); // Piccolo delay per stabilità
}

// Funzione per aggiornare gli output in base allo stato corrente
void aggiornaOutput() {
  // Imposta l'intensità della lampada
  analogWrite(outputLampada, INTENSITA[statoCorrente]);
  
  // Aggiorna i LED
  for (int i = 0; i < NUM_LED; i++) {
    // Accende solo il LED corrispondente allo stato corrente (se non è SPENTO)
    // statoCorrente 1 -> LED 0, statoCorrente 2 -> LED 1, statoCorrente 3 -> LED 2
    digitalWrite(ledPins[i], (statoCorrente > 0 && i == statoCorrente - 1) ? HIGH : LOW);
  }
}
```

>[Torna all'indice generale](indexstatifiniti.md)
