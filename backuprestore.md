>[Torna a backup](backup.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

# **Restore** 

## **Script Bash di Ripristino (Pull) con NFS** 

Lo script di restore viene eseguito sul server da ripristinare.

``` C++
#!/bin/bash

# Monta la condivisione NFS sul server da ripristinare
sudo mount server_ip:/path/to/backup/folder /mnt/backup

# Esegui il ripristino con rsync
rsync -av --delete /mnt/backup/ /path/to/local/data/

# Smonta la condivisione NFS
sudo umount /mnt/backup
```
Rendere eseguibile lo script
``` C++
chmod +x /path/to/restore_script.sh
```
Per eseguire manualmente lo script di ripristino, basta eseguire sul **server da ripristinare**:
``` C++
bash /path/to/restore_script.sh
```

## **Script Bash di Ripristino (Pull) con Samba** 

Lo script di restore viene eseguito sul server da ripristinare.

``` C++
#!/bin/bash

# Monta la condivisione Samba sul server da ripristinare
sudo mount -t cifs -o username=your_username,password=your_password //server_ip/path/to/backup/folder /mnt/backup

# Esegui il ripristino con rsync
rsync -av --delete /mnt/backup/ /path/to/local/data/

# Smonta la condivisione Samba
sudo umount /mnt/backup
```
Rendere eseguibile lo script
``` C++
chmod +x /path/to/restore_script.sh
```
Per eseguire manualmente lo script di ripristino, basta eseguire sul **server da ripristinare**:
``` C++
bash /path/to/restore_script.sh
```

## **Script Bash di Ripristino (PULL) con rsync** 

Lo script di restore viene eseguito sul server da ripristinare.

Passo 1: Creare la coppia di chiavi SSH e copiare la chiave pubblica sul server NAS
``` C++
ssh-keygen -t rsa
ssh-copy-id user@nas_host
```
Passo 2: Creare lo script restore.sh eseguito dal server sorgente (server da ripristinare)
``` C++
#!/bin/bash
rsync -avz --delete user@nas_host:/path/to/backup /path/to/restore
```
Rendere eseguibile lo script
``` C++
chmod +x /path/to/restore.sh
```
Per eseguire manualmente lo script di ripristino, basta eseguire:
``` C++
bash /path/to/restore.sh
```

## **Script Bash di Ripristino (PUSH) con rsync** 

Lo script di restore viene eseguito sul NAS dove è conservato il backup.

Passo 2: Creare lo script restore.sh eseguito dal server sorgente (server da ripristinare)
``` C++
#!/bin/bash
rsync -avz --delete /path/to/source user@destination_host:/path/to/destination
```
Rendere eseguibile lo script
``` C++
chmod +x /path/to/backup.sh
```
Per eseguire manualmente lo script di ripristino, basta eseguire:
``` C++
bash /path/to/backup.sh
```

>[Torna a backup](backup.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

