// =============================================================================
//  MQTT-BUS.JS — wrapper su MQTT.js con un piccolo event bus interno
// =============================================================================
//
//  Tutti i dispositivi simulati (lampada, pulsante, sensore, display) si
//  appoggiano a questo modulo per parlare con il broker. In questo modo:
//
//    * la libreria mqtt.js viene istanziata UNA SOLA VOLTA e condivisa,
//    * ogni device si registra con `subscribe(topic, callback)` senza dover
//      sapere come funziona internamente la libreria,
//    * tutti i messaggi (in entrata e in uscita) passano da qui, quindi
//      possiamo loggarli in modo uniforme nella console.
//
//  La libreria mqtt.js viene caricata da CDN nell'index.html e diventa
//  disponibile come oggetto globale `window.mqtt`.
//
// =============================================================================

import { CONFIG } from "./config.js";

// Mappa topic -> array di callback iscritti.
// Una stessa stringa di topic può avere più callback (es. il display ascolta
// lo stato per aggiornare la UI, e la console log ascolta tutto).
const iscrizioni = new Map();

// Listener per gli eventi "di sistema" (connesso, disconnesso, errore) e per
// il logging unificato dei messaggi entrata/uscita.
const eventListeners = {
  connect:    [],
  disconnect: [],
  error:      [],
  log:        [],
};

let client = null;

// -----------------------------------------------------------------------------
// connetti() — apre la connessione al broker.
// -----------------------------------------------------------------------------
export function connetti() {
  if (client) return client;

  // mqtt.connect è esposto globalmente dalla libreria caricata da CDN
  client = window.mqtt.connect(CONFIG.brokerUrl, {
    // clientId univoco per evitare collisioni con altri tab
    clientId: `mqtt-sim-${Math.random().toString(16).slice(2, 10)}`,
    clean: true,
    reconnectPeriod: 2000,
    connectTimeout: 10000,
  });

  client.on("connect", () => {
    emit("connect");
    emit("log", { dir: "sys", topic: "—", payload: "connesso al broker" });
  });

  client.on("close", () => {
    emit("disconnect");
    emit("log", { dir: "sys", topic: "—", payload: "disconnesso" });
  });

  client.on("error", (err) => {
    emit("error", err);
    emit("log", { dir: "sys", topic: "—", payload: `errore: ${err.message}` });
  });

  // Smistamento dei messaggi in arrivo: per ogni topic ricevuto, controlliamo
  // se qualche iscrizione (compresa una con wildcard) corrisponde, e in tal
  // caso invochiamo le sue callback.
  client.on("message", (topic, payloadBuf) => {
    const payload = payloadBuf.toString();
    emit("log", { dir: "in", topic, payload });

    for (const [pattern, callbacks] of iscrizioni) {
      if (matchTopic(pattern, topic)) {
        for (const cb of callbacks) {
          try {
            cb(topic, payload);
          } catch (e) {
            console.error(`Errore nella callback per ${pattern}:`, e);
          }
        }
      }
    }
  });

  return client;
}

// -----------------------------------------------------------------------------
// subscribe(topic, callback) — registra una sottoscrizione e una callback.
//
// Se è la prima volta che ci si iscrive a quel pattern, mandiamo anche una
// SUBSCRIBE al broker. Le iscrizioni successive allo stesso pattern aggiungono
// solo callback in locale, senza generare traffico aggiuntivo.
// -----------------------------------------------------------------------------
export function subscribe(topic, callback) {
  if (!iscrizioni.has(topic)) {
    iscrizioni.set(topic, []);
    if (client && client.connected) {
      client.subscribe(topic);
      emit("log", { dir: "sys", topic, payload: "SUBSCRIBE" });
    } else if (client) {
      // Se siamo in fase di riconnessione, mandiamo la subscribe al connect
      client.once("connect", () => {
        client.subscribe(topic);
        emit("log", { dir: "sys", topic, payload: "SUBSCRIBE (post-reconnect)" });
      });
    }
  }
  iscrizioni.get(topic).push(callback);
}

// -----------------------------------------------------------------------------
// publish(topic, payload, opts) — pubblica un messaggio sul broker.
//
// Se il payload è un oggetto, lo serializza come JSON. Se è già una stringa,
// lo invia così com'è (per supportare payload non-JSON nei test).
// -----------------------------------------------------------------------------
export function publish(topic, payload, opts = {}) {
  if (!client || !client.connected) {
    emit("log", { dir: "sys", topic, payload: "PUBLISH fallita: non connesso" });
    return;
  }

  const stringa = typeof payload === "string" ? payload : JSON.stringify(payload);
  const qos = opts.qos ?? CONFIG.qosDefault;
  const retain = opts.retain ?? false;

  client.publish(topic, stringa, { qos, retain });
  emit("log", { dir: "out", topic, payload: stringa });
}

// -----------------------------------------------------------------------------
// on(evento, callback) — registra un listener per gli eventi di sistema.
//
// Eventi disponibili: 'connect', 'disconnect', 'error', 'log'.
// -----------------------------------------------------------------------------
export function on(evento, callback) {
  if (!eventListeners[evento]) {
    throw new Error(`Evento sconosciuto: ${evento}`);
  }
  eventListeners[evento].push(callback);
}

// -----------------------------------------------------------------------------
// Helper interni
// -----------------------------------------------------------------------------

function emit(evento, payload) {
  const listeners = eventListeners[evento] || [];
  for (const l of listeners) {
    try {
      l(payload);
    } catch (e) {
      console.error(`Errore nel listener per '${evento}':`, e);
    }
  }
}

// Confronta un pattern MQTT (con eventuali wildcard + e #) con un topic concreto.
// Replica la stessa logica del broker per il routing locale verso le callback.
function matchTopic(pattern, topic) {
  const p = pattern.split("/");
  const t = topic.split("/");

  for (let i = 0; i < p.length; i++) {
    if (p[i] === "#") return true;        // # cattura tutto il resto
    if (p[i] === "+") continue;           // + cattura un singolo livello
    if (p[i] !== t[i]) return false;
  }

  return p.length === t.length;
}
