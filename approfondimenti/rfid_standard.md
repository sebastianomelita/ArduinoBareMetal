> [Torna alla dispensa principale RFID](../archrfid.md)
> [Torna a reti di sensori](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/sensornetworkshort.md)

# **Standard RFID**

L'RFID è coperto da una galassia di standard. I più rilevanti per la **seconda prova** sono:

| Standard | Banda | Applicazione | Portata |
|---|---|---|---|
| **ISO 11784/11785** | LF (134.2 kHz) | Identificazione animali (FDX-B) | < 30 cm |
| **ISO 14443 A/B** | HF (13.56 MHz) | Carte contactless di prossimità (NFC card emulation, MIFARE, EMV contactless) | < 10 cm |
| **ISO 15693** | HF (13.56 MHz) | Vicinity card (libri, controllo accessi a media distanza) | fino 1 m |
| **ISO 18092 (NFCIP-1)** | HF (13.56 MHz) | NFC peer-to-peer | < 10 cm |
| **ISO 18000-63 / EPC Gen2 v2** | UHF (860-960 MHz) | Logistica, retail, antitaccheggio | 1-12 m |
| **ISO 18000-3** | HF | RFID HF "vendor neutral" | < 1 m |
| **ISO 24730** | varie | RTLS (Real Time Location Systems) | > 30 m |

La **scelta dello standard** vincola tutto il resto del progetto: tag, reader, middleware, ecologia di fornitori. È uno degli **aspetti critici** da motivare in fase di progetto.

## **EPC Gen2: anatomia di una lettura**

Per la seconda prova è utile conoscere il funzionamento di **EPC Gen2**, lo standard UHF più diffuso. Il reader segue un ciclo composto da tre fasi:

1. **Select**: il reader invia un comando per selezionare il sottoinsieme di tag su cui operare (es. solo i tag con un certo prefisso EPC).
2. **Inventory**: il reader esegue una **slotted ALOHA** per identificare uno alla volta tutti i tag selezionati. Manda un comando `Query` con un parametro `Q` che determina il numero di slot temporali (2^Q). Ogni tag estrae un numero casuale tra 0 e 2^Q-1 e risponde solo nel proprio slot. Le collisioni vengono risolte aggiustando dinamicamente Q.
3. **Access**: una volta identificato un tag, il reader può leggerne/scriverne la memoria, autenticarsi con password, eseguire un kill.

L'inventory di EPC Gen2 è straordinariamente veloce: un buon reader può identificare **fino a ~700 tag/secondo**.

## **Standard EPCIS**

**EPCIS** (Electronic Product Code Information Services) è uno standard **GS1** che definisce **come** rappresentare e scambiare gli eventi RFID di business tra aziende. Definisce quattro tipi di evento:

- **ObjectEvent**: un singolo oggetto è stato osservato (es. "tag 123 letto al varco di ricezione").
- **AggregationEvent**: un oggetto è stato associato a un contenitore (es. "10 cartoni caricati su questo pallet").
- **TransactionEvent**: un oggetto è stato associato a una transazione di business (es. "questo articolo è parte dell'ordine #4567").
- **TransformationEvent**: un input è stato trasformato in output (es. "10 kg di farina + 5 L di acqua sono diventati 15 kg di pasta").

Per la **seconda prova**, è sufficiente sapere che l'EPCIS è il "vocabolario standard" con cui le aziende si scambiano informazioni RFID nelle filiere globali.

## **Standard EPC: come si dà significato a un identificativo**

Lo standard **EPC** di GS1 introduce una **semantica strutturata** sopra l'identificativo binario. Un EPC SGTIN-96 (Serialized Global Trade Item Number) è composto da:

| Campo | Bit | Significato |
|---|---|---|
| Header | 8 | Tipo di EPC (es. 0x30 = SGTIN-96) |
| Filter | 3 | Tipo di unità logistica (item, case, pallet) |
| Partition | 3 | Indica come dividere i bit successivi tra Company Prefix e Item Reference |
| Company Prefix | 20-40 | Codice azienda assegnato da GS1 |
| Item Reference | 4-24 | Codice articolo all'interno dell'azienda |
| Serial Number | 38 | Numero seriale del singolo oggetto |

Esempio di rappresentazione URI:

```
urn:epc:id:sgtin:0614141.012345.62852
                  │       │      │
                  │       │      └── numero seriale del singolo capo
                  │       └────────  codice articolo (modello)
                  └────────────────  prefisso azienda GS1
```

Lo stesso codice si scrive in formato **binario tag** (96 bit, ciò che è effettivamente memorizzato sul tag) e in formato **EPC pure identity URI** (forma testuale leggibile usata dalle applicazioni). Il middleware si occupa della **conversione** tra le due rappresentazioni.

## **Protocollo LLRP**

Il **protocollo standard** tra reader e middleware è **LLRP** (Low Level Reader Protocol, EPCglobal/ISO 19762), un protocollo binario su TCP/IP che permette al middleware di **configurare** i reader (potenza, antenne attive, modalità di inventory) e ricevere le letture in **formato standard**, indipendentemente dal vendor.

In alternativa, molti reader moderni offrono direttamente:

- una **interfaccia REST** o **WebSocket** per integrazioni semplici.
- un **client MQTT** integrato che pubblica le letture su un broker.

> [Torna alla dispensa principale RFID](../archrfid.md)
