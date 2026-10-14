"""Anteprima SVG della scheda (rame, piano di massa, serigrafia, forature)."""
from board import BOARD_W, BOARD_H, MOUNT_HOLES, MOUNT_DRILL, COMPONENTS, placed_pads, body_rect

S = 12  # pixel per mm


def svg_preview(path, tracks=(), vias=(), plane_clear=(), title='', layers=('bot', 'top', 'silk'), labels=True):
    W, H = BOARD_W * S, BOARD_H * S
    o = []
    a = o.append
    a(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="-20 -40 {W+40} {H+60}" width="{W+40}" height="{H+60}" font-family="Helvetica, Arial, sans-serif">')
    a('<rect x="-20" y="-40" width="100%" height="100%" fill="#ffffff"/>')
    a(f'<text x="0" y="-15" font-size="16" font-weight="bold">{title}</text>')
    a(f'<rect x="0" y="0" width="{W}" height="{H}" fill="#1f5c3a" stroke="#000" stroke-width="2"/>')
    if 'bot' in layers:
        # piano di massa (azzurro) con le aree sgombrate
        a(f'<rect x="{0.5*S}" y="{0.5*S}" width="{W-S}" height="{H-S}" fill="#4a7fb5" opacity="0.55"/>')
        for kind, *g in plane_clear:
            if kind == 'c':
                x, y, r = g
                a(f'<circle cx="{x*S}" cy="{y*S}" r="{r*S}" fill="#1f5c3a"/>')
            elif kind == 'r':
                x0, y0, x1, y1 = g
                a(f'<rect x="{x0*S}" y="{y0*S}" width="{(x1-x0)*S}" height="{(y1-y0)*S}" fill="#1f5c3a"/>')
            elif kind == 's':
                x0, y0, x1, y1, w = g
                a(f'<line x1="{x0*S}" y1="{y0*S}" x2="{x1*S}" y2="{y1*S}" stroke="#1f5c3a" stroke-width="{w*S}" stroke-linecap="round"/>')
        for t in tracks:
            if t['layer'] == 'bot':
                pts = ' '.join(f'{x*S:.1f},{y*S:.1f}' for x, y in t['pts'])
                a(f'<polyline points="{pts}" fill="none" stroke="#2f6fd0" stroke-width="{t["w"]*S}" stroke-linecap="round" stroke-linejoin="round"/>')
    pads = placed_pads()
    if 'top' in layers:
        for t in tracks:
            if t['layer'] == 'top':
                pts = ' '.join(f'{x*S:.1f},{y*S:.1f}' for x, y in t['pts'])
                a(f'<polyline points="{pts}" fill="none" stroke="#d94a3d" stroke-width="{t["w"]*S}" stroke-linecap="round" stroke-linejoin="round" opacity="0.92"/>')
    for p in pads:
        col = '#e8b04a' if p['drill'] else '#e05a4a'
        if p['shape'] == 'circle':
            a(f'<circle cx="{p["x"]*S}" cy="{p["y"]*S}" r="{p["w"]/2*S}" fill="{col}"/>')
        else:
            a(f'<rect x="{(p["x"]-p["w"]/2)*S}" y="{(p["y"]-p["h"]/2)*S}" width="{p["w"]*S}" height="{p["h"]*S}" fill="{col}"/>')
        if p['drill']:
            a(f'<circle cx="{p["x"]*S}" cy="{p["y"]*S}" r="{p["drill"]/2*S}" fill="#111"/>')
    for v in vias:
        a(f'<circle cx="{v["x"]*S}" cy="{v["y"]*S}" r="{0.4*S}" fill="#c9c9c9"/>')
        a(f'<circle cx="{v["x"]*S}" cy="{v["y"]*S}" r="{0.2*S}" fill="#111"/>')
    for (x, y) in MOUNT_HOLES:
        a(f'<circle cx="{x*S}" cy="{y*S}" r="{MOUNT_DRILL/2*S}" fill="#111"/>')
    if 'silk' in layers:
        for ref, (fp, x, y, rot, val, nets) in COMPONENTS.items():
            br = body_rect(ref)
            if br:
                x0, y0, x1, y1 = br
                a(f'<rect x="{x0*S}" y="{y0*S}" width="{(x1-x0)*S}" height="{(y1-y0)*S}" fill="none" stroke="#fff" stroke-width="1.2"/>')
            else:
                a(f'<circle cx="{(x+1.75)*S}" cy="{y*S}" r="{4*S}" fill="none" stroke="#fff" stroke-width="1.2"/>')
            if labels:
                a(f'<text x="{x*S}" y="{y*S-4}" font-size="9" fill="#fff" text-anchor="middle">{ref}</text>')
    if labels:
        for p in pads:
            a(f'<text x="{p["x"]*S}" y="{p["y"]*S+3}" font-size="6" fill="#000" text-anchor="middle">{p["net"]}</text>')
    a('</svg>')
    open(path, 'w', encoding='utf-8').write('\n'.join(o))


if __name__ == '__main__':
    import sys
    svg_preview(sys.argv[1], title='Piazzamento')
