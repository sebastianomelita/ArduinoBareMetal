# **Esercizio di Subnetting IPv6**

## **Esigenza**

Dati i gruppi da 3000 e 40 host:

1\) Realizzare il subnetting classful di un indirizzo di documentazione definendo gli indirizzi link local e GUA di ognidispositivo e di subnet di ogni subnet (GRP).

2\) Definire l'indirizzo multicast di un PC a scelta

3\) Considerato il primo subnetting come una divisione in dipartimenti, farne un secondo con una divisione in settori e trovare l'indirizzo del quarto settore del terzo dipartimento

## **Dati del problema:**

- Gruppi da 3000 e 40 host  
- Richiesta di subnetting classful (a cavallo tra un nibble e l'altro)  
- Necessità di definire indirizzi link-local e GUA (Global Unicast Address)  
- Definire indirizzo multicast di un PC  
- Fare un secondo subnetting per settori all'interno dei dipartimenti

## **1\) Subnetting Classful per 3000 e 40 host**

### **Analisi dei requisiti**

- Per 3000 host: abbiamo bisogno di 12 bit (2^12 \= 4096 \> 3000\)  
- Per 40 host: abbiamo bisogno di 6 bit (2^6 \= 64 \> 40\)

### **Subnetting per 3000 host (dipartimenti)**

Utilizziamo l'indirizzo di documentazione 2001:db8::/32 come base.

Per seguire il suggerimento di fare il subnetting a cavallo tra un nibble e l'altro, definiamo:

- Prefisso base: 2001:db8::/32  
- Per il primo livello di subnetting (dipartimenti), utilizziamo 12 bit, partendo da bit 32  
- Questo ci porta a un prefisso /44 (32 \+ 12), che attraversa il confine tra il terzo e il quarto gruppo esadecimale

#### **Indirizzi dei dipartimenti:**

1. Primo dipartimento: 2001:db8:0000::/44 (rappresentato anche come 2001:db8:0::/44)  
2. Secondo dipartimento: 2001:db8:0010::/44  
3. Terzo dipartimento: 2001:db8:0020::/44  
4. Quarto dipartimento: 2001:db8:0030::/44 ...e così via.

### **Definizione degli indirizzi**

Per ogni subnet del primo livello (dipartimenti), definiamo:

1. **Indirizzo di subnet GUA**:

   - Dipartimento 1: 2001:db8:0000::/44  
   - Dipartimento 2: 2001:db8:0010::/44  
   - Dipartimento 3: 2001:db8:0020::/44  
2. **Indirizzi GUA dei dispositivi**: Per il primo dipartimento:

   - Host 1: 2001:db8:0000::1/44  
   - Host 2: 2001:db8:0000::2/44  
   - Host 3: 2001:db8:0000::3/44  
3. **Indirizzi link-local**: Gli indirizzi link-local iniziano sempre con fe80:: e sono seguiti dall'identificatore di interfaccia.

   - Host 1: fe80::1  
   - Host 2: fe80::2  
   - Host 3: fe80::3

## **2\) Definizione dell'indirizzo multicast di un PC**

Scegliamo l'Host 1 del primo dipartimento per definire un indirizzo multicast.

Gli indirizzi multicast IPv6 iniziano con ff e includono:

- Bandiere (0 \= permanente, 1 \= temporaneo)  
- Ambito (1 \= nodo locale, 2 \= collegamento locale, 5 \= sito locale, e \= globale)  
- ID del gruppo multicast

Per un indirizzo multicast a livello di collegamento locale per l'Host 1:

- **Indirizzo multicast**: ff02::1 (tutti i nodi nel collegamento locale)

## **3\) Secondo livello di subnetting (Settori all'interno dei Dipartimenti)**

Per il secondo livello di subnetting, suddividiamo ulteriormente i dipartimenti in settori:

- Partiamo dal prefisso /44 dei dipartimenti  
- Per i settori (40 host), necessitiamo di 6 bit  
- Quindi il prefisso per i settori sarà /50 (44 \+ 6\)

### **Definizione dei settori all'interno dei dipartimenti**

Per il terzo dipartimento (2001:db8:0020::/44):

1. Primo settore: 2001:db8:0020:0000::/50  
2. Secondo settore: 2001:db8:0020:0400::/50  
3. Terzo settore: 2001:db8:0020:0800::/50  
4. **Quarto settore: 2001:db8:0020:0c00::/50**

L'indirizzo del quarto settore del terzo dipartimento è quindi **2001:db8:0020:0c00::/50**.

## **Verifica della soluzione**

### **Note sul subnetting a cavallo tra nibble**

Nel nostro esercizio:

- Il primo livello di subnetting (dipartimenti) ha un prefisso /44, che attraversa il confine tra il terzo gruppo (0020) e il quarto gruppo (0000)  
- Il secondo livello di subnetting (settori) ha un prefisso /50, che cade a metà del quarto gruppo esadecimale

### **Tabella riassuntiva degli indirizzi**

| Livello | Descrizione | Indirizzo/Prefisso |
| :---- | :---- | :---- |
| Base | Prefisso di documentazione | 2001:db8::/32 |
| Dipartimento 1 | Primo dipartimento | 2001:db8:0000::/44 |
| Dipartimento 2 | Secondo dipartimento | 2001:db8:0010::/44 |
| Dipartimento 3 | Terzo dipartimento | 2001:db8:0020::/44 |
| Settore 3.1 | Primo settore del terzo dipartimento | 2001:db8:0020:0000::/50 |
| Settore 3.2 | Secondo settore del terzo dipartimento | 2001:db8:0020:0400::/50 |
| Settore 3.3 | Terzo settore del terzo dipartimento | 2001:db8:0020:0800::/50 |
| Settore 3.4 | Quarto settore del terzo dipartimento | 2001:db8:0020:0c00::/50 |
| Host 1 GUA | Indirizzo globale Host 1 (dip. 1\) | 2001:db8:0000::1/44 |
| Host 1 LL | Indirizzo link-local Host 1 | fe80::1 |
| Host 1 MC | Indirizzo multicast Host 1 | ff02::1 |

## **7\. Considerazioni Finali**

Come sottolineato nel documento, è importante adattare lo schema di indirizzamento IPv6 alle specifiche esigenze dell'organizzazione:

*"Of course you'll want to do what works best for your organization and aligns with your business goals and requirements. There are no requirements regarding the site and sub-site or even how these are broken down."*

I principali vantaggi del subnetting IPv6 sono:

1. **Spazio di indirizzamento praticamente illimitato**  
2. **Possibilità di strutturare gerarchicamente la rete**  
3. **Semplificazione del routing e dell'aggregazione**  
4. **Eliminazione della necessità di NAT**  
5. **Maggiore flessibilità nella progettazione**

La transizione a IPv6 rappresenta un'opportunità per ripensare e ottimizzare l'architettura di rete, abbandonando le limitazioni e i compromessi imposti da IPv4.

\[IMMAGINE 15: Confronto tra IPv4 e IPv6 che mostra i vantaggi del nuovo protocollo\]

