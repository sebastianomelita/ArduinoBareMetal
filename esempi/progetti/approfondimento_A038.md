# Approfondimenti tecnici — A038 "Sistemi e Reti"
### Allegato a [risoluzione_A038_sistemi_reti_2026.md](risoluzione_A038_sistemi_reti_2026.md)

> Dettagli e comandi a corredo della Prima/Seconda parte: routing e autenticazione Wi-Fi mesh, SSID statico/dinamico, port-forward SSH in IOS, allocazione dei canali, e due ipotesi di continuità di servizio.

---

## 1 · Routing nel Wi-Fi mesh

A differenza di Ethernet e del Wi-Fi infrastruttura, in una **mesh il routing è sempre automatico**: i nodi scoprono da soli i percorsi tra loro, e le subnet dei link si assegnano automaticamente (**LLA / SLAAC**). Sostituisce sia il routing statico sia quello dinamico configurati a mano.

Protocolli tipici (uno solo, scelto dall'apparato):

| Protocollo | Tipo | Quando conviene |
|---|---|---|
| **AODV** | reattivo (on-demand) | topologie che cambiano spesso, pochi flussi |
| **OLSR** | proattivo (tabelle sempre pronte) | molti flussi, percorsi stabili |
| **Babel** | ibrido (distance-vector loop-free) | reti miste cablato/wireless |

**Mesh routed vs bridged** — due modi di trasportare il traffico tra i nodi:
- **routed (L3):** ogni hop è un salto IP; subnet diverse per link. Più scalabile, domini di broadcast piccoli.
- **bridged (L2):** i nodi formano un unico dominio L2; la stessa subnet si estende a tutta la mesh. Più semplice per i client, ma broadcast più ampio.

Per il cantiere conviene la **mesh routed** con almeno **un percorso LOS** tra vicini e **percorsi alternativi (backup)** se cade un nodo centrale (coerente con §3.3 del cheatsheet). Va sempre indicata la **posizione del controller** degli AP.

---

## 2 · Autenticazione dei nodi Wi-Fi mesh

Due piani distinti, entrambi a **L2** (ammissione alla rete), coerenti con lo stack del documento centrale:

1. **Reciproca tra nodi (backhaul).** I nodi mesh si autenticano *l'un l'altro* prima di formare il link wireless di dorsale: **WPA3-SAE** (o chiave mesh pre-condivisa nelle implementazioni 802.11s). Impedisce a un nodo non autorizzato di unirsi alla dorsale.
2. **Nodo ↔ servizi (AP su RADIUS).** Ogni AP/nodo è un client RADIUS e si autentica al server AAA centrale (come gli switch in §19): è il presupposto per l'assegnazione dinamica della VLAN (vedi §3 sotto).

Lato **client** (operatori/tablet) resta l'**802.1X / WPA2-3-Enterprise** verso RADIUS, esattamente come sul Wi-Fi infrastruttura.

Configurazione del nodo come **client RADIUS** (impianto §19 del cheatsheet):

```cisco
! Sull'apparato (switch/WLC che gestisce gli AP)
SW(config)# aaa new-model
SW(config)# aaa authentication dot1x default group radius local
SW(config)# dot1x system-auth-control
SW(config)# radius-server host 10.0.30.10 auth-port 1812 acct-port 1813 key <chiave>
```

```ini
# FreeRADIUS — clients.conf (l'AP/nodo è un client del RADIUS)
client 10.0.99.0/24 {
    secret    = <chiave>          # IDENTICA al "key" sull'apparato
    shortname = ap-cantiere
}
```

> ⚠️ La `secret` di `clients.conf` **deve coincidere** con `key` di `radius-server host`: un mismatch fa fallire silenziosamente tutte le autenticazioni.

---

## 3 · SSID statico vs dinamico (associazione VLAN↔SSID)

| Approccio | Come funziona | Pro / Contro |
|---|---|---|
| **SSID statico** | un **SSID per VLAN** (es. `Cantiere-Tablet`→VLAN 10, `Cantiere-Sensori`→VLAN 30) | semplice; ma molti SSID "inquinano" l'etere e vanno gestiti su ogni AP |
| **SSID dinamico** | **un solo SSID**; è il RADIUS a dire al NAS in quale VLAN mettere l'utente, in base al **gruppo LDAP** | un SSID solo; policy centralizzata; aggiungere un utente = solo LDAP |

Nel caso dinamico il RADIUS restituisce nell'`Access-Accept` gli attributi RFC 2868:

```ini
# Attributi RADIUS restituiti al NAS (contenuto di file)
Tunnel-Type             = VLAN
Tunnel-Medium-Type      = IEEE-802
Tunnel-Private-Group-Id = "10"        # VLAN ID di destinazione
```

Smistamento per **gruppo LDAP** (un unico SSID, ognuno nella VLAN giusta):

```ini
# FreeRADIUS — policy per gruppo (contenuto di file)
DEFAULT Ldap-Group == "cn=tablet,ou=cantiere,dc=sito,dc=it"
        Tunnel-Type = VLAN, Tunnel-Medium-Type = IEEE-802, Tunnel-Private-Group-Id = "10"
DEFAULT Ldap-Group == "cn=sensori,ou=cantiere,dc=sito,dc=it"
        Tunnel-Type = VLAN, Tunnel-Medium-Type = IEEE-802, Tunnel-Private-Group-Id = "30"
DEFAULT Auth-Type := Reject
```

> Stesso meccanismo per confinare un profilo sospeso in una **VLAN di quarantena**. **Consiglio per il cantiere:** SSID **dinamico** se l'identità è centralizzata su AD/LDAP; SSID statici solo per reti molto piccole o dispositivi che non parlano 802.1X.

---

## 4 · Port forwarding SSH in Cisco IOS (DNAT)

Esporre l'SSH di un host interno (`10.13.0.40:22`) su una porta di facciata (`:2222`) dell'IP pubblico del router — stessa idea del Quesito IV, realizzata con un **DNAT** (impianto §8 del cheatsheet).

<img src="../img/ssh_port_forwarding.svg" alt="Schema port forwarding SSH" width="760">

| Traffico in ingresso | Protocollo | Azione | Destinazione tradotta |
|---|---|---|---|
| `200.1.1.1 : 2222` | TCP | DNAT | `10.13.0.40 : 22` |

```cisco
! — Parte comune: interfacce inside/outside
R(config)# interface GigabitEthernet0/0
R(config-if)# ip address 10.13.0.254 255.255.255.0
R(config-if)# ip nat inside
R(config-if)# exit
R(config)# interface GigabitEthernet0/1
R(config-if)# ip address 200.1.1.1 255.255.255.252
R(config-if)# ip nat outside
R(config-if)# exit

! — Port-forward (DNAT): TCP :2222 pubblico → 10.13.0.40:22 interno
! formato: ip nat inside source static tcp <inside-local> <porta> <inside-global> <porta>
R(config)# ip nat inside source static tcp 10.13.0.40 22 200.1.1.1 2222
```

ACL WAN coerente con il default-deny (§17.1) — il `permit` si scrive sull'indirizzo/porta di **facciata**, perché l'ACL inbound è valutata **prima** della traduzione:

```cisco
R(config)# ip access-list extended ACL-WAN
R(config-ext-nacl)# permit tcp any host 200.1.1.1 eq 2222   ! ← SSH esposto
R(config-ext-nacl)# deny   ip any any                        ! ← default deny
R(config-ext-nacl)# exit
R(config)# interface GigabitEthernet0/1
R(config-if)# ip access-group ACL-WAN in
```

Verifica: `R# show ip nat translations` · `R# show ip nat statistics` · `R# debug ip nat`.

> Comando utente: `ssh -p 2222 studente@200.1.1.1` → la sessione termina su `10.13.0.40:22`. Differenza con HAProxy (reverse proxy L4): il DNAT è pura riscrittura di pacchetti, niente health-check / bilanciamento / log di sessione / HA.

---

## 5 · Allocazione dei canali (Wi-Fi mesh tri-band, EU)

**Principio guida:** *vicini nello spazio → frequenze lontane; lontani → frequenze anche vicine* (la propagazione li disaccoppia). Mai **access** e **backhaul** sulla **stessa** radio: il CSMA/CA serializza e dimezza il throughput a ogni hop → da qui la scelta **tri-band**.

<img src="../img/mesh_canali.svg" alt="Allocazione canali Wi-Fi mesh tri-band" width="800">

Le tre radio dell'apparato:

| Radio | Banda | Ruolo | Per chi |
|---|---|---|---|
| R1 | 2.4 GHz | Access | client legacy / IoT |
| R2 | 5 GHz lower (36–64) | Access | client moderni (Wi-Fi 6) |
| R3 | 5 GHz upper DFS (100–144) | **Backhaul** | solo nodi mesh, 80 MHz |

Canali **access** — 4 "colori" (riuso N=4), spettralmente disgiunti:

| Colore | Canale 80 MHz | Slot 20 MHz | DFS |
|---|---|---|---|
| A | `ch 36 @ 80` | 36·40·44·48 | no |
| B | `ch 52 @ 80` | 52·56·60·64 | sì |
| C | `ch 100 @ 80` | 100·104·108·112 | sì |
| D | `ch 116 @ 80` | 116·120·124·128 | sì |

In **2.4 GHz** solo **1 · 6 · 11** non si sovrappongono → riuso a 3 colori.

Canali **backhaul** — sempre fuori dai 4 colori access:

| Canale | DFS | Perché |
|---|---|---|
| `ch 132 @ 80` | sì | lontano dall'access, stabile su tratte fisse |
| `ch 149 @ 80` | **no** | massimo EIRP outdoor (30 dBm), link lunghi |

**Backhaul ad albero — regola dell'alternanza:** le due radio mesh di uno stesso concentratore stanno su canali **opposti** (132 ↔ 149), così le trasmissioni concorrenti vanno in parallelo invece di finire in CSMA/CA sullo stesso canale:

```
        T5 (root)
     132 /     \ 149
       T2        T3        ← concentratori
   149 /  \      /  \ 132
   T1   T6    T4    T8     ← foglie (1 sola radio mesh)
```

**Procedura:** (1) disegna la griglia degli AP; (2) colora le celle access A/B/C/D, adiacenti = colori diversi; (3) riusa un colore solo tra celle ben separate; (4) 2.4 GHz con 1/6/11; (5) backhaul alterna 132/149 lungo l'albero; (6) **site survey** (RSSI/SNR) e affina. **Access in Auto-RF; backhaul a mano** (l'Auto-RF destabilizza i P2P fissi).

EIRP (EU, indicativo): 2.4 GHz 20–24 dBm · 5 GHz DFS ~23 dBm indoor / ~30 dBm outdoor · U-NII-3 (149+) massima outdoor.

---

## 6 · Continuità di servizio — link verso la VPN

**Ipotesi.** Il canale cantiere↔sede non deve interrompersi: si combinano **ridondanza del link** (lato cantiere) e **alta disponibilità del concentratore VPN** (lato sede).

<img src="../img/continuita_link.svg" alt="Continuità link/VPN con dual-WAN e VRRP" width="800">

- **Lato cantiere:** gateway **dual-SIM** (due operatori) o **5G + satellitare** in **failover automatico**; VPN con **Dead Peer Detection** e riconnessione, **due tunnel ridondati** verso la sede; **QoS** che prioritizza gli allarmi.
- **Lato sede:** **due firewall/concentratori VPN** in **VRRP** (keepalived): un **IP virtuale (VIP)** su cui terminano i tunnel; se il MASTER cade, il BACKUP prende il VIP → **failover trasparente**. In più **doppio ISP + 4G** in failover e **UPS**.

Configurazione **VRRP con keepalived** (un solo file, due valori diversi sui due nodi — §10):

```ini
# /etc/keepalived/keepalived.conf
vrrp_script chk_vpn {
    script   "pidof openvpn"      # o il processo del concentratore
    interval 2
}
vrrp_instance VI_1 {
    state            MASTER       # BACKUP sul secondo nodo
    interface        eth0
    virtual_router_id 51          # IDENTICO sui due nodi
    priority         100          # 90 sul BACKUP
    advert_int       1
    authentication { auth_type PASS; auth_pass 1234 }   # IDENTICO
    virtual_ipaddress { 10.0.40.1 }                     # VIP: i tunnel terminano qui
    track_script { chk_vpn }
}
```

> Se il processo VPN muore, `track_script` abbassa la `priority` sotto quella del BACKUP, che subentra. Verifica: `ip addr show eth0` (il VIP compare sul MASTER) · `tcpdump -i eth0 vrrp`.

---

## 7 · Continuità di servizio — NAS centrale (repository BIM)

**Ipotesi.** Il repository delle nuvole di punti/video è un asset critico: serve **replica**, **failover** e **copia off-site** (regola **3-2-1**).

<img src="../img/continuita_nas.svg" alt="Continuità NAS con DRBD e regola 3-2-1" width="800">

- **Replica a blocchi DRBD** (sincrona, `protocol C`) tra **NAS primario** e **NAS gemello**: la scrittura ritorna OK solo dopo l'ACK del secondario → nessuna perdita.
- **Failover automatico** con Pacemaker/keepalived: al guasto, promozione del secondario, mount del FS, avvio servizio, spostamento della VIP — i client BIM vedono sempre lo stesso IP.
- **Regola 3-2-1:** 3 copie, 2 supporti, 1 **off-site** (cloud), più **snapshot/versioning** sul NAS per il ripristino puntuale. Accesso ai backup riservato agli amministratori (AAA).

Risorsa **DRBD** (§12):

```ini
# /etc/drbd.d/r0.res
resource r0 {
  protocol C;                       # replica sincrona
  on nas1 { device /dev/drbd0; disk /dev/sdb1; address 10.0.30.21:7789; meta-disk internal; }
  on nas2 { device /dev/drbd0; disk /dev/sdb1; address 10.0.30.22:7789; meta-disk internal; }
}
```

```bash
# Init (su ENTRAMBI i nodi)
drbdadm create-md r0 && drbdadm up r0
# Primo sync (SOLO sul primario)
nas1# drbdadm primary --force r0
nas1# mkfs.ext4 /dev/drbd0 && mount /dev/drbd0 /mnt/repository
```

Failover automatico con il cluster manager:

```bash
nas1# pcs resource create drbd ocf:linbit:drbd drbd_resource=r0
nas1# pcs resource promotable drbd promoted-max=1
nas1# pcs resource create fs Filesystem device=/dev/drbd0 directory=/mnt/repository fstype=ext4
nas1# pcs resource create vip IPaddr2 ip=10.0.30.20 cidr_netmask=24
# ordine: promuovi drbd → monta fs → avvia servizio → alza la VIP
```

Copia **off-site** verso il cloud (incrementale, regola 3-2-1):

```bash
# cron sul NAS — ogni notte alle 02:00
0 2 * * *  rsync -avz --delete /mnt/repository/  utente@cloud_host:/backup/repository/
```

> In **restore** usare sempre `--numeric-ids` per conservare UID/GID. Provare prima con `--dry-run`.
