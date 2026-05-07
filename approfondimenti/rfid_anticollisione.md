> [Torna alla dispensa principale RFID](../archrfid.md)
> [Torna a reti di sensori](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/sensornetworkshort.md)

# **Protocolli di accesso al canale (anticollisione)**

Il problema dell'**accesso al canale** in un sistema RFID è radicalmente diverso da quello di una rete di sensori IP, BLE o WiFi:

- nelle reti IoT classiche, **tutti i nodi sono attivi** e la sfida è **coordinarli** affinché parlino senza collidere.
- nell'RFID **passivo**, il **reader è l'unico iniziatore**: i tag rispondono solo se interrogati. Il problema non è quindi "chi parla quando" ma "**come distinguere centinaia di tag che rispondono tutti insieme**" alla stessa interrogazione.

Questo problema si chiama **anticollisione**.

## **Anticollisione Slotted ALOHA (UHF EPC Gen2)**

Lo standard **EPC Gen2** usa un meccanismo **probabilistico** noto come **Q-protocol**, una variante dello **Slotted ALOHA** dinamico:

1. Il reader trasmette un comando `Query` con un parametro `Q` (4 bit, valore 0-15).
2. Ogni tag estrae un **numero casuale** `s` nell'intervallo `[0, 2^Q - 1]`. Questo numero è il "**numero di slot**" che il tag deve attendere prima di rispondere.
3. Il reader incrementa di slot in slot un contatore con `QueryRep`. Quando il contatore raggiunge il valore `s` di un certo tag, quel tag risponde con un **RN16** (numero casuale di 16 bit).
4. **Tre casi possibili**:
    - **Slot vuoto**: nessun tag risponde → il reader passa allo slot successivo.
    - **Slot con un solo tag**: il reader riceve l'RN16 e rispedisce un `ACK(RN16)`. Il tag a questo punto trasmette il proprio **PC + EPC + CRC**, viene "**inventarizzato**" e rimane silente per il resto del round.
    - **Collisione** (più tag rispondono): il reader rileva un segnale corrotto e passa avanti.
5. Se ci sono **troppe collisioni**, il reader **aumenta** Q (più slot) per il prossimo round; se ci sono **troppi slot vuoti**, lo **diminuisce** (meno slot, round più veloce).

Questo meccanismo permette di leggere **centinaia di tag al secondo**, ma è **statistico**: in scenari molto densi (migliaia di tag) qualche lettura può richiedere **più round** per essere completata. Il middleware deve esserne consapevole e implementare strategie di **lettura ridondante**.

## **Anticollisione Binary Tree (HF, ISO 14443/15693)**

Negli standard HF ISO 14443 e 15693 si usa un meccanismo **deterministico** ad **albero binario**:

1. Il reader invia un comando di interrogazione.
2. Tutti i tag rispondono trasmettendo il proprio UID **bit a bit**, sincronizzati.
3. Quando il reader rileva una **collisione** in un certo bit (perché alcuni tag rispondono `0` e altri `1`), **biforca** la ricerca: continua interrogando solo i tag con prefisso `…0` e successivamente quelli con `…1`.
4. Il processo si ripete fino a isolare ciascun tag.

L'albero binario è **più lento** dello slotted ALOHA per popolazioni grandi, ma è **deterministico**: in tempo finito **garantisce** di leggere **tutti** i tag presenti. Per questo è preferito in scenari di **autenticazione** (pagamenti, controllo accessi) dove la lettura deve essere **affidabile** anche se più lenta.

## **Confronto sintetico Slotted ALOHA vs Binary Tree**

| Aspetto | Slotted ALOHA (Q-protocol) | Binary Tree |
|---|---|---|
| **Standard tipico** | UHF EPC Gen2 (ISO 18000-63) | HF ISO 14443 / 15693 |
| **Tipo di algoritmo** | Probabilistico | Deterministico |
| **Velocità** | Molto alta (centinaia/sec) | Inferiore |
| **Garanzia di lettura totale** | Statistica | Garantita in tempo finito |
| **Caso d'uso ideale** | Lettura di massa in logistica | Autenticazione singola affidabile |

## **Bande di frequenza e canalizzazione UHF**

In Europa la banda RFID UHF 865-868 MHz è suddivisa in **15 canali** larghi 200 kHz. Per evitare interferenze tra reader vicini (**dense reader environment**), i reader devono:

- usare il meccanismo di **LBT** (Listen Before Talk), simile al CSMA, per verificare che il canale sia libero prima di trasmettere.
- **saltare** tra canali (**channel hopping**) in modo casuale.
- rispettare il **duty cycle** stabilito dalla normativa (max 100% con LBT, 10% senza LBT su 200 kHz).

Questo è un altro **aspetto critico**: in un magazzino con **decine di reader** si deve fare un **piano dei canali** (frequency planning) e dimensionare correttamente il **timing degli inventory** per evitare che reader adiacenti si "rubino" i tag a vicenda.

> [Torna alla dispensa principale RFID](../archrfid.md)
