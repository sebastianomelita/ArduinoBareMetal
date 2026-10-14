"""Generazione dei file di produzione: Gerber RS-274X, forature Excellon, BOM, posizionamento."""
import json, os, zipfile, math, csv
from board import (BOARD_W, BOARD_H, MOUNT_HOLES, MOUNT_DRILL, ZONES, COMPONENTS, PIN_LABELS, LABEL_SIDE,
                   placed_pads, body_rect, rotate)
from drc import plane_clearances, PLANE_INSET, PLANE_CLR, THERMAL_SPOKE
from route import VIA_D, VIA_DRILL
from font import text_strokes

MASK_EXP = 0.05
SILK_W = 0.16
TITLE = 'ESP32 SOLAR POWER V1'


def fmt(v):
    return str(int(round(v * 1e6)))


class Gerber:
    def __init__(self, name, func):
        self.lines = ['G04 Generato da pcb/gerber.py*', '%FSLAX46Y46*%', '%MOMM*%',
                      f'%TF.FileFunction,{func}*%', '%LPD*%']
        self.aps = {}
        self.body = []
        self.cur = None
        self.name = name

    def ap(self, kind, a, b=None):
        key = (kind, round(a, 4), None if b is None else round(b, 4))
        if key not in self.aps:
            code = 10 + len(self.aps)
            self.aps[key] = code
            if kind == 'C':
                self.lines.append(f'%ADD{code}C,{a:.4f}*%')
            else:
                self.lines.append(f'%ADD{code}R,{a:.4f}X{b:.4f}*%')
        code = self.aps[key]
        if self.cur != code:
            self.body.append(f'D{code}*')
            self.cur = code

    def flash(self, x, y):
        self.body.append(f'X{fmt(x)}Y{fmt(BOARD_H - y)}D03*')

    def line(self, pts):
        self.body.append(f'X{fmt(pts[0][0])}Y{fmt(BOARD_H - pts[0][1])}D02*')
        for x, y in pts[1:]:
            self.body.append(f'X{fmt(x)}Y{fmt(BOARD_H - y)}D01*')

    def region(self, pts):
        self.body.append('G36*')
        self.body.append(f'X{fmt(pts[0][0])}Y{fmt(BOARD_H - pts[0][1])}D02*')
        for x, y in pts[1:] + [pts[0]]:
            self.body.append(f'X{fmt(x)}Y{fmt(BOARD_H - y)}D01*')
        self.body.append('G37*')

    def polarity(self, dark):
        self.body.append('%LPD*%' if dark else '%LPC*%')

    def pad(self, p, exp=0.0):
        if p['shape'] == 'circle':
            self.ap('C', p['w'] + 2 * exp)
        else:
            self.ap('R', p['w'] + 2 * exp, p['h'] + 2 * exp)
        self.flash(p['x'], p['y'])

    def save(self, path):
        self.body.append('M02*')
        open(path, 'w').write('\n'.join(self.lines + ['G01*'] + self.body) + '\n')


def silk_items(routing):
    """Linee di serigrafia: contorni, segni di polarità, sigle, etichette dei pin, legenda."""
    pads = placed_pads()
    lines = []
    texts = []   # (stringa, x, y, h, anchor, vertical)

    for ref, (fp, x, y, rot, val, nets) in COMPONENTS.items():
        br = body_rect(ref)
        if br:
            x0, y0, x1, y1 = br
            lines.append([(x0, y0), (x1, y0), (x1, y1), (x0, y1), (x0, y0)])
        else:  # elettrolitico radiale: cerchio
            cx, cy, r = x + 1.75, y, 4.0
            lines.append([(cx + r * math.cos(t / 36 * 2 * math.pi), cy + r * math.sin(t / 36 * 2 * math.pi)) for t in range(37)])
            texts.append(('+', x - 2.0, y - 0.5, 1.0, 'center', False))
        if fp == 'SMA':  # barra del catodo (pad 1)
            kx, ky = rotate(1, 0, rot)
            ex = x + (-kx) * 3.8 if False else None
            # lato del catodo: segno di dx del pad 1 = -2 ruotato
            cx_, cy_ = rotate(-2.0, 0, rot)
            side = 1 if cx_ > 0 else -1
            for off in (3.6, 3.8, 4.0):
                lines.append([(x + side * off, y - 1.35), (x + side * off, y + 1.35)])
        if fp in ('SOT23', 'SOT89'):  # punto del pin 1
            p1 = [p for p in pads if p['ref'] == ref and p['num'] == 1][0]
            dx = -1 if p1['x'] < x else 1
            px, py = p1['x'] + dx * (p1['w'] / 2 + 0.45), p1['y']
            lines.append([(px - 0.15, py), (px + 0.15, py)])
            lines.append([(px, py - 0.15), (px, py + 0.15)])

    # sigle: prova sopra, sotto, sinistra, destra del corpo e sceglie la prima libera
    obst = [(p['x'] - p['w'] / 2 - 0.2, p['y'] - p['h'] / 2 - 0.2, p['x'] + p['w'] / 2 + 0.2, p['y'] + p['h'] / 2 + 0.2) for p in pads]
    for hx, hy in MOUNT_HOLES:
        obst.append((hx - 2.2, hy - 2.2, hx + 2.2, hy + 2.2))
    for v in routing['vias']:
        obst.append((v['x'] - 0.5, v['y'] - 0.5, v['x'] + 0.5, v['y'] + 0.5))
    placed = []

    def free(bb):
        x0, y0, x1, y1 = bb
        if x0 < 0.8 or y0 < 0.8 or x1 > BOARD_W - 0.8 or y1 > BOARD_H - 0.8:
            return False
        for o in obst + placed:
            if not (x1 < o[0] or o[2] < x0 or y1 < o[1] or o[3] < y0):
                return False
        return True

    # etichette dei pin dei connettori
    for ref, labels in PIN_LABELS.items():
        fp, x, y, rot, val, nets = COMPONENTS[ref]
        ps = sorted([p for p in pads if p['ref'] == ref], key=lambda p: p['num'])
        horizontal = rot in (90, 270)
        for p, lab in zip(ps, labels):
            if not lab:
                continue
            h = 0.8
            _, w = text_strokes(lab, 0, 0, h)
            if horizontal:
                # testo verticale sotto (o sopra se vicino al bordo inferiore) il pin
                if p['y'] > BOARD_H - 8:
                    bx, by = p['x'] - h / 2, p['y'] - 1.25
                    texts.append((lab, bx, by, h, 'left', True))
                    placed.append((bx, by - w, bx + h, by))
                else:
                    bx, by = p['x'] - h / 2, p['y'] + 1.25 + w
                    texts.append((lab, bx, by, h, 'left', True))
                    placed.append((bx, by - w, bx + h, by))
            else:
                if LABEL_SIDE.get(ref, 'L') == 'L':
                    bx = p['x'] - 1.25 - w
                else:
                    bx = p['x'] + 1.25
                by = p['y'] - h / 2
                texts.append((lab, bx, by, h, 'left', False))
                placed.append((bx, by, bx + w, by + h))
    H = 1.0
    for ref, (fp, x, y, rot, val, nets) in COMPONENTS.items():
        br = body_rect(ref) or (x - 2.25, y - 4, x + 5.75, y + 4)
        x0, y0, x1, y1 = br
        _, w = text_strokes(ref, 0, 0, H)
        cands = [((x0 + x1) / 2 - w / 2, y0 - 0.35 - H), ((x0 + x1) / 2 - w / 2, y1 + 0.35),
                 (x0 - 0.4 - w, (y0 + y1) / 2 - H / 2), (x1 + 0.4, (y0 + y1) / 2 - H / 2)]
        for cx, cy in cands + [(c[0] + dx, c[1] + dy) for c in cands for dx in (-1.5, 1.5) for dy in (-1, 1)]:
            bb = (cx, cy, cx + w, cy + H)
            if free(bb):
                placed.append(bb)
                texts.append((ref, cx, cy, H, 'left', False))
                break
        else:
            print('ATTENZIONE: nessuna posizione libera per la sigla', ref)

    # nomi dei connettori (valore) accanto alla sigla
    legend = [(TITLE, 1.1), ('JP1 SRC: BAT=SDS011 3V3=SCD41', 0.8), ('JP2 OUT: BST=SDS011 DIR=SCD41', 0.8),
              ('J3 -> MODULO TP4056', 0.8), ('J6 -> MODULO BOOST 5V', 0.8)]
    ly = 38.6
    for s, h in legend:
        texts.append((s, 35.6, ly, h, 'left', False))
        ly += h + 0.85
    out = list(lines)
    for (s, x, y, h, anchor, vertical) in texts:
        strokes, _ = text_strokes(s, x, y, h, anchor, vertical)
        out += strokes
    return out


def write_all(routing, outdir='gerber'):
    os.makedirs(outdir, exist_ok=True)
    pads = placed_pads()
    base = os.path.join(outdir, 'esp32_solar')
    files = []

    # --- rame top
    g = Gerber('top', 'Copper,L1,Top')
    for p in pads:
        g.pad(p)
    for (net, x0, y0, x1, y1) in ZONES:
        g.region([(x0, y0), (x1, y0), (x1, y1), (x0, y1)])
    for t in routing['tracks']:
        if t['layer'] == 'top':
            g.ap('C', t['w']); g.line(t['pts'])
    for v in routing['vias']:
        g.ap('C', VIA_D); g.flash(v['x'], v['y'])
    g.save(base + '.GTL'); files.append(base + '.GTL')

    # --- rame bottom: piano di massa con sgomberi e relief termici
    g = Gerber('bot', 'Copper,L2,Bot')
    i0, i1 = PLANE_INSET, None
    g.region([(PLANE_INSET, PLANE_INSET), (BOARD_W - PLANE_INSET, PLANE_INSET),
              (BOARD_W - PLANE_INSET, BOARD_H - PLANE_INSET), (PLANE_INSET, BOARD_H - PLANE_INSET)])
    clear, thermals = plane_clearances(routing)
    g.polarity(False)
    for c in clear:
        if c[0] == 'c':
            g.ap('C', 2 * c[3]); g.flash(c[1], c[2])
        elif c[0] == 'r':
            _, x0, y0, x1, y1 = c
            g.ap('R', x1 - x0, y1 - y0); g.flash((x0 + x1) / 2, (y0 + y1) / 2)
        else:
            _, x0, y0, x1, y1, w = c
            g.ap('C', w); g.line([(x0, y0), (x1, y1)])
    g.polarity(True)
    for p in pads:
        if p['drill']:
            g.pad(p)
    for p in thermals:
        L = p['w'] / 2 + PLANE_CLR + 0.1
        g.ap('C', THERMAL_SPOKE)
        g.line([(p['x'] - L, p['y']), (p['x'] + L, p['y'])])
        g.line([(p['x'], p['y'] - L), (p['x'], p['y'] + L)])
    for t in routing['tracks']:
        if t['layer'] == 'bot':
            g.ap('C', t['w']); g.line(t['pts'])
    for v in routing['vias']:
        g.ap('C', VIA_D); g.flash(v['x'], v['y'])
    g.save(base + '.GBL'); files.append(base + '.GBL')

    # --- maschere di saldatura (aperture sui pad; via coperte)
    for side, ext, func in (('top', '.GTS', 'Soldermask,Top'), ('bot', '.GBS', 'Soldermask,Bot')):
        g = Gerber(side, func)
        for p in pads:
            if side == 'top' or p['drill']:
                g.pad(p, MASK_EXP)
        for hx, hy in MOUNT_HOLES:
            g.ap('C', MOUNT_DRILL + 0.2); g.flash(hx, hy)
        g.save(base + ext); files.append(base + ext)

    # --- pasta saldante (solo SMD, lato top)
    g = Gerber('paste', 'Paste,Top')
    for p in pads:
        if not p['drill']:
            g.pad(p)
    g.save(base + '.GTP'); files.append(base + '.GTP')

    # --- serigrafia
    g = Gerber('silk', 'Legend,Top')
    g.ap('C', SILK_W)
    for pl in silk_items(routing):
        g.line(pl)
    g.save(base + '.GTO'); files.append(base + '.GTO')

    # --- contorno
    g = Gerber('outline', 'Profile,NP')
    g.ap('C', 0.1)
    g.line([(0, 0), (BOARD_W, 0), (BOARD_W, BOARD_H), (0, BOARD_H), (0, 0)])
    g.save(base + '.GKO'); files.append(base + '.GKO')

    # --- forature Excellon (metrico)
    def excellon(path, holes, plated):
        tools = sorted({d for d, x, y in holes})
        L = ['M48', '; ' + ('PLATED' if plated else 'NON-PLATED') + ' holes', 'METRIC,TZ', 'FMAT,2']
        for k, d in enumerate(tools, 1):
            L.append(f'T{k}C{d:.3f}')
        L.append('%')
        L.append('G90')
        L.append('G05')
        for k, d in enumerate(tools, 1):
            L.append(f'T{k}')
            for dd, x, y in holes:
                if dd == d:
                    L.append(f'X{x:.3f}Y{BOARD_H - y:.3f}')
        L.append('T0')
        L.append('M30')
        open(path, 'w').write('\n'.join(L) + '\n')
    pth = [(p['drill'], p['x'], p['y']) for p in pads if p['drill']] + [(VIA_DRILL, v['x'], v['y']) for v in routing['vias']]
    excellon(base + '-PTH.DRL', pth, True); files.append(base + '-PTH.DRL')
    excellon(base + '-NPTH.DRL', [(MOUNT_DRILL, x, y) for x, y in MOUNT_HOLES], False); files.append(base + '-NPTH.DRL')

    with zipfile.ZipFile(os.path.join(outdir, 'esp32_solar_gerber.zip'), 'w', zipfile.ZIP_DEFLATED) as z:
        for f in files:
            z.write(f, os.path.basename(f))
    return files


BOM_INFO = {
    'SS34': ('Diodo Schottky 3 A 40 V', 'SMA (DO-214AC)'),
    'AO3401A': ('P-MOSFET -30 V logic level', 'SOT-23'),
    '2N7002': ('N-MOSFET 60 V (o BSS138)', 'SOT-23'),
    'HT7833': ('Regolatore LDO 3,3 V 500 mA', 'SOT-89-3'),
    '100k': ('Resistenza 1%', '0805'), '10k': ('Resistenza 1%', '0805'),
    '4k7*': ('Resistenza 1% - opzionale (pull-up I2C)', '0805'),
    '10uF': ('Cond. ceramico X5R/X7R >= 10 V', '0805'), '1uF': ('Cond. ceramico X7R >= 10 V', '0805'),
    '100nF': ('Cond. ceramico X7R', '0805'),
    '470uF': ('Cond. elettrolitico >= 10 V, D8 mm passo 3,5 mm', 'radiale'),
}


def write_bom(path='gerber/BOM.csv'):
    groups = {}
    for ref, (fp, x, y, rot, val, nets) in COMPONENTS.items():
        if fp.startswith('H'):
            key = ('Connettore a pettine 2,54 mm', f'1x{fp[1:]}')
        else:
            desc, pkg = BOM_INFO[val]
            key = (f'{val} - {desc}', pkg)
        groups.setdefault(key, []).append(ref)
    with open(path, 'w', newline='', encoding='utf-8') as f:
        w = csv.writer(f)
        w.writerow(['Riferimenti', 'Quantita', 'Valore / descrizione', 'Contenitore'])
        for (desc, pkg), refs in sorted(groups.items(), key=lambda kv: kv[1][0]):
            w.writerow([' '.join(refs), len(refs), desc, pkg])
        w.writerow(['(JP1 JP2)', 2, 'Cappuccio per ponticello (jumper) 2,54 mm', '-'])


if __name__ == '__main__':
    routing = json.load(open('routing.json'))
    for f in write_all(routing):
        print(f, os.path.getsize(f))
    write_bom()
