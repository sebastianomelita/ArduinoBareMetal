>[Torna all'indice](indexpulsanti.md) >[Soluzioni](soluzioniEserciziPulsanti.md) >[Soluzioni MicroPython](soluzioniEserciziPulsanti-mpy.md)


# Esercizi sui pulsanti - Consegne

Per entrambi gli esercizi valgono le regole delle dispense:

- pulsante collegato con **pull-down** (premuto = HIGH), su Arduino Uno o ESP32, anche in simulazione su Wokwi;
- programma **non bloccante**: niente `delay()` né attese nel `loop()`, che deve poter fare anche altro;
- tempo di debounce di **50 ms**, mai meno;
- le azioni sul fronte vanno scritte nelle callback `doOnRise()` (pressione) e `doOnFall()` (rilascio), anche se una delle due resta vuota.

Prima di scrivere il codice, rileggere le quattro situazioni critiche in [debounce-fenomeni.md](debounce-fenomeni.md): rimbalzi alla pressione, rimbalzi al rilascio, buco a pulsante premuto, spike a riposo.

| Esercizio | Soluzione |
| --- | --- |
| [1. Tempo di pressione](#1-tempo-di-pressione) | [C++](soluzioniEserciziPulsanti.md#1-tempo-di-pressione) · [MicroPython](soluzioniEserciziPulsanti-mpy.md#1-tempo-di-pressione) |
| [2. Conteggio delle pressioni](#2-conteggio-delle-pressioni) | [C++](soluzioniEserciziPulsanti.md#2-conteggio-delle-pressioni) · [MicroPython](soluzioniEserciziPulsanti-mpy.md#2-conteggio-delle-pressioni) |

## 1. Tempo di pressione

Soluzione: [C++](soluzioniEserciziPulsanti.md#1-tempo-di-pressione) · [MicroPython](soluzioniEserciziPulsanti-mpy.md#1-tempo-di-pressione)

Un distributore di bevande dosa lo zucchero in base a **quanto a lungo** si tiene premuto un pulsante: più lunga la pressione, più zucchero. Si vuole realizzare la parte che misura la durata della pressione.

Specifiche:
- un pulsante P1, un LED di pressione e tre LED di colore diverso per le fasce: verde, giallo e rosso;
- il LED di pressione resta acceso finché il pulsante è premuto;
- al **rilascio**, sul monitor seriale viene stampata la durata della pressione in millisecondi;
- la durata viene classificata in tre fasce, stampate insieme al valore e indicate da un LED:
  - meno di 500 ms: **poco zucchero**, LED verde;
  - da 500 ms a 1500 ms: **medio**, LED giallo;
  - oltre 1500 ms: **molto zucchero**, LED rosso;
- il LED della fascia resta acceso fino alla pressione successiva, che spegne tutti e tre i LED;
- una pressione più breve di 50 ms non deve produrre nessuna misura e non deve cambiare i LED.

Richieste:
1. Disegnare il diagramma temporale di una pressione con i rimbalzi alla pressione e al rilascio. Segnare l'istante in cui il programma riconosce la pressione, quello in cui conferma il rilascio, e la durata vera.
2. Scegliere una tecnica di debounce fra quelle delle dispense e motivare la scelta.
3. Scrivere il programma.
4. **Errore di misura.** Con la tecnica "pressione subito, rilascio confermato" il rilascio è riconosciuto 50 ms dopo l'ultimo rimbalzo. Di quanto sbaglia la misura se si calcola la durata nell'istante della conferma? Come si corregge? Cambia qualcosa con la tecnica a livello stabile in due direzioni (`changed()`), che ritarda di 50 ms **sia** la pressione **sia** il rilascio?
5. **Situazioni critiche.** Che cosa stampa il programma se, a pulsante premuto, il contatto si apre per 2 ms (buco)? E se arriva uno spike di 2 ms a pulsante rilasciato? Rispondere per la tecnica scelta e per `changed()`.
6. **Facoltativa.** Far lampeggiare il LED a 2 Hz mentre il pulsante è premuto, senza rendere il programma bloccante, e verificare che la misura della durata non cambi.

Prove di collaudo:
- dieci pressioni di durata diversa, confrontate con un cronometro (o con il tempo della simulazione Wokwi);
- una pressione per ciascuna fascia: si accende il LED giusto, e solo quello;
- una pressione brevissima (un tocco): nessuna misura stampata e LED delle fasce invariati;
- una pressione di qualche secondo: una sola misura, al rilascio.

## 2. Conteggio delle pressioni

Soluzione: [C++](soluzioniEserciziPulsanti.md#2-conteggio-delle-pressioni) · [MicroPython](soluzioniEserciziPulsanti-mpy.md#2-conteggio-delle-pressioni)

All'ingresso di una sala si vuole contare quante persone entrano: l'addetto preme un pulsante a ogni ingresso. Il conteggio deve essere **esatto**, perché serve a non superare la capienza massima.

Specifiche:
- un pulsante P1 per contare, un LED verde e un LED rosso;
- a ogni pressione il contatore aumenta di uno e il nuovo valore viene stampato sul monitor seriale;
- il conteggio avviene al **rilascio**;
- capienza massima di **10 persone**: quando il contatore la raggiunge si accende il LED rosso e si spegne il verde, e le pressioni successive non aumentano più il conteggio;
- una **pressione lunga** (oltre 2 s) azzera il contatore e riaccende il LED verde. Una pressione lunga non conta come ingresso.

Richieste:
1. Spiegare perché un contatore è più esigente di un LED acceso finché il pulsante è premuto. Usare la distinzione fra logica a **livello** e logica a **fronte**: che cosa succede, nei due casi, dopo un evento falso?
2. Scegliere la tecnica di debounce e motivare la scelta, alla luce della tabella "Situazioni reali: quale metodo" in [debounce-fenomeni.md](debounce-fenomeni.md).
3. Scrivere il programma. La pressione lunga va distinta da quella breve **al rilascio**, misurandone la durata come nell'esercizio 1.
4. **Confronto.** Riscrivere il conteggio con la tecnica "pressione subito, rilascio confermato" (sezione 1 di [pulsanti-rilascio.md](pulsanti-rilascio.md)). Descrivere una situazione reale in cui le due versioni danno conteggi diversi, e dire quale delle quattro situazioni critiche la provoca.
5. **Pressioni rapide.** L'addetto preme molte volte di seguito, molto velocemente. Qual è la frequenza massima di pressioni che il programma riesce a contare con un debounce di 50 ms? Che cosa succede alle pressioni più brevi di 50 ms?
6. **Facoltativa.** Aggiungere un secondo pulsante P2 per le **uscite**, che diminuisce il contatore senza scendere sotto zero. Usare un oggetto `Debouncer` di `urutils` per ogni pulsante e spiegare perché ogni pulsante ha bisogno del proprio oggetto.

Prove di collaudo:
- venti pressioni normali: il contatore deve arrivare esattamente a 10 e fermarsi, con il LED rosso acceso;
- una pressione lunga: contatore a zero, LED verde acceso, nessun ingresso contato;
- in simulazione Wokwi i rimbalzi del pulsante sono attivi per default (attributo `bounce`, che li disattiva se vale `"0"`): ripetere la prova delle venti pressioni con i rimbalzi attivi e poi disattivati. Il conteggio deve essere lo stesso; senza debounce, invece, con i rimbalzi attivi il conteggio sbaglia.

## Riferimenti

- [debounce-fenomeni.md](debounce-fenomeni.md): le quattro situazioni critiche, la finestra di progetto, quale metodo usare
- [pulsanti-rilascio.md](pulsanti-rilascio.md): tecniche non bloccanti, `changed()`, lockout
- [urutils.h](librerie/urutils.h): `DiffTimer`, `Debouncer`, `Lockout`

>[Torna all'indice](indexpulsanti.md) >[Soluzioni](soluzioniEserciziPulsanti.md) >[Soluzioni MicroPython](soluzioniEserciziPulsanti-mpy.md)
