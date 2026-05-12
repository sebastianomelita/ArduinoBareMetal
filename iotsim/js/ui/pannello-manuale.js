// =============================================================================
//  PANNELLO-MANUALE.JS — controlli per publish/subscribe manuali
// =============================================================================
//
//  Riproduce in piccolo le funzionalità di HiveMQ Websocket Client, integrate
//  nella pagina. Serve per gli esercizi in cui lo studente deve:
//    * iscriversi a una wildcard (es. classe5b/+/luci/+/stato)
//    * pubblicare un payload arbitrario (es. {"toggle":"true"}) per testare
//      la robustezza del dispositivo
//    * provare diversi livelli di QoS
//
//  Sotto il cofano, riusa subscribe() e publish() del mqtt-bus.
//
// =============================================================================

import { subscribe, publish } from "../mqtt-bus.js";

export function montaPannelloManuale(contenitore, topicsDefault) {
  contenitore.innerHTML = `
    <div class="pannello-sezione">
      <h3>Sottoscrivi manualmente</h3>
      <input type="text" class="campo-topic-sub" placeholder="topic (es. classe5b/+/luci/+/stato)" />
      <button class="btn-sub">SUBSCRIBE</button>
    </div>

    <div class="pannello-sezione">
      <h3>Pubblica manualmente</h3>
      <input type="text" class="campo-topic-pub" placeholder="topic" value="${topicsDefault.luciComandi}" />
      <textarea class="campo-payload" rows="3" placeholder='payload JSON, es. {"toggle":"true"}'>{"toggle":"true"}</textarea>
      <div class="riga-controlli">
        <label>QoS:
          <select class="campo-qos">
            <option value="0" selected>0</option>
            <option value="1">1</option>
            <option value="2">2</option>
          </select>
        </label>
        <button class="btn-pub">PUBLISH</button>
      </div>
    </div>

    <div class="pannello-sezione">
      <h3>Topic in uso</h3>
      <ul class="lista-topic">
        <li><code>${topicsDefault.luciComandi}</code></li>
        <li><code>${topicsDefault.luciStato}</code></li>
        <li><code>${topicsDefault.luciConfig}</code></li>
        <li><code>${topicsDefault.misure}</code></li>
      </ul>
    </div>
  `;

  // ---- bottone SUBSCRIBE ----------------------------------------------------
  contenitore.querySelector(".btn-sub").addEventListener("click", () => {
    const topic = contenitore.querySelector(".campo-topic-sub").value.trim();
    if (!topic) return alert("Inserisci un topic.");
    // La callback non fa nulla di specifico: il messaggio comparirà comunque
    // nella console log perché il bus traccia tutti i messaggi in entrata.
    subscribe(topic, () => {});
  });

  // ---- bottone PUBLISH ------------------------------------------------------
  contenitore.querySelector(".btn-pub").addEventListener("click", () => {
    const topic = contenitore.querySelector(".campo-topic-pub").value.trim();
    const payload = contenitore.querySelector(".campo-payload").value;
    const qos = parseInt(contenitore.querySelector(".campo-qos").value, 10);

    if (!topic) return alert("Inserisci un topic.");

    // Passiamo il payload come stringa: il dispositivo dall'altra parte se
    // ne servirà (è proprio lì che si fa l'esercizio sulla validazione JSON).
    publish(topic, payload, { qos });
  });

  // Attivazione "click su <code>" per copia rapida del topic
  contenitore.querySelectorAll(".lista-topic code").forEach((node) => {
    node.addEventListener("click", async () => {
      try {
        await navigator.clipboard.writeText(node.textContent);
        node.classList.add("copiato");
        setTimeout(() => node.classList.remove("copiato"), 800);
      } catch (e) {
        /* fallback silente: la clipboard può non essere disponibile in http */
      }
    });
  });
}
