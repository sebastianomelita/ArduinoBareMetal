
# Aspetti critici da documentare

Documento unico: prima il **caso generale** (aspetti comuni a tutte le tecnologie),
poi gli aspetti **particolari** di ciascuna. Per ogni tecnologia vanno sempre documentati
anche gli aspetti comuni.

---

## **Aspetti critici comuni**
- **Schema fisico**:
     - Schema fisico (**planimetria**) dello scenario del problema con la rappresentazione di ambienti
  ed edifici chiave e schema (**indoor** ed **outdoor**) dell'infrastruttura, con **etichettatura univoca**
  di tutti gli asset tecnologici di rete.
     - Schema logico (albero degli **apparati attivi**) di tutti i dispositivi di rete con il loro **ruolo**
  e i **link virtuali** astratti ai vari livelli della **pila ISO/OSI** (tipicamente L2, L3, L7) che tipicamente consistono in:
          - link L2/L3 **tunnel** (VPN) su rete pubblica di un ISP di tipo Secure Network o Trusted Network
          - link L2/L3 **ponte radio 802.11** (WiFi) di tipo Client/Server o bridged
          - link L7 tra sensori/attuatori IP e il broker MQTT, oppure tra i gateway WSN e il broker MQTT.
- **Utenti e dispositivi**
     - Tipologia di **divisione in gruppi** degli utenti e loro caratterizzazione (dislocazione fisica
  delimitata mediante subnet oppure diffusa "a macchia di leopardo" mediante VLAN).
    - Definizione delle **tecnologie dei dispositivi** chiave: sensori/attuatori, **gateway**, e relativo
  **dimensionamento di massima** (quantità, numero di porte, banda, ecc.).
  *(I dettagli specifici — topologie, link radio, tipo di accesso, ecc. — sono nelle sezioni particolari.)*
    - Dislocazione di eventuali **router/Firewall**.
- **Vincoli**
    - Eventuali **vincoli normativi** sulle tecnologie in uso: **cablaggio strutturato**, **potenza**, **EIRP**, **ERP**, **duty cycle**.

- **Indirizzamento e routing**
    - **Subnetting** e definizione degli indirizzi (gruppi di utenti, server farm) e degli **indirizzi dei server**.
    - Definizione del **tipo di routing** (statico o dinamico). In caso di **routing statico**, definizione
  delle **tabelle di routing** più significative.
  *(Eccezione: nel WiFi Mesh il routing è sempre automatico — vedi sezione dedicata.)*
- **Servizi di rete**
     - Definizione della posizione dei **servizi di sistema** (DHCP, DNS), dislocati a scelta a bordo del **FW**, collegati al **CS**, o inseriti in una **server farm**.
     - Eventuale impostazione della **continuità del  servizio** mediante tecniche di replica agenti lungo il piano del servizio, il piano dei dati e il piano del ripristino (backup).
     - Eventuale installazione, sulle interfacce di **ingresso** (direzione IN) di ciascun router, delle **ACL** che definiscono il processo di **filtraggio dei pacchetti** per autorizzare/negare il traffico dati tra i gruppi di utenti del sistema.
     - Installazione sulla interfaccia WAN verso internet del processo **NAT** di traduzione degli indirizzi privati nel pool di indirizzi pubblici del router di confine sul link verso un ISP.
     - Eventuale installazione sulla interfaccia WAN verso internet e/o nella server farm di un servizio di **reverse proxy** per funzioni di **ALG** e **clustering**.
     - Eventuale installazione sulla interfaccia WAN verso internet di un **servizio di VPN** per la realizzazione di:
          - accesso remoto **home-to-site** per manutenzione occasionale e smartworking
          - di una connessione **site-to-site** (secure o trusted) per il collegamento stabile "like wired" verso una sede remota che fornisca proprietà di **autenticazione del nodo** e di **cifratura dei dati**.
- **Autenticazione**
     - Definizione delle **tecniche di autenticazione degli utenti** (es. **802.1X**) per l'accesso alla **risorsa rete** presso un **supplicant** (NAS) a scelta tra autenticazione:
          - **L2 EAP** di porta fisica per accesso presso uno **switch** (in base al MAC o all'id utente mediante **RADIUS/DIAMETER**)
          - **L2 EAP** di porta logica per accesso presso un **AP WiFi** (in base al MAC o all'id utente mediante **RADIUS/DIAMETER**) 
          - **L7 con Captive portal** per accesso presso uno **switch** o presso un **AP WiFi**, mediante sottomissione di username e password o inserimento di un codice vaucher su un form web.
     - Definizione delle tecniche di **autenticazione dei webservice** (openid, psw, sessioni, ecc.).
     - Definizione delle tecniche di **autorizzazione SSO** (openid, kerberos, ecc.).
     - Definizione delle tecniche di **autenticazione dei nodi sensori/attuatori** (certificati, psw, preshared key, ecc.).
     - Definizione delle tecniche di **autenticazione dei nodi di elaborazione/pubblicazione** (certificati, psw, preshared key, ecc.).
     - Definizione delle tecniche di **autenticazione dei nodi di smistamento** (certificati, vpn, ecc.).
- **Applicazione e dati IoT**
     - Definizione della posizione del **broker MQTT**.
     - Definizione dei **topic** utili per i casi d'uso richiesti.
     - Definizione dei **messaggi JSON** per alcuni dispositivi IoT significativi in merito a **comandi**,
  **stato** o **configurazione**.
     - Definizione del percorso dei dati tra sensori ed eventuali attuatori per stabilire la
  **sede dell'elaborazione dei comandi** più opportuna (locale/edge vs remota on-premise/cloud).
     - Definizione (anche in **pseudocodice**) delle **funzioni del firmware** di bordo dei dispositivi IoT.

## **Documentazione cablaggio**

Consiste nell'**ordine**:
1. Planimetria senza cablaggio
2. Planimetria con cablaggio
3. Albero degli apparati passivi
4. Tabella delle dorsali
5. Albero degli apparati attivi
6. Schema degli armadi
   
Alcuni documenti possono essere **trascurati** il relazione a quanto forte nel progetto è la responsabilità della definizione della parte fisica dei servizi. In linea di principio:
- se i servizi stanno in **cloud** la parte fisica corrispondente è delegata al datacenter che li ospita
- se i servizi stanno **on premise** la parte fisica corrispondente è responsabilità del progetto
- nelle reti mesh WiFi e WSN (LoraWAN, Zigbee, RFID) in cui il gateway WSN è assemblato insieme al modem wireless che realizzza l'accesso alla rete Internet o insieme all'AP che realizzza l'accesso alla rete LAN la parte fisica è praticamente nulla.

## Aspetti particolari per tecnologia

### Zigbee
- Posizionamento in planimetria dei **nodi** con relativa etichetta, avendo cura di posizionare tra essi
  almeno **un gateway** che permetta l'accesso a una rete IP. Progettare dei **percorsi alternativi
  (backup)** in caso di guasto del gateway principale.
- Tecnologie dei dispositivi: definizione della **tipologia di servizio** (polling sincrono,
  comando asincrono, ecc.).
- Gestire eventuali **vincoli di prossimità** (mediante controllo di potenza o gestione del roaming)
  ed eventuali **vincoli di posizionamento** (mediante trilaterazione).
- Definizione del percorso dei dati tra sensori ed eventuali attuatori per stabilire la
  **sede dell'elaborazione dei comandi** più opportuna (locale/edge sul gateway vs remota on-premise/cloud).

### Ethernet
- Definizione di **dorsali (wireless)** e di **punti di accesso e aggregazione** dei dispositivi utente.
- **Subnetting** strutturato:
  - subnet di **aggregazione** per i gruppi di utenti (generalmente statica);
  - subnet di **dorsale** (statica o automatica basata su **Link Local**);
  - subnet di **servizio** (server farm e **DMZ**).
- Definizione degli **indirizzi dei server** e dei **range** di quelli dei **dispositivi client IP** (PC, smartphone, tablets, sensori/attuatori).

### BLE
- **Nota:** rispetto alle altre tecnologie, la sezione BLE del documento originale **non specifica**
  le tecniche di autenticazione (utenti/servizi).
- Definizione del percorso dei dati tra sensori ed eventuali attuatori per stabilire la
  **sede dell'elaborazione dei comandi** più opportuna (locale/edge sul gateway vs remota on-premise/cloud).


### WiFi infrastruttura
- Definizione degli **indirizzi dei server** e dei **range** di quelli dei **dispositivi client IP** (PC, smartphone, tablets, sensori/attuatori).
- Gestire i **vincoli di prossimità**, distinguendo tra ambienti **indoor** e **outdoor** (controllo di
 potenza o gestione del roaming) e i **vincoli di posizionamento** (trilaterazione).
- Definizione della **posizione del controller degli AP**.
- Definizione delle tecniche di **autenticazione dei nodi AP** (certificati, psw, preshared key, ecc.) presso i servizi (es. AP su RADIUS).
- Se presenti VLAN, definizione del tipo di **associazione VLAN↔SSID** (statica o dinamica con **Tunnel-Private-Group-Id**).

### WiFi Mesh
- Posizionamento in planimetria dei **nodi** con relativa etichetta, avendo cura che tra essi esista
  almeno **un percorso LOS** (Line Of Sight) con dei vicini. Progettare dei **percorsi alternativi
  (backup)** in caso di guasto dei nodi centrali di traffico.
- Tecnologie dei dispositivi: **topologia** (stella, bus, singolo), **link**, **accesso radio**
  (TDM / CSMA/CA / slotted CSMA/CA) con dimensionamento.
- Definizione di **dorsali wireless** e di **punti di accesso e aggregazione** dei dispositivi utente.
- Definizione di eventuali **link di backhaul** e della relativa tecnologia.
- Definire le **interfacce radio** scegliendo tra **2-band** (dual channel) e **3-band** (three channel).
- Definire i **canali dei link** scegliendoli tra quelli in banda **ISM** e pianificandone il
  **riuso nello spazio**, minimizzando l'**interferenza cocanale**.
- Gestire i **vincoli di prossimità** (indoor/outdoor, controllo potenza/roaming) e i
  **vincoli di posizionamento** (trilaterazione).
- **Subnetting** strutturato: subnet di **aggregazione** (statica), subnet di **dorsale**
  (statica o Link Local), subnet di **servizio** (server farm e **DMZ**); più **indirizzi dei server**
  e **range** dei client.
- Definizione degli **indirizzi dei server** e dei **range** di quelli dei **dispositivi client IP** (PC, smartphone, tablets, sensori/attuatori).
- Definizione delle tecniche di **autenticazione dei nodi AP** (certificati, psw, preshared key, ecc.);
  reciproca (backhaul/link radio) o tra nodi e servizi (es. AP su RADIUS).
- Definizione del **tipo di rete mesh**: **routed mesh** vs **bridged mesh**.
- **Routing sempre automatico** (AODV, OLSR, Babel) con definizione automatica delle subnet dei link
  tra i nodi router (**LLA**, **SLAAC**). *(Sostituisce il routing statico/dinamico generale.)*
- Se presenti VLAN, definizione del tipo di **associazione VLAN↔SSID** (statica o dinamica con
  **Tunnel-Private-Group-Id**).
- Definizione della **posizione del controller degli AP**.

### LoRaWAN
- schema fisico (planimetria) dello scenario del problema con la rappresentazione di ambienti e edifici chiave e schema (indoor ed outdoor) dell'infrastruttura con etichettatura univoca di asset specifici quali sensori/attuatori, gateway/packet forwardere (PF), network server (NS), join server (JS) e application server (AP).
- Definire se è necessaria una **federazione di reti di sensori**: stabilire se **broker** e
  **Network Server** sono individualmente o entrambi parte del router/gateway verso la rete IP,
  oppure se sono **a comune** con più reti LoRaWAN.
- Definizione del percorso dei dati tra sensori ed eventuali attuatori per stabilire la
  **sede dell'elaborazione dei comandi** più opportuna dove effettuare decifrazione del payload. Ovvero, stabilire se l'AS debba stare o meno sul GW insieme al PF e al NS.
- Gestire i **vincoli di prossimità** (indoor/outdoor, controllo potenza/roaming) e i
  **vincoli di posizionamento** (trilaterazione).
- Definizione delle **classi di servizio** dei nodi sensori e attuatori
- Definizione della **modalità di autenticazione** (OTAA o ABP) dei nodi

### RFID
- **Tecnologie dei tag** da usare: **passivi**, **attivi**, **semi-passivi (BAP)**. La scelta determina
  **portata**, **costo** e **durata**.
- **Scelta della frequenza di lavoro** (**LF**, **HF**, **UHF**, **microonde**) in funzione di
  **materiali**, **distanza** e **vincoli normativi nazionali**.
- **Densità dei tag** simultanei nel campo (**dense reader environment**) e necessità di eventuali
  protocolli di **anticollisione**.
- Posizionamento in planimetria dei **nodi** con relativa etichetta, avendo cura che tra essi esista
  almeno **un gateway** che permetta l'accesso a una rete IP. Progettare dei **percorsi alternativi
  (backup)** in caso di guasto del gateway principale.
- Gestire eventuali **vincoli di prossimità** (controllo di potenza o gestione del roaming) ed eventuali
  **vincoli di posizionamento** (trilaterazione).
- Definizione del percorso dei dati tra sensori ed eventuali attuatori per stabilire la
  **sede dell'elaborazione dei comandi** più opportuna (locale/edge sul gateway vs remota on-premise/cloud).


---

# VLAN + OSPF + NAT — Cisco IOS Cheat Sheet

---

## 1 · Subnetting — pianificazione e indirizzamento

| Campo | Valore |
|-------|--------|
| Schema | VLAN 10 → `192.168.1.0/24` · GW: `.1.254` · BC: `.1.255` |
| Schema | VLAN 20 → `192.168.2.0/24` · GW: `.2.254` · BC: `.2.255` |

```
SW(config-if)# ip address 192.168.1.252 255.255.255.0   ← SVI admin
SW(config-if)# no shutdown
SW(config)#   ip default-gateway 192.168.1.254           ← solo su SW L2
```

> `ip default-gateway` si usa solo su switch L2 (senza `ip routing`).
> Ogni switch prende un indirizzo SVI diverso (.252, .251, .250 …)

**Test**
```
SW# show interfaces vlan 10
SW# ping 192.168.1.254
```

---

## 2 · Creazione VLAN

```
SW(config)# vlan 10
SW(config-vlan)# name ROSSA
SW(config-vlan)# exit
```

**Test**
```
SW# show vlan brief
SW# show vlan id 10
```

---

## 3 · Porte di accesso (access)

```
SW(config)# interface fa0/1                  (oppure: interface range fa0/3-18)
SW(config-if)# switchport mode access
SW(config-if)# switchport access vlan 10
SW(config-if)# exit
```

**Test**
```
SW# show vlan brief
SW# show interfaces fa0/1 switchport
```

> `show interfaces switchport` mostra mode (access/trunk) e VLAN assegnata.

---

## 4 · Porte di trunk

```
SW(config-if)# switchport mode trunk
SW(config-if)# switchport trunk native vlan 99
SW(config-if)# switchport trunk allowed vlan 10,20
```

**Test**
```
SW# show interfaces trunk
SW# show interfaces fa0/24 switchport
```

> Native VLAN dedicata (es. 99) diversa dalle VLAN dati — evita attacchi VLAN hopping.
> `show interfaces trunk` mostra le VLAN allowed, active e in STP forwarding.

---

## 5 · Inter-VLAN Routing — 3 metodi

### Tradizionale
```
R(config-if)# ip address 192.168.1.254 255.255.255.0
R(config-if)# no shutdown                               ← ripetere per ogni interfaccia/VLAN
```

### Router-on-a-stick
```
R(config-if)# encapsulation dot1q 10
R(config-if)# ip address 192.168.1.254 255.255.255.0    ← su ogni subinterface
R(config)#   ip route 0.0.0.0 0.0.0.0 <next-hop-WAN>
```

### Switch L3 (SVI)
```
SW(config)#   ip routing
SW(config-if)# ip address 192.168.1.254 255.255.255.0   ← su ogni interface vlan X
SW(config-if)# no switchport                            ← porta uplink verso router
SW(config)#   ip route 0.0.0.0 0.0.0.0 10.0.3.2
```

**Test**
```
R# show ip interface brief
R# show ip route
R# ping 192.168.2.1 source 192.168.1.254
```

> `ping` con `source` verifica il routing tra VLAN — se risponde, l'inter-VLAN funziona.

---

## 6 · OSPF — fasi di lavoro in `(config-router)`

```
R(config)# router ospf 100

R(config-router)# router-id 1.1.1.1                             ← 1. Router-ID

R(config-router)# passive-interface default                      ← 2. Blocca Hello su tutte le porte
R(config-router)# no passive-interface GigabitEthernet0/2        ←    riabilita solo le porte di transito

R(config-router)# area 10 stub                                   ← 3. Area stub (su IR e ABR)
R(config-router)# area 10 stub no-summary                        ← 4. Totally stub (solo ABR)

R(config-router)# area 10 range 192.168.1.0 255.255.255.0        ← 5. Summarization uscente (solo ABR)

R(config)#        ip route 0.0.0.0 0.0.0.0 <ip-ISP>             ← 6. Default route (solo ASBR)
R(config-router)# default-information originate
```

> Passi 3–6 opzionali secondo lo scenario.
> `area range` si imposta **solo sugli ABR** per rotte uscenti dall'area (mai sulla backbone).
> `stub` va configurato sia sull'ABR che su tutti gli IR dell'area.

**Test**
```
R# show ip ospf neighbor       ← lo stato deve essere FULL
R# show ip ospf
R# show ip ospf database       ← mostra il LSDB con tutti i LSA ricevuti
```

---

## 7 · OSPF — fasi di lavoro in `(config-if)`

```
R(config-if)# ip address 192.168.4.2 255.255.255.252
R(config-if)# ip ospf 100 area 0
R(config-if)# no shutdown
```

> Ripetere per ogni interfaccia specificando il numero di area corretto
> (0 = backbone, 10/20 = aree stub di aggregazione).

**Test**
```
R# show ip interface brief
R# show ip ospf interface GigabitEthernet0/0    ← mostra area, cost, stato DR/BDR e timer Hello/Dead
```

---

## 8 · NAT Cisco IOS — configurazione interfacce (comune a tutti i tipi)

```
R(config)# interface fa0/0
R(config-if)# ip address 192.168.1.254 255.255.255.0
R(config-if)# ip nat inside
R(config-if)# no shutdown

R(config)# interface fa0/1
R(config-if)# ip address 213.234.10.2 255.255.255.252
R(config-if)# ip nat outside
R(config-if)# no shutdown
```

> `ip nat inside` / `ip nat outside` va sempre impostato su entrambe le interfacce,
> indipendentemente dal tipo di NAT.

---

## 9 · NAT statico (SNAT 1:1)

```
R(config)# ip nat inside source static 172.16.0.5 10.16.0.5
```

> Mappatura permanente 1:1 — la connessione può essere iniziata anche dall'esterno.
> Usato per server sempre raggiungibili da Internet.

**Test**
```
R# show ip nat translations
R# show ip nat statistics
```

---

## 10 · NAT dinamico (SNAT 1:N con pool)

```
R(config)# ip nat pool DYNAMIC-IP 10.0.16.1 10.0.16.6 prefix-length 29

R(config)# ip access-list standard NAT-LIST
R(config-std-nacl)# permit 172.16.0.0 0.0.0.255
R(config-std-nacl)# exit

R(config)# ip nat inside source list NAT-LIST pool DYNAMIC-IP
```

> Traduzione temporanea 1:N — ogni host inside ottiene un IP del pool per la durata
> della connessione. Se il pool è esaurito la connessione viene rifiutata.

**Test**
```
R# show ip nat translations
R# show ip nat statistics
```

---

## 11 · PAT / PNAT (NAT overload — un solo IP pubblico) + Port Forwarding

```
enable
configure terminal

! Interfaccia inside
interface GigabitEthernet0/0
 description Internal Network
 ip address 192.168.1.1 255.255.255.0
 ip nat inside
 no shutdown

! Interfaccia outside
interface GigabitEthernet0/1
 description External Network
 ip address 203.0.113.1 255.255.255.0
 ip nat outside
 no shutdown

! ACL per identificare il traffico inside da tradurre
access-list 1 permit 192.168.1.0 0.0.0.255

! NAT overload (PAT) — tutti gli inside escono con l'IP di Gi0/1
ip nat inside source list 1 interface GigabitEthernet0/1 overload

! Port forwarding (DNAT) — espone il server interno su porte 80 e 443
ip nat inside source static tcp 192.168.1.100 80  203.0.113.1 80
ip nat inside source static tcp 192.168.1.100 443 203.0.113.1 443

end
copy running-config startup-config
```

> `overload` = PAT — tutti gli host inside condividono l'IP dell'interfaccia outside,
> differenziati per porta TCP/UDP.
> `static tcp` aggiunge un DNAT per ogni porta esposta: le connessioni esterne sulla
> porta pubblica vengono redirette al server interno. Le due regole coesistono: PAT
> gestisce il traffico uscente, il port forwarding gestisce il traffico entrante.

**Test**
```
R# show ip nat translations
R# show ip nat statistics
R# debug ip nat
```

---

## 12 · NAT Linux — iptables SNAT / PAT

```bash
# Abilita il forwarding IP
sudo sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward"

# SNAT / PAT con MASQUERADE
sudo iptables -t nat -A POSTROUTING -o eth1 -j MASQUERADE

# Accetta risposte dall'esterno solo per connessioni già stabilite
sudo iptables -A FORWARD -i eth1 -o eth0 -m state --state RELATED,ESTABLISHED -j ACCEPT

# Accetta tutte le richieste partenti dall'interno
sudo iptables -A FORWARD -i eth0 -o eth1 -j ACCEPT

# Salva le regole
sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"
```

> `eth0` = inside (LAN) · `eth1` = outside (WAN)
> `MASQUERADE` usa automaticamente l'IP corrente dell'interfaccia outside.

**Test**
```bash
sudo iptables -t nat -L -v
sudo iptables -L FORWARD -v
```

---

## 13 · NAT Linux — iptables port forwarding (DNAT)

```bash
# DNAT su porta singola
sudo iptables -t nat -A PREROUTING -i eth1 -p tcp --dport 443 -j DNAT --to 172.31.0.23:443

# DNAT su porte multiple
sudo iptables -t nat -A PREROUTING -i eth1 -p tcp -m multiport --dports 80,443 -j DNAT --to 172.31.0.23:443

# Apri FORWARD verso il server (solo se la policy default è DROP)
sudo iptables -A FORWARD -i eth1 -o eth0 -p tcp --dport 443 -d 172.31.0.23 -j ACCEPT

# Salva le regole
sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"
```

> `PREROUTING` intercetta i pacchetti prima del routing — necessario per il DNAT.
> La regola `FORWARD` è necessaria solo se la policy default è `DROP`.

**Test**
```bash
sudo iptables -t nat -L PREROUTING -v
sudo iptables -L FORWARD -v
```

---

## 14 · VRRP — keepalived (High Availability IP virtuale)

VRRP elegge un **MASTER** che detiene l'IP virtuale; il **BACKUP** subentra automaticamente se il master cade.
`virtual_router_id` e `auth_pass` devono essere **identici** su entrambi i nodi.

### Nodo MASTER (`/etc/keepalived/keepalived.conf`)

```
vrrp_script chk_haproxy {
    script   "pidof haproxy"
    interval 2
}

vrrp_instance VI_1 {
    state            MASTER
    interface        eth0
    virtual_router_id 51
    priority         100          ← più alto = master preferito
    advert_int       1
    authentication {
        auth_type PASS
        auth_pass 1234
    }
    virtual_ipaddress {
        192.168.1.100             ← IP virtuale condiviso
    }
    track_script {
        chk_haproxy               ← se haproxy muore, priority scende sotto il backup
    }
}
```

### Nodo BACKUP (`/etc/keepalived/keepalived.conf`)

```
vrrp_script chk_haproxy {
    script   "pidof haproxy"
    interval 2
}

vrrp_instance VI_1 {
    state            BACKUP
    interface        eth0
    virtual_router_id 51
    priority         90           ← più basso del master
    advert_int       1
    authentication {
        auth_type PASS
        auth_pass 1234
    }
    virtual_ipaddress {
        192.168.1.100
    }
    track_script {
        chk_haproxy
    }
}
```

> `track_script` monitora haproxy: se il processo muore il nodo abbassa la propria
> priority sotto quella del backup, cedendogli automaticamente l'IP virtuale (failover).

**Test**
```bash
sudo systemctl start keepalived
sudo systemctl status keepalived
ip addr show eth0                  ← sul MASTER deve comparire 192.168.1.100
sudo tcpdump -i eth0 vrrp          ← verifica gli advertisement VRRP
```

---

## 15 · HAProxy — ALG, clustering e HA

HAProxy riceve le connessioni sull'IP virtuale VRRP e le smista ai backend.

```
# /etc/haproxy/haproxy.cfg

global
    log /dev/log local0
    log /dev/log local1 notice
    maxconn 4096
    user  haproxy
    group haproxy
    daemon

defaults
    log     global
    mode    http
    option  httplog
    option  dontlognull
    timeout connect 5000
    timeout client  50000
    timeout server  50000

# ── FRONTEND ────────────────────────────────────────────────────────────────
frontend http_front
    bind *:80
    bind *:443 ssl crt /etc/haproxy/cert.pem

    # Redirect HTTP → HTTPS
    http-request redirect scheme https unless { ssl_fc }

    # Informa il backend che il proxy ha terminato TLS
    http-request add-header X-Forwarded-Proto https if { ssl_fc }

    # ACL per ALG (routing L7 per hostname)
    acl is_blog hdr_end(host) -i blog.miosito.com
    acl is_web  hdr_end(host) -i web.miosito.com

    use_backend blog_backend if is_blog
    use_backend web_backend  if is_web

# ── BACKEND BLOG (singolo server) ───────────────────────────────────────────
backend blog_backend
    server blog_server1 blog.miosito.com:80 check

# ── BACKEND WEB (clustering round-robin) ────────────────────────────────────
backend web_backend
    balance roundrobin
    server web_server1 web.miosito.com:80 check
    server web_server2 web.miosito.com:80 check
    server web_server3 web.miosito.com:80 check
```

> **ALG** — `acl` + `use_backend` instrada il traffico in base all'hostname (routing L7):
> ogni dominio va su un pool di server diverso, tutti sulla stessa porta 443 pubblica.
>
> **Clustering** — `balance roundrobin` distribuisce le richieste tra i server del pool.
> Sostituire con `balance leastconn` per bilanciamento sul numero di connessioni attive.
>
> **HA** — `check` abilita l'health check sul server: se non risponde HAProxy lo esclude
> automaticamente dal pool senza interruzione del servizio.
>
> **HA del proxy stesso** — viene garantita da VRRP (keepalived): se il nodo MASTER cade,
> il BACKUP acquisisce l'IP virtuale e HAProxy continua a servire le richieste.

**Test**
```bash
sudo haproxy -c -f /etc/haproxy/haproxy.cfg    ← valida la sintassi
sudo systemctl restart haproxy
sudo systemctl status haproxy

# Statistiche in tempo reale (se abilitato stats socket)
echo "show info" | sudo socat stdio /var/run/haproxy/admin.sock
echo "show stat"  | sudo socat stdio /var/run/haproxy/admin.sock | cut -d',' -f1,2,18,19
```

---

## 16 · ACL — definizione e tipi

Una **ACL** (Access Control List) è una lista ordinata di **ACE** (Access Control Entry).
Il router esamina le ACE in sequenza: alla prima corrispondenza esegue l'azione e si ferma.
Se nessuna ACE corrisponde si applica la **policy implicita di default** (sempre `deny all`).

| Tipo | Range numerico | Filtra per | Posizionamento |
|------|---------------|------------|----------------|
| Standard | 1–99 e 1300–1999 | solo **sorgente IP** | vicino alla **destinazione** |
| Estesa | 100–199 e 2000–2699 | sorgente, destinazione, protocollo, porta, flag TCP | vicino alla **sorgente** |
| Standard con nome | `ip access-list standard NOME` | solo sorgente IP | vicino alla destinazione |
| Estesa con nome | `ip access-list extended NOME` | tutto | vicino alla sorgente |

> Le ACL standard vanno messe vicino alla **destinazione** perché filtrano solo per sorgente:
> messe vicino all'origine bloccherebbero tutto il traffico di quell'host, non solo quello verso la destinazione voluta.
>
> Le ACL estese vanno messe vicino alla **sorgente** per evitare che i pacchetti percorrano
> inutilmente la rete prima di essere scartati.

### Sintassi base

```
! ACL standard (numerata)
access-list 1–99 {permit|deny} <sorgente> <wildcard>

! ACL estesa (numerata)
access-list 100–199 {permit|deny} <protocollo> <sorgente> <wild> <destinazione> <wild> [eq <porta>]

! ACL con nome (preferibile in produzione)
ip access-list {standard|extended} NOME
 {permit|deny} ...

! Applicazione all'interfaccia
interface <X>
 ip access-group <numero|NOME> {in|out}
```

> `in` = filtra pacchetti **entranti** nell'interfaccia (protezione del sistema locale).
> `out` = filtra pacchetti **uscenti** dall'interfaccia (gestisce anche il traffico generato localmente).

**Test**
```
R# show access-lists
R# show ip interface <X>          ← mostra quale ACL è applicata e in che direzione
```

---

## 17 · ACL firewall — scenari tipici

### Permettere un solo host (ACL standard)

```
! Solo 192.168.10.1 può attraversare e0 — tutti gli altri bloccati (deny implicito)
access-list 1 permit host 192.168.10.1

interface ethernet0
 ip access-group 1 in
```

### Negare un host, permettere tutti gli altri (policy permit all)

```
! Regola più selettiva prima — poi permit any come default esplicito
access-list 10 deny   host 192.168.0.1
access-list 10 permit any

interface ethernet0
 ip access-group 10 in
```

### Permettere una subnet verso un'altra subnet (ACL estesa)

```
access-list 101 permit ip 192.168.10.0 0.0.0.255 192.168.200.0 0.0.0.255

interface ethernet0
 ip access-group 101 in
```

### Negare Telnet (porta 23), permettere tutto il resto

```
access-list 102 deny   tcp any any eq 23
access-list 102 permit ip  any any

interface ethernet0
 ip access-group 102 in
```

### Permettere solo DNS (porta 53) — deny implicito su tutto il resto

```
access-list 112 permit udp any any eq domain
access-list 112 permit tcp any any eq domain

interface ethernet0
 ip access-group 112 in
```

### Connessioni monodirezionali — `established` (stateless)

```
! Solo pacchetti di risposta TCP (ACK o RST settati) da NetB verso NetA
! NetA può aprire verso NetB, NetB non può aprire verso NetA
access-list 102 permit tcp any any gt 1023 established

interface ethernet0
 ip access-group 102 in
```

> `established` seleziona i pacchetti con flag ACK o RST — esclude i SYN puri (nuove connessioni).
> È una tecnica **stateless**: un attaccante può falsificare i flag. Preferire le ACL riflessive.

---

## 18 · ACL riflessive (stateful)

Le ACL riflessive creano dinamicamente ACE temporanee al passaggio del traffico uscente,
permettendo automaticamente solo le risposte a connessioni già aperte dall'interno.

```
! Passo 1 — ACL interna: traccia sessioni in uscita e crea ACE temporanee
R1(config)# ip access-list extended ACL_INTERNA
R1(config-ext-nacl)# permit tcp any any eq 80 reflect ACL-RIFLESSIVA-SOLO-WEB
R1(config-ext-nacl)# permit udp any any eq 53 reflect ACL-RIFLESSIVA-SOLO-DNS timeout 10
R1(config-ext-nacl)# exit

! Passo 2 — ACL esterna: valuta le ACE riflessive per il traffico di ritorno
R1(config)# ip access-list extended ACL_ESTERNA
R1(config-ext-nacl)# evaluate ACL-RIFLESSIVA-SOLO-WEB
R1(config-ext-nacl)# evaluate ACL-RIFLESSIVA-SOLO-DNS
R1(config-ext-nacl)# deny ip any any
R1(config-ext-nacl)# exit

! Passo 3 — Applica sulle interfacce
R1(config)# interface s0/0/0
R1(config-if)# ip access-group ACL_INTERNA out    ← traffico uscente verso WAN
R1(config-if)# ip access-group ACL_ESTERNA in     ← traffico entrante dalla WAN
```

> `reflect` crea automaticamente una ACE inversa temporanea ogni volta che un host interno
> apre una sessione. `evaluate` usa quelle ACE per decidere se far passare il traffico di ritorno.
> `timeout` (secondi) definisce per quanto rimane attiva la ACE riflessiva per protocolli stateless (UDP).

---

## 19 · ACL anti-spoofing — firewall WAN e LAN

### Filtro WAN in ingresso — blocca indirizzi non instradabili

```
! Blocca sorgenti non legittime provenienti dall'esterno
access-list 101 deny ip host 0.0.0.0       any              log   ! default gateway
access-list 101 deny ip 127.0.0.0   0.255.255.255  any      log   ! loopback
access-list 101 deny ip 10.0.0.0    0.255.255.255  any      log   ! RFC 1918 classe A
access-list 101 deny ip 172.16.0.0  0.15.255.255   any      log   ! RFC 1918 classe B
access-list 101 deny ip 192.168.0.0 0.0.255.255    any      log   ! RFC 1918 classe C
access-list 101 permit ip any any                                  ! permit esplicito per il resto

interface fa0/1                   ! interfaccia WAN (outside)
 ip access-group 101 in
```

### Filtro LAN in ingresso — blocca indirizzi "marziani"

```
! Blocca traffico uscente con sorgenti estranee al piano di indirizzamento
access-list 112 deny ip 172.16.0.0 0.15.255.255  any
access-list 112 deny ip 10.0.0.0   0.255.255.255 any
access-list 112 permit ip any any

interface fa0/0                   ! interfaccia LAN (inside)
 ip access-group 112 in
```

> **Anti-spoofing WAN**: nessun pacchetto con IP sorgente privato (RFC 1918) dovrebbe mai
> arrivare dall'esterno — se arriva è falsificato.
>
> **Anti-spoofing LAN**: nessun host interno dovrebbe trasmettere con un IP sorgente
> estraneo al piano di indirizzamento locale (indirizzi "marziani").

**Test**
```
R# show access-lists 101
R# show access-lists 112
R# show ip interface fa0/1        ← verifica che l'ACL sia applicata
```

---

## 20 · Tunnel GRE L3 — configurazione interfaccia tunnel

Ogni tunnel è un link punto-punto virtuale tra due router. Vanno creati **su entrambi i capi** con sorgente e destinazione invertite.

```
Router(config)# interface Tunnel0
Router(config-if)# ip address 10.0.11.1 255.255.255.0   ← IP del tunnel (dorsale logica)
Router(config-if)# tunnel source 198.64.5.1              ← IP pubblico locale
Router(config-if)# tunnel destination 39.68.34.121       ← IP pubblico del peer remoto
Router(config-if)# tunnel mode gre ip
Router(config-if)# ip ospf network point-to-point        ← evita elezione DR/BDR sul tunnel
Router(config-if)# ip ospf 100 area 0
Router(config-if)# no shutdown
Router(config-if)# exit
```

> Per ogni sede remota va creato un tunnel separato (Tunnel0, Tunnel1, Tunnel2 …).
> `tunnel source` e `tunnel destination` usano gli IP **pubblici** delle interfacce WAN.
> `ip ospf network point-to-point` è necessario sui tunnel GRE per far funzionare OSPF correttamente.

### Esempio sede centrale (R0 — IP pubblico 198.64.5.1)

```
! Tunnel verso Sede Remota 1 (R1 — 39.68.34.121)
interface Tunnel0
 ip address 10.0.11.2 255.255.255.0
 tunnel source 198.64.5.1
 tunnel destination 39.68.34.121
 tunnel mode gre ip
 ip ospf network point-to-point
 ip ospf 100 area 0
 no shutdown

! Tunnel verso Sede Remota 2 (R2 — 39.68.34.122)
interface Tunnel1
 ip address 10.0.12.2 255.255.255.0
 tunnel source 198.64.5.1
 tunnel destination 39.68.34.122
 tunnel mode gre ip
 ip ospf network point-to-point
 ip ospf 100 area 0
 no shutdown

! Tunnel verso Sede Remota 3 (R3 — 39.68.34.123)
interface Tunnel2
 ip address 10.0.13.2 255.255.255.0
 tunnel source 198.64.5.1
 tunnel destination 39.68.34.123
 tunnel mode gre ip
 ip ospf network point-to-point
 ip ospf 100 area 0
 no shutdown
```

---

## 21 · ACL firewall GRE — permetti traffico tunnel

I pacchetti GRE devono essere esplicitamente permessi sull'interfaccia WAN, altrimenti il firewall li scarta.

```
! ACL estesa applicata all'interfaccia outside (WAN)
Router(config)# ip access-list extended ALLOW_GRE
Router(config-ext-nacl)# permit gre  any any    ← traffico tunnel GRE
Router(config-ext-nacl)# permit ospf any any    ← hello e LSA OSPF
Router(config-ext-nacl)# permit ip   any any    ← tutto il resto (restringere in produzione)
Router(config-ext-nacl)# exit

! Applica all'interfaccia WAN in ingresso
Router(config)# interface GigabitEthernet0/0
Router(config-if)# ip access-group ALLOW_GRE in
Router(config-if)# exit

Router(config)# end
Router# write memory
```

> `permit gre any any` è necessario perché i tunnel GRE usano il protocollo IP numero 47.
> Senza questa regola il router scarta i pacchetti incapsulati e il tunnel non si forma.
> In produzione sostituire `any` con gli IP pubblici specifici dei peer per maggiore sicurezza.

**Test tunnel e OSPF**
```
Router# show ip ospf neighbor               ← i peer sui tunnel devono essere FULL
Router# show ip route ospf                  ← le rotte delle sedi remote devono comparire
Router# show interfaces Tunnel0             ← stato up/up e statistiche pacchetti
Router# ping 10.0.11.1 source 10.0.11.2    ← verifica connettività end-to-end sul tunnel
Router# debug ip ospf events
Router# no debug all
```

---

## 22 · Tunnel L2 Linux — OpenVPN TAP e GRETAP

### Quando usare L2 invece di GRE L3

| | GRE L3 (§20) | OpenVPN TAP / GRETAP |
|---|---|---|
| Trasporta | Pacchetti IP (L3) | Frame Ethernet completi (L2) |
| VLAN 802.1q | Non trasparente | Trasparente — tag passano intatti |
| Broadcast/ARP | Non propagati | Propagati come su cavo fisico |
| Bridge richiesto | No | Sì — tap0/gretap va in bridge con eth0 |
| Subnet tra sedi | Diverse — OSPF propaga le rotte | Identiche — le VLAN si estendono |
| Cifratura | Nessuna (serve IPsec) | TLS nativo (OpenVPN) / nessuna (GRETAP) |

### Incapsulamento PDU (comune a entrambe le opzioni)

```
┌──────────────────────────────────────────────────────────┐
│  PDU INTERNA in chiaro  (frame Ethernet 802.1q)          │
│  MAC dst | MAC src | 802.1q tag | EtherType | payload    │
└───────────────────────────┬──────────────────────────────┘
                            │  cifratura TLS (OpenVPN)
                            │  nessuna cifratura (GRETAP)
                            ▼
┌──────────────┬─────────────────────────────────────────  ┐
│ Header est.  │  SDU = frame Ethernet (cifrato o in chiaro)│
│ OpenVPN: UDP │  tag VLAN 802.1q intatto dentro il payload │
│ GRETAP:  GRE │                                            │
└──────────────┴────────────────────────────────────────────┘
```

### OpenVPN TAP — server (/etc/openvpn/server.conf)

```
mode server
tls-server
proto udp
port 1194
dev tap0          # TAP = L2, trasporta frame Ethernet
dev-type tap

ca   /etc/openvpn/ca.crt
cert /etc/openvpn/server.crt
key  /etc/openvpn/server.key
dh   /etc/openvpn/dh.pem

# server-bridge: pool IP per gli endpoint del bridge
server-bridge 10.0.3.254 255.255.255.0 10.0.3.100 10.0.3.200

persist-tun
persist-key
keepalive 10 120
verb 3
```

### OpenVPN TAP — bridge sede centrale (tap0+eth0)

```bash
ip link add name br0 type bridge && ip link set br0 up
ip link set tap0 master br0    # tunnel VPN
ip link set eth0 master br0    # trunk verso switch (NO ip su eth0)
ip addr add 10.0.3.254/24 dev br0

# Permanente in /etc/network/interfaces:
auto br0
iface br0 inet static
    address 10.0.3.254
    netmask 255.255.255.0
    bridge_ports eth0 tap0
    bridge_stp off
    bridge_fd 0
```

### OpenVPN TAP — client (/etc/openvpn/client.conf)

```
client
tls-client
proto udp
remote <IP-PUBBLICO-SERVER> 1194
dev tap0
dev-type tap

ca   /etc/openvpn/ca.crt
cert /etc/openvpn/client1.crt   # certificato diverso per ogni sede
key  /etc/openvpn/client1.key

persist-tun
persist-key
keepalive 10 120
```

### OpenVPN TAP — bridge sede remota (tap0+eth0)

```bash
ip link add name br0 type bridge && ip link set br0 up
ip link set tap0 master br0    # interfaccia VPN
ip link set eth0 master br0    # trunk verso switch locale

# SNAT per internet (su eth1 WAN, NON sul bridge)
echo 1 > /proc/sys/net/ipv4/ip_forward
iptables -t nat -A POSTROUTING -o eth1 -j MASQUERADE
iptables -A FORWARD -i br0 -o eth1 -j ACCEPT
iptables -A FORWARD -i eth1 -o br0 -m state --state RELATED,ESTABLISHED -j ACCEPT
```

> Il MASQUERADE si applica sull'interfaccia WAN fisica `eth1`, **non** sul bridge.
> Il bridge è trasparente a L2: i frame con tag VLAN attraversano il tunnel intatti.

### GRETAP — sede centrale (una interfaccia per ogni sede remota)

```bash
ip link add gretap1 type gretap local <IP-PUB-CENTRALE> remote <IP-PUB-R1>
ip link add gretap2 type gretap local <IP-PUB-CENTRALE> remote <IP-PUB-R2>
ip link set gretap1 up && ip link set gretap2 up

ip link add name br0 type bridge && ip link set br0 up
ip link set gretap1 master br0
ip link set gretap2 master br0
ip link set eth0    master br0    # trunk verso switch
ip addr add 10.0.3.254/24 dev br0
```

### GRETAP — sede remota

```bash
ip link add gretap0 type gretap local <IP-PUB-REMOTA> remote <IP-PUB-CENTRALE>
ip link set gretap0 up

ip link add name br0 type bridge && ip link set br0 up
ip link set gretap0 master br0
ip link set eth0    master br0    # trunk locale

iptables -t nat -A POSTROUTING -o eth1 -j MASQUERADE   # SNAT per internet
```

> GRETAP è disponibile **solo su Linux** — non su Cisco IOS.
> Non cifra nativamente: usare solo su link fidati o aggiungere IPsec.

**Test**
```bash
# OpenVPN
sudo systemctl status openvpn@server
ip link show tap0
brctl show br0

# GRETAP
ip -d link show gretap0
brctl show br0

# Test L2 (verifica che ARP e VLAN passino trasparenti)
arping -I br0 <IP-host-remoto>
ping <IP> source <IP-locale>
```

---

## 23 · L2TPv3 — Cisco IOS (tunnel L2 nativo)

L2TPv3 è la tecnologia IOS nativa per trasportare frame Ethernet su UDP/IP.
Crea un **pseudowire** punto-punto tra due router senza bridge software.

> L2TPv3 è l'alternativa IOS a OpenVPN TAP e GRETAP.
> Il trunk 802.1q (con i tag VLAN) passa attraverso il pseudowire in modo trasparente.
> `VC-ID` deve essere **identico** su entrambi i lati.

### Sede staccata

```
pseudowire-class PW-CENTRALE
 encapsulation l2tpv3
 protocol none
 ip local interface GigabitEthernet0/1   ! interfaccia con IP pubblico locale

! Porta fisica trunk verso switch — nessun IP, xconnect verso centrale
interface GigabitEthernet0/0
 no ip address
 xconnect <IP-PUBBLICO-CENTRALE> 100 pw-class PW-CENTRALE
```

### Sede centrale

```
pseudowire-class PW-STACCATA
 encapsulation l2tpv3
 protocol none
 ip local interface GigabitEthernet0/1   ! IP pubblico centrale

interface GigabitEthernet0/0
 no ip address
 xconnect <IP-PUBBLICO-STACCATA> 100 pw-class PW-STACCATA
 ! VC-ID 100 deve corrispondere esattamente all'altro lato
```

> Per più sedi remote servono più pseudowire, uno per ogni sede (VC-ID diversi: 100, 101, 102 ...).
> Se si vuole unire più sedi in un unico dominio L2 si usa **VPLS** (IOS-XE/XR con MPLS).

**Test**
```
R# show l2tun session all
R# show xconnect all
R# debug l2tpv3 data
SW# show mac address-table    ! verifica che i MAC remoti vengano appresi
```

---

## 24 · 802.1X — autenticazione utenti sulle porte access

802.1X blocca la porta switch finché l'utente non si autentica tramite un server RADIUS.
La porta rimane in stato `unauthorized` (traffico bloccato) finché l'autenticazione non ha successo.

### Fasi di configurazione

| Fase | Dove | Cosa |
|------|------|------|
| 1 | Switch | Abilita AAA e 802.1X globalmente |
| 2 | Switch | Configura il server RADIUS |
| 3 | Switch | Abilita `dot1x port-control auto` su ogni porta access |
| 4 | Router RADIUS | Configura client (lo switch) e utenti |

### Switch — configurazione globale e RADIUS

```
! Passo 1 — abilita AAA e 802.1X globalmente
aaa new-model
aaa authentication dot1x default group radius
dot1x system-auth-control

! Passo 2 — configura il server RADIUS
! porta auth 1812 e acct 1813 (RFC 2865/2866 — standard moderni)
radius-server host 192.168.1.100 auth-port 1812 acct-port 1813 key <chiave-condivisa>

! username locale di fallback (se RADIUS non raggiungibile)
username admin secret <password>
aaa authentication dot1x default group radius local
```

### Switch — porte access con 802.1X

```
interface range fa0/1-48
 switchport mode access
 switchport access vlan 10
 dot1x port-control auto      ! richiede autenticazione prima di passare traffico
 spanning-tree portfast        ! evita attesa STP (safe su porte access)
```

> `dot1x port-control` ha tre modalità:
> - `auto` — autentica prima di passare traffico (normale)
> - `force-authorized` — non autentica mai (default — evita interruzioni durante il deploy)
> - `force-unauthorized` — blocca sempre la porta

### Server RADIUS — FreeRADIUS (/etc/freeradius/clients.conf e users)

```
# clients.conf — dichiara lo switch come client RADIUS
client 192.168.1.0/24 {
    secret    = <chiave-condivisa>   # deve corrispondere al comando radius-server key
    shortname = switch-lab
}

# users — utenti autorizzati
mario  Cleartext-Password := "password123"
       Reply-Message = "Benvenuto, %u"
```

> La chiave `secret` in `clients.conf` **deve essere identica** al parametro `key` del comando
> `radius-server host` sullo switch. Un mismatch causa il fallimento silenzioso di tutte le autenticazioni.

**Test**
```
SW# show dot1x all
SW# show dot1x interface fa0/1
SW# show aaa sessions
SW# debug dot1x all

# Sul server RADIUS:
sudo tail -f /var/log/freeradius/radius.log
```

---

## ✓ Test end-to-end — comandi utili su tutti i dispositivi

```
ping <ip-destinazione>
ping <ip-dest> source <ip-sorgente>
traceroute <ip-destinazione>

R# show ip route
R# show ip route ospf
R# show ip route 192.168.1.0

R# show ip interface brief
R# show interfaces GigabitEthernet0/0

R# show running-config
R# show running-config | section ospf
R# show running-config | section nat
```

> `traceroute` mostra il percorso hop-by-hop — utile per verificare quale router
> smista il traffico tra VLAN o aree OSPF.

