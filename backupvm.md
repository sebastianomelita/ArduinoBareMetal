>[Torna a backup VM](backup.md#backup-delle-vm)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 


# **Backup delle VM**

Backup delle macchine virtuali di segreteria e didattica
In questo caso, TrueNAS non è direttamente responsabile del processo di backup ma si limita a mettere a disposizione delle condivisioni NFS (Network File System). Ogni condivisione è specifica per una certa macchina VMWARE ESXI (hypervisor) ed è accessibile utilizzando la versione NFS4 del protocollo che richiede l’autenticazione della macchina al momento del montaggio della condivisione stessa. Le macchine, al momento, si autenticano presso il nas con la password di un utente amministratore del NAS. 
I path di montaggio sono:
- /mnt/UniventionBackup/ESXI_VM_backups/ESXI _10.0.0.2
- /mnt/UniventionBackup/ESXI_VM_backups/ESXI _10.0.0.6
- /mnt/UniventionBackup/ESXI_VM_backups/ESXI _10.0.0.7
- /mnt/UniventionBackup/ESXI_VM_backups/ESXI_10.0.0.8
- /mnt/UniventionBackup/ESXI_VM_backups/ESXI _10.0.0.12

Il montaggio è protetto da una ACL impostata sul NAS che limita gli accessi ai soli client NFS con IP incluso nell’elenco degli IP delle macchine ESXI. 

Il processo di backup è sotto la responsabilità di ciascuna macchina ESXI che, individualmente, programma il proprio backup periodico tramite un servizio di cron. La forma della entry relativa nella cron table è analoga a:
``` C++
30 4 * * * /vmfs/volumes/datastore1/ghettoVCB/ghettoVCB.sh -a -g /vmfs/volumes/datastore1/ghettoVCB/ghettoVCB.conf > /vmfs/volumes/VM_backup_10.0.0.8/ghettoVCB-\$(date +\\%s).log"
```

Si comanda l’esecuzione di un certo script  30 minuti dopo le ore 4 tutti i giorni, tutte le settimane e tutti i mesi (carattere *) con destinazione la condivisione NFS VM_backup_10.0.0.8 (ESXI della segreteria in questo caso). I backup sono corredati di log memorizzato sulla cartella della condivisione. La rotazione dei backup è di 3, ovvero vengono mantenute solamente le copie degli ultimi 3 giorni. 

L’applicativo che realizza il backup è uno **script linux bash ghettoVCB.sh** che è un prodotto freeware fornito e mantenuto da VMWARE. Lo script contiene internamente i comandi della shell Vmware per eseguire snapshot a caldo delle macchine virtuali e i comandi per l’esecuzione di una copia su una cartella locale, nonché per la successiva eliminazione dello snapshot a copia eseguita. 

Lo script con l’impostazione del **parametro -a** esegue lo snapshot di tutte le macchine virtuali del sistema e di tutti i dischi montati su ciascuna VM ad eccezione di quelli aggregati in modalità indipendente persistente che non devono essere oggetto di backup con questo sistema. 

Il **parametro -g** richiede prelevare i dettagli di configurazione del backup dal file di configurazione ghettoVCB.conf comune per tutte le VM della macchina ESXI. Un prerequisito fondamentale del backup via script è che i dischi siano tutti consolidati, cioè privi di snapshot.

In  ghettoVCB.conf  è stato impostato il **flag** ALLOW_VMS_WITH_SNAPSHOTS_TO_BE_BACKEDUP=0 che imposta il consolidamento manuale di tutti i dischi prima di eseguire il loro backup. La funzione di consolidamento elimina eventuali snapshot del disco creati dal sistemista tra un backup e l’altro, creando un unico punto di ripristino a partire dallo snapshot corrente. In presenza di snapshot, una VM non può essere clonata, almeno fino a quanto questi non vengono consolidati.

Gli **snapshot** sono un altro strumento tipico degli ambienti virtualizzati che permette di effettuare **copie di sicurezza** dell’intero sistema prima di effettuare su di esso **modifiche rischiose** e potenzialmente irreversibili quali **aggiornamenti**, e installazioni di SW non affidabile.

Lo svantaggio di questa impostazione è che se è necessario fare degli snapshot prima di un aggiornamento  adesso si devono fare avendo cura di eliminarli (premendo, su ESXI, il il pulsante che fa partire la funzione che consolida il disco) se si desidera avere dei nuovi backup su quella VM.

Si rimarca che, in questo processo di backup, l’iniziativa di eseguire la copia dei dati è presa indipendentemente da **ciascun hypervisor Vmaware ESXI** che preleva i dati da locale e li conserva in remoto con **politica PUSH**. Si producono, sul NAS di backup, 3 copie aggiornate con periodicità giornaliera. Le copie, al momento, sono mantenute in chiaro. L’accesso alle copie è riservato ai soli sistemisti della rete previa autenticazione.




>[Torna a backup VM](backup.md#backup-delle-vm)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

