# Backup, Regola 3-2-1, Snapshot e WORM — Il Piano del Ripristino

> Piano del Ripristino · Approfondimento di [continuità di servizio](continuita_di_servizio.md)

## Il problema che risolve

Tutte le tecnologie di replica ([DRBD](drbd.md), [HCI](hci_ceph.md), [VMware FT](vmware_ft.md)) propagano fedelmente *qualsiasi* stato del sistema. Se un ransomware cifra i dati, o un bug scrive dati errati, o un amministratore cancella per sbaglio una tabella, la replica copia diligentemente lo stato corrotto su tutti i nodi. Contro la **corruzione logica**, l'unica difesa è un backup che conservi una copia di uno stato verificato come sano. È il terzo piano, irrinunciabile.

## La regola 3-2-1

Il principio base di ogni strategia di backup resiliente:

- **3 copie** dei dati (l'originale + 2 backup).
- **2 supporti** di tipo diverso (così un guasto di un'intera tecnologia non le elimina tutte: es. disco interno + NAS, o NAS + cloud).
- **1 copia fuori sede** (offsite), fisicamente separata, per sopravvivere a incendio, alluvione o furto dell'intera sede.

Esempio concreto (scenario tipico di una PMI/studio professionale):

- **Copia 1:** dati live sul server.
- **Copia 2:** backup notturno o orario su NAS locale (Synology/QNAP). RPO = intervallo di backup.
- **Copia 3:** replica giornaliera/settimanale su cloud (S3, Backblaze B2, Glacier).

Esistono evoluzioni moderne (**3-2-1-1-0**): aggiungono "1" copia *immutabile/offline* e "0" errori verificati nei ripristini.

## RPO: dipende dalla frequenza

Il **Recovery Point Objective** (massima perdita di dati accettabile) è determinato dalla granularità del backup:

| Strategia | RPO tipico | Cosa si perde a un guasto |
|---|---|---|
| Replica sincrona (HCI/DRBD/FT) | &lt; 1 secondo | quasi nulla — ma NON copre la corruzione logica |
| Snapshot orario | ~59 minuti | il lavoro dell'ultima ora |
| Backup notturno | ~24 ore | il lavoro della giornata |
| Backup incrementale orario | ~59 minuti | con tempo di backup ridotto (solo i cambiamenti) |

Aumentare la frequenza riduce l'RPO ma aumenta il carico sul sistema e l'occupazione di spazio.

## Backup completo, incrementale, differenziale

- **Completo:** copia tutto ogni volta. Semplice da ripristinare, costoso in spazio e tempo.
- **Incrementale:** copia solo ciò che è cambiato dall'*ultimo backup* (di qualsiasi tipo). Veloce e leggero; il ripristino richiede la catena completa.
- **Differenziale:** copia ciò che è cambiato dall'*ultimo completo*. Compromesso: ripristino più semplice dell'incrementale, più spazio.

## Snapshot a caldo

Lo **snapshot a caldo** cattura lo stato del sistema (disco e, in alcune implementazioni, memoria) **senza spegnere** la macchina, in modo atomico. RPO = il momento esatto dello snapshot.

Uso ideale: **prima di un'operazione rischiosa** (aggiornamento, modifica di configurazione). Se qualcosa va storto, il *rollback* riporta il sistema allo stato pre-operazione in pochi secondi, invece di passare ore a diagnosticare conflitti.

**Attenzione — snapshot ≠ backup:**

- Lo snapshot resta sullo **stesso storage** della VM: non protegge da un guasto hardware che distrugga quel disco.
- Gli snapshot accumulati degradano le prestazioni (catene di delta) e occupano spazio crescente: non vanno usati come archivio a lungo termine.
- È uno strumento di *rollback rapido*, complementare al backup vero e proprio su storage separato.

## WORM e immutabilità: la difesa anti-ransomware

Il ransomware moderno cerca attivamente di cifrare anche i backup raggiungibili. La contromisura è rendere la copia **immutabile**:

- **WORM (Write Once, Read Many):** i dati, una volta scritti, non possono essere modificati né cancellati per un periodo definito.
- **Object Lock** (S3, MinIO) e snapshot immutabili dei NAS implementano questo a livello di storage.
- Una copia **offline** (air-gapped) o offsite immutabile è l'ultima linea di difesa: anche se l'attaccante compromette l'infrastruttura live e i backup raggiungibili, non può toccare la copia bloccata.

Punto chiave: **RPO quasi-zero con replica e protezione anti-corruzione con backup immutabile sono complementari, non alternativi.** Servono entrambi.

## Crittografia e conformità

Per dati personali (GDPR art. 32) il backup va protetto con:

- **Crittografia a riposo** (AES-256) su server, NAS e tutte le copie: un disco rubato resta illeggibile.
- **Crittografia in transito** (TLS) anche dentro la LAN.
- **Conservazione a norma:** alcuni documenti (es. atti notarili, cartelle cliniche) hanno obblighi di conservazione pluriennale; lo storage immutabile garantisce che non vengano cancellati prima del termine.

## La regola d'oro: testare i ripristini

Un backup mai testato è una scommessa, non una garanzia. Lo "0" del modello 3-2-1-1-0 ricorda di **verificare periodicamente che i ripristini funzionino davvero** — backup corrotti o incompleti si scoprono troppo spesso solo durante l'emergenza reale.

## Quando basta solo il backup (senza replica)

Per servizi non mission-critical (scuola, PA piccola, studio professionale) con RTO accettabile di ore e RPO di ore/giorni, un backup ben fatto + UPS è la scelta proporzionata: la complessità (e il costo) di un cluster HCI non sarebbe giustificata. Il rischio residuo si accetta consapevolmente.

## Configurazione di esempio (didattica)

La regola 3-2-1 messa in pratica su Proxmox: Copia 1 = VM live; Copia 2 = backup notturno su NAS; Copia 3 = replica offsite cifrata su cloud immutabile.

```
# --- COPIA 2: backup notturno della VM 100 sul NAS (cron, ogni notte alle 02:00) ---
0 2 * * * vzdump 100 --storage nas-backup --mode snapshot --compress zstd

# --- COPIA 3: replica offsite cifrata su S3 con restic ---
restic -r s3:s3.amazonaws.com/mio-bucket init        # solo la prima volta
restic -r s3:s3.amazonaws.com/mio-bucket backup /mnt/nas-backup
restic -r s3:s3.amazonaws.com/mio-bucket forget --keep-daily 7 --keep-weekly 4
```

L'**immutabilità** (difesa anti-ransomware) si attiva lato bucket, non lato client — così nemmeno chi compromette il server può cancellare le copie:

```
# Sul bucket S3: Object Lock in modalità compliance, retention 30 giorni
aws s3api put-object-lock-configuration --bucket mio-bucket \
  --object-lock-configuration '{"ObjectLockEnabled":"Enabled",
    "Rule":{"DefaultRetention":{"Mode":"COMPLIANCE","Days":30}}}'
```

Rotazione di snapshot ZFS come ulteriore livello di rollback rapido locale:

```
zfs snapshot tank/dati@$(date +%Y%m%d-%H%M)   # snapshot orario via cron
# (uno script tiene gli ultimi N e cancella i più vecchi)
```

> Promemoria finale: lo snapshot e la replica NON sostituiscono la copia offsite immutabile. E un backup mai testato non è una garanzia — pianificare ripristini di prova periodici (lo "0" del modello 3-2-1-1-0).

## Approfondimento

- **Regola 3-2-1 (spiegazione):** <https://www.backblaze.com/blog/the-3-2-1-backup-strategy/>
- **CISA — #StopRansomware (immutabilità e difesa):** <https://www.cisa.gov/stopransomware>
- **Proxmox Backup Server — documentazione:** <https://pbs.proxmox.com/docs/>
