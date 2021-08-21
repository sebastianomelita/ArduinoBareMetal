>[Torna a gateway LoraWan con modem SW](lorawan.md)

## **Gateway per BME280**

### **Sensore di temperatura, umidità e pressione BME280**

<img src="BME280.jpg" alt="alt text" width="400">

### **Esempio di cablaggio**
<img src="ESP32-bme280_schematic.jpg" alt="alt text" width="600">



### **Librerie del progetto**

Dal **punto di vista SW** seve **4 librerie** da scaricare dentro la solita cartella **libraries**:
- **adafruit bme280**. Si scarica da https://github.com/adafruit/Adafruit_BME280_Library come Adafruit_BME280_Library-master.zip da scompattare e rinominare semplicemente come **Adafruit_BME280_Library**
- **Adafruit Unified Sensor**. Si scarica da https://github.com/adafruit/Adafruit_Sensor come Adafruit_Sensor-master.zip da scompattare e rinominare semplicemente come **Adafruit_Sensor**

### **Gateway I2C-BME280 con modem SW**

```C++
/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 * Copyright (c) 2018 Terry Moore, MCCI
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses OTAA (Over-the-air activation), where where a DevEUI and
 * application key is configured, which are used in an over-the-air
 * activation procedure where a DevAddr and session keys are
 * assigned/generated for use with all further communication.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!

 * To use this sketch, first register your application and device with
 * the things network, to set or generate an AppEUI, DevEUI and AppKey.
 * Multiple devices can use the same AppEUI, but each device has its own
 * DevEUI and AppKey.
 *
 * Do not forget to define the radio type correctly in
 * arduino-lmic/project_config/lmic_project_config.h or from your BOARDS.txt.
 *
 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

// include the DS18B20 Sensor Library
#include <Adafruit_BME280.h>

//
// For normal use, we require that you edit the sketch to replace FILLMEIN
// with values assigned by the TTN console. However, for regression tests,
// we want to be able to compile these scripts. The regression tests define
// COMPILE_REGRESSION_TEST, and in that case we define FILLMEIN to a non-
// working but innocuous value.
//
#define FILLMEIN 1
/*
#ifdef COMPILE_REGRESSION_TEST
#define FILLMEIN 0
#else
#warning "You must replace the values marked FILLMEIN with real values from the TTN control panel!"
#define FILLMEIN (#dont edit this, edit the lines that use FILLMEIN)
#endif
*/
// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8]={ FILLMEIN };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]={ FILLMEIN };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { FILLMEIN };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

// payload to send to TTN gateway
static uint8_t payload[5];
static osjob_t sendjob;
bool flag_TXCOMPLETE = false;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

// Pin mapping
const lmic_pinmap lmic_pins = {
    .nss = 6,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 5,
    .dio = {2, 3, 4},
};

Adafruit_BME280 bme280;

void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("AppSKey: ");
              for (size_t i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                  Serial.print("-");
                printHex2(artKey[i]);
              }
              Serial.println("");
              Serial.print("NwkSKey: ");
              for (size_t i=0; i<sizeof(nwkKey); ++i) {
                      if (i != 0)
                              Serial.print("-");
                      printHex2(nwkKey[i]);
              }
              Serial.println();
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
	        // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
	        flag_TXCOMPLETE = true;
            break;
	case EV_LINK_DEAD:
            initLoRaWAN();
	    break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}

void do_send(osjob_t* j){
		// Check if there is not a current TX/RX job running
		if (LMIC.opmode & OP_TXRXPEND) {
			Serial.println(F("OP_TXRXPEND, not sending"));
		} else {
			// Split both words (16 bits) into 2 bytes of 8
		byte payload[6];
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
        // prepare upstream data transmission at the next possible time.
        // transmit on port 1 (the first parameter); you can use any value from 1 to 223 (others are reserved).
        // don't request an ack (the last parameter, if not zero, requests an ack from the network).
        // Remember, acks consume a lot of network resources; don't ask for an ack unless you really need it.
        LMIC_setTxData2(1, payload, sizeof(payload)-1, 0);
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event.
}

void initLoRaWAN() {
	// LMIC init
	os_init();

	// Reset the MAC state. Session and pending data transfers will be discarded.
	LMIC_reset();

	// by joining the network, precomputed session parameters are be provided.
	//LMIC_setSession(0x1, DevAddr, (uint8_t*)NwkSkey, (uint8_t*)AppSkey);

	// Enabled data rate adaptation
	LMIC_setAdrMode(1);

	// Enable link check validation
	LMIC_setLinkCheckMode(0);

	// Set data rate and transmit power
	LMIC_setDrTxpow(DR_SF7, 21);
}

void sensorInit(){
	bme280.begin();
}

void setup() {
    Serial.begin(9600);
    Serial.println(F("Starting"));

    #ifdef VCC_ENABLE
    // For Pinoccio Scout boards
    pinMode(VCC_ENABLE, OUTPUT);
    digitalWrite(VCC_ENABLE, HIGH);
    delay(1000);
    #endif

   // Setup LoRaWAN state
	initLoRaWAN();
	
	sensorInit();

    // Start job (sending automatically starts OTAA too)
    do_send(&sendjob);
}

void loop() {
	os_runloop_once();
	/* In caso di instabilità
	//Run LMIC loop until he as finish
	while(flag_TXCOMPLETE == false)
	{
		os_runloop_once();
	}
	flag_TXCOMPLETE = false;
	*/
}
```
**Sitografia:**
- https://randomnerdtutorials.com/esp32-mqtt-publish-bme280-arduino/
- https://learn.adafruit.com/i2c-addresses


>[Torna a gateway LoraWan con modemSW](lorawan.md)
