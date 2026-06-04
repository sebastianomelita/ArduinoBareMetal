# HCI e Ceph — Storage Distribuito Iperconvergente

> Piano dei Dati · Approfondimento di [continuità di servizio](continuita_di_servizio.md)

## Il problema che risolve

Nel datacenter tradizionale, compute (server) e storage (SAN/NAS con cablaggio Fibre Channel dedicato) sono mondi separati: hardware distinto, competenze distinte, scaling rigido e disaccoppiato. L'**iperconvergenza (HCI)** elimina questa separazione portando lo storage *dentro* ogni nodo di compute, gestito via software sulla stessa rete Ethernet del traffico applicativo.

## Cos'è l'HCI

Una piattaforma **HCI (Hyperconverged Infrastructure)** combina in ogni nodo:

- **Hypervisor** (KVM in Proxmox, ESXi in VMware) per eseguire le VM,
- **Storage engine distribuito** che aggrega i dischi locali di tutti i nodi in un unico pool,
- **Overlay di rete** per la comunicazione del cluster.

I dischi di tutti i nodi appaiono come un singolo pool condiviso, accessibile da qualsiasi VM. Non serve cablaggio Fibre Channel dedicato.

Soluzioni principali: **VMware vSAN**, **Nutanix AHV**, **Microsoft Azure Stack HCI** (proprietarie); **Proxmox VE + Ceph** (la combinazione open source più diffusa).

## Come funziona Ceph

**Ceph** è il motore di storage distribuito open source alla base di molte installazioni HCI. Concetti fondamentali:

- **RADOS** (Reliable Autonomic Distributed Object Store): il cuore di Ceph. Memorizza tutto come oggetti distribuiti e autogestiti.
- **OSD** (Object Storage Daemon): un demone per ogni disco fisico. Gestisce i dati, la replica e il recupero. Un cluster ha decine o centinaia di OSD.
- **Monitor (MON):** mantengono la mappa dello stato del cluster (la *cluster map*) e il consenso. Tipicamente in numero dispari (3, 5) per il quorum.
- **CRUSH** (Controlled Replication Under Scalable Hashing): l'algoritmo che decide *dove* vanno gli oggetti. Calcola la posizione senza bisogno di un indice centralizzato — sia il client che gli OSD lo eseguono. Questo elimina il collo di bottiglia di un metadata server.
- **Placement Group (PG):** livello di indirezione tra oggetti e OSD. Ogni oggetto → un PG (per hashing del nome) → CRUSH mappa il PG su un *acting set* di OSD.

### Il percorso di una scrittura

1. Il client recupera la cluster map da un monitor.
2. Calcola via CRUSH il PG e l'OSD primario per l'oggetto.
3. Scrive sull'OSD primario, che replica sugli OSD secondari secondo il *replication factor* del pool.
4. La scrittura è confermata al client solo quando le copie minime (`min_size`) sono state scritte → **consistenza forte**.

### Replica vs Erasure Coding

- **Pool replicati (size=3):** 3 copie complete su OSD/nodi diversi. Sopravvive al guasto di un intero server. Efficienza spazio: solo ~33% della capacità raw è dati utili.
- **Erasure coding (K+M):** ogni oggetto in K frammenti dati + M di parità (simile a RAID-6 ma distribuito). Più efficiente in spazio, ma maggiore latenza e CPU in scrittura.

### Failure domain

CRUSH conosce la topologia fisica (disco → host → rack → datacenter) e distribuisce le copie su **failure domain** diversi. Configurato a livello "host", garantisce che le 3 copie stiano su 3 server fisici distinti: il guasto di un intero nodo non perde dati.

### Self-healing e scrubbing

Quando un OSD/nodo cade, CRUSH rimappa i PG su OSD superstiti e ricostruisce le copie mancanti automaticamente (*self-healing*). Lo **scrubbing** periodico confronta i checksum delle repliche per individuare e correggere la *bit rot* (corruzione silenziosa su disco).

## Data locality

Il software HCI preferisce eseguire una VM sullo stesso nodo che contiene la copia primaria dei suoi dati. Questo elimina la latenza di rete verso uno storage SAN remoto (3–5 ms tipici) riducendola a quella di un accesso locale (&lt; 0,1 ms con NVMe).

## Il vantaggio chiave: failover senza trasferimento dati

Quando un nodo fisico cade, l'orchestratore (Proxmox HA, vCenter) riavvia le VM sui nodi superstiti, **che hanno già le repliche**. Nessun trasferimento dati dalla rete, nessun restore da NAS. MTTR dell'ordine dei secondi.

## Scaling lineare

Aggiungere un nodo al cluster porta simultaneamente **più compute (CPU+RAM), più storage e più banda di rete** — una sola operazione. Nell'infrastruttura tradizionale sarebbero tre operazioni separate (nuovo server + nuovo array SAN + nuovi switch).

## Modalità di guasto e insidie

- **Il backup resta necessario.** La replica sincrona propaga fedelmente *qualsiasi* stato, incluso uno corrotto da ransomware o errore umano. Solo un [backup immutabile](backup_321_worm.md) conserva uno stato verificato come sano. La replica protegge dall'hardware, non dalla corruzione logica.
- **Numero minimo di nodi.** Per `size=3` servono almeno 3 nodi; per il quorum dei monitor serve un numero dispari ≥ 3. Sotto 3 nodi, Ceph perde gran parte delle sue garanzie.
- **Rete come collo di bottiglia.** Il traffico di replica viaggia sulla rete Ethernet: serve banda dedicata o QoS per non farlo competere col traffico applicativo (idealmente ≥ 10 GbE per il cluster network).
- **Complessità operativa.** Ceph è potente ma non banale: tuning dei PG, gestione del bilanciamento, monitoraggio dello stato. Sproporzionato per piccoli ambienti.

## ZFS e Ceph: ruoli complementari

In Proxmox si usano spesso insieme: **[ZFS](zfs_raid.md) locale** su ogni nodo per le prestazioni di I/O e gli snapshot a caldo; **Ceph** come pool distribuito per la HA cross-nodo e la live migration. ZFS protegge dalla perdita di dischi *nel nodo*; Ceph protegge dal guasto dell'*intero nodo*.

## Quando usarlo

- Workload mission-critical che richiedono RPO &lt; 1 s e failover in secondi: e-commerce, sistemi sanitari, CRM.
- Ambienti che crescono e vogliono scaling semplice e lineare.

## Configurazione di esempio (didattica)

Creazione di un cluster Ceph iperconvergente su Proxmox VE (3 nodi già uniti in cluster `pve`). Comandi da eseguire via CLI sui nodi.

```
# 1) Su OGNI nodo: installa i pacchetti Ceph
pveceph install

# 2) Inizializza Ceph indicando la rete DEDICATA al traffico del cluster
pveceph init --network 10.0.3.0/24

# 3) Crea un monitor su 3 nodi (numero dispari → quorum)
pveceph mon create        # da ripetere su nodo1, nodo2, nodo3

# 4) Aggiungi ogni disco fisico come OSD (ripeti per ogni disco/nodo)
pveceph osd create /dev/sdb
pveceph osd create /dev/sdc

# 5) Crea il pool per le VM: 3 copie, almeno 2 confermate per scrivere
pveceph pool create vmpool --size 3 --min_size 2
```

Cosa significano i parametri chiave:

- `--size 3` → ogni dato è scritto in **3 copie** su 3 nodi fisici diversi (failure domain = host). Sopravvive al guasto di un intero nodo.
- `--min_size 2` → una scrittura è confermata solo quando almeno **2 copie** sono su disco: garanzia di consistenza. Se restano meno di 2 nodi, il pool va in sola lettura per protezione.

Verifica stato: `ceph -s` (salute generale), `ceph osd tree` (topologia OSD/nodi). Quando un nodo cade, Ceph rimappa i dati automaticamente (self-healing) e l'orchestratore Proxmox HA riavvia le VM sui nodi superstiti.

> Semplificazione didattica: in produzione si separano rete pubblica e rete cluster Ceph (≥ 10 GbE), si dimensionano i Placement Group e si valuta l'erasure coding per i pool a freddo. Il [backup immutabile](backup_321_worm.md) resta obbligatorio.

## Approfondimento

- **Ceph — Architecture (documentazione ufficiale):** <https://docs.ceph.com/en/latest/architecture/>
- **Proxmox VE — Deploy Hyper-Converged Ceph Cluster:** <https://pve.proxmox.com/wiki/Deploy_Hyper-Converged_Ceph_Cluster>
- **VMware vSAN (Broadcom TechDocs):** <https://techdocs.broadcom.com/us/en/vmware-cis/vsan.html>
