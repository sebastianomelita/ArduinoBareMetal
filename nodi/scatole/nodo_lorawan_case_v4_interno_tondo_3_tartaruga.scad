// =====================================================================
//  Versione 4 DA INTERNO "TARTARUGA": dentro c'e' esattamente la scatola
//  della versione da interno (piastra dei sensori con SCD41, SGP41,
//  BME688 o MPL3115A2, ZE07-CO, pila con le lamelle, Heltec, niente GPS,
//  stesse pareti e stesse asole), ma fuori e' coperta da un GUSCIO tondo
//  come il carapace di una tartaruga:
//    - pianta ovale (angoli con raggio 32 contro il muro);
//    - fianchi che salgono tondi (superellisse) senza spigoli;
//    - cima con una leggera bombatura;
//    - fori esagonali come le placche del guscio: tre file in basso sui
//      fianchi (entra l'aria) e un nido d'ape sul coperchio (esce l'aria).
//  Tra guscio e scatola resta un'intercapedine: l'aria entra dai fori
//  del guscio, passa dalle asole della scatola, scalda e sale ai fori
//  del coperchio. USB e antenna hanno un tunnel che attraversa
//  l'intercapedine. Si appende con due asole a chiave nel fondo.
//  Scatola: stampa normale. Coperchio: piano di chiusura sul piatto, con
//  supporti ad albero DENTRO (il soffitto della cupola e' sospeso).
// =====================================================================

include <nodo_lorawan_case_v4_interno.scad>
assert(!is_undef(inner_h) && !is_undef(indoor),
       "MANCA nodo_lorawan_case_v4_interno.scad o nodo_lorawan_case_v4.scad: mettili nella STESSA cartella di questo file, con questi nomi esatti");

v4_render = false;      // il file della v4 non disegna niente da solo
ears      = false;      // si appende con le asole a chiave

/* [Guscio] */
part = "assembly"; // [assembly, base, lid, plate, print]
shell_H = 40;      // altezza del guscio dal muro all'inizio della bombatura in cima
shell_E = 28;      // di quanto il fianco tondo si allarga in pianta, dalla cima al muro
shell_n = 2.5;     // forma del fianco: 2 = ellisse, 3 = piu' pieno; 2.5 = guscio
crown = 3;         // bombatura in piu' sulla cima
top_half = [26, 24.25];   // mezzo "nucleo" piatto della cima (X, Y), poi + top_r
top_r = 4;
shell_t = 2.4;     // spessore del guscio
z_split = inner_h; // piano tra scatola e coperchio (22,5)
n_sl = 16;         // fette per le curve
keyholes = [[20, 12], [70, 64]];   // asole a chiave nel fondo (X, Y del foro grande)

/* [Fori esagonali] */
hex_r = 2.6;       // raggio (al vertice) dei fori esagonali
hex_p = 6;         // passo: tra due fori restano 1.5 mm
side_rows = [5, 10.2, 15.4];       // Z delle file sui fianchi
// fianco: [lato, da, a]   S = in basso, N = in alto, W = sinistra, E = destra
side_hex = [["S", 8, 74], ["W", 6, 28], ["E", 36, 72], ["N", 8, 58]];

cx = inner_l/2;  cy = inner_w/2;
far = shell_E + 30;
module slab(z, hx, hy, r) {   // rettangolo arrotondato centrato, mezze misure hx, hy
    translate([cx - hx, cy - hy, z]) rbox([2*hx, 2*hy, 0.01], r);
}
// guscio: per ogni fetta, il nucleo + un raggio che cala come una superellisse
// o = ingrossa (per i tagli); t = 0 esterno, shell_t interno
module shell_solid(t = 0, o = 0) {
    E = shell_E - t + o;  H = shell_H - t + o;  z0 = -floor_t;
    hull() {
        for (i = [0 : n_sl]) let(ph = 90 * i / n_sl,
                                 w = pow(cos(ph), 2/shell_n), tz = pow(sin(ph), 2/shell_n), R = top_r + E * w)
            slab(z0 + H * tz, top_half[0] + R, top_half[1] + R, R);
        // bombatura della cima: paraboloide (raccordo dolce col fianco)
        for (j = [1 : 8]) let(s = 1 - 0.85 * j / 8)
            translate([cx, cy, 0]) scale([s, s, 1]) translate([-cx, -cy, 0])
                slab(z0 + H + (crown + o) * (1 - s*s), top_half[0] + top_r, top_half[1] + top_r, top_r);
    }
}
module half_space(above) {
    translate([-300, -300, above ? z_split : -50]) cube([600, 600, above ? 100 : z_split + 50]);
}
// esagono a punta in su (stampabile in orizzontale)
module hex2d() { rotate(90) circle(r = hex_r, $fn = 6); }
// estrude un profilo 2D (a = lungo il lato, Z) perpendicolare al lato, tra n0 e n1 (Y per S/N, X per W/E)
module put(side, n0, n1) {
    if (side == "S" || side == "N")
        translate([0, n1, 0]) rotate([90, 0, 0]) linear_extrude(n1 - n0) children();
    else
        translate([n0, 0, 0]) rotate([0, 0, 90]) rotate([90, 0, 0]) linear_extrude(n1 - n0) children();
}
// da fuori fino alla faccia esterna della parete della scatola
function g_out(side) = side == "S" ? [-far, -wall] : side == "N" ? [inner_w + wall, inner_w + far]
                     : side == "W" ? [-far, -wall] : [inner_l + wall, inner_l + far];

// tunnel per USB e antenna attraverso l'intercapedine: [lato, a, Z, larghezza, altezza] (vuoto)
tunnels = [["W", usb_y, usb_z, 16, 12],
           ["N", sma_top[0], sma_top[1] + 0.5, 12.5, 12.5]];
module tunnel2d(t, grow) {
    translate([t[1], t[2]]) offset(r = 2 + grow) square([t[3] - 4, t[4] - 4], center = true);
}

// ---------------------------------------------------------------------
module base_round() {
    difference() {
        union() {
            // guscio sotto il piano di chiusura, col fondo pieno fuori dalla scatola
            difference() {
                intersection() { shell_solid(); half_space(false); }
                intersection() { shell_solid(shell_t); translate([-300, -300, 0]) cube([600, 600, 100]); }
                translate([-wall, -wall, -floor_t - 1]) rbox([inner_l + 2*wall, inner_w + 2*wall, floor_t + 1.01], corner_r);
            }
            // la scatola da interno, intera (fondo con le scritte, pareti con le asole, tutto l'interno)
            base();
            // labbro di centraggio sulla meta' esterna delle pareti: sale di 3
            difference() {
                translate([-wall, -wall, z_split - 0.01]) rbox([inner_l + 2*wall, inner_w + 2*wall, lid_lip_h], corner_r);
                translate([-wall/2, -wall/2, z_split - 1]) rbox([inner_l + wall, inner_w + wall, 10], corner_r - wall/2);
                for (p = col_pos) translate([p[0], p[1], z_split - 1]) cylinder(d = col_d + 1, h = 10);
            }
            // pareti dei tunnel (USB e antenna)
            for (t = tunnels) intersection() {
                put(t[0], g_out(t[0])[0], g_out(t[0])[1]) tunnel2d(t, 1.6);
                shell_solid();
            }
        }
        // fori esagonali sui fianchi (solo nel guscio, la scatola ha le sue asole)
        for (g = side_hex) put(g[0], g_out(g[0])[0], g_out(g[0])[1])
            for (r = [0 : len(side_rows) - 1])
                for (a = [g[1] + (r % 2) * hex_p/2 : hex_p : g[2]])
                    translate([a, side_rows[r]]) hex2d();
        // vuoto dei tunnel (USB e antenna)
        for (t = tunnels) put(t[0], g_out(t[0])[0] - 0.01, g_out(t[0])[1] + 0.01) tunnel2d(t, 0);
        // asole a chiave nel fondo: foro grande per la testa, poi la scatola scende e la vite entra nell'asola
        for (k = keyholes) {
            translate([k[0], k[1], -floor_t - 1]) cylinder(d = 9, h = floor_t + 2);
            translate([k[0] - 2.3, k[1], -floor_t - 1]) cube([4.6, 9, floor_t + 2]);
        }
    }
}

// ---------------------------------------------------------------------
module lid_round() {
    difference() {
        union() {
            difference() {
                intersection() { shell_solid(); half_space(true); }
                shell_solid(shell_t);
            }
            intersection() {
                union() {
                    // "tenda": parete sopra la meta' interna delle pareti della scatola; il labbro ci entra
                    // e separa l'intercapedine dallo spazio sopra le schede
                    difference() {
                        translate([-wall/2 + lid_clear + 0.01, -wall/2 + lid_clear + 0.01, z_split])
                            rbox([inner_l + wall - 2*lid_clear, inner_w + wall - 2*lid_clear, 60], corner_r - wall/2);
                        translate([1.2, 1.2, z_split - 1]) rbox([inner_l - 2.4, inner_w - 2.4, 70], 1);
                    }
                    // torrette delle viti
                    for (p = col_pos) translate([p[0], p[1], z_split]) cylinder(d = col_d, h = 60);
                }
                shell_solid(0.01);
            }
        }
        for (p = col_pos) {
            translate([p[0], p[1], z_split - 1]) cylinder(d = screw_clear_d, h = 80);
            translate([p[0], p[1], z_split + 2]) cylinder(d = screw_head_d, h = 80);   // la testa sta 2 mm sopra il piano
        }
        // nido d'ape sul coperchio: sopra il vano sensori sempre, sopra le schede con aerazione "massima"
        for (j = [0 : 14], i = [0 : 14]) let(x = 6 + i * hex_p + (j % 2) * hex_p/2, y = 4 + j * hex_p * sqrt(3)/2)
            if (x <= inner_l - 6 && y <= inner_w - 4
                && min([for (p = col_pos) norm([x - p[0], y - p[1]])]) > col_d/2 + hex_r + 2
                && (aerazione == "massima" || y < sw_y - 2))
                translate([x, y, z_split - 1]) rotate(30) cylinder(r = hex_r, h = 80, $fn = 6);
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
} else if (part == "plate") {
    sensor_plate_print();
} else if (part == "print") {
    base_round();
    translate([inner_l + 2*shell_E + 25, 0, 0]) lid_round_print();
} else {
    base_round();
    color("Gold") sensor_plate_part();
    %dummies();
    translate([0, 0, explode]) lid_round();
}
