# Approfondimento: metriche e grandezze per l'alta disponibilità

[← Torna alla dispensa principale](../dispensa_replica_storage_HA.md#0-glossario-grandezze-e-metriche-fondamentali)

---

## Metriche di disponibilità

### Uptime

L'uptime è il tempo in cui un servizio è operativo e raggiungibile. Si esprime come percentuale su base annua. I livelli convenzionali sono detti "nine" (dal numero di 9 dopo la virgola).

| Livello | Uptime (%) | Downtime annuo | Esempio tipico |
|---------|-----------|----------------|----------------|
| 2 nine | 99% | 3 giorni 15 ore | Homelab, dev |
| 3 nine | 99.9% | 8 ore 45 minuti | PMI, servizi interni |
| 4 nine | 99.99% | 52 minuti | E-commerce, produzione |
| 5 nine | 99.999% | 5 minuti | Banche, telco, cloud |

Ogni "nine" in più richiede un salto significativo di complessità e costo infrastrutturale. Passare da 99.9% a 99.99% significa passare da "un NAS con backup" a "cluster Ceph con HA automatico".

### Downtime

Il downtime è il complementare dell'uptime: il tempo in cui il servizio è indisponibile. Include sia i guasti imprevisti sia la manutenzione programmata. Per questo la manutenzione a caldo (live migration, rolling update) è importante: non consuma la "quota" di downtime.

---

## Metriche di ripristino

### RTO (Recovery Time Objective)

L'RTO è il tempo massimo accettabile per ripristinare il servizio dopo un guasto. È un **obiettivo** che si definisce prima del guasto e si progetta l'infrastruttura per rispettarlo.

Esempi concreti:

| RTO | Significa | Infrastruttura necessaria |
|-----|----------|--------------------------|
| 7 giorni | Posso aspettare una settimana | Backup offsite, nessuna fretta |
| 4 ore | Devo tornare online entro metà giornata | Backup su NAS + nodo slave (livello 1b/1c) |
| 30 minuti | Massimo mezz'ora di fermo | HA semi-automatico, VM su NAS |
| 2 minuti | Quasi invisibile all'utente | Cluster HA con Ceph RBD (livello 3) |
| ~0 | Zero downtime percepito | Geo-ridondanza multi-sito (livello 4) |

### RPO (Recovery Point Objective)

L'RPO è la quantità massima di dati che si accetta di perdere in caso di guasto, espressa in tempo. È la distanza temporale tra l'ultimo dato salvato e il momento del guasto.

```
        Tempo ───────────────────────────────►

        ultimo backup         guasto        servizio ripristinato
             │                   │                   │
             ├───────────────────┤                   │
             │       RPO         │                   │
             │  (dati persi)     │                   │
                                 ├───────────────────┤
                                 │       RTO         │
                                 │  (tempo di fermo) │
```

| RPO | Significa | Tecnologia necessaria |
|-----|----------|----------------------|
| 24 ore | Perdo al massimo un giorno | Backup notturno |
| 1 ora | Perdo al massimo un'ora | Backup incrementale orario (PBS) |
| 0 | Non perdo nulla | Replica sincrona (Ceph RBD, DRBD sincrono) |

### Relazione tra RPO e RTO

RPO e RTO sono indipendenti: puoi avere RPO = 0 (nessun dato perso) ma RTO = 2 ore (ci metti tempo a ripristinare), oppure RPO = 24h (perdi un giorno di dati) ma RTO = 5 minuti (ripristini velocemente da backup).

---

## Metriche di latenza e prestazioni

### Latenza (delay)

La latenza è il tempo tra una richiesta e la sua risposta. Nel contesto della replica:

- **Replica sincrona:** ogni scrittura deve attendere la conferma dal nodo remoto → la latenza di rete si aggiunge a ogni operazione. Su rete locale (< 1ms) è trascurabile. Tra datacenter distanti (10-50ms) diventa un collo di bottiglia.
- **Replica asincrona:** la scrittura è confermata subito localmente → la latenza di rete non impatta le prestazioni, ma crea un ritardo nella copia (e quindi un RPO > 0).

### Throughput (banda)

La quantità di dati trasferibili nell'unità di tempo. Determina quanto velocemente i dati possono essere replicati.

| Rete | Throughput reale | Sufficiente per |
|------|-----------------|-----------------|
| 1 GbE | ~100 MB/s | NFS per poche VM leggere, backup |
| 10 GbE | ~1 GB/s | Ceph in produzione, replica sincrona |
| 25 GbE | ~2.5 GB/s | Cluster ad alte prestazioni |

### IOPS (Input/Output Operations Per Second)

Il numero di operazioni di lettura o scrittura che lo storage gestisce al secondo. È la metrica critica per i database.

| Storage | IOPS tipici | Uso |
|---------|------------|-----|
| HDD 7200 RPM | 100-200 | Backup, archivio |
| SSD SATA | 10.000-50.000 | VM generiche, web server |
| SSD NVMe | 100.000-500.000 | Database, cache, Ceph OSD |

---

## Metriche di resilienza

### SPOF (Single Point of Failure)

Un componente il cui guasto provoca l'indisponibilità dell'intero servizio. L'obiettivo dell'alta disponibilità è eliminarli tutti. Esempi: un NAS unico, un load balancer singolo, un unico switch di rete, un unico alimentatore.

### Quorum

Il numero minimo di nodi che devono essere operativi perché il cluster possa prendere decisioni. La formula è: quorum = (N / 2) + 1, arrotondato per difetto.

| Nodi | Quorum | Nodi che possono cadere |
|------|--------|------------------------|
| 3 | 2 | 1 |
| 5 | 3 | 2 |
| 2 + QDevice | 2 | 1 (con limitazioni) |

### Split-brain

La situazione in cui due parti di un cluster, non potendo comunicare tra loro, credono entrambe di essere il cluster attivo e operano indipendentemente. Porta a corruzione dei dati perché entrambe le parti scrivono sullo stesso storage.

Il quorum previene lo split-brain: solo la partizione con la maggioranza dei nodi può restare attiva. L'altra partizione si ferma.

### Fencing

Il meccanismo con cui il cluster "isola" un nodo che non risponde, impedendogli di scrivere sui dischi condivisi. Proxmox usa il watchdog hardware o IPMI/iLO per forzare lo spegnimento del nodo guasto **prima** di riavviare le sue VM altrove. Senza fencing, il nodo "fantasma" potrebbe tornare vivo e scrivere sui dischi, causando corruzione.

---

## Relazioni tra le metriche

| Se vuoi... | Ti serve... | Che impatta su... |
|-----------|------------|-------------------|
| RPO = 0 (nessuna perdita dati) | Replica sincrona | Latenza di scrittura (+) |
| RTO basso (ripristino rapido) | HA automatico, Ceph RBD | Costo infrastruttura (+) |
| IOPS alti | SSD/NVMe, Ceph su SSD | Costo storage (+) |
| Eliminare SPOF | Minimo 3 nodi, Ceph | Complessità (+) |
| Prevenire split-brain | Quorum + fencing | Nodi minimi = 3 |

[← Torna alla dispensa principale](../dispensa_replica_storage_HA.md#0-glossario-grandezze-e-metriche-fondamentali)
