# RAID e ZFS — Protezione dei Dischi nel Nodo

> Piano dei Dati · Approfondimento di [continuità di servizio](../continuita_di_servizio.md)

## Il problema che risolve

Il guasto di un disco singolo è uno dei guasti **più frequenti** nel ciclo di vita dello storage. RAID e ZFS lo assorbono in modo trasparente: il servizio continua mentre il disco guasto viene sostituito e ricostruito. Operano *all'interno di un singolo nodo* — questo è insieme la loro forza (efficienza, semplicità) e il loro limite (non coprono il guasto dell'intero server).

## RAID: i livelli essenziali

**RAID (Redundant Array of Independent Disks)** combina più dischi fisici in un'unica unità logica con ridondanza.

| Livello | Come funziona | Tollera | Efficienza spazio |
|---|---|---|---|
| **RAID 1** | mirroring: ogni dato su 2 dischi | 1 disco | 50% |
| **RAID 5** | striping + 1 disco di parità | 1 disco | (n-1)/n |
| **RAID 6** | striping + 2 dischi di parità | 2 dischi | (n-2)/n |
| **RAID 10** | mirror di stripe | 1 per coppia | 50% |

Il RAID hardware (controller dedicato) o software (mdadm su Linux) protegge dalla perdita di dischi, ma il RAID tradizionale ha un punto debole noto: il **write hole** (incoerenza se l'alimentazione cade durante una scrittura di stripe+parità) e non rileva la *bit rot* (corruzione silenziosa dei dati).

## ZFS: filesystem e volume manager insieme

**ZFS** (oggi **OpenZFS**) unisce filesystem, volume manager e RAID in un unico sistema progettato attorno all'**integrità dei dati**. Caratteristiche distintive:

- **Copy-on-write (CoW):** i dati non vengono mai sovrascritti in-place; le modifiche scrivono nuovi blocchi e poi aggiornano i puntatori atomicamente. Elimina il write hole e rende gli snapshot quasi gratuiti.
- **Checksum end-to-end:** ogni blocco ha un checksum verificato a ogni lettura. ZFS rileva e (con ridondanza) **corregge automaticamente** la bit rot — cosa che il RAID classico non fa.
- **Scrub:** verifica periodica in background di tutti i checksum, per individuare e riparare la corruzione prima che si propaghi.
- **Snapshot e clone:** istantanee point-in-time a costo quasi nullo grazie al CoW, utili per rollback rapidi.
- **Compressione (LZ4):** quasi gratuita in CPU, spesso aumenta anche le prestazioni riducendo l'I/O fisico.
- **ARC (Adaptive Replacement Cache):** cache di lettura in RAM molto efficace.

### RAIDZ

L'equivalente ZFS del RAID a parità:

- **RAIDZ1** ≈ RAID 5 (1 disco di parità, tollera 1 guasto).
- **RAIDZ2** ≈ RAID 6 (2 dischi di parità, tollera 2 guasti).
- **RAIDZ3** (3 dischi di parità, tollera 3 guasti).

Con RAIDZ2 su 8 dischi, ~75% della capacità raw è disponibile per i dati: efficienza alta rispetto alla replica a 3 copie di Ceph (~33%). RAIDZ non soffre del write hole grazie al CoW.

## Il limite strutturale

RAID e ZFS proteggono dalla perdita di **dischi all'interno dello stesso server**. **Non proteggono dal guasto dell'intero nodo fisico** (alimentatore, scheda madre, CPU): se il server muore, i dati su quel server sono inaccessibili finché non viene riparato. Per sopravvivere al guasto del server serve replica *tra* nodi → [DRBD](drbd.md) (2 nodi) o [Ceph](hci_ceph.md) (cluster).

## ZFS e Ceph: ruoli complementari

Non sono in competizione, rispondono a domande diverse:

| | **ZFS / RAIDZ** | **Ceph** |
|---|---|---|
| Dominio | nodo singolo | cluster di nodi |
| Protegge da | perdita di dischi | guasto di un intero server |
| Efficienza spazio | alta (RAIDZ2 ~75%) | bassa (3 copie ~33%) |
| Integrità | checksum + scrub | scrubbing distribuito |

In Proxmox si combinano: ZFS locale per prestazioni e snapshot a caldo, Ceph per la HA cross-nodo.

## Modalità di guasto e insidie

- **Il RAID/ZFS non è un backup.** Protegge dal guasto hardware del disco, non da cancellazioni accidentali, ransomware o corruzione logica. Lo snapshot ZFS aiuta sul rollback locale ma resta sullo stesso storage. Serve sempre un [backup separato](backup_321_worm.md).
- **ZFS e RAM.** ZFS ama la RAM (ARC) e, per integrità massima, l'ECC. Su sistemi con poca memoria va dimensionato con attenzione.
- **Ricostruzione (resilver).** Su dischi molto capienti, la ricostruzione dopo un guasto può durare ore/giorni; durante questa finestra un secondo guasto (in RAIDZ1/RAID5) causa perdita totale → per array grandi si preferisce RAIDZ2/RAID6.

## Quando usarlo

- Base di partenza per qualsiasi storage serio: praticamente ogni NAS e ogni nodo di virtualizzazione usa RAID o ZFS.
- ZFS quando l'integrità dei dati e gli snapshot sono prioritari (TrueNAS, Proxmox con storage locale).

## Configurazione di esempio (didattica)

Creazione di un pool ZFS **RAIDZ2** (tollera 2 dischi guasti) su 6 dischi, con compressione e snapshot per il rollback rapido.

```
# Crea il pool 'tank' in RAIDZ2 con 6 dischi
zpool create tank raidz2 /dev/sdb /dev/sdc /dev/sdd /dev/sde /dev/sdf /dev/sdg

# Opzioni consigliate
zfs set compression=lz4 tank   # compressione quasi gratuita (spesso migliora l'I/O)
zfs set atime=off tank         # niente aggiornamento orario di accesso → meno I/O

# Crea un dataset per i dati
zfs create tank/dati

# --- Snapshot a caldo prima di un'operazione rischiosa ---
zfs snapshot tank/dati@pre-update   # istantanea immediata, costo quasi nullo

#   ...se l'aggiornamento va male:
zfs rollback tank/dati@pre-update   # torna allo stato pre-operazione in secondi

# Stato del pool e verifica integrità periodica (rileva/corregge bit rot)
zpool status
zpool scrub tank
```

Equivalente con RAID software classico (mdadm), senza checksum né snapshot nativi:

```
mdadm --create /dev/md0 --level=6 --raid-devices=6 /dev/sd[b-g]
mkfs.ext4 /dev/md0
```

La differenza pratica: ZFS rileva la corruzione silenziosa (checksum su ogni blocco) e offre snapshot atomici; mdadm protegge solo dalla perdita di dischi.

> Promemoria: né ZFS né RAID sono un backup. Proteggono dal guasto dei dischi *nel nodo*, non dalla cancellazione, dal ransomware o dal guasto dell'intero server. Servono sempre [replica tra nodi](hci_ceph.md) e [backup separato](backup_321_worm.md).

## Approfondimento

- **OpenZFS — documentazione ufficiale:** <https://openzfs.github.io/openzfs-docs/>
- **OpenZFS — RAIDZ (concetti base):** <https://openzfs.github.io/openzfs-docs/Basic%20Concepts/RAIDZ.html>
- **Linux mdadm (RAID software):** <https://raid.wiki.kernel.org/index.php/Linux_Raid>
