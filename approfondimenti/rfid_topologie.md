> [Torna alla dispensa principale RFID](../archrfid.md)
> [Torna a reti di sensori](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/sensornetworkshort.md)

# **Topologie di lettura RFID**

Analogamente al caso del BLE — in cui distinguevamo tra **schema a beacon fisso** e **schema a scanner fisso** — nei sistemi RFID si possono individuare topologie analoghe, che corrispondono a **diverse responsabilità** dei componenti del sistema.

## **Topologia a reader fisso**

È l'approccio **dominante** nei sistemi RFID logistici. I **reader sono fissi**, posizionati in **punti strategici** noti a sistema (varchi, gate, scaffali smart) ed **etichettati univocamente**. I **tag** invece **si muovono** sugli oggetti, i quali entrano ed escono dai campi di lettura dei reader.

![alt text](../img/topologia_reader_fisso.png)

Si noti che in questo approccio:

- i **reader fissi** sono **collegati alla rete di distribuzione** (Ethernet/PoE/WiFi).
- i **tag** non hanno nessun collegamento di rete: sono **dispositivi isolati** la cui posizione viene inferita esclusivamente **dal punto in cui sono stati letti**.
- la **responsabilità** dell'elaborazione e del collegamento al server è **del reader** (o del gateway che lo serve).

La particolarità di questo approccio è:

- un **numero contenuto** di reader (relativamente costosi, 500-3000 €) installati nelle zone strategiche.
- un **numero potenzialmente enorme** di tag a basso costo (5 cent - 1 € l'uno) sugli oggetti.
- la **localizzazione** del tag è **a granularità di varco**: si sa che il tag X è passato per il varco Y al tempo Z, ma **non** se ne conosce la posizione tra un varco e l'altro.

**Esempio**: in un magazzino di e-commerce, si installano reader UHF su:

- **varchi di ricezione**: registrano l'ingresso delle merci dai fornitori.
- **stazioni di picking**: registrano la presa dell'articolo da parte dell'operatore.
- **varchi di spedizione**: registrano l'uscita verso il cliente.
- **eventuale tunnel di ribaltamento** per la verifica completa del contenuto di un pallet.

Il **WMS** (Warehouse Management System) ricostruisce la **storia** di ogni articolo correlando questi eventi.

## **Topologia a tag attivo**

Quando si vuole **localizzazione continua** invece di lettura **a varco**, si passa ai **tag attivi**: piccoli dispositivi a batteria che **trasmettono** periodicamente un beacon RFID (es. ogni 1-10 secondi). I **reader fissi** distribuiti nell'ambiente captano questi beacon e, tramite **trilaterazione** o **fingerprinting RSSI**, calcolano la posizione del tag.

![alt text](../img/topologia_tag_attivo.png)

Questa topologia è concettualmente analoga allo schema BLE a **scanner fisso**: anche qui i **dispositivi fissi** ascoltano i tag che si spostano. La differenza è che, mentre i beacon BLE sono **economici** e si possono distribuire a centinaia, i tag attivi RFID sono **più costosi** ma offrono **portate maggiori** (decine di metri vs i 5-15 m del BLE).

In molti sistemi **RTLS** moderni l'RFID attivo è stato **sostituito** da:

- **BLE attivo**: tag BLE 5.x con batteria e funzioni di Direction Finding (AoA/AoD) per localizzazione sub-metrica.
- **UWB**: localizzazione di precisione 10-30 cm grazie alla misurazione del **tempo di volo** (ToF) del segnale.

L'RFID attivo rimane competitivo solo in **scenari specifici** (lunga durata della batteria, ambienti con interferenze WiFi/BLE).

## **Topologia a reader mobile (handheld)**

In questo schema il **reader si muove** (handheld in mano all'operatore) e i **tag sono fissi** sugli oggetti. È l'approccio classico di:

- **inventario** periodico in negozi e magazzini: l'operatore "spazza" lo scaffale con il reader e raccoglie tutti gli EPC presenti.
- **ricerca puntuale** di un articolo specifico (modalità "Geiger counter": il reader emette un beep più rapido man mano che ci si avvicina al tag).
- **conta cespiti** in uffici e laboratori.

Si noti che in questo approccio:

- i **tag** sono fermi e isolati.
- il **reader** è mobile e si collega alla rete di distribuzione tramite **WiFi** o **4G/5G**, oppure raccoglie le letture in memoria locale e le scarica al rientro alla **base** via Ethernet/USB.

> [Torna alla dispensa principale RFID](../archrfid.md)
