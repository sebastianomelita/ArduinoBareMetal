# A038 — ESAME DI MATURITÀ

**Sessione ordinaria 2026 — Seconda prova scritta**
**Ministero dell'Istruzione e del Merito**

Indirizzo **ITIA — INFORMATICA E TELECOMUNICAZIONI**, ARTICOLAZIONE "INFORMATICA"
*(Testo valevole anche per gli indirizzi quadriennali IT32)*

**Disciplina: SISTEMI E RETI**

> *Il candidato svolga la prima parte della prova e due tra i quesiti proposti nella seconda parte.*

---

## PRIMA PARTE

Una grande società di ingegneria ha deciso di avvalersi di nuove tecnologie ed infrastrutture digitali per la realizzazione di alcune importanti opere edilizie, acquisendo presso la propria sede centrale un sistema BIM, acronimo di *Building Information Modeling* (in italiano: Modellazione informatica delle costruzioni). Si tratta di un "metodo per l'ottimizzazione della pianificazione, realizzazione e gestione di costruzioni tramite un processo codificato, attraverso il quale tutti e i dati rilevanti di una costruzione possono essere raccolti, combinati e collegati digitalmente. La costruzione virtuale è visualizzabile inoltre come un modello geometrico tridimensionale" (fonte: Wikipedia.org).

L'utilizzo di un sistema BIM consentirà alla società il monitoraggio dell'avanzamento dei lavori nei cantieri edili, la riduzione di errori, la velocizzazione dei tempi e, su edifici esistenti, faciliterà la rilevazione dello stato di fatto dell'edificio e la progettazione degli interventi di recupero.

Per il conseguimento di tali obiettivi, la società dovrà avvalersi di apparecchiature digitali da utilizzare in cantiere:

a) diversi tablet *rugged* [^1] corredati di laser scanner 3D o sensore Lidar;

b) alcune fotocamere *timelapse*;

c) numerosi sensori di sicurezza (ad es. vibrazioni, gas e fumi, umidità, temperatura o altre proprietà chimico-fisiche).

I tablet rugged saranno utilizzati per i cosiddetti "rilevamenti BIM", operazioni di scansione effettuate con strumenti basati su tecnologia Laser Scanner 3D (alta precisione) o Lidar (bassa/media precisione). Questi strumenti di scansione possono essere interfacciati ai tablet attraverso varie tipologie di porte wired/wireless o possono essere anche integrati negli stessi tablet (come nel caso del sensore Lidar).

In un rilevamento BIM, quando il raggio laser dello scanner colpisce la superficie di un oggetto anche molto irregolare, viene registrata la posizione (X, Y, Z) di un insieme denso di punti-campione di tale superficie. L'insieme di tutti questi punti campionati, che darà origine alla visualizzazione 3D dell'oggetto, è chiamato "nuvola di punti". Al termine dei rilevamenti, i dati di ogni nuvola di punti acquisiti con le scansioni sono poi trasformati in un modello geometrico tridimensionale parametrico (oggetto BIM) che costituisce una rappresentazione digitale precisa dell'edificio o di una sua parte (ad esempio una facciata, una scala, un impianto, etc.). Ogni oggetto BIM viene solitamente integrato con altri dati informativi quali, ad esempio, dati relativi a misure chimico-fisiche, dati tecnici e immagini digitali provenienti dalla fotocamera degli stessi tablet.

Per quanto riguarda le fotocamere timelapse, queste sono invece dispositivi fotografici automatici progettati per scattare foto a intervalli di tempo regolari e prestabiliti. Garantiscono normalmente alta risoluzione (es. 4K/8K), e offrono, in generale, connettività sia wired (es. via porta USB) che wireless. Le immagini delle foto scattate da una fotocamera timelapse sono utilizzate per assemblare video che "accelerano" il tempo, allo scopo di mostrare in pochi secondi processi lunghi e lenti. La società utilizzerà queste fotocamere per monitorare e documentare l'avanzamento dei lavori e/o creare video promozionali; esse verranno pertanto installate in punti strategici di ciascun cantiere in corso.

Per quanto riguarda infine i sensori per la sicurezza del cantiere, la società ha acquisito sensori interfacciabili a varie tipologie di moduli di trasmissione dati sia wired che wireless.

Con l'adozione del sistema BIM, per l'apertura di ogni nuovo cantiere la società dovrà quindi dotarsi di una infrastruttura di comunicazione che possa di volta in volta essere messa temporaneamente in campo al fine di consentire:

- Il trasferimento, dal cantiere alla sede centrale, dei dati grezzi delle nuvole di punti derivanti dai rilevamenti BIM, che verranno utilizzate in software specialistici per le successive operazioni di modellazione 3D. Il modello finale verrà gestito in condivisione dati tra i vari uffici della sede per il coordinamento tra i tecnici che lavorano al progetto.
- La trasmissione e la raccolta dei fotogrammi provenienti dalle fotocamere in un opportuno sistema di repository presso la sede centrale, per costruire i video in timelapse.
- La trasmissione dei dati provenienti dai sensori di sicurezza, sia internamente al cantiere verso i sistemi locali per l'eventuale attivazione di notifiche ed allarmi in tempo reale, sia verso i sistemi remoti in sede centrale per le segnalazioni, le ulteriori elaborazioni e per mantenere un registro di log storico.

Tutte le operazioni di comunicazione locale (nei cantieri e nella sede centrale) e remota (tra singolo cantiere e sede) dovranno avvenire mediante idonee misure di sicurezza ed autenticazione sia a livello di singoli apparati che degli operatori.

La società dispone già di una infrastruttura di rete locale presso la sede a cui afferiscono i computer degli uffici tecnici per l'accesso condiviso ai sistemi che conterranno i software BIM specialistici; tale rete locale è provvista di accesso ad internet mediante router con un collegamento WAN datato basato su tecnologia ADSL.

Il candidato, analizzata la realtà di riferimento e formulate le opportune ipotesi aggiuntive, incluse ipotesi di dimensionamento del numero degli apparati correlato alla complessità di ciascun cantiere e del numero massimo dei cantieri contemporaneamente attivi, sviluppi i seguenti punti:

1. un progetto generale dell'infrastruttura di rete da realizzare in un cantiere, anche supportato da uno schema grafico, indicando tipologia e caratteristiche dei canali locali, apparati, protocolli, schemi di indirizzamento e servizi da adottare;

2. una descrizione della ipotetica struttura della rete pre-esistente in sede centrale, con proposte di integrazione e potenziamento necessarie per l'adozione del sistema BIM;

3. la scelta della tipologia e delle caratteristiche dei canali di comunicazione da realizzare tra cantieri e sede centrale, inclusa una stima della capacità trasmissiva sufficiente a supportare il traffico dati, individuando gli apparati da adottare;

4. le modalità, i protocolli e i servizi con cui consentire agli operatori di autenticarsi presso i sistemi in sede centrale sia nella sede stessa che dai cantieri da remoto.

---

## SECONDA PARTE

**I.** In relazione al tema proposto nella prima parte, per quanto riguarda le finalità di archiviazione dei dati (scansioni, immagini, dati da sensori, etc.) si individuino e descrivano, illustrandone anche vantaggi e svantaggi, le differenze tra possibili soluzioni tecnologiche "on premise" e soluzioni cloud-based, da adottare ed integrare nel progetto.

**II.** In relazione al tema proposto nella prima parte, per quanto riguarda le operazioni di comunicazione locale e remota, si individuino e si descrivano, oltre alle modalità di autenticazione già discusse al punto 4 della prima parte, le ulteriori misure di sicurezza informatica da adottare nella sede centrale e nei cantieri, incluse quelle idonee a garantire la continuità trasmissiva del canale tra i singoli cantieri e la sede centrale.

**III.** Il candidato ipotizzi di essere l'amministratore della rete didattica di un istituto ad indirizzo informatico. Dall'analisi dei log dei sistemi e da osservazioni dirette effettuate dai docenti, è stato riscontrato che diversi studenti, in modo non autorizzato, fanno sviluppare da piattaforme di Intelligenza Artificiale il codice dei programmi delle tracce proposte dai docenti per le attività in laboratorio. Il candidato illustri e dettagli con esemplificazioni le possibili misure e tecniche da attuare per impedire l'accesso a tali piattaforme. Proponga inoltre eventuali modalità per *schedulare* tale blocco (ed il successivo sblocco) in specifici momenti dell'orario scolastico e/o da specifici laboratori.

**IV.** In relazione al protocollo SSH, il candidato immagini di impartire il comando

```
ssh -p 25500 administrator@200.1.1.1
```

Sapendo che sul dispositivo con indirizzo 200.1.1.1 è configurata una regola che reindirizza il traffico, in ingresso sulla porta 25500, ad un altro dispositivo con indirizzo 172.16.1.100, il candidato esponga gli effetti del comando impartito e le finalità del suo utilizzo.

---

### Strumenti consentiti

- Durata massima della prova: **6 ore**.
- È consentito l'uso di manuali tecnici e di calcolatrici scientifiche o grafiche purché non siano dotate della capacità di elaborazione simbolica algebrica e non abbiano disponibilità di connessione a Internet.
- È consentito l'uso del dizionario bilingue (italiano-lingua del Paese di provenienza) per i candidati di madrelingua non italiana.
- Non è consentito lasciare l'Istituto prima che siano trascorse **3 ore** dalla consegna della traccia.

[^1]: I tablet "rugged" sono progettati per funzionare in modo affidabile in condizioni di lavoro difficili, con resistenza a urti e cadute, impermeabilità, resistenza alla polvere, batterie a lunga durata e/o sostituibili, porte per vari supporti e connettività avanzate, display più facilmente leggibili all'aperto.
