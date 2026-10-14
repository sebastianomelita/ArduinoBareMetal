"""Lettore minimo di Gerber RS-274X ed Excellon: rilegge i file prodotti e li disegna (verifica)."""
import re, sys, math
from PIL import Image, ImageDraw

PX = 20  # pixel per mm


def parse_gerber(path):
    txt = open(path).read()
    aps = {}
    for m in re.finditer(r'%ADD(\d+)([CR]),([\d.]+)(?:X([\d.]+))?\*%', txt):
        aps[int(m.group(1))] = (m.group(2), float(m.group(3)), float(m.group(4)) if m.group(4) else None)
    ops = []
    cur = None
    x = y = 0.0
    region = None
    dark = True
    for tok in re.findall(r'%[^%]*%|[^*%\n]+\*', txt):
        t = tok.strip()
        if t == '%LPD*%':
            dark = True; continue
        if t == '%LPC*%':
            dark = False; continue
        if t.startswith('%'):
            continue
        t = t.rstrip('*')
        if t == 'G36':
            region = []; continue
        if t == 'G37':
            ops.append(('region', dark, region)); region = None; continue
        m = re.fullmatch(r'D(\d+)', t)
        if m and int(m.group(1)) >= 10:
            cur = int(m.group(1)); continue
        m = re.fullmatch(r'X(-?\d+)Y(-?\d+)D0([123])', t)
        if m:
            nx, ny, d = int(m.group(1)) / 1e6, int(m.group(2)) / 1e6, m.group(3)
            if region is not None:
                if d == '2':
                    region.append([])
                region[-1].append((nx, ny)) if region else None
                if d == '1':
                    region[-1].append((nx, ny))
            elif d == '1':
                ops.append(('line', dark, aps[cur], (x, y), (nx, ny)))
            elif d == '3':
                ops.append(('flash', dark, aps[cur], (nx, ny)))
            x, y = nx, ny
    return ops


def parse_drill(path):
    tools, holes, cur = {}, [], None
    for line in open(path):
        line = line.strip()
        m = re.fullmatch(r'T(\d+)C([\d.]+)', line)
        if m:
            tools[int(m.group(1))] = float(m.group(2)); continue
        m = re.fullmatch(r'T(\d+)', line)
        if m:
            cur = int(m.group(1)); continue
        m = re.fullmatch(r'X([\d.]+)Y([\d.]+)', line)
        if m:
            holes.append((tools[cur], float(m.group(1)), float(m.group(2))))
    return holes


def render_layer(ops, W, H):
    img = Image.new('L', (int(W * PX) + 1, int(H * PX) + 1), 0)
    d = ImageDraw.Draw(img)

    def P(x, y):
        return (x * PX, (H - y) * PX)
    for op in ops:
        kind, dark = op[0], op[1]
        col = 255 if dark else 0
        if kind == 'region':
            for poly in op[2]:
                if len(poly) > 2:
                    d.polygon([P(*p) for p in poly], fill=col)
        elif kind == 'flash':
            (sh, a, b), (x, y) = op[2], op[3]
            if sh == 'C':
                r = a / 2 * PX
                cx, cy = P(x, y)
                d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=col)
            else:
                cx, cy = P(x, y)
                d.rectangle([cx - a / 2 * PX, cy - b / 2 * PX, cx + a / 2 * PX, cy + b / 2 * PX], fill=col)
        else:
            (sh, a, b), p0, p1 = op[2], op[3], op[4]
            w = a * PX
            d.line([P(*p0), P(*p1)], fill=col, width=max(1, int(round(w))))
            for p in (p0, p1):
                cx, cy = P(*p)
                d.ellipse([cx - w / 2, cy - w / 2, cx + w / 2, cy + w / 2], fill=col)
    return img


def composite(base, W, H, out_top, out_bot):
    L = {e: render_layer(parse_gerber(base + e), W, H) for e in ('.GTL', '.GBL', '.GTS', '.GBS', '.GTO', '.GKO')}
    holes = parse_drill(base + '-PTH.DRL') + parse_drill(base + '-NPTH.DRL')
    size = L['.GTL'].size
    for side, out in (('top', out_top), ('bot', out_bot)):
        img = Image.new('RGB', size, (20, 20, 20))
        sub = Image.new('RGB', size, (40, 90, 60))
        img.paste(sub, mask=Image.new('L', size, 255))
        cu = L['.GTL'] if side == 'top' else L['.GBL']
        mask = L['.GTS'] if side == 'top' else L['.GBS']
        # rame sotto maschera (verde chiaro), rame scoperto (oro)
        img.paste(Image.new('RGB', size, (70, 140, 90)), mask=cu)
        exposed = Image.fromarray(__import__('numpy').minimum(__import__('numpy').array(cu), __import__('numpy').array(mask)))
        img.paste(Image.new('RGB', size, (215, 170, 70)), mask=exposed)
        if side == 'top':
            img.paste(Image.new('RGB', size, (245, 245, 245)), mask=L['.GTO'])
        d = ImageDraw.Draw(img)
        for dia, x, y in holes:
            r = dia / 2 * PX
            cx, cy = x * PX, (H - y) * PX
            d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=(10, 10, 10))
        img.paste(Image.new('RGB', size, (255, 255, 0)), mask=L['.GKO'])
        if side == 'bot':
            img = img.transpose(Image.FLIP_LEFT_RIGHT)   # vista dal basso, come la vedrai girando la scheda
        img.save(out)


if __name__ == '__main__':
    from board import BOARD_W, BOARD_H
    composite(sys.argv[1], BOARD_W, BOARD_H, sys.argv[2], sys.argv[3])
