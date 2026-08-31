# Mosquitto – Configurazioni di riferimento (Gateway ChirpStack)

Raccolta di configurazioni pronte da conservare per il bridge MQTT del gateway.
Canali gestiti: **up** (`event/up`, in uscita) e **down** (`command/down`, in ingresso).

---

## 1. Base + listener locale

Sezione comune a tutte le configurazioni.

```conf
log_dest syslog

# Impostazioni globali
allow_anonymous true

# Listener MQTT standard (TCP)
listener 1883
```

---

## 2. Bridge interno – broker già esistente (TCP)

Bridge verso la rete privata `marconicloud`, senza restrizioni sui topic.

```conf
connection bridge-esterno

address proxy.marconicloud.it:1883

# UPLINK
topic application/# out 0
topic gateway/# out 0

# DOWNLINK
topic application/+/device/+/command/down in 0

restart_timeout 10
cleansession false
remote_clientid gateway-raspi-01
```

---

## 3. Bridge pubblico HiveMQ via TLS — CONFIGURAZIONE ATTIVA

Bridge verso `broker.hivemq.com` limitato ai soli canali up/down, con:
- **TLS** sulla porta 8883
- **namespace dedicato** (`gw-raspi-01/`) sul broker remoto
- **client id univoco**

Richiede il pacchetto di sistema `ca-certificates` (`sudo apt install ca-certificates`).
Nessun certificato client necessario: il broker pubblico non richiede mTLS.

```conf
connection bridge-pubblico

# Connessione TLS sulla porta 8883
address broker.hivemq.com:8883

# CA di sistema per verificare il certificato del broker
bridge_capath /etc/ssl/certs/
# In alternativa, file unico:
# bridge_cafile /etc/ssl/certs/ca-certificates.crt

bridge_insecure false
bridge_tls_version tlsv1.2

# Topic con namespace dedicato sul broker remoto
# Locale "application/..."  ->  remoto "gw-raspi-01/application/..."
topic application/+/device/+/event/up     out 0 "" gw-raspi-01/
topic application/+/device/+/command/down in  0 "" gw-raspi-01/

restart_timeout 10
cleansession false
remote_clientid gateway-raspi-01-pub
try_private true
notifications false
```

**Sottoscrizioni lato consumatore sul broker pubblico:**
- Uplink da leggere: `gw-raspi-01/application/+/device/+/event/up`
- Downlink da inviare: `gw-raspi-01/application/+/device/+/command/down`

---

## 4. Variante – broker con username / password

Aggiungere queste due righe al blocco `connection` (funziona sia su TCP che TLS).

```conf
remote_username NOME_UTENTE
remote_password PASSWORD
```

---

## 5. Variante – broker privato con certificato self-signed

Il certificato del broker NON è firmato da una CA pubblica, quindi va copiato
manualmente la CA (o il certificato) del broker sul Raspberry Pi, es. in
`/etc/mosquitto/certs/ca-broker.crt`.

```conf
connection bridge-privato

address mio-broker.local:8883

# CA del broker copiata a mano sul gateway
bridge_cafile /etc/mosquitto/certs/ca-broker.crt

bridge_insecure false
bridge_tls_version tlsv1.2

topic application/+/device/+/event/up     out 0 "" gw-raspi-01/
topic application/+/device/+/command/down in  0 "" gw-raspi-01/

restart_timeout 10
cleansession false
remote_clientid gateway-raspi-01-priv
```

> Nota: `bridge_insecure true` disattiva la verifica dell'hostname. Usarlo solo
> per test veloci, mai in produzione.

---

## 6. Variante – mTLS (autenticazione reciproca)

Il broker richiede anche un **certificato client** generato per il gateway.
Servono tre file sul Raspberry Pi: CA del broker, certificato client, chiave privata client.

```conf
connection bridge-mtls

address mio-broker.local:8883

# CA del broker
bridge_cafile /etc/mosquitto/certs/ca-broker.crt

# Certificato + chiave del CLIENT (il gateway)
bridge_certfile /etc/mosquitto/certs/gateway-client.crt
bridge_keyfile  /etc/mosquitto/certs/gateway-client.key

bridge_insecure false
bridge_tls_version tlsv1.2

topic application/+/device/+/event/up     out 0 "" gw-raspi-01/
topic application/+/device/+/command/down in  0 "" gw-raspi-01/

restart_timeout 10
cleansession false
remote_clientid gateway-raspi-01-mtls
```

---

## 7. Comandi utili

```bash
# Installazione CA di sistema (per TLS con broker pubblico)
sudo apt install ca-certificates

# Verifica della configurazione (modalità verbosa)
mosquitto -c /etc/mosquitto/mosquitto.conf -v

# Riavvio del servizio
sudo systemctl restart mosquitto
# (oppure)  /etc/init.d/mosquitto restart

# Stato / porte in ascolto
/etc/init.d/mosquitto status
netstat -tlnp
```

---

## Chi installa i certificati?

| Elemento                 | Certificato              | Chi lo fornisce / installa                          |
|--------------------------|--------------------------|-----------------------------------------------------|
| Broker HiveMQ (pubblico) | Certificato server       | HiveMQ — già pronto, non si tocca nulla             |
| Raspberry Pi (bridge)    | CA radice per la verifica| Tu, col pacchetto `ca-certificates`                 |
| Raspberry Pi (bridge)    | Certificato client       | Non serve col pubblico; serve solo con **mTLS**     |
| Broker privato self-signed | CA del broker          | Tu, copiando a mano la CA sul gateway               |
