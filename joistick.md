>[Torna all'indice](indexpulsanti.md)

## **JOISTICK ANALOGICO**

<img src="img\A85262_Joystick-Module_3.webp" alt="alt text" width="300">

Pins:
- **Vcc.** Alimentazione positiva
- **VERT.**  Uscita verticale analogica
- **HORZ.**  Uscita orizzontale analogica
- **SEL.** 	Pulsante
- **GND.**	Massa

Il joistick è realizzando combinando, su due assi separati ed ortogonali, due potenziometri rotativi che realizzano due partitori di tensione. L'ingresso dei partitori è polarizzato con l'alimentazione positiva Vcc, per cui l'uscita restituisce per entrambi una tensione compresa tra Vcc e 0V, a seconda dell'ammontare della rotazione. 

La tensione in uscita al partitore rappresenta la quantità della rotazione e deve misurata dal microcontrollore tramite una periferica **ADC** di conversione del segnale da analogico a digitale. L'ADC è collegato di base a tutte le porte analogiche di un microcontrollore. 


<img src="img\joyrange.png" alt="alt text" width="900">

La **quantizzazione** della conversione per la maggior parte dell MCU, come Arduino o ESP32, è a 10 bit, circostanza che limita la risoluzione della misura a 1024 **livelli di tensione**, e quindi saranno dello stesso numero anche i diversi campioni di rotazione misurabili con una escursione da un estremo all'altro di uno dei due assi del joistick.


Anche se sembra strano, l'ADC a 10 bit di Arduino (1024 valori) è più preciso e affidabile di quello a 12 bit dell'ESP32 (4096 valori). La quantizzazzione nella MCU ESP32 è sensibilmente non lineare, soprattutto in prossimità dei valori estremi. In sostanza, ciò significa che l'ESP32 non è in grado di distinguere un segnale di 3,2 V e 3,3 V: il valore misurato sarà lo stesso (4095). Allo stesso modo, l'ESP32 non distinguerà tra un segnale 0 V e 0,2 V per piccole tensioni. Si può provare a calibrare l'ADC per ridurre le non linearità attarverso un mappaggio SW, come decritto [quì](https://github.com/e-tinkers/esp32-adc-calibrate).

<img src="img\ESP32 Joystick Interfacing.webp" alt="alt text" width="500">

```C++
#define VERT_PIN A0
#define HORZ_PIN A1
#define SEL_PIN  2

void setup() {
  pinMode(VERT_PIN, INPUT);
  pinMode(HORZ_PIN, INPUT);
  pinMode(SEL_PIN, INPUT_PULLUP);
}

void loop() {
  int vert = analogRead(VERT_PIN);
  int horz = analogRead(HORZ_PIN);
  bool selPressed = digitalRead(SEL_PIN) == LOW;
  // horz goes from 0 (right) to 1023 (left)
  // vert goes from 0 (bottom) to 1023 (top)
  // selPressed is true is the joystick is pressed
}
```



Sitografia:
- https://docs.wokwi.com/parts/wokwi-analog-joystick


>[Torna all'indice](indexpulsanti.md)
