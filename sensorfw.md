>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 


Comandi rsync utilizzati nel CronJob:
``` C++
rsync -av --delete root@ucs1.univention.marconicloud.it:/media/nas/ /mnt/UniventionBackup/UniventionNAS/Daily
rsync -av --delete root@ucs1.univention.marconicloud.it:/media/nas/ /mnt/UniventionBackup/UniventionNAS/Weekly
rsync -av --delete root@ucs1.univention.marconicloud.it:/media/nas/ /mnt/UniventionBackup/UniventionNAS/Montly
rsync -av --delete root@ucs1.univention.marconicloud.it:/media/nas/ /mnt/UniventionBackup/UniventionNAS/Hourly
```
Si rimarca che, in questo processo di backup, l’**iniziativa** di eseguire la copia dei dati è presa dal server TrueNAS che **preleva** i dati da remoto e li conserva in locale con **politica PULL**. Ci sono 4 copie aggiornate con periodicità diversa: oraria, giornaliera, settimanale e mensile. Le copie, al momento, sono mantenute in chiaro. L’accesso alle copie è riservato ai soli sistemisti della rete previa autenticazione.

### **Backup delle VM**



>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

