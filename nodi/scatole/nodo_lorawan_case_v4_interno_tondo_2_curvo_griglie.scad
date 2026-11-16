// =====================================================================
//  Versione 4 DA INTERNO, "TONDA": stesso interno della versione da interno
//  (piastra dei sensori con SCD41, SGP41, BME688 o MPL3115A2, ZE07-CO,
//  pila con le lamelle, Heltec, niente GPS), ma con la forma di un ciottolo:
//    - pianta ad angoli molto arrotondati;
//    - bordo che si rastrema verso il muro (il fianco scende con una gola
//      curva e si assottiglia contro la parete: niente spigoli verso la stanza);
//    - una griglia di aerazione per lato, con cornice in rilievo e arco in cima;
//    - coperchio con una bombatura centrale marcata (8 mm).
//  Il coperchio si stampa con la cupola in alto, con supporti ad albero solo
//  DENTRO (il soffitto e' sospeso); il labbro di centraggio e' sulla scatola. Si appende con due asole a chiave nel
//  fondo (niente alette), una in basso a sinistra e una in alto a destra.
//  USB e antenna hanno una nicchia dall'esterno nel fianco curvo.
// =====================================================================

include <nodo_lorawan_case_v4_interno.scad>
assert(!is_undef(inner_h) && !is_undef(indoor),
       "MANCA nodo_lorawan_case_v4_interno.scad o nodo_lorawan_case_v4.scad: mettili nella STESSA cartella di questo file, con questi nomi esatti");

v4_render = false;      // il file della v4 non disegna niente da solo
ears      = false;      // si appende con le asole a chiave

/* [Forma tonda] */
part = "assembly"; // [assembly, base, lid, plate, print]
flare = 10;        // di quanto il bordo si allarga verso il muro, per lato
r_foot = 18;       // raggio degli angoli contro il muro
r_sh = 6;          // raggio degli angoli della spalla (dove si chiude il coperchio)
z_skirt = 12;      // altezza a cui la fascia curva raggiunge la spalla (sopra il fianco e' dritto)
dome_h = 8;        // bombatura del coperchio
z_split = inner_h; // piano tra scatola e coperchio (22,5)
z_sh = z_split + 4;// fin qui il coperchio ha il fianco dritto, poi comincia la cupola
n_sl = 12;         // fette per le curve
keyholes = [[20, 12], [70, 64]];   // asole a chiave nel fondo (X, Y del foro grande)

/* [Griglie: una per lato, con cornice e arco] */
// [lato, inizio, fine, Z bassa, Z alta]; lato S = in basso, N = in alto, W = sinistra, E = destra
grilles = [["S", 6, 76, 2.5, 18.5],
           ["W", 4, 28, 2.5, 18.5],
           ["E", 34, 50, 2.5, 18.5],
           ["N", 6, 60, 2.5, 18.5]];
arch_rise = 6;     // freccia dell'arco in cima alla griglia (alto 6 su tutte; mai oltre mezza larghezza)
frame_w = 2;       // larghezza della cornice
frame_t = 1.2;     // di quanto la cornice sporge dalla superficie curva
bar_margin = 1.2;  // bordo pieno tra la cornice e le asole

sx = inner_l + 2*wall;  sy = inner_w + 2*wall;   // spalla (esterno)
cx = inner_l/2;  cy = inner_w/2;

module slab(x0, y0, z, w, h, r) { translate([x0, y0, z]) rbox([w, h, 0.01], r); }

// fetta i della fascia curva: profilo a quarto di cerchio (gola), sottile contro il muro
function sk_e(i) = flare * sin(90 * i / n_sl);                       // rientro dal piede
function sk_z(i) = -floor_t + 1.2 + (z_skirt + floor_t - 1.2) * (1 - cos(90 * i / n_sl));
module sk_slice(i, o) {
    e = sk_e(i) - o;
    slab(-wall - flare + e, -wall - flare + e, sk_z(i), sx + 2*flare - 2*e, sy + 2*flare - 2*e,
         r_foot + (r_sh - r_foot) * sk_e(i) / flare + o);
}
// volume esterno del ciottolo, dal muro (Z = -floor_t) alla cima della cupola; o = ingrossato di o
module pebble_outer(o = 0) {
    // piede verticale di 1.2
    translate([-wall - flare - o, -wall - flare - o, -floor_t]) rbox([sx + 2*flare + 2*o, sy + 2*flare + 2*o, 1.2], r_foot + o);
    // fascia curva: a tratti (hull di due fette vicine), cosi' resta concava
    for (i = [0 : n_sl - 1]) hull() { sk_slice(i, o); sk_slice(i + 1, o); }
    // fianco dritto e cupola (convessi: un solo hull)
    hull() {
        slab(-wall - o, -wall - o, z_skirt, sx + 2*o, sy + 2*o, r_sh + o);
        slab(-wall - o, -wall - o, z_sh, sx + 2*o, sy + 2*o, r_sh + o);
        for (j = [1 : n_sl]) let(th = 90 * j / n_sl, sc = max(cos(th), 0.08), z = z_sh + (dome_h + o) * sin(th))
            translate([cx, cy, 0]) scale([sc, sc, 1]) translate([-cx, -cy, 0]) slab(-wall - o, -wall - o, z, sx + 2*o, sy + 2*o, r_sh + o);
    }
}

// --- griglie: profilo 2D (a = lungo la parete, Z) di una finestra con l'arco in cima
module arch2d(a0, a1, z0, z1) {
    w = a1 - a0;
    h = min(arch_rise, w/2, z1 - z0);
    R = (w*w/4 + h*h) / (2*h);
    translate([a0, z0]) square([w, z1 - h - z0 + 0.01]);
    intersection() {
        translate([a0 + w/2, z1 - R]) circle(r = R, $fn = 120);
        translate([a0, z1 - h]) square([w, h]);
    }
}
// estrude un profilo 2D (a, Z) perpendicolare al lato, tra le coordinate n0 e n1 (Y per S/N, X per W/E)
module put(side, n0, n1) {
    if (side == "S" || side == "N")
        translate([0, n1, 0]) rotate([90, 0, 0]) linear_extrude(n1 - n0) children();
    else
        translate([n0, 0, 0]) rotate([0, 0, 90]) rotate([90, 0, 0]) linear_extrude(n1 - n0) children();
}
far = wall + flare + frame_t + 3;
// [fuori: da .. a] (tutto quello che sta fuori dalla parete) e [asole: da .. a] (attraverso la parete)
function g_out(side) = side == "S" ? [-far, -wall] : side == "N" ? [inner_w + wall, inner_w + far]
                     : side == "W" ? [-far, -wall] : [inner_l + wall, inner_l + far];
function g_thr(side) = side == "S" ? [-far, 1] : side == "N" ? [inner_w - 1, inner_w + far]
                     : side == "W" ? [-far, 1] : [inner_l - 1, inner_l + far];
module grille_frames() {
    for (g = grilles) intersection() {
        put(g[0], g_out(g[0])[0], g_out(g[0])[1]) difference() {
            offset(r = frame_w) arch2d(g[1], g[2], g[3], g[4]);
            arch2d(g[1], g[2], g[3], g[4]);
        }
        pebble_outer(frame_t);
    }
}
module grille_cuts() {
    for (g = grilles) {
        // vano (nicchia) fino alla faccia esterna della parete: piu' profondo in basso, dove la fascia e' spessa
        put(g[0], g_out(g[0])[0], g_out(g[0])[1]) arch2d(g[1], g[2], g[3], g[4]);
        // asole verticali (larghe vent_w, passo vent_step) dentro la finestra, con un bordo pieno
        put(g[0], g_thr(g[0])[0], g_thr(g[0])[1]) intersection() {
            offset(delta = -bar_margin) arch2d(g[1], g[2], g[3], g[4]);
            let(w = g[2] - g[1] - 2*bar_margin, n = floor((w + vent_step - vent_w) / vent_step),
                a = g[1] + (g[2] - g[1] - (n - 1) * vent_step - vent_w) / 2)
                for (k = [0 : n - 1]) translate([a + k * vent_step, g[3] - 1]) square([vent_w, g[4] - g[3] + 2]);
        }
    }
}
// vuoto dentro il coperchio: fianco dritto fino a z_sh, poi cupola piu' bassa di uno spessore
module lid_cavity() {
    ri = corner_r - wall;
    hull() {
        slab(0, 0, z_split - 1, inner_l, inner_w, ri);
        slab(0, 0, z_sh, inner_l, inner_w, ri);
        for (j = [1 : n_sl]) let(th = 90 * j / n_sl, s = max(cos(th), 0.08), z = z_sh + (dome_h - wall) * sin(th))
            translate([cx, cy, 0]) scale([s, s, 1]) translate([-cx, -cy, 0]) slab(0, 0, z, inner_l, inner_w, ri);
    }
}

// ---------------------------------------------------------------------
module base_round() {
    ri = corner_r - wall;
    difference() {
        union() {
            // fianchi curvi (senza fondo dentro: il fondo, con le scritte, viene dalla v4)
            difference() {
                intersection() {
                    pebble_outer();
                    translate([-300, -300, -floor_t]) cube([600, 600, z_split + floor_t]);
                }
                translate([0, 0, -floor_t - 1]) rbox([inner_l, inner_w, 100], ri);
            }
            // fondo e tutto quello che c'e' dentro, presi dalla v4 da interno (senza le sue pareti)
            intersection() {
                base();
                translate([0, 0, -floor_t - 0.01]) rbox([inner_l, inner_w, z_split + floor_t + 0.02], ri);
            }
            grille_frames();
            // labbro di centraggio, attaccato alla faccia interna delle pareti: parte 3 sotto il piano
            // e sale di 3 dentro il coperchio (sopra lo ZE07 e sulle colonnine e' interrotto)
            difference() {
                translate([0, 0, z_split - 3]) rbox([inner_l, inner_w, 3 + lid_lip_h], ri);
                translate([lid_lip_t, lid_lip_t, z_split - 4]) cube([inner_l - 2*lid_lip_t, inner_w - 2*lid_lip_t, 10]);
                for (p = col_pos) translate([p[0], p[1], z_split]) cylinder(d = col_d + 1, h = 10);
                if (ze07) translate([ze07_xy[0] - 1, -1, z_split - 4]) cube([ze07_size[0] + 2, 3, 4]);
            }
        }
        round_cuts_base();
    }
}
module round_cuts_base() {
    // aerazione: una griglia per lato, con cornice e arco
    grille_cuts();
    // USB-C: asola nella parete + nicchia dall'esterno nel fianco curvo (lo spinotto arriva alla presa)
    translate([0, usb_y, usb_z]) hull()
        for (dy = [-(usb_w - usb_h)/2, (usb_w - usb_h)/2])
            translate([-wall - 1, dy, 0]) rotate([0, 90, 0]) cylinder(d = usb_h, h = wall + 2);
    translate([-far - 2, usb_y - 9, usb_z - 6.5]) cube([far + 2 - wall + 0.01, 18, 13]);
    // antenna SMA sulla parete in alto + nicchia per il dado esterno e l'antenna
    translate([sma_top[0], inner_w - 1, sma_top[1]]) rotate([-90, 0, 0]) cylinder(d = sma_d, h = wall + 2);
    translate([sma_top[0], inner_w + wall - 0.01, sma_top[1]]) rotate([-90, 0, 0]) cylinder(d = 14, h = far);
    // asole a chiave nel fondo: foro grande per la testa, poi la scatola scende e la vite entra nell'asola
    for (k = keyholes) {
        translate([k[0], k[1], -floor_t - 1]) cylinder(d = 9, h = floor_t + 2);
        translate([k[0] - 2.3, k[1], -floor_t - 1]) cube([4.6, 9, floor_t + 2]);
    }
}

// ---------------------------------------------------------------------
module lid_round() {
    difference() {
        union() {
            difference() {
                intersection() {
                    pebble_outer();
                    translate([-300, -300, z_split]) cube([600, 600, 100]);
                }
                lid_cavity();
                // sede del labbro: in basso il vuoto e' piu' largo di 0,3 per 3,3 mm
                translate([-lid_clear, -lid_clear, z_split - 1])
                    rbox([inner_l + 2*lid_clear, inner_w + 2*lid_clear, 1 + lid_lip_h + lid_clear], corner_r - wall + lid_clear);
            }
            // torrette delle viti: scendono fino alla cima delle colonnine della scatola
            // (tagliate dalla forma esterna, cosi' agli angoli non bucano la cupola)
            intersection() {
                for (p = col_pos) translate([p[0], p[1], z_split]) cylinder(d = col_d, h = z_sh - z_split + dome_h);
                pebble_outer();
            }
        }
        for (p = col_pos) {
            translate([p[0], p[1], z_split - 1]) cylinder(d = screw_clear_d, h = 60);
            translate([p[0], p[1], z_split + 2]) cylinder(d = screw_head_d, h = 60);   // la testa sta 2 mm sopra il piano
        }
        // asole nella cupola: sopra il vano sensori e (aerazione "massima") sopra le schede
        for (x = [8 : 4 : 74]) translate([x, 2.5, z_split - 1]) cube([2, sw_y - 4.5, 60]);
        if (aerazione == "massima") {
            for (x = [8 : 4 : 72]) translate([x, stack_y + 4, z_split - 1]) cube([2, 15, 60]);
            for (x = [8 : 4 : 72]) translate([x, stack_y + 22, z_split - 1]) cube([2, inner_w - 5 - stack_y - 22, 60]);
        }
    }
}
module lid_round_print() {
    translate([0, 0, -z_split]) lid_round();     // piano di chiusura sul piatto, cupola in alto
}

// ---------------------------------------------------------------------
if (part == "base") {
    base_round();
} else if (part == "lid") {
    lid_round_print();
    translate([inner_l + 30, 20, 0]) usb_plug();
} else if (part == "plate") {
    sensor_plate_print();
} else if (part == "print") {
    base_round();
    translate([inner_l + 40, 0, 0]) lid_round_print();
} else {
    base_round();
    color("Gold") sensor_plate_part();
    %dummies();
    translate([0, 0, explode]) lid_round();
}
