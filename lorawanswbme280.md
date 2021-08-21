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


```
**Sitografia:**
- https://randomnerdtutorials.com/esp32-mqtt-publish-bme280-arduino/
- https://learn.adafruit.com/i2c-addresses


>[Torna a gateway LoraWan con modemSW](lorawan.md)
