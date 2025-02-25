
>[Torna all'indice](indexstatifiniti.md)

## **MACCHINA A STATI GENERICA PER ARDUINO**

```C++
#include <Arduino.h>

enum {
  NOOP,
  WAIT,
  STATE1, // stato geenerico, sostituiscilo con un nome significativo per questo stato
  STATE2, // stato geenerico, sostituiscilo con un nome significativo per questo stato
  STATE3  // stato geenerico, sostituiscilo con un nome significativo per questo stato
};

State nextState;
State state;

uint32_t prevTime = millis();
uint32_t currentTime = millis();
uint32_t waitDelay;

void setup() {
  Serial.begin(9600);
}

void nextStateAfter(State ss, uint32_t d) {
  waitDelay = d;
  nextState = ss;
  state = State::wait;
  prevTime = currentTime;
}

void loop() {
  currentTime = millis();

  switch (state) {
    case NOOP:
      //maybe wait for input? (for now just send anything to serial);
      if (Serial.available() > 0) {
        Serial.readString();
        state = STATE1;
      }
      break;
    case WAIT:
      if (currentTime - prevTime > waitDelay) {
        state = nextState;
        prevTime = currentTime;
      }
      break;
    case STATE1:
      nextStateAfter(STATE2, 1000);
      Serial.println("state1");
      break;

    case STATE2:
      nextStateAfter(STATE3, 2000);
      Serial.println("state2");
      break;

    case STATE3:
      nextStateAfter(STATE1, 3000);
      Serial.println("state3");
      break;

  }
}
```

**Sitografia:**
- https://9g.lt/blog/arduino-state-machine-with-millis-instead-of-delay

>[Torna all'indice](indexstatifiniti.md)


```

**Sitografia:**
- https://9g.lt/blog/arduino-state-machine-with-millis-instead-of-delay

>[Torna all'indice](indexstatifiniti.md)
