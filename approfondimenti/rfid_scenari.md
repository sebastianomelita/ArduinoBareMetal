> [Torna alla dispensa principale RFID](../archrfid.md)
> [Torna a reti di sensori](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/sensornetworkshort.md)

# **Confronto con tecnologie alternative per scenari tipici**

Per ciascun caso d'uso tipico, l'RFID è una delle **possibili scelte tecnologiche**. Saper **giustificare la scelta** rispetto alle alternative è uno dei compiti centrali della seconda prova.

![alt text](../img/scelta_tecnologia.png)

## **Scenario 1: tracciamento merci in magazzino**

| Tecnologia | Costo unitario tag | Portata | LoS richiesta | Lettura di massa | Scelta consigliata |
|---|---|---|---|---|---|
| **Codice a barre 1D** | quasi 0 | contatto | Sì | No, uno alla volta | inventario manuale di basso volume |
| **QR code** | quasi 0 | contatto | Sì | No, uno alla volta | beni con tracciabilità all'unità (farmaci, libri) |
| **RFID UHF (EPC Gen2)** | 5-15 cent | 1-12 m | No | Sì, centinaia/sec | **scelta principale** per logistica massiva |
| **BLE beacon** | 3-15 € | 5-30 m | No | Sì ma più lenta | per asset di alto valore, tracciamento continuo |
| **UWB** | 15-50 € | 50-100 m | No | Sì, con localizzazione precisa | RTLS di precisione su asset critici |

**Argomentazione tipica**: per tracciare migliaia di pallet/scatoloni al giorno in transito attraverso varchi di magazzino, **RFID UHF** è imbattibile per **costo del consumabile** e **velocità di lettura di massa**. Il BLE diventa preferibile solo per gli **asset di alto valore** che giustificano un tag attivo e per cui serve una **posizione continua** (es. carrelli elevatori, attrezzature riutilizzabili).

## **Scenario 2: controllo accessi in azienda**

| Tecnologia | Sicurezza | Costo lettore | UX (utente) | Note |
|---|---|---|---|---|
| **Tessera magnetica** | molto bassa | basso | strisciare la tessera | obsoleta, clonabile |
| **RFID LF (EM4100)** | bassa (no crypto) | basso | avvicinare la tessera | usata ancora ma vulnerabile a clone |
| **RFID HF MIFARE Classic** | bassa (Crypto-1 craccato dal 2008) | medio | avvicinare la tessera | da evitare in nuovi progetti |
| **RFID HF MIFARE DESFire EV3** | alta (AES-128) | medio | avvicinare la tessera | **scelta consigliata** per nuovi progetti |
| **NFC su smartphone** | alta (con HCE + Secure Element) | medio | avvicinare lo smartphone | comoda, integrabile con app |
| **Biometria** | molto alta | alto | dito/volto | richiede consenso GDPR esplicito |
| **BLE smart key** | alta | medio | tasca, hands-free | richiede batteria nel token |

**Argomentazione tipica**: per un nuovo impianto di controllo accessi si sceglie **MIFARE DESFire EV3** per la **sicurezza crittografica** (AES-128) e l'**ecosistema standardizzato**, eventualmente **affiancato** da **NFC su smartphone** (con HCE e tokenizzazione lato server) per gli utenti che preferiscono non portare un badge fisico.

## **Scenario 3: pagamenti contactless**

In questo scenario l'**NFC** (RFID HF a 13.56 MHz) è praticamente **monopolista**, perché:

- è **standardizzato globalmente** (ISO 14443, EMV contactless).
- ha un **modello di sicurezza** maturo (Secure Element + tokenization).
- è **integrato in tutti gli smartphone** moderni.
- il **corto raggio** (< 4 cm) è una **caratteristica di sicurezza**, non un limite: previene letture accidentali.

**Le alternative storiche sono cadute**:

- **codici QR dinamici** (modello cinese di Alipay/WeChat Pay) sono usati ma richiedono di aprire un'app e inquadrare un codice, UX inferiore.
- **BLE Beacons** sono stati provati (es. PayPal Beacon) ma abbandonati per problemi di sicurezza e di esperienza utente.

## **Scenario 4: ticketing trasporti**

| Tecnologia | Esempi | Vantaggi | Svantaggi |
|---|---|---|---|
| **RFID HF (Calypso, MIFARE)** | Navigo, Oyster, ATM Milano | Affidabilità, transazione offline, tempo < 300 ms | Costo della carta plastica |
| **NFC con smartphone** | Apple/Google Pay transit | Niente carta fisica, ricarica online | Richiede smartphone e batteria carica |
| **Codice QR** | Trenitalia, biglietti aerei | Niente hardware sui tornelli, stampabile | Più lento, sensibile a riflessi e schermi sporchi |
| **Tag UHF "wearable"** | Festival, parchi divertimento | Hands-free, lettura a distanza | Sicurezza ridotta, costo dispositivo |

**Argomentazione tipica**: per un sistema di ticketing urbano si scelgono **carte HF Calypso o MIFARE DESFire** per l'affidabilità in **ambiente offline** (i tornelli devono validare anche con rete temporaneamente caduta) e tempi di transazione **< 300 ms**. Si **affianca** il sistema con **NFC mobile** e **QR code** come canali alternativi per l'utenza che preferisce non portare la carta.

## **Scenario 5: identificazione paziente in ospedale**

| Tecnologia | Casi d'uso | Note |
|---|---|---|
| **Braccialetto QR/codice a barre** | matching paziente-farmaco al letto | richiede LoS, può strapparsi |
| **Braccialetto RFID HF** | accessi a reparti, identificazione al letto | lettura senza LoS, anche su paziente incosciente |
| **Tag RFID UHF** su sacche di sangue, strumenti chirurgici | tracciabilità fluidi e sterilizzazione | importante per reverse logistics in centrale di sterilizzazione |
| **BLE beacon** su strumenti di alto valore | localizzazione di defibrillatori, pompe infusione | RTLS reparto |
| **NFC su badge personale** | accesso a cartelle cliniche, somministrazioni | doppio fattore: badge + PIN |

In questo scenario è tipica una **architettura ibrida**: HF per identificazione persona, UHF per consumabili e materiali, BLE per RTLS di apparecchiature.

## **Scenario 6: identificazione di animali**

In questo scenario lo standard **ISO 11784/11785** **LF a 134.2 kHz** è **vincolante per legge** in molti Paesi (es. Italia per cani, in alcuni per equini e bovini). La banda LF è imposta perché:

- **trapassa** tessuti biologici, pelo, fango.
- è **immune** a interferenze in ambiente rurale.
- è **lo standard storico** dei lettori veterinari.

Le **alternative** (HF/UHF) non sono adottate perché esiste un **lock-in** internazionale dei lettori veterinari sui 134.2 kHz.

## **Sintesi: come scegliere tra NFC, RFID HF e UHF**

La figura in apertura della sezione riassume la decisione in tre colonne:

- **NFC** quando la traccia coinvolge **interazione con utente finale**, **smartphone**, **pagamenti**, **biglietti elettronici**, **pairing rapido**, **autenticazione fisica con HCE/Secure Element**.
- **RFID HF "puro"** (MIFARE DESFire EV3) quando la traccia parla di **tessere riusabili "tradizionali"** senza coinvolgere lo smartphone (badge dipendenti, tessere biblioteca, biglietti urbani al tornello).
- **UHF EPC Gen2** quando la traccia richiede **lettura di massa** (logistica, antitaccheggio, inventario di magazzino).

> [Torna alla dispensa principale RFID](../archrfid.md)
