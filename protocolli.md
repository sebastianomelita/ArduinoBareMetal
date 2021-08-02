>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)

## **Protocolli**

I protocolli sono **regole di comunicazione** che tutti gli **interlocutori** devono seguire per **portare avanti** la **comunicazione** su di un **canale**.
Un protocollo **definisce** il **formato** e l’**ordine dei messaggi** scambiati tra due o più entità di rete, le **azioni in risposta** ai messaggi ricevuti o **altri eventi**.
Sono valide per un **determinato canale** e si realizzano in **maniera software** (o al limite con logica digitale cablata) tramite programmi che girano sui nodi della rete.

Realizzano **algoritmi distribuiti** che comprendono:
- funzioni sui nodi della rete
- scambio di messaggi lungo i canali della rete

Definisce **cosa** va comunicato, **come** e **quando** va comunicato. Gli **elementi chiave** di un protocollo sono: s**intassi**, **semantica**, **temporizzazione**.
<img src="MACframeEthernet.png" alt="alt text" width="600">

Il **formato** è definito indicando:
- il **significato** dei vari campi. Ad esempio **DA** rappresenta il Destination Address mentre **SA** rappresenta il Sorce Address. Il campo **Information** rappresenta il payload, cioè il carico utile, ovvero le informazioni che il protocollo deve spostare dalla sorgente alla destinazione. Tutto ciò che **sta prima** del campo paysload è l'**Header**, ovvero l'**intestazione** del messaggio detta anche **PCI (Protocol Control Information)**.
- la **lunghezza** dei vari campi, può essere:
    - **fissa**. E' misurata in byte ed è indicata con dei numeretti posti sopra il nome del campo che misurano.
    - **variabile**. Normalmente viene indicata la lunghezza minima e quella massima. Talvolta un campo aggiuntivo (Length in figura) è deputato proprio a contenere la lunghezza del payload 

Per **accedere** (in lettura o in scrittura) ad un **determinato campo** basta sommare la lunghezza dei campi che precedono il campo da accedere per determinare il suo **spiazzamento** dall'inizio del messaggio.




>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)
