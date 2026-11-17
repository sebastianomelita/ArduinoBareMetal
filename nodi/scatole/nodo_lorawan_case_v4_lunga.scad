// =====================================================================
//  Versione 4 LUNGA con SDS011 (polveri sottili PM2.5 / PM10)
//  Una scatola sola, 4 viti agli angoli, un solo coperchio con 2 pannelli.
//
//  Parte sinistra = v4 con una striscia in piu' tra vano sensori e batteria:
//    parete in BASSO -> vano sensori -> BOOST 5 V + MOS + condensatore
//    -> batteria + BMS -> Heltec con GPS -> parete in ALTO
//  poi una parete interna verticale e il vano dell'SDS011:
//    - ingresso: tubo di silicone 6 x 8 dal beccuccio, curva a L, foro in BASSO;
//    - uscita: canale di 12 mm accanto alla parete interna, griglia in BASSO;
//    - DS18B20 accanto all'ingresso dell'aria;
//    - divisorio orizzontale nel canale a 2/3 dell'altezza: sotto scende lo scarico,
//      sopra (tasca riparata) stanno l'antenna SMA (X 85, non sopra il GPS) e il
//      passacavo da 9 nella parete interna, per tutti i fili e il cavetto IPEX;
//    - consiglio: fai uscire il tubo 2-3 cm sotto il collare.
//  Coperchio unico inclinato a 11,5 gradi come la v3, in due versioni (pv_count):
//  A = 2 pannelli coricati, B = 4 pannelli in piedi. La scatola e' profonda
//  36 in basso e 20,7 in alto: l'SDS011 misurato e' spesso 24 sulla ventola (alta 34,
//  che finisce a 38 dal bordo basso dopo 2 di plastica e 1-2 di basetta), poi 15 di
//  colpo; coi piedini da 3 e mezzo mm d'aria la ventola vuole 27,5 sotto il coperchio.
//  Se cambi le misure: l'assert in fondo ti dice la
//  profondita' minima, a cui portare inner_h.
//  Fili nella parete interna (passacavo da 9, nella tasca): 5 V, GND, TX, RX
//  dell'SDS011, 3 del DS18B20, 2 del secondo pannello, cavetto IPEX dell'antenna.
//  Stessa origine e stessi assi della v4.
// =====================================================================

/* [Cosa visualizzare] */
// quale coperchio: si sceglie qui (o nel Customizer); la scatola e' la stessa
coperchio_lunga = "A"; // [A:A - 2 pannelli coricati, B:B - 4 pannelli in piedi]

// modulo RS485 (MAX485 col morsetto verde): true = la scatola si allunga di 22 mm e tra il GPS e la
// parete interna nasce un vano per il modulo, con un secondo pressacavo PG7 sulla parete in basso
rs485_lunga = false; // [false, true]
rs_bay = rs485_lunga ? 22 : 0;

include <nodo_lorawan_case_v4.scad>
include <sds011_sezione.scad>

v4_render  = false;
// (qui solo numeri: queste misure la v4 le usa prima di leggere il resto)
strip_h    = 14;          // striscia elettronica (Y 16,5..30,5)
inner_w    = 75;          // 61 + 14
inner_l    = 182.4 + rs_bay;   // 77 + parete interna 2,4 + canale 12 + SDS011 71 + tubo 20
inner_h    = 36.0;        // profondita' sulla parete in BASSO: la decide la parte dell'SDS011 con la ventola (vedi l'assert in fondo)
box_tilt   = 11.5;        // come la v3
sds_mod    = [71, 70, 24];   // SDS011: spessore massimo (ventola) misurato 24
sds_t_top  = 15;          // parte alta dell'SDS011 (senza ventola): misurata 15
sds_fan_h  = 38;          // dal bordo basso fin dove finisce la ventola: 1-2 di basetta + 2 di plastica + 34 di ventola (poi 15 di colpo)
ds_hole    = false;       // il DS18B20 entra nel vano SDS011
sds_side   = false;
// due coperchi per la stessa scatola (si sceglie con coperchio_lunga, in cima al file):
//   A -> 2 pannelli coricati, uno sopra la Heltec e uno sopra l'SDS011
//   B -> 4 pannelli in piedi affiancati (sporge 9 mm per lato, scende 12 sotto la scatola)
pv_count   = coperchio_lunga == "B" ? 4 : 1;
pv_gap2    = 6;           // in piedi: 6 mm tra due pannelli (ci stanno le due lamelle a scatto con le loro fessure)
pv_xs      = pv_count == 1 ? [41, 77 + rs_bay + wall + 51.5]
                           : [for (i = [0 : pv_count - 1]) inner_l/2 + (i - (pv_count - 1)/2) * (pv_w + pv_gap2)];
ext_x0     = pv_count == 1 ? -wall + 0.5                 // A: scende davanti a tutta la parete in basso (visiera)
                           : min(-wall, inner_l/2 - pv_count*pv_w/2 - (pv_count - 1)*pv_gap2/2 - pv_side - 3.2);
ext_w      = pv_count == 1 ? inner_l + 2*wall - 1 : inner_l - 2*ext_x0;
// vano SDS011 dentro la stessa scatola
s_x0       = 77 + rs_bay; // faccia sinistra della parete interna: subito a destra della guida del GPS (76,9)
s_bay      = 0;
s_top_gap  = 1.5;         // 1,5 + 70 + 3,5 = 75
s_bot_gap  = 3.5;
sma_side   = "top";
sma_top    = [85 + rs_bay, 8];     // antenna SMA sopra il canale di scarico, oltre la parete: il dado non sta davanti al GPS
baffle_y   = 50;          // divisorio orizzontale nel canale di scarico, a 2/3 dell'altezza: sopra resta una tasca riparata
mid_cable  = [64, 10];    // passacavo nella parete interna, nella tasca: fili dell'SDS011, del DS18B20, del pannello 2 e cavetto IPEX
mid_cable_d = 10;         // (anche i 3 fili del sensore del terreno)
soil_ds_yz = [45, 12];    // DS18B20 del terreno, sotto il pressacavo (sopra le mollette del tubo)
gland_yz   = [60, 12];    // pressacavo PG7 sulla parete destra del vano SDS011, sopra la curva del tubo

assert(!is_undef(v4_render), "MANCA nodo_lorawan_case_v4.scad: deve stare nella stessa cartella, con questo nome esatto");
assert(!is_undef(s_ix1), "MANCA sds011_sezione.scad: deve stare nella stessa cartella, con questo nome esatto");
assert(abs(s_ix1 - inner_l) < 0.01, "inner_l non torna con le misure del vano SDS011");
sds_need_h = max(sds_pad_h + sds_mod[2] + 0.5 + (m_y0 + sds_fan_h) * tan(box_tilt),
                 sds_pad_h + sds_t_top + 0.5 + m_y1 * tan(box_tilt));
assert(inner_h >= sds_need_h - 0.01, str("inner_h deve essere almeno ", sds_need_h));
echo(str("profondita' minima per l'SDS011: ", sds_need_h, " (inner_h = ", inner_h, ")"));

baccello_uv = "no";  // [no, si] "si" = baccello del sensore UV sulla parete in alto, accanto alla Heltec
uv_x   = 38;      // al centro del vano della Heltec (X 0..77)
uv_pod = (baccello_uv == "si");   // non toccare: la comanda la voce qui sopra
part = "assembly"; // [assembly, base, lid, plate, print]   (plate = piastra estraibile dei sensori)

// parete interna: fino a 0,6 sotto il coperchio, con le tacche per il labbro alle due estremita'
module mid_wall() {
    difference() {
        intersection() {
            translate([s_x0, 0, 0]) cube([wall, inner_w, 60]);
            below_rim(-seal_gap);
        }
        intersection() {
            union() {
                translate([s_x0 - 1, -1, 0]) cube([wall + 2, 2.2 + 1, 80]);
                translate([s_x0 - 1, inner_w - 2.2, 0]) cube([wall + 2, 3.2, 80]);
            }
            above_rim(-lid_lip_h - 0.3);
        }
        translate([s_x0 - 1, mid_cable[0], mid_cable[1]]) rotate([0, 90, 0]) cylinder(d = mid_cable_d, h = wall + 2);
    }
}
// divisorio nel canale di scarico: l'aria della ventola (uscita in basso a sinistra del sensore)
// scende alla griglia; sopra restano il dado dell'antenna e il passacavo, fuori dal flusso
module baffle() {
    intersection() {
        translate([s_ix0 - 0.01, baffle_y, 0]) cube([m_x0 - 0.4 - s_ix0 + 0.01, 2, 60]);
        below_rim(-seal_gap);
    }
}
// VANO DEL MODULO RS485 (solo con rs485_lunga = true): tra la guida del GPS (X 76,9) e la parete interna (X 99).
// Il modulo (basetta 44 x 14) sta coricato lungo Y, pin in giu' negli incavi del fondo, morsetto verde in basso
// (verso Y = 0) con le bocche verso sinistra: i fili A/B arrivano dal pressacavo li' sotto.
rs_xy = [82.5, 8];        // angolo in basso a sinistra della basetta
rs_gland = [88, 12];      // secondo pressacavo PG7 sulla parete in BASSO (X, Z)
module rs485_bay() {
    c = 0.5; lg = 5; t = 1.2; h = rs485_pad + 4;
    x0 = rs_xy[0]; y0 = rs_xy[1]; x1 = x0 + rs485_size[1]; y1 = y0 + rs485_size[0];
    for (y = [y0 + 7, y1 - 9]) translate([x0, y, 0]) cube([rs485_size[1], 2, rs485_pad]);     // appoggi
    for (ex = [[x0 - c - t, 1], [x1 + c, -1]], ey = [[y0 - c - t, 1], [y1 + c, -1]]) {
        translate([ex[0], ey[1] > 0 ? y0 - c - t : y1 + c - lg + t, 0]) cube([t, lg, h]);
        translate([ex[1] > 0 ? x0 - c - t : x1 + c - lg + t, ey[0], 0]) cube([lg, t, h]);
    }
}
module rs485_bay_cuts() {
    x0 = rs_xy[0]; y0 = rs_xy[1];
    for (y = [y0, y0 + rs485_size[0] - 5]) translate([x0 + 1, y, -1.4]) cube([rs485_size[1] - 2, 5, 1.5]);   // incavi dei pin
    translate([rs_gland[0], 1, rs_gland[1]]) rotate([90, 0, 0]) cylinder(d = gland_d, h = wall + 2);          // pressacavo
    if (labels) floor_label(x0 + rs485_size[1]/2, y0 + rs485_size[0]/2, "RS485", 2.5, 90);
}
module base_long() {
    difference() {
        union() { base(); mid_wall(); baffle(); sds_inner_parts(); if (rs485_lunga) rs485_bay(); }
        sds_cuts();
        if (rs485_lunga) rs485_bay_cuts();
    }
}
// coperchio: quello della v4 allungato, piu' quattro tamponi che tengono giu' l'SDS011
// (0,3 di gioco: metti un velo di gomma)
module lid_long() {
    lid();
    for (x = [m_x0 + 10, m_x0 + sds_mod[0] - 10], y = [m_y0 + 10, m_y1 - 10]) {
        top = sds_pad_h + (y < m_y0 + sds_fan_h ? sds_mod[2] : sds_t_top) + 0.3;
        intersection() {
            translate([x, y, top]) cylinder(d = 8, h = 40);
            below_rim(1);
        }
    }
}
module lid_long_print() {
    translate([0, 0, lid_T]) mirror([0, 0, 1]) multmatrix(LMinv) translate([0, 0, -inner_h]) lid_long();
}

if (part == "base") {
    base_long();
} else if (part == "lid") {
    lid_long_print();
    translate([30, min(ext_v0, v_lo) - 10, 0]) usb_plug();   // staccato dal coperchio (A o B)
    if (gland) translate([55, min(ext_v0, v_lo) - 12, 0]) gland_plug();
    if (rs485_lunga) translate([78, min(ext_v0, v_lo) - 12, 0]) gland_plug();
    if (uv_pod) translate([105, min(ext_v0, v_lo) - 12, 0]) uv_cap();
} else if (part == "plate") {
    if (sensor_plate) sensor_plate_print();
} else if (part == "print") {
    base_long();
    translate([0, inner_w + 2*wall + 14 - ext_v0, 0]) lid_long_print();
    if (uv_pod) translate([inner_l + 30, 20, 0]) uv_cap();
} else {
    base_long(); if (sensor_plate) color("Gold") sensor_plate_part(); %dummies(); %sds_dummies();
    translate([0, 0, explode]) { lid_long(); color("MidnightBlue") panels(); }
    if (uv_pod) { color("SteelBlue") translate([0, explode, 0]) uv_cap_placed(); %uv_dummy(); }
}
