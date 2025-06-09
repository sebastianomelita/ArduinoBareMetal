>[Torna all'indice](indexpulsanti.md)

## **JOYSTICK ANALOGICO**

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
#define VERT_PIN A3
#define HORZ_PIN A0
#define SEL_PIN  2

void setup() {
  Serial.begin(115200);
  pinMode(VERT_PIN, INPUT);
  pinMode(HORZ_PIN, INPUT);
}


void loop() {
  int x = analogRead(VERT_PIN);
  int y = analogRead(HORZ_PIN);
  
  Serial.print("x: ");Serial.print(x); Serial.print(" y: ");Serial.println(y);
  delay(500);
}
```
Simulazione di una MCU ESP32 con Wokwi: https://wokwi.com/projects/391096564707796993

### *PROBLEMA DELLA MISURA CENTRALE*

Nelle applicazioni pratiche il valore centrale spesso non è quello che ci si aspetta, cioè esattamente il valore centrale di 1024 nel caso di un ADC a 12 bit, per via di errori di misura dovuti alle non linearità della perifrerica ADC o imprecisione costruttiva del joistick. Una soluzione potrebbe essere la calibrazione del joistick prima dell'uso. Un'alra soluzione potrebbe essere l'introduzione di una isteresi sui valori centrali, detta anche zona morta, in cui la lettura del joistick restituisce sempre lo stesso valore centrale in corrispondenza di un range di valori effettivamente misurati. In quest'ultima soluzione, viene scartata la misura dei valori troppo vicini alla zona di incertezza.

```C++
#define VERT_PIN A3
#define HORZ_PIN A0
#define SEL_PIN  2

void setup() {
  Serial.begin(115200);
  pinMode(VERT_PIN, INPUT);
  pinMode(HORZ_PIN, INPUT);
}

int isteresi(int value)
{
  if (value >= 2200)
  {
    value = map(value, 2200, 4095, 127, 255);
  }
  else if (value <= 1800)
  {
    value = map(value, 1800, 0, 127, 0);  
  }
  else
  {
    value = 127;
  }
  
  return value;
}

void loop() {
  int vert = analogRead(VERT_PIN);
  int horz = analogRead(HORZ_PIN);

  int x = isteresi(horz);
  int y = isteresi(vert);
  
  Serial.print("x: ");Serial.print(x); Serial.print(" y: ");Serial.println(y);
  delay(500);
}
```
Il mappaggio dell'intervallo [0 2048] -> [0 127] serve per collegare, a valle del joistick, una periferica di attuazione come, ad esempio, un modulatore PWM da utilizzare, per esempio, per comandare un motore in CC.

Simulazione di una MCU ESP32 con Wokwi: https://wokwi.com/projects/391091401329352705



Sitografia:
- https://docs.wokwi.com/parts/wokwi-analog-joystick
- https://www.upesy.com/blogs/tutorials/measure-voltage-on-esp32-with-adc-with-arduino-code#ce375cca55443a8e7b5d517d45a47d
- https://www.upesy.com/blogs/tutorials/esp32-pinout-reference-gpio-pins-ultimate-guide
- https://github.com/un0038998/ESPNOW_Car/blob/main/ESPNOW_Car_Transmitter/ESPNOW_Car_Transmitter.ino
- https://docs.sunfounder.com/projects/esp32-starter-kit/en/latest/micropython/basic_projects/py_joystick.html


>[Torna all'indice](indexpulsanti.md)
