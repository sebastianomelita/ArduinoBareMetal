// =============================================================================
//  CONFIG.JS — parametri globali della piattaforma di simulazione MQTT
// =============================================================================
//
//  Questo file contiene TUTTI i parametri che governano il comportamento della
//  piattaforma. È il primo file da leggere per capire come è impostata la
//  comunicazione MQTT e da modificare per adattarla a un'altra classe o a un
//  altro broker.
//
//  Scelte di design didattico:
//    * Tutti i topic vengono prefissati con CLASSE/UTENTE per evitare che
//      messaggi di studenti diversi si mischino sul broker pubblico.
//    * Il broker default è HiveMQ pubblico via WebSocket TLS (porta 8884).
//      Funziona da qualsiasi browser, zero installazione.
//    * Lo username "studente" viene chiesto al primo accesso e salvato in
//      localStorage, così chi torna sulla pagina ritrova il proprio ambiente.
//
// =============================================================================

export const CONFIG = {
  // Indirizzo del broker MQTT su WebSocket Secure.
  // Per usare un broker della scuola, sostituire con: "wss://broker.scuola.it:8884/mqtt"
  // brokerUrl: "wss://broker.hivemq.com:8884/mqtt",
  brokerUrl: "wss://proxy.marconicloud.it:8884/mqtt",

  // Nome della classe — usato come primo livello del topic.
  // Cambiarlo all'inizio dell'anno scolastico per fare "pulizia" rispetto
  // alle classi precedenti.
  classe: "classe5b",

  // Periodo (millisecondi) di pubblicazione automatica delle misure del
  // sensore di temperatura. Modificabile a runtime tramite il topic config.
  periodoMisureMs: 3000,

  // Quality of Service di default per le pubblicazioni.
  // 0 = at most once (fire and forget), 1 = at least once, 2 = exactly once
  qosDefault: 0,

  // Numero massimo di righe tenute in memoria nella console log.
  // Le righe più vecchie vengono scartate per non saturare il browser.
  maxLogRighe: 200,
};

// -----------------------------------------------------------------------------
// Gestione dell'identità dello studente.
//
// Al primo accesso chiediamo il nome e generiamo un suffisso random a 4 caratteri
// esadecimali. Il risultato viene salvato in localStorage così la pagina lo
// ritrova alle visite successive.
//
// Esempio: "marco" diventa "marco-a3f2" (univoco per questo browser).
// -----------------------------------------------------------------------------

const CHIAVE_STORAGE = "mqtt-sim-utente";

export function getUtente() {
  let utente = localStorage.getItem(CHIAVE_STORAGE);

  if (!utente) {
    const nomeBase = (prompt("Inserisci il tuo nome (solo lettere):") || "anonimo")
      .toLowerCase()
      .replace(/[^a-z0-9]/g, "")
      .slice(0, 20) || "anonimo";

    // Genera un suffisso casuale a 4 caratteri esadecimali
    const suffisso = Math.floor(Math.random() * 0xffff)
      .toString(16)
      .padStart(4, "0");

    utente = `${nomeBase}-${suffisso}`;
    localStorage.setItem(CHIAVE_STORAGE, utente);
  }

  return utente;
}

export function resetUtente() {
  localStorage.removeItem(CHIAVE_STORAGE);
  location.reload();
}
