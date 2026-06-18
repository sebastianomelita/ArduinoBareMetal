# Subnetting IPv6 — Gerarchia Geografica Italia
### Prefisso ISP: `2001:db8:1::/48` — Classful e Classless a confronto

---

## Indice

1. [Contesto e obiettivo](#1-contesto-e-obiettivo)
2. [Struttura degli indirizzi IPv6](#2-struttura-degli-indirizzi-ipv6)
3. [Subnetting Classful (nibble-aligned)](#3-subnetting-classful-nibble-aligned)
4. [Subnetting Classless (bit-exact)](#4-subnetting-classless-bit-exact)
5. [Dorsali e infrastruttura di rete](#5-dorsali-e-infrastruttura-di-rete)
6. [Confronto tra i due approcci](#6-confronto-tra-i-due-approcci)
7. [Tabella riepilogativa finale](#7-tabella-riepilogativa-finale)

---

## 1. Contesto e obiettivo

Un ISP assegna all'organizzazione il prefisso `/48`:

```
2001:db8:1::/48
```

L'obiettivo è strutturare gerarchicamente lo spazio di indirizzamento in modo da rispecchiare la geografia italiana, riservando però anche uno spazio dedicato alla **rete di trasporto** (la dorsale) che interconnette i router della gerarchia:

```
Italia  →  Regioni  →  Province  →  Subnet di sede
   └─────→  Infrastruttura / Dorsale (asse parallelo)
```

I dati di riferimento sono:

| Livello             | Entità reali     | Note                              |
|---------------------|-----------------|-----------------------------------|
| Infrastruttura      | slot dedicato   | Loopback router + link P2P        |
| Regioni             | 20              | Regioni italiane                  |
| Province            | max 12          | Lombardia (la più grande)         |
| Subnet/sede         | a scelta        | LAN, Server, VoIP, DMZ, ecc.     |

> La gerarchia geografica descrive **dove** stanno le sedi; la dorsale descrive **come** sono collegate. Sono assi ortogonali: per questo allo spazio dell'infrastruttura si riserva uno **slot di livello regione** (lo slot `00`), separato dalle regioni reali. Dettagli nella [sezione 5](#5-dorsali-e-infrastruttura-di-rete).

---

## 2. Struttura degli indirizzi IPv6

Un indirizzo IPv6 è composto da **128 bit**, suddivisi in **8 gruppi da 16 bit** (hexteto), separati da `:`.

```
2001:0db8:0001:XXYY:0000:0000:0000:0000
|______________|____|__________________|
   48 bit ISP   16b     64 bit host
               nostra
             gerarchia
```

I **16 bit centrali** (bit 48–63) sono quelli che possiamo suddividere per la gerarchia interna.

Rappresentati in esadecimale, questi 16 bit formano l'**hexteto 4**:

```
.... .... .... .... | R R P P | .... ....
                      ^ ^^ ^
                      | ||_|_____ PP = province / subnet (nibble basso)
                      |_|________ RR = regioni (nibble alto, 2 nibble = 8 bit)
```

> Lo **slot regione `00`** è convenzionalmente riservato all'infrastruttura: nessuna regione reale lo usa, così ogni indirizzo che inizia per `2001:db8:1:00..` è immediatamente riconoscibile come "dorsale".

---

## 3. Subnetting Classful (nibble-aligned)

Il subnetting **classful in IPv6** allinea i tagli ai confini di nibble (4 bit).  
I prefissi ammessi sono: `/48`, `/52`, `/56`, `/60`, `/64`.

### Distribuzione dei nibble disponibili

Disponiamo di **4 nibble** (16 bit) tra `/48` e `/64`:

```
/48 ──── /52 ──── /56 ──── /60 ──── /64
   nibble1  nibble2  nibble3  nibble4
```

Scelta ottimale:

| Nibble usati | Livello   | Prefisso | Slot disponibili | Slot usati |
|:------------:|-----------|:--------:|:----------------:|:----------:|
| 2            | Regioni   | /56      | 256              | 20 ✅      |
| 1            | Province  | /60      | 16               | 12 ✅      |
| 1            | Subnet    | /64      | **16**           | a scelta   |

> **Con 1 nibble per le regioni (/52) si ottengono solo 16 slot — insufficienti per le 20 regioni italiane.**  
> Quindi si usano 2 nibble per le regioni → prefisso /56.

> Lo slot `0x0000` (regione `00`) è riservato all'infrastruttura/dorsale; le regioni reali partono quindi da `0x0100`.

---

### Livello 1 — Regioni `/56`

Incremento: `+0x01` nel byte alto dell'hexteto 4.

| #  | Regione          | Prefisso /56             |
|----|-----------------|--------------------------|
| 00 | *Infrastruttura* | `2001:db8:1:0000::/56`   |
| 01 | Piemonte         | `2001:db8:1:0100::/56`   |
| 02 | Valle d'Aosta    | `2001:db8:1:0200::/56`   |
| 03 | Lombardia        | `2001:db8:1:0300::/56`   |
| 04 | Trentino-AA      | `2001:db8:1:0400::/56`   |
| 05 | Veneto           | `2001:db8:1:0500::/56`   |
| 06 | Friuli-VG        | `2001:db8:1:0600::/56`   |
| 07 | Liguria          | `2001:db8:1:0700::/56`   |
| 08 | Emilia-Romagna   | `2001:db8:1:0800::/56`   |
| 09 | Toscana          | `2001:db8:1:0900::/56`   |
| 10 | Umbria           | `2001:db8:1:0a00::/56`   |
| 11 | Marche           | `2001:db8:1:0b00::/56`   |
| 12 | Lazio            | `2001:db8:1:0c00::/56`   |
| 13 | Abruzzo          | `2001:db8:1:0d00::/56`   |
| 14 | Molise           | `2001:db8:1:0e00::/56`   |
| 15 | Campania         | `2001:db8:1:0f00::/56`   |
| 16 | Puglia           | `2001:db8:1:1000::/56`   |
| 17 | Basilicata       | `2001:db8:1:1100::/56`   |
| 18 | Calabria         | `2001:db8:1:1200::/56`   |
| 19 | Sicilia          | `2001:db8:1:1300::/56`   |
| 20 | Sardegna         | `2001:db8:1:1400::/56`   |

> Slot liberi: da `0x1500` a `0xff00` → **235 regioni di riserva** (espansione futura).

---

### Livello 2 — Province della Lombardia `/60`

Prendiamo **Lombardia** → `2001:db8:1:0300::/56`  
Incremento: `+0x10` nel nibble basso dell'hexteto 4.

| #  | Provincia      | Prefisso /60               |
|----|---------------|---------------------------|
| 1  | Milano         | `2001:db8:1:0300::/60`    |
| 2  | Bergamo        | `2001:db8:1:0310::/60`    |
| 3  | Brescia        | `2001:db8:1:0320::/60`    |
| 4  | Como           | `2001:db8:1:0330::/60`    |
| 5  | Cremona        | `2001:db8:1:0340::/60`    |
| 6  | Lecco          | `2001:db8:1:0350::/60`    |
| 7  | Lodi           | `2001:db8:1:0360::/60`    |
| 8  | Mantova        | `2001:db8:1:0370::/60`    |
| 9  | Monza-Brianza  | `2001:db8:1:0380::/60`    |
| 10 | Pavia          | `2001:db8:1:0390::/60`    |
| 11 | Sondrio        | `2001:db8:1:03a0::/60`    |
| 12 | Varese         | `2001:db8:1:03b0::/60`    |
| —  | (riserva)      | `03c0` → `03f0`           |

> Slot liberi per provincia: 4 (da `03c0` a `03f0`).

---

### Livello 3 — Subnet sede di Milano `/64`

Prendiamo **Milano** → `2001:db8:1:0300::/60`  
Il nibble residuo genera **16 subnet /64** (incremento `+0x1` nel nibble finale).

| #  | Prefisso /64               | Uso suggerito         |
|----|---------------------------|-----------------------|
| 1  | `2001:db8:1:0300::/64`    | LAN utenti            |
| 2  | `2001:db8:1:0301::/64`    | Server interni        |
| 3  | `2001:db8:1:0302::/64`    | VoIP                  |
| 4  | `2001:db8:1:0303::/64`    | DMZ                   |
| 5  | `2001:db8:1:0304::/64`    | Management / OOB      |
| 6  | `2001:db8:1:0305::/64`    | IoT / Building Mgmt   |
| 7  | `2001:db8:1:0306::/64`    | Wi-Fi ospiti          |
| 8  | `2001:db8:1:0307::/64`    | Backup / Storage      |
| 9–16 | `0308` → `030f`        | Riserva               |

> **Ogni sede provinciale dispone di 16 subnet /64** con ~18,4 × 10¹⁸ indirizzi host ciascuna.

---

### Spreco del classful

| Livello  | Bit reali necessari | Bit assegnati (nibble) | Bit sprecati |
|----------|:-------------------:|:----------------------:|:------------:|
| Regioni  | 5 (per 20)          | 8 (2 nibble)           | **3 bit**    |
| Province | 4 (per 12)          | 4 (1 nibble)           | 0 bit        |
| Subnet   | 4 (per 16)          | 4 (1 nibble)           | 0 bit        |
| **Totale** |                   |                        | **3 bit**    |

I 3 bit sprecati al livello regioni significano che **7/8 dello spazio** di quel livello è inutilizzato.

---

## 4. Subnetting Classless (bit-exact)

Il subnetting **classless** usa esattamente i bit necessari, indipendentemente dai confini di nibble.

### Calcolo bit necessari

| Livello  | Entità | Bit esatti         | Prefisso        |
|----------|-------:|:------------------:|:---------------:|
| Regioni  | 20     | 5 bit (2⁵=32≥20)  | /48+5 = **/53** |
| Province | 12     | 4 bit (2⁴=16≥12)  | /53+4 = **/57** |
| Subnet   | 16     | 4 bit (2⁴=16)     | /57+4 = **/61** |

> ⚠️ Il prefisso `/61` **attraversa il confine del /64**, il che **rompe SLAAC** (Stateless Address AutoConfiguration).  
> Per preservare la compatibilità SLAAC, si estende l'ultimo livello fino a /64 — sprecando 3 bit sul livello subnet — oppure si riduce il livello subnet a 3 bit (8 subnet).

**Soluzione consigliata con SLAAC rispettato:**

| Livello  | Bit | Prefisso | Slot   |
|----------|:---:|:--------:|:------:|
| Regioni  | 5   | /53      | 32     |
| Province | 4   | /57      | 16     |
| Subnet   | 7 → esteso a /64 | **/64** | 128 ✅ |

> Con questo approccio le subnet per sede diventano **128** invece di 16.  
> Anche qui lo **slot regione `00`** è riservato all'infrastruttura: le regioni reali partono da `0x0800`.

---

### Livello 1 — Regioni `/53`

Incremento: `+0x0800` nell'hexteto 4 (ogni passo avanza di 2¹¹ = 2048 in decimale).

| #  | Regione          | Prefisso /53             |
|----|-----------------|--------------------------|
| 00 | *Infrastruttura* | `2001:db8:1:0000::/53`   |
| 01 | Piemonte         | `2001:db8:1:0800::/53`   |
| 02 | Valle d'Aosta    | `2001:db8:1:1000::/53`   |
| 03 | Lombardia        | `2001:db8:1:1800::/53`   |
| 04 | Trentino-AA      | `2001:db8:1:2000::/53`   |
| 05 | Veneto           | `2001:db8:1:2800::/53`   |
| 06 | Friuli-VG        | `2001:db8:1:3000::/53`   |
| 07 | Liguria          | `2001:db8:1:3800::/53`   |
| 08 | Emilia-Romagna   | `2001:db8:1:4000::/53`   |
| 09 | Toscana          | `2001:db8:1:4800::/53`   |
| 10 | Umbria           | `2001:db8:1:5000::/53`   |
| 11 | Marche           | `2001:db8:1:5800::/53`   |
| 12 | Lazio            | `2001:db8:1:6000::/53`   |
| 13 | Abruzzo          | `2001:db8:1:6800::/53`   |
| 14 | Molise           | `2001:db8:1:7000::/53`   |
| 15 | Campania         | `2001:db8:1:7800::/53`   |
| 16 | Puglia           | `2001:db8:1:8000::/53`   |
| 17 | Basilicata       | `2001:db8:1:8800::/53`   |
| 18 | Calabria         | `2001:db8:1:9000::/53`   |
| 19 | Sicilia          | `2001:db8:1:9800::/53`   |
| 20 | Sardegna         | `2001:db8:1:a000::/53`   |

> Slot liberi: da `0xa800` a `0xf800` → **11 regioni di riserva** (contro le 235 del classful), oltre allo slot `00` dedicato all'infrastruttura.

---

### Livello 2 — Province della Lombardia `/57`

Prendiamo **Lombardia** → `2001:db8:1:1800::/53`  
Incremento: `+0x0080` (2⁷ = 128 per ogni passo).

| #  | Provincia      | Prefisso /57               |
|----|---------------|---------------------------|
| 1  | Milano         | `2001:db8:1:1800::/57`    |
| 2  | Bergamo        | `2001:db8:1:1880::/57`    |
| 3  | Brescia        | `2001:db8:1:1900::/57`    |
| 4  | Como           | `2001:db8:1:1980::/57`    |
| 5  | Cremona        | `2001:db8:1:1a00::/57`    |
| 6  | Lecco          | `2001:db8:1:1a80::/57`    |
| 7  | Lodi           | `2001:db8:1:1b00::/57`    |
| 8  | Mantova        | `2001:db8:1:1b80::/57`    |
| 9  | Monza-Brianza  | `2001:db8:1:1c00::/57`    |
| 10 | Pavia          | `2001:db8:1:1c80::/57`    |
| 11 | Sondrio        | `2001:db8:1:1d00::/57`    |
| 12 | Varese         | `2001:db8:1:1d80::/57`    |
| —  | (riserva)      | `1e00` → `1f80`           |

> Slot liberi per provincia: 4 (da `1e00` a `1f80`).

---

### Livello 3 — Subnet sede di Milano `/64`

Prendiamo **Milano** → `2001:db8:1:1800::/57`  
I 7 bit rimanenti fino al /64 generano **128 subnet /64**.

| #   | Prefisso /64               | Uso suggerito         |
|-----|---------------------------|-----------------------|
| 1   | `2001:db8:1:1800::/64`    | LAN utenti            |
| 2   | `2001:db8:1:1801::/64`    | Server interni        |
| 3   | `2001:db8:1:1802::/64`    | VoIP                  |
| 4   | `2001:db8:1:1803::/64`    | DMZ                   |
| 5   | `2001:db8:1:1804::/64`    | Management / OOB      |
| 6   | `2001:db8:1:1805::/64`    | IoT / Building Mgmt   |
| 7   | `2001:db8:1:1806::/64`    | Wi-Fi ospiti          |
| 8   | `2001:db8:1:1807::/64`    | Backup / Storage      |
| …   | `1808` → `187f`           | Riserva (120 subnet)  |

> **Ogni sede provinciale dispone di 128 subnet /64** — 8× il classful.

---

## 5. Dorsali e infrastruttura di rete

Il piano fin qui indirizza le **sedi** (LAN, Server, VoIP, DMZ…), ma una rete reale ha bisogno anche di indirizzi per la **rete di trasporto**: i link punto-punto fra i router e i loro indirizzi di **loopback**. Questo è l'insieme che chiamiamo **dorsale**.

### Perché riservarle uno spazio dedicato

- **Separazione infrastruttura/utenti** — gli indirizzi di dorsale non si mescolano con le subnet di produzione. Si può applicare una *infrastructure ACL* (iACL) che renda la dorsale non raggiungibile né pingabile dall'esterno: meno superficie d'attacco.
- **Sommarizzazione** — un unico blocco si riassume in una sola route, mantenendo pulite le tabelle IGP/BGP.
- **Loopback stabili** — ogni router ha bisogno di un `/128` fisso per management, router-id IGP, peering BGP ed eventuale MPLS, indipendente da quale interfaccia fisica è attiva.
- **Leggibilità e troubleshooting** — un indirizzo `2001:db8:1:00..` è riconoscibile a colpo d'occhio come "dorsale".

### Regole di assegnazione

| Risorsa            | Prefisso per elemento | Origine                       | Riferimento |
|--------------------|:---------------------:|-------------------------------|-------------|
| Loopback router    | **/128**              | `2001:db8:1:0000::/64`        | —           |
| Link punto-punto   | **/127**              | `2001:db8:1:0001::/64`        | RFC 6164    |

Un link tra due router usa solo 2 indirizzi (`::0` e `::1` del `/127`). La **RFC 6164** raccomanda il `/127` sui link inter-router perché evita due problemi noti del `/64`: l'attacco di esaurimento della cache di *Neighbor Discovery* (NDP exhaustion) e il fenomeno del *ping-pong*. Da un singolo `/64` si ricavano comunque innumerevoli `/127`.

> Chi preferisce l'uniformità a volte mantiene un `/64` anche sui P2P usando solo `::1`/`::2`: funziona ed è coerente con il resto del piano, ma il `/127` resta la best practice di sicurezza tra router.

### Mappa del blocco infrastruttura (`2001:db8:1:0000::/56` classful · `…/53` classless)

```
2001:db8:1:0000::/56   ── Slot 00 · Infrastruttura / Dorsale
  ├─ 2001:db8:1:0000::/64    Loopback dei router  (1 × /128 per nodo)
  │     2001:db8:1:0000::1/128   core-milano
  │     2001:db8:1:0000::2/128   core-roma
  │     2001:db8:1:0000::3/128   core-torino
  │     2001:db8:1:0000::4/128   core-napoli
  ├─ 2001:db8:1:0001::/64    Link P2P dorsale     (1 × /127 per link)
  │     2001:db8:1:0001::0/127   MI ↔ RM
  │     2001:db8:1:0001::2/127   RM ↔ NA
  │     2001:db8:1:0001::4/127   NA ↔ TO
  │     2001:db8:1:0001::6/127   TO ↔ MI
  └─ 2001:db8:1:0002::/64    Transito verso ISP / link di peering
```

### Figura — anello di core router

<svg viewBox="0 0 720 520" xmlns="http://www.w3.org/2000/svg" style="max-width:720px;width:100%;height:auto;font-family:system-ui,-apple-system,Segoe UI,Roboto,sans-serif">
  <rect x="0" y="0" width="720" height="520" fill="#f8fafc" rx="14"/>
  <text x="360" y="34" text-anchor="middle" font-size="18" font-weight="700" fill="#0f172a">Dorsale IPv6 — anello di core router</text>
  <text x="360" y="56" text-anchor="middle" font-size="13" fill="#475569">Blocco infrastruttura: 2001:db8:1:0000::/56</text>

  <!-- Links (drawn first, boxes overlap the ends) -->
  <!-- MI - RM (top) -->
  <line x1="280" y1="135" x2="440" y2="135" stroke="#475569" stroke-width="2.5"/>
  <g>
    <rect x="300" y="123" width="120" height="24" rx="12" fill="#ffffff" stroke="#cbd5e1"/>
    <text x="360" y="139" text-anchor="middle" font-size="12" font-family="ui-monospace,Menlo,Consolas,monospace" fill="#0f172a">0001::0/127</text>
  </g>
  <!-- RM - NA (right) -->
  <line x1="560" y1="162" x2="560" y2="358" stroke="#475569" stroke-width="2.5"/>
  <g>
    <rect x="500" y="248" width="120" height="24" rx="12" fill="#ffffff" stroke="#cbd5e1"/>
    <text x="560" y="264" text-anchor="middle" font-size="12" font-family="ui-monospace,Menlo,Consolas,monospace" fill="#0f172a">0001::2/127</text>
  </g>
  <!-- NA - TO (bottom) -->
  <line x1="440" y1="385" x2="280" y2="385" stroke="#475569" stroke-width="2.5"/>
  <g>
    <rect x="300" y="373" width="120" height="24" rx="12" fill="#ffffff" stroke="#cbd5e1"/>
    <text x="360" y="389" text-anchor="middle" font-size="12" font-family="ui-monospace,Menlo,Consolas,monospace" fill="#0f172a">0001::4/127</text>
  </g>
  <!-- TO - MI (left) -->
  <line x1="160" y1="358" x2="160" y2="162" stroke="#475569" stroke-width="2.5"/>
  <g>
    <rect x="100" y="248" width="120" height="24" rx="12" fill="#ffffff" stroke="#cbd5e1"/>
    <text x="160" y="264" text-anchor="middle" font-size="12" font-family="ui-monospace,Menlo,Consolas,monospace" fill="#0f172a">0001::6/127</text>
  </g>

  <!-- Routers -->
  <!-- MI top-left -->
  <g>
    <rect x="60" y="108" width="200" height="54" rx="11" fill="#eef2ff" stroke="#6366f1" stroke-width="2"/>
    <text x="160" y="132" text-anchor="middle" font-size="15" font-weight="700" fill="#1e1b4b">Core Milano</text>
    <text x="160" y="151" text-anchor="middle" font-size="11.5" font-family="ui-monospace,Menlo,Consolas,monospace" fill="#4338ca">2001:db8:1:0000::1/128</text>
  </g>
  <!-- RM top-right -->
  <g>
    <rect x="460" y="108" width="200" height="54" rx="11" fill="#eef2ff" stroke="#6366f1" stroke-width="2"/>
    <text x="560" y="132" text-anchor="middle" font-size="15" font-weight="700" fill="#1e1b4b">Core Roma</text>
    <text x="560" y="151" text-anchor="middle" font-size="11.5" font-family="ui-monospace,Menlo,Consolas,monospace" fill="#4338ca">2001:db8:1:0000::2/128</text>
  </g>
  <!-- TO bottom-left -->
  <g>
    <rect x="60" y="358" width="200" height="54" rx="11" fill="#eef2ff" stroke="#6366f1" stroke-width="2"/>
    <text x="160" y="382" text-anchor="middle" font-size="15" font-weight="700" fill="#1e1b4b">Core Torino</text>
    <text x="160" y="401" text-anchor="middle" font-size="11.5" font-family="ui-monospace,Menlo,Consolas,monospace" fill="#4338ca">2001:db8:1:0000::3/128</text>
  </g>
  <!-- NA bottom-right -->
  <g>
    <rect x="460" y="358" width="200" height="54" rx="11" fill="#eef2ff" stroke="#6366f1" stroke-width="2"/>
    <text x="560" y="382" text-anchor="middle" font-size="15" font-weight="700" fill="#1e1b4b">Core Napoli</text>
    <text x="560" y="401" text-anchor="middle" font-size="11.5" font-family="ui-monospace,Menlo,Consolas,monospace" fill="#4338ca">2001:db8:1:0000::4/128</text>
  </g>

  <!-- Legend -->
  <g>
    <line x1="120" y1="468" x2="160" y2="468" stroke="#475569" stroke-width="2.5"/>
    <text x="168" y="472" font-size="12" fill="#334155">Link P2P /127 — da 2001:db8:1:0001::/64</text>
    <rect x="430" y="459" width="18" height="18" rx="4" fill="#eef2ff" stroke="#6366f1" stroke-width="2"/>
    <text x="456" y="472" font-size="12" fill="#334155">Loopback /128 — da 2001:db8:1:0000::/64</text>
  </g>
</svg>

> Le sedi regionali e provinciali (sezioni 3–4) si agganciano a questo anello tramite i propri router di edge, che ricevono a loro volta un loopback dal blocco `2001:db8:1:0000::/64` e un link `/127` dal blocco `2001:db8:1:0001::/64`.

---

## 6. Confronto tra i due approcci

| Caratteristica              | Classful (nibble)       | Classless (bit-exact)   |
|-----------------------------|-------------------------|-------------------------|
| Prefisso regioni            | /56                     | /53                     |
| Prefisso province           | /60                     | /57                     |
| Prefisso subnet sede        | /64                     | /64                     |
| Blocco dorsale dedicato     | ✅ slot 00 → /56         | ✅ slot 00 → /53         |
| Loopback router             | /128 da `0000::/64`     | /128 da `0000::/64`     |
| Link P2P (RFC 6164)         | /127 da `0001::/64`     | /127 da `0001::/64`     |
| Regioni disponibili         | 256 (1 infra + 20 + 235)| 32 (1 infra + 20 + 11)  |
| Province per regione        | 16                      | 16                      |
| **Subnet /64 per sede**     | **16**                  | **128**                 |
| Spreco bit gerarchia        | 3 bit                   | 0 bit                   |
| Leggibilità indirizzi hex   | ✅ Molto alta            | ⚠️ Media (tagli oddly-aligned) |
| Compatibilità SLAAC         | ✅ Sì (/64 finale)       | ✅ Sì (/64 finale)       |
| Semplicità configurazione   | ✅ Alta                  | ⚠️ Media                |
| Efficienza spazio           | ⚠️ Media                 | ✅ Alta                  |

---

## 7. Tabella riepilogativa finale

### Classful

| Livello           | Descrizione                  | Prefisso                     |
|-------------------|------------------------------|------------------------------|
| ISP               | Blocco assegnato             | `2001:db8:1::/48`            |
| Infrastruttura    | Dorsale (slot 00)            | `2001:db8:1:0000::/56`       |
| ↳ Loopback        | es. core Milano              | `2001:db8:1:0000::1/128`     |
| ↳ Link P2P        | es. MI ↔ RM                  | `2001:db8:1:0001::0/127`     |
| Regione 03        | Lombardia                    | `2001:db8:1:0300::/56`       |
| Provincia 01      | Milano (Lombardia)           | `2001:db8:1:0300::/60`       |
| Subnet 1          | LAN utenti — Milano          | `2001:db8:1:0300::/64`       |
| Subnet 2          | Server — Milano              | `2001:db8:1:0301::/64`       |
| …                 | …                            | …                            |
| Subnet 16         | Ultima subnet — Milano       | `2001:db8:1:030f::/64`       |

### Classless

| Livello           | Descrizione                  | Prefisso                     |
|-------------------|------------------------------|------------------------------|
| ISP               | Blocco assegnato             | `2001:db8:1::/48`            |
| Infrastruttura    | Dorsale (slot 00)            | `2001:db8:1:0000::/53`       |
| ↳ Loopback        | es. core Milano              | `2001:db8:1:0000::1/128`     |
| ↳ Link P2P        | es. MI ↔ RM                  | `2001:db8:1:0001::0/127`     |
| Regione 03        | Lombardia                    | `2001:db8:1:1800::/53`       |
| Provincia 01      | Milano (Lombardia)           | `2001:db8:1:1800::/57`       |
| Subnet 1          | LAN utenti — Milano          | `2001:db8:1:1800::/64`       |
| Subnet 2          | Server — Milano              | `2001:db8:1:1801::/64`       |
| …                 | …                            | …                            |
| Subnet 128        | Ultima subnet — Milano       | `2001:db8:1:187f::/64`       |

---

> **Conclusione:** il classful è preferibile per leggibilità e semplicità operativa; il classless è preferibile quando si vogliono massimizzare le subnet disponibili per sede (128 vs 16) senza sprecare bit nella gerarchia superiore. In entrambi i casi lo **slot `00`** è riservato alla dorsale — loopback `/128` e link punto-punto `/127` (RFC 6164) — e il prefisso finale `/64` garantisce la piena compatibilità con SLAAC ed EUI-64.
