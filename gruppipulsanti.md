
>[Torna all'indice](indexpulsanti.md)
## **GESTIONE GRUPPI DI PULSANTI**

La **logica di molti** pulsanti potrebbe essere gestita sotto forma di **dati strutturati** (multivalore) in una delle seguenti maniere: array, struct ed oggetti.

In ciascuna delle precedenti strutture dati vanno memorizzate almeno **due cose**: lo **stato** del pulsante, la memoria del suo **ingresso passato**, cioè il valore dell’ingresso al loop precedente (serve per rilevare una transizione).

La **differenza** tra i vari modi di gestione consiste in come si memorizzano stati, ingressi e logica:

- **Array**. Una **sola funzione** gestisce la logica di più pulsanti, **stato** e **ingressi passati** sono memorizzati sulle celle di **due array paralleli**. Gli array sono comuni a tutti i pulsanti, ma ogni **indice** di entrambi gli array si riferisce sempre ad un solo pulsante.

- **Struct**. Una **sola funzione** gestisce la logica di più pulsanti, **stato** e **ingressi passati** sono memorizzati su una struct **separata** per ogni pulsante. Al limite le struct possono essere organizzate in un array, soprattutto in presenza di molti tasti.

- **Oggetti. Stato** e **ingressi passati** sono memorizzati su un oggetto **separato** per ogni pulsante. Ogni oggetto possiede anche la funzione per manipolare i dati contenuti al suo interno (metodo). I metodi sono accessibili, a partire dalla variabile oggetto, usando la “dot” notation cioè, il nome della variabile, un punto, ed il nome del metodo. Anche in questo caso, volendo, per molti tasti gli oggetti potrebbero essere organizzati in un array o in una matrice.

**Esempio di due pulsanti toggle gestiti con due array paralleli**
```C++
/*
Scrivere un programma Arduino che accenda due led (cucina, soggiorno).
Accenderli con due pulsanti toggle separati.
*/
#define TBASE 100
#define CMDSOGGIORNO 2
#define CMDCUCINA 3
#define LEDSOGGIORNO 12
#define LEDCUCINA 13
enum btn
{
	CUCINA  =0,
	SOGGIORNO  =1
};

byte precval[2]={0,0};
byte stato[2]={0,0};
unsigned long precm=0;

void setup(){
	pinMode(CMDSOGGIORNO, INPUT);
	pinMode(CMDCUCINA, INPUT);
	pinMode(LEDSOGGIORNO, OUTPUT);
	pinMode(LEDCUCINA, OUTPUT);
}

bool transizione(byte val,byte n){ //transizione di un pulsante
	bool cambiato=false;
	cambiato = (precval[n] != val);
	precval[n] = val;
	return cambiato;
}

void loop(){
	byte in;

	if(millis()-precm>=(unsigned long)TBASE){ //schedulatore e antirimbalzo
		precm=millis();
		//polling pulsante cucina
		in=digitalRead(CMDCUCINA);
		if(transizione(in,CUCINA))				{
			if(in==HIGH){ //se fronte di salita
				stato[CUCINA] = !stato[CUCINA];
				digitalWrite(LEDCUCINA,stato[CUCINA]);
			}
		}
		//polling pulsante soggiorno
		in=digitalRead(CMDSOGGIORNO);
		if(transizione(in,SOGGIORNO)){
			if(in==HIGH){ //se fronte di salita
				stato[SOGGIORNO] = !stato[SOGGIORNO];
				digitalWrite(LEDSOGGIORNO,stato[SOGGIORNO]);
			}
		}
	} //chiudi schedulatore
}
```
Si noti l’uso delle due **costanti enum** CUCINA e SOGGIORNO per gestire in maniera **automatica** gli **indici** dei due array paralleli
```C++
byte stato[2];
byte precval[2];
```
Ad esempio, il valori dello **stato** e dell’**ingresso** **passato** del pulsante CUCINA si trovano rispettivamente:
```C++
statoCucina = stato[CUCINA];
precvalCucina = precval[CUCINA];
```
Negli esempi successivi utilizzeremo una funzione di rilevazione delle transizioni diversa perché mirata a rilevare soltanto i fronti di salita valutando la condizione 
```C++
precval  == LOW  && val == HIGH.
```
In maniera analoga si possono creare rilevatori di fronti di discesa valutando la condizione 
```C++
precval  == HIGH  && val == LOW.
```
L’uso di queste funzioni permette di **spostare** la **logica del comando** toggle (stato = !stato dal loop alla funzione di gestione del tasto, semplificando il codice del loop.

Adesso però conviene **cambiare il nome** della funzione poiché non rappresenta più soltanto un generico rilevatore di transizioni ma anche la logica “toggle” valutata sul fronte di salita.

Volendo, una funzione analoga si poteva usare anche sopra, nel codice dei pulsanti con gli array.

**Esempio di due pulsanti toggle gestiti con due struct**
```C++
/*
Scrivere un programma Arduino che accenda due led (cucina, soggiorno).
Accenderli con due pulsanti toggle separati.
*/
#define TBASE 100
#define CMDSOGGIORNO 2
#define CMDCUCINA 3
#define LEDSOGGIORNO 12
#define LEDCUCINA 13
struct Toggle
{
	uint8_t precval;
	uint8_t stato;
} cucina, soggiorno;

unsigned long precm=0;
void setup(){
	pinMode(CMDSOGGIORNO, INPUT);
	pinMode(CMDCUCINA, INPUT);
	pinMode(LEDSOGGIORNO, OUTPUT);
	pinMode(LEDCUCINA, OUTPUT);
}

bool toggleH(byte val, struct Toggle &btn) { 		//transizione di un pulsante
	bool cambiato = false;
	if (btn.precval  == LOW  && val == HIGH){
		cambiato = true;
		btn.stato = !btn.stato;
	}
	return cambiato;
}

void loop(){
	byte in;
	
	if(millis()-precm>=(unsigned long)TBASE){ 	//schedulatore e antirimbalzo
		precm=millis();
		//polling pulsante cucina
		in=digitalRead(CMDCUCINA);
		if(toggleH(in,cucina)){
			digitalWrite(LEDCUCINA, cucina.stato);
		}
		//polling pulsante soggiorno
		in=digitalRead(CMDSOGGIORNO);
		if(toggleH(in,soggiorno)){
			digitalWrite(LEDSOGGIORNO, soggiorno.stato);
		}
	} //chiudi schedulatore
}
```
**Esempio di due pulsanti toggle gestiti con due oggetti**
```C++
/*
Scrivere un programma Arduino che accenda due led (cucina, soggiorno).
Accenderli con due pulsanti toggle separati.
*/
#define TBASE 100
#define CMDSOGGIORNO 2
#define CMDCUCINA 3
#define LEDSOGGIORNO 12
#define LEDCUCINA 13

class Toggle
{
	private:
	uint8_t precval;
	uint8_t stato;

	public:
	Toggle(uint8_t new_precval = 0, uint8_t new_stato = 0){
		precval = new_precval;
		stato = new_stato;
	}

	bool toggleH(uint8_t val){ //transizione di un pulsante
		bool cambiato = false;
		if (precval  == LOW  && val == HIGH){
			cambiato = true;
			stato = !stato;
		}
		precval = val;
		return cambiato;
	}

	uint8_t getPrecval()
	{
		return precval;
	}

	void setPrecval(uint8_t new_precval)
	{
		precval = new_precval;
	}
	
	uint8_t getStato()
	{
		return stato;
	}

	void setStato(uint8_t new_stato)
	{
		stato = new_stato;
	}
};

Toggle cucina(0,0);
Toggle soggiorno(0,0);
unsigned long precm=0;

void setup(){
	pinMode(CMDSOGGIORNO, INPUT);
	pinMode(CMDCUCINA, INPUT);
	pinMode(LEDSOGGIORNO, OUTPUT);
	pinMode(LEDCUCINA, OUTPUT);
}

void loop(){
	uint8_t in;
	if(millis()-precm>=(unsigned long)TBASE){ 	//schedulatore e antirimbalzo
		precm=millis();
		//polling pulsante cucina
		in=digitalRead(CMDCUCINA);
		if(cucina.toggleH(in)){
			digitalWrite(LEDCUCINA, cucina.getStato());
		}
		//polling pulsante soggiorno
		in=digitalRead(CMDSOGGIORNO);
		if(soggiorno.toggleH(in)){
			digitalWrite(LEDSOGGIORNO, 			soggiorno.getStato());
		}
	} //chiudi schedulatore
}
>[Torna all'indice](indexpulsanti.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbNzYwMzMwMTMxXX0=
-->