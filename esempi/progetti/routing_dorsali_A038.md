# Approfondimento — Routing sulle dorsali virtuali `/30` (cantieri ↔ sede)

> 📎 Approfondimento del **[Punto 3](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/esempi/progetti/risoluzione_A038_sistemi_reti_2026.md#punto-3--canali-cantiere--sede-e-dimensionamento-della-banda)** della risoluzione A038. Qui si dettaglia **come si raggiungono** le subnet remote sopra i tunnel punto-punto: prima con **routing statico** (le sole rotte da scrivere a mano), poi con **OSPF**.

---

## 1. Richiamo dello schema d'indirizzamento

**Subnet di accesso/servizio (`/24`)**

| Sito | Blocco | Subnet di esempio |
| ---- | ------ | ----------------- |
| Sede | `10.0.0.0/16` | `10.0.10.0/24` (uffici), `10.0.20.0/24` (Wi‑Fi), `10.0.30.0/24` (server farm), `10.0.40.0/24` (DMZ), `10.0.99.0/24` (mgmt) |
| Cantiere *k* | `10.k.0.0/16` | `10.k.10.0/24`, `10.k.20.0/24`, `10.k.30.0/24`, `10.k.99.0/24` |

**Dorsale dei tunnel (`/30`, da `10.255.0.0/24`)**

| Tunnel | Tratta | Subnet `/30` | IP cantiere | IP sede |
| ------ | ------ | ------------ | ----------- | ------- |
| `Tunnel1` | sede ↔ cantiere 1 | `10.255.0.0/30` | `10.255.0.1` | `10.255.0.2` |
| `Tunnel2` | sede ↔ cantiere 2 | `10.255.0.4/30` | `10.255.0.5` | `10.255.0.6` |
| `Tunnel3` | sede ↔ cantiere 3 | `10.255.0.8/30` | `10.255.0.9` | `10.255.0.10` |
| `Tunnel4` | sede ↔ cantiere 4 | `10.255.0.12/30` | `10.255.0.13` | `10.255.0.14` |
| `Tunnel5` | sede ↔ cantiere 5 | `10.255.0.16/30` | `10.255.0.17` | `10.255.0.18` |

La scelta dei blocchi è ciò che rende il routing **banale da riassumere**: tutta la sede sta in `10.0.0.0/16`, ogni cantiere in un singolo `10.k.0.0/16`.

## 2. Topologia logica: hub‑and‑spoke

```
        10.1.0.0/16            10.0.0.0/16            10.2.0.0/16
        ┌──────────┐  Tunnel1  ┌──────────┐  Tunnel2  ┌──────────┐
        │ CANTIERE1│═══════════│   SEDE   │═══════════│ CANTIERE2│
        │ (.0.1)   │  /30      │  (.0.2)  │  /30      │ (.0.5)   │
        └──────────┘           │ (.0.6)   │           └──────────┘
                          Tun3 │ (.0.10)  │ Tun4/Tun5
                    ┌──────────┘   hub    └──────────┐
              ┌──────────┐                     ┌──────────┐
              │ CANTIERE3│   ...               │ CANTIERE4│ ...
              └──────────┘                     └──────────┘
```

La **sede è l'hub**: ha 5 interfacce tunnel, una per cantiere. Ogni **cantiere è uno spoke** con un solo tunnel verso la sede. Non esistono tunnel diretti cantiere↔cantiere.

## 3. Cosa è «direttamente connesso» e cosa no

Una rotta **non** va scritta a mano se la subnet è raggiungibile da un'interfaccia attiva del router (entra in tabella come `C` / `L`). Vanno scritte a mano **solo** le destinazioni *remote*.

**Sul router di cantiere** sono già connesse: le 4 (o N) LAN `10.k.x.0/24` e il `/30` del proprio tunnel. → Manca **solo** la via verso la sede.

**Sul router di sede** sono già connesse: le 5 LAN `10.0.x.0/24` e **tutti** i `/30` dei tunnel. → Mancano **solo** le vie verso i cantieri.

---

## 4. Soluzione A — Routing statico

### 4.1 Lato cantiere (spoke) — *una sola rotta a mano*

Tutta la sede è in `10.0.0.0/16`: basta **una rotta riassuntiva** verso il capo‑tunnel della sede. (Il default `0.0.0.0/0` resta puntato sull'uscita Internet 4G/5G locale, **non** sul tunnel.)

| Cantiere | Destinazione remota | Next‑hop (sede) | Comando IOS |
| -------- | ------------------- | --------------- | ----------- |
| 1 | `10.0.0.0/16` | `10.255.0.2`  | `ip route 10.0.0.0 255.255.0.0 10.255.0.2` |
| 2 | `10.0.0.0/16` | `10.255.0.6`  | `ip route 10.0.0.0 255.255.0.0 10.255.0.6` |
| 3 | `10.0.0.0/16` | `10.255.0.10` | `ip route 10.0.0.0 255.255.0.0 10.255.0.10` |
| 4 | `10.0.0.0/16` | `10.255.0.14` | `ip route 10.0.0.0 255.255.0.0 10.255.0.14` |
| 5 | `10.0.0.0/16` | `10.255.0.18` | `ip route 10.0.0.0 255.255.0.0 10.255.0.18` |

```
! Esempio: router del cantiere 1
R-CANT1(config)# ip route 10.0.0.0 255.255.0.0 10.255.0.2
! (in alternativa, via interfaccia punto-punto:)
! R-CANT1(config)# ip route 10.0.0.0 255.255.0.0 Tunnel1
```

### 4.2 Lato sede (hub) — *una rotta per cantiere*

Ogni cantiere è un singolo `10.k.0.0/16`: **una rotta riassuntiva per spoke**, con next‑hop = IP del cantiere sul rispettivo `/30`.

| Destinazione remota | Next‑hop (cantiere) | Tunnel | Comando IOS |
| ------------------- | ------------------- | ------ | ----------- |
| `10.1.0.0/16` | `10.255.0.1`  | `Tunnel1` | `ip route 10.1.0.0 255.255.0.0 10.255.0.1` |
| `10.2.0.0/16` | `10.255.0.5`  | `Tunnel2` | `ip route 10.2.0.0 255.255.0.0 10.255.0.5` |
| `10.3.0.0/16` | `10.255.0.9`  | `Tunnel3` | `ip route 10.3.0.0 255.255.0.0 10.255.0.9` |
| `10.4.0.0/16` | `10.255.0.13` | `Tunnel4` | `ip route 10.4.0.0 255.255.0.0 10.255.0.13` |
| `10.5.0.0/16` | `10.255.0.17` | `Tunnel5` | `ip route 10.5.0.0 255.255.0.0 10.255.0.17` |

```
! Router di sede (hub): 5 rotte, una per spoke
R-SEDE(config)# ip route 10.1.0.0 255.255.0.0 10.255.0.1
R-SEDE(config)# ip route 10.2.0.0 255.255.0.0 10.255.0.5
R-SEDE(config)# ip route 10.3.0.0 255.255.0.0 10.255.0.9
R-SEDE(config)# ip route 10.4.0.0 255.255.0.0 10.255.0.13
R-SEDE(config)# ip route 10.5.0.0 255.255.0.0 10.255.0.17
```

**Bilancio:** 1 rotta statica su ogni cantiere + 5 rotte sulla sede. Tutto il resto è connesso.

### 4.3 Verifica

```
R-SEDE# show ip route static
S    10.1.0.0/16 [1/0] via 10.255.0.1
S    10.2.0.0/16 [1/0] via 10.255.0.5
...
R-CANT1# show ip route static
S    10.0.0.0/16 [1/0] via 10.255.0.2
R-CANT1# ping 10.0.30.10 source 10.1.30.1     ! tablet/server di cantiere → server farm sede
```

### 4.4 Varianti

- **Senza riassunto (più rotte, più esplicito).** Se si preferisce non riassumere, sul cantiere si scrivono le 5 rotte `/24` della sede (`10.0.10.0/24 … 10.0.99.0/24`) tutte via lo stesso next‑hop; sulla sede, le 4 rotte `/24` di ciascun cantiere. Funziona identico ma la tabella si gonfia.
- **Spoke‑to‑spoke (cantiere↔cantiere).** Con il solo statico minimale **non** è previsto: il traffico inter‑cantiere richiederebbe, su ogni spoke, una rotta verso `10.j.0.0/16` con next‑hop = sede (che già sa raggiungere ogni cantiere e fa da transito). Nello scenario A038 i cantieri dialogano **solo** con la sede, quindi si omette.
- **Floating static / failover.** Con un secondo tunnel di backup si aggiunge la stessa rotta con *distance* maggiore (es. `… 10.255.x.y 200`): entra in tabella solo se il primario cade.

---

## 5. Soluzione B — Routing dinamico OSPF

Stesso indirizzamento, stessi tunnel: si tolgono **tutte** le rotte statiche e si lascia che ogni router **annunci le proprie reti** e impari le altre. È la scelta indicata nella traccia (`ip ospf network point-to-point`, `area 0`, processo `100`).

### 5.1 Idea

Ogni `Tunnel` è un link **point‑to‑point** in **area 0**. Ogni router immette in OSPF le proprie LAN `/24` e il `/30` del tunnel. Il database link‑state si propaga: la **sede impara tutti i `10.k.0.0/16`**, ogni **cantiere impara `10.0.0.0/16`** della sede. Niente rotte a mano, e l'aggiunta del cantiere 6 non tocca la sede.

### 5.2 Configurazione lato sede (hub)

```
R-SEDE(config)# router ospf 100
R-SEDE(config-router)#  router-id 10.255.0.2
R-SEDE(config-router)#  passive-interface default        ! nessun hello sulle LAN...
R-SEDE(config-router)#  no passive-interface Tunnel1      ! ...ma adiacenze sui tunnel
R-SEDE(config-router)#  no passive-interface Tunnel2
R-SEDE(config-router)#  no passive-interface Tunnel3
R-SEDE(config-router)#  no passive-interface Tunnel4
R-SEDE(config-router)#  no passive-interface Tunnel5
R-SEDE(config-router)#  network 10.0.0.0 0.0.255.255 area 0     ! tutte le LAN di sede
R-SEDE(config-router)#  network 10.255.0.0 0.0.0.255 area 0     ! tutti i /30 dei tunnel
R-SEDE(config-router)#  exit
! su OGNI interfaccia tunnel:
R-SEDE(config)# interface range Tunnel1 - 5
R-SEDE(config-if-range)#  ip ospf network point-to-point
```

> `network 10.0.0.0 0.0.255.255` cattura `10.0.10.254`, `10.0.20.254`, … (tutti i gateway `/24` della sede); `network 10.255.0.0 0.0.0.255` cattura i cinque `/30`.

### 5.3 Configurazione lato cantiere (spoke)

```
! Esempio: cantiere 1
R-CANT1(config)# router ospf 100
R-CANT1(config-router)#  router-id 10.255.0.1
R-CANT1(config-router)#  passive-interface default
R-CANT1(config-router)#  no passive-interface Tunnel1
R-CANT1(config-router)#  network 10.1.0.0 0.0.255.255 area 0    ! le LAN del cantiere
R-CANT1(config-router)#  network 10.255.0.0 0.0.0.3   area 0    ! il /30 del tunnel
R-CANT1(config-router)#  exit
R-CANT1(config)# interface Tunnel1
R-CANT1(config-if)#  ip ospf network point-to-point
```

Le `network` cambiano solo nei numeri, per cantiere:

| Cantiere | LAN | Tunnel `/30` |
| -------- | --- | ------------ |
| 1 | `network 10.1.0.0 0.0.255.255 area 0` | `network 10.255.0.0 0.0.0.3 area 0` |
| 2 | `network 10.2.0.0 0.0.255.255 area 0` | `network 10.255.0.4 0.0.0.3 area 0` |
| 3 | `network 10.3.0.0 0.0.255.255 area 0` | `network 10.255.0.8 0.0.0.3 area 0` |
| 4 | `network 10.4.0.0 0.0.255.255 area 0` | `network 10.255.0.12 0.0.0.3 area 0` |
| 5 | `network 10.5.0.0 0.0.255.255 area 0` | `network 10.255.0.16 0.0.0.3 area 0` |

### 5.4 Perché GRE + `point-to-point` (e non IPsec «nudo»)

OSPF scambia hello/LSA in **multicast** (`224.0.0.5`). L'IPsec in *tunnel mode* puro **non trasporta multicast**: per questo la traccia usa **GRE protetto da IPsec** — è GRE a incapsulare il traffico di routing, IPsec a cifrarlo. Il comando `ip ospf network point-to-point` evita l'elezione di **DR/BDR** (inutile su un link a 2 nodi) e accelera la convergenza.

### 5.5 Verifica

```
R-SEDE# show ip ospf neighbor          ! 5 vicini, uno per tunnel, stato FULL
R-SEDE# show ip route ospf
O    10.1.0.0/16 [110/...] via 10.255.0.1, Tunnel1
O    10.2.0.0/16 [110/...] via 10.255.0.5, Tunnel2
...
R-CANT1# show ip route ospf
O    10.0.0.0/16 [110/...] via 10.255.0.2, Tunnel1
```

### 5.6 Note importanti

- **Nessun default in OSPF.** Non si usa `default-information originate`: ogni sito mantiene la **propria uscita Internet** (4G/5G in cantiere, fibra in sede). In OSPF viaggiano **solo** le reti interne `10.0.0.0/8`.
- **Spoke‑to‑spoke “gratis”.** Essendo tutto in area 0, ogni cantiere impara anche i `10.j.0.0/16` degli altri, con **next‑hop = sede** (transito sull'hub). Se si vuole **isolare** i cantieri tra loro, si filtra con `distribute-list`/ACL o ridisegnando le aree.
- **Sicurezza OSPF.** Su ogni tunnel conviene l'autenticazione del protocollo (`ip ospf authentication message-digest` + `ip ospf message-digest-key 1 md5 <chiave>`) per evitare adiacenze fasulle.

---

## 6. Statico vs OSPF — confronto sintetico

| Criterio | Statico | OSPF |
| -------- | ------- | ---- |
| Rotte da scrivere | 1 per spoke + 5 sull'hub | nessuna rotta, solo `network`/interfacce |
| Aggiunta cantiere 6 | si modifica **anche** la sede | il nuovo spoke si annuncia da solo |
| Guasto/cambio link | nessuna reazione (serve floating static) | riconvergenza automatica |
| Spoke‑to‑spoke | da aggiungere a mano | automatico via hub |
| Carico/complessità | nullo, deterministico | hello/LSA, va autenticato |
| Quando preferirlo | pochi siti, stabili, controllo totale | rete che cresce/cambia (caso A038) |

**In sintesi:** con 5 cantieri stabili lo statico è compattissimo (grazie ai blocchi `/16`); ma poiché i cantieri sono *temporanei e variabili*, la traccia adotta **OSPF**, che propaga i `/24` da solo e assorbe aggiunte e guasti senza ritoccare l'hub.

---

## 7. Come collegarlo al documento principale

Salva questo file come `routing_dorsali_A038.md` accanto agli altri e aggiungi nel Punto 3 una riga del tipo:

```markdown
> 📎 **Routing sulle dorsali `/30`** (tabelle statiche lato cantiere/sede con le sole rotte
> non connesse, + soluzione OSPF equivalente): vedi
> [routing_dorsali_A038.md](https://github.com/sebastianomelita/ArduinoBareMetal/blob/master/esempi/progetti/routing_dorsali_A038.md).
```
