#ifndef __URUTILS_H__
#define __URUTILS_H__

#include <Arduino.h>

// attesa evento con tempo minimo di attesa
inline void waitUntil(bool &c, unsigned t)
{
    while(!c){
        delay(t);
    }
}

// attesa bloccante finché il pin non è LOW (rilascio in pull-down).
// Dopo il primo LOW attende altri t ms e ricontrolla: se il pin è
// tornato HIGH (rimbalzo o contatto che si è aperto per un attimo)
// ricomincia ad aspettare. Esce solo con un LOW confermato.
inline void waitUntilInputLow(int btn, unsigned t) {
  do {
    do {
      delay(t);
    } while (digitalRead(btn) != LOW);
    delay(t);   // attesa fine rimbalzi del rilascio
  } while (digitalRead(btn) != LOW);   // rilascio confermato
}

// come sopra, per il pin HIGH (rilascio in pull-up)
inline void waitUntilInputHigh(int btn, unsigned t) {
  do {
    do {
      delay(t);
    } while (digitalRead(btn) != HIGH);
    delay(t);   // attesa fine rimbalzi del rilascio
  } while (digitalRead(btn) != HIGH);  // rilascio confermato
}

struct DiffTimer
{
	volatile unsigned long elapsed = 0, last = 0;
	volatile bool timerState=false;
	volatile byte state = 0;
	volatile byte count = 0;
	void reset(){
		elapsed = 0;
		last = millis();
	}
	void toggle(){
		if(timerState){
			stop();
		}else{
			start();
		}
	}
	void stop(){
		if(timerState){
			timerState = false;
			elapsed += millis() - last;
		}
	}
	void start(){
		if(!timerState){
			timerState = true;
			last = millis();
		}
	}
	unsigned long get(){
		if(timerState){
			return (unsigned long) millis() - last + elapsed;
		}
		return elapsed;
	}
	void set(unsigned long e){
		reset();
		elapsed = e;
	}
};

struct DiffTimer2
{
	volatile unsigned long elapsed = 0, tbase = 1;
	volatile bool timerState=false;
	volatile byte state = 0;
	volatile byte count = 0;
	void reset(){
		elapsed = 0;
	}
	void toggle(){
		if(timerState){
			stop();
		}else{
			start();
		}
	}
	void stop(){
		if(timerState){
			timerState = false;
		}
	}
	void start(){
		if(!timerState){
			timerState = true;
		}
	}
	unsigned long get(){
		if(timerState){
			elapsed += tbase;
		}
		return elapsed;
	}
	unsigned long peek(){
		return elapsed;
	}
	void set(unsigned long e){
		reset();
		elapsed = e;
	}
	void setBase(unsigned long e){
		tbase = e;
	}
};

// attesa non bloccante: restituisce true una sola volta, quando la
// condizione c è rimasta vera per più di t ms di fila.
// Finché c è falsa il timer riparte a ogni chiamata.
inline bool waitUntilNB(DiffTimer &d, bool c, unsigned t) {
  bool ok = false;
  if (!c) {
    d.start();
    d.reset();   // il timer riparte a ogni chiamata con c falsa
  }
  if (d.get() > t && c) {
    d.stop();
    d.reset();
    ok = true;
  }
  return ok;
}

#endif
