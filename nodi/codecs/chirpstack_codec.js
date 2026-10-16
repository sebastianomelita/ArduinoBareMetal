// ChirpStack Codec - Marconi Cloud Sensori Ambientali
// ================================================================
//
// UPLINK (device → server):
//   Schema 0x41 (SDS011 + BME280)            - 25 byte su FPort 1
//   Schema 0x42 (SCD41 + L76K + batteria)    - 32 byte su FPort 1
//   Schema 0x43 (STATE: config + diagnostica) - 26 byte (31 con l'upgrade) su FPort 2
//   Schema 0x44 (MULTISENSORE, a blocchi)     - 10-43 byte su FPort 1
//               SCD41, L76K, SDS011, CCS811, MPL3115A2: ogni sensore ha un
//               blocco solo se e' presente nella maschera sensor_mask
//   Output: campo 'object' del JSON MQTT ChirpStack
//
// DOWNLINK (server → device):
//   FPort 10 = comandi di azione (0x01-0x07)
//   FPort 20 = configurazione persistente (0x11-0x1D)
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
// payload (range 0x01-0x07 → FPort 10, range 0x11-0x1D → FPort 20).
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

    // Schema 0x44: upgrade multisensore, lunghezza variabile (10-43 byte)
    if (schemaId === 0x44) {
        return decodeSchema44(bytes);
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
        cfg_scd_enabled:        b[25] !== 0,
        // ---- Estensione v3: upgrade multisensore (byte 26-30, solo FW >= 2) ----
        cfg_sds_enabled:        b.length >= 31 ? b[26] !== 0 : null,
        cfg_ccs_enabled:        b.length >= 31 ? b[27] !== 0 : null,
        cfg_mpl_enabled:        b.length >= 31 ? b[28] !== 0 : null,
        cfg_sds_warmup_s:       b.length >= 31 ? b[29] : null,
        sensors_compiled:       b.length >= 31 ? sensorMaskToObject(b[30]) : null
    };
}

// =============================================================
// Schema 0x44: MISURE MULTISENSORE (lunghezza variabile)
// =============================================================
//
// Intestazione (10 byte):
//   b[0]      schema_id   0x44
//   b[1]      sensor_mask bit0 SCD41, bit1 GPS, bit2 SDS011, bit3 CCS811, bit4 MPL3115A2
//   b[2]      flags       bit0 timestamp = epoch GPS, bit1 posizione stale,
//                         bit2 CCS811 in riscaldamento (primi 20 minuti),
//                         bit3 umidita' alta durante la misura PM (PM sovrastimati),
//                         bit4 SCD41 compensato con la pressione dell'MPL3115A2,
//                         bit5 SCD41 compensato con la pressione stimata dalla quota GPS
//   b[3]      battery_pct uint8
//   b[4..5]   vbat_mv     uint16 LE
//   b[6..9]   timestamp   uint32 LE (epoch UTC o secondi dal boot)
// Poi un blocco per ogni bit della maschera, sempre in quest'ordine:
//   SCD41      6 byte  co2_ppm u16, temp_c100 i16, hum_pct100 u16
//   GPS       14 byte  fix u8, sat u8, lat_e7 i32, lon_e7 i32, alt_m i16, hdop_x100 u16
//   SDS011     4 byte  pm25_x10 u16, pm10_x10 u16
//   CCS811     5 byte  eco2_ppm u16, tvoc_ppb u16, error_id u8
//   MPL3115A2  4 byte  press_hpa10 u16, temp_c100 i16
// Sentinelle: 0xFFFF (u16) e 0x7FFF (i16) = lettura non riuscita -> null.
// Un sensore assente non ha blocco: i suoi campi valgono null.
//
// I nomi dei campi comuni (co2_ppm, temp_c, hum_pct, battery_pct, vbat_mv,
// location, ...) sono gli stessi dello schema 0x42: le dashboard esistenti
// continuano a funzionare e trovano in piu' i campi dei nuovi sensori.
var BLOCK_LEN_44 = [6, 14, 4, 5, 4];   // nell'ordine dei bit 0..4

// ---- Correzioni calcolate nel codec (modificabili senza riprogrammare il nodo)
// Crescita igroscopica del particolato (kappa-Koehler, per la massa):
//   PM_secco = PM_misurato / f,   f = 1 + (KAPPA / DENSITA') * a / (1 - a),
//   a = RH/100   (Crilley et al., 2018)
// KAPPA dipende dal tipo di aerosol: ~0.2 urbano/traffico, ~0.4 con molti
// solfati/nitrati, fino a ~0.6 vicino al mare. Si tara confrontando il nodo
// con una centralina ARPA vicina nei giorni umidi.
var PM_KAPPA = 0.4;
var PM_DENSITY = 1.65;       // densita' delle particelle secche, g/cm3
var PM_RH_MAX = 95;          // oltre, la formula esplode: si satura qui
// Quota della stazione per riportare la pressione al livello del mare.
// null = usa la quota del GPS (solo se il fix e' attuale, non "stale").
var STATION_ALTITUDE_M = null;

function pmHumidityFactor(rh) {
    if (rh === null || rh === undefined) return null;
    var a = Math.min(Math.max(rh, 0), PM_RH_MAX) / 100;
    return 1 + (PM_KAPPA / PM_DENSITY) * a / (1 - a);
}

// Pressione ridotta al livello del mare (atmosfera standard):
//   p0 = p / (1 - h/44330)^5.255
function seaLevelPressure(pHpa, altM) {
    if (pHpa === null || altM === null || altM === undefined) return null;
    return pHpa / Math.pow(1 - altM / 44330, 5.255);
}

function round1(v) { return v === null ? null : Math.round(v * 10) / 10; }

function sensorMaskToObject(m) {
    return {
        scd41:     (m & 0x01) !== 0,
        gps:       (m & 0x02) !== 0,
        sds011:    (m & 0x04) !== 0,
        ccs811:    (m & 0x08) !== 0,
        mpl3115a2: (m & 0x10) !== 0
    };
}

function decodeSchema44(b) {
    if (b.length < 10) {
        return { errors: ["Schema 0x44 richiede almeno 10 byte, ricevuti " + b.length] };
    }
    var mask  = b[1];
    var flags = b[2];

    // Controllo di coerenza: la lunghezza deve corrispondere alla maschera
    var expected = 10;
    for (var k = 0; k < BLOCK_LEN_44.length; k++) {
        if (mask & (1 << k)) expected += BLOCK_LEN_44[k];
    }
    if (b.length < expected) {
        return { errors: ["Schema 0x44: la maschera 0x" + mask.toString(16) +
                          " richiede " + expected + " byte, ricevuti " + b.length] };
    }

    var U16 = 0xFFFF, I16 = 0x7FFF;
    function u16(off, scale) { var v = readU16LE(b, off); return v === U16 ? null : (scale ? v / scale : v); }
    function i16(off, scale) { var v = readI16LE(b, off); return v === I16 ? null : (scale ? v / scale : v); }

    var r = {
        schema_id:     b[0],
        payload_type:  "measure",
        sensors:       sensorMaskToObject(mask),
        sensor_mask:   mask,
        battery_pct:   b[3],
        vbat_mv:       readU16LE(b, 4),
        timestamp:     readU32LE(b, 6),
        timestamp_is_epoch: (flags & 0x01) !== 0,
        // valori di default: sensore assente
        co2_ppm: null, temp_c: null, hum_pct: null,
        fix_quality: null, satellites: null, alt_m: null, hdop: null,
        gps_stale: (flags & 0x02) !== 0,
        pm25: null, pm10: null,
        eco2_ppm: null, tvoc_ppb: null, ccs_error: null,
        ccs_warmup: (flags & 0x04) !== 0,
        pressure_hpa: null, mpl_temp_c: null
    };

    var o = 10;
    if (mask & 0x01) {            // SCD41
        r.co2_ppm = u16(o);
        r.temp_c  = i16(o + 2, 100);
        r.hum_pct = u16(o + 4, 100);
        o += 6;
    }
    if (mask & 0x02) {            // GPS
        r.fix_quality = b[o];
        r.satellites  = b[o + 1];
        var lat  = readI32LE(b, o + 2) / 1e7;
        var lon  = readI32LE(b, o + 6) / 1e7;
        r.alt_m  = readI16LE(b, o + 10);
        var hd   = readU16LE(b, o + 12);
        r.hdop   = (hd === U16 || hd === 9999) ? null : hd / 100;
        if (hd !== U16 && (lat !== 0 || lon !== 0)) {
            r.location = {
                latitude:  lat,
                longitude: lon,
                altitude:  r.alt_m,
                source:    r.gps_stale ? "GPS (last known)" : "GPS",
                accuracy:  r.hdop !== null ? Math.round(r.hdop * 5) : null
            };
        }
        o += 14;
    }
    if (mask & 0x04) {            // SDS011
        r.pm25 = u16(o, 10);
        r.pm10 = u16(o + 2, 10);
        o += 4;
    }
    if (mask & 0x08) {            // CCS811
        r.eco2_ppm  = u16(o);
        r.tvoc_ppb  = u16(o + 2);
        r.ccs_error = b[o + 4];
        o += 5;
    }
    if (mask & 0x10) {            // MPL3115A2
        r.pressure_hpa = u16(o, 10);
        r.mpl_temp_c   = i16(o + 2, 100);
        o += 4;
    }

    // ---- Correzioni incrociate
    // SCD41: quale pressione ha usato il nodo per compensare la CO2
    r.scd_pressure_comp = (flags & 0x10) ? "MPL3115A2" : ((flags & 0x20) ? "GPS" : null);
    // SDS011: flag di umidita' alta e PM corretti con l'umidita' del SCD41
    r.pm_humidity_warning = (flags & 0x08) !== 0;
    var f = pmHumidityFactor(r.hum_pct);
    r.pm_rh_factor = f === null ? null : Math.round(f * 100) / 100;
    r.pm25_rh_corr = (r.pm25 !== null && f !== null) ? round1(r.pm25 / f) : null;
    r.pm10_rh_corr = (r.pm10 !== null && f !== null) ? round1(r.pm10 / f) : null;
    // MPL3115A2: pressione al livello del mare (confrontabile con i bollettini)
    var h = STATION_ALTITUDE_M;
    if (h === null && r.alt_m !== null && r.fix_quality > 0 && !r.gps_stale) h = r.alt_m;
    r.pressure_sea_hpa = round1(seaLevelPressure(r.pressure_hpa, h));

    // Compatibilita' con le dashboard dello schema 0x42
    r.scd_disabled = (mask & 0x01) === 0;
    r.gps_disabled = (mask & 0x02) === 0;
    return { data: r };
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
// FPort 20 - CONFIGURAZIONE (byte 0x11-0x1D, persistente in NVS del device)
//   [0x11, preset]                                  SET_TX_INTERVAL (preset 0-10, 10=STORAGE MODE)
//   [0x12, sf]                                      SET_LORAWAN_SF (7-12)
//   [0x13, dbm]                                     SET_TX_POWER (2-14)
//   [0x14, sec_lsb, sec_msb]                        SET_GPS_TIMEOUT (10-300 s)
//   [0x15, em_lsb, em_msb, rec_lsb, rec_msb]        SET_BATT_THRESHOLDS (mV)
//   [0x16, 0|1]                                     SET_ADR_ENABLED
//   [0x17, N]                                       SET_GPS_SKIP (fix ogni N+1 cicli)
//   [0x18, 0|1]                                     SET_GPS_ENABLED
//   [0x19, 0|1]                                     SET_SCD_ENABLED
//   [0x1A, 0|1]                                     SET_SDS_ENABLED   (upgrade multisensore)
//   [0x1B, 0|1]                                     SET_CCS_ENABLED   (upgrade multisensore)
//   [0x1C, 0|1]                                     SET_MPL_ENABLED   (upgrade multisensore)
//   [0x1D, sec]                                     SET_SDS_WARMUP    (5-120 s, upgrade multisensore)
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
    // Comandi di CONFIG (FPort 20): primo byte 0x11-0x1D
    //   0x11 SET_TX_INTERVAL  0x12 SET_LORAWAN_SF   0x13 SET_TX_POWER
    //   0x14 SET_GPS_TIMEOUT  0x15 SET_BATT_THRESH  0x16 SET_ADR_ENABLED
    //   0x17 SET_GPS_SKIP     0x18 SET_GPS_ENABLED  0x19 SET_SCD_ENABLED
    //   0x1A SET_SDS_ENABLED  0x1B SET_CCS_ENABLED  0x1C SET_MPL_ENABLED
    //   0x1D SET_SDS_WARMUP
    var firstByte = bytes[0];
    var fPort;

    if (firstByte >= 0x01 && firstByte <= 0x07) {
        fPort = 10;
    } else if (firstByte >= 0x11 && firstByte <= 0x1D) {
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
    // ">>> 0" rende il risultato senza segno: senza, i valori >= 2^31
    // (per esempio un timestamp lontano) diventerebbero negativi.
    return ((b[off]) | (b[off+1] << 8) | (b[off+2] << 16) | (b[off+3] << 24)) >>> 0;
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
