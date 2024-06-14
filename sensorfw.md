>[Torna a reti ethernet](archeth.md)

- [Dettaglio architettura Zigbee](archzigbee.md)
- [Dettaglio architettura BLE](archble.md)
- [Dettaglio architettura WiFi infrastruttura](archwifi.md)
- [Dettaglio architettura WiFi mesh](archmesh.md) 
- [Dettaglio architettura LoraWAN](lorawanclasses.md) 

# Fasi Principali del Firmware di un Sensore che Utilizza MQTT

1. **Inizializzazione dei Parametri di Connessione**
   - Configurare i parametri del broker MQTT (indirizzo, porta, username, password).
   - Configurare il pin del sensore di temperatura e l'intervallo di lettura.

2. **Connessione al Broker MQTT**
   - Stabilire la connessione con il broker MQTT utilizzando i parametri configurati.

3. **Inizializzazione del Sensore di Temperatura**
   - Configurare il pin del sensore per la lettura della temperatura.

4. **Ciclo Principale**
   - Ottenere il tempo corrente.
   - Leggere il valore della temperatura dal sensore.
   - Creare un messaggio con il valore della temperatura.
   - Inviare il messaggio al broker MQTT se è trascorso l'intervallo prefissato.
   - Aggiornare il timestamp dell'ultima lettura inviata.

5. **Attesa Prima della Prossima Iterazione**
   - Attendere un breve periodo (ad esempio, 1 secondo) prima di ripetere il ciclo.


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

