> [Torna alla dispensa principale RFID](../archrfid.md)
> [Torna a reti di sensori](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/sensornetworkshort.md)

# **Esempio di traccia svolta in stile seconda prova**

## **Traccia**

> Una catena di abbigliamento intende rinnovare il proprio sistema di tracciabilità dei capi nei propri 50 punti vendita italiani. Si vogliono raggiungere i seguenti obiettivi:
> - **inventario in tempo reale** dello stock a scaffale (con accuratezza > 98%);
> - **antitaccheggio** automatico ai varchi di uscita;
> - **velocità della cassa** migliorata (lettura simultanea di più capi nel cestino senza sbarcodare uno alla volta);
> - **integrazione** con l'**ERP centrale** in cloud per analytics di vendita e riassortimento automatico.
>
> Il candidato proponga un'architettura completa del sistema, motivando le scelte tecnologiche, la dislocazione dei dispositivi, il protocollo di comunicazione tra negozio e centrale, il formato dei messaggi e le contromisure di sicurezza e privacy.

## **Soluzione proposta**

### **Scelta della tecnologia di Auto-ID**

Si sceglie **RFID UHF EPC Gen2 v2** (ISO 18000-63) perché:

- consente **lettura di massa** rapida (centinaia di tag/secondo) — necessaria per la lettura del cestino in cassa e per l'inventario di scaffale.
- ha **costo per tag** compatibile con l'applicazione su ogni capo (5-10 cent), inferiore al costo del tag antitaccheggio EAS tradizionale.
- offre **portata** sufficiente per varchi (3-5 m) ed è già lo **standard del settore retail** (Decathlon, Zara/Inditex, Uniqlo lo usano).

Si scartano:

- **Codice a barre**: richiederebbe scansione manuale uno alla volta in cassa, incompatibile con l'obiettivo di velocità.
- **BLE beacon**: tag troppo costosi per essere applicati a capi venduti.
- **NFC HF**: portata troppo corta per varchi e scaffali smart.

### **Architettura del singolo punto vendita**

Per ciascun punto vendita si prevede:

- **2 reader UHF a 4 antenne ciascuno** ai varchi di uscita per antitaccheggio (configurati a potenza ridotta per evitare letture in profondità nel negozio).
- **1 reader UHF "smart shelf"** dietro ogni scaffale (4-8 a negozio) per inventario continuo dello stock esposto.
- **1 reader UHF a basso campo** integrato nel piano cassa per lettura simultanea del cestino.
- **1 reader handheld** per inventario notturno e ricerca di articoli specifici.
- **1 gateway/controller locale** (industrial PC fanless) che esegue:
  * il **middleware RFID** (filtraggio, deduplica, smoothing);
  * il **client MQTT** verso il broker centrale;
  * un **buffer locale persistente** per gestire la **caduta del link** verso il cloud.

I reader si collegano al gateway via **Ethernet PoE+** su una **VLAN dedicata RFID**, separata dalla VLAN office del negozio.

### **Architettura di backend**

Il backend è **cloud-based** e comprende:

- **Broker MQTT** (es. **EMQX** o **HiveMQ**) ridondato su due AZ.
- **Stream processor** (es. **Apache Kafka + Kafka Streams** o **AWS Kinesis**) che ingerisce gli eventi MQTT e li **persiste**.
- **Data lake** per analytics storiche.
- **API gateway REST** per l'integrazione con l'**ERP** e l'**e-commerce**.
- **Dashboard web** per i punti vendita.

### **Schema di subnetting (esempio)**

| Sottorete | VLAN | Indirizzamento | Note |
|---|---|---|---|
| Reader RFID negozio Milano1 | VLAN 100 | 10.10.100.0/24 | gateway 10.10.100.1 |
| Office negozio Milano1 | VLAN 200 | 10.10.200.0/24 | dipendenti, casse |
| Voce | VLAN 300 | 10.10.30.0/24 | telefonia |
| Tunnel VPN verso cloud | — | 10.250.0.0/16 | tutti i negozi |

Tra il gateway e il cloud si stabilisce una **VPN IPsec** site-to-site permanente: tutto il traffico MQTT del negozio passa **cifrato** sul tunnel.

### **Topic MQTT**

```
shop/<negozio_id>/<area>/<reader_id>/letture
shop/<negozio_id>/<area>/<reader_id>/stato
shop/<negozio_id>/<area>/<reader_id>/config
shop/<negozio_id>/cassa/<cassa_id>/transazioni
shop/<negozio_id>/varco/<varco_id>/allarmi
```

### **Esempio di messaggi JSON**

*Lettura di un capo a scaffale:*

```json
{
  "eventTime": "2025-09-12T10:32:15.234Z",
  "eventType": "shelf_inventory",
  "epc": "urn:epc:id:sgtin:8004680.012345.62852",
  "shop": "MIL01",
  "reader": "shelf-uomo-camicie-3",
  "antenna": 2,
  "rssi": -48,
  "product": {
    "gtin": "08004680123459",
    "model": "Camicia Aurora",
    "size": "M",
    "color": "blu"
  }
}
```

*Allarme antitaccheggio al varco:*

```json
{
  "eventTime": "2025-09-12T18:45:02.001Z",
  "eventType": "eas_alarm",
  "epc": "urn:epc:id:sgtin:8004680.012345.62852",
  "shop": "MIL01",
  "varco": "varco-uscita-1",
  "soldStatus": "NOT_SOLD",
  "operatorAck": false
}
```

*Comando di disattivazione tag dalla cassa:*

```json
{
  "eventTime": "2025-09-12T18:30:00.000Z",
  "eventType": "tag_deactivate",
  "epcs": [
    "urn:epc:id:sgtin:8004680.012345.62852",
    "urn:epc:id:sgtin:8004680.012345.62853"
  ],
  "transactionId": "TX-2025-09-12-MIL01-00345"
}
```

### **Pseudocodice del firmware del gateway**

```text
loop forever:
    eventi = leggi_da_reader_via_LLRP()      // batch da tutti i reader
    eventi_filtrati = filtra_per_RSSI(eventi, soglia=-65)
    eventi_dedup = aggrega_per_EPC_e_finestra(eventi_filtrati, finestra=500ms)
    for evt in eventi_dedup:
        evt.product = lookup_articolo_da_EPC(evt.epc)         // cache locale
        evt.direction = inferisci_direzione(evt.reader, evt.antenna, storia)
        json_msg = serializza(evt)
        if connessione_cloud_ok:
            mqtt_publish(topic_di(evt), json_msg, qos=1)
        else:
            buffer_locale.append(json_msg)
    if connessione_cloud_ok and not buffer_locale.vuoto():
        flush_buffer_su_cloud(buffer_locale)
```

### **Sicurezza e privacy**

- **Crittografia** del traffico MQTT con **TLS 1.3** sul tunnel VPN.
- **Autenticazione** dei gateway al broker tramite **certificati X.509** mutual-TLS.
- I **tag UHF** dei capi sono **kilati alla cassa** (`Kill command` con password) al momento del pagamento, prima di consegnare il capo al cliente. Questo:
  * disabilita il tag in modo **permanente**;
  * impedisce il **tracking** del cliente fuori dal negozio;
  * elimina il rischio di **falsi allarmi** se il cliente rientra in negozio in seguito;
  * è una misura di **privacy by design** richiesta dal GDPR.
- **Logo RFID** visibile al cliente nei punti di lettura (varchi, casse) con informativa.
- **Log centralizzato** delle letture per **audit** e per gestione dei **diritti dell'interessato** (richieste di cancellazione GDPR).

### **Considerazioni finali**

Si è scelta una soluzione **edge-cloud ibrida**: il gateway locale gestisce la latenza-critical logic (allarmi antitaccheggio in tempo reale), mentre il backend cloud gestisce analytics e replenishment. Questa separazione di responsabilità garantisce **continuità operativa** anche in caso di interruzione del link Internet (modalità degradata: il negozio continua a vendere, gli eventi vengono accumulati e sincronizzati al ripristino del link).

> [Torna alla dispensa principale RFID](../archrfid.md)
