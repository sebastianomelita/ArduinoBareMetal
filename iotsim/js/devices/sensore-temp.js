// =============================================================================
//  SENSORE-TEMP.JS — simula un sensore ambientale
// =============================================================================
//
//  Ruolo MQTT:
//    * PUBLISHER su topic misure (ogni N secondi, configurabile via config)
//
//  In un sistema reale i valori arrivano da un BME280 o simile via I2C.
//  Qui generiamo valori plausibili facendo oscillare leggermente attorno
//  a un valore base, così lo studente vede il messaggio cambiare di poco
//  ad ogni pubblicazione e capisce che è "live".
//
// =============================================================================

import { publish } from "../mqtt-bus.js";
import { CONFIG } from "../config.js";

export function creaSensoreTemp({ topicMisure, elementoUI }) {
  // Valori "centrali" attorno a cui oscilliamo. In una scena domotica questi
  // sarebbero i valori medi della stanza.
  const baseTemp = 23.0;
  const basePress = 1013;
  const baseHum = 45;

  let periodoMs = CONFIG.periodoMisureMs;
  let timer = null;

  function pubblicaMisura() {
    // Oscillazione casuale di +/- 0.5 °C, +/- 2 hPa, +/- 3% umidità
    const temp = +(baseTemp + (Math.random() - 0.5) * 1.0).toFixed(1);
    const press = Math.round(basePress + (Math.random() - 0.5) * 4);
    const hum = +(baseHum + (Math.random() - 0.5) * 6).toFixed(1);

    const payload = {
      envSensor: { temp, press, hum },
      timestamp: new Date().toISOString(),
    };

    publish(topicMisure, payload);
    aggiornaUI(elementoUI, temp);
  }

  function avvia() {
    if (timer) clearInterval(timer);
    timer = setInterval(pubblicaMisura, periodoMs);
    pubblicaMisura(); // pubblicazione immediata, così non si aspetta il primo tick
  }

  // Permettiamo di cambiare il periodo di pubblicazione a runtime
  // (utile se in futuro si vuole pilotare il sensore via topic config).
  function setPeriodo(nuovoMs) {
    if (typeof nuovoMs === "number" && nuovoMs >= 500) {
      periodoMs = nuovoMs;
      avvia();
    }
  }

  avvia();

  // Esponiamo qualche metodo nel caso si voglia controllare il sensore
  // dall'esterno (per esempio dal pannello di config o da console browser).
  return { setPeriodo };
}

// Aggiornamento dell'etichetta "23 °C" sotto al sensore nello SVG.
function aggiornaUI(el, temp) {
  if (!el) return;
  el.textContent = `${temp.toFixed(1)} °C`;
}
