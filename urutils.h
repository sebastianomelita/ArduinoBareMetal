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
	byte state = 0;
	byte count = 0;
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
	byte state = 0;
	byte count = 0;
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
#endif
