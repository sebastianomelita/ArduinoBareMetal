typedef enum ASYNC_EVT { ASYNC_INIT = 0, ASYNC_CONT = ASYNC_INIT, ASYNC_DONE = 1 } async;
#define async_state unsigned _async_k
struct as_state { async_state; };
#define async_begin(k) unsigned *_async_k = &(k)->_async_k; switch(*_async_k) { default:
#define async_end *_async_k=ASYNC_DONE; case ASYNC_DONE: return ASYNC_DONE; }
#define await(cond) await_while(!(cond))
#define await_while(cond) *_async_k = __LINE__; case __LINE__: if (cond) return ASYNC_CONT
#define async_yield *_async_k = __LINE__; return ASYNC_CONT; case __LINE__:
#define async_exit *_async_k = ASYNC_DONE; return ASYNC_DONE
#define async_init(state) (state)->_async_k=ASYNC_INIT
#define async_done(state) (state)->_async_k==ASYNC_DONE
#define async_call(f, state) (async_done(state) || (f)(state))
#define await_delay(delay) \
{ \
  do { \
    static unsigned long as_sleep; \
    as_sleep = millis(); \
    await(millis() - as_sleep > delay); \
  } while(false); \
}

#define MAX_BUFFER 		64
#define WNDW			10
#define STOP_BIT  		10
#define MSG  			55 	
#define SOFV			254	  
#define ACK  			129 //(100000001)  il primo (MSB) bit è un ack bit
#define NACK  			128 //(100000001)  il primo (MSB) bit è un ack bit
#define TBASE			3000
#define MAXATTEMPTS  	5
#define WNDW    		2000
#define TXTIMEOUT 		2000

#include <SoftwareSerial.h>
#define swPin 3
#define swSpeed	2400
#define dirPin 13

SoftwareSerial *mySerial;


// struttura dati parallela (non serializzata) che costituisce il messaggio
typedef struct{
   uint8_t u8sof;
   uint8_t u8da;          
   uint8_t u8sa;
   uint8_t u8group;         
   uint8_t u8si;    
   uint8_t data[20];    //lunghezza massima del payload!!!  
   int8_t msglen;
   bool multicast;
} telegram_t;

void sendMsg(telegram_t *tosend);
bool sendData(telegram_t *tosend);
void parallelToSerial(const telegram_t *tosend);
int8_t poll(telegram_t *rt);
void rcvEvent(telegram_t* rcvd, uint8_t msglen);
//lo calcola dal primo byte del messaggio (header compreso)
uint16_t calcCRC(uint8_t);

// offesets (spiazzamenti) dei campi del messaggio a partire dall’inizio
enum MESSAGE
{
    DA            = 0, //!< destination address position
    SA,		//!< source address position
    GROUP, 		//!< Function code position
    SI, 		//!< Service identifier position
    BYTE_CNT,  	//!< byte counter position
    PAYLOAD 	//!<  start of data position
};

// stati del protocollo
enum PROTO_STATE
{
    MSGWAIT             	= 1,
    ACKWAIT              	= 2
};
enum FLAGS
{
    ACKARRIVED             	= 0,
    MSGARRIVED              = 1,
	TIMEOUTEVNT             = 2,
	CHANNELFREE             = 3
};

enum ERR_LIST
{
    ERR_BUFF_OVERFLOW             = -1,
    ERR_BAD_CRC                   = -2,
};

uint8_t u8Buffer[MAX_BUFFER]; // buffer unico condiviso per TX e RX (da usare alternativamente)
uint8_t u8lastRec; // per rivelatore di fine trama (stop bit)
uint32_t u32time, u32difsTime;
uint8_t _txpin;
uint8_t n=0;
uint8_t mysa;
uint8_t mygroup;
uint8_t u8state = MSGWAIT;
unsigned long precAck=0;
unsigned long timeoutTime=TXTIMEOUT;
Stream *port;
bool flags[4];// flags[ACKARRIVED], flags[MSGARRIVED], flags[TIMEOUTEVNT]

char* rcvdata;
telegram_t txobj, rxobj, ackobj;
unsigned long prec=0;
byte txBtn = 4;
byte led=9; 
long tt;
uint8_t u8BufferSize = 0;

uint8_t getMySA(){
	return mysa;
}

uint8_t getMyGroup(){
	return mygroup;
}

bool isMsgWaitState(){
	return (u8state == MSGWAIT);
}

void rcvEventCallback(){
	Serial.print("Ricevuto:");
	Serial.println(rcvdata);
}

bool dataFrameArrived(){
	return flags[MSGARRIVED];	
}

bool ack_received(){
	return flags[ACKARRIVED];
}

bool ackOrTimeout(){
	return (flags[ACKARRIVED] || flags[TIMEOUTEVNT]);
}

bool channelFree(){
	return flags[CHANNELFREE];
}

long getBackoff(){
	return random(0, WNDW*pow(2,n));
}

void init(Stream *port485, uint8_t _txpin485, uint8_t mysa485, uint8_t mygroup485, uint32_t u32speed=0){
	randomSeed(analogRead(0));
	port = port485;
	_txpin = _txpin485;
	mysa = mysa485;
	mygroup = mygroup485;
	if(u32speed > 0){
		static_cast<HardwareSerial*>(port)->begin(u32speed);
	}
	ackobj.u8sa = mysa;
	ackobj.u8group = mygroup;
	ackobj.u8si = ACK;
	//ackobj.data = 0;
	ackobj.msglen = 0;
}

void sendAck(){
	flags[MSGARRIVED] = false;
	ackobj.u8si = ACK;
	sendMsg(&ackobj); 
	Serial.println("receiver ack sent ");
}

bool sendData(telegram_t *tosend){
	bool sent = false;
	tosend->u8si = MSG;
	sendMsg(tosend);
	u8state = ACKWAIT;
	if(u8state == MSGWAIT){
		sent = true;
	}
	precAck = millis();
	flags[ACKARRIVED] = false;
	flags[TIMEOUTEVNT] = false;
	Serial.print("Sent data: ");
	return sent;
}
//invia un messaggio in formato "parallelo" sotto forma di struct 
// sull'uscita seriale
void sendMsg(telegram_t *tosend){
	tosend->u8sa = mysa;
	tosend->u8group = mygroup;
	parallelToSerial(tosend);
	sendTxBuffer(u8Buffer[ BYTE_CNT ]); //trasmette sul canale
}

void printRxBuffer(uint8_t u8BufferSize)
{
	uint8_t i=0;
	Serial.println();
    while ( i < u8BufferSize ) // finchè ce ne sono, leggi i caratteri && u8Buffer[ i ]!=0
    {					  // e mettili sul buffer di ricezione
		Serial.print("(");
		Serial.print((char) u8Buffer[ i ]);
		Serial.print(":");
		Serial.print((uint8_t) u8Buffer[ i ],DEC);
		Serial.print("),");
        i ++;
    }
	Serial.println();
}

// carica i campi della struct nella giusta posizione nel buffer 
// secondo il formato del messaggio stabilito dal protocollo
void parallelToSerial(const telegram_t *tosend){
	//copia header
	u8Buffer[ DA ] = tosend->u8da; //2
	u8Buffer[ SA ] = tosend->u8sa; //1
	u8Buffer[ GROUP ] = tosend->u8group; //1
	u8Buffer[ SI ] = tosend->u8si; //55
	u8Buffer[ BYTE_CNT ] = tosend->msglen + PAYLOAD; //15
	//copia payload
	for(int i=0; i < tosend->msglen; i++){
		u8Buffer[i+PAYLOAD] = tosend->data[i];
	}
}

//lo calcola dal primo byte del messaggio (header compreso)
uint16_t calcCRC(uint8_t u8length)
{
    unsigned int temp, temp2, flag;
    temp = 0xFFFF;
    for (unsigned char i = 0; i < u8length; i++)
    {
        temp = temp ^ u8Buffer[i];
        for (unsigned char j = 1; j <= 8; j++)
        {
            flag = temp & 0x0001;
            temp >>=1;
            if (flag)
                temp ^= 0xA001;
        }
    }
    // Reverse byte order.
    temp2 = temp >> 8;
    temp = (temp << 8) | temp2;
    temp &= 0xFFFF;
    // the returned value is already swapped
    // crcLo byte is first & crcHi byte is last
    return temp;
}

bool checkCRC(uint8_t u8BufferSize){
	bool ok = true;
	
	// gli ultimi due byte del messaggio sono il CRC calcolato dal Tx
	// confonta il CRC ricevuto dal Tx con quello calcolato in locale
    uint16_t u16MsgCRC = // etraggo il CRC del Tx dal messaggio
        ((u8Buffer[u8BufferSize - 2] << 8)
         | u8Buffer[u8BufferSize - 1]); // combine the crc Low & High bytes
    // calcolo il CRC del Rx e lo confronto con quello del Tx
    if ( calcCRC( u8BufferSize-2 ) != u16MsgCRC ) //except last two byte
    {
            ok = false;
    }
	return ok;
}

// ascolta in polling l'ingresso seriale
int8_t poll(telegram_t *rt)
{
	uint8_t u8current;
	//GENERATORE EVENTI
	// Lista di condizioni annidate. La valutazione di una condizione è vincolata alla verità di tutte quelle che la precedono.
	// La relazione tra le condizioni consiste in un AND logico tra una condizione e la successiva.
	// Per rendere più leggibile il codice, la valutazione annidata delle condizioni è sostituita da una valutazione 
	// sequenziale delle stesse che può far ritornare in anticipo la funzione prima dell'esecuzione del codice a valle 
	// di una condizione non vera.
	// Ogni condizione valuta l'accadere di un evento. Un evento può essere un particolare valore in ingresso (messaggo) 
	// o lo scadere del tempo (timeout) di un timer. 
	// Il mancato accadimento di un evento in cima alla lista disattiva la valutazione dell'accadimento di tutti quelli a valle // (short circuit). 
	
	
	// RECEIVE EVENT DETECTOR. Polling che controlla quanti caratteri del messaggio sono arrivati
	// sulla coda di ricezione
	u8current = port->available();

	if (u8current == 0){ // nessun messaggio 
		flags[CHANNELFREE]=true;
		//allora valuta lo scadere del timer
		if(u8state == ACKWAIT){
			if(millis()-precAck > timeoutTime){
				flags[TIMEOUTEVNT] = true;
			}
		}
		// rendi mutuamente esclusivo il blocco di codice
		return 0;  // se non è arrivato nulla ricontrolla al prossimo giro
	}

	flags[CHANNELFREE]=false;
	
	// RESET STOP BIT TIMER. Se arrivano nuovi caratteri rimani in ascolto
	// perchè il messaggio è ancora incompleto
	if (u8current != u8lastRec)
	{
		// aggiorna ogni volta che arriva un nuovo carattere!
		u8lastRec = u8current;
		u32time = millis();
		// rendi mutuamente esclusivo il blocco di codice
		return 0;
	}
	
	// STOP BIT DETECTOR. Se non arrivano nuovi caratteri ma è troppo presto allora aspetta
	if ((unsigned long)(millis() -u32time) < (unsigned long)STOP_BIT) {
		// rendi mutuamente esclusivo il blocco di codice
		return 0;
	}
	// COPY TO RECEIVE BUFFER. Se non arrivano nuovi caratteri ma è passato il tempo di interframe
	// alllora vuol dire che la trama è completa allora bufferizza
   	int8_t i8state = getRxBuffer();
	Serial.print("Received: ");
	printRxBuffer(u8Buffer[ BYTE_CNT ]);
    // INCOMPLETE MESSAGES DETECTOR. Se è palesemente incompleta scartala!
   	if (i8state < PAYLOAD) 
	{
		// rendi mutuamente esclusivo il blocco di codice
		if(i8state == ERR_BAD_CRC){
			Serial.println("Rilevata probabile collisione!");
		}
		Serial.println("Messaggio incompleto: ");
		return i8state;
	}
	
	 if (((u8Buffer[ DA ] == mysa) || ((u8Buffer[ GROUP ] == mygroup)) && (u8Buffer[ DA ] == 255))){	
		Serial.println("msg destinato a me");
		Serial.print("DA: ");
		Serial.println((uint8_t)u8Buffer[ DA ]);
		Serial.print("SA mio: ");
		Serial.println((uint8_t)mysa);
		Serial.print("SI: ");
		Serial.println((uint8_t)u8Buffer[ SI ]);
	}else{
		Serial.println("msg NON destinato a me");
		return 0;  // altrimenti se il messaggio non è indirizzato a me...scarta
	}
	
	// MSSAGE SELECTOR.
    if (u8Buffer[ SI ] == MSG){ // se ricevo un messaggio
		// prendi l'indirizzo di sorgente del messaggio ricevuto
		// e fallo diventare indirizzo di destinazione del messaggio di ack
		ackobj.u8da = u8Buffer[ SA ]; 
		rcvEvent(rt, i8state); // parallelizza
		// se ricevo un messaggio invio l'ack
		ackobj.u8si = ACK;
		flags[MSGARRIVED] = true;
		// funzione che realizza l'azione da compiere all'arrivo del  //messaggio
		rcvdata = (char*)rt->data;
		// rendi mutuamente esclusivo il blocco di codice
		return i8state; 
	}else if (u8Buffer[ SI ] == ACK){ // se ricevo un ack
		if(u8state == ACKWAIT){
			u8state = MSGWAIT;	//next go to MSGWAIT
			flags[ACKARRIVED] = true;
		}//else messaggio di ack si perde....
	}
	return i8state;
}

void sendTxBuffer(uint8_t u8BufferSize){
	// switch to TX
	mySerial->end();   // nicely close the serial port
	delete mySerial;    // remove the object
	mySerial = new SoftwareSerial( -1, swPin);  // does -1 disable the RX ? I'm not sure.
	mySerial->begin(swSpeed);
	uint16_t u16crc = calcCRC( u8BufferSize );
    u8Buffer[ u8BufferSize ] = u16crc >> 8;  //seleziona il byte più significativo
    u8BufferSize++;
    u8Buffer[ u8BufferSize ] = u16crc & 0x00ff; //seleziona il byte meno significativo
    u8BufferSize++;
	// transfer buffer to serial line
	port->write( u8Buffer, u8BufferSize );
	port->flush();
	while(port->read() >= 0);
	// switch to RX
	mySerial->end();   // nicely close the serial port
	delete mySerial;    // remove the object
	mySerial = new SoftwareSerial( swPin, -1);  // does -1 disable the RX ? I'm not sure.
	mySerial->begin(swSpeed);
	Serial.print("Transmitted: ");
	printRxBuffer(u8BufferSize);
}

int8_t getRxBuffer()
{
    //uint8_t u8BufferSize = 0;
	uint8_t app;

    while ( port->available() && u8BufferSize <= BYTE_CNT) // finchè ce ne sono, leggi i caratteri
    {					  // e mettili sul buffer di ricezione
        u8Buffer[ u8BufferSize ] = port->read();
        u8BufferSize ++;
    }
	//leggi il messaggio più i due byte di CRC
	while ( port->available() && u8BufferSize < u8Buffer[ BYTE_CNT ]+2) // finchè ce ne sono, leggi i caratteri
    {					  // e mettili sul buffer di ricezione
        u8Buffer[ u8BufferSize ] = port->read();
        u8BufferSize ++;
		// segnala evento di buffer overflow (un attacco hacker?)
        if (u8BufferSize >= MAX_BUFFER){
			return ERR_BUFF_OVERFLOW;
		}
    }
	if(u8BufferSize == u8Buffer[ BYTE_CNT ]+2){
		app = u8BufferSize;
		u8BufferSize = 0;
		if(!checkCRC(u8Buffer[ BYTE_CNT ]+2)){
			return ERR_BAD_CRC;
		}
	}
    return app;
}

// deserializzzazione in ricezione
void rcvEvent(telegram_t* rcvd, uint8_t msglen){
// converti da formato seriale (array di char) in formato parallelo   
//(non serializzato)
	// header
	rcvd->u8da = u8Buffer[ SA ];
	rcvd->u8group = u8Buffer[ GROUP ];
	rcvd->u8si = u8Buffer[ SI ];
	rcvd->msglen = u8Buffer[ BYTE_CNT ];
	// payload
	for(int i=0; i < msglen-PAYLOAD; i++){
		rcvd->data[i] = u8Buffer[i+PAYLOAD];
	}
	// notifica l'evento di ricezione all'applicazione con una callback
}
//----------------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------------------------
as_state ptRcv, ptSend;
async rcvTask(as_state *pt);
async sendTask(as_state *pt);

async rcvTask(as_state *pt) {
  async_begin(pt);
  // Loop forever
  while(true) {
	await(dataFrameArrived());
	rcvEventCallback();
	sendAck();  
  }
  async_end;
}

async sendTask(as_state *pt) {
  async_begin(pt);
  // Loop forever
  while(true) {
	 Serial.println("Premi il tasto per trasmettere un messaggio.");
	 await(digitalRead(txBtn));
	 digitalWrite(led, HIGH);
	 await_delay(50);
	 digitalWrite(led, LOW);
	 await_delay(50);	
	 n = 0;  //azzera conteggio
	 while(n < MAXATTEMPTS){
		Serial.println("Attendo che si liberi il canale: ");
		await(channelFree()); 
		sendData(&txobj);
		Serial.println("Attendo ack o timeout: ");
		await(ackOrTimeout());
		if(ack_received()){
			n = MAXATTEMPTS;
			Serial.println("Ricevuto ack: ");
		}else{
			Serial.print("Timeout n: ");
			Serial.print(n);
			Serial.print(": ritrasmissione tra: ");
			tt = getBackoff();
			Serial.print((float) tt/1000);
			Serial.println(" secondi");
			/* timeout scaduto: ritrasmissione*/
			await_delay(tt);
			Serial.print(" Ritrasmesso.");
			n++;			
		}
	 }
  }
  async_end;
}

void setup(){
	// default: RX
	mySerial = new SoftwareSerial(swPin, -1);  // does -1 disable the TX ? I'm not sure.
	mySerial->begin(swSpeed);
	pinMode(led, OUTPUT); 
	pinMode(txBtn, INPUT);
	async_init(&ptRcv);
	async_init(&ptSend);
	Serial.println("I am sender!");
	Serial.begin(115200);
	init(mySerial, dirPin, 3, 1, 0); // port485, txpin, mysa, mygroup4, speed=9600
	//preparazione messaggio TX (parallelo)
	txobj.u8da = 1;
	strcpy((char*)txobj.data,"ciaop");
	txobj.msglen = strlen((char*) txobj.data) + 1;
}

void loop() // run over and over
{
	poll(&rxobj);
	rcvTask(&ptRcv);
	sendTask(&ptSend);
}