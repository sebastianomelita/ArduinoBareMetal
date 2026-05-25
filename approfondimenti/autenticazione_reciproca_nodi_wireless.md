# Autenticazione reciproca dei nodi wireless

Nelle reti tradizionali cablate, la fiducia fra apparati è data per scontata: chi ha accesso fisico al cavo ha accesso alla rete. Nel wireless questo non vale più — il mezzo è condiviso, intercettabile e impersonabile — e diventa necessario che **ciascun nodo dimostri la propria identità all'altro** prima che lo scambio di traffico cominci. Questo principio si chiama **autenticazione reciproca** (o *mutual authentication*) e va distinto dall'autenticazione asimmetrica tipica dei portali web (dove solo il client autentica il server tramite certificato HTTPS, ma il server non sa chi è il client finché non si presenta).

In una rete wireless ben progettata l'autenticazione reciproca si applica a **due livelli distinti**, che usano tecnologie diverse perché risolvono problemi diversi:

- **Autenticazione client ↔ infrastruttura** (i tablet/dipendenti verso gli AP e il server RADIUS);
- **Autenticazione fra nodi del backhaul** (gli AP fra loro, sui link mesh).

## Client ↔ infrastruttura: 802.1X / EAP

Sul lato access — il "front office" della rete, dove client e AP si parlano — l'autenticazione reciproca è gestita dal framework **IEEE 802.1X** in coppia con un metodo **EAP** (Extensible Authentication Protocol). Il dialogo è triangolare e coinvolge supplicant (client), autenticatore (AP) e server di autenticazione (RADIUS). La parte "reciproca" emerge nei dettagli del metodo EAP scelto:

- **EAP-TLS** è il caso più rigoroso: entrambe le parti presentano un certificato X.509 e ognuna verifica quello dell'altra contro una CA fidata. Il client autentica il server controllando che il certificato sia firmato dalla CA aziendale e che il subject coincida col nome atteso; il server autentica il client verificando la firma del suo certificato e che non sia revocato.
- **PEAP/MSCHAPv2** ed **EAP-TTLS**: il client autentica il server con il certificato; il server autentica il client con username e password (dentro un tunnel TLS).

In entrambi i casi è il **server RADIUS** (FreeRADIUS, Microsoft NPS, Aruba ClearPass, Cisco ISE) il gestore centrale: custodisce la CA, valida i certificati e/o le password, e decide chi entra. La revoca di un certificato compromesso o l'aggiornamento delle policy si fanno una volta sola lì, e si propagano a tutti gli AP automaticamente.

## Backhaul mesh: autenticazione fra AP

Sul lato backhaul — il "back office" della rete, dove gli AP si parlano fra loro per inoltrare il traffico mesh — il problema è diverso. Non c'è un "client" e un "server": ci sono N nodi paritari (peer) che devono potersi fidare reciprocamente prima di scambiarsi frame, altrimenti un AP rogue piazzato vicino al sito da un attaccante potrebbe inserirsi nella mesh e intercettare/manipolare il traffico.

Le tecnologie disponibili dipendono dallo standard mesh adottato.

### IEEE 802.11s — SAE (Simultaneous Authentication of Equals)

Lo standard 802.11s definisce **SAE**, lo stesso meccanismo introdotto da WPA3 per l'access lato client, ma applicato fra peer mesh. SAE è una variante del **Dragonfly key exchange** (RFC 7664) basata su una **passphrase pre-condivisa** fra i nodi mesh. Caratteristiche essenziali:

- **Simmetrico**: nessun nodo è "client" o "server", entrambi seguono lo stesso protocollo;
- **Resistente al dizionario offline**: anche se un attaccante cattura tutto lo scambio, non può provare password a velocità arbitraria contro l'hash, perché ogni tentativo richiede un'interazione attiva con un peer (Forward Secrecy garantita);
- **Negozia una chiave di sessione fresca** ad ogni associazione, usata poi per cifrare i frame con AES-CCMP a livello link.

Una volta autenticati con SAE, i due nodi mesh stabiliscono una **Mesh Peering Management** session (MPM), che è l'analogo dell'associazione client-AP ma fra peer paritari. Tutti i frame successivi sul link sono cifrati e autenticati.

La gestione è **distribuita**: ogni nodo mesh conserva la propria passphrase, configurata al momento dell'installazione (tipicamente dal controller WLAN). Non c'è un server centrale come il RADIUS: la passphrase deve essere la stessa su tutti i nodi della mesh.

### Soluzioni proprietarie (Cisco, Aruba, MikroTik, Ubiquiti)

I principali vendor offrono varianti più sofisticate per ambienti enterprise:

- **Cisco Adaptive Wireless Path Protocol (AWPP)** sui Mesh AP, con autenticazione mutua basata su certificati X.509 firmati dalla CA del controller WLAN. Ogni Mesh AP riceve un certificato univoco al momento del *zero-touch provisioning* contro il WLC.
- **Aruba InstantOS Mesh** usa certificati per l'autenticazione mesh, sempre gestiti dal controller (Mobility Conductor).
- **MikroTik CAPsMAN + nv2** usa chiavi condivise o certificati a discrezione, ma è meno automatizzato.
- **Ubiquiti UniFi Mesh** usa una chiave condivisa derivata dal sito UniFi e dalla password admin, gestita centralmente dal UniFi Controller.

Il **denominatore comune** di queste soluzioni proprietarie è la **gestione centralizzata** via controller WLAN: il sistemista non configura singolarmente ogni AP, ma adotta una policy a livello di sito che il controller applica via *zero-touch provisioning* su ciascun nuovo nodo che entra nella mesh. La rotazione delle chiavi, la revoca dei certificati, l'aggiunta di nuovi AP autorizzati si fanno tutte lì.

### IETF / mesh community — IPsec / WireGuard sopra il link mesh

In alternativa, o in aggiunta, all'autenticazione mesh nativa di 802.11s, si può **incapsulare** il traffico fra nodi in un tunnel IPsec o WireGuard che gestisce indipendentemente l'autenticazione mutua a livello IP:

- **IPsec con IKEv2** + certificati X.509: standard maturo, supportato da qualunque router serio. Adatto se si vuole isolare il piano di gestione (es. il traffico verso il controller WLAN) dentro un tunnel cifrato sopra il link mesh.
- **WireGuard**: più moderno, basato su chiavi pubbliche Curve25519, configurazione semplicissima (una chiave per peer). Sta diventando lo standard di fatto nelle mesh community come Freifunk.

Questa via è quella adottata tipicamente quando i nodi mesh sono macchine Linux generaliste (es. router OpenWRT) piuttosto che AP dedicati di un vendor, e la flessibilità prevale sulla semplicità.

## Chi gestisce cosa, in sintesi

| Cosa autentica | Tecnologia | Gestore | Dove vive la "verità" |
|---|---|---|---|
| Client ↔ AP (access) | 802.1X + EAP-TLS / PEAP | Server RADIUS | LDAP / Active Directory / DB locale |
| AP ↔ AP (backhaul mesh) standard | SAE / 802.11s | Controller WLAN o config locale | Passphrase mesh distribuita |
| AP ↔ AP backhaul vendor | Certificati X.509 vendor | Controller WLAN (WLC, Conductor, UniFi) | CA interna del controller |
| AP ↔ Controller (management) | TLS reciproco (CAPWAP, UniFi inform protocol) | Controller WLAN | Certificati WLC |
| Tunnel management AP ↔ Gateway | IPsec IKEv2 / WireGuard | Sistemista / orchestrator | CA interna o chiavi statiche |

## Implementazione nel nostro progetto

Sul sito archeologico, riassumendo le scelte fatte nei paragrafi 3.a e 3.c:

- **Access dei minitablet**: EAP-TLS con certificati X.509 pre-installati, validati dal server **FreeRADIUS** in VLAN 20 che a sua volta interroga **LDAP** per il database utenti. Il certificato del RADIUS è firmato dalla CA interna dell'Ente, pre-installata nel trust store dei tablet.
- **Access dei dipendenti**: PEAP/MSCHAPv2 con credenziali in LDAP, gestione VLAN dinamica via `Tunnel-Private-Group-Id`.
- **Backhaul mesh fra T1...T8 e T7-GW**: **SAE** di 802.11s con passphrase mesh, oppure — se il vendor lo supporta — certificati gestiti centralmente dal controller WLAN (Cisco WLC, UniFi, o equivalente). Il controller si occupa anche del provisioning iniziale di ogni nuovo AP.
- **Traffico di management AP ↔ controller**: protocollo CAPWAP (Cisco/Aruba) o UniFi inform protocol, entrambi con autenticazione mutua via certificati gestiti dal controller stesso. In opzione, ulteriore incapsulamento in tunnel IPsec/WireGuard verso la VLAN 10.

Il principio di base è sempre lo stesso: **chi gestisce la sicurezza è il punto centrale di amministrazione** (il controller WLAN per il mesh, il RADIUS per il client), in modo che le credenziali non siano duplicate fra apparati e che la revoca o l'aggiornamento richiedano un'azione singola.
