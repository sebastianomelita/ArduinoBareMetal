// =============================================================================
//  TOPIC-BUILDER.JS — costruzione coerente dei nomi dei topic
// =============================================================================
//
//  Per evitare che ogni file ricostruisca a mano i topic con il prefisso
//  CLASSE/UTENTE, centralizziamo qui la logica.
//
//  La struttura è gerarchica:
//
//    classe5b/marco-a3f2/luci/soggiorno/comandi
//    classe5b/marco-a3f2/luci/soggiorno/stato
//    classe5b/marco-a3f2/luci/soggiorno/config
//    classe5b/marco-a3f2/soggiorno/misure
//
//  In questo modo:
//    * ciascuno studente ha la sua "stanza virtuale" isolata,
//    * con la wildcard `classe5b/+/luci/+/stato` si possono "spiare" gli
//      stati di tutti i compagni (esercizio sulle wildcard).
//
// =============================================================================

import { CONFIG } from "./config.js";

export function buildTopics(utente) {
  const base = `${CONFIG.classe}/${utente}`;

  return {
    // Topic relativi alla lampada del soggiorno
    luciComandi: `${base}/luci/soggiorno/comandi`,
    luciStato:   `${base}/luci/soggiorno/stato`,
    luciConfig:  `${base}/luci/soggiorno/config`,

    // Topic relativi al sensore ambientale del soggiorno
    misure:      `${base}/soggiorno/misure`,

    // Wildcard per spiare i compagni — non si usa per pubblicare, solo per
    // sottoscrizioni "panoramiche" durante esercizi specifici
    wildcardStatiClasse: `${CONFIG.classe}/+/luci/+/stato`,
    wildcardMisureClasse: `${CONFIG.classe}/+/soggiorno/misure`,
  };
}
