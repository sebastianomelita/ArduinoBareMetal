/**
 * MQTT Downlink Encoder
 * =====================
 *
 * Modulo pure-JS per costruire i payload downlink da inviare al firmware
 * Heltec V4 SCD41+L76K. Ogni funzione ritorna un oggetto:
 *
 *   { data: "base64_string", fPort: 10|20 }
 *
 * Il chiamante costruisce il JSON MQTT completo che ChirpStack richiede:
 *
 *   Topic:   application/<UUID>/device/<DevEUI>/command/down
 *   Payload: {
 *              "devEui":    "<DevEUI>",
 *              "fPort":     10,
 *              "confirmed": false,
 *              "data":      "<base64_string>"
 *            }
 *
 * Il codec ChirpStack (chirpstack_codec.js) decodifica la base64 in bytes.
 * Il fPort e' fornito esplicitamente da chi pubblica (necessario per la
 * validazione ChirpStack), coerente col fPort che il codec dedurrebbe
 * comunque dal primo byte del payload.
 *
 * Esempio d'uso:
 *   const cmd = MqttDownlink.encodeReboot();
 *   // cmd = { data: "AQ==", fPort: 10 }
 *   mqttClient.publish(topic, JSON.stringify({
 *       devEui: deviceEui,
 *       fPort: cmd.fPort,
 *       confirmed: false,
 *       data: cmd.data
 *   }));
 *
 * Nessuna dipendenza esterna. Funziona in browser e in Node.js.
 */

(function (global) {
    'use strict';

    // -------------------------------------------------------------
    // Utility interne
    // -------------------------------------------------------------

    /**
     * Converte un array di bytes in stringa base64.
     * In browser usa btoa(), in Node.js usa Buffer.
     */
    function bytesToBase64(bytes) {
        if (typeof btoa === 'function') {
            // Browser
            var s = '';
            for (var i = 0; i < bytes.length; i++) {
                s += String.fromCharCode(bytes[i]);
            }
            return btoa(s);
        } else if (typeof Buffer !== 'undefined') {
            // Node.js
            return Buffer.from(bytes).toString('base64');
        } else {
            throw new Error('No base64 encoder available');
        }
    }

    /**
     * Valida che un numero sia intero nel range [min, max].
     * Lancia Error con messaggio esplicito se fuori range.
     */
    function validateRange(value, min, max, name) {
        if (typeof value !== 'number' || !Number.isFinite(value) ||
            !Number.isInteger(value)) {
            throw new Error(name + ' deve essere un intero, ricevuto: ' + value);
        }
        if (value < min || value > max) {
            throw new Error(name + ' fuori range [' + min + '-' + max + '], ricevuto: ' + value);
        }
    }

    // -------------------------------------------------------------
    // Costanti: mapping preset TX interval
    // -------------------------------------------------------------

    var TX_INTERVAL_PRESETS = {
        0: { seconds: 10,   label: '10 secondi' },
        1: { seconds: 20,   label: '20 secondi' },
        2: { seconds: 60,   label: '1 minuto' },
        3: { seconds: 300,  label: '5 minuti' },
        4: { seconds: 600,  label: '10 minuti' },
        5: { seconds: 1800, label: '30 minuti' }
    };

    // -------------------------------------------------------------
    // Codici comando (allineati col firmware .ino)
    // -------------------------------------------------------------

    // FPort 10: azioni
    var CMD_REBOOT       = 0x01;
    var CMD_IDENTIFY     = 0x02;
    var CMD_FORCE_TX_NOW = 0x03;
    var CMD_CLEAR_NVS    = 0x04;
    var CMD_IDENTIFY_ON  = 0x05;
    var CMD_IDENTIFY_OFF = 0x06;
    var CMD_GET_STATE    = 0x07;

    // FPort 20: configurazione
    var CFG_SET_TX_INTERVAL = 0x11;
    var CFG_SET_LORAWAN_SF  = 0x12;
    var CFG_SET_TX_POWER    = 0x13;
    var CFG_SET_GPS_TIMEOUT = 0x14;
    var CFG_SET_BATT_THRESH = 0x15;
    var CFG_SET_ADR_ENABLED = 0x16;

    // Magic byte richiesto da CLEAR_NVS per evitare cancellazioni accidentali
    var CLEAR_NVS_MAGIC = 0xA5;

    // FPort assegnati per convenzione (allineati col firmware)
    var FPORT_ACTION = 10;
    var FPORT_CONFIG = 20;

    /**
     * Helper interno: costruisce l'oggetto {data, fPort} dai bytes.
     * Deduce il fPort dal primo byte usando le stesse regole del codec:
     *   0x01-0x07 -> FPort 10 (azioni)
     *   0x11-0x16 -> FPort 20 (configurazione)
     */
    function wrap(bytes) {
        var first = bytes[0];
        var fPort;
        if (first >= 0x01 && first <= 0x07) {
            fPort = FPORT_ACTION;
        } else if (first >= 0x11 && first <= 0x16) {
            fPort = FPORT_CONFIG;
        } else {
            throw new Error('Byte comando fuori range noto: 0x' + first.toString(16));
        }
        return {
            data: bytesToBase64(bytes),
            fPort: fPort
        };
    }

    // -------------------------------------------------------------
    // FPort 10 - Comandi di azione (one-shot, non persistenti)
    // -------------------------------------------------------------

    /** Riavvia il device via ESP.restart(). */
    function encodeReboot() {
        return wrap([CMD_REBOOT]);
    }

    /** LED lampeggia 10 volte (blink singolo one-shot). */
    function encodeIdentify() {
        return wrap([CMD_IDENTIFY]);
    }

    /** Attiva modalita' identify persistente (blink continuo). */
    function encodeIdentifyOn() {
        return wrap([CMD_IDENTIFY_ON]);
    }

    /** Disattiva modalita' identify persistente. */
    function encodeIdentifyOff() {
        return wrap([CMD_IDENTIFY_OFF]);
    }

    /** Salta lo sleep normale, prossimo TX dopo 2s. */
    function encodeForceTxNow() {
        return wrap([CMD_FORCE_TX_NOW]);
    }

    /**
     * Cancella la NVS namespace 'lora' e riavvia. Richiede magic byte 0xA5
     * per evitare cancellazioni accidentali. Il byte magic e' aggiunto
     * automaticamente da questa funzione.
     */
    function encodeClearNvs() {
        return wrap([CMD_CLEAR_NVS, CLEAR_NVS_MAGIC]);
    }

    /**
     * Richiede al device di inviare il payload state 0x43 nel prossimo TX.
     * Utile per popolare il form di configurazione senza aspettare eventi.
     */
    function encodeGetState() {
        return wrap([CMD_GET_STATE]);
    }

    // -------------------------------------------------------------
    // FPort 20 - Configurazione persistente in NVS
    // -------------------------------------------------------------

    /**
     * Imposta l'intervallo TX. Preset 0-5:
     *   0=10s, 1=20s, 2=1min, 3=5min, 4=10min, 5=30min
     */
    function encodeSetTxInterval(preset) {
        validateRange(preset, 0, 5, 'preset');
        return wrap([CFG_SET_TX_INTERVAL, preset]);
    }

    /** Imposta lo Spreading Factor LoRa. Range 7-12. */
    function encodeSetLorawanSf(sf) {
        validateRange(sf, 7, 12, 'sf');
        return wrap([CFG_SET_LORAWAN_SF, sf]);
    }

    /** Imposta la potenza TX in dBm. Range 2-14. */
    function encodeSetTxPower(dbm) {
        validateRange(dbm, 2, 14, 'dbm');
        return wrap([CFG_SET_TX_POWER, dbm]);
    }

    /** Imposta il timeout di attesa fix GPS in secondi. Range 10-300. */
    function encodeSetGpsTimeout(seconds) {
        validateRange(seconds, 10, 300, 'seconds');
        // uint16 little-endian
        var lo = seconds & 0xFF;
        var hi = (seconds >> 8) & 0xFF;
        return wrap([CFG_SET_GPS_TIMEOUT, lo, hi]);
    }

    /**
     * Imposta le soglie batteria: emergency e recovery in mV.
     * Regole: 2500 <= em <= 4200, 2500 <= rec <= 4200, rec > em.
     */
    function encodeSetBattThresholds(emergencyMv, recoveryMv) {
        validateRange(emergencyMv, 2500, 4200, 'emergencyMv');
        validateRange(recoveryMv, 2500, 4200, 'recoveryMv');
        if (recoveryMv <= emergencyMv) {
            throw new Error('recoveryMv (' + recoveryMv + ') deve essere > emergencyMv (' + emergencyMv + ')');
        }
        // Due uint16 little-endian consecutivi
        var em_lo = emergencyMv & 0xFF;
        var em_hi = (emergencyMv >> 8) & 0xFF;
        var rc_lo = recoveryMv & 0xFF;
        var rc_hi = (recoveryMv >> 8) & 0xFF;
        return wrap([CFG_SET_BATT_THRESH, em_lo, em_hi, rc_lo, rc_hi]);
    }

    /**
     * Abilita/disabilita ADR. Solo effettivo se il firmware e' USE_OTAA=1;
     * in ABP il valore viene salvato in NVS ma ignorato dal codice.
     */
    function encodeSetAdrEnabled(enabled) {
        var v = enabled ? 1 : 0;
        return wrap([CFG_SET_ADR_ENABLED, v]);
    }

    // -------------------------------------------------------------
    // Esportazione API
    // -------------------------------------------------------------

    var api = {
        // FPort 10 - azioni
        encodeReboot:            encodeReboot,
        encodeIdentify:          encodeIdentify,
        encodeIdentifyOn:        encodeIdentifyOn,
        encodeIdentifyOff:       encodeIdentifyOff,
        encodeForceTxNow:        encodeForceTxNow,
        encodeClearNvs:          encodeClearNvs,
        encodeGetState:          encodeGetState,

        // FPort 20 - configurazione
        encodeSetTxInterval:     encodeSetTxInterval,
        encodeSetLorawanSf:      encodeSetLorawanSf,
        encodeSetTxPower:        encodeSetTxPower,
        encodeSetGpsTimeout:     encodeSetGpsTimeout,
        encodeSetBattThresholds: encodeSetBattThresholds,
        encodeSetAdrEnabled:     encodeSetAdrEnabled,

        // Costanti utili
        TX_INTERVAL_PRESETS: TX_INTERVAL_PRESETS,

        // Utility esposta per testing
        _bytesToBase64: bytesToBase64
    };

    // Esporta per browser (window.MqttDownlink) e Node.js (module.exports)
    if (typeof module !== 'undefined' && module.exports) {
        module.exports = api;
    } else {
        global.MqttDownlink = api;
    }

})(typeof window !== 'undefined' ? window : this);
