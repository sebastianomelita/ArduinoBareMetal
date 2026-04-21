# Approfondimento: Ceph — architettura, replica e quorum

[← Torna alla dispensa principale](../dispensa_replica_storage_HA.md#4-ceph-architettura-e-quando-usare-cephfs)

---

## Cos'è Ceph

Ceph è una piattaforma di storage distribuito open source. Distribuisce i dati su un cluster di nodi (minimo 3), li replica automaticamente, e si auto-ripara in caso di guasto. È integrato nativamente in Proxmox, OpenStack e Kubernetes.

---

## Architettura interna

Ceph è composto da quattro tipi di demoni che collaborano:

```
┌──────────────────────────────────────────────────┐
│                  Cluster Ceph                    │
│                                                  │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐           │
│  │   MON   │  │   MON   │  │   MON   │  Monitor  │
│  │ (nodo1) │  │ (nodo2) │  │ (nodo3) │  (quorum) │
│  └─────────┘  └─────────┘  └─────────┘           │
│                                                  │
│  ┌─────────┐  ┌─────────┐  ┌─────────┐           │
│  │   MDS   │  │   MDS   │  │         │  Metadata │
│  │ (attivo)│  │(standby)│  │         │  (CephFS) │
│  └─────────┘  └─────────┘  │         │           │
│                             │         │          │
│  ┌───┐┌───┐  ┌───┐┌───┐  ┌───┐┌───┐  │           │
│  │OSD││OSD│  │OSD││OSD│  │OSD││OSD│  │  Storage  │
│  │sda││sdb│  │sda││sdb│  │sda││sdb│  │  (dischi) │
│  └───┘└───┘  └───┘└───┘  └───┘└───┘  │           │
│   Nodo 1      Nodo 2      Nodo 3     │           │
└──────────────────────────────────────────────────┘
```

### OSD (Object Storage Daemon)

Ogni disco fisico del cluster ha un demone OSD. L'OSD gestisce la lettura e scrittura dei dati sul suo disco e comunica con gli altri OSD per la replica. Un nodo con 4 dischi ha 4 OSD.

### MON (Monitor)

I Monitor mantengono la mappa del cluster: quali OSD sono attivi, dove stanno i dati, qual è lo stato di salute. Servono in numero dispari (3 o 5) per garantire il quorum.

### MDS (Metadata Server)

Serve solo per CephFS. Gestisce il namespace del filesystem (directory, permessi, lock). Non serve per RBD né per RGW.

### MGR (Manager)

Gestisce il monitoraggio, le dashboard, e i moduli aggiuntivi. Non è coinvolto nel flusso dei dati.

---

## Come funziona la replica

### RADOS e i Placement Group

Ceph non replica file interi — lavora a livello di **oggetti**. Ogni file, blocco di VM, o dato viene spezzato in oggetti (tipicamente 4 MB ciascuno). Ogni oggetto viene assegnato a un **Placement Group (PG)**, e ogni PG è replicato su più OSD.

```
File upload.jpg (12 MB)
        │
        ▼
┌───────┬───────┬───────┐
│ Obj 1 │ Obj 2 │ Obj 3 │  (4 MB ciascuno)
│       │       │       │
│ PG 5  │ PG 12 │ PG 5  │  (assegnazione a Placement Group)
└───┬───┘───┬───┘───┬───┘
    │       │       │
    ▼       ▼       ▼
 OSD.2   OSD.5   OSD.2   ← primario
 OSD.4   OSD.1   OSD.4   ← replica 1
 OSD.6   OSD.3   OSD.6   ← replica 2
```

### La CRUSH map

Ceph non usa una tabella centralizzata per sapere dove stanno i dati. Usa un algoritmo deterministico chiamato **CRUSH** (Controlled Replication Under Scalable Hashing) che, dato l'identificatore di un oggetto, calcola su quali OSD deve stare — senza consultare nessuno.

Questo significa che qualsiasi nodo del cluster può calcolare autonomamente dove stanno i dati, senza un server centrale. È ciò che rende Ceph scalabile: aggiungere nodi non crea colli di bottiglia.

La CRUSH map definisce anche le **regole di posizionamento**: ad esempio, "le repliche di un oggetto devono stare su nodi fisici diversi" o "su rack diversi" o "in datacenter diversi". Questo garantisce che un guasto di un nodo non perda tutte le copie.

### Il processo di scrittura

Quando un client scrive un dato:

1. Il client calcola (con CRUSH) quale OSD è il **primario** per quell'oggetto
2. Invia il dato all'OSD primario
3. L'OSD primario scrive il dato sul suo disco
4. L'OSD primario inoltra il dato agli OSD **secondari** (repliche)
5. Gli OSD secondari scrivono e confermano al primario
6. Il primario conferma al client solo quando **tutte le repliche** hanno scritto

Questo è replica **sincrona**: il client riceve la conferma solo dopo che il dato è stato scritto su tutti gli OSD previsti. RPO = 0.

```
Client
  │
  │ 1. Scrivi oggetto X
  ▼
OSD Primario (es. OSD.2)
  │
  ├── 2. Scrive localmente
  │
  ├── 3. Inoltra a OSD.4 (replica 1) ──► scrive, conferma
  │
  ├── 4. Inoltra a OSD.6 (replica 2) ──► scrive, conferma
  │
  │ 5. Tutte le repliche hanno confermato
  │
  ▼
Client riceve OK
```

### Self-healing

Se un OSD cade (guasto disco o guasto nodo), il cluster rileva la perdita e **ricrea automaticamente** le copie mancanti sugli OSD rimanenti. Questo processo si chiama **recovery** e avviene in background senza intervento umano.

Esempio: se l'OSD.4 cade, tutti gli oggetti che avevano una replica su OSD.4 vengono ricopiati su un altro OSD sano. Quando la recovery è completa, il cluster torna al livello di replica previsto.

---

## Come funziona il quorum dei Monitor

I Monitor (MON) mantengono la "verità" del cluster: la mappa degli OSD, lo stato del cluster, la configurazione. Per evitare split-brain, i Monitor usano un protocollo di consenso (Paxos): una modifica alla mappa è accettata solo se la **maggioranza** dei Monitor è d'accordo.

| Monitor | Quorum | Tolleranza |
|---------|--------|------------|
| 3 | 2 | 1 monitor può cadere |
| 5 | 3 | 2 monitor possono cadere |

Se il quorum è perso (es. 2 monitor su 3 cadono), il cluster si blocca in sola lettura per prevenire corruzione. Non accetta nuove scritture finché il quorum non è ripristinato.

---

## Le tre interfacce di Ceph

<img src="../img/ceph_stack.svg" alt="Le tre interfacce di Ceph" width="70%">

### RBD (RADOS Block Device)

Un disco virtuale di rete. Si comporta come un disco fisico ma risiede sul cluster Ceph.

- Viene assegnato a **una sola VM alla volta** (accesso esclusivo)
- La VM lo vede come `/dev/vda`
- Proxmox lo usa per i disk image delle VM
- Supporta snapshot, clone, e resize a caldo

**Caso d'uso:** disco delle VM, volume per database, PersistentVolume in Kubernetes (ReadWriteOnce).

### CephFS

Un filesystem POSIX condiviso, montabile contemporaneamente da più VM.

- Più VM montano lo stesso path e vedono gli stessi file
- L'MDS gestisce il namespace, i lock, e la coerenza
- Supporta snapshot a livello di directory

**Caso d'uso:** file upload condivisi tra VM web, home directory in ambienti HPC, configurazioni condivise, PersistentVolume in Kubernetes (ReadWriteMany).

### RGW (RADOS Gateway)

Un object store compatibile con Amazon S3.

- Accesso via HTTP REST, non via mount
- Gestione di bucket e oggetti
- Supporta versionamento, lifecycle policy, multipart upload

**Caso d'uso:** backup, media storage, log aggregati, artefatti CI/CD.

### Confronto

| Interfaccia | Tipo | Accesso | Uso tipico | Analogo |
|-------------|------|---------|------------|---------|
| **RBD** | Blocchi | Una VM sola (esclusivo) | Disco VM, database | Volume EBS (AWS) |
| **CephFS** | Filesystem | Più VM insieme (condiviso) | File upload, config, asset | NFS, GlusterFS |
| **RGW** | Oggetti (S3) | Via HTTP REST | Backup, media, log | Amazon S3, MinIO |

---

## CephFS vs GlusterFS

| Aspetto | CephFS | GlusterFS |
|---------|--------|-----------|
| Architettura | Object store sotto (RADOS) | Brick su filesystem locali |
| Metadati | Server dedicato (MDS) | Distribuiti (no MDS) |
| Integrazione Proxmox | Nativa | Standalone |
| Altre interfacce nello stesso cluster | RBD + RGW | Solo filesystem |
| Complessità | Alta | Media |
| Meglio per | Ambienti già Ceph / Proxmox | Cluster dedicati solo a file |

---

## Schema decisionale: CephFS vs NFS

```
             Decidi tra CephFS e NFS
                      │
         Hai già un cluster Ceph?
                      │
                ┌─────┴─────┐
                NO          SÌ
                │            │
       Quanti nodi hai?   Usa CephFS
                │         (è già lì)
          ┌─────┴─────┐
          1-2        3+
          │            │
     NAS centrale   Vuoi eliminare
     (NFS/Synology) il single point
                    of failure?
                       │
                 ┌─────┴─────┐
                 NO          SÌ
                 │            │
            NAS + backup   Installa Ceph
            è sufficiente  e usa CephFS
```

---

## Ripristino automatico con Ceph RBD e Proxmox HA

Quando lo storage è su Ceph RBD e le VM sono configurate come risorse HA in Proxmox, il ripristino dopo un guasto avviene senza intervento umano. La sequenza è gestita da tre componenti in cascata:

1. **Corosync** rileva che il nodo non risponde al heartbeat (~30 secondi)
2. **Fencing** forza lo spegnimento del nodo guasto via watchdog/IPMI (~30 secondi)
3. **HA Manager** sceglie un nodo sano e riavvia le VM (~60 secondi)

<img src="migrazione_automatica_ceph.svg" alt="Ripristino automatico con Ceph RBD" width="70%">

Il disco Ceph RBD è già accessibile da tutti i nodi — non c'è nulla da copiare o ripristinare. L'HA Manager deve solo dire a un nodo sano "avvia questa VM".

Tempo totale: **~2 minuti**. RPO = 0 (replica sincrona Ceph). RTO ≈ 2 minuti.

[← Torna alla dispensa principale](../dispensa_replica_storage_HA.md#4-ceph-architettura-e-quando-usare-cephfs)
