>[Torna a reti di sensori](../sensornetworkshort.md)>[Torna a reti ethernet](../archeth.md)

- [Dettaglio architettura Zigbee](../archzigbee.md)
- [Dettaglio architettura BLE](../archble.md)
- [Dettaglio architettura WiFi infrastruttura](../archwifi.md)
- [Dettaglio architettura WiFi mesh](../archmesh.md) 
- [Dettaglio architettura LoraWAN](../lorawanclasses.md) 



# Tutorial: configurare un end-device LoRaWAN ABP su MultiTech Conduit (mPower)

Guida operativa basata su un setup reale: gateway **MultiTech Conduit MTCDTIP-267A**, firmware **mPower 6.0.1**, in modalità **Network Server locale** (non packet-forwarder verso TTN). End-device **Heltec ESP32 LoRa V2 / Wireless Stick** con firmware MicroPython che usa la libreria `ulora` in attivazione **ABP**.

L'obiettivo: arrivare a vedere i payload del proprio device decifrati e pubblicati sul broker MQTT interno del Conduit, pronti per essere consumati da un'applicazione.

---

## 0. Prerequisiti e idee chiave

Prima di toccare la UI, fissiamo alcuni concetti che evitano confusione strada facendo.

**ABP vs OTAA.** ABP (Activation By Personalization) significa che `DevAddr`, `NwkSKey` e `AppSKey` sono già configurati sia sul device sia sul network server: non c'è alcun handshake di join. OTAA (Over-The-Air Activation) prevede invece una `JoinRequest` iniziale che genera dinamicamente la session. Le procedure di registrazione sul Conduit sono diverse. Questa guida copre **ABP**.

**Cosa fa il Conduit in modalità Network Server.** Riceve i pacchetti via radio, li decritta usando le chiavi del device, valida il MIC e il frame counter, ed espone i payload decodificati su un broker **MQTT locale** (Mosquitto, porta 1883). I pacchetti **non vengono inoltrati a TTN o ad altri network server esterni** — è una rete LoRaWAN autosufficiente. Se serve TTN, va cambiata la modalità a Packet Forwarder.

**Tre entità, non una.** Sul Conduit per far funzionare un device ABP servono:

1. Un **End Device** (anagrafica: DevEUI, nome, profilo).
2. Una **Session** (le chiavi ABP, agganciate al DevEUI di sopra).
3. Un **Save and Apply** finale per caricare le modifiche nel Network Server.

Saltare uno di questi tre passi è il modo più tipico per ritrovarsi con pacchetti che arrivano in radio ma non vengono mai decifrati.

**Formato dei valori esadecimali.** L'UI mPower accetta gli EUI nel formato `xx-xx-xx-xx-xx-xx-xx-xx` (8 byte separati da trattino, lowercase). Le chiavi a 16 byte e il DevAddr a 4 byte si inseriscono invece come stringa hex compatta (senza separatori), tipo `C77FB78C377ADB11803F5BD31D5B4FC2`. Tenetelo a mente, altrimenti i form rispondono con "invalid".

---

## 1. Preparare i parametri ABP dal firmware

Dal codice del device, recuperare i seguenti valori:

| Parametro | Esempio | Formato per il Conduit |
|---|---|---|
| DevEUI | `26A160FFFE6E86BC` | `26-a1-60-ff-fe-6e-86-bc` |
| DevAddr (4 byte) | `bytearray([0x26,0x0B,0x26,0x2C])` | `260B262C` |
| NwkSKey (16 byte) | `bytearray([0xC7,…,0xC2])` | `C77FB78C377ADB11803F5BD31D5B4FC2` |
| AppSKey (16 byte) | `bytearray([0x7A,…,0xF3])` | `7A95963AB4ADC6E7EA2519114BD4ADF3` |

**Suggerimento.** Aggiungere al firmware una funzione che stampi questi valori all'avvio in formato hex compatto. Risparmia errori di trascrizione al momento di compilare i form.

---

## 2. Verificare la modalità del Conduit

Entrare nella UI del Conduit (es. `https://10.0.0.21`), poi:

**LoRaWAN → Network Settings**

Controllare in alto il dropdown della modalità: deve essere su **NETWORK SERVER**. Nello stato sotto:

- `Packet Forwarder`: **RUNNING**
- `Network Server`: **RUNNING**
- `Lens Server`: irrilevante, può restare DISABLED

Nel blocco **Channel Plan**:

- `Channel Plan`: **EU868**
- Annotarsi gli `Additional Channels` configurati. I tre canali standard EU868 sono **868.1, 868.3, 868.5** MHz. I canali aggiuntivi (es. 867.1, 867.3, 867.5, 867.7, 867.9) dipendono dalla configurazione locale.

Nel blocco **Network**:

- `Network Mode`: **Public LoRaWAN** (sync word `0x34`). Il device deve trasmettere con lo stesso sync word — `ulora` di default è già pubblico.

Se servono canali aggiuntivi (perché la libreria del device trasmette su 867.x), aggiungerli qui e fare poi un **Save and Apply**.

---

## 3. Creare l'End Device

**LoRaWAN → Devices → End Devices → + Add New**

Compilare il form **ADD DEVICE**:

| Campo | Valore |
|---|---|
| Dev EUI | `26-a1-60-ff-fe-6e-86-bc` |
| Name | `heltec-test` (libero) |
| Device Profile | **`LW102-OTA-EU868`** |
| Network Profile | **`DEFAULT-CLASS-A`** |
| Serial Number / Product ID / HW / FW | vuoti |

**Nota importante sul Device Profile.** Sul mPower i profili predefiniti hanno nomi del tipo `LW102-OTA-<regione>`. Il suffisso `OTA` è solo un'etichetta legacy: **il profile contiene solo parametri MAC e radio (versione 1.0.2, regione, EIRP, duty cycle, supporto Class B/C), non determina ABP vs OTAA**. La distinzione ABP/OTAA è data invece dalla presenza o meno di una Session associata. Quindi `LW102-OTA-EU868` è il profile corretto da usare anche per dispositivi ABP.

**OK** per salvare.

A questo punto, in **End Devices**, compare la riga `26-a1-60-ff-fe-6e-86-bc / heltec-test`, con `Last Seen` ancora `unknown` (e così resta finché il NS non vede un pacchetto decifrato).

---

## 4. Creare la Session ABP

Sempre nella stessa pagina `/lora/devices`, scorrere giù fino alla sezione **Sessions → + Add New**.

Compilare il form **ADD SESSION**:

| Campo | Valore |
|---|---|
| Dev EUI | `26-a1-60-ff-fe-6e-86-bc` |
| Dev Addr | `260B262C` |
| App EUI | `0000000000000001` |
| Join EUI | `0000000000000001` |
| Net ID | `000000` |
| App Session Key | `7A95963AB4ADC6E7EA2519114BD4ADF3` |
| Net Session Key | `C77FB78C377ADB11803F5BD31D5B4FC2` |
| Multicast Session | No |

**Note sui campi "inutili ma obbligatori".** App EUI, Join EUI e Net ID non servono crittograficamente per ABP, ma il form li valida come obbligatori. Valori "innocui" che la form accetta:

- App EUI / Join EUI: `0000000000000001` (gli all-zero spesso vengono rifiutati come non validi)
- Net ID: `000000`

**Errori tipici a questo passo.**

- *"invalid dev eui"* → si è inserito il DevEUI senza trattini o in formato sbagliato. Usare `xx-xx-xx-xx-xx-xx-xx-xx` lowercase.
- *"invalid join eui" / "invalid app eui"* → mettere `0000000000000001` invece di lasciare vuoto.
- *"invalid net id"* → mettere `000000` invece di lasciare vuoto.
- *"device not found"* → si sta cercando di creare la Session ma l'End Device del punto 3 non esiste ancora. Tornare al punto 3.

**OK** per salvare.

A questo punto in **Sessions** compare la riga con DevEUI, Dev Addr `260b262c`, Up FCnt `0`, Down FCnt `0`, Joined `local`. "Local" significa "sessione ABP locale" ed è esattamente lo stato desiderato.

---

## 5. Save and Apply

In alto a destra nella UI, cliccare **Save and Apply**.

È il passo che il 90% delle volte viene dimenticato. Senza questo, il Network Server non vede né il nuovo device né la nuova session, e i pacchetti del firmware vengono scartati come "unknown DevAddr" — anche se nella UI sembra tutto a posto.

---

## 6. Far partire il firmware

Dal lato device, accendere l'Heltec con il firmware MicroPython che usa `ulora`. Punti di attenzione nel codice:

**Incremento manuale del FrameCounter.** Alcune versioni di `ulora` non incrementano automaticamente `frame_counter` dopo `send_data`. Va fatto a mano:

```python
lora.send_data(payload, len(payload), lora.frame_counter)
lora.frame_counter += 1
```

Senza questo, ogni TX usa `FCnt=0`, e il NS scarta tutti i pacchetti tranne (al massimo) il primo, come replay attack.

**Datarate.** Default ragionevole per i test: `SF9BW125`. Se il link è marginale (segnale debole, RSSI peggio di -110 dBm con CRC ERR sul gateway), passare temporaneamente a `SF12BW125` per guadagnare ~10 dB di sensibilità. SF12 ha però un tempo-in-aria molto alto (~1.5 s per 24 byte di payload): tenere un intervallo di invio di almeno 60 s per non sforare il duty cycle EU868 dell'1%.

**Intervallo TX.** Per accelerare i test, scendere a `SEND_INTERVAL_S = 15` con SF9 va bene. Con SF12 restare a 60 s o più.

---

## 7. Diagnostica progressiva

Ora si verifica lungo la catena. Sono **tre tabelle distinte** sul Conduit, con significato diverso, ed è importante non confonderle.

### 7.1 Recent Rx Packets: la radio raw

**LoRaWAN → Packets** → scorrere in basso fino a **Recent Rx Packets**.

Questa tabella mostra **tutto quello che la radio capta**, anche pacchetti corrotti o di altri device. Le colonne chiave:

- `Freq`, `Datarate`: dovrebbero corrispondere a ciò che trasmette il device (es. `868.300 / SF12BW125`).
- `CRC`: deve essere **OK**. Se è **ERR**, il pacchetto è stato captato ma è arrivato corrotto a livello fisico → problema di link radio (antenna, distanza, interferenza, frequenza fuori plan).
- `RSSI` e `SNR`: indicatori di qualità del link. RSSI sopra -110 dBm è confortevole, sotto inizia a essere marginale.
- `Type`: per uplink deve essere `UpUnc` (unconfirmed) o `UpCnf` (confirmed).

**Se non si vede nessun pacchetto del proprio device qui:** il problema è radio. Controllare:
1. Antenna collegata su entrambi i lati.
2. Frequenza del device tra i canali ascoltati dal gateway (vedere Channel Plan al punto 2).
3. Distanza ragionevole, niente metallo o pareti spesse di mezzo.
4. Sync word coerente (public/private).

**Se si vede CRC ERR ripetuti:** segnale troppo debole. Avvicinare, alzare lo SF, controllare antenne. Vedere §7.5 per la procedura di adattamento dello Spreading Factor.

**Se si vede CRC OK ma Type `Unknown`:** pacchetto valido ma il NS non lo riconosce come LoRaWAN noto. Quasi sempre è un device terzo, non il proprio.

### 7.2 Packets: il network server LoRaWAN

Sempre nella stessa pagina, in cima, c'è la tabella **Packets**. Questa è "ufficiale": mostra solo i pacchetti **riconosciuti come LoRaWAN**, già associati a un DevEUI. Le colonne chiave:

- `Device EUI`: deve essere il proprio.
- `FCnt`: il frame counter validato dal NS.
- `Type`: `UpUnc` per i propri uplink, `DnUnc` per i downlink che il NS spedisce in risposta (es. MAC commands come `LinkADRReq`).

**Cosa aspettarsi al primo TX andato a buon fine:**
- Una riga `UpUnc` con `FCnt = 00000000` (o `00000001` se il counter del firmware partiva da 1).
- Pochi secondi dopo (di solito 2-3 s) una riga `DnUnc` automatica del NS: è normale, è il `LinkADRReq` con cui il NS propone i parametri radio.

### 7.3 Devices / Sessions: lo stato del device

**LoRaWAN → Devices**

- Riga del proprio device in **End Devices**: il campo `Last Seen` passa da `unknown` a un timestamp recente.
- Riga della propria session in **Sessions**: `Up FCnt` cresce ad ogni TX successivo (1, 2, 3, …), `Last Seen` aggiornato.

**Questa è la verifica definitiva.** Se `Up FCnt` cresce, il NS ha:
1. Ricevuto il pacchetto in radio con CRC OK.
2. Trovato la session corrispondente al DevAddr.
3. Validato il MIC con NwkSKey.
4. Validato il frame counter.
5. Decifrato il payload con AppSKey.
6. Pubblicato il risultato sul broker MQTT interno.

### 7.4 Mappa errore → causa

| Sintomo | Causa probabile |
|---|---|
| Niente in Recent Rx Packets | Radio: antenna, distanza, frequenze fuori plan |
| CRC ERR ripetuti in Recent Rx | Link debole: avvicinare, alzare SF |
| CRC OK ma Type `Unknown` | Non è il proprio device, è traffico altrui |
| UpUnc in Packets ma `Up FCnt` non incrementa | Strict frame counter check: il device riparte da 0 dopo reset e i pacchetti vengono scartati come replay |
| Niente in Packets ma c'è in Recent Rx | Sessione non riconosciuta: chiavi sbagliate, DevAddr sbagliato, oppure manca `Save and Apply` |
| `Up FCnt` resta a 0 ma `Down FCnt` cresce | Manca l'incremento di `lora.frame_counter` nel firmware |

### 7.5 Adattare lo Spreading Factor alla distanza (caso reale)

Lo Spreading Factor determina il bilancio fra **sensibilità** (link che regge anche con segnale debole) e **time-on-air** (durata della trasmissione, occupazione del canale, duty cycle, consumo). Più alto è SF, più si va lontano, ma più costa in tempo radio.

In LoRaWAN "vero" l'adattamento (ADR) è gestito automaticamente dal Network Server, che osserva la qualità degli uplink e manda al device dei `LinkADRReq` come downlink. Il Conduit lo fa: nello scenario descritto qui, dopo il primo UpUnc compariva immediatamente un `DnUnc` proprio per concordare i parametri radio. Il problema è che `ulora` non implementa il parsing dei MAC commands ricevuti, quindi quelle indicazioni del NS non vengono applicate sul device. Risultato: lo SF resta fissato a quello configurato nel firmware (`LORA_DATARATE`), e l'adattamento va fatto a mano.

**Procedura pratica osservata durante il setup descritto:**

1. **Partire da uno SF intermedio**: `SF9BW125` è un buon default. Time-on-air di un payload di ~24 byte è circa 180 ms, sensibilità ~-127 dBm.
2. **Far girare il firmware per qualche minuto** e guardare la tabella **Recent Rx Packets** sul Conduit (vedere §7.1):
   - Se compaiono righe con la frequenza e lo SF giusti, ma **CRC = ERR** e RSSI peggiore di -110 dBm → il segnale arriva ma è troppo debole per essere decodificato. Esempio osservato nello scenario: `Freq 867.700 / SF9BW125 / CRC ERR / SNR -14.8 / RSSI -115`.
   - Se compaiono righe **CRC OK** con il proprio DevEUI → SF già sufficiente, andare al punto 4.
   - Se non compare nulla → problema di radio più grave (antenna, frequenza fuori plan, distanza enorme), non si risolve con lo SF.
3. **Salire di un livello di SF** se si è nel primo caso. Modificare nel firmware:

   ```python
   LORA_DATARATE = "SF10BW125"   # poi SF11, poi SF12 se serve ancora
   ```

   Ogni "scalino" di SF aggiunge circa 2.5–3 dB di sensibilità, ma raddoppia (più o meno) il time-on-air. Ricaricare e ripetere il punto 2. Nello scenario descritto, salendo a `SF12BW125` il pacchetto è passato a `CRC OK` con RSSI ancora a -116 dBm ma SNR sufficiente, e il Network Server ha potuto decifrarlo (Up FCnt che inizia a salire).
4. **Quando si arriva a CRC OK stabile**, l'adattamento è riuscito. Si può eventualmente provare a **scendere** di uno SF per recuperare airtime, se le condizioni lo permettono (distanza fissa, antenna stabile). Se a SF inferiore tornano CRC ERR → risalire di un livello e fermarsi lì.

**Tabella di riferimento dei trade-off (EU868, BW125):**

| Datarate | Sensibilità | ToA payload 24 B | Max TX/ora @ 1% duty |
|---|---|---|---|
| `SF7BW125`  | ~-123 dBm | ~60 ms   | ~600 |
| `SF8BW125`  | ~-126 dBm | ~110 ms  | ~325 |
| `SF9BW125`  | ~-129 dBm | ~185 ms  | ~190 |
| `SF10BW125` | ~-132 dBm | ~370 ms  | ~95  |
| `SF11BW125` | ~-134 dBm | ~740 ms  | ~48  |
| `SF12BW125` | ~-137 dBm | ~1500 ms | ~24  |

I valori di "Max TX/ora" sono indicativi e considerano solo il duty cycle: la realtà operativa è più bassa. Sopra SF10, riconsiderare l'intervallo di invio (almeno 60 s, meglio di più), e tenere in mente che un eventuale ritrasmesso/ack consuma altro tempo.

**Trappole tipiche:**

- Tornare giù di SF "perché tanto adesso funziona" senza ricontrollare la qualità. Una giornata umida o uno spostamento di 2 metri possono rendere marginale un link che a SF basso sembrava ok.
- Lasciare a SF12 anche quando non serve. Si occupa il gateway, si spreca batteria sul device, si rischia di sforare il duty cycle.
- Confondere CRC ERR con "chiavi sbagliate". CRC ERR è radio (livello fisico). Chiavi sbagliate si manifestano come CRC OK + Up FCnt che non incrementa (livello LoRaWAN).

### 7.6 Reset del frame counter

In ABP, ogni reset del device fa ripartire il `FCnt` da 0. Il NS, se ha già visto valori più alti, li scarta come replay.

Soluzioni:

- **Skip Frame Counter Check**: cercare l'opzione nei dettagli della Session (icona occhio in Sessions) oppure in **Network Settings → Network Server Testing**. Per uso in laboratorio è la via giusta. Sconsigliato in produzione.
- **Reset Counters**: pulsante presente sulla riga della Session, azzera il contatore lato NS dopo un reset del device.
- **Persistere FCnt su NVS**: soluzione corretta lato firmware per uso in produzione (salvare `frame_counter` in flash a ogni TX e ricaricarlo a boot). Fuori scope di questo tutorial.

---

## 8. Leggere i payload dal broker MQTT del Conduit

Una volta che `Up FCnt` cresce, i payload decifrati sono pubblicati sul broker Mosquitto interno al Conduit.

**Parametri di default:**

- Host: IP del Conduit (es. `10.0.0.21`)
- Porta: `1883` (MQTT in chiaro, senza TLS)
- Topic uplink: `lora/+/up`, dove `+` è il DevEUI del singolo device
- Autenticazione: di solito assente sul mPower base; se richiesta, configurata in **Setup → MQTT** o **Administration**

### 8.1 Test rapido da riga di comando

Da un PC sulla stessa rete (`mosquitto-clients` installato):

```bash
mosquitto_sub -h 10.0.0.21 -p 1883 -t 'lora/+/up' -v
```

Ad ogni uplink del device, compare un JSON con i campi:

```json
{
  "tmst": 742934660,
  "freq": 868.3,
  "datr": "SF12BW125",
  "rssi": -116,
  "lsnr": -14.2,
  "size": 24,
  "data": "<base64>",
  "deveui": "26-a1-60-ff-fe-6e-86-bc",
  "devaddr": "260b262c",
  "fcnt": 7,
  "port": 1
}
```

Il campo `data` è il payload applicativo in **base64**: contiene i byte originali che il firmware ha messo dentro al `struct.pack`.

### 8.2 Anatomia del topic e comandi `mosquitto_*` utili

Il topic non è solo una "stringa di indirizzo": codifica chi è il publisher, chi è il subscriber e in che direzione viaggia il messaggio. Capirne la struttura permette di filtrare in modo chirurgico cosa ascoltare e dove pubblicare.

**Struttura generale (modello LoRaWAN standard, es. ChirpStack/TTN):**

```
lorawan/<APP-EUI>/<DEV-EUI>/<direzione>
```

dove:

- **`<APP-EUI>`** identifica l'applicazione (o il gateway/network server) — è il MAC EUI-64 dell'entità che raccoglie i dati.
- **`<DEV-EUI>`** identifica il singolo dispositivo IoT, anch'esso in formato EUI-64.
- **`<direzione>`** è `up` o `down`:
  - **`up`** → uplink, il dispositivo è publisher e il server applicativo è subscriber. Si legge per ricevere misure, stato, telemetria.
  - **`down`** → downlink, il server applicativo è publisher e il dispositivo è subscriber. Si scrive per inviare comandi, attuazioni, configurazioni.

**Esempio dal materiale di riferimento:**

```
lorawan/8b-6c-f0-8e-ee-df-1b-b6/00-80-00-ff-ff-00-00-03/up
        \__________________/  \__________________/  \_/
              APP-EUI                DEV-EUI       direzione
```

**Struttura sul Conduit mPower (versione semplificata):**

Il Conduit usa una variante più compatta, senza l'APP-EUI esplicito:

```
lora/<DEV-EUI>/up
lora/<DEV-EUI>/down
```

ad esempio:

```
lora/26-a1-60-ff-fe-6e-86-bc/up
```

Il motivo è che il NS interno al Conduit non gestisce molteplici applicazioni separate: c'è un solo "tenant" implicito, quindi l'APP-EUI viene omesso. La logica però è la stessa: l'ultimo segmento (`up`/`down`) indica la direzione, il segmento centrale identifica il device.

**Wildcards MQTT.** Due caratteri jolly da conoscere:

- **`+`** copre **un solo livello** del topic. Esempio: `lora/+/up` cattura gli uplink di **tutti** i device, ma non i downlink.
- **`#`** copre **tutti i livelli successivi**. Deve stare in fondo. Esempio: `lora/#` cattura tutto il traffico LoRa, in entrambe le direzioni.

**Esempi pratici da console Linux (con `mosquitto-clients`):**

Installa una sola volta:

```bash
sudo apt install mosquitto-clients
```

**Ascoltare tutti gli uplink di tutti i device (modalità "fire-hose"):**

```bash
mosquitto_sub -h 10.0.0.21 -p 1883 -t 'lora/+/up' -v
```

L'opzione `-v` ("verbose") fa stampare anche il topic prima del payload, utile per capire da quale device viene il messaggio quando si ascoltano più device contemporaneamente.

**Ascoltare solo gli uplink di un singolo device:**

```bash
mosquitto_sub -h 10.0.0.21 -p 1883 -t 'lora/26-a1-60-ff-fe-6e-86-bc/up' -v
```

**Ascoltare tutto (uplink, downlink, eventi di sistema):**

```bash
mosquitto_sub -h 10.0.0.21 -p 1883 -t 'lora/#' -v
```

Utile in fase di debug iniziale per vedere "che cosa pubblica davvero questo broker", senza ipotesi su quali topic siano attivi.

**Filtrare solo i downlink** (per vedere cosa il Network Server sta mandando ai device, es. i `LinkADRReq`):

```bash
mosquitto_sub -h 10.0.0.21 -p 1883 -t 'lora/+/down' -v
```

**Inviare un downlink a un device** (publisher, non subscriber):

```bash
mosquitto_pub -h 10.0.0.21 -p 1883 \
  -t 'lora/26-a1-60-ff-fe-6e-86-bc/down' \
  -m '{"data":"AQID","port":1,"ack":false}'
```

Il campo `data` è in base64: `AQID` decodifica nei tre byte `0x01 0x02 0x03`. Il device, alla prossima finestra RX, riceverà questi byte sul `fport=1`.

**Salvare il flusso su file per analisi a posteriori:**

```bash
mosquitto_sub -h 10.0.0.21 -p 1883 -t 'lora/#' -v >> uplinks.log
```

**Combinare con `jq` per estrarre solo i campi che interessano:**

`mosquitto_sub -v` stampa "topic + spazio + payload JSON" su una riga. Per processarlo con `jq` (parser JSON da riga di comando) conviene buttare via il topic:

```bash
mosquitto_sub -h 10.0.0.21 -p 1883 -t 'lora/+/up' \
  | jq '{devaddr, fcnt, rssi, lsnr, freq, datr}'
```

oppure:

```bash
mosquitto_sub -h 10.0.0.21 -p 1883 -t 'lora/+/up' \
  | jq -r '"[\(.devaddr)] fcnt=\(.fcnt)  rssi=\(.rssi)  snr=\(.lsnr)"'
```

Esempio di output:

```
[260b262c] fcnt=7  rssi=-116  snr=-14.2
[260b262c] fcnt=8  rssi=-114  snr=-12.1
```

**Decodificare il payload base64 direttamente da shell** (per chi vuole evitare lo script Python di §8.3):

```bash
mosquitto_sub -h 10.0.0.21 -p 1883 -t 'lora/+/up' \
  | jq -r '.data' \
  | while read b64; do echo "$b64" | base64 -d | xxd; done
```

Stampa il dump esadecimale dei byte applicativi reali (prima del `struct.pack`), comodo per verificare a colpo d'occhio che i 24 byte attesi siano lì.

**Opzioni `mosquitto_sub` più usate in debug:**

| Opzione | Cosa fa |
|---|---|
| `-h <host>` | Host del broker |
| `-p <port>` | Porta (1883 chiaro, 8883 TLS) |
| `-t <topic>` | Topic, può contenere `+` o `#` |
| `-v` | Stampa il topic insieme al messaggio |
| `-C 1` | Esce dopo il primo messaggio ricevuto (utile in script) |
| `-u <user> -P <pwd>` | Credenziali se il broker le richiede |
| `-d` | Mostra il debug della connessione (CONNECT, PINGREQ, ecc.) |
| `--retained-only` | Riceve solo i messaggi retained (utili per stati persistenti) |

### 8.3 Subscriber Python con decodifica

Esempio per un payload costruito con `struct.pack('<QHHHHHhI', ts, sid, pm10, pm25, eCO2, tVOC, temp, press)`:

```python
# pip install paho-mqtt
import base64, json, struct
import paho.mqtt.client as mqtt

BROKER = "10.0.0.21"
PORT   = 1883
TOPIC  = "lora/+/up"

def on_connect(c, u, f, rc):
    print("Connected, rc =", rc)
    c.subscribe(TOPIC)

def on_message(c, u, msg):
    p = json.loads(msg.payload)
    raw = base64.b64decode(p["data"])
    ts, sid, pm10, pm25, eCO2, tVOC, temp, press = struct.unpack('<QHHHHHhI', raw)
    print(f"[{p.get('devaddr')}] fcnt={p.get('fcnt')} "
          f"rssi={p.get('rssi')} snr={p.get('lsnr')}")
    print(f"  pm10={pm10/10}  pm25={pm25/10}  eCO2={eCO2}  tVOC={tVOC} "
          f" temp={temp/10}  press={press}")

c = mqtt.Client()
c.on_connect = on_connect
c.on_message = on_message
c.connect(BROKER, PORT, 60)
c.loop_forever()
```

### 8.4 Downlink

Per spedire un downlink al device, pubblicare su `lora/<DevEUI>/down` un JSON tipo:

```json
{ "data": "<base64>", "port": 1, "ack": false }
```

Il downlink verrà consegnato nella prossima finestra RX disponibile (RX1 o RX2 dopo il successivo uplink, perché in Class A il device ascolta solo dopo aver trasmesso).

---

## 9. Checklist sintetica

Lista da spuntare quando si configura un nuovo device:

- [ ] Recuperati DevEUI, DevAddr, NwkSKey, AppSKey dal firmware
- [ ] Conduit in modalità **NETWORK SERVER** con Packet Forwarder e NS in RUNNING
- [ ] Channel Plan EU868 con i canali necessari (default 868.1/.3/.5 + eventuali aggiuntivi)
- [ ] Network Mode = Public LoRaWAN
- [ ] Creato l'**End Device** con DevEUI in formato `xx-xx-xx-xx-xx-xx-xx-xx` e Device Profile `LW102-OTA-EU868`
- [ ] Creata la **Session** ABP con chiavi corrette, AppEUI/JoinEUI = `0000000000000001`, NetID = `000000`
- [ ] **Save and Apply** fatto
- [ ] Firmware con `lora.frame_counter += 1` esplicito dopo ogni `send_data`
- [ ] **Recent Rx Packets**: pacchetti del device con CRC OK
- [ ] **Packets**: righe UpUnc con il DevEUI del device e FCnt che cresce
- [ ] **Sessions**: `Up FCnt` incrementa, `Last Seen` aggiornato
- [ ] `mosquitto_sub` riceve i JSON su `lora/+/up`

Quando tutti questi punti sono verdi, la catena **firmware → radio → gateway → network server → MQTT** è completa e si può iniziare a costruire l'applicazione vera (dashboard, storage su DB, ponti verso broker esterni, Node-RED, eccetera).
