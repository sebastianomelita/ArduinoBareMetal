# Approfondimento: NAS centralizzato vs distribuito

[← Torna alla dispensa principale](../dispensa_replica_storage_HA.md#3-nas-centralizzato-vs-nas-distribuito)

---

## NAS centralizzato

Un singolo server espone storage via rete. I protocolli più comuni sono NFS (Network File System, tipico in ambienti Linux/Unix), SMB/CIFS (tipico in ambienti Windows), e iSCSI (che espone blocchi raw via rete, come un disco virtuale).

### Come funziona

Il NAS è una macchina dedicata con molti dischi in RAID. I client (VM, server, hypervisor) montano i volumi via rete e li usano come se fossero dischi locali. L'intelligenza del RAID, degli snapshot e della gestione dei permessi sta tutta nel NAS.

<img src="../img/nas_confronto.svg" alt="NAS centralizzato vs distribuito" width="70%">

### Vantaggi

- **Semplicità:** un solo dispositivo da gestire, interfaccia web, configurazione guidata
- **Costo basso:** un Synology o QNAP entry-level costa poche centinaia di euro
- **Affidabilità del singolo dispositivo:** RAID interno protegge dai guasti disco
- **Ideale per:** PMI, homelab, ambienti con budget limitato, storage condiviso per piccoli cluster

### Limiti

- **Single point of failure:** se il NAS cade, tutti i client perdono accesso
- **Scalabilità verticale:** per più spazio o più prestazioni, devi comprare un NAS più grande o aggiungere shelf di espansione — c'è un tetto fisico
- **Banda condivisa:** tutti i client passano per la stessa interfaccia di rete del NAS

### Esempi di prodotto

- **Synology** (serie DS per piccoli, RS per rack)
- **QNAP** (serie TS, TVS)
- **TrueNAS** (software open source su hardware generico)
- **FreeNAS** (predecessore di TrueNAS)

---

## NAS distribuito

Lo storage è distribuito su più nodi. Non esiste un singolo server che tiene tutti i dati — ogni nodo contribuisce con i suoi dischi e i dati sono replicati automaticamente su più nodi.

### Come funziona

Ogni nodo ha uno o più dischi (OSD in Ceph, brick in GlusterFS). I dati vengono suddivisi e replicati tra i nodi. Un layer software coordina la distribuzione e garantisce la coerenza. Se un nodo cade, il cluster sa dove stanno le altre copie e continua a servire i dati.

### Vantaggi

- **Nessun single point of failure:** la perdita di un nodo non causa interruzione
- **Scalabilità orizzontale:** si aggiungono nodi per più spazio e più prestazioni
- **Self-healing:** il cluster rileva i guasti e ricrea automaticamente le copie mancanti
- **Ideale per:** produzione, cluster virtualizzati, ambienti che richiedono HA reale

### Limiti

- **Complessità:** installazione, configurazione e troubleshooting richiedono competenze specifiche
- **Overhead di rete:** i dati viaggiano tra i nodi, serve rete dedicata e veloce (10 GbE minimo)
- **Costo:** minimo 3 nodi con dischi SSD per prestazioni accettabili
- **Risorse:** Ceph in particolare consuma RAM significativa per gli OSD (1-2 GB per TB di storage)

### Tecnologie principali

| Tecnologia | Tipo | Note |
|-----------|------|------|
| **Ceph** | Blocchi + filesystem + oggetti | Il più diffuso, integrato in Proxmox e OpenStack |
| **GlusterFS** | Filesystem distribuito | Più semplice di Ceph, ma meno funzionalità |
| **MinIO** | Object storage (S3) | Solo oggetti, non blocchi né filesystem |

---

## Confronto dettagliato

| Aspetto | NAS centralizzato | NAS distribuito |
|---------|-------------------|-----------------|
| Nodi necessari | 1 | Minimo 3 |
| Single point of failure | **SÌ** (il NAS) | **NO** |
| Scalabilità | Verticale (più dischi) | Orizzontale (più nodi) |
| Complessità | Bassa | Alta |
| Costo iniziale | Basso | Alto |
| Prestazioni | Limitate dalla singola macchina | Crescono con i nodi |
| Rete richiesta | 1 GbE sufficiente | 10 GbE consigliato |
| Self-healing | NO | SÌ (automatico) |
| Live migration VM | SÌ | SÌ |
| HA automatico VM | SÌ (ma NAS è SPOF) | SÌ (nessun SPOF) |

---

## Schema decisionale

```
          Ho bisogno di storage condiviso?
                      │
                ┌─────┴─────┐
                NO          SÌ
                │            │
          ZFS locale     Quanti nodi ho?
          basta              │
                       ┌─────┴─────┐
                       1-2        3+
                       │            │
                  NAS centrale   Posso gestire
                  (Synology,     la complessità?
                   TrueNAS)          │
                               ┌─────┴─────┐
                               NO          SÌ
                               │            │
                          NAS centrale   NAS distribuito
                          + backup       (Ceph, Gluster)
```

[← Torna alla dispensa principale](../dispensa_replica_storage_HA.md#3-nas-centralizzato-vs-nas-distribuito)
