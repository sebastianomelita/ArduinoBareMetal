> [Torna alla dispensa principale RFID](../archrfid.md)
> [Torna a reti di sensori](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/sensornetworkshort.md)

# **Classificazione per frequenza di lavoro**

La scelta della **frequenza** di lavoro è la **prima decisione progettuale** in un sistema RFID, perché determina **portata**, **velocità di lettura**, **comportamento in presenza di liquidi e metalli** e **costo dei tag**.

![alt text](../img/rfid_gerarchia.png)

| Banda | Frequenza | Principio | Portata tipica | Velocità | Comportamento su metallo/acqua | Esempi |
|---|---|---|---|---|---|---|
| **LF** | 125-134 kHz | Induttivo | < 10 cm | bassa (~1 kbps) | Ottimo: trapassa | Microchip animali, immobilizer auto |
| **HF / NFC** | 13.56 MHz | Induttivo | < 10 cm (NFC), fino 1 m (HF) | media (~106-848 kbps) | Buono | Pagamenti, ticketing, passaporti, biglietti, controllo accessi |
| **UHF** | 860-960 MHz | Backscatter | 1-12 m | alta (~640 kbps) | Pessimo: riflesso/assorbito | Logistica, retail, antitaccheggio |
| **Microonde** | 2.45 GHz | Backscatter | 1-2 m | molto alta | Pessimo | Telepass, tag attivi |

## **LF (Low Frequency, 125-134 kHz)**

È la frequenza **più antica** dell'RFID. Caratteristiche:

- **Portata** brevissima (qualche centimetro).
- **Immunità** quasi totale a metalli, liquidi e tessuti biologici (il campo magnetico a bassa frequenza penetra praticamente tutto).
- **Velocità** di trasferimento dati molto bassa.
- **Tag economici** ma fisicamente **più grandi** (la bobina deve avere molte spire).

Casi d'uso **dove queste caratteristiche sono vincenti**:

- **Identificazione animali**: il microchip sottocutaneo (FDX-B, ISO 11784/11785) deve funzionare attraverso pelle, tessuti e pelo dell'animale.
- **Immobilizer automobilistici**: la chiave dell'auto contiene un tag LF letto da un'antenna intorno al blocchetto di accensione.
- **Controllo accessi industriale** in ambienti sporchi (concerie, fabbriche metalmeccaniche) dove HF/UHF non funzionerebbero.

## **HF (High Frequency, 13.56 MHz) e NFC**

È la frequenza più **versatile** per l'identificazione "di prossimità":

- **Portata** corta (max 10 cm per NFC, fino a 1 m per HF "vicinity").
- **Velocità** medio-alta sufficiente per gestire **autenticazione crittografica**.
- **Standard maturi**: ISO 14443 (proximity, < 10 cm), ISO 15693 (vicinity, fino 1 m), ISO 18092 (NFC).
- **Tag** in formato **carta di credito**, **adesivo sottile**, **portachiavi**.
- Compatibilità con **smartphone**: tutti gli smartphone moderni hanno un'antenna NFC, il che rende HF la frequenza preferita per **applicazioni rivolte all'utente finale**.

**NFC** (Near Field Communication) è un **sottoinsieme** dell'HF a 13.56 MHz, standardizzato per garantire l'**interoperabilità** tra dispositivi mobili e tag. Aggiunge la modalità **peer-to-peer** (due smartphone che si scambiano dati avvicinandoli) e la modalità **card emulation** (lo smartphone si comporta come una carta contactless, alla base di Apple Pay e Google Pay).

Casi d'uso:

- **Pagamenti contactless** (carte di debito, smartphone).
- **Trasporti pubblici** (Oyster Card a Londra, Navigo a Parigi, biglietti ATM).
- **Documenti d'identità elettronici** (CIE, passaporto biometrico).
- **Controllo accessi sicuro** (badge aziendali con MIFARE DESFire).
- **Smart poster** e marketing interattivo.

### **Come si inquadra NFC rispetto a RFID?**

L'**NFC** (Near Field Communication, ISO 18092 / NFCIP-1) è una **fonte di confusione** tipica nelle dispense, perché viene presentato a volte come "tecnologia a sé" (smartphone, pagamenti) e altre volte come "RFID HF". In realtà è **entrambe le cose**: a livello fisico è un **sottoinsieme di RFID HF** a 13.56 MHz, ma aggiunge **estensioni** che lo rendono molto più di una semplice carta contactless.

NFC condivide con l'RFID HF "puro" la **frequenza** (13.56 MHz) e il **principio fisico** (accoppiamento induttivo near-field). La base normativa di NFC (ISO 18092) cita esplicitamente la **compatibilità** con i preesistenti **ISO 14443** (proximity card) e **ISO 15693** (vicinity card): un reader NFC può leggere senza problemi una qualunque carta MIFARE o un tag ISO 15693.

Cosa **aggiunge NFC** rispetto al "puro" RFID HF:

- **Distanza ridotta a 4 cm**: non è un limite tecnico ma una **scelta di sicurezza by design**. Il corto raggio è una contromisura intrinseca contro l'**eavesdropping** e contro le letture accidentali in scenari come i pagamenti.
- **Tre modalità operative** invece di una sola:
  * **Reader/Writer**: lo smartphone si comporta come un reader RFID HF e legge tag NDEF (es. smart poster, etichette in un museo).
  * **Card Emulation (HCE)**: lo smartphone si fa passare **per un tag** davanti a un POS. È il meccanismo dietro **Apple Pay**, **Google Pay** e i biglietti elettronici dei trasporti urbani sullo smartphone.
  * **Peer-to-Peer**: due smartphone si scambiano dati avvicinandoli. Modalità oggi quasi obsoleta, sostituita dall'**handover** verso Bluetooth/WiFi.
- **Formato dati standardizzato NDEF** (NFC Data Exchange Format): un wrapping che permette a un tag di contenere un **URL**, una **vCard**, un **comando di pairing WiFi**, ecc. con sintassi nota a tutti gli smartphone, garantendo l'**interoperabilità** universale.
- **Ecosistema mobile**: l'**NFC Forum** (industry consortium) ha standardizzato l'ecosistema e oggi **tutti gli smartphone moderni** integrano nativamente un controller NFC.

#### **Differenze pratiche rispetto a "RFID HF puro"**

| Aspetto | RFID HF "classico" (ISO 15693) | NFC (ISO 18092) |
|---|---|---|
| **Frequenza** | 13.56 MHz | 13.56 MHz (identica) |
| **Distanza** | fino a 1 m | < 4 cm (per design) |
| **Asimmetria** | reader sempre attivo, tag sempre passivo | entrambi i lati possono essere attivi (P2P) o passivi |
| **Standard dati** | proprietario / dipende dall'applicazione | NDEF (standardizzato dall'NFC Forum) |
| **Card Emulation** | non prevista | sì, è una delle modalità chiave |
| **Integrazione mobile** | rara, richiede hardware dedicato | nativa in tutti gli smartphone |
| **Casi d'uso tipici** | controllo accessi industriale, biblioteche, biglietti riusabili | pagamenti, biglietti trasporti, smart poster, pairing |

## **UHF (Ultra High Frequency, 860-960 MHz)**

È la frequenza **dominante** nella **logistica**:

- **Portata** lunga (fino a 12 m con reader ad alta potenza e tag ottimizzati).
- **Lettura simultanea** di centinaia di tag al secondo (importante per leggere un pallet intero in un colpo solo).
- **Tag economici** in formato **inlay** (etichetta adesiva con antenna stampata).
- **Banda** non armonizzata globalmente: in Europa **865-868 MHz** (ETSI EN 302 208), negli USA **902-928 MHz** (FCC), in Giappone 916-921 MHz.
- **Sensibilità** ai **metalli** (riflessione) e ai **liquidi** (assorbimento). Esistono tag **on-metal** progettati per essere applicati su superfici metalliche.

Lo standard di riferimento è **EPC Gen2 v2** (ISO 18000-63) gestito da **GS1**. Il tag contiene un **EPC** (Electronic Product Code), tipicamente di 96 bit, che identifica univocamente l'oggetto.

Casi d'uso:

- **Magazzino e logistica**: ricezione merce, inventario, picking.
- **Retail**: tracciamento dei capi di abbigliamento dalla produzione allo scaffale, antitaccheggio integrato.
- **Cantieri** e **manufacturing**: tracciamento utensili, materie prime, work-in-progress.
- **Telepass** autostradali (in molti casi tag attivi o semi-passivi a UHF/microonde).

## **Microonde (2.45 GHz)**

Banda meno usata, riservata a:

- **Tag attivi** per **RTLS** in cui si richiede maggiore velocità di lettura o tag con sensori complessi.
- **Telepass** in alcune realizzazioni (sebbene la tendenza sia migrare verso UHF a 5.8 GHz in DSRC).
- Applicazioni **militari** e **industriali** specializzate.

> [Torna alla dispensa principale RFID](../archrfid.md)
