# Piattaforma MQTT — domotica didattica

Simulatore web di una stanza domotica con 4 dispositivi MQTT (lampada, pulsante,
sensore di temperatura, display), pensato per studenti di 5° informatica come
complemento a [HiveMQ Websocket Client](https://www.hivemq.com/demos/websocket-client/).

## Caratteristiche

- **Pagina statica singola**: zero build, zero npm. Si pubblica copiando i file
  su qualunque server web.
- **Multiutente**: ogni studente al primo accesso sceglie un nome; la pagina
  genera un suffisso casuale e prefissa tutti i topic con `<classe>/<nome>/...`,
  così le stanze degli studenti restano isolate sul broker pubblico.
- **Console log integrata**: tutti i messaggi MQTT in entrata, in uscita e di
  sistema sono visibili in tempo reale.
- **Pannello manuale**: replica le funzioni di HiveMQ Websocket Client (sub/pub
  arbitrari, scelta del QoS) integrate nella pagina.
- **Dispositivi reattivi**: lampada, sensore e display sono autonomi e usano
  realmente MQTT; non sono mock.

## Deploy

Il modo più semplice è pubblicare la cartella su un server statico. Da
qualunque sistema:

```bash
# Esempio: copia nella web root di Apache/nginx della scuola
scp -r piattaforma-mqtt/ utente@server.scuola.it:/var/www/html/mqtt-sim/
```

Gli studenti aprono `https://studenti.marconicloud.it/shared/iotsim/)`.

> **Nota**: la pagina richiede una connessione `wss://` al broker. Se usate il
> broker HiveMQ pubblico (default), la pagina deve essere servita via HTTPS,
> altrimenti il browser blocca la connessione mista. Per test locale potete
> usare `http://localhost` (alcuni browser lo trattano come sicuro).

## Configurazione

Il file [`js/config.js`](js/config.js) contiene tutti i parametri:

- `brokerUrl` — indirizzo del broker (default HiveMQ pubblico)
- `classe` — nome della classe (es. `classe5b`), usato come primo livello dei topic
- `periodoMisureMs` — frequenza di pubblicazione del sensore di temperatura
- `qosDefault` — QoS di default per le pubblicazioni

Per usare un broker della scuola (Mosquitto con WebSocket abilitato):

```js
brokerUrl: "wss://broker.scuola.it:8884/mqtt"
```

## Struttura dei topic

Ogni studente ha la propria gerarchia, in stile domotica realistica:

```
classe5b/marco-a3f2/luci/soggiorno/comandi    [pulsante → lampada]
classe5b/marco-a3f2/luci/soggiorno/stato      [lampada → display]
classe5b/marco-a3f2/luci/soggiorno/config     [studente → lampada]
classe5b/marco-a3f2/soggiorno/misure          [sensore → server]
```

Wildcard utili per gli esercizi:

```
classe5b/+/luci/+/stato        — stati di tutte le lampade della classe
classe5b/+/soggiorno/misure    — misure di tutti i sensori della classe
classe5b/marco-a3f2/#          — tutto quello che riguarda Marco
```

## Scaletta esercizi

Gli esercizi vanno svolti aprendo **due tab affiancati**: la piattaforma e
HiveMQ Websocket Client (entrambi connessi a `broker.hivemq.com:8884`).

**1. Osservazione**
Aprire HiveMQ Websocket Client. Iscriversi a `classe5b/<vostro-nome>/#`.
Premere il pulsante nella piattaforma e osservare i messaggi che arrivano in
HiveMQ. Identificare quali sono `comandi`, quali `stato` e quali `misure`.

**2. Comando da HiveMQ**
Da HiveMQ pubblicare `{"toggle":"true"}` sul topic comandi del proprio utente.
Verificare che la lampada nella piattaforma cambi stato.

**3. Validazione del payload**
Pubblicare da HiveMQ un messaggio non valido (es. `{"foo":"bar"}` oppure
`questo non è json`). La lampada non reagisce. Aprire `js/devices/lampada.js`
e identificare il punto in cui il payload viene scartato.

**4. Wildcard**
Iscriversi a `classe5b/+/luci/+/stato` e osservare gli stati delle lampade di
tutti i compagni in tempo reale. Discussione: in che modo le wildcard rendono
scalabile un sistema domotico?

**5. Configurazione runtime**
Pubblicare `{"stateperiod":"1000"}` sul topic config della propria lampada.
La lampada inizia a pubblicare il proprio stato ogni secondo. Provare con
`{"stateperiod":"5000"}` per rallentarla.

**6. QoS**
Pubblicare lo stesso comando con QoS 0, 1, 2 dal pannello manuale e osservare
nel log come cambia il pattern di pubblicazione (per QoS > 0 il broker
risponde con un PUBACK/PUBCOMP che si vede nel log).

**7. Multi-tab e LWT (avanzato)**
Aprire due tab della piattaforma con lo stesso nome utente. Osservare cosa
succede al broker quando entrambi pubblicano sullo stesso topic. Discutere
il concetto di "Last Will and Testament" (non implementato in questo MVP,
può essere un esercizio di estensione).

## Come è fatto il codice

Tutto il codice è commentato come materiale di studio. Suggerimento di lettura:

1. [`js/config.js`](js/config.js) — parametri e identità studente
2. [`js/topic-builder.js`](js/topic-builder.js) — costruzione dei topic
3. [`js/mqtt-bus.js`](js/mqtt-bus.js) — wrapper su MQTT.js, event bus
4. [`js/devices/pulsante.js`](js/devices/pulsante.js) — il più semplice
5. [`js/devices/lampada.js`](js/devices/lampada.js) — il più ricco
6. [`js/devices/sensore-temp.js`](js/devices/sensore-temp.js) — pubblicazione periodica
7. [`js/devices/display.js`](js/devices/display.js) — subscriber passivo
8. [`js/app.js`](js/app.js) — orchestratore

Ogni modulo è scritto come funzione `crea*({…parametri})` che lo studente può
copiare/modificare per creare nuovi dispositivi (es. una seconda lampada in
cucina, un sensore di umidità, un termostato).

## Licenza

Materiale didattico open source, riutilizzabile e modificabile.
