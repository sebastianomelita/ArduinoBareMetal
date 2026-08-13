// ChirpStack Codec - Marconi Cloud Sensori Ambientali
// ================================================================
//
// UPLINK (device → server):
//   Schema 0x41 (SDS011 + BME280) - 25 byte su FPort 1
//   Schema 0x42 (SCD41 + L76K + batteria) - 32 byte su FPort 1
//   Output: campo 'object' del JSON MQTT ChirpStack
//
// DOWNLINK (server → device):
//   FPort 10 = comandi di azione (reboot, identify, force_tx, clear_nvs)
//   FPort 20 = configurazione persistente (tx_interval, sf, tx_power, ...)
//   Input: JSON con {"cmd": "<nome>", "value": ...}
//   Output: bytes binari compatti spediti via LoRaWAN
//
// Come installarlo:
//   1. In ChirpStack: Device Profiles → il tuo device profile → Codec
//   2. Payload codec: seleziona "JavaScript functions"
//   3. Incolla questo codice
//   4. Salva
//
// Come inviare un downlink dal client MQTT:
//   Topic:    application/{app_id}/device/{dev_eui}/command/down
//   Payload:  {"fPort":20, "data":{"cmd":"set_tx_interval","value":3}}
//   (nota: 'data' contiene l'oggetto che finisce in encodeDownlink.input.data)
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
    var result = {
        schema_id:    b[0],
        fix_quality:  b[1],
        satellites:   b[2],
        battery_pct:  b[3],
        timestamp:    readU64LE(b, 4),
        co2_ppm:      readU16LE(b, 12),
        temp_c:       readI16LE(b, 14) / 100.0,
        hum_pct:      readU16LE(b, 16) / 100.0,
        vbat_mv:      readU16LE(b, 18),
        alt_m:        readI16LE(b, 28),
        hdop:         readU16LE(b, 30) / 100.0
    };

    // Latitudine/longitudine come oggetto "location" annidato
    // solo se il fix e' valido - evita marker a Null Island (0,0)
    var lat = readI32LE(b, 20) / 1e7;
    var lon = readI32LE(b, 24) / 1e7;
    if (result.fix_quality > 0 && (lat !== 0 || lon !== 0)) {
        result.location = {
            latitude:  lat,
            longitude: lon,
            altitude:  result.alt_m,
            source:    "GPS",
            accuracy:  Math.round(result.hdop * 5)   // HDOP → metri stimati
        };
    }

    return result;
}


// =============================================================
// ENCODE DOWNLINK (server -> device)
// =============================================================
//
// Input JSON:
//   { cmd: "nome_comando", value: <opzionale> }
//
// I comandi disponibili per FPort:
//
// FPort 10 - COMANDI DI AZIONE (one-shot, non persistenti)
//   { cmd: "reboot" }                    -> [0x01]
//   { cmd: "identify" }                  -> [0x02]
//   { cmd: "force_tx_now" }              -> [0x03]
//   { cmd: "clear_nvs" }                 -> [0x04, 0xA5]   (byte magic per confermare)
//
// FPort 20 - CONFIGURAZIONE (persistente in NVS del device)
//   { cmd: "set_tx_interval", value: 0-5 }    -> [0x11, <preset>]
//                                              (0=10s, 1=20s, 2=1min, 3=5min, 4=10min, 5=30min)
//   { cmd: "set_lorawan_sf", value: 7-12 }    -> [0x12, <sf>]
//   { cmd: "set_tx_power", value: 2-14 }      -> [0x13, <dBm>]
//   { cmd: "set_gps_timeout", value: 10-300 } -> [0x14, <sec_LSB>, <sec_MSB>]
//   { cmd: "set_batt_thresholds",
//     value: { emergency_mv: 3100, recovery_mv: 3300 } }
//                                             -> [0x15, <em_LSB>, <em_MSB>, <rec_LSB>, <rec_MSB>]

function encodeDownlink(input) {
    var data = input.data;
    var port = input.fPort;

    if (!data || !data.cmd) {
        return { errors: ["Downlink richiede { cmd: '...' }"] };
    }

    var cmd = String(data.cmd).toLowerCase();

    // ---- FPort 10: comandi di azione ----
    if (port === 10) {
        switch (cmd) {
            case "reboot":
                return { bytes: [0x01], fPort: 10 };

            case "identify":
                return { bytes: [0x02], fPort: 10 };

            case "force_tx_now":
                return { bytes: [0x03], fPort: 10 };

            case "clear_nvs":
                return { bytes: [0x04, 0xA5], fPort: 10 };

            default:
                return { errors: ["Comando FPort 10 sconosciuto: " + cmd] };
        }
    }

    // ---- FPort 20: configurazione persistente ----
    if (port === 20) {
        switch (cmd) {

            case "set_tx_interval":
                if (data.value === undefined || data.value < 0 || data.value > 5) {
                    return { errors: ["set_tx_interval richiede value 0-5"] };
                }
                return { bytes: [0x11, data.value], fPort: 20 };

            case "set_lorawan_sf":
                if (data.value === undefined || data.value < 7 || data.value > 12) {
                    return { errors: ["set_lorawan_sf richiede value 7-12"] };
                }
                return { bytes: [0x12, data.value], fPort: 20 };

            case "set_tx_power":
                if (data.value === undefined || data.value < 2 || data.value > 14) {
                    return { errors: ["set_tx_power richiede value 2-14 dBm"] };
                }
                return { bytes: [0x13, data.value], fPort: 20 };

            case "set_gps_timeout":
                if (data.value === undefined || data.value < 10 || data.value > 300) {
                    return { errors: ["set_gps_timeout richiede value 10-300 secondi"] };
                }
                return {
                    bytes: [0x14, data.value & 0xFF, (data.value >> 8) & 0xFF],
                    fPort: 20
                };

            case "set_batt_thresholds":
                if (!data.value || !data.value.emergency_mv || !data.value.recovery_mv) {
                    return { errors: ["set_batt_thresholds richiede value: {emergency_mv, recovery_mv}"] };
                }
                var em = data.value.emergency_mv;
                var rc = data.value.recovery_mv;
                if (em < 2500 || em > 4200 || rc < 2500 || rc > 4200 || rc <= em) {
                    return { errors: ["thresholds: 2500-4200 mV e recovery > emergency"] };
                }
                return {
                    bytes: [0x15,
                            em & 0xFF, (em >> 8) & 0xFF,
                            rc & 0xFF, (rc >> 8) & 0xFF],
                    fPort: 20
                };

            case "set_adr_enabled":
                if (data.value !== 0 && data.value !== 1 &&
                    data.value !== true && data.value !== false) {
                    return { errors: ["set_adr_enabled richiede value 0/1 o true/false"] };
                }
                var v = (data.value === 1 || data.value === true) ? 1 : 0;
                return { bytes: [0x16, v], fPort: 20 };

            default:
                return { errors: ["Comando FPort 20 sconosciuto: " + cmd] };
        }
    }

    return { errors: ["FPort " + port + " non supportato (usa 10 per azioni, 20 per config)"] };
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
