#!/bin/bash
# =============================================================================
# test_downlinks.sh - Comandi di test per downlink LoRaWAN via broker MQTT
# =============================================================================
#
# Uso:
#   ./test_downlinks.sh <comando> [argomenti]
#
# Esempi:
#   ./test_downlinks.sh reboot
#   ./test_downlinks.sh identify
#   ./test_downlinks.sh set_tx_interval 0    # 10s per test veloci
#   ./test_downlinks.sh set_tx_interval 2    # 1min per produzione
#   ./test_downlinks.sh set_tx_power 2       # potenza minima
#   ./test_downlinks.sh set_batt_thresh 3000 3200
#   ./test_downlinks.sh sub_up               # ascolta uplink in tempo reale
#   ./test_downlinks.sh sub_down             # ascolta downlink in transito
#   ./test_downlinks.sh help                 # mostra tutti i comandi
#
# I comandi vengono pubblicati sul broker centrale con il prefisso "edges/<EDGE>/",
# il bridge Mosquitto li riporta al RPi ChirpStack che li accoda e li invia al
# device al prossimo uplink.
#
# =============================================================================
# CONFIGURAZIONE - adatta ai tuoi valori
# =============================================================================

# Broker MQTT centrale (dove pubblichi i comandi)
BROKER_HOST="${BROKER_HOST:-hub.mia-rete.local}"
BROKER_PORT="${BROKER_PORT:-1883}"

# Autenticazione MQTT (opzionale - vuoto se allow_anonymous)
MQTT_USER="${MQTT_USER:-}"
MQTT_PASS="${MQTT_PASS:-}"

# Identificativo edge/sito - usato come prefisso topic
EDGE_ID="${EDGE_ID:-serra-01}"

# ChirpStack application ID - trovalo nell'URL ChirpStack UI:
#   http://<IP>:8080/#/tenants/.../applications/<APP_ID>
APP_ID="${APP_ID:-1}"

# DevEUI del device (lowercase, senza trattini)
DEV_EUI="${DEV_EUI:-26a160fffe6e86bc}"

# Modalità comunicazione:
#   "bridge"  = pubblica sul broker centrale, il bridge porta al RPi (produzione)
#   "direct"  = pubblica direttamente sul RPi (test/debug locale)
MODE="${MODE:-bridge}"

# Per mode=direct, IP del RPi ChirpStack
DIRECT_HOST="${DIRECT_HOST:-192.168.1.100}"


# =============================================================================
# COSTRUZIONE TOPIC (in base a MODE)
# =============================================================================

if [ "$MODE" = "direct" ]; then
    PUB_HOST="$DIRECT_HOST"
    TOPIC_PREFIX=""
else
    PUB_HOST="$BROKER_HOST"
    TOPIC_PREFIX="edges/${EDGE_ID}/"
fi

TOPIC_DOWN="${TOPIC_PREFIX}application/${APP_ID}/device/${DEV_EUI}/command/down"
TOPIC_UP="${TOPIC_PREFIX}application/${APP_ID}/device/${DEV_EUI}/event/up"


# =============================================================================
# HELPER FUNCTIONS
# =============================================================================

# Costruisce i flag di autenticazione per mosquitto_pub/sub
mqtt_auth_flags() {
    local flags=""
    if [ -n "$MQTT_USER" ]; then
        flags="-u $MQTT_USER"
        if [ -n "$MQTT_PASS" ]; then
            flags="$flags -P $MQTT_PASS"
        fi
    fi
    echo "$flags"
}

# Pubblica un comando downlink
publish_downlink() {
    local payload="$1"
    local desc="$2"

    echo "─────────────────────────────────────────────────────────────"
    echo "  Comando: $desc"
    echo "  Broker : $PUB_HOST:$BROKER_PORT"
    echo "  Topic  : $TOPIC_DOWN"
    echo "  Payload: $payload"
    echo "─────────────────────────────────────────────────────────────"

    local auth
    auth=$(mqtt_auth_flags)

    if mosquitto_pub -h "$PUB_HOST" -p "$BROKER_PORT" $auth \
                     -t "$TOPIC_DOWN" -m "$payload"; then
        echo "  ✓ Comando pubblicato"
        echo "  → Arrivera' al device al prossimo uplink (classe A)"
    else
        echo "  ✗ Errore nella pubblicazione"
        return 1
    fi
    echo
}


# =============================================================================
# COMANDI DI AZIONE (FPort 10)
# =============================================================================

cmd_reboot() {
    publish_downlink \
        '{"fPort":10,"object":{"cmd":"reboot"}}' \
        'REBOOT - riavvia il device'
}

cmd_identify() {
    publish_downlink \
        '{"fPort":10,"object":{"cmd":"identify"}}' \
        'IDENTIFY - LED lampeggia 10 volte'
}

cmd_identify_on() {
    publish_downlink \
        '{"fPort":10,"object":{"cmd":"identify_on"}}' \
        'IDENTIFY_ON - modalita' persistente attiva (blink continuo)'
}

cmd_identify_off() {
    publish_downlink \
        '{"fPort":10,"object":{"cmd":"identify_off"}}' \
        'IDENTIFY_OFF - disattiva modalita' identify persistente'
}

cmd_get_state() {
    publish_downlink \
        '{"fPort":10,"object":{"cmd":"get_state"}}' \
        'GET_STATE - richiede invio payload state 0x43 nel prossimo TX'
}

cmd_force_tx() {
    publish_downlink \
        '{"fPort":10,"object":{"cmd":"force_tx_now"}}' \
        'FORCE_TX_NOW - prossimo ciclo TX immediato'
}

cmd_clear_nvs() {
    echo "ATTENZIONE: CLEAR_NVS cancella la sessione LoRaWAN persistente!"
    echo "Il device tornera' a FCnt=0 dopo il riavvio. Sicuro? [y/N]"
    read -r risp
    if [ "$risp" != "y" ] && [ "$risp" != "Y" ]; then
        echo "Annullato."
        return 0
    fi
    publish_downlink \
        '{"fPort":10,"object":{"cmd":"clear_nvs"}}' \
        'CLEAR_NVS - cancella NVS e riavvia'
}


# =============================================================================
# COMANDI DI CONFIGURAZIONE (FPort 20)
# =============================================================================

cmd_set_tx_interval() {
    local preset="$1"
    if [ -z "$preset" ]; then
        echo "Uso: $0 set_tx_interval <preset>"
        echo "  0 = 10s   (per test veloci)"
        echo "  1 = 20s"
        echo "  2 = 1min  (default produzione)"
        echo "  3 = 5min"
        echo "  4 = 10min"
        echo "  5 = 30min"
        return 1
    fi
    if [ "$preset" -lt 0 ] || [ "$preset" -gt 5 ]; then
        echo "Errore: preset deve essere 0-5"
        return 1
    fi
    publish_downlink \
        "{\"fPort\":20,\"object\":{\"cmd\":\"set_tx_interval\",\"value\":$preset}}" \
        "SET_TX_INTERVAL - preset=$preset"
}

cmd_set_lorawan_sf() {
    local sf="$1"
    if [ -z "$sf" ]; then
        echo "Uso: $0 set_lorawan_sf <SF>"
        echo "  SF 7-12 (7=veloce corto raggio, 12=lento lungo raggio)"
        return 1
    fi
    if [ "$sf" -lt 7 ] || [ "$sf" -gt 12 ]; then
        echo "Errore: SF deve essere 7-12"
        return 1
    fi
    publish_downlink \
        "{\"fPort\":20,\"object\":{\"cmd\":\"set_lorawan_sf\",\"value\":$sf}}" \
        "SET_LORAWAN_SF - SF$sf"
}

cmd_set_tx_power() {
    local pwr="$1"
    if [ -z "$pwr" ]; then
        echo "Uso: $0 set_tx_power <dBm>"
        echo "  Range: 2-14 dBm (14 = massimo legale EU868)"
        return 1
    fi
    if [ "$pwr" -lt 2 ] || [ "$pwr" -gt 14 ]; then
        echo "Errore: potenza deve essere 2-14 dBm"
        return 1
    fi
    publish_downlink \
        "{\"fPort\":20,\"object\":{\"cmd\":\"set_tx_power\",\"value\":$pwr}}" \
        "SET_TX_POWER - $pwr dBm"
}

cmd_set_gps_timeout() {
    local sec="$1"
    if [ -z "$sec" ]; then
        echo "Uso: $0 set_gps_timeout <secondi>"
        echo "  Range: 10-300 secondi (90 default cold start)"
        return 1
    fi
    if [ "$sec" -lt 10 ] || [ "$sec" -gt 300 ]; then
        echo "Errore: timeout deve essere 10-300s"
        return 1
    fi
    publish_downlink \
        "{\"fPort\":20,\"object\":{\"cmd\":\"set_gps_timeout\",\"value\":$sec}}" \
        "SET_GPS_TIMEOUT - $sec secondi"
}

cmd_set_batt_thresh() {
    local em="$1"
    local rc="$2"
    if [ -z "$em" ] || [ -z "$rc" ]; then
        echo "Uso: $0 set_batt_thresh <emergency_mv> <recovery_mv>"
        echo "  Range: 2500-4200 mV"
        echo "  recovery_mv deve essere > emergency_mv (isteresi)"
        echo "  Default firmware: emergency=3100, recovery=3300"
        return 1
    fi
    if [ "$em" -lt 2500 ] || [ "$em" -gt 4200 ] || \
       [ "$rc" -lt 2500 ] || [ "$rc" -gt 4200 ] || \
       [ "$rc" -le "$em" ]; then
        echo "Errore: soglie fuori range o recovery <= emergency"
        return 1
    fi
    publish_downlink \
        "{\"fPort\":20,\"object\":{\"cmd\":\"set_batt_thresholds\",\"value\":{\"emergency_mv\":$em,\"recovery_mv\":$rc}}}" \
        "SET_BATT_THRESH - emergency=${em}mV recovery=${rc}mV"
}

cmd_set_adr_enabled() {
    local val="$1"
    if [ -z "$val" ] || { [ "$val" != "0" ] && [ "$val" != "1" ]; }; then
        echo "Uso: $0 set_adr_enabled <0|1>"
        echo "  1 = ADR abilitato (default in OTAA)"
        echo "  0 = ADR disabilitato (necessario se vuoi forzare SF manuale)"
        echo "  Nota: ADR e' comunque ignorato in modalita' ABP"
        return 1
    fi
    publish_downlink \
        "{\"fPort\":20,\"object\":{\"cmd\":\"set_adr_enabled\",\"value\":$val}}" \
        "SET_ADR_ENABLED - ${val}"
}


# =============================================================================
# MONITORING (mosquitto_sub)
# =============================================================================

cmd_sub_up() {
    echo "Ascolto uplink dal device su:"
    echo "  Broker: $PUB_HOST:$BROKER_PORT"
    echo "  Topic : $TOPIC_UP"
    echo "  (Ctrl+C per interrompere)"
    echo
    local auth
    auth=$(mqtt_auth_flags)
    mosquitto_sub -h "$PUB_HOST" -p "$BROKER_PORT" $auth -v -t "$TOPIC_UP"
}

cmd_sub_down() {
    echo "Ascolto downlink in transito su:"
    echo "  Broker: $PUB_HOST:$BROKER_PORT"
    echo "  Topic : $TOPIC_DOWN"
    echo "  (Ctrl+C per interrompere)"
    echo
    local auth
    auth=$(mqtt_auth_flags)
    mosquitto_sub -h "$PUB_HOST" -p "$BROKER_PORT" $auth -v -t "$TOPIC_DOWN"
}

cmd_sub_all() {
    echo "Ascolto TUTTI i topic ChirpStack per il device su:"
    echo "  Broker: $PUB_HOST:$BROKER_PORT"
    echo "  Topic : ${TOPIC_PREFIX}application/+/device/${DEV_EUI}/#"
    echo "  (Ctrl+C per interrompere)"
    echo
    local auth
    auth=$(mqtt_auth_flags)
    mosquitto_sub -h "$PUB_HOST" -p "$BROKER_PORT" $auth -v \
        -t "${TOPIC_PREFIX}application/+/device/${DEV_EUI}/#"
}


# =============================================================================
# HELP
# =============================================================================

cmd_help() {
    cat <<EOF
=======================================================================
  test_downlinks.sh - Comandi test downlink LoRaWAN
=======================================================================

CONFIGURAZIONE ATTUALE (override con variabili ambiente):
  BROKER_HOST    = $BROKER_HOST
  BROKER_PORT    = $BROKER_PORT
  EDGE_ID        = $EDGE_ID
  APP_ID         = $APP_ID
  DEV_EUI        = $DEV_EUI
  MODE           = $MODE     (bridge | direct)
  DIRECT_HOST    = $DIRECT_HOST
  MQTT_USER      = ${MQTT_USER:-<anonymous>}

TOPIC RISULTANTE:
  Downlink: $TOPIC_DOWN
  Uplink:   $TOPIC_UP

COMANDI DI AZIONE (FPort 10):
  reboot                       Riavvia il device
  identify                     LED lampeggia 10 volte
  force_tx                     Forza uplink immediato
  clear_nvs                    Cancella NVS (con conferma)

COMANDI DI CONFIGURAZIONE (FPort 20):
  set_tx_interval <preset>     Intervallo TX: 0=10s 1=20s 2=1m 3=5m 4=10m 5=30m
  set_lorawan_sf <SF>          Spreading factor 7-12
  set_tx_power <dBm>           Potenza TX 2-14 dBm
  set_gps_timeout <secondi>    Timeout fix GPS 10-300s
  set_batt_thresh <em> <rc>    Soglie batteria emergency/recovery in mV
  set_adr_enabled <0|1>        Abilita/disabilita ADR (solo effettivo in OTAA)
  identify_on                  Modalita' identify persistente (blink continuo)
  identify_off                 Disattiva modalita' identify persistente
  get_state                    Richiede invio payload state 0x43 nel prossimo TX

MONITORING:
  sub_up                       Ascolta uplink dal device in tempo reale
  sub_down                     Ascolta downlink in transito
  sub_all                      Ascolta TUTTI i topic del device

ESEMPI:
  $0 reboot
  $0 identify
  $0 set_tx_interval 0                    # 10s per test veloci
  $0 set_tx_interval 2                    # 1min per produzione
  $0 set_tx_power 2                       # potenza minima per test range
  $0 set_batt_thresh 3000 3200            # soglie meno aggressive
  $0 sub_up                               # apri in un altro terminale

ESEMPIO CON OVERRIDE:
  BROKER_HOST=192.168.1.50 EDGE_ID=lab-01 $0 identify

MODALITA' DIRECT (bypass del bridge, connessione diretta al RPi):
  MODE=direct DIRECT_HOST=192.168.1.100 $0 reboot

=======================================================================
EOF
}


# =============================================================================
# MAIN
# =============================================================================

# Verifica dipendenze
if ! command -v mosquitto_pub >/dev/null 2>&1; then
    echo "Errore: mosquitto_pub non installato."
    echo "  Ubuntu/Debian: sudo apt install mosquitto-clients"
    echo "  macOS:         brew install mosquitto"
    exit 1
fi

case "${1:-help}" in
    reboot)             cmd_reboot ;;
    identify)           cmd_identify ;;
    identify_on)        cmd_identify_on ;;
    identify_off)       cmd_identify_off ;;
    get_state)          cmd_get_state ;;
    force_tx)           cmd_force_tx ;;
    clear_nvs)          cmd_clear_nvs ;;

    set_tx_interval)    cmd_set_tx_interval "$2" ;;
    set_lorawan_sf)     cmd_set_lorawan_sf "$2" ;;
    set_tx_power)       cmd_set_tx_power "$2" ;;
    set_gps_timeout)    cmd_set_gps_timeout "$2" ;;
    set_batt_thresh)    cmd_set_batt_thresh "$2" "$3" ;;
    set_adr_enabled)    cmd_set_adr_enabled "$2" ;;

    sub_up)             cmd_sub_up ;;
    sub_down)           cmd_sub_down ;;
    sub_all)            cmd_sub_all ;;

    help|--help|-h)     cmd_help ;;

    *)
        echo "Comando sconosciuto: $1"
        echo "Usa '$0 help' per la lista completa."
        exit 1
        ;;
esac
