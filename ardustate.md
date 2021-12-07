

#include <Arduino.h>

enum class State :uint8_t {
  noop,
  wait,
  state1,
  state2,
  state3
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
    case State::noop:
      //maybe wait for input? (for now just send anything to serial);
      if (Serial.available() > 0) {
        Serial.readString();
        state = State::state1;
      }
      break;
    case State::wait:
      if (currentTime - prevTime > waitDelay) {
        state = nextState;
        prevTime = currentTime;
      }
      break;
    case State::state1:
      nextStateAfter(State::state2, 1000);
      Serial.println("state1");
      break;

    case State::state2:
      nextStateAfter(State::state3, 2000);
      Serial.println("state2");
      break;

    case State::state3:
      nextStateAfter(State::state1, 3000);
      Serial.println("state3");
      break;

  }
}
