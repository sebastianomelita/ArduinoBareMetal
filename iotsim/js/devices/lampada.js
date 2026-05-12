// =============================================================================
//  LAMPADA.JS — simula la lampada (attuatore)
// =============================================================================
//
//  Ruolo MQTT:
//    * SUBSCRIBER su topic comandi  -> riceve {"toggle":"true"} e cambia stato
//    * SUBSCRIBER su topic config   -> riceve impostazioni di funzionamento
//    * PUBLISHER  su topic stato    -> pubblica {"state":"on"} o {"state":"off"}
//                                       quando il proprio stato cambia
//
//  È il dispositivo più "ricco" della scena perché incarna il flusso
//  classico comando -> attuazione -> feedback descritto nel documento.
//
// =============================================================================

import { subscribe, publish } from "../mqtt-bus.js";

export function creaLampada({ topicComandi, topicStato, topicConfig, elementoUI }) {
  // Stato interno del dispositivo. In un IoT reale questo sarebbe il valore
  // letto dal pin GPIO; qui è una semplice variabile JS.
  let stato = "off";

  // ---- ricezione COMANDI ----------------------------------------------------
  subscribe(topicComandi, (_topic, payloadString) => {
    let comando;
    try {
      comando = JSON.parse(payloadString);
    } catch (e) {
      // Payload non JSON o malformato. In un dispositivo reale lo si scarta
      // silenziosamente per non bloccarsi su input ostili.
      console.warn("Lampada: comando non JSON, ignorato:", payloadString);
      return;
    }

    // Riconosciamo {"toggle":"true"} come "inverti lo stato corrente".
    if (comando.toggle === "true" || comando.toggle === true) {
      stato = (stato === "on") ? "off" : "on";
      aggiornaUI(elementoUI, stato);
      pubblicaStato();
    }
    // Volutamente non gestiamo altri comandi: lo studente può estendere il
    // codice per supportare {"on":"true"}, {"off":"true"}, {"dim":50}, ecc.
  });

  // ---- ricezione CONFIG -----------------------------------------------------
  subscribe(topicConfig, (_topic, payloadString) => {
    let cfg;
    try {
      cfg = JSON.parse(payloadString);
    } catch (e) {
      console.warn("Lampada: config non JSON, ignorato:", payloadString);
      return;
    }

    // Esempio di parametro di configurazione: stateperiod (period di
    // pubblicazione automatica dello stato, in ms). Qui dimostriamo come
    // un comando di configurazione possa modificare il comportamento del
    // dispositivo a runtime senza ricompilarlo.
    if (cfg.stateperiod) {
      const ms = parseInt(cfg.stateperiod, 10);
      if (!Number.isNaN(ms) && ms >= 500) {
        avviaPubblicazionePeriodica(ms);
      }
    }
  });

  // ---- pubblicazione STATO --------------------------------------------------
  function pubblicaStato() {
    publish(topicStato, { state: stato });
  }

  // Pubblicazione periodica dello stato: si attiva solo se è stato
  // configurato un `stateperiod` via topic config.
  let timerPeriodico = null;
  function avviaPubblicazionePeriodica(ms) {
    if (timerPeriodico) clearInterval(timerPeriodico);
    timerPeriodico = setInterval(pubblicaStato, ms);
  }

  // Stato iniziale: pubblichiamo subito off per allineare i subscriber appena
  // collegati (il display, ad esempio, deve sapere che la lampada parte spenta).
  aggiornaUI(elementoUI, stato);
  pubblicaStato();
}

// Aggiornamento del rendering della lampadina nello SVG: la classe CSS
// "lampada-on" attiva il bagliore, l'assenza tiene la lampada spenta.
function aggiornaUI(el, stato) {
  if (!el) return;
  if (stato === "on") el.classList.add("lampada-on");
  else                 el.classList.remove("lampada-on");
}
