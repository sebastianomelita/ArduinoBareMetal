"""Router su griglia (A*, due strati) con piano di massa sul lato inferiore.

Regole (compatibili con i fornitori cinesi più comuni, che ammettono ~0,13 mm):
  isolamento rame-rame  0,25 mm (verifica), 0,30 mm durante lo sbroglio
  piste di potenza      0,8 mm; segnali 0,3 mm; massa 0,6 mm
  via                   foro 0,4 mm, anello 0,8 mm
"""
import heapq, math, json
import numpy as np
from board import BOARD_W, BOARD_H, MOUNT_HOLES, MOUNT_DRILL, POWER_NETS, ZONES, placed_pads

G = 0.2
NX, NY = int(round(BOARD_W / G)) + 1, int(round(BOARD_H / G)) + 1
CLR_ROUTE = 0.30
EDGE_CLR = 0.5
VIA_D, VIA_DRILL = 0.8, 0.4
W_POWER, W_SIG, W_GND = 0.8, 0.3, 0.6
COST_BOT, COST_VIA, COST_TURN = 4.0, 25.0, 0.6

XS = np.arange(NX) * G
YS = np.arange(NY) * G


def width_of(net):
    if net == 'GND':
        return W_GND
    return W_POWER if net in POWER_NETS else W_SIG


# ------------------------------------------------------------ oggetti di rame
def pad_objects(p):
    layers = ['top', 'bot'] if p['drill'] else ['top']
    out = []
    for L in layers:
        if p['shape'] == 'circle':
            out.append(dict(net=p['net'], layer=L, kind='cap', x0=p['x'], y0=p['y'], x1=p['x'], y1=p['y'], r=p['w'] / 2, src=p['ref']))
        else:
            out.append(dict(net=p['net'], layer=L, kind='rect', x0=p['x'] - p['w'] / 2, y0=p['y'] - p['h'] / 2,
                            x1=p['x'] + p['w'] / 2, y1=p['y'] + p['h'] / 2, src=p['ref']))
    return out


def seg_object(net, layer, a, b, w):
    return dict(net=net, layer=layer, kind='cap', x0=a[0], y0=a[1], x1=b[0], y1=b[1], r=w / 2, src='track')


def via_objects(net, x, y):
    return [dict(net=net, layer=L, kind='cap', x0=x, y0=y, x1=x, y1=y, r=VIA_D / 2, src='via') for L in ('top', 'bot')]


def cells_within(obj, r):
    """Maschera (indici) delle celle il cui centro dista <= r dall'oggetto."""
    if obj['kind'] == 'rect':
        x0, y0, x1, y1 = obj['x0'], obj['y0'], obj['x1'], obj['y1']
        ext = r
    else:
        x0, y0, x1, y1 = min(obj['x0'], obj['x1']), min(obj['y0'], obj['y1']), max(obj['x0'], obj['x1']), max(obj['y0'], obj['y1'])
        ext = r + obj['r']
    i0, i1 = max(0, int(math.floor((x0 - ext) / G))), min(NX - 1, int(math.ceil((x1 + ext) / G)))
    j0, j1 = max(0, int(math.floor((y0 - ext) / G))), min(NY - 1, int(math.ceil((y1 + ext) / G)))
    if i0 > i1 or j0 > j1:
        return None
    X = XS[i0:i1 + 1][:, None]
    Y = YS[j0:j1 + 1][None, :]
    if obj['kind'] == 'rect':
        dx = np.maximum(np.maximum(x0 - X, X - x1), 0)
        dy = np.maximum(np.maximum(y0 - Y, Y - y1), 0)
        d = np.hypot(dx, dy)
        m = d <= r
    else:
        ax, ay, bx, by = obj['x0'], obj['y0'], obj['x1'], obj['y1']
        vx, vy = bx - ax, by - ay
        L2 = vx * vx + vy * vy
        if L2 == 0:
            t = 0
        else:
            t = np.clip(((X - ax) * vx + (Y - ay) * vy) / L2, 0, 1)
        d = np.hypot(X - (ax + t * vx), Y - (ay + t * vy))
        m = d <= r + obj['r']
    return (slice(i0, i1 + 1), slice(j0, j1 + 1), m)


def apply(mask, obj, r, value=True):
    c = cells_within(obj, r)
    if c is None:
        return
    si, sj, m = c
    sub = mask[si, sj]
    sub[m] = value
    mask[si, sj] = sub


# ------------------------------------------------------------ router
class Router:
    def __init__(self):
        self.pads = placed_pads()
        self.objs = []
        for p in self.pads:
            self.objs += pad_objects(p)
        for (net, x0, y0, x1, y1) in ZONES:
            self.objs.append(dict(net=net, layer='top', kind='rect', x0=x0, y0=y0, x1=x1, y1=y1, src='zone'))
        self.tracks = []   # dict(net, layer, w, pts)
        self.vias = []     # dict(net, x, y)

    def blocked(self, net, r_extra):
        """Maschere [top, bot] delle celle vietate per un oggetto di raggio r_extra della rete net."""
        B = np.zeros((2, NX, NY), dtype=bool)
        for k, L in enumerate(('top', 'bot')):
            for o in self.objs:
                if o['layer'] == L and o['net'] != net:
                    apply(B[k], o, r_extra + CLR_ROUTE)
            for (hx, hy) in MOUNT_HOLES:
                apply(B[k], dict(kind='cap', x0=hx, y0=hy, x1=hx, y1=hy, r=MOUNT_DRILL / 2), r_extra + 0.6)
        e = int(math.ceil((EDGE_CLR + r_extra) / G))
        B[:, :e, :] = True; B[:, -e:, :] = True; B[:, :, :e] = True; B[:, :, -e:] = True
        return B

    def own_cells(self, net, members):
        """Celle (layer, i, j) coperte dal rame già collegato della rete."""
        M = np.zeros((2, NX, NY), dtype=bool)
        for o in self.objs:
            if o['net'] == net and (o['src'] in ('track', 'via') or (o.get('pid') in members)):
                apply(M[0 if o['layer'] == 'top' else 1], o, -0.0001 if o['kind'] == 'rect' else 0)
        return M

    def pad_cells(self, p, shrink=0.05):
        M = np.zeros((2, NX, NY), dtype=bool)
        for o in pad_objects(p):
            oo = dict(o)
            if oo['kind'] == 'rect':
                oo.update(x0=oo['x0'] + shrink, y0=oo['y0'] + shrink, x1=oo['x1'] - shrink, y1=oo['y1'] - shrink)
                apply(M[0 if o['layer'] == 'top' else 1], oo, 0)
            else:
                oo['r'] = max(0.05, oo['r'] - shrink)
                apply(M[0 if o['layer'] == 'top' else 1], oo, 0)
        return M

    def astar(self, src, dst, Btr, Bvia, allow_bot=True):
        DIRS = [(1, 0), (-1, 0), (0, 1), (0, -1), (1, 1), (1, -1), (-1, 1), (-1, -1)]
        tl, ti, tj = np.nonzero(dst)
        if len(ti) == 0:
            return None
        tcx, tcy = ti.mean(), tj.mean()
        dstset = set(zip(tl.tolist(), ti.tolist(), tj.tolist()))

        def h(i, j):
            dx, dy = abs(i - tcx), abs(j - tcy)
            return (dx + dy) + (math.sqrt(2) - 2) * min(dx, dy)

        openq = []
        best = {}
        came = {}
        sl, si, sj = np.nonzero(src & ~Btr | src)
        for L, i, j in zip(sl.tolist(), si.tolist(), sj.tolist()):
            st = (L, i, j, -1)
            best[st] = 0
            heapq.heappush(openq, (h(i, j), 0, st))
        found = None
        n = 0
        while openq:
            f, g, st = heapq.heappop(openq)
            if best.get(st, 1e18) < g:
                continue
            L, i, j, d = st
            if (L, i, j) in dstset:
                found = st
                break
            n += 1
            if n > 600000:
                break
            for k, (di, dj) in enumerate(DIRS):
                ni, nj = i + di, j + dj
                if not (0 <= ni < NX and 0 <= nj < NY):
                    continue
                if Btr[L, ni, nj] and not dst[L, ni, nj]:
                    continue
                if k >= 4 and (Btr[L, i + di, j] or Btr[L, i, j + dj]) and not (dst[L, i + di, j] or dst[L, i, j + dj]):
                    continue
                step = math.sqrt(2) if k >= 4 else 1.0
                c = step * (COST_BOT if L == 1 else 1.0)
                if d != -1 and d != k:
                    c += COST_TURN
                ns = (L, ni, nj, k)
                ng = g + c
                if ng < best.get(ns, 1e18):
                    best[ns] = ng
                    came[ns] = st
                    heapq.heappush(openq, (ng + h(ni, nj), ng, ns))
            if allow_bot and not Bvia[0, i, j] and not Bvia[1, i, j]:
                ns = (1 - L, i, j, -1)
                ng = g + COST_VIA
                if ng < best.get(ns, 1e18):
                    best[ns] = ng
                    came[ns] = st
                    heapq.heappush(openq, (ng + h(i, j), ng, ns))
        if found is None:
            return None
        path = [found]
        while path[-1] in came:
            path.append(came[path[-1]])
        path.reverse()
        return [(L, i, j) for (L, i, j, d) in path]

    def commit_path(self, net, path, w):
        # spezza in tratti per strato, crea via ai cambi di strato
        segs = []
        cur = [path[0]]
        for a, b in zip(path, path[1:]):
            if a[0] != b[0]:
                self.vias.append(dict(net=net, x=a[1] * G, y=a[2] * G))
                self.objs += via_objects(net, a[1] * G, a[2] * G)
                segs.append(cur)
                cur = [b]
            else:
                cur.append(b)
        segs.append(cur)
        for s in segs:
            if len(s) < 2:
                continue
            pts = [(s[0][1], s[0][2])]
            for k in range(1, len(s) - 1):
                (x0, y0), (x1, y1), (x2, y2) = (s[k - 1][1], s[k - 1][2]), (s[k][1], s[k][2]), (s[k + 1][1], s[k + 1][2])
                if (x1 - x0, y1 - y0) != (x2 - x1, y2 - y1):
                    pts.append((x1, y1))
            pts.append((s[-1][1], s[-1][2]))
            pts = [(x * G, y * G) for x, y in pts]
            L = 'top' if s[0][0] == 0 else 'bot'
            self.tracks.append(dict(net=net, layer=L, w=w, pts=pts))
            for a, b in zip(pts, pts[1:]):
                self.objs.append(seg_object(net, L, a, b, w))

    def route_net(self, net, allow_bot=True):
        w = width_of(net)
        pads = [p for p in self.pads if p['net'] == net]
        for k, p in enumerate(pads):
            p['pid'] = (p['ref'], p['num'])
        for o in self.objs:
            pass
        # assegna pid agli oggetti dei pad
        for o, p in self._obj_pad_pairs():
            o['pid'] = (p['ref'], p['num'])
        groups = {}
        for p in pads:
            groups.setdefault(p['pid'], []).append(p)
        pids = list(groups)
        connected = {pids[0]}
        Btr = self.blocked(net, w / 2)
        Bvia = self.blocked(net, VIA_D / 2)
        while len(connected) < len(pids):
            # pad non collegato più vicino al gruppo collegato
            def dist(pid):
                p = groups[pid][0]
                return min(math.hypot(p['x'] - q['x'], p['y'] - q['y']) for c in connected for q in groups[c])
            nxt = min((pid for pid in pids if pid not in connected), key=dist)
            src = self.own_cells(net, connected)
            for c in connected:
                for p in groups[c]:
                    src |= self.pad_cells(p)
            dst = np.zeros((2, NX, NY), dtype=bool)
            for p in groups[nxt]:
                dst |= self.pad_cells(p)
            path = self.astar(src, dst, Btr, Bvia, allow_bot)
            if path is None:
                raise RuntimeError(f'Impossibile collegare {net}: {nxt}')
            self.commit_path(net, path, w)
            connected.add(nxt)
            Btr = self.blocked(net, w / 2)
            Bvia = self.blocked(net, VIA_D / 2)

    def _obj_pad_pairs(self):
        i = 0
        for p in self.pads:
            for o in pad_objects(p):
                yield self.objs[i], p
                i += 1

    def gnd_vias(self, only=None, skip=()):
        """Per ogni pad SMD di massa: breve pista sul lato top fino a una via verso il piano."""
        w = W_GND
        Btr = self.blocked('GND', w / 2)
        Bvia = self.blocked('GND', VIA_D / 2)
        for p in self.pads:
            if p['net'] != 'GND' or p['drill']:
                continue
            if (only is not None and p['ref'] not in only) or p['ref'] in skip:
                continue
            src = self.pad_cells(p)
            # BFS sul top fino alla prima cella dove si può mettere una via
            from collections import deque
            si, sj = np.nonzero(src[0])
            q = deque()
            prev = {}
            for i, j in zip(si.tolist(), sj.tolist()):
                prev[(i, j)] = None
                q.append((i, j))
            goal = None
            while q:
                i, j = q.popleft()
                if not Bvia[0, i, j] and not Bvia[1, i, j] and math.hypot(i * G - p['x'], j * G - p['y']) >= 0.9:
                    goal = (i, j)
                    break
                for di, dj in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                    ni, nj = i + di, j + dj
                    if 0 <= ni < NX and 0 <= nj < NY and (ni, nj) not in prev and (not Btr[0, ni, nj] or src[0, ni, nj]):
                        prev[(ni, nj)] = (i, j)
                        q.append((ni, nj))
            if goal is None:
                raise RuntimeError(f'Nessuna via di massa per {p["ref"]}.{p["num"]}')
            cells = [goal]
            while prev[cells[-1]] is not None:
                cells.append(prev[cells[-1]])
            path = [(0, i, j) for i, j in reversed(cells)]
            self.commit_path('GND', path, w)
            self.vias.append(dict(net='GND', x=goal[0] * G, y=goal[1] * G))
            self.objs += via_objects('GND', goal[0] * G, goal[1] * G)
            Btr = self.blocked('GND', w / 2)
            Bvia = self.blocked('GND', VIA_D / 2)


ORDER = ['QCS', 'LGATE', 'CTL', 'VSOL', 'VUSB', 'VIN_EXT', '3V3E', 'VBAT', 'VLOAD', '3V3', 'SWIN', 'SWOUT', 'BOOSTOUT', 'SENSVCC',
         'ADC', 'DIVT', 'Q2G', 'Q4G', 'Q5D', 'EN_DIV', 'EN_SENS', 'IO1', 'IO2']


def run(order=ORDER):
    R = Router()
    # pad di massa chiusi tra altri pad (U2.2): via di massa prima dello sbroglio
    EARLY = {'U2'}
    R.gnd_vias(only=EARLY)
    for net in order:
        R.route_net(net)
        print('instradata', net, flush=True)
    R.gnd_vias(skip=EARLY)
    print('vie di massa:', sum(1 for v in R.vias if v['net'] == 'GND'))
    return R


if __name__ == '__main__':
    R = run()
    json.dump(dict(tracks=R.tracks, vias=R.vias), open('routing.json', 'w'), indent=1)
