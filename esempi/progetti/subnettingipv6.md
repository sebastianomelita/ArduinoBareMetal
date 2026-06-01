# Subnetting IPv6 — Gerarchia Geografica Italia
### Prefisso ISP: `2001:db8:1::/48` — Classful e Classless a confronto

---

## Indice

1. [Contesto e obiettivo](#1-contesto-e-obiettivo)
2. [Struttura degli indirizzi IPv6](#2-struttura-degli-indirizzi-ipv6)
3. [Subnetting Classful (nibble-aligned)](#3-subnetting-classful-nibble-aligned)
4. [Subnetting Classless (bit-exact)](#4-subnetting-classless-bit-exact)
5. [Confronto tra i due approcci](#5-confronto-tra-i-due-approcci)
6. [Tabella riepilogativa finale](#6-tabella-riepilogativa-finale)

---

## 1. Contesto e obiettivo

Un ISP assegna all'organizzazione il prefisso `/48`:

```
2001:db8:1::/48
```

L'obiettivo è strutturare gerarchicamente lo spazio di indirizzamento in modo da rispecchiare la geografia italiana:

```
Italia  →  Regioni  →  Province  →  Subnet di sede
```

I dati di riferimento sono:

| Livello       | Entità reali     | Note                          |
|---------------|-----------------|-------------------------------|
| Regioni       | 20              | Regioni italiane              |
| Province      | max 12          | Lombardia (la più grande)     |
| Subnet/sede   | a scelta        | LAN, Server, VoIP, DMZ, ecc. |

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

---

### Livello 1 — Regioni `/56`

Incremento: `+0x01` nel byte alto dell'hexteto 4.

| #  | Regione          | Prefisso /56             |
|----|-----------------|--------------------------|
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

---

### Livello 1 — Regioni `/53`

Incremento: `+0x0800` nell'hexteto 4 (ogni passo avanza di 2¹¹ = 2048 in decimale).

| #  | Regione          | Prefisso /53             |
|----|-----------------|--------------------------|
| 01 | Piemonte         | `2001:db8:1:0000::/53`   |
| 02 | Valle d'Aosta    | `2001:db8:1:0800::/53`   |
| 03 | Lombardia        | `2001:db8:1:1000::/53`   |
| 04 | Trentino-AA      | `2001:db8:1:1800::/53`   |
| 05 | Veneto           | `2001:db8:1:2000::/53`   |
| 06 | Friuli-VG        | `2001:db8:1:2800::/53`   |
| 07 | Liguria          | `2001:db8:1:3000::/53`   |
| 08 | Emilia-Romagna   | `2001:db8:1:3800::/53`   |
| 09 | Toscana          | `2001:db8:1:4000::/53`   |
| 10 | Umbria           | `2001:db8:1:4800::/53`   |
| 11 | Marche           | `2001:db8:1:5000::/53`   |
| 12 | Lazio            | `2001:db8:1:5800::/53`   |
| 13 | Abruzzo          | `2001:db8:1:6000::/53`   |
| 14 | Molise           | `2001:db8:1:6800::/53`   |
| 15 | Campania         | `2001:db8:1:7000::/53`   |
| 16 | Puglia           | `2001:db8:1:7800::/53`   |
| 17 | Basilicata       | `2001:db8:1:8000::/53`   |
| 18 | Calabria         | `2001:db8:1:8800::/53`   |
| 19 | Sicilia          | `2001:db8:1:9000::/53`   |
| 20 | Sardegna         | `2001:db8:1:9800::/53`   |

> Slot liberi: da `0xa000` a `0xf800` → **12 regioni di riserva** (contro le 235 del classful).

---

### Livello 2 — Province della Lombardia `/57`

Prendiamo **Lombardia** → `2001:db8:1:1000::/53`  
Incremento: `+0x0080` (2⁷ = 128 per ogni passo).

| #  | Provincia      | Prefisso /57               |
|----|---------------|---------------------------|
| 1  | Milano         | `2001:db8:1:1000::/57`    |
| 2  | Bergamo        | `2001:db8:1:1080::/57`    |
| 3  | Brescia        | `2001:db8:1:1100::/57`    |
| 4  | Como           | `2001:db8:1:1180::/57`    |
| 5  | Cremona        | `2001:db8:1:1200::/57`    |
| 6  | Lecco          | `2001:db8:1:1280::/57`    |
| 7  | Lodi           | `2001:db8:1:1300::/57`    |
| 8  | Mantova        | `2001:db8:1:1380::/57`    |
| 9  | Monza-Brianza  | `2001:db8:1:1400::/57`    |
| 10 | Pavia          | `2001:db8:1:1480::/57`    |
| 11 | Sondrio        | `2001:db8:1:1500::/57`    |
| 12 | Varese         | `2001:db8:1:1580::/57`    |
| —  | (riserva)      | `1600` → `17ff`           |

> Slot liberi per provincia: 4 (da `1600` a `1780`).

---

### Livello 3 — Subnet sede di Milano `/64`

Prendiamo **Milano** → `2001:db8:1:1000::/57`  
I 7 bit rimanenti fino al /64 generano **128 subnet /64**.

| #   | Prefisso /64               | Uso suggerito         |
|-----|---------------------------|-----------------------|
| 1   | `2001:db8:1:1000::/64`    | LAN utenti            |
| 2   | `2001:db8:1:1001::/64`    | Server interni        |
| 3   | `2001:db8:1:1002::/64`    | VoIP                  |
| 4   | `2001:db8:1:1003::/64`    | DMZ                   |
| 5   | `2001:db8:1:1004::/64`    | Management / OOB      |
| 6   | `2001:db8:1:1005::/64`    | IoT / Building Mgmt   |
| 7   | `2001:db8:1:1006::/64`    | Wi-Fi ospiti          |
| 8   | `2001:db8:1:1007::/64`    | Backup / Storage      |
| … | `1008` → `107f`           | Riserva (120 subnet)  |

> **Ogni sede provinciale dispone di 128 subnet /64** — 8× il classful.

---

## 5. Confronto tra i due approcci

| Caratteristica              | Classful (nibble)       | Classless (bit-exact)   |
|-----------------------------|-------------------------|-------------------------|
| Prefisso regioni            | /56                     | /53                     |
| Prefisso province           | /60                     | /57                     |
| Prefisso subnet sede        | /64                     | /64                     |
| Regioni disponibili         | 256 (usate 20)          | 32 (usate 20)           |
| Province per regione        | 16                      | 16                      |
| **Subnet /64 per sede**     | **16**                  | **128**                 |
| Spreco bit gerarchia        | 3 bit                   | 0 bit                   |
| Leggibilità indirizzi hex   | ✅ Molto alta            | ⚠️ Media (tagli oddly-aligned) |
| Compatibilità SLAAC         | ✅ Sì (/64 finale)       | ✅ Sì (/64 finale)       |
| Semplicità configurazione   | ✅ Alta                  | ⚠️ Media                |
| Efficienza spazio           | ⚠️ Media                 | ✅ Alta                  |

---

## 6. Tabella riepilogativa finale

### Classful

| Livello           | Descrizione                  | Prefisso                     |
|-------------------|------------------------------|------------------------------|
| ISP               | Blocco assegnato             | `2001:db8:1::/48`            |
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
| Regione 03        | Lombardia                    | `2001:db8:1:1000::/53`       |
| Provincia 01      | Milano (Lombardia)           | `2001:db8:1:1000::/57`       |
| Subnet 1          | LAN utenti — Milano          | `2001:db8:1:1000::/64`       |
| Subnet 2          | Server — Milano              | `2001:db8:1:1001::/64`       |
| …                 | …                            | …                            |
| Subnet 128        | Ultima subnet — Milano       | `2001:db8:1:107f::/64`       |

---

> **Conclusione:** il classful è preferibile per leggibilità e semplicità operativa; il classless è preferibile quando si vogliono massimizzare le subnet disponibili per sede (128 vs 16) senza sprecare bit nella gerarchia superiore. In entrambi i casi il prefisso finale `/64` garantisce la piena compatibilità con SLAAC e EUI-64.
