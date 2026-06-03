# Continuità di Servizio
## Guida Pratica alle Tecnologie per l'Alta Disponibilità

> Una mappa visiva per orientarsi tra tecnologie, gradi di disponibilità e tipi di guasto — senza perdersi nei parametri SLA.

---

## 1. I Tre Piani della Continuità

Prima di tutto: non esiste una tecnologia che fa tutto. La continuità di servizio risponde a **tre domande distinte**, e confonderle porta invariabilmente a scegliere la soluzione sbagliata.

![I tre piani della continuità di servizio](img/01_tre_piani.svg)

| Piano | La domanda | Cosa non fa |
|---|---|---|
| **Servizio** | Come rendo il servizio sempre *raggiungibile*? | Non protegge i dati |
| **Dati** | Come rendo i dati sempre *integri e disponibili*? | Non protegge da corruzione logica |
| **Ripristino** | Come *torno* a uno stato precedente sano? | Non è HA — agisce *dopo* il danno |

La replica sincrona (piano 2) propaga fedelmente qualsiasi stato del sistema — incluso uno stato cifrato da ransomware. Il backup (piano 3) è l'unica difesa contro la corruzione logica, e rimane necessario anche nell'architettura più sofisticata.

---

## 2. I Guasti: dalla Più Probabile alla Più Rara

Il principio guida è semplice: investire prima nella protezione dai guasti più frequenti, poi salire verso quelli più rari man mano che il costo aumenta.

![Guasti per probabilità di occorrenza](img/02_piramide_guasti.svg)

Leggendo dal basso verso l'alto:

**Crash di un processo o servizio** — il guasto più comune, spesso quotidiano. Basta un health check automatico (HAProxy, systemd watchdog) per rilevarlo e ridirigere il traffico in pochi secondi.

**Guasto di un disco singolo** — si verifica regolarmente nel ciclo di vita di ogni storage. Il RAID (o ZFS/RAIDZ) lo assorbe completamente senza interruzione del servizio.

**Guasto di un server backend** — periodico, coperto dal clustering DNAT con health check: il proxy smette di inviare traffico al nodo irraggiungibile e lo dirige verso quelli sani.

**Guasto del proxy / load balancer** — raro ma catastrofico se non coperto: è il singolo punto di guasto dell'intero piano del servizio. VRRP trasferisce il Virtual IP allo slave in 2–3 secondi.

**Guasto di un nodo hypervisor** — tutte le VM su quel nodo scompaiono contemporaneamente. HCI (Ceph/vSAN) o DRBD permettono il riavvio automatico sui nodi superstiti in pochi secondi.

**Corruzione logica / ransomware** — raro ma devastante, e invisibile alla replica. Unica difesa: backup su storage immutabile (WORM).

**Guasto dell'alimentazione / sala server** — richiede ridondanza fisica (UPS, generatori, TIA-942).

**Disastro del datacenter** — richiede un sito geograficamente separato con backup offsite.

---

## 3. Piano del Servizio: Sempre Raggiungibile

Il piano del servizio è costruito su due livelli sovrapposti, non alternativi.

![Piano del Servizio: VRRP + HAProxy + DNAT](img/03_vrrp_haproxy.svg)

### Livello 1 — Ridondanza del proxy (VRRP / keepalived)

Due proxy (HAProxy o Nginx) condividono un **Virtual IP (VIP)**: i client raggiungono sempre lo stesso indirizzo. Il protocollo VRRP, gestito da keepalived, monitora i nodi tramite heartbeat periodici. Se il master smette di rispondere, lo slave acquisisce il VIP in **2–3 secondi** — in modo completamente trasparente ai client, che non cambiano indirizzo.

VRRP non sa nulla di cosa il proxy fa al traffico: si occupa solo di garantire che ci sia sempre un proxy attivo.

### Livello 2 — Distribuzione del traffico (DNAT / ALG)

Il proxy attivo distribuisce le connessioni in ingresso ai server backend attraverso **DNAT** (riscrittura dell'indirizzo di destinazione a livello pacchetto). Tre modalità principali:

- **Clustering / Load Balancing** — le connessioni vengono distribuite su un pool di server identici, migliorando prestazioni e resilienza ai guasti singoli.
- **Alta Disponibilità** — health check ogni 500 ms; il server irraggiungibile viene escluso automaticamente dal pool.
- **ALG (Application Level Gateway)** — routing basato sul path URL o sull'hostname. Richiede SSL termination perché il proxy deve leggere il contenuto HTTP.

**SSL Termination**: il proxy apre due connessioni TCP separate — HTTPS col client, HTTP (o HTTPS separato) con i backend. Questo alleggerisce i server dal carico crittografico e permette il routing L7 su traffico cifrato.

---

## 4. Piano dei Dati: Mai Perdere un Byte

Che il servizio sia raggiungibile non implica che i dati siano al sicuro. Se il nodo che acquisisce il VIP non ha una copia aggiornata dei dati, il servizio riparte ma con un database vuoto.

![Piano dei Dati: HCI con replica distribuita e DRBD](img/04_hci_drbd.svg)

### HCI — Iperconvergenza (Proxmox + Ceph, VMware vSAN)

L'iperconvergenza porta lo storage **dentro** ogni nodo di compute. I dischi di tutti i nodi vengono visti come un unico pool distribuito, accessibile da qualsiasi VM del cluster. Ogni scrittura viene confermata su almeno N nodi fisici diversi prima di essere completata (replica sincrona).

Vantaggi chiave:
- **RPO < 1 secondo** per i dati storage
- Guasto di un nodo → VM riavviate automaticamente sui superstiti, senza trasferire dati dalla rete
- **Data locality**: la VM gira sullo stesso nodo che contiene la copia primaria dei suoi dati, eliminando la latenza SAN
- **Scaling lineare**: aggiungere un nodo porta simultaneamente più compute, più storage e più banda

### DRBD — RAID-1 di rete

DRBD (Distributed Replicated Block Device) è una soluzione più semplice ed economica: replica i blocchi di un disco su un secondo nodo remoto, in modalità **Protocol C** (sincrona — ogni scrittura confermata sul secondario prima di completarsi). RPO < 1 secondo per i dati disco. Limite: solo memoria e registri CPU non vengono replicati — al failover è necessario un breve riavvio della VM (decine di secondi). Funziona bene in combinazione con VRRP/keepalived.

### ZFS — Protezione intra-nodo

ZFS con RAIDZ2 protegge dalla perdita di fino a 2 dischi sullo **stesso server fisico**, con alta efficienza dello spazio (≈75% su 8 dischi). Aggiunge snapshot atomici, compressione LZ4 e deduplicazione. Limite strutturale: non protegge dal guasto dell'intero server.

In Proxmox, ZFS e Ceph si usano insieme: ZFS per le prestazioni di I/O locali e gli snapshot, Ceph per la ridondanza tra nodi.

### VMware Fault Tolerance — L'eccezione

VMware FT sincronizza non solo i dati disco ma **tutto lo stato della VM** — memoria, registri CPU, stato dei dispositivi. La VM slave è già in esecuzione in parallelo sul nodo secondario. Al guasto il failover avviene senza riavvio e senza interruzione. RPO e MTTR entrambi prossimi a zero. Costo: consumo di banda significativo e obbligo di host fisicamente separato.

---

## 5. Piano del Ripristino: Tornare a uno Stato Sano

Il backup non è la rete di sicurezza dell'infrastruttura poco seria — è lo strato indispensabile che completa qualsiasi architettura, per difendersi dalla corruzione logica che nessuna replica può coprire.

![Piano del Ripristino: Regola 3-2-1 e snapshot](img/05_backup_321.svg)

### La Regola 3-2-1

La struttura minima di un piano di backup resiliente:

**3 copie** dei dati — la copia live sul server, una copia locale su NAS, una copia offsite.

**2 supporti diversi** — almeno due tecnologie di storage fisicamente distinte, così un singolo tipo di guasto (es. bug firmware SAN) non cancella tutte le copie.

**1 copia fuori sede** — fisicamente separata dall'edificio principale. Protegge da incendio, alluvione e furto.

### Backup WORM / Immutabile

Per la protezione specifica contro il ransomware, la copia offsite deve essere **immutabile**: i dati non possono essere modificati né cancellati prima di un periodo definito (WORM — Write Once, Read Many). Soluzioni: storage S3 con Object Lock, MinIO in modalità immutabile, Backblaze B2 con versionamento.

### Snapshot a Caldo

Lo snapshot cattura l'intero stato del sistema — disco, memoria, configurazione — senza spegnere il server. Uso ideale: subito prima di un aggiornamento rischioso. Se qualcosa va storto, il rollback riporta il sistema allo stato pre-operazione in pochi secondi. Lo snapshot rimane sullo stesso storage (non protegge da guasti hardware) e non sostituisce il backup offsite.

### Frequenza e RPO

Il Recovery Point Objective (la massima perdita di dati accettabile) dipende direttamente dalla frequenza del backup. Esempi pratici:

| Frequenza backup | RPO massimo | Scenario tipico |
|---|---|---|
| Replica sincrona (HCI/DRBD) | < 1 secondo | E-commerce, ospedale, CRM |
| Snapshot orario | ~59 minuti | Studio professionale, PA media |
| Backup notturno | ~23 ore | Scuola, servizi interni |

---

## 6. La Scala della Disponibilità

Quanti "nove" vuole il contratto — e cosa serve per raggiungerli?

![La scala della disponibilità](img/06_scala_nines.svg)

La scala della disponibilità è utile perché mette in relazione diretta il downtime annuo tollerato con la complessità e il costo dell'infrastruttura. Non esiste una soluzione giusta in assoluto: esiste la soluzione giusta per il profilo di rischio specifico.

Da 99% a 99.9% il salto principale è l'aggiunta della virtualizzazione con HA (failover automatico in minuti). Da 99.9% a 99.99% si aggiungono VRRP, HCI e DNAT per portare il MTTR a secondi. Oltre i quattro nove si entra nel territorio di VMware FT e dei datacenter TIA-942 Tier IV, con costi proporzionalmente molto più elevati.

---

## 7. Chi Protegge da Cosa

La mappa completa: quale tecnologia risponde a quale tipo di guasto.

![Chi protegge da cosa](img/07_chi_copre_cosa.svg)

La colonna più importante è quella del ransomware/corruzione logica: tutte le tecnologie di replica (DRBD, HCI, VMware FT) mostrano **✗** perché propagano fedelmente qualsiasi stato, incluso quello corrotto. Solo backup e WORM mostrano **✓✓✓** in quella riga.

---

## 8. Come si Combinano: Architettura Completa

I tre livelli non si escludono — si sovrappongono e si completano.

![Architettura completa a tre livelli](img/08_architettura_completa.svg)

In un'architettura di produzione completa:

**Livello 1 (Piano del Servizio)** — I client arrivano sempre al Virtual IP condiviso. Il proxy attivo distribuisce le richieste ai backend sani via DNAT. Se il proxy crasha, VRRP dà il VIP allo slave in 2–3 secondi.

**Livello 2 (Piano dei Dati)** — Ogni dato scritto da una VM viene confermato su almeno 2 nodi fisici prima della risposta al client. Se un nodo cade, le sue VM vengono riavviate automaticamente sui nodi superstiti (già in possesso delle repliche) in pochi secondi, senza trasferire nulla dalla rete.

**Livello 3 (Piano del Ripristino)** — Il backup notturno su storage WORM offsite è l'unica difesa se ransomware o errore umano corrompono i dati: in quel caso la replica si trasforma da risorsa in problema, avendo propagato lo stato corrotto ovunque.

---

## 9. Guida Rapida alla Scelta

Una domanda alla volta per trovare il livello giusto.

```
Qual è il downtime annuo massimo accettabile?

   Giorni/ore  ──────────── Backup NAS + UPS + VM Proxmox
                            (scuola, PA, servizi interni)

   Ore/minuti  ──────────── Virtualizzazione con HA cluster
                            (storage condiviso SAN/NAS)

   Minuti/secondi ────────── VRRP + HCI (Ceph/vSAN) + DNAT
                             (e-commerce, ospedale, CRM)

   Secondi/zero ──────────── VMware Fault Tolerance + datacenter ridondato
                             (trading, controllo industriale, pagamenti)
```

```
Il servizio tratta dati personali o ha obblighi normativi?

   Sì (GDPR / NIS2)  ──────  Backup WORM offsite obbligatorio
                             RTO < 72 ore (obbligo notifica GDPR/NIS2)
                             Crittografia a riposo e in transito

   No  ────────────────────  Proporzionale al rischio economico
```

```
Quale guasto preoccupa di più?

   Crash processo/servizio  ──── Health check HAProxy + restart automatico
   Guasto disco             ──── RAID/ZFS (già incluso in qualsiasi NAS)
   Guasto server backend    ──── DNAT Clustering
   Guasto proxy             ──── VRRP / keepalived
   Guasto nodo hypervisor   ──── HCI (Ceph/vSAN) o DRBD
   Ransomware / corruzione  ──── Backup WORM — unica soluzione
   Disastro datacenter      ──── Sito DR geograficamente separato
```

---

## Riferimenti Tecnologici Rapidi

| Tecnologia | Cosa fa | Piano | Scope |
|---|---|---|---|
| **VRRP / keepalived** | Condivide un Virtual IP tra proxy master e slave | Servizio | Proxy HA |
| **DNAT Clustering** | Distribuisce connessioni su pool di backend; health check automatici | Servizio | Backend HA |
| **HAProxy / Nginx** | Proxy con routing L4/L7, SSL termination, health check | Servizio | Load balancer |
| **DRBD** | RAID-1 di rete tra due nodi, replica blocchi a livello kernel | Dati | 2 nodi |
| **HCI / Ceph / vSAN** | Storage distribuito su N nodi, replica sincrona, data locality | Dati | Cluster |
| **ZFS / RAIDZ** | Protezione da perdita di dischi sullo stesso server, snapshot | Dati | Nodo singolo |
| **VMware FT** | Replica completa VM (disco + memoria + CPU) su host secondario | Dati | 2 host dedicati |
| **Snapshot a caldo** | Immagine istantanea dello stato VM per rollback rapido | Ripristino | Locale |
| **Backup su NAS** | Immagini VM periodiche su storage locale separato | Ripristino | Locale |
| **Backup WORM offsite** | Copie immutabili su cloud o sito fisicamente separato | Ripristino | Offsite |

---

*Dispensa — Continuità di Servizio e Alta Disponibilità*
