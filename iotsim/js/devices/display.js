// =============================================================================
//  DISPLAY.JS — simula un display LED che mostra lo stato della lampada
// =============================================================================
//
//  Ruolo MQTT:
//    * SUBSCRIBER su topic stato -> riceve lo stato corrente e lo visualizza
//
//  È il dispositivo "passivo" della scena: non pubblica mai. Serve per
//  mostrare graficamente che il feedback dell'attuatore arriva — anche se
//  il display sta in un punto diverso della stanza rispetto alla lampada.
//
// =============================================================================

import { subscribe } from "../mqtt-bus.js";

export function creaDisplay({ topicStato, elementoUI }) {
  subscribe(topicStato, (_topic, payloadString) => {
    let dati;
    try {
      dati = JSON.parse(payloadString);
    } catch (e) {
      // Stato non JSON: mostriamo "??" per dare segnale visivo dell'errore.
      aggiornaUI(elementoUI, "??");
      return;
    }

    if (dati.state === "on" || dati.state === "off") {
      aggiornaUI(elementoUI, dati.state.toUpperCase());
    } else {
      aggiornaUI(elementoUI, "??");
    }
  });
}

function aggiornaUI(el, testo) {
  if (!el) return;
  el.textContent = testo;
}
