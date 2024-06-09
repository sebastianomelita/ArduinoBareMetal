>>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

## **ERRORI COMUNI SUL CABLAGGIO STRUTTURATO** 

non ci sono etichette degli spazi con il nome (o un identificativo) della funzione. Come faccio a
sapere quale scopo ha quella stanza? E&#39; un laboratorio? un aula? La sala insegnanti?
- I cablaggi sono sempre unici. Non esiste che nel progetto di un compito, per lo stesso edificio, tu
debba fare 2 cablaggi separati. Si può fare ma sono due progetti diversi e quà noi ne facciamo uno
solo.
- mancano le dimensioni lineari sulla planimetria
- Manca numerazione dei TO
- Manca la traccia delle canalizzazioni nella planimetria
- I TO NON sono placche, una presa/placchetta è fatta da più TO (tipicamente 2 o 3)
- la separazione dei gruppi di utenti ho sempre detto che non la facciamo mai fisica ad L1 col
cablaggio ma logica ad L2 con le VLAN sugli SW o a L3 con i router o a L7 con le autorizzazioni. Noi
per quest&#39;anno, usiamo solo i metodi L2 e L3
- manca numerazione degli armadi, dei TO e delle dorsali
- tutte le etichette dei TO iniziano con 1, ma che informazione porta mai 1 se è uguale per tutti i TO?
Forse hai numerato pure il CD? Se è così in realtà quel numero è inutile, appesantisce solo il
disegno, senza portare nessuna informazione utile
- il TO in mezzo a una riga di puntini di sospensione è inutile
- manca legenda TO col significato della numerazione dei TO
- l&#39;etichetta delle dorsali orizzontali non mi sembra corretta. Per esempio, FD2 del 3 piano, servito
da BD3, avrebbe dovuto avere un cavo etichettato come FD2-BD3. O avevi un&#39;altra idea?
- CD lo avrei collassato sul BD/FD del suo piano perché vi è un solo edificio/piano
- l&#39;albero degli apparati passivi è incompleto, dal CD dovrebbero uscire altre dorsali di piano verso i
piani non rappresentati nel disegno attuale
- due SW in cascata senza commutare su più uscite non hanno senso
- mancano etichettature degli apparati attivi
- le dorsali degli apparati passivi devono essere disegnate sempre DISTINTE
- le dorsali degli apparati attivi devono essere disegnate sempre DISTINTE
- nell’albero degli apparati passivi i cablaggi orizzontali devono essere disegnati sempre DISTINTI
- nell’albero degli apparati passivi i collegamenti verso gli host devono essere disegnati sempre
DISTINTI
- Mancano le ipotesi iniziali sul tipo di edificio e sul tipo di cablaggio.
- manca qualunque ipotesi sulle esigenze di banda sia della LAN che del collegamento WAN
- errore molteplicità: il cavo ISP/CD lo fornisce il gestore, di solito ha molteplicità 2 se in fibra e più
che al firewall va al modem (che a casa in effetti è un modem router firewall)
- mancano riferimenti sul tipo di SW, su quante porte abbiano, se siano o meno 802.1Q se abbiano
le porte SFP per i transceiver ottici o meno
- non è spiegato il perchè delle tue scelte, perche le VLAN piuttosto che un router con una subnet
per porta. E’ congruente la decisione con la disposizione dei gruppi nello spazio?
- la molteplicità del cavo UTP è standard ed è 4 non 6 o 8
- Incongruenza sui patch panel: c&#39;è un solo patch panel per armadio anche quando su di esso solo
terminati più di 24 cavi, come si fa a terminare 60 fili con 24 prese soltanto?
- Incongruenza con la consegna: manca la LAN DMZ la cui presenza sarebbe opportuna visto, che da
consegna, l&#39;azienda possiede un server che pubblica all&#39;esterno. Mancano, nell&#39;albero degli apparati
attivi, sia i server di sistema (DNS, DHCP almeno) che di business (un NAS con i dati almeno).
- Incongruenza postazioni TO: la consegna dice: su ogni piano si trovano 30 postazioni di lavoro
adesso da te sono 30 ma con 60 dispositivi quindi 60 prese RJ45, 60 cablaggi orizzontali

- Incongruenza ipotesi edificio: le tue ipotesi di cablaggio a stella di un edificio nuovo cozzano con la
planimetria a bus del cablaggio in pianta
- Incongruenza PP armadi: le definizioni degli armadi sono scorrette. il numero di PP non è
sufficiente a terminare il numero elevato di collegamenti che fanno capo ad ogni armadio
- Incongruenza planimetria-albero dorsali: hai almeno 5 armadi FD in planimetria ma nell&#39;albero
delle dorsali ce e stanno 3
- Incongruenza etichette albero-tabella dorsali: le dorsali nell&#39;albero degli apparati passivi
avrebbero dovuto essere etichettate con le stesse etichette usate nella tabella delle dorsali
- Incongruenza tra alberi apparati passivi e albero apparati attivi: sono 4 armadi FD per piano,
ciascuno con dentro uno SW di accesso (AS). Se i piani sono 4, e gli armadi sono 4 per piano allora
gli SW AS sono complessivamente 16 mentre nel disegno degli apparati attivi sono solo 4.
- Incongruenza alberi attivi/passivi: c&#39;è una incongruenza tra un FD collassato sul BD in planimetria e
invece separato nell&#39;albero degli apparati passivi


>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

