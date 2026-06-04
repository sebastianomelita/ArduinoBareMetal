# VMware Fault Tolerance — Continuità Senza Interruzione

> Piano dei Dati (caso speciale) · Approfondimento di [continuità di servizio](continuita_di_servizio.md)

## Il problema che risolve

Tutte le altre tecnologie di HA richiedono, al guasto, un **riavvio** della VM (vSphere HA, [DRBD](drbd.md), [HCI](hci_ceph.md)) o almeno una breve finestra di indisponibilità. Per i servizi dove anche un secondo di downtime — o una singola transazione persa — è inaccettabile, serve qualcosa di più: una continuità *senza riavvio e senza perdita di stato*. Questo è VMware Fault Tolerance.

## Cos'è VMware FT

**vSphere Fault Tolerance (FT)** mantiene una **copia secondaria della VM in esecuzione in parallelo** su un host fisico diverso. La VM protetta è la *Primary*; la copia è la *Secondary*. La Secondary esegue le stesse identiche istruzioni della Primary e ne rispecchia lo stato completo — **memoria, registri CPU, stato dei dispositivi, disco** — istante per istante.

È il caso speciale che **integra entrambi i piani** (servizio e dati) in un'unica soluzione: non solo i dati su disco, ma l'intero stato di esecuzione della VM.

## Come funziona

1. La Primary e la Secondary partono dallo stesso stato e restano sincronizzate continuamente attraverso una **rete FT dedicata** (logging network).
2. Le due VM si **monitorano a vicenda** con heartbeat continui.
3. Se l'host della Primary si guasta, la Secondary **subentra immediatamente e in modo trasparente**: diventa la nuova Primary, e il sistema genera automaticamente una nuova Secondary su un altro host per ripristinare la protezione.
4. I ruoli sono dinamici: dopo un failover, primary e secondary si scambiano.

Il risultato: **failover senza riavvio**, connessioni TCP attive mantenute, **RPO e MTTR entrambi prossimi a zero**, zero pacchetti persi. È l'unica tecnologia che garantisce packet loss nullo anche durante un guasto hardware.

## Anti split-brain

FT evita lo split-brain (due copie attive dopo un recovery) usando il **locking atomico di file su storage condiviso**: solo un lato continua come Primary, e una nuova Secondary viene rigenerata automaticamente. Questo coordinamento impedisce che, dopo un guasto di rete, entrambe le copie si credano "quella buona".

## Requisiti e vincoli

- **Due host fisici separati:** la Primary e la Secondary non possono mai girare sullo stesso host (altrimenti il guasto dell'host perderebbe entrambe).
- **Rete FT dedicata a bassa latenza:** raccomandata almeno **10 GbE**, separata dal traffico vMotion e applicativo. Il traffico di sincronizzazione (logging) può raggiungere diversi Gbps per VM ad alta intensità: se condivide il fabric col traffico applicativo, lo riduce. Va isolato.
- **CPU compatibili con vMotion** e con la virtualizzazione MMU hardware (Intel EPT / AMD RVI), con frequenze simili tra i due host (frequenze divergenti causano restart frequenti della Secondary).
- **Limiti di vCPU:** dipendono dal licensing vSphere (es. fino a 2 vCPU con Standard/Enterprise, 4+ con Enterprise Plus, a seconda della versione). Esistono limiti per host sul numero di VM FT e di vCPU aggregate.
- **Storage condiviso** accessibile dai due host per i file di configurazione.

## Il costo

- **Banda:** la sincronizzazione continua di memoria e CPU consuma banda proporzionale alla variazione dello stato. Per VM molto attive può essere significativa — da qui l'obbligo della rete dedicata.
- **Risorse doppie:** la Secondary consuma CPU, RAM e storage propri (copie complete dei VMDK).
- **Licenza** e complessità di gestione.

Per questo VMware FT è **l'eccezione, non la regola**: si riserva alle VM dove il valore di un secondo di downtime giustifica il raddoppio delle risorse.

## On-Demand Fault Tolerance

Un uso elegante: una VM protetta normalmente solo da vSphere HA, sulla quale si **attiva FT temporaneamente** durante periodi critici (es. l'esecuzione di un report di fine trimestre che non deve essere interrotto), per poi disattivarlo dopo. Protezione massima solo quando serve.

## Confronto con le alternative

| | **VMware FT** | **DRBD + VRRP** | **HCI / Ceph** |
|---|---|---|---|
| Sincronizza | disco + RAM + CPU | solo disco | solo disco (storage) |
| Failover | senza riavvio | con riavvio VM | con riavvio VM |
| RPO | ~0 | ~0 (disco) | ~0 (storage) |
| Packet loss al guasto | nullo | breve finestra | breve finestra |
| Costo | molto alto | basso | medio |

## Quando usarlo

- Servizi mission-critical dove la perdita di una singola transazione o frazione di secondo è inaccettabile: **trading**, **sistemi di controllo industriale**, **pagamenti real-time**, applicazioni con connessioni client persistenti che non devono cadere.

## Quando NON usarlo

- Quando un riavvio di pochi secondi è tollerabile → [HCI](hci_ceph.md) o [DRBD](drbd.md) costano molto meno.
- Su link WAN ad alta latenza (la sincronizzazione di ogni istruzione diventa proibitiva).

## Configurazione di esempio (didattica)

VMware FT si attiva soprattutto dall'interfaccia grafica, ma richiede prima una preparazione di rete. Procedura tipica.

**1) Prerequisito — rete FT dedicata (su ogni host):** creare un adattatore VMkernel marcato per il traffico *Fault Tolerance logging*, idealmente su una scheda fisica separata da ≥ 10 GbE.

```
vSphere Client → Host → Configure → VMkernel adapters → Add Networking
  → seleziona "Fault Tolerance logging" tra i servizi abilitati
  → assegna una vmnic dedicata (es. vmnic2, 10 GbE)
```

**2) Attivare FT su una VM:**

```
vSphere Client → click destro sulla VM "VM-Critica"
  → Fault Tolerance → Turn On Fault Tolerance
  → scegli il datastore per i file della VM Secondary
  → scegli un host DIVERSO su cui collocare la Secondary
```

Da quel momento la Secondary gira in parallelo su un altro host e rispecchia ogni istruzione della Primary. Se l'host della Primary cade, la Secondary subentra senza riavvio e senza perdere connessioni.

**Equivalente in PowerCLI** (per automazione):

```powershell
# Attiva FT sulla VM indicata
$vm = Get-VM "VM-Critica"
$vm.ExtensionData.CreateSecondaryVM_Task($null)
```

> Semplificazione didattica: in produzione si verificano compatibilità CPU/vMotion tra gli host, frequenze simili (per evitare restart della Secondary), licensing vSphere per il numero di vCPU, e si tiene la rete FT logging separata da vMotion e dal traffico applicativo. On-Demand FT: si può attivare solo durante finestre critiche e disattivare dopo.

## Approfondimento

- **How Fault Tolerance Works (Broadcom TechDocs):** <https://techdocs.broadcom.com/us/en/vmware-cis/vsphere/vsphere/8-0/vsphere-availability/providing-fault-tolerance-for-virtual-machines/how-fault-tolerance-works.html>
- **Fault Tolerance — requisiti e limiti:** <https://techdocs.broadcom.com/us/en/vmware-cis/vsphere/vsphere/8-0/vsphere-availability/providing-fault-tolerance-for-virtual-machines/fault-tolerance-requirements-limits-and-licensing.html>
