# DNAT, Clustering e Load Balancing — Distribuzione del Traffico

> Piano del Servizio · Approfondimento di [continuità di servizio](../continuita_di_servizio.md)

## Il problema che risolve

[VRRP](vrrp_keepalived.md) garantisce che ci sia sempre un proxy vivo. Ma quel proxy deve poi *distribuire* le richieste verso più server backend, per due obiettivi distinti: **prestazioni** (nessun server singolo si satura) e **affidabilità** (se un backend cade, il traffico va sugli altri). Questo è il compito del DNAT e del load balancing.

## DNAT: il meccanismo di base

**DNAT (Destination NAT)** riscrive l'indirizzo IP (ed eventualmente la porta) di *destinazione* dei pacchetti in transito. Una connessione diretta a un indirizzo pubblico/virtuale viene reindirizzata verso uno tra più server interni. Lavorando a livello di pacchetto (L3/L4), il DNAT puro è veloce ma "cieco": non vede il contenuto applicativo del traffico.

Su questo meccanismo si costruiscono tre funzioni.

### 1. Clustering / Load Balancing

Le connessioni verso un VIP vengono distribuite su un **pool** di server che pubblicano lo stesso servizio. Algoritmi tipici:

- **Round Robin:** ogni nuova connessione al server successivo, a rotazione.
- **Least Connections:** alla macchina con meno connessioni attive — bilancia meglio sotto carico disomogeneo.
- **Source hash / sticky:** stesso client sempre allo stesso backend (utile per sessioni con stato).

È uno **scale-out orizzontale**: aggiungere un server al pool aumenta immediatamente il throughput aggregato, senza limiti teorici, a differenza dello scale-up (un server più potente).

### 2. Alta Disponibilità (HA) dei backend

Il load balancer esegue **health check** continui sui server del pool. Se un backend non risponde, viene tolto dalla rotazione: le nuove connessioni vanno solo ai server sani. Il *packet loss applicativo* (connessioni rifiutate verso un server morto) si azzera.

### 3. ALG (Application Level Gateway)

Routing basato sul **contenuto applicativo** della richiesta: il proxy esamina il *path* dell'URL (la parte tra hostname e query) o l'*hostname* stesso e instrada verso il backend associato a una regola. Permette:

- **Condivisione delle porte ben note:** più servizi dietro la stessa porta esterna (443/22), superando il limite del port forwarding tradizionale (una porta esterna → un solo server interno).
- **Partizionamento del carico** per tipo di servizio o per provenienza geografica.
- **Effetto camera stagna:** un guasto resta circoscritto al perimetro di un singolo backend pool.

Esempio: `https://sito.com/wordpress/` → pool A, `https://sito.com/nodered/` → pool B, entrambi sulla 443.

## Layer 4 vs Layer 7

| | **Layer 4 (TCP)** | **Layer 7 (HTTP)** |
|---|---|---|
| Cosa vede | Solo IP e porte | Header, path, hostname, cookie |
| Velocità | Massima (non legge il contenuto) | Leggermente inferiore |
| Routing | Per IP/porta | Per URL, header, SNI |
| Health check | Connessione TCP (SYN/ACK) | Risposta HTTP (codici di stato) |
| Casi d'uso | DB (MySQL, Postgres), Redis | Applicazioni web, API |

Il routing L7 sul path (ALG) richiede di leggere il contenuto HTTP — quindi, su traffico cifrato, richiede la SSL termination.

## SSL Termination

Il DNAT puro non può leggere il contenuto cifrato. I reverse proxy applicativi risolvono aprendo **due connessioni TCP separate**:

- Il client parla **HTTPS** con il proxy, che **decifra** il traffico.
- Il proxy parla con i backend nella LAN privata, in chiaro (porta 80) o con cifratura separata.

Vantaggi: un **unico certificato** gestito in un solo punto (il proxy presenta un certificato controfirmato da una CA, valido per il dominio e i sottodomini), i backend alleggeriti dal carico crittografico, e la possibilità di fare routing L7 sul contenuto decifrato.

## Health check: attivi e passivi

- **Attivi:** il proxy interroga periodicamente ogni backend (es. ogni 2 s di default in HAProxy; spesso configurato a 500 ms–5 s). Una connessione TCP riuscita o un codice HTTP valido = server "up".
- **Passivi:** il proxy osserva il traffico reale; se un backend accumula errori, lo marca down. Convivono con quelli attivi, che restano responsabili di "rianimare" un server tornato sano.

## Modalità di guasto e insidie

- **Il load balancer è esso stesso uno SPOF:** va reso ridondante con [VRRP](vrrp_keepalived.md). DNAT clustering copre i backend, non il proxy.
- **Sessioni con stato:** senza sticky session o stato condiviso, un client può finire su backend diversi e perdere il login. Gestire con stick-tables o store esterno (Redis).
- **Health check troppo aggressivi/lenti:** intervalli troppo brevi caricano i backend; troppo lunghi allungano la finestra di packet loss durante un guasto.
- **DNAT non protegge i dati:** distribuisce traffico, ma se i backend non condividono lo storage, ognuno risponde con dati diversi. Serve il [piano dei dati](../continuita_di_servizio.md).

## Prodotti

- **HAProxy:** load balancer L4/L7 ad alte prestazioni, health check granulari, stick-tables.
- **Nginx:** reverse proxy/web server con modulo `upstream` per il bilanciamento.
- **iptables/nftables + IPVS:** DNAT a livello kernel (la base di LVS, usato da keepalived).

## Configurazione di esempio (didattica)

Un HAProxy che fa SSL termination, bilancia un pool web in round robin con health check ogni 500 ms, e usa l'ALG per instradare due servizi sulla stessa porta 443 in base al path. File `/etc/haproxy/haproxy.cfg` (estratto).

```
defaults
    mode http
    timeout connect 5s
    timeout client  30s
    timeout server  30s
    option httpchk GET /health      # health check L7: chiede /health ai backend

frontend web
    bind *:443 ssl crt /etc/haproxy/certs/sito.pem   # SSL termination (un solo certificato)

    # ALG: routing per path verso pool diversi, stessa porta 443
    use_backend pool_wordpress if { path_beg /wordpress/ }
    use_backend pool_nodered   if { path_beg /nodered/ }
    default_backend pool_web

backend pool_web
    balance roundrobin
    server web1 10.0.1.11:80 check inter 500ms   # 'check' = health check attivo
    server web2 10.0.1.12:80 check inter 500ms   # 'inter 500ms' = ogni mezzo secondo

backend pool_wordpress
    balance leastconn
    server wp1 10.0.1.21:80 check

backend pool_nodered
    server nr1 10.0.1.31:1880 check
```

Cosa succede: il client parla HTTPS con HAProxy (che decifra), HAProxy sceglie il pool in base al path e gira la richiesta in chiaro al backend sano. Se `web1` smette di rispondere a `/health`, viene tolto dalla rotazione entro la finestra di check.

> Semplificazione didattica: in produzione si aggiungono `option redispatch`, retry, stick-tables per le sessioni, rate limiting e una pagina `stats`. HAProxy va reso ridondante con [VRRP](vrrp_keepalived.md).

## Approfondimento

- **HAProxy — manuale di configurazione:** <https://docs.haproxy.org/>
- **HAProxy — tutorial L4/L7 e health check:** <https://www.haproxy.com/documentation/haproxy-configuration-tutorials/>
- **Nginx — modulo upstream (load balancing):** <https://nginx.org/en/docs/http/ngx_http_upstream_module.html>
