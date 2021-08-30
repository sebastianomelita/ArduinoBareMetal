>[Torna a gateway LoraWan con modem RAK811](gatewayi2crak811hw.md)

## **Gateway per BUS I2C (TwoWire)**

### **Schema di principio del BUS I2C**

Notare i **collegamenti in parallelo** tra tutti i dispositivi attorno a 3 fili (2 fili escluso massa).

 <img src="I2C-communication-protocol-ESP32.png" alt="alt text" width="600">

### **Il sensore di movimento MPU6050**

<img src="MPU6050-Module-Accelerometer-Gyroscope-Temperature-Sensor.jpg" alt="alt text" width="400">

### **Esempio di cablaggio**
<img src="MPU6050_ESP32_Wiring-Schematic-Diagram.png" alt="alt text" width="600">

### **Librerie del progetto**

Dal **punto di vista SW** seve **una libreria** da scaricare dentro la solita cartella **libraries** (disponibile su Git al link https://github.com/ElectronicCats/mpu6050) :
- **mpu6050-master.zip** da scompattare e rinominare semplicemente **mpu6050**. Per installare il protocollo standard I2C e il driver del sensore MPU6050.

### **Gateway MPU6050-LoRaWan con modem RN2483**

La libreria MQTT è asincrona per cui non bloccante. E' adoperabile sia per **ESP8266** che per **ESP32**.

```C++
/*
 * Author: JP Meijers
 * Date: 2016-10-20
 *
 * Transmit a one byte packet via TTN. This happens as fast as possible, while still keeping to
 * the 1% duty cycle rules enforced by the RN2483's built in LoRaWAN stack. Even though this is
 * allowed by the radio regulations of the 868MHz band, the fair use policy of TTN may prohibit this.
 *
 * CHECK THE RULES BEFORE USING THIS PROGRAM!
 *
 * CHANGE ADDRESS!
 * Change the device address, network (session) key, and app (session) key to the values
 * that are registered via the TTN dashboard.
 * The appropriate line is "myLora.initABP(XXX);" or "myLora.initOTAA(XXX);"
 * When using ABP, it is advised to enable "relax frame count".
 *
 * Connect the RN2xx3 as follows:
 * RN2xx3 -- ESP8266
 * Uart TX -- GPIO4
 * Uart RX -- GPIO5
 * Reset -- GPIO15
 * Vcc -- 3.3V
 * Gnd -- Gnd
 *
 */
#include <rn2xx3.h>
#include <SoftwareSerial.h>
#include <I2Cdev.h>
#include <Wire.h>
#include <MPU6050.h>

#define RESET 15
//sensors defines
SoftwareSerial mySerial(4, 5); // RX, TX !! labels on relay board is swapped !!
//create an instance of the rn2xx3 library,
//giving the software UART as stream to use,
//and using LoRa WAN
rn2xx3 myLora(mySerial);
// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;

void set_sensor_offset() {
  // use the code below to change accel/gyro offset values
  Serial.println("Updating internal sensor offsets...");
    // -76  -2359 1688  0 0 0
    Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
    Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
    Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
    Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
    Serial.print("\n");
    accelgyro.setXGyroOffset(220);
    accelgyro.setYGyroOffset(76);
    accelgyro.setZGyroOffset(-85);
    Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
    Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
    Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
    Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
    Serial.print("\n");
}

void inline sensorsInit() {
	// join I2C bus (I2Cdev library doesn't do this automatically)
	Wire.begin();
	// initialize device
	delay(2000);
	Serial.println("Initializing I2C devices...");
	accelgyro.initialize();
	delay(2000);
	// verify connection
	Serial.println("Testing device connections...");
	Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");
	delay(500);
	//Set accel/gyro offsets
	set_sensor_offset();
}

void inline readSensorsAndTx() {
	// Split both words (16 bits) into 2 bytes of 8
	byte payload[12];
	
	accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
	
	payload[0] = highByte(ax);
	payload[1] = lowByte(ax);
	payload[2] = highByte(ay);
	payload[3] = lowByte(ay);
	payload[4] = highByte(az);
	payload[5] = lowByte(az);
	payload[6] = highByte(gx);
	payload[7] = lowByte(gx);
	payload[8] = highByte(gy);
	payload[9] = lowByte(gy);
	payload[10] = highByte(gz);
	payload[11] = lowByte(gz);
	
	//myLora.tx("!"); //send String, blocking function
	myLora.txBytes(payload, sizeof(payload)); // blocking function
}

// the setup routine runs once when you press reset:
void setup() {
  // LED pin is GPIO2 which is the ESP8266's built in LED
  pinMode(2, OUTPUT);
  led_on();

  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  mySerial.begin(57600);
  
  sensorsInit();
  
  delay(1000); //wait for the arduino ide's serial console to open

  Serial.println("Startup");

  initialize_radio();

  //transmit a startup message
  myLora.tx("TTN Mapper on ESP8266 node");

  led_off();

  delay(2000);
}

void initialize_radio()
{
  //reset RN2xx3
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, LOW);
  delay(100);
  digitalWrite(RESET, HIGH);

  delay(100); //wait for the RN2xx3's startup message
  mySerial.flush();

  //check communication with radio
  String hweui = myLora.hweui();
  while(hweui.length() != 16)
  {
    Serial.println("Communication with RN2xx3 unsuccessful. Power cycle the board.");
    Serial.println(hweui);
    delay(10000);
    hweui = myLora.hweui();
  }

  //print out the HWEUI so that we can register it via ttnctl
  Serial.println("When using OTAA, register this DevEUI: ");
  Serial.println(hweui);
  Serial.println("RN2xx3 firmware version:");
  Serial.println(myLora.sysver());

  //configure your keys and join the network
  Serial.println("Trying to join TTN");
  bool join_result = false;

  //ABP: initABP(String addr, String AppSKey, String NwkSKey);
  join_result = myLora.initABP("02017201", "8D7FFEF938589D95AAD928C2E2E7E48F", "AE17E567AECC8787F749A62F5541D522");

  //OTAA: initOTAA(String AppEUI, String AppKey);
  //join_result = myLora.initOTAA("70B3D57ED00001A6", "A23C96EE13804963F8C2BD6285448198");

  while(!join_result)
  {
    Serial.println("Unable to join. Are your keys correct, and do you have TTN coverage?");
    delay(60000); //delay a minute before retry
    join_result = myLora.init();
  }
  Serial.println("Successfully joined TTN");

}

// the loop routine runs over and over again forever:
void loop() {
    led_on();
    
    readSensorsAndTx();
	
    led_off();
    delay(200);
}

void led_on()
{
  digitalWrite(2, 1);
}

void led_off()
{
  digitalWrite(2, 0);
}
```

**Sitografia:**
- https://randomnerdtutorials.com/esp32-mpu-6050-web-server/


>[Torna a gateway LoraWan con modem RAK811](gatewayi2crak811hw.md)
