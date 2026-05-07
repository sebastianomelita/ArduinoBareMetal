> [Torna alla dispensa principale RFID](../archrfid.md)
> [Torna a reti di sensori](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/sensornetworkshort.md)

# **Principi fisici dell'RFID**

L'RFID funziona grazie all'**accoppiamento elettromagnetico** tra un'antenna del reader e un'antenna del tag. Esistono **due principi fisici** distinti, che dipendono dalla **frequenza di lavoro** e quindi dalla **lunghezza d'onda** del campo:

- **Accoppiamento induttivo (near-field)**: usato a **LF e HF**. Reader e tag si comportano come due **bobine** di un trasformatore: il reader genera un **campo magnetico variabile** e induce una corrente nella bobina del tag. La distanza di lettura è limitata a **frazioni della lunghezza d'onda** (qualche cm fino a ~1 m). È il principio di **NFC** e di tutte le tessere contactless ISO 14443.
- **Accoppiamento elettromagnetico (far-field) per backscatter**: usato a **UHF e microonde**. Il reader emette un'**onda elettromagnetica** che raggiunge il tag, il quale **non genera** un proprio segnale ma **modula la propria impedenza d'antenna** (commutando tra "match" e "mismatch") riflettendo indietro una porzione dell'onda incidente. Il reader rileva queste variazioni di riflessione e ne estrae i dati. Distanze tipiche: **fino a 10-12 m** in spazio libero.

Questa differenza è **fondamentale** per la progettazione: il **near-field** è **immune** alle interferenze su lunghe distanze ma ha portata corta e richiede un **avvicinamento esplicito** dell'oggetto al reader; il **far-field** consente letture di **massa** a distanza ma soffre di **riflessioni** (multipath), assorbimenti da parte di **liquidi** (acqua, corpo umano) e schermature da parte di **metalli**.

## **Alimentazione del tag passivo**

Il **tag passivo** non ha batteria. La sua alimentazione è **prelevata direttamente dal campo elettromagnetico** del reader tramite un circuito di **rectenna** (antenna + raddrizzatore + condensatore di accumulo). Questo meccanismo, detto **energy harvesting RF**, è alla base del **bassissimo costo** del tag (pochi centesimi di euro per i tag UHF EPC Gen2) e della sua **durata illimitata** (non ci sono parti attive che si esauriscono).

Il **prezzo** di questa scelta è:

- il tag ha pochissima energia disponibile (tipicamente < 100 µW), per cui può eseguire solo logica **estremamente semplice** (un microcontroller dedicato a basso consumo).
- la **distanza di lettura** è limitata dall'**inverse-square law** del campo: raddoppiando la distanza, l'energia disponibile al tag si riduce di un fattore 4 (near-field: di un fattore 64, perché il campo magnetico decade con il cubo della distanza).
- il tag **non può iniziare** una comunicazione: parla solo se interrogato. Non può quindi fare polling autonomo, non può "svegliarsi" autonomamente, non può inviare allarmi.

## **Tag attivi e semi-passivi**

Per superare i limiti dei tag passivi sono nati i **tag attivi** e **semi-passivi**:

- **Tag attivo**: dotato di **batteria** propria (litio, durata 3-10 anni) e capace di **trasmettere autonomamente**. Può funzionare come un piccolo **beacon RFID** che annuncia periodicamente la propria presenza, raggiungendo distanze di **decine o centinaia di metri**. Costo unitario molto più alto (5-50 €). Usato in tracciamento container, **RTLS** (Real Time Location System) industriali, telepass autostradali (in realtà semi-passivi).
- **Tag semi-passivo (BAP, Battery-Assisted Passive)**: ha una **batteria** che alimenta solo l'**elettronica del tag** (ad esempio sensori di temperatura, accelerometri), ma la **comunicazione radio** continua ad avvenire per **backscatter** del segnale del reader. Compromesso utile per applicazioni di **cold chain** (catena del freddo) dove il tag deve **registrare la temperatura** anche in assenza di reader.

## **Confronto sintetico**

| Tipo di tag | Batteria | Inizia comunicazione | Portata tipica | Costo | Casi d'uso |
|---|---|---|---|---|---|
| **Passivo** | No | No | qualche cm – 12 m | 0.05 – 1 € | Logistica, retail, controllo accessi |
| **Semi-passivo (BAP)** | Sì (solo elettronica) | No (risponde via backscatter) | come passivo + sensori | 5 – 30 € | Cold chain, monitoring ambientale |
| **Attivo** | Sì (intera radio) | Sì (beacon autonomo) | decine – centinaia di m | 5 – 50 € | RTLS, tracciamento container |

> [Torna alla dispensa principale RFID](../archrfid.md)
