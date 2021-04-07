---


---

<h2 id="gestione-dei-pulsanti-nei-microcontrollori"><strong>GESTIONE DEI PULSANTI NEI MICROCONTROLLORI</strong></h2>
<p>Un pulsante è un <strong>contatto</strong> che <strong>chiude</strong> un circuito alla <strong>pressione</strong> esercitata dall’esterno su di un <strong>pistoncino</strong> e che ritorna automaticamente ad una <strong>posizione di riposo</strong> di circuito <strong>aperto</strong> per effetto di una molla interna di richiamo.</p>
<p>Il pulsante è <strong>senza stato</strong> nel senso che non “ricorda” né elettricamente nè meccanicamente lo stato della sua impostazione <strong>precedente</strong>.</p>
<p><img src="https://db3pap001files.storage.live.com/y4pRpxO3QHTmemnyAlXrnl3jaxGAJwEcmL_JZBcU539FfG-YgyIvqgfMuC7m7Wu46kElE8Tj6ly_nXns0Awo_-dRPBJUF24LwReshg9azJF3aGlnDygCURi0EYq847sV81eanV8l5SEMZa7f7Uh2xmmhhxFrsJsn9MwramAiOELz2yz2TAO9uZaIjRQmhLEyEBSg5tQdPyxlXIcYrWqr35WMzhBCYl_eVM-5A5nk20IhW4?psid=2&amp;width=318&amp;height=158" alt="pulsante"></p>
<p>Il <strong>contatto</strong> che si chiude alla pressione del pulsante è in realtà <strong>uno solo</strong> e unisce <strong>due terminali</strong> indicati in figura con a-b e c-d.</p>
<p>I <strong>terminali</strong> <strong>esternamente</strong> sono 4 ma in realtà sono <strong>elettricamente solo due</strong> poiché i terminali a-b e c-d sono <strong>cortocircuitati</strong> <strong>all’interno</strong> del pulsante, in pratica sono lo stesso terminale.</p>
<p><strong>Visivamente</strong> le “gambine” che si guardano l’una con l’altra <strong>di faccia</strong> sono quelle <strong>cortocircuitate</strong>, quelle che sono <strong>affiancate di taglio</strong> sono quelle <strong>scollegate</strong> a circuito aperto.</p>
<p>Per <strong>cablare</strong> un pulsante ad un ingresso digitale di un microcontrollore sono possibili due schemi (topologie) di collegamento: <strong>pullup</strong> e <strong>pulldown</strong>.</p>
<p><img src="https://db3pap001files.storage.live.com/y4pDbJCNANzowooPag2ZkdesuhI3SRAkv53BJlkUgTaSEFDYZyCAOZx_ElFAjg67XfP9jEEqFlVMkcz64wSYJ2opET-lsS_3AIWl-YSInGsnz4TpnP4DL4BvIIdtwfuxrw8JFp6RPkvvvi87kit03yPUSzJZ-EU4SyOn5aPdORGe33Eq8OurJpUO--AlISojrYC/pullupdown.png?psid=1&amp;width=990&amp;height=516" alt="pullupdown"></p>
<p><strong>Pull</strong> vuol dire tirare per cui lo scopo dei due metodi di collegamento è letteralmente “tirare in alto” e “tirare in basso”.</p>
<ul>
<li>
<p><strong>“tirare chi?</strong> Evidentemente <strong>l’ingresso</strong> <strong>digitale</strong> del microcontrollore, specificatamente il parametro elettrico <strong>tensione</strong>, quello che si misura in <strong>volt.</strong> I valori possibili possono essere tutti quelli compresi tra la tensione <strong>GND</strong> (normalmente 0V) e quella di alimentazione della scheda (<strong>5V</strong> per Arduino, <strong>3,3V</strong> per dispositivi più piccoli). Valori diversi da quelli compresi entro questo range potrebbero essere tollerati fino ad un certo limite superato il quale si danneggia irreversibilmente la porta o, nel caso peggiore, tutta la scheda.</p>
</li>
<li>
<p><strong>“tirare dove?</strong> O in <strong>UP</strong> alla **Vcc (**ad esempio 5V), cioè l’ingresso del microcontrollore viene portato alla alimentazione positiva della scheda, oppure in <strong>DOWN</strong> ovvero <strong>GND</strong> cioè l’ingresso del microcontrollore viene portato al potenziale zero, cioè è <strong>virtualmente</strong> cortocircuitato verso massa.</p>
</li>
<li>
<p><strong>“tirare quando?</strong> La topologia rappresentata dal collegamento (pull up o pull down) fotografa la situazione in cui il pulsante <strong>non è premuto</strong>, cioè la situazione del pulsante che realizza un <strong>circuito aperto</strong> tra i suoi terminali.</p>
</li>
<li>
<p><strong>“tirare come?”</strong> Il piedino di ingresso del microcontrollore non è <strong>mai</strong> collegato direttamente in su alla Vcc o in giù al GND ma indirettamente <strong>tramite una resistenza</strong> di valore opportuno (1-10K Ohm).</p>
</li>
</ul>
<p><strong>Ruolo del pulsante</strong>, serve:</p>
<ul>
<li>
<p>a <strong>pulsante aperto</strong>, lasciare l’ingresso polarizzato al suo valore di default, Vcc nel caso del <strong>pullup</strong> o GND nel caso del <strong>pulldown</strong>. Internamente questi valori sono codificati rispettivamente, con lo stato logico alto <strong>HIGH</strong> (normalmente vale1) e <strong>LOW</strong> (di solito 0)</p>
</li>
<li>
<p>a <strong>pulsante chiuso</strong>, cortocircuitare (collegare direttamente) l’ingresso del microcontrollore a potenziale GND nel caso della polarizzazione <strong>pullup</strong>, o a potenziale <strong>Vcc</strong> nel caso di quella <strong>pulldown</strong>. L’effetto è evidentemente quello di invertire lo stato logico di default che, nella situazione di <strong>pulsante premuto</strong>, si codifica con <strong>LOW</strong> nel caso del collegamento <strong>pullup</strong> e con <strong>HIGH</strong> nel caso di quello <strong>pulldown</strong></p>
</li>
</ul>
<p><img src="https://db3pap001files.storage.live.com/y4p6W_mY1BH6SzZzQ8dJzDvO9RN5fJq20R8RVuMvAXimuF3z7gyUYrkRpLZaj7CRY9pHbj1cZNG-2-oLFH77E3RD9E_fvM4dRx_VkZpD8c3WpVGUnaYeMq0zIscUiTQwBuCHGQtt2KV0MBx7Ut2QGFCDarsmTG6uutyhp1xhJVzdDFqWBZ1qQidSEqvfoK0I5TE/pullupelectric.png?psid=1&amp;width=568&amp;height=164" alt="elettrico"></p>
<p><strong>Ruolo della resistenza</strong> è quello di <strong>polarizzare</strong> (forzare tensione e corrente) adeguatamente <strong>l’ingresso</strong> nelle due <strong>situazioni</strong> possibili:</p>
<ul>
<li>
<p><strong>A pulsante aperto</strong>, poiché il micro in prima approssimazione non consuma corrente (in realtà e trascurabile) la caduta di tensione sulla resistenza è nulla. In altre parole, la resistenza è assimilabile ad un pezzo di <strong>filo</strong> che collega l’ingresso del micro <strong>in alto a Vcc (pullup)</strong> o <strong>in basso a GND (pulldown).</strong></p>
</li>
<li>
<p><strong>A pulsante chiuso, limitare</strong> la <strong>corrente</strong> che scorre nel pulsante quando questo viene chiuso, infatti, per effetto del cortocircuito ai suoi capi, su di esso scorre una corrente generata dalla tensione che si viene a determinare <strong>ai capi</strong> della resistenza (<strong>caduta di tensione</strong>) pari in genere a <strong>Vcc</strong>. Se la resistenza fosse stata semplicemente <strong>un filo</strong> (resistenza nulla) la corrente sarebbe quella di <strong>cortocircuito</strong> (teoricamente infinita). Se la resistenza fosse stata semplicemente un <strong>circuito aperto</strong> l’ingresso non sarebbe stato <strong>preventivamente tirato</strong> al suo valore di default.</p>
</li>
</ul>
<p><strong>Valori della resistenza</strong> troppo bassi o troppo alti determinano vantaggi e svantaggi:</p>
<ul>
<li>
<p><strong>Molto bassi</strong> hanno il <strong>vantaggio</strong> di garantire una buona <strong>immunità</strong> <strong>ai disturbi</strong> (EMI e RFI) ma lo <strong>svantaggio</strong> di determinare potenzialmente un <strong>consumo di corrente eccessivo</strong> (eccessivo consumo delle batterie)</p>
</li>
<li>
<p><strong>Molto alti</strong> hanno il <strong>vantaggio</strong> di garantire <strong>un basso consumo</strong> di corrente a riposo (lunga durata delle batterie ma lo <strong>svantaggio</strong> di determinare potenzialmente una <strong>predisposizione ad assorbire rumore</strong> di tipo EMI o RFI.</p>
</li>
</ul>
<p>Un valore di <strong>compromesso</strong> spazia da qualche KOhm fino a 50 KOhm (tipicamente <strong>10 KOhm</strong>).</p>
<p><strong>Riassumendo,</strong> lo <strong>stato logico</strong> dei pulsanti nelle due configurazioni è:</p>
<ul>
<li>
<p><strong>PULL UP</strong> –&gt; Interruttore aperto –&gt; HIGH<br>
<strong>PULL UP</strong> –&gt; Interruttore chiuso –&gt; LOW</p>
</li>
<li>
<p><strong>PULL DOWN</strong> –&gt; Interruttore aperto –&gt; LOW<br>
<strong>PULL DOWN</strong> –&gt; Interruttore chiuso –&gt; HIGH</p>
</li>
</ul>
<p>Di seguito è riportato lo schema di un <strong>possibile collegamento</strong> di due pulsanti in pulldown (a sinistra) e in pullup (a destra) su una breadboard:</p>
<p><img src="https://db3pap001files.storage.live.com/y4pdPqmcKsnuxCfaII5kOiI3PZzApZw6Cn1s5Tmb4yMn8UBQcCmjDrToNGqi_5qQALseZXXgElW0BpeXeurGR7JOMtH0n-9xZcejomPt6lyqbSQtNPbSRxk_AFTCXTb7WUJimUqM-TghT0Wl-ESR1pGhxbhFmpMg7K8xht3nI8tez1O6Xse-mnmQEbdipVMOCSb/pulsantiboard.png?psid=1&amp;width=817&amp;height=403" alt="pulsantiboard"></p>

