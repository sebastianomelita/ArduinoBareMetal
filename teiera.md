
# **TEIERA AUTOMATICA**

Scrivere un programma che realizzi una teiera automatica tale che:
- si accenda premendo il pulsante A, alla cui pressione si mette a blinkare il led S di stato dell'acqua che così indica che la temperatura ancora non è quella giusta. 
- Una pressione del pulsante A mentre il led S blinka non ha alcun effetto.
- l'acqua è a temperatura quando, dopo 10 secondi, il led S non blinka più ma è a luminosità fissa
- premere il pulsante A con il  led S è fisso mette in azione la erogazione del te da un ugello, segnalata da un led E acceso
- l'erogazione termina dopo 3 secondi, passati i quali il led E si spegne.
- una pressione prolungata del pulsante A (più di 3 secondi) spegne la teiera disattivando tutti i led.

Prima di tutto, identifico gli stati presenti nel programma:
1. SPENTO - La teiera è spenta
2. RISCALDAMENTO - L'acqua si sta riscaldando (led S/led_A lampeggia)
3. BOLLITURA - L'acqua ha raggiunto la temperatura (led S/led_A fisso)
4. EROGAZIONE - Erogazione del tè in corso (led E/led_E acceso)

Gli input del sistema sono:
- Pressione del pulsante A (breve o prolungata)
- Timer t1 ≥ 10 secondi (riscaldamento completato)
- Timer t2 ≥ 3 secondi (erogazione completata)

Ecco la tabella di transizione di Mealy:

| Stato attuale | Input | Stato prossimo | Output |
|---------------|-------|---------------|--------|
| SPENTO | Pressione pulsante A | RISCALDAMENTO | Led S lampeggiante |
| RISCALDAMENTO | t1 < 10s | RISCALDAMENTO | Led S lampeggiante |
| RISCALDAMENTO | t1 ≥ 10s | BOLLITURA | Led S fisso |
| RISCALDAMENTO | Pressione pulsante A | RISCALDAMENTO | Led S lampeggiante (nessun effetto) |
| BOLLITURA | Pressione breve pulsante A | EROGAZIONE | Led E acceso |
| BOLLITURA | Pressione prolungata pulsante A (≥ 3s) | SPENTO | Led S e Led E spenti |
| EROGAZIONE | t2 < 3s | EROGAZIONE | Led E acceso |
| EROGAZIONE | t2 ≥ 3s | BOLLITURA | Led E spento, Led S fisso |

Questa tabella descrive il comportamento completo del sistema della teiera automatica secondo il codice fornito, mostrando come gli stati cambiano in risposta agli input e quali output vengono generati durante ciascuna transizione.

```mermaid
%%{init: {'theme': 'default', 'themeVariables': { 'primaryColor': '#ffffff', 'primaryTextColor': '#000000', 'primaryBorderColor': '#000000', 'lineColor': '#000000', 'secondaryColor': '#f4f4f4', 'tertiaryColor': '#ffffff' }}}%%
stateDiagram-v2
    [*] --> SPENTO
    
    SPENTO --> RISCALDAMENTO : Pressione pulsante A / Led S lampeggiante
    
    RISCALDAMENTO --> RISCALDAMENTO : t1 < 10s / Led S lampeggiante
    RISCALDAMENTO --> RISCALDAMENTO : Pressione pulsante A / Nessun effetto
    RISCALDAMENTO --> BOLLITURA : t1 ≥ 10s / Led S fisso
    
    BOLLITURA --> EROGAZIONE : Pressione breve pulsante A / Led E acceso
    BOLLITURA --> SPENTO : Pressione prolungata pulsante A (≥ 3s) / Led S e Led E spenti
    
    EROGAZIONE --> EROGAZIONE : t2 < 3s / Led E acceso
    EROGAZIONE --> BOLLITURA : t2 ≥ 3s / Led E spento, Led S fisso
    
    SPENTO --> [*] : Spegnimento
```

```C++
/*
Scrivere un programma che realizzi una teiera automatica tale che:
- si accenda premendo il pulsante A, alla cui pressione si mette a blinkare il led S di stato dell'acqua che così indica che la temperatura ancora non è quella giusta.
- Una pressione del pulsante A mentre il led S blinka non ha alcun effetto.
- l'acqua è a temperatura quando, dopo 10 secondi, il led S non blinka più ma è a luminosità fissa
- premere il pulsante A con il led S è fisso mette in azione la erogazione del te da un ugello, segnalata da un led E acceso
- l'erogazione termina dopo 3 secondi, passati i quali il led E si spegne.
- una pressione prolungata del pulsante A (più di 3 secondi) spegne la teiera disattivando tutti i led.
*/
#include "urutils.h"
int led_E=13;
int led_A=12;
int pulsante1=14;
int state;
DiffTimer t1, t2;

enum Stati{
  SPENTO,
  RISCALDAMENTO,
  BOLLITURA,
  EROGAZIONE
};

void blink(int led) {
  digitalWrite(led, !digitalRead(led));
}

void setup(){
  pinMode(pulsante1, INPUT);
  pinMode(led_E, OUTPUT);
  pinMode(led_A, OUTPUT);
  Serial.begin(115200);
  state = SPENTO; // inizialmente l'acqua è fredda
  t2.stop();
}

void loop(){
  if(state == SPENTO){
    Serial.println("SPENTO");
    if(digitalRead(pulsante1)==HIGH){
      waitUntilInputLow(pulsante1, 50); //fronte di discesa
      state = RISCALDAMENTO;
      Serial.println("RISCALDAMENTO");
      t1.start();
      t1.reset();
      t2.start();
      t2.reset();
    }
  }else if(state == RISCALDAMENTO){
    // eseguo il blink del led A
    if(t1.get() < 10000){
      if (t2.get() > 1000) { // polling timer blink
        t2.reset(); // riarmo timer blink
        Serial.println("blink");
        blink(led_A);
      }
    }else{
      t1.stop();
      t2.stop();
      digitalWrite(led_A, HIGH);
      state = BOLLITURA;
      Serial.println("BOLLITURA");
    }
  }else if(state == BOLLITURA){
    if(digitalRead(pulsante1)==HIGH){
      t1.start();
      t1.reset();
      waitUntilInputLow(pulsante1, 50); //fronte di discesa
      if(t1.get() > 3000){
        // spegnimento
        digitalWrite(led_E, LOW);
        digitalWrite(led_A, LOW);
        state = SPENTO;
        Serial.println("SPENTO");
      }else{
        t1.stop();
        state = EROGAZIONE;
        Serial.println("EROGAZIONE");
        t2.start();
        t2.reset();
      }
    }
  }else if(state == EROGAZIONE){
    if(t2.get() < 3000){
      digitalWrite(led_E, HIGH);
    }else{
      t2.stop();
      digitalWrite(led_E, LOW);
      state = BOLLITURA;
      Serial.println("BOLLITURA");
    }
  }
  delay(10);
}
```
