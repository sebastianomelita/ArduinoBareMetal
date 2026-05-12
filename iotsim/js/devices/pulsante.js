// =============================================================================
//  PULSANTE.JS — simula un pulsante a parete
// =============================================================================
//
//  Ruolo MQTT: PUBLISHER puro sul topic comandi.
//
//  Quando lo studente clicca sul pulsante grafico nella scena, la pagina
//  pubblica il payload `{"toggle":"true"}` sul topic comandi della sua
//  istanza. Il broker poi fa fan-out a tutti gli iscritti — nel nostro
//  caso, alla lampada (e a chi si è iscritto manualmente da HiveMQ).
//
// =============================================================================

import { publish } from "../mqtt-bus.js";

export function creaPulsante({ topicComandi, elementoUI }) {
  // L'elementoUI è il nodo DOM cliccabile (un cerchio SVG nella scena).
  // Quando viene premuto, pubblichiamo il messaggio toggle.
  elementoUI.addEventListener("click", () => {
    publish(topicComandi, { toggle: "true" });
    animaPressione(elementoUI);
  });
}

// Piccolo feedback visivo per far capire allo studente che il click è stato
// registrato (utile soprattutto sul broker pubblico, dove c'è un po' di
// latenza prima che lo stato della lampada cambi).
function animaPressione(el) {
  el.classList.add("pulsante-premuto");
  setTimeout(() => el.classList.remove("pulsante-premuto"), 180);
}
