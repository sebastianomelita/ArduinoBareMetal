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
    while(!digitalRead(btn)==LOW){
	delay(t);
    }
}

typedef struct 
{
	unsigned long elapsed, last;
	bool timerState=false;
	void reset(){
		elapsed = 0;
		last = millis();
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
} DiffTimer;
#endif
