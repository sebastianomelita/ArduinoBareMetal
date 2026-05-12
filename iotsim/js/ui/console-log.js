// =============================================================================
//  CONSOLE-LOG.JS — log scorrevole di tutti i messaggi MQTT
// =============================================================================
//
//  Si aggancia agli eventi 'log' del mqtt-bus e stampa una riga per ogni
//  messaggio in entrata, in uscita, o di sistema. Ogni riga contiene:
//    * timestamp HH:MM:SS
//    * direzione (IN, OUT, SYS)
//    * topic
//    * payload (JSON, evidenziato monospace)
//
//  Il log scorre dall'alto verso il basso (più recente in basso) e tiene
//  in memoria al massimo CONFIG.maxLogRighe messaggi per non saturare il DOM.
//
// =============================================================================

import { on } from "../mqtt-bus.js";
import { CONFIG } from "../config.js";

export function montaConsoleLog(elementoContenitore) {
  const lista = document.createElement("div");
  lista.className = "log-lista";
  elementoContenitore.appendChild(lista);

  on("log", ({ dir, topic, payload }) => {
    const riga = document.createElement("div");
    riga.className = `log-riga log-${dir}`;

    const ora = new Date();
    const hh = String(ora.getHours()).padStart(2, "0");
    const mm = String(ora.getMinutes()).padStart(2, "0");
    const ss = String(ora.getSeconds()).padStart(2, "0");

    riga.innerHTML = `
      <span class="log-tempo">${hh}:${mm}:${ss}</span>
      <span class="log-dir log-dir-${dir}">${etichettaDir(dir)}</span>
      <span class="log-topic">${escapeHtml(topic)}</span>
      <span class="log-payload">${escapeHtml(payload)}</span>
    `;

    lista.appendChild(riga);

    // Rimuovi le righe più vecchie se superiamo il limite
    while (lista.children.length > CONFIG.maxLogRighe) {
      lista.removeChild(lista.firstChild);
    }

    // Auto-scroll in fondo
    elementoContenitore.scrollTop = elementoContenitore.scrollHeight;
  });
}

function etichettaDir(dir) {
  return { in: "IN", out: "OUT", sys: "SYS" }[dir] || dir;
}

function escapeHtml(s) {
  return String(s).replace(/[&<>"']/g, (c) =>
    ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;" }[c])
  );
}
