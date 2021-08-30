>[Torna a gateway LoraWan con modem HW](gatewayi2crak811hw.md)

## **Gateway per BME280**

### **Sensore di temperatura, umidità e pressione BME280**

<img src="BME280.jpg" alt="alt text" width="400">

### **Esempio di cablaggio**
<img src="ESP32-bme280_schematic.jpg" alt="alt text" width="600">



### **Librerie del progetto**

Dal **punto di vista SW** seve **4 librerie** da scaricare dentro la solita cartella **libraries**:
- **adafruit bme280**. Si scarica da https://github.com/adafruit/Adafruit_BME280_Library come Adafruit_BME280_Library-master.zip da scompattare e rinominare semplicemente come **Adafruit_BME280_Library**
- **Adafruit Unified Sensor**. Si scarica da https://github.com/adafruit/Adafruit_Sensor come Adafruit_Sensor-master.zip da scompattare e rinominare semplicemente come **Adafruit_Sensor**

### **Gateway I2C-BME280 con modem RAK811**

```C++
/********************************************************
 * This demo is only supported after RUI firmware version 3.0.0.13.X on RAK811
 * Master Board Uart Receive buffer size at least 128 bytes. 
 ********************************************************/

#include "RAK811.h"
#include "SoftwareSerial.h"
#include <Adafruit_BME280.h>
#define WORK_MODE LoRaWAN   //  LoRaWAN or LoRaP2P
#define JOIN_MODE OTAA    //  OTAA or ABP
#if JOIN_MODE == OTAA
String DevEui = "8680000000000001";
String AppEui = "70B3D57ED00285A7";
String AppKey = "DDDFB1023885FBFF74D3A55202EDF2B1";
#else JOIN_MODE == ABP
String NwkSKey = "69AF20AEA26C01B243945A28C9172B42";
String AppSKey = "841986913ACD00BBC2BE2479D70F3228";
String DevAddr = "260125D7";
#endif
#define TXpin 11   // Set the virtual serial port pins
#define RXpin 10
//#define SensorPin A0  // used for Arduino and ESP8266
#define SensorPin 4     // used for ESP32
#define DebugSerial Serial
#define TX_INTERVAL ((uint32_t) 300)
#define RESET 15
//sensors defines
#define ONWIREPORT 10
SoftwareSerial ATSerial(RXpin,TXpin);    // Declare a virtual serial port
char buffer[]= "72616B776972656C657373";

bool InitLoRaWAN(void);
RAK811 RAKLoRa(ATSerial,DebugSerial);
// Setup the one wire connection on pin 10
Adafruit_BME280 bme280;

void inline sensorsInit() {
	bme280.begin();
}

void setup() {
  DebugSerial.begin(115200);
  sensorsInit();
  while(DebugSerial.available())
  {
    DebugSerial.read(); 
  }
  
  ATSerial.begin(9600); //set ATSerial baudrate:This baud rate has to be consistent with  the baud rate of the WisNode device.
  while(ATSerial.available())
  {
    ATSerial.read(); 
  }

  if(!RAKLoRa.rk_setWorkingMode(0))  //set WisNode work_mode to LoRaWAN.
  {
    DebugSerial.println(F("set work_mode failed, please reset module."));
    while(1);
  }
  
  RAKLoRa.rk_getVersion();  //get RAK811 firmware version
  DebugSerial.println(RAKLoRa.rk_recvData());  //print version number

  DebugSerial.println(F("Start init RAK811 parameters..."));
 
  if (!InitLoRaWAN())  //init LoRaWAN
  {
    DebugSerial.println(F("Init error,please reset module.")); 
    while(1);
  }

  DebugSerial.println(F("Start to join LoRaWAN..."));
  while(!RAKLoRa.rk_joinLoRaNetwork(60))  //Joining LoRaNetwork timeout 60s
  {
    DebugSerial.println();
    DebugSerial.println(F("Rejoin again after 5s..."));
    delay(5000);
  }
  DebugSerial.println(F("Join LoRaWAN success"));

  if(!RAKLoRa.rk_isConfirm(0))  //set LoRa data send package type:0->unconfirm, 1->confirm
  {
    DebugSerial.println(F("LoRa data send package set error,please reset module.")); 
    while(1);    
  }
}

bool InitLoRaWAN(void)
{
  if(RAKLoRa.rk_setJoinMode(JOIN_MODE))  //set join_mode:OTAA
  {
    if(RAKLoRa.rk_setRegion(5))  //set region EU868
    {
      if (RAKLoRa.rk_initOTAA(DevEui, AppEui, AppKey))
      {
        DebugSerial.println(F("RAK811 init OK!"));  
        return true;    
      }
    }
  }
  return false;
}

bool readSensorsAndTx() {
	// Split both words (16 bits) into 2 bytes of 8
	char payload[6];

	Serial.print("Requesting data...");
	// Get temperature event and print its value.
	
	uint16_t temperature = bme280.readTemperature();
	if (temperature) {
		payload[0] = highByte(temperature);
		payload[1] = lowByte(temperature);
	} else {
		Serial.println("Error reading temperature!");
	}

	// Get pressure event and print its value.
	uint16_t pressure = bme280.readPressure() / 100.0F;
	if (pressure) {
		payload[2] = highByte(pressure);
		payload[3] = lowByte(pressure);
	} else {
		Serial.println("Error reading pressure!");
	}

	// Get humidity and print its value.
	uint16_t humidity = bme280.readHumidity();
	if (humidity) {
		payload[4] = highByte(humidity);
		payload[5] = lowByte(humidity);
	} else {
		Serial.println("Error reading humidity!");
	}
	Serial.print("Temperature: ");
	Serial.println(temperature);
	
	Serial.println(F("Packet queued"));
  
	return RAKLoRa.rk_sendData(1, payload);
}

void loop() {
  DebugSerial.println(F("Start send data..."));
  if (readSensorsAndTx())
  {    
    for (unsigned long start = millis(); millis() - start < 90000L;)
    {
      String ret = RAKLoRa.rk_recvData();
      if(ret != NULL)
      { 
        DebugSerial.println(ret);
      }
      if((ret.indexOf("OK")>0)||(ret.indexOf("ERROR")>0))
      {
        DebugSerial.println(F("Go to Sleep."));
        RAKLoRa.rk_sleep(1);  //Set RAK811 enter sleep mode
        delay(TX_INTERVAL * 1000);  //delay 10s
        RAKLoRa.rk_sleep(0);  //Wakeup RAK811 from sleep mode
        break;
      }
    }
  }
}
```
**Sitografia:**
- https://randomnerdtutorials.com/esp32-mqtt-publish-bme280-arduino/
- https://learn.adafruit.com/i2c-addresses


>[Torna a gateway LoraWan con modem HW](gatewayi2crak811hw.md)
