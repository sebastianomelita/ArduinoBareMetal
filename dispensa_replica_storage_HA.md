# Replica, Storage Distribuito e Alta Disponibilità

## Dispensa per il quinto anno di informatica

---

## Indice

1. Tipi di replica del disco
2. Tipi di replica del servizio
3. Quando serve anche la replica del disco
4. NAS centralizzato vs distribuito
5. CephFS: quando conviene
6. Migrazione delle VM in Proxmox
7. Livelli di HA: dalla protezione blanda alla HA reale

---

## 1. Tipi di replica del disco

La replica del disco ha lo scopo di garantire che i dati sopravvivano al guasto di uno o più dispositivi fisici. Esistono tre grandi categorie.

### 1.1 Replica locale (stesso nodo)

I dati vengono duplicati tra dischi fisici della stessa macchina. **Tecnologie:** RAID hardware, RAID software (mdadm), ZFS (mirror, RAIDZ). **Protegge da:** guasto di un disco fisico. **Non protegge da:** guasto del nodo intero (scheda madre, alimentatore, incendio).

![Replica locale](img/replica_locale.svg)

### 1.2 Replica sincrona di rete (tra nodi)

Ogni scrittura viene confermata solo quando tutti i nodi coinvolti hanno scritto il dato. Nessun dato viene perso in caso di guasto di un nodo. **Tecnologie:** DRBD (sincrono), Ceph (con replica factor ≥ 2). **Protegge da:** guasto di un nodo intero. **Costo:** latenza aggiuntiva su ogni scrittura (attende la conferma remota).

![Replica sincrona di rete](img/replica_sincrona.svg)

### 1.3 Replica asincrona di rete (tra nodi)

La scrittura viene confermata subito sul nodo primario. La copia remota avviene dopo, in background. È possibile perdere le ultime scritture in caso di guasto. **Tecnologie:** ZFS send/receive (sanoid/syncoid), DRBD asincrono, rsync periodico. **Protegge da:** guasto del nodo, con possibile perdita delle ultime modifiche. **Costo:** basso impatto sulle prestazioni, ma rischio di perdita dati (RPO > 0).

![Replica asincrona di rete](img/replica_asincrona.svg)

### 1.4 Tassonomia riassuntiva

```
               Replica del disco
               ┌──────┴──────┐
           Locale          Di rete
          (1 nodo)       (più nodi)
        ┌────┴────┐     ┌────┴────┐
      RAID     ZFS    Sincrona  Asincrona
    hardware  mirror  (DRBD,    (ZFS send,
    (mdadm)  (RAIDZ)   Ceph)     rsync)
```

---

## 2. Tipi di replica del servizio

La replica del servizio ha lo scopo di garantire la continuità dell'applicazione. Non riguarda i blocchi del disco, ma la logica applicativa.

### 2.1 Nessuna replica (single instance)

Un'unica istanza del servizio gira su una sola VM. Se la VM cade, il servizio è indisponibile fino al ripristino. **Uso:** ambienti di sviluppo, servizi non critici, homelab.

### 2.2 Active-Passive (failover)

Due istanze del servizio esistono, ma solo una è attiva. La seconda subentra automaticamente in caso di guasto. **Tecnologie:** Keepalived, Pacemaker/Corosync, Proxmox HA. **Uso:** database, servizi stateful dove una sola istanza deve scrivere.

![Active-passive failover](img/active_passive.svg)

### 2.3 Active-Active con load balancer

Più istanze servono contemporaneamente il traffico. Un bilanciatore distribuisce le richieste. **Tecnologie:** HAProxy, Nginx, Traefik. **Uso:** web app, API, microservizi. **Requisito fondamentale:** le VM applicative devono essere STATELESS. Lo stato (sessioni, dati, file) viene esternalizzato su un servizio dedicato.

![Active-active con load balancer](img/active_active.svg)

### 2.4 Cluster multi-master (database)

Più istanze del database accettano scritture contemporaneamente e si sincronizzano tra loro con un protocollo interno. **Tecnologie:** Galera Cluster, MySQL Group Replication, PostgreSQL Patroni. **Uso:** database in alta disponibilità che devono sopravvivere alla perdita di un nodo.

![Cluster multi-master Galera](img/multi_master.svg)

### 2.5 Tassonomia riassuntiva

```
                  Replica del servizio
                  ┌────────┴────────┐
              Passiva              Attiva
          (hot standby)        (tutte servono)
          ┌────┴────┐         ┌────┴────┐
     Manuale    Automatica   Con LB    Multi-master
    (restart)  (Keepalived,  (HAProxy,  (Galera,
               Pacemaker)    Nginx)     Patroni)
```

---

## 3. Quando serve ANCHE la replica del disco

Questa è la domanda chiave: in quali casi la replica del servizio richiede anche la replica sincronizzata del disco sottostante?

### 3.1 Quadro completo dei casi comuni

| Caso | Replica servizio | Replica disco sincronizzata | Motivo |
|------|------------------|-----------------------------|--------|
| Web app stateless dietro LB | Active-Active (HAProxy) | **NO** | Niente stato locale da replicare |
| Sessioni HTTPS | Active-Active | **NO** | Si usa Redis condiviso |
| File upload condivisi | Active-Active | **NO** | Si usa CephFS / NFS |
| Config condivise | Active-Active | **NO** | Si usa etcd / Consul |
| DB MySQL multi-master (Galera) | Multi-master | **NO** | Galera replica internamente via wsrep |
| DB MySQL active-passive con failover | Failover | **SÌ** | Il disco del DB deve essere accessibile dal nodo standby |
| HA VM su cluster Proxmox | Failover automatico | **SÌ** | Ceph RBD replica i blocchi su tutti i nodi |
| VM su nodo singolo con LB locale | Active-Active locale | **NO** | ZFS protegge il disco singolo, non replica tra VM |

### 3.2 La regola generale

La replica sincronizzata del disco serve quando una VM deve poter ripartire su un altro nodo fisico con lo stesso disco (Ceph RBD per HA Proxmox), oppure quando un servizio stateful in failover deve trovare i dati identici sul nodo di standby (DRBD active-passive, Ceph RBD). In tutti gli altri casi si preferisce esternalizzare lo stato (Redis, DB, S3), usare storage condiviso (CephFS, NFS) oppure usare replica applicativa (Galera, Patroni).

### 3.3 Schema decisionale

```
        L'applicazione ha stato locale?
                    │
              ┌─────┴─────┐
              NO          SÌ
              │            │
        App stateless    Posso esternalizzarlo?
        LB basta         (Redis, DB, S3, NFS)
                          │
                    ┌─────┴─────┐
                    SÌ          NO
                    │            │
              Esternalizza    Serve replica disco
              lo stato        sincrona (Ceph RBD,
                              DRBD)
```

---

## 4. NAS centralizzato vs NAS distribuito

### 4.1 NAS centralizzato

Un singolo server espone storage via rete (NFS, SMB/CIFS, iSCSI). **Vantaggi:** semplice da gestire, basso costo, ideale per PMI e homelab. **Limiti:** se il NAS cade, tutti i client perdono accesso allo storage. Scalabilità limitata a quanto entra nel singolo chassis.

### 4.2 NAS distribuito

Lo storage è distribuito su più nodi. Non esiste un singolo punto di guasto. **Tecnologie:** Ceph, GlusterFS, MinIO (per S3). **Vantaggi:** nessun single point of failure, scalabilità orizzontale, self-healing automatico. **Limiti:** complessità di gestione, overhead di rete, serve hardware e rete performanti.

![NAS centralizzato vs distribuito](img/nas_confronto.svg)

### 4.3 Confronto diretto

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
| Esempi | Synology, QNAP, TrueNAS | Ceph, GlusterFS, MinIO |

---

## 5. CephFS: quando conviene

CephFS è il filesystem condiviso di Ceph. Non è un prodotto a sé ma un'interfaccia costruita sopra un cluster Ceph esistente, accanto a RBD (blocchi) e RGW (oggetti S3).

### 5.1 Le tre interfacce di Ceph

![Le tre interfacce di Ceph](img/ceph_stack.svg)

| Interfaccia | Tipo | Accesso | Uso tipico | Analogo |
|-------------|------|---------|------------|---------|
| **RBD** | Blocchi | Una VM sola (esclusivo) | Disco VM, database | Volume EBS (AWS) |
| **CephFS** | Filesystem | Più VM insieme (condiviso) | File upload, config, asset | NFS, GlusterFS |
| **RGW** | Oggetti (S3) | Via HTTP REST | Backup, media, log | Amazon S3, MinIO |

### 5.2 Quando CephFS è la scelta giusta

**USA CephFS quando:** hai già un cluster Ceph (es. Proxmox con Ceph); più VM devono leggere/scrivere gli stessi file; serve accesso POSIX (mount come directory locale); i file sono di dimensioni medie (documenti, config, upload utenti, asset web); serve scalabilità oltre un singolo NAS; serve tolleranza al guasto senza single point.

**NON usare CephFS quando:** hai 1-2 nodi (Ceph richiede minimo 3); ti basta un NAS centralizzato (Synology, TrueNAS); le VM non devono condividere file tra loro; lo storage serve per blocchi VM (usa RBD); lo storage serve per backup/media via HTTP (usa RGW); non hai rete 10 GbE o superiore.

### 5.3 Schema decisionale CephFS vs NFS

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

### 5.4 CephFS vs GlusterFS

| Aspetto | CephFS | GlusterFS |
|---------|--------|-----------|
| Architettura | Object store sotto (RADOS) | Brick su filesystem locali |
| Metadati | Server dedicato (MDS) | Distribuiti (no MDS) |
| Integrazione Proxmox | Nativa | Standalone |
| Altre interfacce nello stesso cluster | RBD + RGW | Solo filesystem |
| Complessità | Alta | Media |
| Meglio per | Ambienti già Ceph / Proxmox | Cluster dedicati solo a file |

---

## 6. Migrazione delle VM in Proxmox

La migrazione è lo spostamento di una VM da un nodo fisico a un altro all'interno dello stesso cluster Proxmox. Il suo ruolo nell'alta disponibilità è centrale.

### 6.1 Tipi di migrazione

| Aspetto | Live (a caldo) | Offline (a freddo) |
|---------|----------------|--------------------|
| La VM resta accesa? | **SÌ** (millisecondi di interruzione) | **NO** (spenta durante lo spostamento) |
| Storage richiesto | **Deve essere condiviso** (Ceph RBD, NFS) | Può essere anche locale (ZFS, LVM) |
| Cosa si sposta | Solo la RAM (il disco è già accessibile) | RAM + disco (copia completa) |
| Tempo di migrazione | Secondi | Minuti/ore (dipende dalla dimensione) |
| Uso tipico | Manutenzione pianificata, bilanciamento carico, HA | Spostamento tra cluster, cambio storage |

### 6.2 Perché la live migration richiede storage condiviso

![Live migration: Ceph RBD vs ZFS locale](img/live_migration.svg)

### 6.3 Quando la migrazione è automatica e quando manuale

| Scenario | Tipo | Cosa succede |
|----------|------|-------------|
| Guasto improvviso di un nodo | **AUTOMATICA** | Proxmox HA Manager rileva il nodo down e riavvia la VM su un altro nodo |
| Manutenzione programmata | **MANUALE** | L'amministratore avvia la live migration dalla GUI o CLI |
| Bilanciamento carico tra nodi | **MANUALE** | L'amministratore sposta VM per distribuire il carico |
| Nodo in reboot imprevisto | **AUTOMATICA** | Proxmox HA rileva il nodo fuori dal quorum e avvia le VM altrove |

### 6.4 Prerequisiti per la migrazione automatica (HA)

Per l'HA automatico in Proxmox servono: minimo 3 nodi nel cluster per il quorum (oppure 2 nodi + 1 QDevice esterno); storage condiviso (Ceph RBD, NFS, iSCSI); VM configurata come "HA resource"; rete affidabile tra i nodi con rete dedicata per Corosync e per Ceph. Se manca anche solo uno di questi requisiti, la migrazione non sarà automatica.

### 6.5 Cosa succede quando un nodo cade

![Sequenza HA failover Proxmox](img/ha_failover.svg)

**Nota importante:** nell'HA automatico di Proxmox, le VM vengono **riavviate** (cold restart), non migrate a caldo. La live migration richiede che la VM sia accesa e funzionante, il che non è possibile se il nodo è guasto. L'utente subisce un'interruzione di servizio pari al tempo di riavvio della VM (tipicamente 1-3 minuti).

---

## 7. Livelli di HA: dalla protezione blanda alla HA reale

Non tutti i contesti richiedono (o possono permettersi) un'alta disponibilità completa. I livelli seguenti rappresentano una scala progressiva di protezione, ognuno con costi e complessità crescenti.

![Livelli di alta disponibilità](img/livelli_ha.svg)

### 7.1 Livello 0 — Backup manuale (nessuna HA)

| Aspetto | Dettaglio |
|---------|-----------|
| Infrastruttura | 1 nodo, ZFS locale, backup offsite periodico |
| Protegge da | Perdita dati (se il backup è aggiornato) |
| Non protegge da | Guasto hardware, corruzione in corso, errori recenti |
| Downtime stimato | **Ore** (restore manuale da backup) |
| Costo | Minimo |
| Uso tipico | Homelab, sviluppo, servizi non critici |

### 7.2 Livello 1 — Restart automatico locale

| Aspetto | Dettaglio |
|---------|-----------|
| Infrastruttura | 1 nodo, ZFS + snapshot automatici, watchdog, systemd restart |
| Protegge da | Crash del processo, corruzione filesystem VM, aggiornamento fallito |
| Non protegge da | Guasto hardware del nodo |
| Downtime stimato | **Minuti** (restart automatico della VM o del servizio) |
| Costo | Basso |
| Uso tipico | Piccole aziende, servizi interni a tolleranza media |

### 7.3 Livello 2 — Load balancer + VM multiple (nodo singolo)

| Aspetto | Dettaglio |
|---------|-----------|
| Infrastruttura | 1 nodo, più VM dietro HAProxy, Redis per sessioni, ZFS locale |
| Protegge da | Guasto singola VM, rolling update senza downtime applicativo |
| Non protegge da | Guasto hardware del nodo (tutte le VM cadono) |
| Downtime stimato | **Zero per guasti VM**, ore per guasti hardware |
| Costo | Medio-basso |
| Uso tipico | PMI, pre-produzione, servizi web con tolleranza al guasto HW |

### 7.4 Livello 3 — Cluster multi-nodo con HA automatico

| Aspetto | Dettaglio |
|---------|-----------|
| Infrastruttura | 3+ nodi Proxmox, Ceph RBD, HA Manager, rete 10 GbE |
| Protegge da | Guasto di un intero nodo fisico, guasto disco, guasto VM |
| Non protegge da | Guasto del sito (incendio, alluvione, blackout prolungato) |
| Downtime stimato | **1-3 minuti** (tempo di riavvio VM su altro nodo) |
| Costo | Alto (3+ server, rete dedicata, competenze) |
| Uso tipico | Produzione aziendale, e-commerce, servizi critici |

### 7.5 Livello 4 — Geo-ridondanza (multi-sito)

| Aspetto | Dettaglio |
|---------|-----------|
| Infrastruttura | 2+ siti geografici, replica asincrona tra siti, DNS failover o anycast |
| Protegge da | Disastro dell'intero sito (incendio, terremoto, blackout regionale) |
| Non protegge da | Errore applicativo globale, corruzione dati replicata |
| Downtime stimato | **Prossimo allo zero** (dipende dal tipo di replica e dal DNS TTL) |
| Costo | Molto alto (doppia infrastruttura, banda inter-sito, competenze avanzate) |
| Uso tipico | Banche, cloud provider, infrastrutture critiche nazionali |

### 7.6 Come scegliere il livello giusto

```
        Quanto costa un'ora di downtime?
                    │
              ┌─────┴─────┐
        Poco/niente      Molto
         (homelab,       (e-commerce,
          sviluppo)       produzione)
              │                │
        Livello 0-1     Quanti siti servono?
                              │
                        ┌─────┴─────┐
                     Uno solo     Più siti
                        │            │
                  Livello 3      Livello 4
                  (3 nodi +      (multi-sito +
                   Ceph RBD)      geo-replica)
```

### 7.7 Confronto riassuntivo dei livelli

| Livello | Nodi | Storage | Downtime max | Costo | Complessità |
|---------|------|---------|-------------|-------|-------------|
| 0 — Backup | 1 | ZFS locale | Ore | Minimo | Bassa |
| 1 — Restart locale | 1 | ZFS + snapshot | Minuti | Basso | Bassa |
| 2 — LB nodo singolo | 1 | ZFS locale | Zero (VM) / Ore (HW) | Medio | Media |
| 3 — Cluster HA | 3+ | Ceph RBD | 1-3 minuti | Alto | Alta |
| 4 — Geo-ridondanza | 6+ | Ceph + replica inter-sito | ~0 | Molto alto | Molto alta |

---

## Riepilogo finale

**REPLICA DISCO** protegge i DATI dal guasto hardware (ZFS, RAID, Ceph RBD, DRBD).

**REPLICA SERVIZIO** protegge il SERVIZIO dall'interruzione (LB, failover, multi-master).

Le due cose sono **indipendenti**: puoi avere replica disco senza replica servizio (un DB su ZFS mirror, single instance); puoi avere replica servizio senza replica disco (app stateless dietro LB); servono entrambe solo per HA completa (VM su Ceph RBD + HA Proxmox).

**STORAGE CONDIVISO** (CephFS, NFS) è una terza cosa: non replica né disco né servizio, ma permette a più VM di accedere agli stessi file.

**MIGRAZIONE** è il meccanismo che collega il tutto: senza storage condiviso non c'è migrazione live, senza migrazione non c'è HA automatico.

**LIVELLO DI HA** va scelto in base al costo del downtime: non tutti i servizi hanno bisogno di un cluster a 3 nodi, e non tutti i cluster a 3 nodi hanno bisogno di geo-ridondanza.

---

*Dispensa per il corso di Sistemi e Reti — 5° anno Informatica*
*Argomento: Virtualizzazione, storage distribuito e alta disponibilità*
