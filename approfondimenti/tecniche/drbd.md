# DRBD — Replica dei Dati a Livello di Blocco

> Piano dei Dati · Approfondimento di [continuità di servizio](../continuita_di_servizio.md)

## Il problema che risolve

Avere il servizio sempre raggiungibile (piano del servizio) non serve a nulla se il nodo che subentra non ha i dati aggiornati. DRBD risolve questo per il piano dei dati: mantiene una **copia sincronizzata di un disco** su un secondo nodo, in tempo reale, a livello di blocco.

## Cos'è DRBD

**DRBD (Distributed Replicated Block Device)** è un modulo del kernel Linux che intercetta le scritture su un dispositivo a blocchi e le replica via rete su un altro nodo. Si descrive comunemente come un **"RAID-1 di rete"** in architettura *shared-nothing*: ogni nodo ha il proprio disco fisico, e DRBD li tiene speculari. Le letture sono locali; le scritture vengono mirrorate sul peer remoto.

<img src="../img/drbd-manuale.svg" alt="DRBD senza failover" width="700">

#### A.1 Funzionamento normale
Primario attivo (FS montato + servizio), secondario in standby (FS non montato).
Replica sincrona `protocol C`: la write ritorna OK al client solo dopo l'ACK del secondario.
```
node# drbdadm status r0
node# cat /proc/drbd
```

Importante: DRBD **non è un filesystem distribuito**. Replica blocchi, non file. Presenta a ogni nodo un dispositivo a blocchi virtuale (`/dev/drbdX`) sopra cui si monta un normale filesystem.

## I protocolli di replica (A, B, C)

Il livello di garanzia dipende dal protocollo scelto:

| Protocollo | Quando la scrittura è "completata" | RPO | Uso tipico |
|---|---|---|---|
| **A (asincrono)** | appena scritta localmente e inviata in coda TCP | dati in volo persi a un crash | WAN, link a latenza alta |
| **B (semi-sincrono)** | quando il peer ha *ricevuto* il dato | piccola finestra | compromesso |
| **C (sincrono)** | quando il peer ha *scritto su disco* e confermato | ~0 sul disco | LAN, HA — il più usato |

Con il **Protocol C**, ogni scrittura è confermata dal nodo secondario prima di essere considerata completa: l'RPO sui dati disco è prossimo a zero. Il prezzo è che il throughput di scrittura è limitato dalla latenza del link di replica — ottimo su LAN, problematico su WAN.

## Il limite rispetto a VMware FT

DRBD replica **solo i dati su disco**. Lo stato di memoria e dei registri CPU della VM **non** è sincronizzato. Conseguenza: al failover, la VM va **riavviata** sul nodo secondario (decine di secondi), perché riparte da uno stato di disco coerente ma "freddo". È la differenza fondamentale con [VMware FT](vmware_ft.md), che sincronizza anche RAM e CPU e non richiede riavvio.

## Split-brain: il pericolo principale

Lo **split-brain** è la situazione in cui, per un guasto temporaneo di *tutti* i link di rete tra i nodi, **entrambi** passano al ruolo *primary* mentre disconnessi. Risultato: due insiemi di dati divergenti, modificati indipendentemente, che non si possono fondere banalmente. È il modo in cui un sistema di replica può *distruggere* dati invece di proteggerli.

DRBD distingue lo *split-brain DRBD* (descritto sopra) dalla *cluster partition* (perdita di tutta la connettività gestita dal cluster manager).

### Come si previene

- **Quorum.** Da DRBD 9, il quorum richiede almeno **3 nodi** (il terzo può essere un *arbitratore diskless*, senza dati): un nodo accetta scritture solo se è in maggioranza. Evita che una minoranza isolata continui a scrivere.
- **Fencing / STONITH.** "Shoot The Other Node In The Head": prima di promuoversi, un nodo si assicura che l'altro sia davvero spento (via IPMI, PDU gestita, watchdog). Integrazione con cluster manager come **Pacemaker**.
- **Politiche di recovery automatico.** DRBD può risolvere automaticamente lo split-brain scartando le modifiche del primario "più giovane" o "più vecchio" — ma attenzione: *automatizzare la risoluzione significa configurare una potenziale perdita di dati automatica*. La documentazione ufficiale raccomanda di risolverlo manualmente e di eliminarne la causa, salvo casi in cui la perdita controllata sia accettabile.

## Dual-primary

DRBD supporta una modalità *dual-primary* (entrambi i nodi in scrittura simultanea), ma richiede Protocol C e un filesystem cluster-aware (es. OCFS2, GFS2) che gestisca il locking distribuito. È sensibile alla latenza e inadatto alle WAN. Nella maggior parte degli scenari HA si usa **single-primary** (un solo nodo scrive, l'altro è replica passiva).

<img src="../img/drbd-failover.svg" alt="DRBD con failover automatico" width="700">

## Quando usarlo

- HA a basso costo per database, sistemi di messaggistica, VM, dove serve RPO quasi-zero sui dati disco ma non si vuole il costo di un cluster HCI completo.
- Tipicamente in coppia con [keepalived/VRRP](vrrp_keepalived.md) per il piano del servizio e con un cluster manager (Pacemaker) per orchestrare il failover in sicurezza.

## Quando NON usarlo

- Quando serve continuità *senza riavvio* della VM → [VMware FT](vmware_ft.md).
- Quando si vuole ridondanza distribuita su molti nodi → [HCI/Ceph](hci_ceph.md).
- Su WAN ad alta latenza in modalità sincrona (degrada pesantemente il throughput di scrittura).

## Configurazione di esempio (didattica)

Una risorsa DRBD `r0` replicata in modo sincrono (Protocol C) tra due nodi, sul disco `/dev/sdb1`. File `/etc/drbd.d/r0.res` (identico sui due nodi).

```
resource r0 {
    protocol C;                  # sincrono: il peer conferma la scrittura su disco
    on nodo1 {
        device    /dev/drbd0;    # il device virtuale che useremo
        disk      /dev/sdb1;     # il disco fisico sottostante
        address   10.0.2.1:7788; # IP del nodo1 sulla rete di replica dedicata
        meta-disk internal;
    }
    on nodo2 {
        device    /dev/drbd0;
        disk      /dev/sdb1;
        address   10.0.2.2:7788; # IP del nodo2
        meta-disk internal;
    }
}
```

Comandi di inizializzazione (la prima volta):

```
drbdadm create-md r0          # crea i metadati DRBD — su ENTRAMBI i nodi
drbdadm up r0                 # attiva la risorsa — su ENTRAMBI i nodi
drbdadm primary --force r0    # promuove il PRIMO nodo a primario (solo qui, una volta)
mkfs.ext4 /dev/drbd0          # filesystem SOPRA il device DRBD (non su /dev/sdb1!)
mount /dev/drbd0 /mnt/dati    # si monta /dev/drbd0, non il disco fisico
```

Al failover, sul nodo secondario: `drbdadm primary r0 && mount /dev/drbd0 /mnt/dati`. Da qui il riavvio del servizio (la VM/applicazione riparte da uno stato disco coerente ma "freddo").

> Semplificazione didattica: in produzione NON si gestisce il failover a mano — si delega a un cluster manager ([Pacemaker](drbd.md)) con fencing/STONITH, e con 3 nodi si abilita il quorum per prevenire lo split-brain.

## Approfondimento

- **DRBD 9 — User's Guide ufficiale (LINBIT):** <https://linbit.com/drbd-user-guide/drbd-guide-9_0-en/>
- **Documentazione LINBIT (indice):** <https://docs.linbit.com/>
