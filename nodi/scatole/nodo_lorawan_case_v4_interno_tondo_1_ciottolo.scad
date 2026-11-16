// =====================================================================
//  Versione 4 DA INTERNO, "TONDA": stesso interno della versione da interno
//  (piastra dei sensori con SCD41, SGP41, BME688 o MPL3115A2, ZE07-CO,
//  pila con le lamelle, Heltec, niente GPS), ma con la forma di un ciottolo:
//    - pianta ad angoli molto arrotondati;
//    - bordo che si rastrema verso il muro (il fianco scende curvo e si
//      assottiglia contro la parete: niente spigoli verso la stanza);
//    - coperchio con una bombatura centrale marcata (8 mm).
//  Il coperchio si stampa con la cupola in alto, con supporti ad albero solo
//  DENTRO (il soffitto e' sospeso); il labbro di centraggio e' sulla scatola. Si appende con due asole a chiave nel
//  fondo (niente alette), una in basso a sinistra e una in alto a destra.
//  Tutte le aperture (USB, antenna, asole di aerazione) attraversano il
//  fianco curvo; USB e antenna hanno una nicchia dall'esterno.
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
z_skirt = 12;      // altezza a cui la parte che si allarga raggiunge la spalla
dome_h = 8;        // bombatura del coperchio
z_split = inner_h; // piano tra scatola e coperchio (22,5)
z_sh = z_split + 4;// fin qui il coperchio ha il fianco dritto, poi comincia la cupola
n_sl = 10;         // fette per le curve
keyholes = [[20, 12], [70, 64]];   // asole a chiave nel fondo (X, Y del foro grande)

sx = inner_l + 2*wall;  sy = inner_w + 2*wall;   // spalla (esterno)
cx = inner_l/2;  cy = inner_w/2;

module slab(x0, y0, z, w, h, r) { translate([x0, y0, z]) rbox([w, h, 0.01], r); }

// volume esterno del ciottolo, dal muro (Z = -floor_t) alla cima della cupola
module pebble_outer() {
    hull() {
        translate([-wall - flare, -wall - flare, -floor_t]) rbox([sx + 2*flare, sy + 2*flare, 1.2], r_foot);
        for (i = [1 : n_sl]) let(u = i / n_sl, e = flare * sqrt(2*u - u*u),
                                 z = -floor_t + 1.2 + u * (z_skirt + floor_t - 1.2), r = r_foot + (r_sh - r_foot) * u)
            slab(-wall - flare + e, -wall - flare + e, z, sx + 2*flare - 2*e, sy + 2*flare - 2*e, r);
        slab(-wall, -wall, z_sh, sx, sy, r_sh);
        for (j = [1 : n_sl]) let(th = 90 * j / n_sl, s = max(cos(th), 0.08), z = z_sh + dome_h * sin(th))
            translate([cx, cy, 0]) scale([s, s, 1]) translate([-cx, -cy, 0]) slab(-wall, -wall, z, sx, sy, r_sh);
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
    far = wall + flare + 3;
    // aerazione: asole verticali che attraversano il fianco curvo (stesse posizioni della versione da interno)
    for (x = [8 : vent_step : 72]) translate([x, -far, 2]) cube([vent_w, far + 1, 17]);                       // parete in basso
    for (y = [7, 10.5]) translate([-far, y, 3]) cube([far + 1, 2.5, 13.5]);                                    // fianco sinistro del vano
    for (y = [stack_y + 1.5 : 4 : stack_y + 15]) translate([inner_l - 1, y, 3]) cube([far + 1, 2.5, 15.5]);   // fianco destro, ingressi
    for (x = [8 : vent_step : sma_top[0] - 8]) translate([x, inner_w - 1, 2]) cube([vent_w, far + 1, 17]);  // parete in alto, uscite
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
