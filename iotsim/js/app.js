// =============================================================================
//  APP.JS — orchestratore principale della piattaforma
// =============================================================================
//
//  Questo file mette insieme tutti i pezzi: chiede l'identità dello studente,
//  costruisce i topic, apre la connessione al broker, monta i quattro
//  dispositivi nella scena, accende la console log e il pannello manuale.
//
//  È volutamente lineare: leggendolo dall'alto al basso si ricostruisce
//  l'intero ciclo di vita della pagina.
//
// =============================================================================

import { CONFIG, getUtente, resetUtente } from "./config.js";
import { buildTopics } from "./topic-builder.js";
import { connetti, on } from "./mqtt-bus.js";

import { creaPulsante }    from "./devices/pulsante.js";
import { creaLampada }     from "./devices/lampada.js";
import { creaSensoreTemp } from "./devices/sensore-temp.js";
import { creaDisplay }     from "./devices/display.js";

import { montaConsoleLog }    from "./ui/console-log.js";
import { montaPannelloManuale } from "./ui/pannello-manuale.js";

// -----------------------------------------------------------------------------
// 1) Identità studente e topic
// -----------------------------------------------------------------------------
const utente = getUtente();
const topics = buildTopics(utente);

// Mostra l'identità nell'header in alto
document.getElementById("etichetta-utente").textContent = utente;
document.getElementById("etichetta-broker").textContent = CONFIG.brokerUrl;
document.getElementById("etichetta-classe").textContent = CONFIG.classe;

document.getElementById("btn-cambia-utente").addEventListener("click", () => {
  if (confirm("Vuoi cambiare nome utente? La pagina si ricaricherà.")) {
    resetUtente();
  }
});

// -----------------------------------------------------------------------------
// 2) Console log e pannello manuale (montati subito, anche prima della
//    connessione, così lo studente vede gli eventi di sistema)
// -----------------------------------------------------------------------------
montaConsoleLog(document.getElementById("console-log"));
montaPannelloManuale(document.getElementById("pannello-manuale"), topics);

// -----------------------------------------------------------------------------
// 3) Indicatore di stato connessione
// -----------------------------------------------------------------------------
const indicatore = document.getElementById("indicatore-stato");

on("connect", () => {
  indicatore.textContent = "● connesso";
  indicatore.className = "stato-ok";
});

on("disconnect", () => {
  indicatore.textContent = "● disconnesso";
  indicatore.className = "stato-ko";
});

on("error", () => {
  indicatore.textContent = "● errore";
  indicatore.className = "stato-ko";
});

// -----------------------------------------------------------------------------
// 4) Connessione e montaggio dei dispositivi
// -----------------------------------------------------------------------------
connetti();

// Riferimenti agli elementi grafici nella scena (vedi index.html)
const elPulsante = document.getElementById("scena-pulsante");
const elLampada  = document.getElementById("scena-lampada");
const elSensore  = document.getElementById("scena-sensore-valore");
const elDisplay  = document.getElementById("scena-display");

creaPulsante({
  topicComandi: topics.luciComandi,
  elementoUI: elPulsante,
});

creaLampada({
  topicComandi: topics.luciComandi,
  topicStato:   topics.luciStato,
  topicConfig:  topics.luciConfig,
  elementoUI:   elLampada,
});

creaSensoreTemp({
  topicMisure: topics.misure,
  elementoUI:  elSensore,
});

creaDisplay({
  topicStato: topics.luciStato,
  elementoUI: elDisplay,
});

// -----------------------------------------------------------------------------
// 5) Avviso "broker pubblico"
// -----------------------------------------------------------------------------
// Se il broker è quello pubblico di HiveMQ, mostra un piccolo avviso che
// ricorda di non inviare dati sensibili. Se in futuro si configura un
// broker scolastico, l'avviso non appare.
if (CONFIG.brokerUrl.includes("hivemq.com")) {
  const avviso = document.getElementById("avviso-broker");
  avviso.style.display = "block";
}
