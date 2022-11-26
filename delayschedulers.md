>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in Python](taskschedpy.md)


```C++
byte led1 = 13;
byte led2 = 12;
byte led3 = 11;
byte led4 = 10;
int count;
bool uno;

void setup()
{
  Serial.begin(115200);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  count = 1;
  uno = true;
  setLed();
  delay(1000);
  resetLed();
}

void resetLed(){
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);
  digitalWrite(led3, LOW);
  digitalWrite(led4, LOW);
}

void setLed(){
  digitalWrite(led1, HIGH);
  digitalWrite(led2, HIGH);
  digitalWrite(led3, HIGH);
  digitalWrite(led4, HIGH);
}

void loop()
{
  if(uno){
    if(count<=40){
		digitalWrite(led1, HIGH);
		delay(500);
		digitalWrite(led1, LOW);
		delay(500);
		if(count==10){
			digitalWrite(led2, HIGH);
		}
		if(count==20){
			digitalWrite(led3, HIGH);	
		}
		if(count==30){
			resetLed();
			uno = false;
			count = 1;
		}else{
			count++;
		}
    }
  }else{
    if(count<=40){
		digitalWrite(led4, HIGH);
		delay(500);
		digitalWrite(led4, LOW);
		delay(500);
		if(count==10){
			digitalWrite(led3, HIGH);
		}
		if(count==20){
			digitalWrite(led2, HIGH);	
		}
		if(count==30){
			resetLed();
			uno = true;
			count = 1;
		}else{
			count++;
		}
	 }
   }
}
```




>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in Python](taskschedpy.md)
