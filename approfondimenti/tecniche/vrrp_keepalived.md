# VRRP e keepalived — Ridondanza del Punto di Ingresso

> Piano del Servizio · Approfondimento di [continuità di servizio](../continuita_di_servizio.md)

## Il problema che risolve

Un proxy o un router che riceve tutto il traffico in ingresso è un **singolo punto di guasto** (SPOF): se cade, l'intera infrastruttura a valle diventa irraggiungibile, anche se i server backend sono perfettamente sani. VRRP elimina questo SPOF facendo condividere a due o più dispositivi un unico indirizzo IP, in modo che ne esista sempre uno "vivo" che lo detiene.

## Cos'è VRRP

**VRRP (Virtual Router Redundancy Protocol)** è uno standard IETF — definito nella **RFC 5798** — che permette a un gruppo di router/host di presentarsi alla rete come un singolo *router virtuale*, identificato da un **VRID** (Virtual Router IDentifier) e da uno o più **indirizzi IP virtuali (VIP)**.

**keepalived** è l'implementazione software di VRRP più diffusa su Linux. Oltre a VRRP, integra un modulo di health checking (basato su LVS/IPVS) che può monitorare anche i server reali a valle, non solo i nodi VRRP.

## Come funziona, passo per passo

1. **Elezione del master.** Ogni nodo del gruppo VRRP ha una *priorità* (0–255). Il nodo con priorità più alta diventa **master** e si assume la proprietà del VIP. Gli altri restano **backup**.

2. **Annuncio del VIP via ARP gratuito.** Quando un nodo diventa master, invia un *gratuitous ARP* per associare il VIP al proprio indirizzo MAC: gli switch aggiornano le loro tabelle e il traffico verso il VIP arriva fisicamente a quel nodo. Il VIP non cambia mai dal punto di vista dei client.

3. **Heartbeat (advertisement).** Il master invia periodicamente messaggi VRRP multicast (di default ogni 1 secondo) per dire "sono vivo". I backup li ascoltano.

4. **Failover.** Se i backup non ricevono advertisement per un intervallo (il *Master Down Interval*, tipicamente ~3 secondi), considerano il master caduto. Il backup con priorità più alta si promuove a master, invia il gratuitous ARP e acquisisce il VIP. Il failover è quindi **in 2–3 secondi** ed è trasparente ai client.

5. **Preemption.** Se il vecchio master torna online con priorità superiore, può riprendersi il VIP (comportamento configurabile: spesso si disabilita la preemption per evitare doppi failover ravvicinati).

## Concetti chiave

- **VIP (Virtual IP):** l'indirizzo "stabile" verso cui puntano i client. Non appartiene fisicamente a nessuna scheda di rete: è gestito via software e "fluttua" tra i nodi.
- **Priorità:** determina chi è master. Si può anche abbassare dinamicamente con i `vrrp_script` (es. se HAProxy locale è morto, abbasso la mia priorità così cedo il VIP).
- **Track scripts:** keepalived può eseguire script di verifica (es. "il processo HAProxy è attivo?") e modulare la priorità in base al risultato. Questo collega lo stato di VRRP allo stato reale del servizio, non solo alla raggiungibilità di rete.

## VRRP non sa cosa fa il proxy

Punto cruciale spesso frainteso: **VRRP garantisce solo che esista sempre un nodo attivo che detiene il VIP.** Non sa nulla di cosa quel nodo fa al traffico — non bilancia il carico, non controlla i backend. Per quello serve un secondo livello (DNAT / load balancer, vedi [DNAT e load balancing](dnat_load_balancing.md)). VRRP e il bilanciamento sono due strati sovrapposti e complementari.

## Modalità di guasto e insidie

- **Split-brain VRRP.** Se i nodi non si vedono più tra loro (es. guasto della rete di heartbeat, ma entrambi ancora raggiungibili dai client) possono diventare *entrambi master* e rispondere allo stesso VIP, generando conflitti ARP e comportamenti erratici. Mitigazioni: rete di heartbeat ridondata e separata, uso di unicast invece di multicast in ambienti dove il multicast è filtrato.
- **VIP su sottoreti diverse.** VRRP lavora a livello 2/3: tutti i nodi del gruppo devono stare nella stessa sottorete/VLAN.
- **Failover ≠ continuità delle sessioni.** Se i due proxy non condividono lo stato delle sessioni, le connessioni TCP attive al momento del failover possono interrompersi. Per sessioni persistenti serve sincronizzazione dello stato (es. `stick-tables` sincronizzate in HAProxy).

## Quando usarlo

- Ridondanza di reverse proxy / load balancer (HAProxy, Nginx).
- Ridondanza di firewall e gateway (spesso con CARP, l'equivalente su BSD/pfSense).
- Default ground per qualsiasi servizio dove un singolo nodo di ingresso bloccherebbe tutto.

## Configurazione di esempio (didattica)

Due nodi che condividono il VIP `10.0.0.1`, con keepalived che abbassa la priorità se HAProxy locale muore (così cede il VIP al peer sano). File `/etc/keepalived/keepalived.conf`.

**Nodo MASTER:**

```
# Verifica periodica: HAProxy è vivo?
vrrp_script chk_haproxy {
    script "killall -0 haproxy"   # ritorna 0 se il processo esiste
    interval 2                    # controlla ogni 2 secondi
    weight -20                    # se fallisce, togli 20 alla priorità
}

vrrp_instance VI_1 {
    state MASTER
    interface eth0
    virtual_router_id 51          # STESSO VRID sui due nodi
    priority 150                  # il master ha priorità più alta
    advert_int 1                  # heartbeat ogni 1 s
    authentication {
        auth_type PASS
        auth_pass segreta
    }
    virtual_ipaddress {
        10.0.0.1/24               # il VIP condiviso
    }
    track_script {
        chk_haproxy               # collega lo stato VRRP a quello del proxy
    }
}
```

**Nodo BACKUP:** identico, ma con `state BACKUP` e `priority 100`. Se il master cade (o HAProxy muore e la priorità scende sotto 100), il backup acquisisce il VIP in 2–3 s.

> Semplificazione didattica: in produzione si aggiungono rete di heartbeat dedicata/ridondata, unicast se il multicast è filtrato, e si valuta `nopreempt` per evitare doppi failover.

## Approfondimento

- **Standard ufficiale (RFC 5798):** <https://datatracker.ietf.org/doc/html/rfc5798>
- **keepalived — sito ufficiale:** <https://www.keepalived.org/>
- **keepalived — documentazione:** <https://keepalived.readthedocs.io/en/latest/>
