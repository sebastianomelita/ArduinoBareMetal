"""Definizione della scheda: impronte, componenti, netlist e piazzamento.

Coordinate in mm, origine in alto a sinistra, y verso il basso.
Le rotazioni sono multipli di 90 gradi (senso antiorario visto dall'alto).
"""
import math

BOARD_W = 72.0
BOARD_H = 52.0
MOUNT_HOLES = [(3.5, 3.5), (BOARD_W - 3.5, 3.5), (3.5, BOARD_H - 3.5), (BOARD_W - 3.5, BOARD_H - 3.5)]
MOUNT_DRILL = 3.2

# ---------------------------------------------------------------- impronte
# Ogni pad: (numero, forma, dx, dy, w, h, drill)  forma: 'rect' | 'circle'
# 'tht' se drill > 0 (presente su entrambi i lati), altrimenti SMD sul lato top.
# Un pad può avere più rettangoli (pad composito): stessa numerazione ripetuta.

def fp_0805():
    # KiCad R_0805_2012Metric_Pad1.20x1.40mm_HandSolder
    return [(1, 'rect', -1.0, 0, 1.2, 1.4, 0), (2, 'rect', 1.0, 0, 1.2, 1.4, 0)]

def fp_sot23():
    # KiCad SOT-23: 1 = G, 2 = S, 3 = D per AO3401A / AO3400A / 2N7002
    return [(1, 'rect', -1.0, -0.95, 0.9, 0.8, 0), (2, 'rect', -1.0, 0.95, 0.9, 0.8, 0),
            (3, 'rect', 1.0, 0.0, 0.9, 0.8, 0)]

def fp_sot23_6():
    # KiCad SOT-23-6 (va bene anche per TSOT-23-6 / ThinSOT dell'LTC4412)
    # 1-2-3 a sinistra dall'alto in basso, 4-5-6 a destra dal basso in alto
    return [(1, 'rect', -1.1375, -0.95, 1.325, 0.6, 0), (2, 'rect', -1.1375, 0, 1.325, 0.6, 0),
            (3, 'rect', -1.1375, 0.95, 1.325, 0.6, 0), (4, 'rect', 1.1375, 0.95, 1.325, 0.6, 0),
            (5, 'rect', 1.1375, 0, 1.325, 0.6, 0), (6, 'rect', 1.1375, -0.95, 1.325, 0.6, 0)]


def fp_sot89():
    # KiCad SOT-89-3: pad 2 composito (pad + linguetta). HT7833: 1 GND, 2 VIN, 3 VOUT
    return [(1, 'rect', -1.65, -1.5, 1.3, 0.9, 0), (3, 'rect', -1.65, 1.5, 1.3, 0.9, 0),
            (2, 'rect', -1.5625, 0, 1.475, 0.9, 0), (2, 'rect', 0.7375, 0, 3.125, 1.733, 0)]

def fp_sma():
    # KiCad D_SMA: 1 = catodo, 2 = anodo
    return [(1, 'rect', -2.0, 0, 2.5, 1.8, 0), (2, 'rect', 2.0, 0, 2.5, 1.8, 0)]

def fp_header(n):
    pads = []
    for i in range(n):
        pads.append((i + 1, 'rect' if i == 0 else 'circle', 0, i * 2.54, 1.7, 1.7, 1.0))
    return pads

def fp_cp_radial_d8():
    # Elettrolitico radiale D8 mm passo 3,5 mm: 1 = +
    return [(1, 'rect', 0, 0, 1.6, 1.6, 0.8), (2, 'circle', 3.5, 0, 1.6, 1.6, 0.8)]

FOOTPRINTS = {
    '0805': fp_0805, 'SOT23': fp_sot23, 'SOT23_6': fp_sot23_6, 'SOT89': fp_sot89, 'SMA': fp_sma,
    'H2': lambda: fp_header(2), 'H3': lambda: fp_header(3), 'H4': lambda: fp_header(4),
    'H7': lambda: fp_header(7), 'CP8': fp_cp_radial_d8,
}

# Contorno serigrafia (rettangolo del corpo, relativo all'origine) per ogni impronta
BODY = {
    '0805': (-1.9, -0.95, 1.9, 0.95),
    'SOT23': (-1.75, -1.6, 1.75, 1.6),
    'SOT23_6': (-2.05, -1.75, 2.05, 1.75),
    'SOT89': (-2.6, -2.3, 2.45, 2.3),
    'SMA': (-3.6, -1.35, 3.6, 1.35),
    'H2': (-1.27, -1.27, 1.27, 2.54 + 1.27),
    'H3': (-1.27, -1.27, 1.27, 2 * 2.54 + 1.27),
    'H4': (-1.27, -1.27, 1.27, 3 * 2.54 + 1.27),
    'H7': (-1.27, -1.27, 1.27, 6 * 2.54 + 1.27),
}

# ---------------------------------------------------------------- componenti
# ref: (impronta, x, y, rot, valore, {pad: net})
POWER_NETS = {'VSOL', 'VUSB', 'VIN_EXT', 'VBAT', 'VLOAD', '3V3', 'SWIN', 'SWOUT', 'BOOSTOUT', 'SENSVCC', 'QCS', '3V3E'}

COMPONENTS = {
    # ingressi
    'J1': ('H2', 5.0, 11.0, 0, 'SOLAR', {1: 'VSOL', 2: 'GND'}),
    'J2': ('H2', 5.0, 21.0, 0, 'USB 5V', {1: 'VUSB', 2: 'GND'}),
    'D1': ('SMA', 12.0, 11.0, 180, 'SS34', {1: 'VIN_EXT', 2: 'VSOL'}),
    'D2': ('SMA', 12.0, 21.0, 180, 'SS34', {1: 'VIN_EXT', 2: 'VUSB'}),
    # alternativa a D1 (montare D1 OPPURE U2+Q6+Q7+RCTL): diodo ideale LTC4412
    # Q6 e Q7 contrapposti (sorgenti in comune) per staccare davvero il pannello
    'Q6': ('SOT23', 20.4, 11.0, 0, 'AO3401A', {1: 'LGATE', 2: 'QCS', 3: 'VSOL'}),
    'Q7': ('SOT23', 20.4, 15.2, 0, 'AO3401A', {1: 'LGATE', 2: 'QCS', 3: 'VIN_EXT'}),
    'U2': ('SOT23_6', 19.4, 19.6, 0, 'LTC4412', {1: 'VSOL', 2: 'GND', 3: 'CTL', 4: 'NC_STAT', 5: 'LGATE', 6: 'VIN_EXT'}),
    'RCTL': ('0805', 16.5, 25.5, 0, '100k', {1: 'VUSB', 2: 'CTL'}),
    'J3': ('H4', 14.0, 4.5, 90, 'TP4056', {1: 'VIN_EXT', 2: 'GND', 3: 'VBAT', 4: 'GND'}),
    # load sharing
    'D3': ('SMA', 23.0, 25.0, 180, 'SS34', {1: 'VLOAD', 2: 'VIN_EXT'}),
    'Q1': ('SOT23', 29.0, 15.0, 0, 'AO3401A', {1: 'VIN_EXT', 2: 'VLOAD', 3: 'VBAT'}),
    'R3': ('0805', 24.5, 15.0, 90, '100k', {1: 'VIN_EXT', 2: 'GND'}),
    # regolatore
    'U1': ('SOT89', 40.0, 16.0, 0, 'HT7833', {1: 'GND', 2: 'VLOAD', 3: '3V3'}),
    'C1': ('0805', 35.0, 20.5, 90, '10uF', {1: 'VLOAD', 2: 'GND'}),
    'C2': ('0805', 40.0, 22.5, 0, '10uF', {1: '3V3', 2: 'GND'}),
    'C4': ('0805', 40.0, 25.5, 0, '100nF', {1: '3V3', 2: 'GND'}),
    'C3': ('CP8', 34.0, 7.0, 0, '470uF', {1: '3V3', 2: 'GND'}),
    # partitore attivabile
    'Q2': ('SOT23', 21.0, 34.0, 0, 'AO3401A', {1: 'Q2G', 2: 'VBAT', 3: 'DIVT'}),
    'R4': ('0805', 16.5, 34.0, 90, '100k', {1: 'VBAT', 2: 'Q2G'}),
    'Q3': ('SOT23', 16.5, 40.5, 0, '2N7002', {1: 'EN_DIV', 2: 'GND', 3: 'Q2G'}),
    'R5': ('0805', 11.5, 40.5, 90, '100k', {1: 'EN_DIV', 2: 'GND'}),
    'R1': ('0805', 26.0, 34.0, 90, '100k', {1: 'DIVT', 2: 'ADC'}),
    'R2': ('0805', 26.0, 39.5, 90, '100k', {1: 'ADC', 2: 'GND'}),
    'C5': ('0805', 30.0, 39.5, 90, '100nF', {1: 'ADC', 2: 'GND'}),
    # interruttore sensore
    'JP1': ('H3', 44.0, 8.0, 90, 'SRC', {1: 'VBAT', 2: 'SWIN', 3: '3V3'}),
    'Q4': ('SOT23', 50.0, 16.0, 0, 'AO3401A', {1: 'Q4G', 2: 'SWIN', 3: 'SWOUT'}),
    'R6': ('0805', 46.5, 16.0, 90, '100k', {1: 'SWIN', 2: 'Q4G'}),
    'RS': ('0805', 46.5, 22.5, 90, '10k', {1: 'Q4G', 2: 'Q5D'}),
    'CGD': ('0805', 52.0, 21.0, 90, '1uF', {1: 'Q4G', 2: 'SWOUT'}),
    'Q5': ('SOT23', 46.5, 28.5, 0, '2N7002', {1: 'EN_SENS', 2: 'GND', 3: 'Q5D'}),
    'R7': ('0805', 41.5, 30.0, 90, '100k', {1: 'EN_SENS', 2: 'GND'}),
    'J6': ('H3', 67.0, 10.0, 0, 'BOOST', {1: 'SWOUT', 2: 'GND', 3: 'BOOSTOUT'}),
    'JP2': ('H3', 57.0, 28.0, 0, 'OUT', {1: 'SWOUT', 2: 'SENSVCC', 3: 'BOOSTOUT'}),
    'C6': ('CP8', 59.5, 42.5, 0, '470uF', {1: 'SENSVCC', 2: 'GND'}),
    'C7': ('0805', 62.0, 28.5, 90, '10uF', {1: 'SENSVCC', 2: 'GND'}),
    'C8': ('0805', 62.0, 33.0, 90, '100nF', {1: 'SENSVCC', 2: 'GND'}),
    'R8': ('0805', 51.0, 33.5, 90, '4k7*', {1: 'SENSVCC', 2: 'IO1'}),
    'R9': ('0805', 53.5, 33.5, 90, '4k7*', {1: 'SENSVCC', 2: 'IO2'}),
    'J5': ('H4', 67.0, 25.5, 0, 'SENSOR', {1: 'SENSVCC', 2: 'GND', 3: 'IO1', 4: 'IO2'}),
    # verso l'ESP32
    # JP3: togliere il cappuccio quando si programma l'ESP32 via USB
    'JP3': ('H2', 9.0, 47.0, 90, 'PROG', {1: '3V3', 2: '3V3E'}),
    'J4': ('H7', 18.0, 48.0, 90, 'ESP32', {1: '3V3E', 2: 'GND', 3: 'ADC', 4: 'EN_DIV', 5: 'EN_SENS', 6: 'IO1', 7: 'IO2'}),
}

# Aree di rame aggiuntive sul lato top: (rete, x0, y0, x1, y1)
# Dissipatore per la linguetta (VIN) dell'HT7833
ZONES = [('VLOAD', 42.0, 12.6, 44.6, 19.4)]

# lato delle etichette dei pin per i connettori verticali ('L' o 'R')
# lato preferito della sigla (T, B, L, R) dove la scelta automatica non va bene
REF_SIDE = {'Q6': 'L', 'Q7': 'L'}

LABEL_SIDE = {'J1': 'R', 'J2': 'R', 'J5': 'L', 'J6': 'L', 'JP2': 'R'}

PIN_LABELS = {
    'J1': ['+', '-'], 'J2': ['+', '-'],
    'J3': ['IN+', 'IN-', 'OUT+', 'OUT-'],
    'J4': ['3V3', 'GND', 'ADC', 'EDIV', 'ESNS', 'IO1', 'IO2'],
    'J5': ['VCC', 'GND', 'IO1', 'IO2'],
    'J6': ['IN', 'GND', 'OUT'],
    'JP1': ['BAT', '', '3V3'],
    'JP2': ['DIR', '', 'BST'],
    'JP3': ['3V3', 'ESP'],
}


def rotate(dx, dy, rot):
    r = math.radians(rot)
    c, s = round(math.cos(r)), round(math.sin(r))
    # rotazione antioraria vista dall'alto con asse y verso il basso
    return dx * c + dy * s, -dx * s + dy * c


def placed_pads():
    """Restituisce l'elenco dei pad posizionati:
    dict(ref, num, net, shape, x, y, w, h, drill)"""
    out = []
    for ref, (fp, x, y, rot, val, nets) in COMPONENTS.items():
        for (num, shape, dx, dy, w, h, drill) in FOOTPRINTS[fp]():
            rx, ry = rotate(dx, dy, rot)
            if rot in (90, 270):
                w, h = h, w
            out.append(dict(ref=ref, num=num, net=nets[num], shape=shape,
                            x=x + rx, y=y + ry, w=w, h=h, drill=drill))
    return out


def body_rect(ref):
    fp, x, y, rot, val, nets = COMPONENTS[ref]
    if fp == 'CP8':
        return None
    x0, y0, x1, y1 = BODY[fp]
    pts = [rotate(px, py, rot) for px, py in ((x0, y0), (x1, y1))]
    xs = sorted(p[0] for p in pts)
    ys = sorted(p[1] for p in pts)
    return (x + xs[0], y + ys[0], x + xs[1], y + ys[1])
