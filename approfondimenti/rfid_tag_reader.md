> [Torna alla dispensa principale RFID](../archrfid.md)
> [Torna a reti di sensori](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/sensornetworkshort.md)

# **Tag e Reader RFID**

## **Caratteristiche dei tag**

### **Composizione interna**

Un tag RFID passivo è composto da:

- **Antenna**: ne determina **dimensioni**, **frequenza** di lavoro, **direttività** e quindi **distanza di lettura**. Le antenne **dipolo** UHF sono lunghe ~16 cm; le antenne HF sono **bobine** stampate.
- **Chip integrato (IC)**: contiene la **memoria** (32 bit - 64 KB), la **logica** di anticollisione, il **modulatore di backscatter** (UHF) o il **modulatore di carico** (HF/LF) e il **rectifier** per l'alimentazione.
- **Substrato**: pellicola plastica (PET, PVC), carta o materiale ceramico (per tag on-metal).

### **Memoria del tag**

I tag UHF EPC Gen2 hanno **quattro banchi di memoria**:

| Banco | Contenuto | Note |
|---|---|---|
| **Reserved (00)** | password di **kill** e di **access** | scrivibili, possono essere protetti |
| **EPC (01)** | codice EPC dell'oggetto + CRC + PC | tipicamente 96 bit, leggibile da chiunque |
| **TID (10)** | identificativo univoco del **chip**, scritto dal produttore | non riscrivibile, utile per anti-clonazione |
| **User (11)** | memoria utente libera | da 0 a 8 KB a seconda del modello |

Il **TID** è particolarmente importante per la **sicurezza**: è scritto in fabbrica e **non può essere modificato**, quindi può essere usato per verificare l'**autenticità** del tag (mentre l'EPC potrebbe essere copiato su un tag-clone, il TID no).

### **Tipologie commerciali**

I formati commerciali più diffusi sono:

- **Inlay**: chip + antenna su pellicola PET, pronto per essere incapsulato in un'**etichetta adesiva** stampabile. Il più economico, usato in retail e logistica.
- **Smart label**: inlay laminato in un'etichetta di carta stampabile con stampante termica.
- **Hard tag**: incapsulato in plastica rigida, riutilizzabile (es. tag antitaccheggio sui capi).
- **On-metal tag**: con strato isolante o ceramico, progettato per essere applicato su superfici metalliche.
- **Wearable**: braccialetti per ospedali, festival, parchi divertimento.
- **Impiantabile**: microchip in capsula di vetro biocompatibile per animali.

## **Reader RFID**

Il **reader** (o **interrogatore**) è il dispositivo **attivo** dell'architettura RFID. Contiene:

- un **trasmettitore** RF capace di erogare la potenza necessaria a illuminare i tag (fino a 2 W ERP in UHF Europa).
- uno o più **ricevitori** in grado di rilevare il debole segnale di backscatter dei tag.
- un **microcontroller** (o SoC ARM Cortex-A) che esegue il protocollo di anticollisione e gestisce la comunicazione verso il middleware.
- una o più **porte di antenna** (4-16 nei reader fissi industriali) per coprire varchi multipli o aree estese.
- **interfacce di rete**: Ethernet (RJ45, PoE), WiFi, USB, RS232/RS485, GPIO per integrazione con PLC industriali.

### **Reader fissi vs handheld**

I reader si dividono in due categorie principali:

- **Reader fissi**: installati in posizioni note (varchi, gate logistici, scaffali smart, nastri trasportatori). Hanno **potenza alta**, **antenne esterne** e **molteplici porte**. Sono i **listener** del sistema: stanno costantemente in ascolto di tag che attraversano il loro campo. **Esempio**: Impinj Speedway R420, Zebra FX9600.
- **Reader handheld**: dispositivi mobili con **batteria**, **antenna integrata** e spesso un **lettore di codici a barre** abbinato. Usati per inventario manuale, picking, ricerca di un singolo tag. **Esempio**: Zebra MC3300R, Honeywell IH40.
- **Reader integrati in smartphone**: tutti gli smartphone moderni hanno un **reader NFC** integrato (HF a 13.56 MHz). Per UHF esistono custodie e add-on con reader UHF (sled).

### **Antenne**

L'**antenna** è il componente **più critico** per la performance del sistema. Va scelta in base a:

- **Polarizzazione**: **lineare** (massima portata in una direzione precisa, ma il tag deve essere allineato) oppure **circolare** (portata leggermente inferiore ma indipendente dall'orientamento del tag — la scelta di default per logistica generica).
- **Guadagno**: tipicamente 6-9 dBi per antenne fisse UHF. Più alto = più portata ma campo più stretto.
- **Beamwidth**: l'angolo del **lobo principale**. Antenne strette (30°) per varchi puntuali, antenne larghe (70°-90°) per coprire un'area.
- **Near-field vs far-field**: per applicazioni come scaffali smart o casse retail si usano antenne **near-field** UHF che funzionano per accoppiamento induttivo a corto raggio, evitando di leggere tag fuori area.

### **Posizionamento delle antenne**

In una progettazione tipica si distinguono:

- **Varchi (portal)**: due antenne montate su un telaio ai lati di una porta di magazzino. Il tag viene letto mentre passa nel mezzo. Il **trigger** di inizio lettura è spesso una **fotocellula** o una **barriera ottica** che rileva il passaggio fisico, in modo da delimitare la finestra di lettura ed evitare letture di tag fermi nelle vicinanze.
- **Gate per nastri trasportatori**: antenne installate sopra e sotto il nastro, sincronizzate con la velocità del nastro per individuare la posizione esatta del tag.
- **Scaffali smart**: antenne distribuite sui ripiani per inventario continuo (smart shelf).
- **Cabine di lettura (tunnel)**: tunnel completamente schermato in cui il pallet entra per la registrazione completa del contenuto (utile per evitare false letture di pallet adiacenti).
- **Overhead**: antenne montate sul soffitto per coprire ampie aree (es. ingresso negozio).

> [Torna alla dispensa principale RFID](../archrfid.md)
