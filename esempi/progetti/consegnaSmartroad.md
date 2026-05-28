>[Torna a svolgimento Smart Road](svolgimento_smart_road.md)>[Torna a Dettaglio architettura LoRaWAN](/lorawanclasses.md) 
>

# Esame di Stato di Istituto Tecnico 2023/24 — Traccia

**Indirizzo:** Informatica e Telecomunicazioni
**Articolazione:** Informatica
**Disciplina:** Sistemi e Reti

> Il candidato svolga la prima parte della prova e due tra i quesiti proposti nella seconda parte.

---

## Prima parte

La società di gestione delle autostrade, in collaborazione con l'autorità governativa preposta, intende sperimentare a livello nazionale un progetto smart-road finalizzato a rendere più sicuro e sostenibile il traffico autostradale di persone e merci.

A questo scopo sono individuati alcuni tratti autostradali sperimentali distribuiti in tutte le regioni, nei quali, a intervalli di un chilometro l'uno dall'altro, sono installati **smart-gate** costituiti da:

- **maxi-schermi** per la visualizzazione dinamica della segnaletica (limiti di velocità, chiusura di corsie, ecc.) e di informazioni (percorsi preferenziali, deviazioni obbligatorie, condizioni meteorologiche inclusa la visibilità e relative alle condizioni del fondo stradale, ecc.);
- **telecamere** per il monitoraggio e il controllo del traffico, anche attraverso il riconoscimento delle tipologie e delle targhe dei veicoli;
- **sensori** per la rilevazione delle condizioni meteorologiche, della visibilità, delle condizioni del fondo stradale e dei livelli di inquinamento acustico e dell'aria.

Gli smart-gate elaborano localmente alcune informazioni e sono abilitati a impostare autonomamente la segnaletica e le informazioni visualizzate sui maxi-schermi, e sono connessi a un **centro di controllo** del tratto autostradale sperimentale, dove un operatore umano ha la possibilità di monitorare i dati acquisiti dalle telecamere e dai sensori e di integrare o modificare la segnaletica e/o le informazioni visualizzate.

Tutti i centri di controllo sono interconnessi in una **rete nazionale** che consente di distribuire dati di traffico o relativi a eventuali interruzioni, utilizzabili per la visualizzazione da parte degli smart-gate di informazioni relative ai percorsi preferenziali per le varie direzioni, allo scopo di analizzare a posteriori con tecniche di data-analysis il progetto smart-road.

Tutti i dati acquisiti e trasmessi, le segnaletiche e le informazioni visualizzate sono memorizzate in un **database nazionale**, che deve anche consentire a un'**APP** — specificatamente sviluppata e liberamente utilizzabile dai guidatori — di verificare in tempo reale la segnaletica e le informazioni visualizzate da ogni smart-gate della rete autostradale.

Uno degli scopi del progetto smart-road consiste nel facilitare l'impiego di veicoli elettrici per lunghi tragitti sulla rete autostradale: a questo scopo le **stazioni di ricarica** presenti sono interconnesse alla rete nazionale del progetto per rendere disponibili in tempo reale lo stato dei punti di ricarica disponibili e per consentirne la prenotazione sulla base dell'orario di arrivo e della durata stimata per l'operazione.

Il candidato analizzi la realtà di riferimento e, fatte le opportune ipotesi aggiuntive, sviluppi i seguenti punti:

1. Il progetto, anche mediante rappresentazioni grafiche, dell'infrastruttura tecnologica e informatica necessaria a realizzare il progetto smart-road, dettagliando:
   - a. l'architettura della rete e le caratteristiche dei sistemi di elaborazione e di comunicazione impiegati nei vari nodi (smart-gate, centro di controllo, livello nazionale), motivandone la scelta della tipologia e della collocazione;
   - b. le tecnologie e le modalità di comunicazione tra i nodi della rete e tra i dispositivi presenti all'interno dei singoli nodi.
2. La configurazione dei dispositivi di rete presenti nei vari nodi della stessa, con riferimento a un opportuno piano di indirizzamento.
3. Le tecnologie e le soluzioni idonee a garantire sia la continuità di servizio che la sicurezza dell'infrastruttura tecnologica e informatica progettata.

---

## Seconda parte

*(Il candidato svolga due tra i seguenti quesiti.)*

1. Con riferimento al progetto sperimentale smart-road presentato nella prima parte, risulta necessario un database centralizzato che consenta di gestire lo stato e le prenotazioni relative ai singoli punti di ricarica delle stazioni per le auto elettriche presenti sulla rete autostradale. Progettare a livello logico il database relazionale.

2. Con riferimento al progetto sperimentale smart-road presentato nella prima parte, individuare una possibile tecnologia di comunicazione a livello applicativo per l'interazione con il database nazionale da parte dell'APP utilizzabile dai guidatori e documentare un possibile protocollo applicativo.

3. Il protocollo client/server HTTP, nella sua versione sicura HTTPS, è sempre più utilizzato — oltre che per la fruizione di siti e applicazioni web che interagiscono direttamente con un utente umano — per l'implementazione di servizi web destinati all'interazione tra componenti software. Descrivere le caratteristiche fondamentali di questo protocollo e la sua evoluzione nel corso del tempo.

4. Oltre ai tradizionali algoritmi di crittografia simmetrici e asimmetrici, molti protocolli di rete sicuri impiegano funzioni hash crittografiche. Descrivere scopo, caratteristiche e applicazioni di questa categoria di algoritmi.

---

## Quesiti svolti in questo repository

Lo svolgimento ([`svolgimento_smart_road.md`](./svolgimento_smart_road.md)) affronta la **prima parte** completa e, della seconda parte, i quesiti **1** (database prenotazioni ricarica) e **2** (protocollo applicativo per l'APP), scelti perché si innestano direttamente sull'infrastruttura progettata nella prima parte.
