#ifndef __URUTILS_H__
#define __URUTILS_H__

// attesa evento con tempo minimo di attesa
void waitUntil(bool &c, unsigned t)
{
    while(!c){
	delay(t);
    }
}

void waitUntilInputLow(int btn, unsigned t)
{
	do{
		delay(t);
	}while(digitalRead(btn)!=LOW);
}

void waitUntilInputHigh(int btn, unsigned t)
{
	do{
		delay(t);
	}while(digitalRead(btn)!=HIGH);
}
/*
void waitUntilInputLow(int btn, unsigned t)
{
    while(!digitalRead(btn)==LOW){
	delay(t);
    }
}

void waitUntilInputHigh(int btn, unsigned t)
{
    while(digitalRead(btn)==LOW){
	    delay(t);
    }
}
*/
struct DiffTimer
{
	volatile unsigned long elapsed, last;
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
			return millis() - last + elapsed;
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
	volatile unsigned long elapsed, tbase;
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

bool waitUntilLow(DiffTimer &d, bool val, unsigned t);

bool waitUntilNB(DiffTimer &d, bool c, unsigned t){
	bool ok = false;
	if (!c) {// fronte di salita condizione
		d.start(); // campionamento singleton del tempo
	}
	// fronte di discesa dopo rimbalzi
	if(d.get() > t && c){
		// disabilitazione fino al prossimo fronte di salita
		d.stop();
		d.reset();
		ok = true;
	}
	return ok;
}
#endif
