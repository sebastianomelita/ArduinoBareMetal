>[Torna a reti WiFi Mesh](../archmesh.md#esempio-1)


# Esempio — Mesh WiFi routed con dorsali IPv6 link-local

> Variante L3 dell'esempio classico "3 router WiFi mesh con backhaul dedicato e
> due nodi di aggregazione sensori". Le **subnet di accesso ai client restano
> IPv4 statiche**, esattamente come nello schema originale, mentre **le dorsali
> tra i router sono unnumbered** (solo IPv6 link-local automatico). Il routing
> è interamente dinamico e gestito da **Babel**, che trasporta le rotte IPv4
> usando i link-local IPv6 come next-hop (RFC 8950).

## Scenario

Tre router WiFi mesh:

- **R1** — gateway verso Internet, nessun client direttamente collegato.
- **R2** — nodo di aggregazione, ospita la subnet sensori `192.168.2.0/24`.
- **R3** — nodo di aggregazione, ospita la subnet sensori `192.168.3.0/24`.

Ogni router ha due interfacce radio: una dedicata al **backhaul** (dorsale verso
gli altri router) su canale isolato, e una dedicata all'**accesso client**
(sensori).

R1 è collegato a R2 e R3 con due link mesh wireless, ciascuno su un canale
dedicato (come da specifica originale "ogni router ha una dorsale con canale
di comunicazione dedicato").

## Schema della rete

```
                       ┌──────────────────┐
                       │   Internet       │
                       └────────┬─────────┘
                                │ WAN (DHCP)
                       ┌────────┴─────────┐
                       │       R1         │
                       │   gateway        │
                       │ lo: 10.255.0.1   │
                       └───┬──────────┬───┘
                           │          │
              dorsale ch.A │          │ dorsale ch.B
              fe80::/10    │          │ fe80::/10
              (unnumbered) │          │ (unnumbered)
                           │          │
                    ┌──────┴───┐  ┌───┴──────┐
                    │    R2    │  │    R3    │
                    │lo:10.255 │  │lo:10.255 │
                    │   .0.2   │  │   .0.3   │
                    └────┬─────┘  └────┬─────┘
                         │             │
              SSID Sensors_R2   SSID Sensors_R3
              192.168.2.0/24    192.168.3.0/24
              gw .254           gw .254
                  │                 │
              ┌───┴───┐         ┌───┴───┐
              │sensori│         │sensori│
              │  S2   │         │  S3   │
              └───────┘         └───────┘
```

**Cosa cambia rispetto allo schema "tutto IPv4 statico":**

- Non esiste più la subnet di dorsale `192.168.1.0/24` (con R1=.1, R2=.2,
  R3=.3). Le dorsali non hanno **nessun** indirizzo IPv4.
- Ogni nodo ha invece un **loopback `/32`** che funge da identità (router-id) e
  da punto di management raggiungibile via routing.
- Non si configurano rotte statiche: Babel scopre i vicini sulle dorsali via
  link-local e propaga automaticamente le subnet di accesso e il loopback.

## Piano di indirizzamento

### Loopback (identità dei nodi)

| Nodo | Loopback IPv4 |
|------|---------------|
| R1   | `10.255.0.1/32` |
| R2   | `10.255.0.2/32` |
| R3   | `10.255.0.3/32` |

Pool riservato: `10.255.0.0/24` (un `/32` per nodo, non è una subnet di rete
condivisa). Usato per SSH/management e come router-id Babel.

### Subnet di accesso (IPv4 statiche, invariate)

| Subnet | Rete | Gateway | SSID | DHCP |
|--------|------|---------|------|------|
| S2 (sensori R2) | `192.168.2.0/24` | `192.168.2.254` (R2) | `Sensors_R2` | `.1`–`.253` |
| S3 (sensori R3) | `192.168.3.0/24` | `192.168.3.254` (R3) | `Sensors_R3` | `.1`–`.253` |

### Dorsali

| Link | IPv4 | IPv6 | SSID mesh |
|------|------|------|-----------|
| R1 ↔ R2 | nessuno | `fe80::/10` (auto) | `Mesh_Backhaul_A` |
| R1 ↔ R3 | nessuno | `fe80::/10` (auto) | `Mesh_Backhaul_B` |

## Configurazione (OpenWrt)

I file mostrati sono semplificati e contengono solo le parti rilevanti per il
routing mesh; tralascio firewall, system, ecc. che sono standard.

### R1 — gateway

**`/etc/config/network`**

```
config interface 'loopback'
    option device 'lo'
    option proto 'static'
    option ipaddr '127.0.0.1'
    option netmask '255.0.0.0'

# Identità del nodo (router-id Babel, management)
config interface 'routerid'
    option device 'lo'
    option proto 'static'
    option ipaddr '10.255.0.1'
    option netmask '255.255.255.255'

# Uplink Internet
config interface 'wan'
    option device 'eth0'
    option proto 'dhcp'

# Dorsale verso R2 — nessun IPv4, solo link-local IPv6 automatico
config interface 'backhaul_r2'
    option device 'mesh0'
    option proto 'none'
    option ipv6 '1'

# Dorsale verso R3 — nessun IPv4, solo link-local IPv6 automatico
config interface 'backhaul_r3'
    option device 'mesh1'
    option proto 'none'
    option ipv6 '1'
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

**`/etc/config/babeld`**

```
config general
    option router_id '10.255.0.1'
    option ipv6_subtrees 'true'
    option local_server '/var/run/babeld.sock'

# Interfacce su cui parlare Babel
config interface
    option ifname 'mesh0'
    option type 'wireless'

config interface
    option ifname 'mesh1'
    option type 'wireless'

# Esporta le rotte connesse (loopback)
config filter
    option type 'redistribute'
    option local 'true'
    option proto '4'
    option action 'redistribute'

# Esporta la default route verso Internet
config filter
    option type 'redistribute'
    option ip '0.0.0.0/0'
    option proto 'static'
    option action 'redistribute'
```

R1 NAT-ta il traffico in uscita sull'interfaccia `wan` (configurazione firewall
standard di OpenWrt, zona `wan` con `masq '1'`).

### R2 — nodo di aggregazione sensori `192.168.2.0/24`

**`/etc/config/network`**

```
config interface 'loopback'
    option device 'lo'
    option proto 'static'
    option ipaddr '127.0.0.1'
    option netmask '255.0.0.0'

config interface 'routerid'
    option device 'lo'
    option proto 'static'
    option ipaddr '10.255.0.2'
    option netmask '255.255.255.255'

# Subnet di accesso sensori — IPv4 statico come da specifica originale
config interface 'lan'
    option device 'br-lan'
    option proto 'static'
    option ipaddr '192.168.2.254'
    option netmask '255.255.255.0'

# Dorsale verso R1 — nessun IPv4
config interface 'backhaul_r1'
    option device 'mesh0'
    option proto 'none'
    option ipv6 '1'
```

**`/etc/config/wireless`** (estratto)

```
# Backhaul verso R1 (stesso mesh_id e chiave di R1)
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

**`/etc/config/babeld`**

```
config general
    option router_id '10.255.0.2'
    option ipv6_subtrees 'true'

config interface
    option ifname 'mesh0'
    option type 'wireless'

# Ridistribuisci le rotte connesse: loopback (10.255.0.2/32)
# E subnet di accesso (192.168.2.0/24)
config filter
    option type 'redistribute'
    option local 'true'
    option proto '4'
    option action 'redistribute'
```

### R3 — nodo di aggregazione sensori `192.168.3.0/24`

Configurazione identica a R2 a meno degli indirizzi:

| Parametro | Valore |
|-----------|--------|
| `routerid` (lo) | `10.255.0.3/32` |
| `lan` ipaddr | `192.168.3.254/24` |
| SSID accesso | `Sensors_R3` |
| `mesh_id` backhaul | `Mesh_Backhaul_B` |
| Chiave backhaul | `CHIAVE_BACKHAUL_B` |
| `router_id` Babel | `10.255.0.3` |

## Cosa succede a regime

All'accensione, in pochi secondi:

1. Le interfacce mesh ottengono automaticamente un IPv6 link-local
   (`fe80::...`). Nessuna configurazione IPv4 sui link di dorsale.
2. Babel manda pacchetti hello sulle interfacce di backhaul e scopre i vicini
   identificandoli tramite il loro link-local.
3. R2 annuncia a R1: *"sono `10.255.0.2`; dietro di me c'è `192.168.2.0/24`;
   raggiungimi via `fe80::aabb:cc...%mesh0`"*.
4. R3 fa lo stesso per `192.168.3.0/24`.
5. R1 annuncia agli altri la default `0.0.0.0/0 → me`.
6. Le tabelle di routing vengono popolate automaticamente.

**Tabella di routing risultante su R1** (estratto):

```
default          via inet6 ...                  dev eth0   proto static
10.255.0.2       via inet6 fe80::aabb:cc01%mesh0 dev mesh0 proto babel
10.255.0.3       via inet6 fe80::aabb:cc02%mesh1 dev mesh1 proto babel
192.168.2.0/24   via inet6 fe80::aabb:cc01%mesh0 dev mesh0 proto babel
192.168.3.0/24   via inet6 fe80::aabb:cc02%mesh1 dev mesh1 proto babel
```

**Su R2:**

```
default          via inet6 fe80::aabb:cc00%mesh0 dev mesh0 proto babel
10.255.0.1       via inet6 fe80::aabb:cc00%mesh0 dev mesh0 proto babel
10.255.0.3       via inet6 fe80::aabb:cc00%mesh0 dev mesh0 proto babel
192.168.2.0/24   dev br-lan                    proto kernel scope link
192.168.3.0/24   via inet6 fe80::aabb:cc00%mesh0 dev mesh0 proto babel
```

Nessuna delle rotte è stata configurata a mano.

## Confronto con la versione "tutto IPv4 statico"

| Aspetto | Versione originale | Versione link-local + Babel |
|---------|---------------------|------------------------------|
| Subnet di dorsale | `192.168.1.0/24` (3 IP usati + .0/.255) | **Eliminata** |
| Subnet di accesso S2, S3 | Statiche, una per nodo | Statiche, una per nodo (invariate) |
| Rotte statiche da configurare | 3 (`R1→S2`, `R1→S3`, `R2→S3`, `R3→S2`) | 0 |
| Aggiunta di un nodo R4 | Riconfigurare R1, R2, R3 a mano | Accendi R4: Babel lo integra da solo |
| Failover su link alternativo | Non disponibile | Automatico se aggiungi link ridondanti |
| IP "consumati" dalle dorsali | 1 subnet `/24` per ogni dominio di broadcast | 0 |
| Compatibile con ACL per subnet | Sì | Sì (le subnet S2, S3 sono identiche) |

## Vantaggi pratici nello scenario sensori

Per il caso d'uso "sensori sparsi sul territorio agricolo":

- **Scalabilità senza configurazione.** Aggiungere R4 con la subnet
  `192.168.4.0/24` per una nuova zona richiede solo di accendere il nodo: la
  rete si autoconfigura.
- **Ridondanza opzionale gratuita.** Se domani aggiungi un link diretto R2↔R3
  (sempre solo link-local, nessun IP da pianificare), Babel lo userà come
  percorso alternativo in caso R1 abbia problemi o un canale congestionato.
- **Sicurezza identica all'originale.** Le subnet IPv4 dei gruppi sensori
  restano statiche e separate, quindi le ACL del tipo *"abilita traffico da
  `192.168.2.0/24` verso il broker MQTT"* continuano a funzionare senza
  modifiche.
- **Management semplice.** Per amministrare un qualunque nodo basta fare SSH
  al suo loopback (`ssh root@10.255.0.2`), raggiungibile da ovunque nella
  mesh perché annunciato da Babel.

## Note operative

- **Loopback come source per ICMP.** Per traceroute leggibili dall'esterno,
  conviene impostare il kernel in modo che i router rispondano con l'IP di
  loopback invece che con il link-local dell'interfaccia di ingresso.
  Su OpenWrt si può forzare via `sysctl` o via configurazione dell'interfaccia.
- **MTU sui link mesh.** I link 802.11s riducono leggermente l'MTU utile per
  via dell'overhead del frame mesh. Babel ne tiene conto automaticamente nei
  costi di routing.
- **Protocollo alternativo a Babel.** OLSRv2 e BIRD/BGP unnumbered offrono
  funzioni simili. Babel è scelto qui perché ha gestione nativa di link
  wireless variabili, supporto diretto a IPv4-over-IPv6-nexthop e
  configurazione minima.
- **No IPv6 ai client (per ora).** In questa configurazione IPv6 è usato
  **solo** sulle dorsali per il trasporto del routing. I sensori usano IPv4
  come da progetto originale. Se in futuro si volesse offrire IPv6 anche ai
  client basta aggiungere prefix delegation e RA sulle interfacce LAN, senza
  toccare le dorsali.

## Riepilogo

> Le dorsali della mesh non hanno alcun subnetting IPv4: usano esclusivamente
> IPv6 link-local, che è generato automaticamente da ogni interfaccia. Babel
> sfrutta questi indirizzi come next-hop per propagare le rotte IPv4 delle
> subnet di accesso e dei loopback. Il risultato è una mesh in cui le subnet
> client restano pianificate staticamente (per esigenze di segmentazione e
> sicurezza), ma l'intera infrastruttura di dorsale è zero-touch.
