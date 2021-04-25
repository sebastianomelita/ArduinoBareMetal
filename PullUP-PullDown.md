

>[Torna all'indice](indexpulsanti.md)
## **GESTIONE ELETTRICA DEI PULSANTI**

Un pulsante è un **contatto** che **chiude** un circuito alla **pressione** esercitata dall’esterno su di un **pistoncino** e che ritorna automaticamente ad una **posizione di riposo** di circuito **aperto** per effetto di una molla interna di richiamo.

Il pulsante è **senza stato** nel senso che non “ricorda” né elettricamente nè meccanicamente lo stato della sua impostazione **precedente**.

![pulsante](pulsante.png)


Il **contatto** che si chiude alla pressione del pulsante è in realtà **uno solo** e unisce **due terminali** indicati in figura con a-b e c-d.

I **terminali** **esternamente** sono 4 ma in realtà sono **elettricamente solo due** poiché i terminali a-b e c-d sono **cortocircuitati** **all’interno** del pulsante, in pratica sono lo stesso terminale.

**Visivamente** le “gambine” che si guardano l’una con l’altra **di faccia** sono quelle **cortocircuitate**, quelle che sono **affiancate di taglio** sono quelle **scollegate** a circuito aperto.

Per **cablare** un pulsante ad un ingresso digitale di un microcontrollore sono possibili due schemi (topologie) di collegamento: **pullup** e **pulldown**.

![pullupdown](pullupdown.png)

**Pull** vuol dire tirare per cui lo scopo dei due metodi di collegamento è letteralmente “tirare in alto” e “tirare in basso”.

- **“tirare chi?** Evidentemente **l’ingresso** **digitale** del microcontrollore, specificatamente il parametro elettrico **tensione**, quello che si misura in **volt.** I valori possibili possono essere tutti quelli compresi tra la tensione **GND** (normalmente 0V) e quella di alimentazione della scheda (**5V** per Arduino, **3,3V** per dispositivi più piccoli). Valori diversi da quelli compresi entro questo range potrebbero essere tollerati fino ad un certo limite superato il quale si danneggia irreversibilmente la porta o, nel caso peggiore, tutta la scheda.

-  **“tirare dove?** O in **UP** alla **Vcc (**ad esempio 5V), cioè l’ingresso del microcontrollore viene portato alla alimentazione positiva della scheda, oppure in **DOWN** ovvero **GND** cioè l’ingresso del microcontrollore viene portato al potenziale zero, cioè è **virtualmente** cortocircuitato verso massa.

-  **“tirare quando?** La topologia rappresentata dal collegamento (pull up o pull down) fotografa la situazione in cui il pulsante **non è premuto**, cioè la situazione del pulsante che realizza un **circuito aperto** tra i suoi terminali.

- **“tirare come?”** Il piedino di ingresso del microcontrollore non è **mai** collegato direttamente in su alla Vcc o in giù al GND ma indirettamente **tramite una resistenza** di valore opportuno (1-10K Ohm).

**Ruolo del pulsante**, serve:

-  a **pulsante aperto**, lasciare l’ingresso polarizzato al suo valore di default, Vcc nel caso del **pullup** o GND nel caso del **pulldown**. Internamente questi valori sono codificati rispettivamente, con lo stato logico alto **HIGH** (normalmente vale1) e **LOW** (di solito 0)

-  a **pulsante chiuso**, cortocircuitare (collegare direttamente) l’ingresso del microcontrollore a potenziale GND nel caso della polarizzazione **pullup**, o a potenziale **Vcc** nel caso di quella **pulldown**. L’effetto è evidentemente quello di invertire lo stato logico di default che, nella situazione di **pulsante premuto**, si codifica con **LOW** nel caso del collegamento **pullup** e con **HIGH** nel caso di quello **pulldown**

![elettrico](pullupelectric.png)

**Ruolo della resistenza** è quello di **polarizzare** (forzare tensione e corrente) adeguatamente **l’ingresso** nelle due **situazioni** possibili:

- **A pulsante aperto**, poiché il micro in prima approssimazione non consuma corrente (in realtà e trascurabile) la caduta di tensione sulla resistenza è nulla. In altre parole, la resistenza è assimilabile ad un pezzo di **filo** che collega l’ingresso del micro **in alto a Vcc (pullup)** o **in basso a GND (pulldown).**

- **A pulsante chiuso, limitare** la **corrente** che scorre nel pulsante quando questo viene chiuso, infatti, per effetto del cortocircuito ai suoi capi, su di esso scorre una corrente generata dalla tensione che si viene a determinare **ai capi** della resistenza (**caduta di tensione**) pari in genere a **Vcc**. Se la resistenza fosse stata semplicemente **un filo** (resistenza nulla) la corrente sarebbe quella di **cortocircuito** (teoricamente infinita). Se la resistenza fosse stata semplicemente un **circuito aperto** l’ingresso non sarebbe stato **preventivamente tirato** al suo valore di default.

**Valori della resistenza** troppo bassi o troppo alti determinano vantaggi e svantaggi:

- **Molto bassi** hanno il **vantaggio** di garantire una buona **immunità** **ai disturbi** (EMI e RFI) ma lo **svantaggio** di determinare potenzialmente un **consumo di corrente eccessivo** (eccessivo consumo delle batterie)

- **Molto alti** hanno il **vantaggio** di garantire **un basso consumo** di corrente a riposo (lunga durata delle batterie ma lo **svantaggio** di determinare potenzialmente una **predisposizione ad assorbire rumore** di tipo EMI o RFI.

Un valore di **compromesso** spazia da qualche KOhm fino a 50 KOhm (tipicamente **10 KOhm**).

**Riassumendo,** lo **stato logico** dei pulsanti nelle due configurazioni è:

-  **PULL UP** –> Interruttore aperto –> HIGH  
**PULL UP** –> Interruttore chiuso –> LOW

- **PULL DOWN** –> Interruttore aperto –> LOW  
**PULL DOWN** –> Interruttore chiuso –> HIGH

Di seguito è riportato lo schema di un **possibile collegamento** di due pulsanti in pulldown (a sinistra) e in pullup (a destra) su una breadboard:

![pulsantiboard](pulsantiboard.png)

>[Torna all'indice](indexpulsanti.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbLTEwNzE0MzIzOTldfQ==
-->
