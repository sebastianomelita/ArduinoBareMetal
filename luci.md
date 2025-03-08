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

| Stato attuale | Input | Stato prossimo | Output |
|---------------|-------|----------------|--------|
| SPENTO | Pressione pulsante P1 | BASSA_INTENSITA | Lampada accesa a bassa intensità, LED L1 acceso |
| BASSA_INTENSITA | Pressione pulsante P1 | MEDIA_INTENSITA | Lampada accesa a media intensità, LED L1 spento, LED L2 acceso |
| BASSA_INTENSITA | Inattività > 5 minuti | SPENTO | Lampada spenta, LED L1 spento |
| MEDIA_INTENSITA | Pressione pulsante P1 | ALTA_INTENSITA | Lampada accesa ad alta intensità, LED L2 spento, LED L3 acceso |
| MEDIA_INTENSITA | Inattività > 5 minuti | SPENTO | Lampada spenta, LED L2 spento |
| ALTA_INTENSITA | Pressione pulsante P1 | SPENTO | Lampada spenta, LED L3 spento |
| ALTA_INTENSITA | Inattività > 5 minuti | SPENTO | Lampada spenta, LED L3 spento |

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

## **Codice Arduino**

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
  
  // Stato iniziale: lampada spenta
  aggiornaUscite();
  
  Serial.println("Sistema Lampada Intelligente inizializzato");
}

void loop() {
  // Macchina a stati
  switch (statoCorrente) {
    case SPENTO:
      Serial.println("SPENTO");
      // Controllo pressione pulsante P1 (HIGH con pull-down quando premuto)
      if (digitalRead(pulsanteP1) == HIGH) {
        waitUntilInputLow(pulsanteP1, 50); // Debounce tramite waitUntilInputLow
        statoCorrente = BASSA_INTENSITA;
        timerInattivita.reset();
        timerInattivita.start();
        aggiornaUscite();
      }
      break;
      
    case BASSA_INTENSITA:
      Serial.println("BASSA_INTENSITA");
      // Controllo pressione pulsante P1
      if (digitalRead(pulsanteP1) == HIGH) {
        waitUntilInputLow(pulsanteP1, 50);
        statoCorrente = MEDIA_INTENSITA;
        timerInattivita.reset(); // Reset del timer di inattività
        aggiornaUscite();
      }
      
      // Controllo movimento (rilevato = HIGH)
      if (digitalRead(pirSensor) == HIGH) {
        timerInattivita.reset(); // Reset del timer di inattività
        Serial.println("Movimento rilevato - Timer resettato");
      }
      
      // Verifica inattività
      if (timerInattivita.get() > TEMPO_INATTIVITA) {
        Serial.println("Inattività rilevata - Spegnimento automatico");
        timerInattivita.stop();
        statoCorrente = SPENTO;
        aggiornaUscite();
      }
      break;
      
    case MEDIA_INTENSITA:
      Serial.println("MEDIA_INTENSITA");
      // Controllo pressione pulsante P1
      if (digitalRead(pulsanteP1) == HIGH) {
        waitUntilInputLow(pulsanteP1, 50);
        statoCorrente = ALTA_INTENSITA;
        timerInattivita.reset(); // Reset del timer di inattività
        aggiornaUscite();
      }
      
      // Controllo movimento
      if (digitalRead(pirSensor) == HIGH) {
        timerInattivita.reset(); // Reset del timer di inattività
        Serial.println("Movimento rilevato - Timer resettato");
      }
      
      // Verifica inattività
      if (timerInattivita.get() > TEMPO_INATTIVITA) {
        Serial.println("Inattività rilevata - Spegnimento automatico");
        timerInattivita.stop();
        statoCorrente = SPENTO;
        aggiornaUscite();
      }
      break;
      
    case ALTA_INTENSITA:
      Serial.println("ALTA_INTENSITA");
      // Controllo pressione pulsante P1
      if (digitalRead(pulsanteP1) == HIGH) {
        waitUntilInputLow(pulsanteP1, 50);
        statoCorrente = SPENTO;
        timerInattivita.stop(); // Ferma il timer di inattività
        aggiornaUscite();
      }
      
      // Controllo movimento
      if (digitalRead(pirSensor) == HIGH) {
        timerInattivita.reset(); // Reset del timer di inattività
        Serial.println("Movimento rilevato - Timer resettato");
      }
      
      // Verifica inattività
      if (timerInattivita.get() > TEMPO_INATTIVITA) {
        Serial.println("Inattività rilevata - Spegnimento automatico");
        timerInattivita.stop();
        statoCorrente = SPENTO;
        aggiornaUscite();
      }
      break;
  }
  
  delay(10); // Piccolo delay per stabilità
}

// Funzione per aggiornare le uscite in base allo stato
void aggiornaUscite() {
  // Spegne tutti i LED indicatori
  digitalWrite(ledL1, LOW);
  digitalWrite(ledL2, LOW);
  digitalWrite(ledL3, LOW);
  
  // Imposta l'intensità della lampada in base allo stato
  switch (statoCorrente) {
    case SPENTO:
      analogWrite(outputLampada, 0);
      break;
    case BASSA_INTENSITA:
      analogWrite(outputLampada, INTENSITA_BASSA);
      digitalWrite(ledL1, HIGH);  // Accende solo L1
      break;
    case MEDIA_INTENSITA:
      analogWrite(outputLampada, INTENSITA_MEDIA);
      digitalWrite(ledL2, HIGH);  // Accende solo L2
      break;
    case ALTA_INTENSITA:
      analogWrite(outputLampada, INTENSITA_ALTA);
      digitalWrite(ledL3, HIGH);  // Accende solo L3
      break;
  }
}
```

>[Torna all'indice generale](indexstatifiniti.md)
