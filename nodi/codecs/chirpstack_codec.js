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
//   { cmd: "identify" }                  -> [0x02]   (blink singolo one-shot)
//   { cmd: "identify_on" }               -> [0x05]   (identify persistente ON)
//   { cmd: "identify_off" }              -> [0x06]   (identify persistente OFF)
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

// =============================================================
// ENCODE DOWNLINK - ACCETTA SOLO BASE64
// =============================================================
//
// Input: { data: "base64_string" }
// Output: bytes decodificati dalla base64
//
// Se il campo data NON è una stringa base64 valida, restituisce errore
// =============================================================

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
    // Per semplicità, usiamo fPort=10 se il primo byte è 0x01-0x06, altrimenti fPort=20
    var firstByte = bytes[0];
    var fPort;

    // Comandi di azione (FPort 10)
    if (firstByte === 0x01 || firstByte === 0x02 || firstByte === 0x03 ||
        firstByte === 0x04 || firstByte === 0x05 || firstByte === 0x06) {
        fPort = 10;
    } else {
        // Comandi di configurazione (FPort 20)
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
