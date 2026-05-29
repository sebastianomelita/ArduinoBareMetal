>[Torna a svolgimento Smart Road](svolgimento_smart_road.md)>[Torna a Dettaglio architettura LoRaWAN](/lorawanclasses.md) 
>


# Dettaglio — Variante di subnetting: da classful a classless per i tratti regionali

> Documento di dettaglio collegato allo svolgimento principale (sezione §4, Piano di indirizzamento).
> Mostra come evolvere lo schema di indirizzamento da un'assegnazione "classful" (un ottetto per i tratti, massimo 256) a una **classless** in grado di contare oltre 1000 tratti per regione, con margine per estensioni future.
> Impostazione ispirata al modello di riferimento `sebastianomelita/ArduinoBareMetal` (albapparatiattivi.md, "Subnetting azienda sanitaria pubblica").

---

## Indice

- [1. Il limite dello schema classful iniziale](#1-il-limite-dello-schema-classful-iniziale)
- [2. Requisiti della nuova ripartizione](#2-requisiti-della-nuova-ripartizione)
- [3. Calcolo dei bit: regione / tratto / host](#3-calcolo-dei-bit-regione--tratto--host)
- [4. Struttura del campo indirizzo](#4-struttura-del-campo-indirizzo)
- [5. Tabella delle subnet di tratto](#5-tabella-delle-subnet-di-tratto)
- [6. Aggregazione per regione (route summarization)](#6-aggregazione-per-regione-route-summarization)
- [7. Margine per i tratti pericolosi delle statali](#7-margine-per-i-tratti-pericolosi-delle-statali)

---

## 1. Il limite dello schema classful iniziale

Nello svolgimento principale (§4) il piano di indirizzamento usa una ripartizione "a ottetti", facilmente leggibile:

```
10.<RR>.<TT>.<NNN>
   │    │    │
   │    │    └── host nel tratto (un ottetto: 0-255)
   │    └─────── numero tratto nella regione (un ottetto: 0-255)
   └──────────── codice regione (un ottetto: 1-20)
```

Questo schema è di fatto **classful semplificato**: ogni campo occupa esattamente un ottetto, quindi i confini cadono sempre su multipli di 8 bit. È elegante e leggibile, ma ha un limite strutturale: **il campo tratto, occupando un solo ottetto, conta al massimo 256 tratti per regione**.

Finché i tratti sperimentali sono pochi (~50 per regione) il limite non si avverte. Ma se il progetto si estende — per esempio integrando nella smart-road non solo le autostrade ma anche i **tratti pericolosi delle strade statali** di ciascuna regione — i 256 tratti possono non bastare più. Serve quindi passare a uno schema **classless**, in cui i confini tra i campi non cadono più necessariamente sui multipli di 8 bit, ma si scelgono in funzione del numero effettivo di elementi da contare.

---

## 2. Requisiti della nuova ripartizione

1. **Regioni**: almeno **5 bit**, per contare **32 regioni** (le 20 italiane più un margine per province autonome, raggruppamenti speciali, o future riorganizzazioni).
2. **Tratti per regione**: capacità di contare **oltre 1000 tratti** (superando il limite dei 256), con **margine** per aggiungere in futuro le subnet dei tratti pericolosi delle statali.
3. **Host per tratto**: mantenere **256 indirizzi** (un ottetto intero) per ciascun tratto, così da preservare la leggibilità dell'ultimo ottetto e disporre di spazio per gli smart-gate, le stazioni di ricarica e gli apparati del tratto.

---

## 3. Calcolo dei bit: regione / tratto / host

Si parte dallo spazio privato **`10.0.0.0/8`** (RFC 1918), che lascia **24 bit** liberi dopo il primo ottetto. Questi 24 bit vanno ripartiti tra regione, tratto e host:

```
24 bit disponibili = bit_regione + bit_tratto + bit_host
```

Fissando i due requisiti rigidi:

- `bit_regione = 5`  →  2⁵ = **32 regioni**
- `bit_host = 8`  →  2⁸ = **256 indirizzi per tratto** (253 host utili)

i bit rimanenti per i tratti sono:

```
bit_tratto = 24 − 5 − 8 = 11  →  2¹¹ = 2048 tratti per regione
```

| Campo | Bit | Capacità | Note |
|-------|-----|----------|------|
| Regione | 5 | 32 regioni | 20 italiane + 12 di margine |
| Tratto | 11 | 2048 tratti/regione | requisito ">1000" abbondantemente soddisfatto |
| Host | 8 | 256 indirizzi/tratto | 253 host utili (esclusi rete, broadcast, gateway) |
| **Totale** | **24** | | dentro `10.0.0.0/8` |

Il prefisso di ogni subnet di tratto risulta quindi:

```
/(8 + 5 + 11) = /24
```

Da notare un fatto elegante: con questa scelta (5+11+8) il prefisso di tratto torna a essere un **/24**, cioè un "classico" blocco da 256 indirizzi facilissimo da leggere — ma a differenza dello schema classful iniziale, **il campo tratto ora si estende su due ottetti** (gli ultimi 3 bit del secondo ottetto + tutto il terzo), proprio come nell'esempio dell'azienda sanitaria del materiale di riferimento, dove il prefisso era "spalmato" su due ottetti.

---

## 4. Struttura del campo indirizzo

I 24 bit dopo `10.` sono organizzati così:

```
   ottetto 2          ottetto 3          ottetto 4
 ┌───────────┐     ┌───────────┐      ┌───────────┐
 R R R R R T T T   T T T T T T T T    H H H H H H H H
 └─── 5 ──┘└─ 3 ┘  └──── 8 ─────┘     └──── 8 ──────┘
  regione   │                          host
            └──────── tratto: 11 bit ──┘
            (3 bit alti nel 2° ottetto + 8 bit nel 3°)
```

Il campo **regione** (5 bit) occupa i bit alti del secondo ottetto; il campo **tratto** (11 bit) è "a cavallo" tra i 3 bit bassi del secondo ottetto e tutto il terzo ottetto; il campo **host** (8 bit) coincide con il quarto ottetto.

**Esempio di decomposizione** — Regione 3, Tratto 700:

```
24 bit:   00011 01010111100 00000000
          └─R─┘ └──tratto──┘ └─host─┘
regione = 00011        = 3
tratto  = 01010111100  = 700
host    = 00000000     = 0 (indirizzo di rete)

Subnet risultante: 10.26.188.0/24
```

(Il secondo ottetto `00011 010` = 26, il terzo ottetto `10111100` = 188.)

---

## 5. Tabella delle subnet di tratto

Sul modello della tabella dell'azienda sanitaria, ogni subnet di tratto è un `/24`. Il range DHCP è qui ipotizzato sugli ultimi indirizzi prima del gateway (gli host bassi sono riservati ad assegnazioni statiche per smart-gate e apparati di rete).

| Tratto (R, T) | Ind. subnet | Broadcast | Ind. gateway | Range host | Range DHCP |
|---------------|-------------|-----------|--------------|------------|------------|
| Reg 0, Tratto 0 | `10.0.0.0/24` | `10.0.0.255` | `10.0.0.254` | `.0.1 – .0.253` | `.0.200 – .0.253` |
| Reg 0, Tratto 1 | `10.0.1.0/24` | `10.0.1.255` | `10.0.1.254` | `.1.1 – .1.253` | `.1.200 – .1.253` |
| Reg 0, Tratto 2 | `10.0.2.0/24` | `10.0.2.255` | `10.0.2.254` | `.2.1 – .2.253` | `.2.200 – .2.253` |
| … | … | … | … | … | … |
| Reg 0, Tratto 49 | `10.0.49.0/24` | `10.0.49.255` | `10.0.49.254` | `.49.1 – .49.253` | `.49.200 – .49.253` |
| … | … | … | … | … | … |
| Reg 0, Tratto 2047 | `10.7.255.0/24` | `10.7.255.255` | `10.7.255.254` | `.255.1 – .255.253` | `.255.200 – .255.253` |
| Reg 1, Tratto 0 | `10.8.0.0/24` | `10.8.0.255` | `10.8.0.254` | `.0.1 – .0.253` | `.0.200 – .0.253` |
| Reg 2, Tratto 0 | `10.16.0.0/24` | `10.16.0.255` | `10.16.0.254` | `.0.1 – .0.253` | `.0.200 – .0.253` |
| … | … | … | … | … | … |
| Reg 31, Tratto 2047 | `10.255.255.0/24` | `10.255.255.255` | `10.255.255.254` | `.255.1 – .255.253` | `.255.200 – .255.253` |

Si osservi come, passando da una regione alla successiva, il "salto" nel secondo ottetto sia di **8** (`10.0.x` → `10.8.x` → `10.16.x` …): è la conseguenza diretta del fatto che i 3 bit bassi del secondo ottetto appartengono al campo tratto, quindi ogni regione occupa un blocco di 8 valori del secondo ottetto. 

In altre parole, considerando come parte di interesse tutta quella che comprende i bit di tratto (11 bit), questa è composta di 11 bit di cui solo 3 sono nel secondo ottetto (l'ottetto di lavoro per questo host). Il riporto per questa subnet nell'ottetto di lavoro è proprio di 2^3=8 che è il limite a partire dal quale terminal'ultimo tratto e scatta una nuova regione (il prefisso di tratto diventa di nuovo tutto 0 e un uno si aggiunge al prefisso di regione). 

---

## 6. Aggregazione per regione (route summarization)

Un vantaggio fondamentale di questo schema gerarchico è la **summarization**: tutti i 2048 tratti di una regione sono contigui e aggregabili in un'unica rotta. Il prefisso aggregato di regione è:

```
/(8 + 5) = /13
```

| Regione | Blocco aggregato /13 | Contiene |
|---------|----------------------|----------|
| Reg 0 | `10.0.0.0/13` | tratti `10.0.0.0/24` … `10.7.255.0/24` |
| Reg 1 | `10.8.0.0/13` | tratti `10.8.0.0/24` … `10.15.255.0/24` |
| Reg 2 | `10.16.0.0/13` | tratti `10.16.0.0/24` … `10.23.255.0/24` |
| … | … | … |
| Reg 31 | `10.248.0.0/13` | tratti `10.248.0.0/24` … `10.255.255.0/24` |

Nella rete nazionale, il Centro Nazionale e i router di dorsale non devono conoscere le 2048 rotte interne di una regione: ne basta **una sola** (`10.<blocco>.0.0/13`) per raggiungere l'intero Centro di Controllo regionale, che poi instrada internamente verso i singoli tratti. Questo riduce drasticamente la dimensione delle tabelle di routing del backbone nazionale e isola i guasti: un problema su un tratto della Lombardia non genera churn nelle tabelle di routing della Sicilia.

---

## 7. Margine per i tratti pericolosi delle statali

Il requisito di lasciare un margine per estensioni future è ampiamente soddisfatto. Stimando i tratti autostradali sperimentali in circa **50 per regione**, la capacità di **2048 tratti** lascia un margine di **circa 1998 subnet** per regione, utilizzabili per integrare progressivamente:

- i **tratti pericolosi delle strade statali** della regione (curve cieche, gallerie a rischio, attraversamenti soggetti a nebbia o ghiaccio), ciascuno gestito come un tratto smart-road a sé stante con i propri smart-gate;
- tratti provinciali o urbani ad alta incidentalità;
- aree speciali (caselli, aree di servizio attrezzate, svincoli complessi).

Una buona pratica organizzativa è **riservare a priori dei sotto-range** del campo tratto a categorie diverse, per mantenere ordine. Ad esempio, all'interno degli 11 bit di tratto:

| Range tratto | Uso |
|--------------|-----|
| `0 – 255` | Tratti autostradali sperimentali (compatibilità con lo schema classful originario) |
| `256 – 1023` | Tratti pericolosi delle strade statali |
| `1024 – 1791` | Tratti provinciali/urbani ad alta incidentalità |
| `1792 – 2047` | Aree speciali e riserva |

In questo modo la migrazione dallo schema classful a quello classless è **retro-compatibile**: i tratti già numerati 0–255 mantengono lo stesso identificativo logico, e le nuove categorie si aggiungono nei range superiori senza rinumerare nulla.
