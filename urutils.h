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

// attesa NON bloccante del rilascio in pull-down: va chiamata a ogni giro
// di loop(). Restituisce true una sola volta, quando il pin, dopo essere
// stato HIGH, è rimasto LOW per più di t ms di fila. Ogni lettura HIGH
// (pulsante premuto, rimbalzo o disturbo) fa ripartire il conteggio.
// Il DiffTimer d (uno per pulsante) misura il tempo e ricorda che il
// pulsante era premuto: d.timerState è vero dalla pressione al rilascio.
inline bool waitUntilInputLowNB(DiffTimer &d, int btn, unsigned t) {
  if (digitalRead(btn) == HIGH) {
    d.start();   // no-op se già attivo
    d.reset();   // il conteggio riparte
  } else if (d.timerState && d.get() > t) {
    d.stop();    // rilascio confermato
    d.reset();
    return true;
  }
  return false;
}

// come sopra, per il rilascio in pull-up: t ms di HIGH continuo
inline bool waitUntilInputHighNB(DiffTimer &d, int btn, unsigned t) {
  if (digitalRead(btn) == LOW) {
    d.start();
    d.reset();
  } else if (d.timerState && d.get() > t) {
    d.stop();
    d.reset();
    return true;
  }
  return false;
}

// debounce a livello stabile (macchina a stati "prima gli ingressi", vedi debouncerFSM.md):
// accetta un nuovo livello, in TUTTE E DUE le direzioni, solo quando è rimasto fermo
// per più di t ms. Filtra rimbalzi, buchi a pulsante premuto e spike a pulsante rilasciato.
// Un oggetto per pulsante, creato con il livello di riposo (LOW in pull-down, HIGH in pull-up).
//   val0  = ultima lettura: ogni sua variazione fa ripartire il conteggio da last
//   val00 = livello accettato
// changed() va chiamata a ogni giro di loop() e restituisce true una sola volta, nel giro in
// cui il nuovo livello viene accettato: guardando val00 si sa se è una pressione o un rilascio.
struct Debouncer {
  byte val0;
  byte val00;
  unsigned long last;

  Debouncer(byte riposo = LOW) : val0(riposo), val00(riposo), last(0) {}

  bool changed(int btn, unsigned t) {
    byte val = digitalRead(btn);
    if (val != val0) {                    // variazione: il conteggio riparte
      val0 = val;
      last = millis();
    } else if (val != val00 && millis() - last > t) {
      val00 = val;                        // livello fermo da più di t ms: accettato
      return true;
    }
    return false;
  }
};

#endif
