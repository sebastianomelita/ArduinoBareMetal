> [Torna alla dispensa principale RFID](../archrfid.md)
> [Torna a reti di sensori](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/sensornetworkshort.md)

# **Sicurezza e privacy nell'RFID**

L'RFID, per le caratteristiche stesse del mezzo (radio non confinata) e dei tag (logica minimale, no batteria), pone sfide di sicurezza **specifiche**.

## **Minacce principali**

- **Eavesdropping** (intercettazione): un attaccante con un'antenna direttiva può **leggere un tag UHF a 30-50 m** (molto più della distanza di lettura nominale del reader). Particolarmente grave per tag che memorizzano dati personali.
- **Clonazione**: i tag low-end (es. EM4100 LF, MIFARE Classic HF) hanno **memoria leggibile e riscrivibile** senza autenticazione. Un attaccante può leggere il tag della vittima e scrivere lo stesso UID/EPC su un tag vergine, ottenendo un **clone funzionalmente identico**.
- **Tracking** (sorveglianza): se un utente porta con sé tag UHF persistenti (es. tag cuciti nei vestiti acquistati al retail), può essere **tracciato** spazialmente da reader malevoli installati nelle vicinanze.
- **Tampering**: il tag può essere **fisicamente rimosso** dall'oggetto e applicato a un altro oggetto fraudolento (problema delle **swap attacks** nel retail).
- **Denial of service**: un trasmettitore malevolo nella stessa banda può **disturbare** la comunicazione tra reader e tag.
- **Replay attack**: registrare la comunicazione tra un tag legittimo e un reader e **riprodurla** in seguito davanti a un altro reader.

## **Contromisure**

Le contromisure si stratificano su livelli diversi:

- **Autenticazione mutua**: tag di fascia alta (MIFARE DESFire EV3, NTAG 424 DNA) implementano protocolli **challenge-response** basati su **AES-128**: il reader manda un nonce, il tag risponde con `AES_K(nonce)`, e solo se la risposta è corretta il reader concede l'accesso. Previene clonazione e replay.
- **Crittografia del payload**: i dati sensibili memorizzati sul tag sono cifrati. La chiave è custodita dal middleware/back-end, non dal reader.
- **Tag con TID univoco**: il **TID** scritto in fabbrica è non modificabile e crittograficamente firmato (es. **NXP UCODE 9** con autenticazione **Crypto-1.5**). Il reader può verificare il TID e rifiutare i cloni.
- **Kill command**: lo standard EPC Gen2 prevede un comando `Kill` protetto da password che **disabilita permanentemente** il tag. Usato per **disabilitare i tag al momento della vendita** in retail (privacy by design).
- **Access password**: alcune operazioni di scrittura sui tag richiedono una password di 32 bit, che deve essere conosciuta solo dal sistema legittimo.
- **Schermatura fisica**: portafogli e custodie **RFID-blocking** (con foglio metallico) impediscono a un attaccante di leggere tag NFC/HF ravvicinato.
- **Rolling codes**: per applicazioni di pagamento e telepass, ogni transazione usa una chiave **diversa** generata dinamicamente, vanificando il replay.

## **Privacy by design**

Il **Garante della Privacy** italiano e il **GDPR** europeo richiedono, per i sistemi RFID che trattano dati personali (carte di lealtà, badge dipendenti, biglietti integrati), di applicare il principio **privacy by design**:

- **minimizzare** i dati memorizzati sul tag (memorizzare solo un identificativo opaco; i dati personali stanno nel back-end protetto).
- prevedere il **kill** del tag quando l'utente lo richiede o al termine del ciclo (es. uscita dal negozio).
- **informare** l'utente della presenza del tag e della sua lettura (logo RFID visibile).
- **registrare i log** dei reader per audit.

Questo aspetto è **eticamente** importante e va citato esplicitamente nelle progettazioni della seconda prova quando si trattano scenari **rivolti al pubblico** (retail, trasporti, sanità).

## **Tabella riassuntiva delle contromisure per minaccia**

| Minaccia | Contromisure principali |
|---|---|
| **Eavesdropping** | crittografia del payload, schermatura fisica, NFC < 4 cm by design |
| **Clonazione** | autenticazione mutua AES, verifica del TID, MIFARE DESFire EV3 |
| **Tracking** | kill del tag alla vendita, identificativo opaco, logo informativo |
| **Tampering** | sigilli a strappo, tag distruttivi all'apertura |
| **Denial of service** | rilevamento jamming, ridondanza dei reader |
| **Replay attack** | rolling codes, nonce + AES challenge-response |

> [Torna alla dispensa principale RFID](../archrfid.md)
