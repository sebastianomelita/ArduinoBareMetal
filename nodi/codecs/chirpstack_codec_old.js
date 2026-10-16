// ChirpStack Codec - Marconi Cloud Sensori Ambientali
// ================================================================
//
// UPLINK (device → server):
//   Schema 0x41 (SDS011 + BME280)            - 25 byte su FPort 1
//   Schema 0x42 (SCD41 + L76K + batteria)    - 32 byte su FPort 1
//   Schema 0x43 (STATE: config + diagnostica) - 23 byte su FPort 2
//   Output: campo 'object' del JSON MQTT ChirpStack
//
// DOWNLINK (server → device):
//   FPort 10 = comandi di azione (0x01-0x07)
//   FPort 20 = configurazione persistente (0x11-0x19)
//   Input: JSON con campo 'data' = stringa base64 dei bytes
//   Output: bytes decodificati con fPort dedotto dal primo byte
//
// Come installarlo:
//   1. In ChirpStack: Device Profiles → il tuo device profile → Codec
//   2. Payload codec: seleziona "JavaScript functions"
//   3. Incolla questo codice
//   4. Salva
//
// Come inviare un downlink dal client MQTT:
//   Topic:    application/{app_uuid}/device/{dev_eui}/command/down
//   Payload:  {"data": "AQ=="}      // AQ== = base64 di [0x01] = REBOOT
//
// La pagina web di configurazione costruisce i bytes lato client e li
// invia base64-encoded. Il codec deduce il fPort dal primo byte del
// payload (range 0x01-0x07 → FPort 10, range 0x11-0x19 → FPort 20).
//
// ================================================================

// =============================================================
// DECODE UPLINK (device -> server)
// =============================================================
function decodeUplink(input) {
    var bytes = input.bytes;

    if (bytes.length < 1) {
        return { errors: ["Payload vuoto"] };
    }

    var schemaId = bytes[0];

    // Schema 0x41: SDS011 + BME280
    if (schemaId === 0x41) {
        if (bytes.length < 25) {
            return { errors: ["Schema 0x41 richiede 25 byte, ricevuti " + bytes.length] };
        }
        return { data: decodeSchema41(bytes) };
    }

    // Schema 0x42: SCD41 + L76K + batteria
    if (schemaId === 0x42) {
        if (bytes.length < 32) {
            return { errors: ["Schema 0x42 richiede 32 byte, ricevuti " + bytes.length] };
        }
        return { data: decodeSchema42(bytes) };
    }

    // Schema 0x43: state del device (config runtime + diagnostica)
    // Trasmesso su FPort 2, distinto dalle misure su FPort 1.
    // La pagina web di configurazione filtra per payload_type == "state"
    if (schemaId === 0x43) {
        if (bytes.length < 26) {
            return { errors: ["Schema 0x43 richiede 26 byte, ricevuti " + bytes.length] };
        }
        return { data: decodeSchema43(bytes) };
    }

    return {
        warnings: ["Schema sconosciuto: 0x" + schemaId.toString(16)],
        data: { schema_id: schemaId, raw_hex: bytesToHex(bytes) }
    };
}

function decodeSchema41(b) {
    return {
        schema_id:   b[0],
        timestamp:   readU64LE(b, 1),
        pmSensorID:  readU16LE(b, 9),
        pm10:        readU16LE(b, 11) / 10.0,
        pm25:        readU16LE(b, 13) / 10.0,
        eCO2:        readU16LE(b, 15),
        tVOC:        readU16LE(b, 17),
        temp:        readI16LE(b, 19) / 10.0,
        press:       readU32LE(b, 21) / 100.0
    };
}

function decodeSchema42(b) {
    // Valori placeholder per sensori disabilitati (v2, modulo SENSOR DISABLE del firmware)
    // Se il device manda questi valori "sentinel", significa che il sensore
    // e' stato disattivato via downlink (SET_GPS_ENABLED=0 / SET_SCD_ENABLED=0).
    // La dashboard non deve mostrare valori spuri: li restituiamo come null.
    var SENTINEL_U16 = 0xFFFF;
    var SENTINEL_I16 = 0x7FFF;

    var co2Raw    = readU16LE(b, 12);
    var tempRaw   = readI16LE(b, 14);
    var humRaw    = readU16LE(b, 16);
    var hdopRaw   = readU16LE(b, 30);

    var result = {
        schema_id:    b[0],
        fix_quality:  b[1],
        satellites:   b[2],
        battery_pct:  b[3],
        timestamp:    readU64LE(b, 4),
        co2_ppm:      (co2Raw  === SENTINEL_U16) ? null : co2Raw,
        temp_c:       (tempRaw === SENTINEL_I16) ? null : (tempRaw / 100.0),
        hum_pct:      (humRaw  === SENTINEL_U16) ? null : (humRaw / 100.0),
        vbat_mv:      readU16LE(b, 18),
        alt_m:        readI16LE(b, 28),
        hdop:         (hdopRaw === SENTINEL_U16) ? null : (hdopRaw / 100.0),
        // Marker semantici per la dashboard: sensori disabilitati o dati "stale"
        scd_disabled: (co2Raw === SENTINEL_U16),
        gps_disabled: (hdopRaw === SENTINEL_U16 && b[1] === 0 && b[2] === 0),
        gps_stale:    (hdopRaw === 9999)   // ultima posizione nota (modulo GPS SKIP)
    };

    // Latitudine/longitudine come oggetto "location" annidato
    // solo se il fix e' valido - evita marker a Null Island (0,0)
    // e non mostra location se GPS disabilitato via downlink
    var lat = readI32LE(b, 20) / 1e7;
    var lon = readI32LE(b, 24) / 1e7;
    if (!result.gps_disabled && (lat !== 0 || lon !== 0)) {
        result.location = {
            latitude:  lat,
            longitude: lon,
            altitude:  result.alt_m,
            source:    result.gps_stale ? "GPS (last known)" : "GPS",
            accuracy:  result.hdop !== null ? Math.round(result.hdop * 5) : null
        };
    }

    return result;
}

// Schema 0x43: STATE del device (26 byte)
// Trasmesso su FPort 2 su richiesta (get_state) o automaticamente:
//   - Al primo boot dopo power cycle
//   - Dopo un cambio config via downlink FPort 20
//
// Layout:
//   b[0]       schema_id (0x43)
//   b[1..4]    bootCount           uint32 LE
//   b[5..8]    uptime_s            uint32 LE (wall-clock dal power-on)
//   b[9]       battery_pct         uint8
//   b[10]      cfg_tx_interval     uint8 (preset 0-10, 10 = STORAGE MODE)
//   b[11]      cfg_lorawan_sf      uint8 (7-12)
//   b[12]      cfg_tx_power        uint8 (dBm 2-14)
//   b[13..14]  cfg_gps_timeout_s   uint16 LE
//   b[15..16]  cfg_vbat_emerg_mv   uint16 LE
//   b[17..18]  cfg_vbat_recov_mv   uint16 LE
//   b[19]      cfg_adr             uint8 (0/1)
//   b[20]      feature_flags       uint8 bit-packed
//   b[21]      fw_version          uint8
//   b[22]      reset_reason        uint8 (esp_reset_reason)
//   b[23]      cfg_gps_enabled     uint8 (0/1) - modulo SENSOR DISABLE
//   b[24]      cfg_gps_skip_cycles uint8 (0-255) - modulo GPS SKIP
//   b[25]      cfg_scd_enabled     uint8 (0/1) - modulo SENSOR DISABLE
function decodeSchema43(b) {
    var flags = b[20];

    // Mappa TX_INTERVAL_PRESET a stringa human-readable
    // (deve corrispondere all'array TX_INTERVAL_SECONDS nel firmware)
    var txIntervals = ["10s", "20s", "1min", "2min", "5min", "10min", "30min", "1h", "1d", "1w", "STORAGE"];
    var preset = b[10];
    var txIntervalLabel = (preset < txIntervals.length) ? txIntervals[preset] : "?";

    // Mappa esp_reset_reason() a stringa (enum ESP-IDF)
    var resetReasons = {
        0: "UNKNOWN", 1: "POWERON", 2: "EXT", 3: "SW",
        4: "PANIC", 5: "INT_WDT", 6: "TASK_WDT", 7: "WDT",
        8: "DEEPSLEEP", 9: "BROWNOUT", 10: "SDIO", 11: "USB",
        12: "JTAG", 13: "EFUSE", 14: "PWR_GLITCH", 15: "CPU_LOCKUP"
    };
    var resetReasonCode = b[22];
    var resetReasonLabel = resetReasons[resetReasonCode] || "?";

    return {
        schema_id:              b[0],
        payload_type:           "state",       // discriminatore per il frontend
        boot_count:             readU32LE(b, 1),
        uptime_s:               readU32LE(b, 5),
        battery_pct:            b[9],
        cfg_tx_interval_preset: preset,
        cfg_tx_interval_label:  txIntervalLabel,
        cfg_lorawan_sf:         b[11],
        cfg_tx_power_dbm:       b[12],
        cfg_gps_timeout_s:      readU16LE(b, 13),
        cfg_vbat_emergency_mv:  readU16LE(b, 15),
        cfg_vbat_recovery_mv:   readU16LE(b, 17),
        cfg_adr:                b[19] !== 0,
        // Feature flags come oggetto (piu' leggibile del byte crudo).
        // Sono compile-time: NON modificabili da downlink, informativi per l'UI
        // (la pagina web puo' disabilitare campi non applicabili al firmware corrente)
        features: {
            use_otaa:            (flags & 0x01) !== 0,
            nvs_persistence:     (flags & 0x02) !== 0,
            downlink_handler:    (flags & 0x04) !== 0,
            watchdog:            (flags & 0x08) !== 0,
            battery_protection:  (flags & 0x10) !== 0,
            debug_no_deep_sleep: (flags & 0x20) !== 0,
            storage_mode:        (flags & 0x40) !== 0,   // preset 10 disponibile
            triple_rst_reset:    (flags & 0x80) !== 0    // emergency unlock disponibile
        },
        feature_flags_raw:      flags,          // byte crudo per debug
        fw_version:             b[21],
        reset_reason_code:      resetReasonCode,
        reset_reason:           resetReasonLabel,
        // ---- Estensione v2: features risparmio energetico (byte 23-25) ----
        cfg_gps_enabled:        b[23] !== 0,
        cfg_gps_skip_cycles:    b[24],
        cfg_scd_enabled:        b[25] !== 0
    };
}


// =============================================================
// ENCODE DOWNLINK (server -> device)
// =============================================================
//
// Input JSON dal MQTT publisher:
//   { data: "base64_string" }
//
// La pagina web di configurazione costruisce i bytes lato client e li
// invia base64-encoded. Il codec deduce il fPort dal primo byte del payload.
//
// FPort 10 - COMANDI DI AZIONE (byte 0x01-0x07, one-shot, non persistenti)
//   [0x01]                                          REBOOT
//   [0x02]                                          IDENTIFY (blink one-shot)
//   [0x03]                                          FORCE_TX_NOW
//   [0x04, 0xA5]                                    CLEAR_NVS (magic 0xA5 obbligatorio)
//   [0x05]                                          IDENTIFY_ON (persistente)
//   [0x06]                                          IDENTIFY_OFF
//   [0x07]                                          GET_STATE (richiede payload state 0x43)
//
// FPort 20 - CONFIGURAZIONE (byte 0x11-0x19, persistente in NVS del device)
//   [0x11, preset]                                  SET_TX_INTERVAL (preset 0-10, 10=STORAGE MODE)
//   [0x12, sf]                                      SET_LORAWAN_SF (7-12)
//   [0x13, dbm]                                     SET_TX_POWER (2-14)
//   [0x14, sec_lsb, sec_msb]                        SET_GPS_TIMEOUT (10-300 s)
//   [0x15, em_lsb, em_msb, rec_lsb, rec_msb]        SET_BATT_THRESHOLDS (mV)
//   [0x16, 0|1]                                     SET_ADR_ENABLED
//   [0x17, N]                                       SET_GPS_SKIP (fix ogni N+1 cicli)
//   [0x18, 0|1]                                     SET_GPS_ENABLED
//   [0x19, 0|1]                                     SET_SCD_ENABLED
//
// Esempi di generazione base64 lato client (JavaScript):
//   btoa(String.fromCharCode(0x01))            → "AQ=="   REBOOT
//   btoa(String.fromCharCode(0x07))            → "Bw=="   GET_STATE
//   btoa(String.fromCharCode(0x11, 4))         → "EQQ="   SET_TX_INTERVAL=5min (preset 4)
//   btoa(String.fromCharCode(0x11, 8))         → "EQg="   SET_TX_INTERVAL=1 giorno (preset 8)
//   btoa(String.fromCharCode(0x11, 9))         → "EQk="   SET_TX_INTERVAL=1 settimana (preset 9)
//   btoa(String.fromCharCode(0x11, 10))        → "EQo="   SET_TX_INTERVAL=STORAGE MODE (preset 10)
//   btoa(String.fromCharCode(0x14, 0x3C, 0))   → "FDwA"   SET_GPS_TIMEOUT=60s
//   btoa(String.fromCharCode(0x17, 9))         → "Fwk="   SET_GPS_SKIP=9 (fix ogni 10 cicli)
//   btoa(String.fromCharCode(0x18, 0))         → "GAA="   SET_GPS_ENABLED=false (disattiva GPS)
//
// Se il campo data non e' una stringa base64 valida, il codec restituisce errore.

function encodeDownlink(input) {
    var data = input.data;

    // Controllo 1: data deve esistere
    if (!data) {
        return { errors: ["missing data"] };
    }

    // Controllo 2: data deve essere una stringa
    if (typeof data !== "string") {
        return { errors: ["data must be a base64 string, got " + typeof data] };
    }

    // Controllo 3: deve essere base64 valida (almeno 2 caratteri, no caratteri strani)
    var base64Regex = /^[A-Za-z0-9+/]+=*$/;
    if (!base64Regex.test(data)) {
        return { errors: ["invalid base64 string: " + data] };
    }

    // Decodifica base64 → bytes
    var bytes = base64ToBytes(data);

    // Controllo 4: non può essere vuota
    if (bytes.length === 0) {
        return { errors: ["empty payload"] };
    }

    // Estrai il fPort dal primo byte (opzionale, o usa fPort fisso)
    // Comandi di AZIONE (FPort 10): primo byte 0x01-0x07
    //   0x01 REBOOT           0x02 IDENTIFY (blink one-shot)
    //   0x03 FORCE_TX_NOW     0x04 CLEAR_NVS (richiede magic 0xA5)
    //   0x05 IDENTIFY_ON      0x06 IDENTIFY_OFF
    //   0x07 GET_STATE        (richiede invio payload state 0x43)
    // Comandi di CONFIG (FPort 20): primo byte 0x11-0x19
    //   0x11 SET_TX_INTERVAL  0x12 SET_LORAWAN_SF   0x13 SET_TX_POWER
    //   0x14 SET_GPS_TIMEOUT  0x15 SET_BATT_THRESH  0x16 SET_ADR_ENABLED
    //   0x17 SET_GPS_SKIP     0x18 SET_GPS_ENABLED  0x19 SET_SCD_ENABLED
    var firstByte = bytes[0];
    var fPort;

    if (firstByte >= 0x01 && firstByte <= 0x07) {
        fPort = 10;
    } else if (firstByte >= 0x11 && firstByte <= 0x19) {
        fPort = 20;
    } else {
        // Byte fuori dai range noti: default a 20 (config).
        // Il device rigettera' comandi sconosciuti con un warning nel log.
        fPort = 20;
    }

    // Oppure: se vuoi un fPort fisso, decommenta questa riga e commenta sopra
    // fPort = 20;  // fisso a 20

    return {
        bytes: bytes,
        fPort: fPort
    };
}

// =============================================================
// Utility: base64 → bytes
// =============================================================
function base64ToBytes(base64) {
    // Decodifica base64 in stringa binaria
    var binaryString = atob(base64);
    var bytes = new Array(binaryString.length);
    for (var i = 0; i < binaryString.length; i++) {
        bytes[i] = binaryString.charCodeAt(i);
    }
    return bytes;
}

// =============================================================
// UTILITY - lettura little-endian
// =============================================================
function readU16LE(b, off) {
    return (b[off]) | (b[off+1] << 8);
}
function readI16LE(b, off) {
    var v = readU16LE(b, off);
    return v >= 0x8000 ? v - 0x10000 : v;
}
function readU32LE(b, off) {
    return (b[off]) | (b[off+1] << 8) | (b[off+2] << 16) | (b[off+3] << 24);
}
function readI32LE(b, off) {
    var v = readU32LE(b, off);
    return v >= 0x80000000 ? v - 0x100000000 : v;
}
function readU64LE(b, off) {
    // JavaScript non ha uint64. Ricostruiamo come Number (precisione fino a 2^53).
    // Per timestamp epoch UTC va bene fino all'anno 285.428.751 dC.
    var lo = readU32LE(b, off);
    var hi = readU32LE(b, off + 4);
    return hi * 4294967296 + lo;
}
function bytesToHex(b) {
    var s = "";
    for (var i = 0; i < b.length; i++) {
        var h = b[i].toString(16);
        if (h.length < 2) h = "0" + h;
        s += h;
    }
    return s;
}
