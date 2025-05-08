
# segmentazione

## Perche segmentare

I motivi per i quali una LAN debba essere divisa in più segmenti di LAN (fisici o logici) sono essenzialmente due:
- Privacy. Isolare gruppi di utenti o di dispositivi che hanno interesse a rimanere isolati da accessi estranei per ragioni di privacy o sicurezza di database o impianti critici.
- Prestazioni. Isolare gruppi di utenti o di servizi che devono essere trattati con priorità diversa o che devono usufruire di una banda diversa.

I motivi per i quali gli stessi segmenti debbano rimanere parte di una stessa infrastruttura di rete fisica senza una separazione completa (air gap completo) sono:
- Condivisione della stessa connessione Internet tramite lo stesso router di confine
- Condivisione di alcuni servizi di business che sono a comune tra uno o più gruppi di utenti (ERP, server farm, NAS, server DHCP, ecc..)
- Condivisione di alcuni servizi di sistema che sono a comune tra uno o più gruppi di utenti (server DHCP, server DNS, controller degli AP, servizi di autenticazione RADIUS, ecc..)

## Segmentazione fisica

- Le subnet sono generate sul router da due link fisici collegati a interfacce IP fisicamente distinte 
- Sui link fisici si diramano LAN fisicamente separate
- Subnet mappate su LAN con infrastruttura fisica distinta e quindi allocabili solo in ambienti diversi
- Subnets spazialmente allocate che comprendono gruppi di host separati in base alla dislocazione fisica degli apparati di rete

## Segmentazione fisica emulata

- Le subnet sono generate sul router da due link logici collegati a interfacce IP logicamente distinte 
- Sui link logici si diramano LAN fisicamente separate
- Subnet mappate su LAN con infrastruttura fisica distinta e quindi allocabili solo in ambienti diversi
- Subnets spazialmente allocate che comprendono gruppi di host separati in base alla dislocazione fisica degli apparati di rete

## Segmentazione logica

- Le subnet sono generate sul router da due link virtuali collegati a interfacce IP logicamente distinte
- Sui link virtuali si diramano LAN logicamente separate (VLAN)
- Le VLAN condividono la stessa infrastruttura fisica e quindi sono allocabili in qualunque ambiente
- Subnet mappate su LAN logiche
- Subnets delocalizzate che comprendono gruppi di host separati in base alla funzione o al servizio






