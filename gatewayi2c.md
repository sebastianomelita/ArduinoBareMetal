>[Torna a gateway BUS](gateway.md)

## **Gateway per BUS I2C (TwoWire)**

### **Schema di principio del BUS I2C**

Notare i **collegamenti in parallelo** tra tutti i dispositivi attorno a 3 fili (2 fili escluso massa).

 <img src="I2C-communication-protocol-ESP32.png" alt="alt text" width="600">

### **Il sensore di movimento MPU6050**

<img src="MPU6050-Module-Accelerometer-Gyroscope-Temperature-Sensor.jpg" alt="alt text" width="400">

### **Esempio di cablaggio**
<img src="MPU6050_ESP32_Wiring-Schematic-Diagram.png" alt="alt text" width="600">

Dal **punto di vista SW** sevono **due librerie** da scaricare dentro la solita cartella **libraries**:
- **mpu6050-master.zip** da scompattare e rinominare semplicemente **mpu6050**. Per installare il protocollo standard I2C e il driver del sensore MPU6050.

### **Gateway I2C-MQTT per la lettura di un solo sensore**

La libreria MQTT è asincrona per cui non bloccante. E' adoperabile sia per **ESP8266** che per **ESP32**.


>[Torna a gateway BUS](gateway.md)
