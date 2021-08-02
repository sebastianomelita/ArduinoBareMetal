>[Torna a reti di sensori](sensornetworkshort.md#interfaccia-radio)

## **Banda ISM**

Le bande libere sono le frequenze di uso libero, non tutelate, che non richiedono concessioni per il loro impiego. Sono spesso indicate come ISM (Industrial, Scientific and Medical)[Nota 1].

In realtà ISM è un sottogruppo di tutte le frequenze disponibili.
L’uso di tali bande è regolamentato in modo da consentirne l’impiego condiviso ed evitare che un utente o un servizio possa monopolizzare la risorsa.

In tabella un elenco parziale con le principali limitazioni:

<img src="ismband.png" alt="alt text" width="600">

L'ente regolatore, in Italia il Ministero dello Sviluppo Economico (MISE), fissa al cuni criteri per stabilire il diritto di accesso quali la distinzione nell’uso privato e non o la destinazione di frequenze ad uso contemporaneo a differenti servizi aventi differenti finalità. L’uso delle bande libere è destinato ad apparati denominati SRD (Short Range Devices). Curiosamente la banda 862-876 MHz (sub-GHz) molto utilizzata da dispositivi SRD, è gestita dal Ministero della Difesa e non dal MISE.  


I criteri riguardano anche alcuni dettagli fisici e tecnici:
- Il **Duty Cycle** fa riferimento al rapporto fra il tempo di trasmissione e il tempo di ricezione più il tempo di ricezione. Ad esempio un duty cycle dell’ 1% impone che a fronte della trasmissione di un pacchetto dati di un secondo, l’apparato non possa trasmettere per 99 secondi. Questo parametro, di fatto, limita a trasmissioni brevi, non frequenti ed esclude gli streaming audio e video.
- **Potenza**. La potenza in Watt. In genere dell’ordine dei millesimi di watt (mW) è spesso espressa in dBm.
In taluni casi, in relazione alla larghezza di banda, si fa riferimento alla densità di potenza, ovvero alla potenza, in milliwatt per MHz o per KHz. E’ il caso degli apparati WLAN E HiperLAN. La potenza è generalmente riferita al segnale irradiato ERP. EIRP. L’antenna è un componete passivo, il guadagno quantifica la capacità dell’antenna di concentrare l’energia irradiata (o ricevuta) in una determinata direzione. Il termine **ERP** indica la potenza applicata all’antenna aumentata del guadagno di antenna. L'**EIRP** è la potenza emessa da un'antenna isotropica, che, non avendo direzioni di emissione privilegiate, emette ugual potenza in tutte le direzioni. Il guadagno di un'antenna in una certa direzione si calcola eseguendo il rapporto tra la potenza misurata in quella direzione e la potenza EIRP.
- Altri limiti sono le modalità di accesso al canale radio. Due termini frequenti sono di **LBT** e **AFA**. **LBT** è l’acronimo di **Listen Before Talk**, ovvero l’apparato, prima di trasmettere, deve ricevere ed accertarsi che non ci siano trasmissioni in atto da parte di altri apparati ed attendere che il canale sia libero. **AFA**, Adaptive Frequency Agility, indica una modalità di gestione dinamica della frequenza di trasmissione (canale, nel lambito della stessa banda), che **varia ad ogni accesso**.

I **parametri** e le **limitazioni** discusse sono tecniche volte a ridurre **mutui disturbi** fra i vari servizi dei vari utenti e la **monopolizzazione** di uncanale da parte di un singolo utente.

<img src="433.png" alt="alt text" width="800">
<img src="868.png" alt="alt text" width="800">
<img src="2400.png" alt="alt text" width="800">

>[Torna a reti di sensori](sensornetworkshort.md#interfaccia-radio)
