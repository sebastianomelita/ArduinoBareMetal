"""Verifica indipendente dello stampato: distanze (DRC), collegamenti (LVS) e piano di massa."""
import math, json
import numpy as np
from board import BOARD_W, BOARD_H, MOUNT_HOLES, MOUNT_DRILL, ZONES, placed_pads, COMPONENTS
from route import pad_objects, seg_object, via_objects, VIA_D

CLR = 0.25          # isolamento minimo rame-rame
EDGE = 0.4          # rame - bordo scheda / fori di fissaggio
PLANE_CLR = 0.4     # isolamento del piano di massa dagli oggetti di altre reti
PLANE_INSET = 0.5   # arretramento del piano dal bordo


def seg_seg(ax, ay, bx, by, cx, cy, dx, dy):
    def pt_seg(px, py, x0, y0, x1, y1):
        vx, vy = x1 - x0, y1 - y0
        L2 = vx * vx + vy * vy
        t = 0 if L2 == 0 else max(0, min(1, ((px - x0) * vx + (py - y0) * vy) / L2))
        return math.hypot(px - (x0 + t * vx), py - (y0 + t * vy))

    def cross(ox, oy, px, py, qx, qy):
        return (px - ox) * (qy - oy) - (py - oy) * (qx - ox)
    d1, d2 = cross(ax, ay, bx, by, cx, cy), cross(ax, ay, bx, by, dx, dy)
    d3, d4 = cross(cx, cy, dx, dy, ax, ay), cross(cx, cy, dx, dy, bx, by)
    if (d1 * d2 < 0) and (d3 * d4 < 0):
        return 0.0
    return min(pt_seg(ax, ay, cx, cy, dx, dy), pt_seg(bx, by, cx, cy, dx, dy),
               pt_seg(cx, cy, ax, ay, bx, by), pt_seg(dx, dy, ax, ay, bx, by))


def rect_edges(o):
    x0, y0, x1, y1 = o['x0'], o['y0'], o['x1'], o['y1']
    return [(x0, y0, x1, y0), (x1, y0, x1, y1), (x1, y1, x0, y1), (x0, y1, x0, y0)]


def inside(o, x, y):
    return o['x0'] <= x <= o['x1'] and o['y0'] <= y <= o['y1']


def dist(a, b):
    if a['kind'] == 'rect' and b['kind'] == 'rect':
        dx = max(a['x0'] - b['x1'], b['x0'] - a['x1'], 0)
        dy = max(a['y0'] - b['y1'], b['y0'] - a['y1'], 0)
        return math.hypot(dx, dy)
    if a['kind'] == 'rect':
        a, b = b, a
    if b['kind'] == 'rect':   # a cap, b rect
        if inside(b, a['x0'], a['y0']) or inside(b, a['x1'], a['y1']):
            return -a['r']
        d = min(seg_seg(a['x0'], a['y0'], a['x1'], a['y1'], *e) for e in rect_edges(b))
        return d - a['r']
    return seg_seg(a['x0'], a['y0'], a['x1'], a['y1'], b['x0'], b['y0'], b['x1'], b['y1']) - a['r'] - b['r']


def bbox(o):
    if o['kind'] == 'rect':
        return o['x0'], o['y0'], o['x1'], o['y1']
    r = o['r']
    return min(o['x0'], o['x1']) - r, min(o['y0'], o['y1']) - r, max(o['x0'], o['x1']) + r, max(o['y0'], o['y1']) + r


def build(routing):
    objs = []
    for p in placed_pads():
        for o in pad_objects(p):
            o['id'] = f"{p['ref']}.{p['num']}"
            objs.append(o)
    for k, (net, x0, y0, x1, y1) in enumerate(ZONES):
        objs.append(dict(net=net, layer='top', kind='rect', x0=x0, y0=y0, x1=x1, y1=y1, src='zone', id=f'zone{k}'))
    for k, t in enumerate(routing['tracks']):
        for a, b in zip(t['pts'], t['pts'][1:]):
            o = seg_object(t['net'], t['layer'], a, b, t['w'])
            o['id'] = f'track{k}'
            objs.append(o)
    for k, v in enumerate(routing['vias']):
        for o in via_objects(v['net'], v['x'], v['y']):
            o['id'] = f'via{k}'
            objs.append(o)
    return objs


def check(routing, verbose=True):
    objs = build(routing)
    errors = []
    # 1) distanze tra reti diverse sullo stesso strato
    bbs = [bbox(o) for o in objs]
    n = len(objs)
    parent = list(range(n))

    def find(i):
        while parent[i] != i:
            parent[i] = parent[parent[i]]
            i = parent[i]
        return i
    for i in range(n):
        a = objs[i]
        ax0, ay0, ax1, ay1 = bbs[i]
        for j in range(i + 1, n):
            b = objs[j]
            if a['layer'] != b['layer']:
                continue
            bx0, by0, bx1, by1 = bbs[j]
            if bx0 > ax1 + CLR or ax0 > bx1 + CLR or by0 > ay1 + CLR or ay0 > by1 + CLR:
                continue
            d = dist(a, b)
            if a['net'] != b['net']:
                same_fp = a['id'].split('.')[0] == b['id'].split('.')[0] and '.' in a['id'] and '.' in b['id']
                lim = 0.2 if same_fp else CLR
                if d < lim - 1e-6:
                    errors.append(f"isolamento {d:.3f} mm < {lim} tra {a['id']}({a['net']}) e {b['id']}({b['net']}) su {a['layer']}")
            elif d <= 1e-6:
                parent[find(i)] = find(j)
    # stesso id (pad THT, via) = collegamento tra strati
    byid = {}
    for i, o in enumerate(objs):
        if o['id'].startswith('via') or (o['src'] != 'track' and o['layer'] == 'bot'):
            byid.setdefault(o['id'], []).append(i)
    for i, o in enumerate(objs):
        if o['id'] in byid:
            for j in byid[o['id']]:
                parent[find(i)] = find(j)
    # 2) bordo e fori di fissaggio
    for o in objs:
        x0, y0, x1, y1 = bbox(o)
        if x0 < EDGE or y0 < EDGE or x1 > BOARD_W - EDGE or y1 > BOARD_H - EDGE:
            errors.append(f"{o['id']} troppo vicino al bordo")
        for hx, hy in MOUNT_HOLES:
            hole = dict(kind='cap', x0=hx, y0=hy, x1=hx, y1=hy, r=MOUNT_DRILL / 2)
            if dist(o, hole) < EDGE:
                errors.append(f"{o['id']} troppo vicino al foro di fissaggio ({hx},{hy})")
    # 3) piano di massa: continuità (raster 0,1 mm)
    plane_ok, plane_msg, gnd_plane_ids = plane_connectivity(routing, objs)
    if not plane_ok:
        errors.append(plane_msg)
    gnd_plane_idx = [i for i, o in enumerate(objs) if o['id'] in gnd_plane_ids and o['layer'] == 'bot']
    for i in gnd_plane_idx[1:]:
        parent[find(i)] = find(gnd_plane_idx[0])
    # 4) ogni rete deve essere un'unica isola
    comps = {}
    for i, o in enumerate(objs):
        comps.setdefault(o['net'], set()).add(find(i))
    for net, c in sorted(comps.items()):
        if len(c) != 1:
            pads_in = {}
            for i, o in enumerate(objs):
                if o['net'] == net and '.' in o['id']:
                    pads_in.setdefault(find(i), set()).add(o['id'])
            errors.append(f"rete {net} divisa in {len(c)} parti: {list(pads_in.values())}")
    if verbose:
        print(f"Oggetti di rame: {n}; reti: {len(comps)}")
        print('ERRORI:' if errors else 'Nessun errore.', *errors, sep='\n  ')
    return errors


def plane_clearances(routing):
    """Elenco delle aree da sgombrare nel piano (lato bottom) e dei pad di massa con relief termico."""
    clear, thermals = [], []
    for p in placed_pads():
        if not p['drill']:
            continue
        if p['net'] == 'GND':
            thermals.append(p)
            clear.append(('c', p['x'], p['y'], p['w'] / 2 + PLANE_CLR))
        elif p['shape'] == 'circle':
            clear.append(('c', p['x'], p['y'], p['w'] / 2 + PLANE_CLR))
        else:
            clear.append(('r', p['x'] - p['w'] / 2 - PLANE_CLR, p['y'] - p['h'] / 2 - PLANE_CLR,
                          p['x'] + p['w'] / 2 + PLANE_CLR, p['y'] + p['h'] / 2 + PLANE_CLR))
    for v in routing['vias']:
        if v['net'] != 'GND':
            clear.append(('c', v['x'], v['y'], VIA_D / 2 + PLANE_CLR))
    for t in routing['tracks']:
        if t['layer'] == 'bot' and t['net'] != 'GND':
            for a, b in zip(t['pts'], t['pts'][1:]):
                clear.append(('s', a[0], a[1], b[0], b[1], t['w'] + 2 * PLANE_CLR))
    for hx, hy in MOUNT_HOLES:
        clear.append(('c', hx, hy, MOUNT_DRILL / 2 + 0.6))
    return clear, thermals


THERMAL_SPOKE = 0.5


def plane_raster(routing, res=0.05):
    nx, ny = int(BOARD_W / res) + 1, int(BOARD_H / res) + 1
    X = (np.arange(nx) * res)[:, None]
    Y = (np.arange(ny) * res)[None, :]
    M = (X >= PLANE_INSET) & (X <= BOARD_W - PLANE_INSET) & (Y >= PLANE_INSET) & (Y <= BOARD_H - PLANE_INSET)
    clear, thermals = plane_clearances(routing)
    for c in clear:
        if c[0] == 'c':
            _, x, y, r = c
            M &= ~(np.hypot(X - x, Y - y) <= r)
        elif c[0] == 'r':
            _, x0, y0, x1, y1 = c
            M &= ~((X >= x0) & (X <= x1) & (Y >= y0) & (Y <= y1))
        else:
            _, x0, y0, x1, y1, w = c
            vx, vy = x1 - x0, y1 - y0
            L2 = vx * vx + vy * vy or 1e-12
            t = np.clip(((X - x0) * vx + (Y - y0) * vy) / L2, 0, 1)
            M &= ~(np.hypot(X - (x0 + t * vx), Y - (y0 + t * vy)) <= w / 2)
    for p in thermals:   # pad + 4 razze
        r = p['w'] / 2
        M |= np.hypot(X - p['x'], Y - p['y']) <= r
        L = r + PLANE_CLR + 0.1
        M |= (np.abs(Y - p['y']) <= THERMAL_SPOKE / 2) & (np.abs(X - p['x']) <= L)
        M |= (np.abs(X - p['x']) <= THERMAL_SPOKE / 2) & (np.abs(Y - p['y']) <= L)
    return M, res


def plane_connectivity(routing, objs):
    from collections import deque
    M, res = plane_raster(routing)
    # etichetta le componenti connesse (4-vicini)
    lab = np.zeros(M.shape, dtype=np.int32)
    cur = 0
    nx, ny = M.shape
    pts = []
    for p in placed_pads():
        if p['net'] == 'GND' and p['drill']:
            pts.append((f"{p['ref']}.{p['num']}", p['x'], p['y']))
    for k, v in enumerate(routing['vias']):
        if v['net'] == 'GND':
            pts.append((f'via{k}', v['x'], v['y']))
    for name, x, y in pts:
        i, j = int(round(x / res)), int(round(y / res))
        if not M[i, j]:
            return False, f'{name} non tocca il piano di massa', set()
        if lab[i, j]:
            continue
        cur += 1
        q = deque([(i, j)])
        lab[i, j] = cur
        while q:
            a, b = q.popleft()
            for da, db in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                c, d = a + da, b + db
                if 0 <= c < nx and 0 <= d < ny and M[c, d] and not lab[c, d]:
                    lab[c, d] = cur
                    q.append((c, d))
    groups = {}
    for name, x, y in pts:
        groups.setdefault(lab[int(round(x / res)), int(round(y / res))], []).append(name)
    if len(groups) > 1:
        return False, f'piano di massa diviso in {len(groups)} isole: {list(groups.values())}', set()
    return True, '', {name for name, x, y in pts}


if __name__ == '__main__':
    import sys
    check(json.load(open(sys.argv[1] if len(sys.argv) > 1 else 'routing.json')))
