>[Torna all'indice generale](indexstatifiniti.md)

# **AUTOLAVAGGIO**

Un'azienda di autolavaggi ha la necessità di automatizzare il processo di lavaggio dei veicoli per migliorare l'efficienza operativa e garantire un servizio uniforme. Il sistema deve gestire automaticamente le varie fasi del lavaggio, assicurandosi che il veicolo sia correttamente posizionato e che ogni fase venga completata prima di passare alla successiva.

Il sistema deve rilevare quando un veicolo entra nell'area di lavaggio e avviarsi solo quando il veicolo è fermo nella posizione corretta. Il ciclo di lavaggio comprende diverse fasi, tra cui prelavaggio di un minuto, lavaggio principale di 5 minuti e asciugatura di un minuto. Al termine del processo, il sistema deve notificare al conducente quando è possibile uscire.

Gli studenti dovranno:
- Identificare gli stati necessari per descrivere il funzionamento del sistema.
- Definire le transizioni tra gli stati in base agli eventi rilevati dai sensori e alle azioni richieste dal sistema.
- Individuare quali siano gli ingressi (input) e le uscite (output) che il sistema deve gestire.
- Implementare la logica della FSM in un microcontrollore (Arduino, ESP32 o altro) utilizzando un linguaggio di programmazione adeguato.
- Documentare il lavoro svolto con un diagramma a stati e una breve relazione che descriva il funzionamento del sistema e le scelte progettuali adottate.

## Tabella di Transizione del Sistema di Autolavaggio con Sensori di Transito

| Stato attuale | Input | Stato prossimo | Output |
|---------------|-------|----------------|--------|
| LIBERO | Fronte salita sensore A | INGRESSO | LED giallo ingresso, barriera bloccata |
| INGRESSO | Fronte discesa sensore A AND Sensore C attivo | IN_ATTESA | LED giallo lampeggiante, indicazioni di posizionamento |
| INGRESSO | Fronte discesa sensore A AND Sensore C inattivo | LIBERO | LED verde sistema pronto, reset sistema |
| INGRESSO | Timeout (>30s) | ALLARME | LED rosso lampeggiante, segnalatore acustico |
| IN_ATTESA | Sensore C attivo | POSIZIONATO | LED verde posizionamento, indicazioni di avvio |
| IN_ATTESA | Timeout (>60s) | ALLARME | LED rosso lampeggiante, segnalatore acustico |
| POSIZIONATO | Conferma avvio (automatica o manuale) | PRELAVAGGIO | Attivazione spruzzatori, LED fase prelavaggio |
| PRELAVAGGIO | Timer scaduto (1 min) | LAVAGGIO | Disattivazione spruzzatori, attivazione spazzole e detergente, LED fase lavaggio |
| LAVAGGIO | Timer scaduto (5 min) | ASCIUGATURA | Disattivazione spazzole e detergente, attivazione ventole, LED fase asciugatura |
| ASCIUGATURA | Timer scaduto (1 min) | COMPLETAMENTO | Disattivazione ventole, LED verde completamento, messaggio "Procedere all'uscita" |
| COMPLETAMENTO | Fronte salita sensore B | USCITA | LED giallo uscita, barriera uscita aperta |
| USCITA | Fronte discesa sensore B AND Sensore C inattivo | LIBERO | LED verde sistema pronto, reset sistema |
| USCITA | Fronte discesa sensore B AND Sensore C attivo | COMPLETAMENTO | LED verde completamento, messaggio "Procedere all'uscita" |
| USCITA | Timeout (>30s) | ALLARME | LED rosso lampeggiante, segnalatore acustico |
| ALLARME | Reset manuale | LIBERO | LED verde sistema pronto, reset sistema |

## **Diagramma degli stati**

```mermaid
%%{init: {'theme': 'default', 'themeVariables': { 'primaryColor': '#ffffff', 'primaryTextColor': '#000000', 'primaryBorderColor': '#000000', 'lineColor': '#000000', 'secondaryColor': '#f4f4f4', 'tertiaryColor': '#ffffff' }}}%%
stateDiagram-v2

    [*] --> Libero
    
    Libero --> Ingresso: Fronte salita sensore A
    Ingresso --> InAttesa: Fronte discesa sensore A AND Sensore C attivo
    Ingresso --> Libero: Fronte discesa sensore A AND Sensore C inattivo
    Ingresso --> Allarme: Timeout (>30s)
    
    InAttesa --> Posizionato: Sensore C attivo
    InAttesa --> Allarme: Timeout (>60s)
    
    Posizionato --> Prelavaggio: Sequenza avvio
    
    Prelavaggio --> Lavaggio: Timer (1 min)
    Lavaggio --> Asciugatura: Timer (5 min)
    Asciugatura --> Completamento: Timer (1 min)
    
    Completamento --> Uscita: Fronte salita sensore B
    Uscita --> Libero: Fronte discesa sensore B AND Sensore C inattivo
    Uscita --> Completamento: Fronte discesa sensore B AND Sensore C attivo
    Uscita --> Allarme: Timeout (>30s)
    
    Allarme --> Libero: Reset manuale
    
    note right of Libero
        Area lavaggio vuota
        Sensori A, B, C inattivi
    end note
    
    note right of Ingresso
        Veicolo a cavallo ingresso
        Sensore A attivo
        Si verifica C per confermare
        direzione del veicolo
    end note
    
    note right of InAttesa
        Veicolo entrato ma non
        correttamente posizionato
        Sensore C inattivo
    end note
    
    note right of Posizionato
        Veicolo correttamente 
        posizionato per lavaggio
        Sensore C attivo
    end note
    
    note right of Uscita
        Veicolo a cavallo uscita
        Sensore B attivo
        Si verifica C per confermare
        direzione del veicolo
    end note
    
    note right of Allarme
        Veicolo bloccato o
        anomalia rilevata
        Intervento operatore necessario
    end note
```

## **Codice Arduino**

```C++
// Variabile di stato
uint8_t statoCorrente;//##### urutils.h #####
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

// Definizione pin per sensori e attuatori
int sensore_A = 2;       // Sensore di ingresso (fronte)
int sensore_B = 3;       // Sensore di uscita (fronte)
int sensore_C = 4;       // Sensore interno (livello)

// LED indicatori
int led_verde = 5;       // Sistema libero/pronto
int led_giallo = 6;      // Ingresso/uscita/attesa
int led_fase = 7;        // Indicatore fase lavaggio
int led_rosso = 8;       // Allarme

// Attuatori
int spruzzatori = 9;     // Spruzzatori prelavaggio
int spazzole = 10;       // Spazzole lavaggio
int ventole = 11;        // Ventole asciugatura
int buzzer = 12;         // Segnalatore acustico

// Timer per le diverse fasi
DiffTimer timerProcesso;
DiffTimer timerTimeout;

// Definizione stati del sistema
enum Stati {
  LIBERO,
  INGRESSO,
  IN_ATTESA,
  POSIZIONATO,
  PRELAVAGGIO,
  LAVAGGIO,
  ASCIUGATURA,
  COMPLETAMENTO,
  USCITA,
  ALLARME
};

// Struttura per gestire i fronti dei sensori
struct SensoreFronte {
  int pin;                // Pin del sensore
  bool livelloAttuale;    // Livello attuale
  bool livelloPrecedente; // Livello precedente
  bool fronteSalita;      // Fronte di salita rilevato
  bool fronteDiscesa;     // Fronte di discesa rilevato
  
  // Inizializzazione
  void init(int _pin) {
    pin = _pin;
    livelloPrecedente = LOW;
    fronteSalita = false;
    fronteDiscesa = false;
  }
  
  // Aggiornamento e rilevazione fronti
  void aggiorna() {
    livelloAttuale = digitalRead(pin);
    fronteSalita = livelloAttuale && !livelloPrecedente;
    fronteDiscesa = !livelloAttuale && livelloPrecedente;
    livelloPrecedente = livelloAttuale;
  }
};

// Definizione sensori
SensoreFronte sensoreA;
SensoreFronte sensoreB;

void setup() {
  // Inizializzazione I/O
  pinMode(sensore_A, INPUT);
  pinMode(sensore_B, INPUT);
  pinMode(sensore_C, INPUT);
  
  pinMode(led_verde, OUTPUT);
  pinMode(led_giallo, OUTPUT);
  pinMode(led_fase, OUTPUT);
  pinMode(led_rosso, OUTPUT);
  
  pinMode(spruzzatori, OUTPUT);
  pinMode(spazzole, OUTPUT);
  pinMode(ventole, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  // Inizializzazione sensori
  sensoreA.init(sensore_A);
  sensoreB.init(sensore_B);
  
  // Inizializzazione stato
  statoCorrente = LIBERO;
  
  // Inizializzazione seriale per debug
  Serial.begin(115200);
  
  // Stato iniziale: sistema libero
  digitalWrite(led_verde, HIGH);
  Serial.println("Sistema Autolavaggio inizializzato - STATO: LIBERO");
}

void loop() {
  // Lettura sensori e aggiornamento fronti
  sensoreA.aggiorna();
  sensoreB.aggiorna();
  bool livelloC = digitalRead(sensore_C);

  // Macchina a stati
  switch (statoCorrente) {
    case LIBERO:
      // Sistema in attesa di veicoli
      if (sensoreA.fronteSalita) {
        // Veicolo rilevato in ingresso
        Serial.println("STATO: INGRESSO - Veicolo a cavallo dell'ingresso");
        digitalWrite(led_verde, LOW);
        digitalWrite(led_giallo, HIGH);
        timerTimeout.reset();
        timerTimeout.start();
        statoCorrente = INGRESSO;
      }
      break;
      
    case INGRESSO:
      // Veicolo a cavallo dell'ingresso
      if (sensoreA.fronteDiscesa && livelloC) {
        // Veicolo entrato completamente
        Serial.println("STATO: IN_ATTESA - Veicolo entrato, in attesa di posizionamento");
        // LED giallo lampeggiante (gestito esternamente con blink())
        timerTimeout.reset();
        statoCorrente = IN_ATTESA;
      } 
      else if (sensoreA.fronteDiscesa && !livelloC) {
        // Veicolo tornato indietro
        Serial.println("STATO: LIBERO - Veicolo tornato indietro");
        digitalWrite(led_giallo, LOW);
        digitalWrite(led_verde, HIGH);
        timerTimeout.stop();
        statoCorrente = LIBERO;
      }
      else if (timerTimeout.get() > 30000) {
        // Timeout veicolo bloccato in ingresso
        Serial.println("STATO: ALLARME - Veicolo bloccato in ingresso");
        digitalWrite(led_giallo, LOW);
        digitalWrite(led_rosso, HIGH);
        digitalWrite(buzzer, HIGH);
        statoCorrente = ALLARME;
      }
      break;
      
    case IN_ATTESA:
      // Veicolo entrato ma non posizionato
      if (livelloC) {
        // Veicolo posizionato correttamente
        Serial.println("STATO: POSIZIONATO - Veicolo in posizione corretta");
        digitalWrite(led_giallo, LOW);
        digitalWrite(led_verde, HIGH);
        timerTimeout.stop();
        statoCorrente = POSIZIONATO;
      }
      else if (timerTimeout.get() > 60000) {
        // Timeout veicolo non posizionato
        Serial.println("STATO: ALLARME - Timeout posizionamento");
        digitalWrite(led_giallo, LOW);
        digitalWrite(led_rosso, HIGH);
        digitalWrite(buzzer, HIGH);
        statoCorrente = ALLARME;
      }
      break;
      
    case POSIZIONATO:
      // Veicolo pronto per iniziare il processo
      // Avvio automatico dopo breve pausa
      delay(2000);
      Serial.println("STATO: PRELAVAGGIO - Inizio ciclo di lavaggio");
      digitalWrite(led_verde, LOW);
      digitalWrite(led_fase, HIGH);
      digitalWrite(spruzzatori, HIGH);
      timerProcesso.reset();
      timerProcesso.start();
      statoCorrente = PRELAVAGGIO;
      break;
      
    case PRELAVAGGIO:
      // Fase di prelavaggio
      if (timerProcesso.get() > 60000) { // 1 minuto
        Serial.println("STATO: LAVAGGIO - Passaggio a fase di lavaggio principale");
        digitalWrite(spruzzatori, LOW);
        digitalWrite(spazzole, HIGH);
        timerProcesso.reset();
        statoCorrente = LAVAGGIO;
      }
      break;
      
    case LAVAGGIO:
      // Fase di lavaggio principale
      if (timerProcesso.get() > 300000) { // 5 minuti
        Serial.println("STATO: ASCIUGATURA - Passaggio a fase di asciugatura");
        digitalWrite(spazzole, LOW);
        digitalWrite(ventole, HIGH);
        timerProcesso.reset();
        statoCorrente = ASCIUGATURA;
      }
      break;
      
    case ASCIUGATURA:
      // Fase di asciugatura
      if (timerProcesso.get() > 60000) { // 1 minuto
        Serial.println("STATO: COMPLETAMENTO - Ciclo di lavaggio completato");
        digitalWrite(ventole, LOW);
        digitalWrite(led_fase, LOW);
        digitalWrite(led_verde, HIGH);
        timerProcesso.stop();
        statoCorrente = COMPLETAMENTO;
      }
      break;
      
    case COMPLETAMENTO:
      // Lavaggio completato, in attesa di uscita
      if (sensoreB.fronteSalita) {
        Serial.println("STATO: USCITA - Veicolo in fase di uscita");
        digitalWrite(led_verde, LOW);
        digitalWrite(led_giallo, HIGH);
        timerTimeout.reset();
        timerTimeout.start();
        statoCorrente = USCITA;
      }
      break;
      
    case USCITA:
      // Veicolo a cavallo dell'uscita
      if (sensoreB.fronteDiscesa && !livelloC) {
        // Veicolo uscito completamente
        Serial.println("STATO: LIBERO - Veicolo uscito, sistema pronto");
        digitalWrite(led_giallo, LOW);
        digitalWrite(led_verde, HIGH);
        timerTimeout.stop();
        statoCorrente = LIBERO;
      }
      else if (sensoreB.fronteDiscesa && livelloC) {
        // Veicolo tornato indietro
        Serial.println("STATO: COMPLETAMENTO - Veicolo tornato nell'area");
        digitalWrite(led_giallo, LOW);
        digitalWrite(led_verde, HIGH);
        timerTimeout.stop();
        statoCorrente = COMPLETAMENTO;
      }
      else if (timerTimeout.get() > 30000) {
        // Timeout veicolo bloccato in uscita
        Serial.println("STATO: ALLARME - Veicolo bloccato in uscita");
        digitalWrite(led_giallo, LOW);
        digitalWrite(led_rosso, HIGH);
        digitalWrite(buzzer, HIGH);
        statoCorrente = ALLARME;
      }
      break;
      
    case ALLARME:
      // Gestione allarmi - Reset manuale necessario
      // Simuliamo il reset manuale con la presenza e successiva assenza di un veicolo
      if (!livelloA && !livelloB && !livelloC) {
        delay(5000); // Simulazione di un reset dopo 5 secondi
        Serial.println("STATO: LIBERO - Sistema resettato dopo allarme");
        digitalWrite(led_rosso, LOW);
        digitalWrite(buzzer, LOW);
        digitalWrite(led_verde, HIGH);
        // Reset di tutti gli attuatori per sicurezza
        digitalWrite(spruzzatori, LOW);
        digitalWrite(spazzole, LOW);
        digitalWrite(ventole, LOW);
        statoCorrente = LIBERO;
      }
      break;
  }
  
  // Gestione LED lampeggiante in stato di IN_ATTESA
  if (statoCorrente == IN_ATTESA) {
    static unsigned long ultimoLampeggio = 0;
    if (millis() - ultimoLampeggio > 500) { // Lampeggio ogni 500ms
      digitalWrite(led_giallo, !digitalRead(led_giallo));
      ultimoLampeggio = millis();
    }
  }
  
  delay(10); // Piccolo delay per stabilità
}
```

E' stata realizzata una struttura **SensoreFronte** che incapsula:
- Pin del sensore
- Livello attuale e precedente
- Rilevazione automatica dei fronti di salita e discesa
- Metodi per inizializzazione e aggiornamento
- Funzioni di gestione:
     - ```init()```: inizializza il sensore con il pin specificato
     - ```aggiorna()```: legge il nuovo valore, rileva eventuali fronti e aggiorna lo stato

>[Torna all'indice generale](indexstatifiniti.md)
