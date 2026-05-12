# Dispensa: dal firmware di un sensore MQTT all'architettura di una MCU

## Indice

1. [Premessa: MCU vs SBC](#premessa-mcu-vs-sbc)
2. [Fasi principali del firmware di un sensore MQTT](#fasi-principali-del-firmware-di-un-sensore-mqtt)
3. [Schema a blocchi di una MCU](#schema-a-blocchi-di-una-mcu)
4. [Mapping fasi firmware → blocchi MCU](#mapping-fasi-firmware--blocchi-mcu)
5. [Schema a blocchi di un SBC](#schema-a-blocchi-di-un-sbc)
6. [Confronto MCU vs SBC](#confronto-mcu-vs-sbc)
7. [Considerazioni progettuali](#considerazioni-progettuali)
8. [Riferimenti](#riferimenti)

---

## Premessa: MCU vs SBC

Prima di analizzare il firmware è importante chiarire una distinzione che spesso genera confusione.

Un **microcontrollore (MCU)** è un chip singolo che integra al suo interno CPU, memorie (Flash e RAM) e periferiche di I/O. Esempi tipici sono ATmega328 (Arduino UNO), ESP32, STM32. Si programma "bare metal", cioè senza un sistema operativo sottostante: il programma utente gira direttamente sull'hardware tramite un `loop()` infinito.

Un **Single Board Computer (SBC)** è invece una scheda che integra un microprocessore completo, memoria RAM esterna, storage esterno (tipicamente microSD) e periferiche. Esempi: Raspberry Pi, BeagleBone. Su un SBC gira un sistema operativo completo (Linux), e i programmi sono processi gestiti dallo scheduler del kernel.

Il documento di riferimento `sensorfw.md` mostra due implementazioni dello stesso firmware: una in Python (più adatta a un SBC) e una in C++ con `WiFi.h` e `PubSubClient` (schiettamente per MCU come ESP32). In questa dispensa ci concentriamo sulla seconda perché evidenzia meglio il rapporto diretto tra software e hardware.

---

## Fasi principali del firmware di un sensore MQTT

Il firmware di un nodo sensore che pubblica dati su un broker MQTT si articola in cinque fasi, schematizzate qui sotto.

<svg width="100%" viewBox="0 0 680 720" xmlns="http://www.w3.org/2000/svg">
<defs>
<marker id="arrow1" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
<path d="M2 1L8 5L2 9" fill="none" stroke="context-stroke" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/>
</marker>
<style>
.t { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 14px; fill: #2C2C2A; }
.ts { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 12px; fill: #5F5E5A; }
.th { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 14px; font-weight: 500; fill: #2C2C2A; }
.arr { stroke: #888780; stroke-width: 1.5; fill: none; }
</style>
</defs>

<rect x="240" y="30" width="200" height="44" rx="22" fill="#F1EFE8" stroke="#5F5E5A" stroke-width="0.5"/>
<text class="th" x="340" y="52" text-anchor="middle" dominant-baseline="central" fill="#2C2C2A">Avvio firmware</text>
<line x1="340" y1="74" x2="340" y2="100" class="arr" marker-end="url(#arrow1)"/>

<rect x="200" y="100" width="280" height="56" rx="8" fill="#EEEDFE" stroke="#534AB7" stroke-width="0.5"/>
<text class="th" x="340" y="120" text-anchor="middle" dominant-baseline="central" fill="#3C3489">1. Inizializzazione parametri</text>
<text class="ts" x="340" y="140" text-anchor="middle" dominant-baseline="central" fill="#534AB7">Broker, porta, credenziali, pin sensore</text>
<line x1="340" y1="156" x2="340" y2="180" class="arr" marker-end="url(#arrow1)"/>

<rect x="200" y="180" width="280" height="56" rx="8" fill="#E1F5EE" stroke="#0F6E56" stroke-width="0.5"/>
<text class="th" x="340" y="200" text-anchor="middle" dominant-baseline="central" fill="#085041">2. Connessione al broker MQTT</text>
<text class="ts" x="340" y="220" text-anchor="middle" dominant-baseline="central" fill="#0F6E56">WiFi + handshake con il broker</text>
<line x1="340" y1="236" x2="340" y2="260" class="arr" marker-end="url(#arrow1)"/>

<rect x="200" y="260" width="280" height="56" rx="8" fill="#E1F5EE" stroke="#0F6E56" stroke-width="0.5"/>
<text class="th" x="340" y="280" text-anchor="middle" dominant-baseline="central" fill="#085041">3. Inizializzazione sensore</text>
<text class="ts" x="340" y="300" text-anchor="middle" dominant-baseline="central" fill="#0F6E56">Configura pin di lettura temperatura</text>
<line x1="340" y1="316" x2="340" y2="346" class="arr" marker-end="url(#arrow1)"/>

<rect x="80" y="346" width="520" height="240" rx="14" fill="#FAECE7" stroke="#993C1D" stroke-width="0.5"/>
<text class="th" x="340" y="368" text-anchor="middle" dominant-baseline="central" fill="#712B13">4. Ciclo principale</text>

<rect x="110" y="390" width="200" height="44" rx="8" fill="#FAEEDA" stroke="#854F0B" stroke-width="0.5"/>
<text class="ts" x="210" y="412" text-anchor="middle" dominant-baseline="central" fill="#633806">Leggi tempo corrente</text>
<line x1="310" y1="412" x2="370" y2="412" class="arr" marker-end="url(#arrow1)"/>

<rect x="370" y="390" width="200" height="44" rx="8" fill="#FAEEDA" stroke="#854F0B" stroke-width="0.5"/>
<text class="ts" x="470" y="412" text-anchor="middle" dominant-baseline="central" fill="#633806">Leggi temperatura</text>

<path d="M470 434 L470 450 L210 450 L210 466" stroke="#854F0B" stroke-width="0.5" fill="none" marker-end="url(#arrow1)"/>

<rect x="110" y="466" width="200" height="44" rx="8" fill="#FAEEDA" stroke="#854F0B" stroke-width="0.5"/>
<text class="ts" x="210" y="488" text-anchor="middle" dominant-baseline="central" fill="#633806">Componi messaggio JSON</text>
<line x1="310" y1="488" x2="370" y2="488" class="arr" marker-end="url(#arrow1)"/>

<rect x="370" y="466" width="200" height="44" rx="8" fill="#FAEEDA" stroke="#854F0B" stroke-width="0.5"/>
<text class="ts" x="470" y="488" text-anchor="middle" dominant-baseline="central" fill="#633806">Pubblica se è ora</text>

<path d="M470 510 L470 526 L210 526 L210 540" stroke="#854F0B" stroke-width="0.5" fill="none" marker-end="url(#arrow1)"/>

<rect x="110" y="540" width="460" height="36" rx="8" fill="#FAEEDA" stroke="#854F0B" stroke-width="0.5"/>
<text class="ts" x="340" y="558" text-anchor="middle" dominant-baseline="central" fill="#633806">Aggiorna timestamp ultimo invio</text>

<line x1="340" y1="586" x2="340" y2="610" class="arr" marker-end="url(#arrow1)"/>

<rect x="200" y="610" width="280" height="56" rx="8" fill="#E6F1FB" stroke="#185FA5" stroke-width="0.5"/>
<text class="th" x="340" y="630" text-anchor="middle" dominant-baseline="central" fill="#0C447C">5. Attesa</text>
<text class="ts" x="340" y="650" text-anchor="middle" dominant-baseline="central" fill="#185FA5">Breve delay prima della prossima iterazione</text>

<path d="M200 638 L40 638 L40 412 L110 412" stroke="#185FA5" stroke-width="0.5" fill="none" marker-end="url(#arrow1)"/>
<text class="ts" x="48" y="525" text-anchor="start" fill="#185FA5">loop</text>
</svg>


### 1. Inizializzazione dei parametri di connessione

Si configurano i parametri necessari al funzionamento: indirizzo e porta del broker MQTT, username e password, pin del sensore di temperatura, intervallo di lettura. In C++ sono tipicamente costanti `const char*` o `#define`; in Python sono variabili a livello modulo.

```cpp
const char* mqtt_server = "broker_address";
const int mqtt_port = 1883;
const long interval = 60000;  // millisecondi
```

### 2. Connessione al broker MQTT

Si stabilisce prima la connessione di rete (su ESP32 tipicamente WiFi) e poi la connessione MQTT vera e propria, che include l'handshake del protocollo con il broker.

```cpp
WiFi.begin(ssid, password);
// ... attesa connessione WiFi ...
client.setServer(mqtt_server, mqtt_port);
client.connect("ArduinoClient", mqtt_user, mqtt_password);
```

### 3. Inizializzazione del sensore di temperatura

Si configura il pin o l'interfaccia di comunicazione con il sensore. Nel caso più semplice è una `pinMode()`; con sensori digitali come BME280 o DS18B20 servono librerie dedicate che gestiscono il protocollo specifico (I2C, SPI, 1-Wire).

### 4. Ciclo principale

È il cuore del firmware, eseguito ripetutamente nel `loop()`:

- ottenere il tempo corrente con `millis()`
- leggere il valore della temperatura
- comporre un messaggio JSON con sensor_id e valore
- pubblicare il messaggio al broker se è trascorso l'intervallo
- aggiornare il timestamp dell'ultimo invio

```cpp
unsigned long currentTime = millis();
if (currentTime - lastSentTime >= interval) {
    float temperature = read_temperature();
    snprintf(message, sizeof(message),
        "{\"sensor_id\": \"%s\", \"temperature\": %.2f}",
        sensor_id, temperature);
    client.publish(mqtt_topic, message);
    lastSentTime = currentTime;
}
```

Il pattern `currentTime - lastSentTime >= interval` è preferibile a un `delay(interval)` perché lascia la CPU libera di gestire altri compiti (come il `client.loop()` di MQTT che mantiene viva la connessione).

### 5. Attesa prima della prossima iterazione

Un breve `delay()` (tipicamente 1 secondo) o, nelle implementazioni a basso consumo, una modalità sleep più o meno profonda.

---

## Schema a blocchi di una MCU

Una MCU tipica contiene questi blocchi funzionali, tutti integrati nello stesso chip.

<svg width="100%" viewBox="0 0 680 620" xmlns="http://www.w3.org/2000/svg">
<defs>
<marker id="arrow2" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
<path d="M2 1L8 5L2 9" fill="none" stroke="context-stroke" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/>
</marker>
<style>
.t2 { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 14px; }
.ts2 { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 12px; }
.th2 { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 14px; font-weight: 500; }
.arr2 { stroke: #888780; stroke-width: 1.5; fill: none; }
</style>
</defs>

<rect x="30" y="30" width="620" height="500" rx="14" fill="none" stroke="#888780" stroke-width="1" stroke-dasharray="4 4" opacity="0.6"/>
<text class="ts2" x="50" y="50" text-anchor="start" fill="#5F5E5A">Microcontrollore (chip singolo)</text>

<rect x="60" y="80" width="180" height="80" rx="8" fill="#EEEDFE" stroke="#534AB7" stroke-width="0.5"/>
<text class="th2" x="150" y="110" text-anchor="middle" dominant-baseline="central" fill="#3C3489">CPU</text>
<text class="ts2" x="150" y="132" text-anchor="middle" dominant-baseline="central" fill="#534AB7">Core, ALU, registri</text>

<rect x="260" y="80" width="180" height="80" rx="8" fill="#E1F5EE" stroke="#0F6E56" stroke-width="0.5"/>
<text class="th2" x="350" y="110" text-anchor="middle" dominant-baseline="central" fill="#085041">Flash</text>
<text class="ts2" x="350" y="132" text-anchor="middle" dominant-baseline="central" fill="#0F6E56">Memoria programma</text>

<rect x="460" y="80" width="180" height="80" rx="8" fill="#E1F5EE" stroke="#0F6E56" stroke-width="0.5"/>
<text class="th2" x="550" y="110" text-anchor="middle" dominant-baseline="central" fill="#085041">RAM</text>
<text class="ts2" x="550" y="132" text-anchor="middle" dominant-baseline="central" fill="#0F6E56">Memoria dati</text>

<rect x="60" y="200" width="580" height="40" rx="6" fill="#F1EFE8" stroke="#5F5E5A" stroke-width="0.5"/>
<text class="th2" x="350" y="220" text-anchor="middle" dominant-baseline="central" fill="#2C2C2A">Bus interno (dati, indirizzi, controllo)</text>

<line x1="150" y1="160" x2="150" y2="200" class="arr2"/>
<line x1="350" y1="160" x2="350" y2="200" class="arr2"/>
<line x1="550" y1="160" x2="550" y2="200" class="arr2"/>

<line x1="120" y1="240" x2="120" y2="280" class="arr2"/>
<line x1="260" y1="240" x2="260" y2="280" class="arr2"/>
<line x1="400" y1="240" x2="400" y2="280" class="arr2"/>
<line x1="540" y1="240" x2="540" y2="280" class="arr2"/>

<rect x="50" y="280" width="140" height="70" rx="8" fill="#FAECE7" stroke="#993C1D" stroke-width="0.5"/>
<text class="th2" x="120" y="305" text-anchor="middle" dominant-baseline="central" fill="#712B13">GPIO</text>
<text class="ts2" x="120" y="325" text-anchor="middle" dominant-baseline="central" fill="#993C1D">Pin digitali I/O</text>

<rect x="200" y="280" width="120" height="70" rx="8" fill="#FAECE7" stroke="#993C1D" stroke-width="0.5"/>
<text class="th2" x="260" y="305" text-anchor="middle" dominant-baseline="central" fill="#712B13">UART/SPI/I2C</text>
<text class="ts2" x="260" y="325" text-anchor="middle" dominant-baseline="central" fill="#993C1D">Comunicazione seriale</text>

<rect x="340" y="280" width="120" height="70" rx="8" fill="#FAECE7" stroke="#993C1D" stroke-width="0.5"/>
<text class="th2" x="400" y="305" text-anchor="middle" dominant-baseline="central" fill="#712B13">Timer / PWM</text>
<text class="ts2" x="400" y="325" text-anchor="middle" dominant-baseline="central" fill="#993C1D">Conteggi e segnali</text>

<rect x="480" y="280" width="120" height="70" rx="8" fill="#FAECE7" stroke="#993C1D" stroke-width="0.5"/>
<text class="th2" x="540" y="305" text-anchor="middle" dominant-baseline="central" fill="#712B13">ADC / DAC</text>
<text class="ts2" x="540" y="325" text-anchor="middle" dominant-baseline="central" fill="#993C1D">Conversione analogica</text>

<line x1="120" y1="350" x2="120" y2="385" class="arr2" marker-end="url(#arrow2)"/>
<line x1="260" y1="350" x2="260" y2="385" class="arr2" marker-end="url(#arrow2)"/>
<line x1="400" y1="350" x2="400" y2="385" class="arr2" marker-end="url(#arrow2)"/>
<line x1="540" y1="350" x2="540" y2="385" class="arr2" marker-end="url(#arrow2)"/>

<rect x="60" y="385" width="580" height="36" rx="6" fill="#F1EFE8" stroke="#5F5E5A" stroke-width="0.5"/>
<text class="ts2" x="350" y="403" text-anchor="middle" dominant-baseline="central" fill="#2C2C2A">Pin esterni verso sensori, attuatori, altri dispositivi</text>

<rect x="60" y="450" width="170" height="60" rx="8" fill="#FAEEDA" stroke="#854F0B" stroke-width="0.5"/>
<text class="th2" x="145" y="473" text-anchor="middle" dominant-baseline="central" fill="#633806">Oscillatore / Clock</text>
<text class="ts2" x="145" y="493" text-anchor="middle" dominant-baseline="central" fill="#854F0B">Frequenza di sistema</text>

<rect x="255" y="450" width="170" height="60" rx="8" fill="#FAEEDA" stroke="#854F0B" stroke-width="0.5"/>
<text class="th2" x="340" y="473" text-anchor="middle" dominant-baseline="central" fill="#633806">Reset / Power</text>
<text class="ts2" x="340" y="493" text-anchor="middle" dominant-baseline="central" fill="#854F0B">Avvio e alimentazione</text>

<rect x="450" y="450" width="190" height="60" rx="8" fill="#FAEEDA" stroke="#854F0B" stroke-width="0.5"/>
<text class="th2" x="545" y="473" text-anchor="middle" dominant-baseline="central" fill="#633806">Controllore interrupt</text>
<text class="ts2" x="545" y="493" text-anchor="middle" dominant-baseline="central" fill="#854F0B">Gestione eventi asincroni</text>

<line x1="145" y1="450" x2="145" y2="240" stroke="#854F0B" stroke-width="0.5" stroke-dasharray="3 3" fill="none"/>
<line x1="340" y1="450" x2="340" y2="240" stroke="#854F0B" stroke-width="0.5" stroke-dasharray="3 3" fill="none"/>
<line x1="545" y1="450" x2="545" y2="240" stroke="#854F0B" stroke-width="0.5" stroke-dasharray="3 3" fill="none"/>
</svg>


### Blocchi di elaborazione e memoria

| Blocco | Funzione |
|--------|----------|
| **CPU** | Core, ALU, registri. Esegue le istruzioni del programma. |
| **Flash** | Memoria non volatile che contiene il programma (firmware). |
| **RAM** | Memoria volatile per dati, stack, heap, variabili globali. |

Su un PC questi tre elementi sono fisicamente separati e collegati da bus esterni. Su una MCU sono tutti dentro lo stesso package, ed è proprio questa integrazione che rende il chip economico e adatto a sistemi embedded.

### Bus interno

Connette CPU, memorie e periferiche. Trasporta tre tipi di informazione: dati, indirizzi di memoria, segnali di controllo.

### Periferiche di I/O

| Blocco | Funzione |
|--------|----------|
| **GPIO** | Pin digitali general purpose: input o output, alti o bassi. |
| **UART / SPI / I2C** | Protocolli seriali per comunicare con sensori, display, altri chip. |
| **Timer / PWM** | Contatori hardware per misurare il tempo o generare segnali a duty cycle variabile. |
| **ADC / DAC** | Convertitori analogico-digitale e digitale-analogico per leggere o generare tensioni. |

Su MCU moderne come ESP32 si aggiungono blocchi radio integrati (WiFi, Bluetooth) che sono a tutti gli effetti periferiche dedicate.

### Blocchi di supporto

| Blocco | Funzione |
|--------|----------|
| **Oscillatore / Clock** | Genera il segnale di clock che scandisce tutte le operazioni. |
| **Reset / Power** | Gestisce l'avvio del chip e l'alimentazione. |
| **Controllore interrupt** | Permette alle periferiche di interrompere la CPU quando si verifica un evento, evitando il polling continuo. |

---

## Mapping fasi firmware → blocchi MCU

Ciascuna fase del firmware coinvolge blocchi hardware specifici. Comprendere questa corrispondenza è ciò che distingue chi sa "far funzionare" un firmware da chi sa progettarlo.

<svg width="100%" viewBox="0 0 680 760" xmlns="http://www.w3.org/2000/svg">
<defs>
<marker id="arrow3" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
<path d="M2 1L8 5L2 9" fill="none" stroke="context-stroke" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/>
</marker>
<style>
.t3 { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 14px; }
.ts3 { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 12px; }
.th3 { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 14px; font-weight: 500; }
.arr3 { stroke: #888780; stroke-width: 1.5; fill: none; }
</style>
</defs>

<text class="ts3" x="60" y="30" text-anchor="start" fill="#5F5E5A">Fasi firmware</text>
<text class="ts3" x="430" y="30" text-anchor="start" fill="#5F5E5A">Blocchi MCU coinvolti</text>

<rect x="40" y="50" width="240" height="56" rx="8" fill="#EEEDFE" stroke="#534AB7" stroke-width="0.5"/>
<text class="th3" x="160" y="70" text-anchor="middle" dominant-baseline="central" fill="#3C3489">1. Inizializzazione parametri</text>
<text class="ts3" x="160" y="90" text-anchor="middle" dominant-baseline="central" fill="#534AB7">Broker, credenziali, pin</text>

<rect x="410" y="50" width="230" height="56" rx="8" fill="#E1F5EE" stroke="#0F6E56" stroke-width="0.5"/>
<text class="th3" x="525" y="70" text-anchor="middle" dominant-baseline="central" fill="#085041">CPU + Flash + RAM</text>
<text class="ts3" x="525" y="90" text-anchor="middle" dominant-baseline="central" fill="#0F6E56">Carica costanti in variabili</text>
<line x1="280" y1="78" x2="410" y2="78" class="arr3" marker-end="url(#arrow3)"/>

<rect x="40" y="140" width="240" height="56" rx="8" fill="#EEEDFE" stroke="#534AB7" stroke-width="0.5"/>
<text class="th3" x="160" y="160" text-anchor="middle" dominant-baseline="central" fill="#3C3489">2. Connessione al broker</text>
<text class="ts3" x="160" y="180" text-anchor="middle" dominant-baseline="central" fill="#534AB7">WiFi + handshake MQTT</text>

<rect x="410" y="140" width="230" height="56" rx="8" fill="#E1F5EE" stroke="#0F6E56" stroke-width="0.5"/>
<text class="th3" x="525" y="160" text-anchor="middle" dominant-baseline="central" fill="#085041">Radio WiFi + stack TCP/IP</text>
<text class="ts3" x="525" y="180" text-anchor="middle" dominant-baseline="central" fill="#0F6E56">Periferica dedicata + RAM</text>
<line x1="280" y1="168" x2="410" y2="168" class="arr3" marker-end="url(#arrow3)"/>

<rect x="40" y="230" width="240" height="56" rx="8" fill="#EEEDFE" stroke="#534AB7" stroke-width="0.5"/>
<text class="th3" x="160" y="250" text-anchor="middle" dominant-baseline="central" fill="#3C3489">3. Inizializzazione sensore</text>
<text class="ts3" x="160" y="270" text-anchor="middle" dominant-baseline="central" fill="#534AB7">Configura pin di lettura</text>

<rect x="410" y="230" width="230" height="56" rx="8" fill="#E1F5EE" stroke="#0F6E56" stroke-width="0.5"/>
<text class="th3" x="525" y="250" text-anchor="middle" dominant-baseline="central" fill="#085041">GPIO o ADC o I2C/SPI</text>
<text class="ts3" x="525" y="270" text-anchor="middle" dominant-baseline="central" fill="#0F6E56">Dipende dal tipo di sensore</text>
<line x1="280" y1="258" x2="410" y2="258" class="arr3" marker-end="url(#arrow3)"/>

<rect x="40" y="320" width="240" height="280" rx="8" fill="#FAECE7" stroke="#993C1D" stroke-width="0.5"/>
<text class="th3" x="160" y="345" text-anchor="middle" dominant-baseline="central" fill="#712B13">4. Ciclo principale</text>

<rect x="60" y="365" width="200" height="40" rx="6" fill="#FAEEDA" stroke="#854F0B" stroke-width="0.5"/>
<text class="ts3" x="160" y="385" text-anchor="middle" dominant-baseline="central" fill="#633806">millis() / time.time()</text>

<rect x="410" y="365" width="230" height="40" rx="6" fill="#E6F1FB" stroke="#185FA5" stroke-width="0.5"/>
<text class="ts3" x="525" y="385" text-anchor="middle" dominant-baseline="central" fill="#0C447C">Timer di sistema</text>
<line x1="260" y1="385" x2="410" y2="385" class="arr3" marker-end="url(#arrow3)"/>

<rect x="60" y="415" width="200" height="40" rx="6" fill="#FAEEDA" stroke="#854F0B" stroke-width="0.5"/>
<text class="ts3" x="160" y="435" text-anchor="middle" dominant-baseline="central" fill="#633806">Leggi temperatura</text>

<rect x="410" y="415" width="230" height="40" rx="6" fill="#E6F1FB" stroke="#185FA5" stroke-width="0.5"/>
<text class="ts3" x="525" y="435" text-anchor="middle" dominant-baseline="central" fill="#0C447C">ADC o I2C/SPI + GPIO</text>
<line x1="260" y1="435" x2="410" y2="435" class="arr3" marker-end="url(#arrow3)"/>

<rect x="60" y="465" width="200" height="40" rx="6" fill="#FAEEDA" stroke="#854F0B" stroke-width="0.5"/>
<text class="ts3" x="160" y="485" text-anchor="middle" dominant-baseline="central" fill="#633806">Componi JSON</text>

<rect x="410" y="465" width="230" height="40" rx="6" fill="#E6F1FB" stroke="#185FA5" stroke-width="0.5"/>
<text class="ts3" x="525" y="485" text-anchor="middle" dominant-baseline="central" fill="#0C447C">CPU + RAM (buffer stringa)</text>
<line x1="260" y1="485" x2="410" y2="485" class="arr3" marker-end="url(#arrow3)"/>

<rect x="60" y="515" width="200" height="40" rx="6" fill="#FAEEDA" stroke="#854F0B" stroke-width="0.5"/>
<text class="ts3" x="160" y="535" text-anchor="middle" dominant-baseline="central" fill="#633806">Pubblica messaggio</text>

<rect x="410" y="515" width="230" height="40" rx="6" fill="#E6F1FB" stroke="#185FA5" stroke-width="0.5"/>
<text class="ts3" x="525" y="535" text-anchor="middle" dominant-baseline="central" fill="#0C447C">Radio WiFi (TCP/MQTT)</text>
<line x1="260" y1="535" x2="410" y2="535" class="arr3" marker-end="url(#arrow3)"/>

<rect x="60" y="565" width="200" height="30" rx="6" fill="#FAEEDA" stroke="#854F0B" stroke-width="0.5"/>
<text class="ts3" x="160" y="580" text-anchor="middle" dominant-baseline="central" fill="#633806">Aggiorna timestamp</text>

<rect x="410" y="565" width="230" height="30" rx="6" fill="#E6F1FB" stroke="#185FA5" stroke-width="0.5"/>
<text class="ts3" x="525" y="580" text-anchor="middle" dominant-baseline="central" fill="#0C447C">Variabile in RAM</text>
<line x1="260" y1="580" x2="410" y2="580" class="arr3" marker-end="url(#arrow3)"/>

<rect x="40" y="630" width="240" height="56" rx="8" fill="#EEEDFE" stroke="#534AB7" stroke-width="0.5"/>
<text class="th3" x="160" y="650" text-anchor="middle" dominant-baseline="central" fill="#3C3489">5. Attesa</text>
<text class="ts3" x="160" y="670" text-anchor="middle" dominant-baseline="central" fill="#534AB7">delay / sleep</text>

<rect x="410" y="630" width="230" height="56" rx="8" fill="#E1F5EE" stroke="#0F6E56" stroke-width="0.5"/>
<text class="th3" x="525" y="650" text-anchor="middle" dominant-baseline="central" fill="#085041">Timer + interrupt</text>
<text class="ts3" x="525" y="670" text-anchor="middle" dominant-baseline="central" fill="#0F6E56">Eventualmente deep sleep</text>
<line x1="280" y1="658" x2="410" y2="658" class="arr3" marker-end="url(#arrow3)"/>

<rect x="40" y="710" width="12" height="12" rx="2" fill="#E1F5EE" stroke="#0F6E56" stroke-width="0.5"/>
<text class="ts3" x="58" y="716" dominant-baseline="central" fill="#5F5E5A">Blocchi usati una volta (setup)</text>
<rect x="280" y="710" width="12" height="12" rx="2" fill="#E6F1FB" stroke="#185FA5" stroke-width="0.5"/>
<text class="ts3" x="298" y="716" dominant-baseline="central" fill="#5F5E5A">Blocchi usati a ogni iterazione</text>
</svg>


### Fase 1 — Inizializzazione parametri

**Blocchi coinvolti:** CPU, Flash, RAM.

Le costanti scritte nel codice sorgente finiscono in Flash al momento della programmazione. All'avvio, la CPU le copia (o vi accede direttamente, a seconda dell'architettura) e le rende disponibili come variabili in RAM.

### Fase 2 — Connessione al broker

**Blocchi coinvolti:** periferica radio WiFi, stack TCP/IP, RAM.

Su ESP32 il blocco radio è una periferica dedicata che gestisce in autonomia gran parte del protocollo. Lo stack TCP/IP gira sulla CPU principale (o su un secondo core, se presente) e usa RAM per buffer di rete. È la fase più "pesante" in termini di consumo energetico.

### Fase 3 — Inizializzazione del sensore

**Blocchi coinvolti:** GPIO, ADC, o I2C/SPI (dipende dal sensore).

Questa varia molto in funzione dell'hardware reale:

- **NTC, fotoresistore:** ADC, perché il sensore restituisce una tensione analogica
- **DS18B20:** GPIO con protocollo 1-Wire emulato in software
- **BME280, SHT31:** I2C o SPI

Nel codice del documento di riferimento la lettura è simulata con `random()`, quindi non viene effettivamente usato alcun blocco dedicato.

### Fase 4 — Ciclo principale

È la fase con il mapping più ricco:

| Sotto-fase | Blocco MCU |
|------------|-----------|
| `millis()` / `time.time()` | Timer di sistema |
| Lettura temperatura | ADC o I2C/SPI |
| Composizione JSON | CPU + RAM (buffer stringa) |
| Pubblicazione messaggio | Radio WiFi (stack TCP → MQTT) |
| Aggiornamento timestamp | Variabile in RAM |

Questi blocchi vengono attivati ad ogni iterazione, quindi sono i candidati principali per ottimizzazioni di consumo e prestazioni.

### Fase 5 — Attesa

**Blocchi coinvolti:** Timer, controllore interrupt, eventualmente blocco di gestione low-power.

Un semplice `delay()` lascia la CPU in idle ma alimentata. Un **deep sleep** spegne CPU, RAM e la maggior parte delle periferiche, lasciando attivo solo un timer a bassissima frequenza (RTC) che risveglia il chip al momento giusto.

---

## Schema a blocchi di un SBC

Per confronto, ecco l'architettura di un Single Board Computer come il Raspberry Pi.

<svg width="100%" viewBox="0 0 680 680" xmlns="http://www.w3.org/2000/svg">
<defs>
<marker id="arrow4" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
<path d="M2 1L8 5L2 9" fill="none" stroke="context-stroke" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/>
</marker>
<style>
.t4 { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 14px; }
.ts4 { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 12px; }
.th4 { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 14px; font-weight: 500; }
.arr4 { stroke: #888780; stroke-width: 1.5; fill: none; }
</style>
</defs>

<rect x="30" y="30" width="620" height="600" rx="14" fill="none" stroke="#888780" stroke-width="1" stroke-dasharray="4 4" opacity="0.6"/>
<text class="ts4" x="50" y="50" text-anchor="start" fill="#5F5E5A">Single Board Computer (es. Raspberry Pi)</text>

<rect x="60" y="80" width="560" height="160" rx="12" fill="#EEEDFE" stroke="#534AB7" stroke-width="0.5" opacity="0.4"/>
<text class="ts4" x="80" y="100" text-anchor="start" fill="#3C3489">SoC (System on Chip)</text>

<rect x="80" y="115" width="160" height="110" rx="8" fill="#EEEDFE" stroke="#534AB7" stroke-width="0.5"/>
<text class="th4" x="160" y="145" text-anchor="middle" dominant-baseline="central" fill="#3C3489">CPU multi-core</text>
<text class="ts4" x="160" y="167" text-anchor="middle" dominant-baseline="central" fill="#534AB7">ARM Cortex-A</text>
<text class="ts4" x="160" y="185" text-anchor="middle" dominant-baseline="central" fill="#534AB7">Cache L1, L2, L3</text>
<text class="ts4" x="160" y="203" text-anchor="middle" dominant-baseline="central" fill="#534AB7">MMU per memoria virtuale</text>

<rect x="260" y="115" width="160" height="110" rx="8" fill="#EEEDFE" stroke="#534AB7" stroke-width="0.5"/>
<text class="th4" x="340" y="145" text-anchor="middle" dominant-baseline="central" fill="#3C3489">GPU</text>
<text class="ts4" x="340" y="167" text-anchor="middle" dominant-baseline="central" fill="#534AB7">Grafica e video</text>
<text class="ts4" x="340" y="185" text-anchor="middle" dominant-baseline="central" fill="#534AB7">Decodifica hardware</text>
<text class="ts4" x="340" y="203" text-anchor="middle" dominant-baseline="central" fill="#534AB7">Calcolo parallelo</text>

<rect x="440" y="115" width="160" height="110" rx="8" fill="#EEEDFE" stroke="#534AB7" stroke-width="0.5"/>
<text class="th4" x="520" y="145" text-anchor="middle" dominant-baseline="central" fill="#3C3489">Controller I/O</text>
<text class="ts4" x="520" y="167" text-anchor="middle" dominant-baseline="central" fill="#534AB7">USB, Ethernet</text>
<text class="ts4" x="520" y="185" text-anchor="middle" dominant-baseline="central" fill="#534AB7">HDMI, DSI</text>
<text class="ts4" x="520" y="203" text-anchor="middle" dominant-baseline="central" fill="#534AB7">DMA</text>

<rect x="60" y="270" width="560" height="40" rx="6" fill="#F1EFE8" stroke="#5F5E5A" stroke-width="0.5"/>
<text class="th4" x="340" y="290" text-anchor="middle" dominant-baseline="central" fill="#2C2C2A">Bus ad alta velocità (AXI / AMBA)</text>

<line x1="160" y1="240" x2="160" y2="270" class="arr4"/>
<line x1="340" y1="240" x2="340" y2="270" class="arr4"/>
<line x1="520" y1="240" x2="520" y2="270" class="arr4"/>

<line x1="160" y1="310" x2="160" y2="345" class="arr4"/>
<line x1="340" y1="310" x2="340" y2="345" class="arr4"/>
<line x1="520" y1="310" x2="520" y2="345" class="arr4"/>

<rect x="60" y="345" width="200" height="80" rx="8" fill="#E1F5EE" stroke="#0F6E56" stroke-width="0.5"/>
<text class="th4" x="160" y="370" text-anchor="middle" dominant-baseline="central" fill="#085041">RAM (esterna)</text>
<text class="ts4" x="160" y="392" text-anchor="middle" dominant-baseline="central" fill="#0F6E56">DDR4, parecchi GB</text>
<text class="ts4" x="160" y="410" text-anchor="middle" dominant-baseline="central" fill="#0F6E56">Chip separati sulla scheda</text>

<rect x="280" y="345" width="200" height="80" rx="8" fill="#E1F5EE" stroke="#0F6E56" stroke-width="0.5"/>
<text class="th4" x="380" y="370" text-anchor="middle" dominant-baseline="central" fill="#085041">Storage (microSD/eMMC)</text>
<text class="ts4" x="380" y="392" text-anchor="middle" dominant-baseline="central" fill="#0F6E56">Sistema operativo e dati</text>
<text class="ts4" x="380" y="410" text-anchor="middle" dominant-baseline="central" fill="#0F6E56">Decine di GB</text>

<rect x="500" y="345" width="120" height="80" rx="8" fill="#FAEEDA" stroke="#854F0B" stroke-width="0.5"/>
<text class="th4" x="560" y="370" text-anchor="middle" dominant-baseline="central" fill="#633806">PMU</text>
<text class="ts4" x="560" y="392" text-anchor="middle" dominant-baseline="central" fill="#854F0B">Power management</text>
<text class="ts4" x="560" y="410" text-anchor="middle" dominant-baseline="central" fill="#854F0B">Clock di sistema</text>

<rect x="60" y="465" width="560" height="40" rx="6" fill="#F1EFE8" stroke="#5F5E5A" stroke-width="0.5"/>
<text class="th4" x="340" y="485" text-anchor="middle" dominant-baseline="central" fill="#2C2C2A">Connettori e interfacce esterne</text>

<line x1="160" y1="425" x2="160" y2="465" class="arr4"/>
<line x1="380" y1="425" x2="380" y2="465" class="arr4"/>

<line x1="120" y1="505" x2="120" y2="540" class="arr4" marker-end="url(#arrow4)"/>
<line x1="260" y1="505" x2="260" y2="540" class="arr4" marker-end="url(#arrow4)"/>
<line x1="400" y1="505" x2="400" y2="540" class="arr4" marker-end="url(#arrow4)"/>
<line x1="540" y1="505" x2="540" y2="540" class="arr4" marker-end="url(#arrow4)"/>

<rect x="50" y="540" width="140" height="70" rx="8" fill="#FAECE7" stroke="#993C1D" stroke-width="0.5"/>
<text class="th4" x="120" y="565" text-anchor="middle" dominant-baseline="central" fill="#712B13">USB</text>
<text class="ts4" x="120" y="585" text-anchor="middle" dominant-baseline="central" fill="#993C1D">Tastiera, dischi, webcam</text>

<rect x="200" y="540" width="120" height="70" rx="8" fill="#FAECE7" stroke="#993C1D" stroke-width="0.5"/>
<text class="th4" x="260" y="565" text-anchor="middle" dominant-baseline="central" fill="#712B13">Ethernet / WiFi</text>
<text class="ts4" x="260" y="585" text-anchor="middle" dominant-baseline="central" fill="#993C1D">Rete TCP/IP</text>

<rect x="340" y="540" width="120" height="70" rx="8" fill="#FAECE7" stroke="#993C1D" stroke-width="0.5"/>
<text class="th4" x="400" y="565" text-anchor="middle" dominant-baseline="central" fill="#712B13">HDMI / display</text>
<text class="ts4" x="400" y="585" text-anchor="middle" dominant-baseline="central" fill="#993C1D">Output video</text>

<rect x="480" y="540" width="140" height="70" rx="8" fill="#FAECE7" stroke="#993C1D" stroke-width="0.5"/>
<text class="th4" x="550" y="565" text-anchor="middle" dominant-baseline="central" fill="#712B13">GPIO header</text>
<text class="ts4" x="550" y="585" text-anchor="middle" dominant-baseline="central" fill="#993C1D">UART, SPI, I2C, pin</text>
</svg>


La differenza principale rispetto alla MCU è la separazione fisica dei componenti.

### Il SoC

Il cuore della scheda è un **SoC (System on Chip)** che integra al suo interno:

| Blocco | Funzione |
|--------|----------|
| **CPU multi-core** | ARM Cortex-A a 1-2 GHz, con cache multi-livello e MMU per gestire la memoria virtuale richiesta dal sistema operativo. |
| **GPU** | Accelerazione grafica, decodifica video hardware, calcolo parallelo. Su alcuni SBC è usata anche per ML. |
| **Controller I/O** | Gestisce in hardware le interfacce USB, Ethernet, HDMI, DMA. |

### Memorie esterne

A differenza della MCU, RAM e storage sono **chip separati** sulla scheda, collegati al SoC tramite bus ad alta velocità:

- **RAM (DDR4):** parecchi GB, indispensabili per far girare un sistema operativo completo
- **Storage (microSD o eMMC):** decine di GB, contiene il filesystem del sistema operativo

### Interfacce esterne

Le periferiche di un SBC sono di natura diversa rispetto a quelle di una MCU. Mentre la MCU ha pin diretti verso sensori e attuatori, l'SBC espone:

- **USB** per periferiche standard del mondo PC (tastiere, dischi, webcam)
- **Ethernet/WiFi** per la rete TCP/IP
- **HDMI** per output video
- **GPIO header** per le interfacce hardware tipiche del mondo embedded (UART, SPI, I2C, pin digitali), che permettono all'SBC di "scendere" al livello della MCU quando serve

È proprio il GPIO header che rende il Raspberry Pi popolare nei progetti maker: dà accesso "stile MCU" pur avendo a disposizione tutta la potenza di un computer.

---

## Confronto MCU vs SBC

<svg width="100%" viewBox="0 0 680 560" xmlns="http://www.w3.org/2000/svg">
<defs>
<style>
.t5 { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 14px; }
.ts5 { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 12px; }
.th5 { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; font-size: 14px; font-weight: 500; }
</style>
</defs>

<rect x="30" y="30" width="300" height="500" rx="14" fill="#EEEDFE" stroke="#534AB7" stroke-width="0.5" opacity="0.3"/>
<text class="th5" x="180" y="55" text-anchor="middle" fill="#3C3489">MCU</text>
<text class="ts5" x="180" y="73" text-anchor="middle" fill="#534AB7">Microcontrollore</text>

<rect x="350" y="30" width="300" height="500" rx="14" fill="#E1F5EE" stroke="#0F6E56" stroke-width="0.5" opacity="0.3"/>
<text class="th5" x="500" y="55" text-anchor="middle" fill="#085041">SBC</text>
<text class="ts5" x="500" y="73" text-anchor="middle" fill="#0F6E56">Single Board Computer</text>

<line x1="30" y1="100" x2="650" y2="100" stroke="#888780" stroke-width="0.5"/>

<text class="th5" x="50" y="125" fill="#2C2C2A">Esempi</text>
<text class="ts5" x="50" y="145" fill="#5F5E5A">ATmega328, ESP32, STM32</text>
<text class="ts5" x="370" y="145" fill="#5F5E5A">Raspberry Pi, BeagleBone</text>

<line x1="30" y1="170" x2="650" y2="170" stroke="#D3D1C7" stroke-width="0.5"/>

<text class="th5" x="50" y="195" fill="#2C2C2A">Architettura</text>
<text class="ts5" x="50" y="215" fill="#5F5E5A">CPU, RAM, Flash sullo</text>
<text class="ts5" x="50" y="231" fill="#5F5E5A">stesso chip</text>
<text class="ts5" x="370" y="215" fill="#5F5E5A">SoC + RAM + storage</text>
<text class="ts5" x="370" y="231" fill="#5F5E5A">come componenti separati</text>

<line x1="30" y1="250" x2="650" y2="250" stroke="#D3D1C7" stroke-width="0.5"/>

<text class="th5" x="50" y="275" fill="#2C2C2A">Software</text>
<text class="ts5" x="50" y="295" fill="#5F5E5A">Bare metal: un solo</text>
<text class="ts5" x="50" y="311" fill="#5F5E5A">programma (loop infinito)</text>
<text class="ts5" x="370" y="295" fill="#5F5E5A">Sistema operativo Linux,</text>
<text class="ts5" x="370" y="311" fill="#5F5E5A">processi e scheduler</text>

<line x1="30" y1="330" x2="650" y2="330" stroke="#D3D1C7" stroke-width="0.5"/>

<text class="th5" x="50" y="355" fill="#2C2C2A">Memoria</text>
<text class="ts5" x="50" y="375" fill="#5F5E5A">RAM: KB; Flash: MB</text>
<text class="ts5" x="370" y="375" fill="#5F5E5A">RAM: GB; storage: GB</text>

<line x1="30" y1="395" x2="650" y2="395" stroke="#D3D1C7" stroke-width="0.5"/>

<text class="th5" x="50" y="420" fill="#2C2C2A">Consumo</text>
<text class="ts5" x="50" y="440" fill="#5F5E5A">Da µW (deep sleep)</text>
<text class="ts5" x="50" y="456" fill="#5F5E5A">a poche centinaia di mW</text>
<text class="ts5" x="370" y="440" fill="#5F5E5A">Da 2-3 W a oltre 10 W</text>
<text class="ts5" x="370" y="456" fill="#5F5E5A">in carico</text>

<line x1="30" y1="475" x2="650" y2="475" stroke="#D3D1C7" stroke-width="0.5"/>

<text class="th5" x="50" y="500" fill="#2C2C2A">Uso tipico</text>
<text class="ts5" x="50" y="520" fill="#5F5E5A">Sensori, attuatori, IoT edge</text>
<text class="ts5" x="370" y="520" fill="#5F5E5A">Gateway, computer vision, ML</text>
</svg>


Stesso firmware MQTT, comportamento profondamente diverso:

| Aspetto | MCU (ESP32) | SBC (Raspberry Pi) |
|---------|-------------|---------------------|
| **Esecuzione** | `loop()` hardware-driven sul bare metal | Processo Python gestito dallo scheduler Linux |
| **`delay()` / `sleep()`** | Blocca tutto il programma | Rilascia la CPU ad altri processi |
| **MQTT** | Libreria linkata nel firmware | Può essere un servizio `systemd` indipendente |
| **Consumo minimo** | Pochi µA in deep sleep | Diversi watt sempre |
| **Tempo di avvio** | Millisecondi | 20-60 secondi (boot Linux) |
| **Costo unitario** | 2-10 € | 30-80 € |

Per applicazioni IoT "edge" semplici (un sensore che pubblica un valore), la MCU è quasi sempre la scelta giusta. Per applicazioni con elaborazione complessa (computer vision, ML on-device, gateway che aggregano dati da più nodi), un SBC è necessario.

Una configurazione tipica è proprio quella **ibrida**: nodi MCU sparsi nell'ambiente che misurano e pubblicano via MQTT, un SBC centrale che fa da broker, archivia i dati, espone dashboard.

---

## Considerazioni progettuali

### Efficienza energetica

In un nodo a batteria la differenza tra `delay()` e `deep sleep` è drammatica. Su ESP32:

| Stato | Consumo tipico |
|-------|----------------|
| CPU attiva con WiFi | 80–160 mA |
| CPU attiva senza WiFi | 20–30 mA |
| Light sleep | 0.8 mA |
| Deep sleep | 5–10 µA |

Per un sensore che pubblica ogni minuto, il pattern ottimale è: **deep sleep → wake up → misura → connetti WiFi → pubblica → torna in deep sleep**, tutto in 1-2 secondi di attività.

### Costo della radio

La radio WiFi è il blocco più costoso in termini energetici e temporali. La connessione iniziale (fase 2) può richiedere diversi secondi e centinaia di milliampere. Per questo motivo, in scenari batteria-critici si preferiscono protocolli a basso consumo come **Zigbee**, **BLE** o **LoRaWAN** rispetto a WiFi.

### Polling vs interrupt

Il pattern `currentTime - lastSentTime >= interval` è polling: la CPU controlla continuamente se è ora di agire. Un'alternativa più elegante è far scattare un timer hardware che genera un interrupt all'intervallo desiderato — la CPU può così dedicarsi ad altro (o dormire) finché l'interrupt non la sveglia.

### Quando scegliere cosa

- **Scegli una MCU** se: alimentazione a batteria, costo unitario basso, funzioni semplici e ripetitive, tempo di avvio rapido, affidabilità nel tempo (niente OS che si rompe).
- **Scegli un SBC** se: elaborazione complessa, necessità di un OS (per librerie, networking avanzato, container), interfaccia utente grafica, integrazione con servizi cloud non banali.

---

## Riferimenti

- Documento originale: [sensorfw.md su GitHub](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/sensorfw.md)
- Repo di riferimento: [ArduinoBareMetal](https://github.com/sebastianomelita/ArduinoBareMetal)
- Architetture correlate trattate nel repo: Zigbee, BLE, WiFi infrastruttura, WiFi mesh, LoRaWAN
