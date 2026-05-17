>[Torna a reti WiFi Mesh](../archmesh.md#esempio-1)


# Esempio — Mesh WiFi routed con subnetting e routing statico

> Variante L3 dell'esempio classico "3 router WiFi mesh con backhaul dedicato e
> due nodi di aggregazione sensori", realizzata interamente con **subnetting
> statico** e **routing statico**. Nessun protocollo dinamico, nessun
> meccanismo di auto-configurazione: ogni indirizzo IP è pianificato a tavolino
> e ogni rotta è scritta a mano nei file di configurazione.

## Scenario

Tre router WiFi mesh:

- **R1** — gateway verso Internet, nessun client direttamente collegato.
- **R2** — nodo di aggregazione, ospita la subnet sensori `192.168.2.0/24`.
- **R3** — nodo di aggregazione, ospita la subnet sensori `192.168.3.0/24`.

Ogni router ha due interfacce radio: una dedicata al **backhaul** (dorsale
verso gli altri router) su canale isolato, e una dedicata all'**accesso
client** (sensori).

R1 è collegato a R2 e R3 con due link mesh wireless distinti, ciascuno su un
canale dedicato (come da specifica "ogni router ha una dorsale con canale di
comunicazione dedicato"). Non c'è link diretto R2↔R3: tutto il traffico
inter-subnet passa da R1.

## Schema della rete

```
                       ┌──────────────────┐
                       │   Internet       │
                       └────────┬─────────┘
                                │ WAN (DHCP/statico)
                       ┌────────┴─────────┐
                       │       R1         │
                       │   gateway        │
                       │                  │
                       │ mesh0: .1.1/30   │
                       │ mesh1: .1.5/30   │
                       └───┬──────────┬───┘
                           │          │
              dorsale ch.A │          │ dorsale ch.B
              192.168.1.0  │          │ 192.168.1.4
                  /30      │          │     /30
                           │          │
                    ┌──────┴───┐  ┌───┴──────┐
                    │    R2    │  │    R3    │
                    │mesh0:    │  │mesh0:    │
                    │  .1.2/30 │  │  .1.6/30 │
                    │br-lan:   │  │br-lan:   │
                    │ .2.254   │  │ .3.254   │
                    └────┬─────┘  └────┬─────┘
                         │             │
              SSID Sensors_R2   SSID Sensors_R3
              192.168.2.0/24    192.168.3.0/24
                  │                 │
              ┌───┴───┐         ┌───┴───┐
              │sensori│         │sensori│
              │  S2   │         │  S3   │
              └───────┘         └───────┘
```

**Caratteristiche della variante statica:**

- Ogni link di dorsale ha la sua subnet `/30` dedicata (2 host utili, perfetta
  per un link punto-punto).
- Tutti gli indirizzi sono assegnati manualmente da un piano IP scritto.
- Ogni router ha rotte statiche esplicite per raggiungere le subnet non
  direttamente connesse.
- Non gira alcun demone di routing: niente Babel, niente OLSR, niente OSPF.

## Piano di indirizzamento

### Subnet di dorsale (link punto-punto `/30`)

| Link | Subnet | R1 | R2/R3 |
|------|--------|----|----|
| R1 ↔ R2 (canale A) | `192.168.1.0/30` | `192.168.1.1` | R2 = `192.168.1.2` |
| R1 ↔ R3 (canale B) | `192.168.1.4/30` | `192.168.1.5` | R3 = `192.168.1.6` |

Spiegazione del `/30`: ogni subnet contiene 4 indirizzi, di cui 2 utili. Per
esempio per `192.168.1.0/30`: `.0` è network, `.1` e `.2` sono gli host, `.3`
è broadcast. È lo standard per i link di transito numerati.

### Subnet di accesso (sensori)

| Subnet | Rete | Gateway | SSID | DHCP |
|--------|------|---------|------|------|
| S2 (sensori R2) | `192.168.2.0/24` | `192.168.2.254` (R2) | `Sensors_R2` | `.1`–`.253` |
| S3 (sensori R3) | `192.168.3.0/24` | `192.168.3.254` (R3) | `Sensors_R3` | `.1`–`.253` |

### Riepilogo indirizzi per nodo

**R1**
- `eth0` (WAN): da DHCP del provider
- `mesh0` (verso R2): `192.168.1.1/30`
- `mesh1` (verso R3): `192.168.1.5/30`

**R2**
- `mesh0` (verso R1): `192.168.1.2/30`
- `br-lan` (sensori S2): `192.168.2.254/24`

**R3**
- `mesh0` (verso R1): `192.168.1.6/30`
- `br-lan` (sensori S3): `192.168.3.254/24`

## Tabelle di routing statico

### R1 (gateway)

R1 è direttamente connesso a tutte le subnet di dorsale e all'uplink WAN. Le
subnet sensori sono raggiunte attraverso i rispettivi nodi di aggregazione:

| Destinazione | Next-hop | Interfaccia |
|--------------|----------|-------------|
| `192.168.1.0/30` | (connessa) | mesh0 |
| `192.168.1.4/30` | (connessa) | mesh1 |
| `192.168.2.0/24` | `192.168.1.2` (R2) | mesh0 |
| `192.168.3.0/24` | `192.168.1.6` (R3) | mesh1 |
| `0.0.0.0/0` | gateway ISP | eth0 |

### R2

R2 conosce direttamente la sua subnet sensori e la sua dorsale verso R1.
Tutto il resto del mondo (Internet + S3) passa attraverso R1:

| Destinazione | Next-hop | Interfaccia |
|--------------|----------|-------------|
| `192.168.2.0/24` | (connessa) | br-lan |
| `192.168.1.0/30` | (connessa) | mesh0 |
| `192.168.3.0/24` | `192.168.1.1` (R1) | mesh0 |
| `0.0.0.0/0` | `192.168.1.1` (R1) | mesh0 |

In pratica, dato che sia S3 che Internet passano da R1, basta la default verso
R1 e si può anche omettere la rotta esplicita per `192.168.3.0/24`. La tengo
per chiarezza didattica.

### R3

Simmetrico a R2:

| Destinazione | Next-hop | Interfaccia |
|--------------|----------|-------------|
| `192.168.3.0/24` | (connessa) | br-lan |
| `192.168.1.4/30` | (connessa) | mesh0 |
| `192.168.2.0/24` | `192.168.1.5` (R1) | mesh0 |
| `0.0.0.0/0` | `192.168.1.5` (R1) | mesh0 |

## Configurazione (OpenWrt)

I file mostrati sono semplificati e contengono solo le parti rilevanti per il
routing; tralascio firewall, system, ecc. che sono standard.

### R1 — gateway

**`/etc/config/network`**

```
config interface 'loopback'
    option device 'lo'
    option proto 'static'
    option ipaddr '127.0.0.1'
    option netmask '255.0.0.0'

# Uplink Internet
config interface 'wan'
    option device 'eth0'
    option proto 'dhcp'

# Dorsale verso R2
config interface 'backhaul_r2'
    option device 'mesh0'
    option proto 'static'
    option ipaddr '192.168.1.1'
    option netmask '255.255.255.252'

# Dorsale verso R3
config interface 'backhaul_r3'
    option device 'mesh1'
    option proto 'static'
    option ipaddr '192.168.1.5'
    option netmask '255.255.255.252'

# Rotte statiche: come raggiungere le subnet sensori
config route
    option interface 'backhaul_r2'
    option target '192.168.2.0/24'
    option gateway '192.168.1.2'

config route
    option interface 'backhaul_r3'
    option target '192.168.3.0/24'
    option gateway '192.168.1.6'
```

**`/etc/config/wireless`** (estratto)

```
config wifi-iface 'mesh_a'
    option device 'radio0'
    option network 'backhaul_r2'
    option mode 'mesh'
    option mesh_id 'Mesh_Backhaul_A'
    option encryption 'sae'
    option key 'CHIAVE_BACKHAUL_A'

config wifi-iface 'mesh_b'
    option device 'radio1'
    option network 'backhaul_r3'
    option mode 'mesh'
    option mesh_id 'Mesh_Backhaul_B'
    option encryption 'sae'
    option key 'CHIAVE_BACKHAUL_B'
```

R1 NAT-ta il traffico in uscita sull'interfaccia `wan` (zona firewall `wan` con
`masq '1'`). Le subnet sensori `192.168.2.0/24` e `192.168.3.0/24` devono
essere abilitate a uscire attraverso la WAN (regola di forwarding zona
`lan` → `wan`).

### R2 — nodo di aggregazione sensori `192.168.2.0/24`

**`/etc/config/network`**

```
config interface 'loopback'
    option device 'lo'
    option proto 'static'
    option ipaddr '127.0.0.1'
    option netmask '255.0.0.0'

# Subnet di accesso sensori
config interface 'lan'
    option device 'br-lan'
    option proto 'static'
    option ipaddr '192.168.2.254'
    option netmask '255.255.255.0'

# Dorsale verso R1
config interface 'backhaul_r1'
    option device 'mesh0'
    option proto 'static'
    option ipaddr '192.168.1.2'
    option netmask '255.255.255.252'
    option gateway '192.168.1.1'

# Rotta esplicita verso la subnet sensori di R3 (didattica;
# tecnicamente la default route verso R1 basterebbe)
config route
    option interface 'backhaul_r1'
    option target '192.168.3.0/24'
    option gateway '192.168.1.1'
```

L'attributo `option gateway '192.168.1.1'` su `backhaul_r1` imposta la **default
route** del nodo R2 puntando a R1.

**`/etc/config/wireless`** (estratto)

```
# Backhaul verso R1
config wifi-iface 'mesh_to_r1'
    option device 'radio0'
    option network 'backhaul_r1'
    option mode 'mesh'
    option mesh_id 'Mesh_Backhaul_A'
    option encryption 'sae'
    option key 'CHIAVE_BACKHAUL_A'

# Accesso sensori
config wifi-iface 'access_sensors'
    option device 'radio1'
    option network 'lan'
    option mode 'ap'
    option ssid 'Sensors_R2'
    option encryption 'psk2'
    option key 'CHIAVE_SENSORI_R2'
```

**`/etc/config/dhcp`** (estratto)

```
config dhcp 'lan'
    option interface 'lan'
    option start '1'
    option limit '253'
    option leasetime '12h'
```

### R3 — nodo di aggregazione sensori `192.168.3.0/24`

Configurazione speculare a R2. Indirizzi diversi, struttura identica.

**`/etc/config/network`**

```
config interface 'loopback'
    option device 'lo'
    option proto 'static'
    option ipaddr '127.0.0.1'
    option netmask '255.0.0.0'

config interface 'lan'
    option device 'br-lan'
    option proto 'static'
    option ipaddr '192.168.3.254'
    option netmask '255.255.255.0'

config interface 'backhaul_r1'
    option device 'mesh0'
    option proto 'static'
    option ipaddr '192.168.1.6'
    option netmask '255.255.255.252'
    option gateway '192.168.1.5'

config route
    option interface 'backhaul_r1'
    option target '192.168.2.0/24'
    option gateway '192.168.1.5'
```

**Parametri specifici di R3:**

| Parametro | Valore |
|-----------|--------|
| `lan` ipaddr | `192.168.3.254/24` |
| `backhaul_r1` ipaddr | `192.168.1.6/30` |
| default gateway | `192.168.1.5` (R1) |
| SSID accesso | `Sensors_R3` |
| `mesh_id` backhaul | `Mesh_Backhaul_B` |
| Chiave backhaul | `CHIAVE_BACKHAUL_B` |

## Verifica del funzionamento

Una volta caricate le configurazioni, su ciascun nodo si può verificare la
tabella di routing con:

```sh
ip route
```

**Output atteso su R1:**

```
default via <ip-isp> dev eth0
192.168.1.0/30 dev mesh0 proto kernel scope link src 192.168.1.1
192.168.1.4/30 dev mesh1 proto kernel scope link src 192.168.1.5
192.168.2.0/24 via 192.168.1.2 dev mesh0
192.168.3.0/24 via 192.168.1.6 dev mesh1
```

**Output atteso su R2:**

```
default via 192.168.1.1 dev mesh0
192.168.1.0/30 dev mesh0 proto kernel scope link src 192.168.1.2
192.168.2.0/24 dev br-lan proto kernel scope link src 192.168.2.254
192.168.3.0/24 via 192.168.1.1 dev mesh0
```

**Test di raggiungibilità:**

Da un sensore in S2 (es. `192.168.2.10`):

```sh
ping 192.168.2.254    # gateway R2: deve rispondere subito
ping 192.168.1.1      # interfaccia dorsale di R1: deve rispondere
ping 192.168.1.6      # interfaccia di R3 verso R1: deve rispondere
ping 192.168.3.10     # un sensore in S3: deve rispondere (passa per R1)
ping 8.8.8.8          # Internet: deve rispondere (NAT su R1)
```

## Considerazioni operative

### Cosa va bene

- **Trasparenza totale.** Ogni indirizzo è tracciabile sul piano IP cartaceo,
  ogni hop è visibile in traceroute, ogni rotta è scritta da qualcuno e
  motivabile. Per chi studia o per chi deve documentare la rete a fini
  certificativi, è la variante più chiara.
- **Diagnostica semplice.** Se R2 non vede S3 si sa esattamente dove
  guardare: la rotta `192.168.3.0/24 via 192.168.1.1` su R2 o quella
  `192.168.3.0/24 via 192.168.1.6` su R1.
- **Nessuna dipendenza da daemon esterni.** Funziona con il solo kernel
  Linux, senza pacchetti aggiuntivi tipo `babeld`, `frr`, `bird`.

### Limiti

- **Aggiungere un nodo R4 richiede modifiche su tutti gli altri.** Bisogna
  pianificare una nuova subnet di dorsale (es. `192.168.1.8/30`), aggiungere
  rotte statiche su R1 e su tutti i nodi che devono raggiungere la nuova
  zona, eventualmente aggiungere rotte da R4 verso S2 ed S3.
- **Nessun failover automatico.** Se il link R1↔R2 cade, R2 perde Internet
  e la raggiungibilità da S3, anche se ci fosse un link alternativo. La
  ridondanza va gestita manualmente.
- **Spreco di indirizzi.** Ogni `/30` di dorsale "consuma" 4 indirizzi per
  due host utili. Con tante dorsali questo conta.
- **Scalabilità limitata.** Su una mesh di 3 nodi è perfetto. Su 20+ nodi
  diventa rapidamente un foglio Excel ingestibile.

### Quando scegliere questa variante

- Reti piccole (3–5 nodi) e stabili nel tempo.
- Ambienti dove le policy aziendali o normative richiedono piano IP
  esplicito e documentato.
- Scenari didattici, dove l'obiettivo è capire come funziona il routing
  IP "sotto il cofano" prima di passare a soluzioni dinamiche.
- Casi in cui non si vuole installare software aggiuntivo sui router (es.
  hardware con flash limitata, ambienti certificati che vietano binari
  non strettamente necessari).

Per reti più grandi, dinamiche o con esigenze di ridondanza, conviene la
variante con Babel + dorsali link-local (vedi documento separato).

## Riepilogo

> In questa variante tutto è esplicito: ogni dorsale tra due router ha una
> propria subnet `/30` numerata, ogni rotta non direttamente connessa è
> scritta a mano, e non gira alcun protocollo di routing. La rete funziona
> in modo prevedibile e completamente documentabile su carta, al prezzo di
> dover riconfigurare manualmente ogni nodo coinvolto quando la topologia
> cambia.
