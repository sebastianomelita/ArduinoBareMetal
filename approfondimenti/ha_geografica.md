# HA Geografica — Active-Active, Active-Passive e Quorum tra Datacenter

> Piano del Servizio + Piano dei Dati (a scala geografica) · Approfondimento di [continuità di servizio](../continuita_di_servizio.md)

## Il problema che risolve

Le tecniche viste finora coprono i guasti *dentro un sito*: un proxy che cade ([VRRP](vrrp_keepalived.md)), un nodo che cade ([HCI](hci_ceph.md)), un disco che cade ([ZFS](zfs_raid.md)). Ma cosa succede se cade **l'intero datacenter** — incendio, alluvione, blackout prolungato, taglio della fibra? Serve un secondo sito, geograficamente separato. È qui che entra la HA geografica, e con essa un problema nuovo che dentro un sito non esisteva: **la distanza**.

Attenzione a non confondere: **VRRP non risolve questo problema.** VRRP vive in un solo segmento L2 (stessa sottorete/VLAN) ed è *active-passive* (un solo nodo serve). Per più sedi servono meccanismi diversi.

## I due ingredienti della HA geografica

A scala geografica i due piani della continuità si realizzano con tecnologie specifiche.

### Piano del servizio: GSLB e anycast

Il punto d'ingresso non può essere un VIP condiviso (non attraversa i datacenter). Si usa invece:

- **GSLB (Global Server Load Balancing):** bilanciamento basato sul DNS. Il resolver restituisce l'IP del sito più vicino/sano in base a geolocalizzazione, latenza e health check. Cambiare sito = cambiare la risposta DNS (con il limite del *TTL* e della cache dei resolver).
- **Anycast:** lo stesso indirizzo IP è annunciato via BGP da più datacenter. La rete instrada ogni client al sito "più vicino" secondo il routing. Il failover è gestito dal protocollo di routing stesso (se un sito ritira l'annuncio, il traffico converge sugli altri).

### Piano dei dati: replica geografica

I dati vanno replicati tra i siti. Qui la distanza impone una scelta fondamentale:

- **Replica sincrona:** ogni scrittura è confermata solo quando è arrivata anche all'altro sito. Garantisce RPO ≈ 0, ma **ogni scrittura paga la latenza di andata e ritorno** tra i datacenter (es. Milano–Roma, RTT ~5–10 ms). Su transazioni frequenti questo degrada pesantemente le prestazioni.
- **Replica asincrona:** la scrittura si completa localmente e viene propagata "dopo". Nessuna penalità di latenza, ma **RPO > 0**: a un guasto improvviso si perdono i dati non ancora replicati.

È lo stesso compromesso dei [protocolli DRBD](drbd.md) (C sincrono vs A asincrono), ma amplificato dalla distanza geografica.

## Le quattro varianti

Per lo stesso obiettivo (sopravvivere alla perdita di un sito) esistono architetture diverse, con compromessi netti.

| Variante | Entrambi i siti servono? | Replica | RTO | RPO | Insidia principale |
|---|---|---|---|---|---|
| **Active-active geo** | Sì | sincrona | ≈ 0 | ≈ 0 | **split-brain** in partizione di rete |
| **Active-passive geo (DR)** | No (uno in standby caldo) | asincrona | minuti | > 0 | sito di DR inutilizzato; failover da testare |
| **Active-active single-site** | Sì (stesso DC) | sincrona LAN | ≈ 0 | ≈ 0 | non sopravvive alla perdita del sito |
| **Quorum a 3 siti** | Sì (i 2 di maggioranza) | sincrona | ≈ 0 | ≈ 0 | richiede una terza sede (anche solo arbitro) |

- **Active-active geo:** massimo sfruttamento (nessuna risorsa in standby) e RTO nullo, ma è il caso più difficile da rendere sicuro (vedi split-brain sotto).
- **Active-passive geo (DR site):** il secondo sito è in *standby caldo*, riceve replica asincrona e subentra solo al guasto del primario. Più semplice e senza rischio di divergenza, ma con RPO > 0 e risorse "ferme".
- **Active-active single-site:** qui [VRRP](vrrp_keepalived.md) + replica LAN sincrona funzionano bene — niente latenza geografica. Ma è ridondanza *intra-sito*: non protegge dalla perdita del datacenter. È un complemento, non una soluzione geografica.
- **Quorum a 3 siti:** il terzo sito fa da arbitro per stabilire la maggioranza ed evitare lo split-brain — esattamente il ruolo del [quorum di DRBD/Ceph](drbd.md), portato a livello geografico.

## Split-brain e teorema CAP

Lo **split-brain** è il pericolo strutturale dell'active-active geografico. Se il collegamento tra i due siti si interrompe ma entrambi restano raggiungibili dai client, ciascun sito continua a servire e a scrivere **indipendentemente**: i due insiemi di dati divergono e non si possono più fondere banalmente. Per un servizio con stato critico (un database, un servizio di chiavi) questo è catastrofico.

Il problema è inevitabile e ha un nome: **teorema CAP**. Durante una *partizione* di rete (P) si può garantire al massimo *una* tra **consistenza** (C) e **disponibilità** (A), non entrambe:

- scegliere **consistenza** → la sede in minoranza si ferma (smette di servire) per non divergere;
- scegliere **disponibilità** → entrambe continuano a servire, accettando il rischio di divergenza.

Le mitigazioni pratiche:

- **Quorum / arbitro su un terzo sito:** solo la maggioranza continua a scrivere; la minoranza si blocca. Elimina lo split-brain a costo di una terza sede (che può essere un piccolo nodo arbitro, anche in cloud).
- **Scritture su una sola sede (active-active asimmetrico):** entrambe leggono ovunque, ma le *scritture* vanno a un solo master per dato. Riduce la concorrenza ma elimina la divergenza.
- **Fencing:** isolare attivamente la sede non in maggioranza.

## Esempio guida: il Join Server LoRaWAN

Un caso reale dove questo problema è concreto. Nell'architettura LoRaWAN (spec **LoRaWAN Backend Interfaces**), il **Join Server (JS)** è il componente che custodisce le *root key* dei dispositivi (in un **HSM**) e, durante la procedura di attivazione *over-the-air* (OTAA), deriva le *session key* (NwkSKey, AppSKey). È identificato da un `JoinEUI` e viene contattato dal **Network Server** a ogni join.

Renderlo altamente disponibile è critico: se il JS è giù, nessun dispositivo nuovo può attivarsi. Un'architettura **active-active geografica** mette un JS a Milano e uno a Roma, entrambi attivi, con il DB chiavi (e l'HSM) replicati e un GSLB/anycast che instrada le richieste di join al più vicino.

![Join Server in active-active geografico](../img/09_geo_active_active.svg)

Ma proprio perché il JS gestisce **chiavi crittografiche**, è il caso peggiore per l'active-active geografico:

- la replica sincrona del DB chiavi fa pagare a ogni join la latenza Milano–Roma;
- in partizione di rete, due copie del DB chiavi che divergono significano session key incoerenti — un disastro di sicurezza;
- l'HSM stesso va sincronizzato cross-site, operazione non banale.

Per questo, in pratica, un JS si progetta spesso o come **active-passive** (un sito primario per le scritture, l'altro di DR) oppure con **quorum a 3 sedi**. È esattamente il tipo di valutazione che la teoria CAP impone di fare consapevolmente.

## Configurazione di esempio (didattica)

**GSLB via DNS** (concetto): un record con risoluzione geografica e health check, TTL basso per failover rapido.

```
; pseudo-zona GSLB — il resolver sceglie in base a geo + salute
join.example.   30  IN  A   192.0.2.10    ; DC Milano  (se sano e più vicino)
join.example.   30  IN  A   198.51.100.10 ; DC Roma     (se sano e più vicino)
; TTL 30 s = finestra massima di failover lato cache DNS
```

**Anycast** (concetto): lo stesso IP annunciato via BGP da entrambi i siti; se Milano ritira l'annuncio, il traffico converge su Roma.

```
# su ciascun border router, annuncio BGP dello stesso prefisso
network 203.0.113.0/24   # stesso IP anycast annunciato da Milano e da Roma
```

**Lato Network Server** (esempio reale, The Things Stack): l'NS prova prima il Join Server locale, poi quelli remoti via Backend Interfaces.

```yaml
# config.yml — l'NS contatta il JS per la procedura di join
join-servers:
  - file: "./js-milano.yml"
    join-euis:
      - "70B3D57ED0000000/24"   # JS che gestisce questo prefisso di JoinEUI
  - file: "./fallback/js.yml"
    join-euis:
      - "0000000000000000/0"    # fallback per tutti gli altri JoinEUI
```

> Semplificazione didattica: in produzione la parte difficile non è il routing (GSLB/anycast) ma la **coerenza del DB chiavi e dell'HSM** tra i siti, e la gestione della partizione di rete. Decidere *prima* la posizione CAP (consistenza o disponibilità) è parte del progetto, non un dettaglio operativo.

## Quando usarlo

- Quando la perdita di un intero datacenter è un rischio da coprire (obblighi di continuità, servizi critici nazionali).
- **Active-active geo** quando il carico è prevalentemente in lettura o partizionabile per chiave, e si può permettere un arbitro a 3 siti.
- **Active-passive geo** quando un RPO di qualche minuto e un failover gestito sono accettabili: più semplice e più sicuro.

## Quando NON usarlo

- Per la sola ridondanza intra-sito → bastano [VRRP](vrrp_keepalived.md) + [HCI](hci_ceph.md), molto più semplici.
- Active-active geografico su un servizio fortemente consistente e a scrittura intensa, senza arbitro: lo split-brain è quasi garantito al primo guasto di rete tra i siti.

## Approfondimento

- **LoRaWAN Backend Interfaces (TS002) — LoRa Alliance:** <https://resources.lora-alliance.org/technical-specifications/ts002-1-1-0-lorawan-backend-interfaces>
- **Anycast — RFC 4786 (Operation of Anycast Services):** <https://datatracker.ietf.org/doc/html/rfc4786>
- **Teorema CAP — "CAP Twelve Years Later" (Eric Brewer):** <https://www.infoq.com/articles/cap-twelve-years-later-how-the-rules-have-changed/>
