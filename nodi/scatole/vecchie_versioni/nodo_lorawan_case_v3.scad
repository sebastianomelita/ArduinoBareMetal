// =====================================================================
//  Contenitore COMPATTO per nodo LoRaWAN  (versione 3: bordi inclinati)
//  Heltec WiFi LoRa 32 V4 + batteria 14500 + BMS 1S + GPS
//  + vano sensori STAGNO con SCD41 e MPL3115A2
//
//  Come la v2, ma il bordo della scatola e' tagliato da un piano inclinato:
//  profonda 22,5 sulla parete in basso (sensori, alti 19) e 13,4 su quella
//  in alto (Heltec, alta 13). Il coperchio e' piatto e inclinato di
//  box_tilt verso il cielo: il pannello solare si incolla direttamente
//  sopra, senza cuneo: un pannello coricato o due in piedi, stessa scatola.
//  Il GPS (L76K, 17,5 x 16,5 x 5,5) sta in piedi accanto al vano sensori,
//  con l'antenna verso la parete in alto: guarda il cielo attraverso di lei,
//  mai coperto dai pannelli. L'antenna LoRa esce dritta in alto.
//
//  Installazione: a muro, lato lungo orizzontale.
//    parete frontale (Y = 0)        -> guarda in BASSO (feritoia sensori)
//    parete posteriore (Y = inner_w) -> guarda in ALTO
//  Antenna SMA sulla parete in alto.
//
//  Misure in mm. Origine: angolo interno sinistro-anteriore del fondo.
//  X = lunghezza, Y = larghezza, Z = profondita' dal fondo verso l'esterno.
//  Quote dei componenti da verificare col calibro (vedi v2).
// =====================================================================

/* [Cosa visualizzare] */
part = "assembly"; // [assembly, base, lid, print]
explode = 20;

/* [Scatola] */
inner_l = 105.5;   // lunghezza interna (X)
inner_w = 44.5;    // larghezza interna (Y): culla batteria 17 + Heltec 25,6 + margini
inner_h = 22.5;    // profondita' interna sulla parete in BASSO (Y = 0); verso l'alto cala con box_tilt
box_tilt = 11.5;   // inclinazione del bordo (gradi): il coperchio guarda un po' verso il cielo
wall    = 2.4;
floor_t = 2.4;
lid_t   = 2.4;
corner_r = 4;      // raggio esterno degli spigoli = raggio delle colonnine d'angolo
$fn = 48;

/* [Coperchio] */
lid_lip_h = 3;
lid_lip_t = 1.6;
lid_clear = 0.3;

/* [Colonnine d'angolo per le viti del coperchio] */
// concentriche con lo spigolo esterno arrotondato: il contorno della scatola le contiene
col_d = 2*corner_r;            // Ø 8
col_c = corner_r - wall;       // centro a 1,6 dall'angolo interno, in X e in Y
screw_pilot_d = 2.6;   // autofilettante M3 (ISO 7049 ST2,9)
screw_pilot_depth = 12;   // in alto le colonnine sono alte solo 13,8
screw_clear_d = 3.4;
screw_head_d  = 6.2;   // testa bombata fino a 6 mm
screw_head_h  = 1.6;

/* [Heltec WiFi LoRa 32 V4] */
h_len = 51.6;      // lungo X (lato USB a sinistra)
h_wid = 25.6;      // lungo Y
h_pcb = 1.7;
h_x   = 6;         // distanza del lato USB dalla parete sinistra: spinotti, cavi e colonnina d'angolo
h_y   = 17.6;
h_so_h = 5.5;      // colonnine: i connettori sotto la scheda sporgono 3,6
h_so_d = 4.2;      // fusto sottile in alto: a 4,2 mm da un foro c'e' il connettore IPEX (lato inferiore)
h_so_base_d = 7;   // base allargata (rastremata verso l'alto): piu' resistente alla flessione
h_so_flare_h = 2.5;// altezza del raccordo conico: sotto i connettori della scheda, che scendono fino a Z 1,9
h_pilot_d = 1.8;   // vite M2 autofilettante
h_pilot_depth = 5;
// fori: [distanza dal lato USB, distanza dal lato lungo]  -> DA VERIFICARE
h_holes = [[17, 5.8], [17, 19.8], [46, 5.8], [46, 19.8]];
// pulsanti (sopra la scheda): [dal lato USB, dal lato lungo]
h_btns  = [[3.45, 5.1], [3.45, 20.5]];
usb_w = 12;        // asola per lo spinotto USB-C (la presa e' 6 mm dentro: deve passare lo spinotto intero)
usb_h = 6.5;       // va bene per quasi tutti i cavi; si chiude col tappo a scatto stampato col coperchio
usb_up = 1.6;      // centro della presa sopra la faccia superiore della scheda

/* [Batteria 14500] */
batt_d = 14.5;
batt_l = 53;       // misura la tua cella: le protette arrivano a 53
batt_x = 6;        // inizio del primo fermo (prima c'e' la colonnina d'angolo)
batt_yc = 8.15;    // asse della cella: a 0,9 dalla parete in basso
batt_span = 17;    // larghezza delle selle (da Y = 0)
stop_t = 2;
saddle_w = 8;
saddle_snap = 2.75; // le selle salgono 2,75 sopra l'asse della cella: la batteria entra a scatto (niente fascette)

/* [BMS 1S in piedi, tra batteria e Heltec] */
bms_l = 35.2;
bms_h = 4.2;       // la larghezza della scheda diventa l'altezza
bms_t = 2.0;       // spessore con i componenti
bms_clr = 0.4;
bms_wall = 1.2;
bms_slot_h = 6;
bms_x = 13.5;      // inizio della scheda: tra la batteria e la Heltec, parallela alla batteria
bms_y = 17.6;      // faccia verso la batteria (dietro le selle); sopra c'e' il bordo della Heltec (Z 5,5)

/* [GPS L76K: modulo 17,5 x 16,5 x 5,5 in piedi, antenna verso la parete in alto] */
gps = true;
gps_mod = [17.5, 16.5, 5.5];   // lungo X (connettore a sinistra, cavo verso la Heltec), alto, spesso (ingombro totale)
gps_x = 58.4;                  // subito a destra della Heltec: il cavo corto originale passa sotto la scheda
gps_y = 17.8;                  // retro (chip e connettore); l'antenna ceramica guarda +Y, cioe' il cielo a scatola appesa
gps_clr = 0.2;                 // gioco per lato nelle guide
gps_wall = 0.8;                // spessore delle guide

/* [Vano sensori stagno] */
ch_x   = 69.5;     // faccia esterna della parete sinistra del vano
ch_wall = 2;
ch_depth = 14.5;   // spazio tra parete frontale e parete dei sensori
seal_gap = 0.6;    // spazio sotto il coperchio per il cordone di silicone
vent_len = 22;     // UNA sola feritoia, sulla parete che guarda in basso
vent_h   = 2.5;
vent_z   = 1.2;    // a filo del fondo: scarica anche la condensa
so_h = 4;          // distanziali dei sensori sulla parete
so_d = 4.5;
so_pilot_d = 1.8;
so_pilot_depth = 5;   // cieco: la parete resta stagna
wire_d = 3.5;      // passacavo da sigillare con silicone
board_z = 1;       // bordo inferiore dei sensori sopra il fondo
// SCD41 (Adafruit 4867) CORICATO sulla parete dei sensori: 25,4 in orizzontale x 17,8 di altezza
scd_w = 25.4;
scd_h = 17.8;
scd_holes = [[2.54, 2.54], [22.86, 2.54], [2.54, 15.26], [22.86, 15.26]];
// MPL3115A2 (scheda rossa, un foro) in piedi sulla parete SINISTRA del vano,
// chip rivolto verso la parete -> MISURARE
mpl_w = 12;        // lungo Y
mpl_h = 18;        // in altezza
mpl_y0 = 2;        // bordo della scheda verso la parete in basso
mpl_holes = [[6, 13.5]];

/* [Passanti] */
ds_d = 4.5;        // foro sigillato per il cavo DS18B20 (parete in basso)
ds_x = 66.5;
ds_z = 12;
sma_d = 6.6;       // antenna SMA
sma_top = [95, 6]; // parete in ALTO (X, Z): antenna dritta verso il cielo; il dado sta sotto il labbro, accanto al GPS

/* [Pannelli solari 80 x 45 incollati sul coperchio] */
pv_count = 1;      // [0, 1, 2] 1 = un pannello coricato; 2 = due in piedi affiancati (il coperchio scende 31 mm sotto la scatola)
pv_w = 45;
pv_l = 80;
// sede a incastro ricavata nello spessore del coperchio (il pannello entra scivolando dall'alto)
pv_edge = 2.0;     // spessore del pannello sul BORDO, valore massimo (circuito + velo di resina: 1,5-2)
pv_edge_min = 1.5; // spessore minimo sul bordo: su questo si calcola il dentino
pv_groove = 0.6;   // gola: altezza in piu' sotto il labbro, assorbe le imprecisioni dello scivolo e dello spessore
pv_side = 0.4;     // gioco laterale per lato (la gola entra anche di lato)
pv_dome = 3.0;     // spessore al CENTRO, in cima alla resina bombata (2,5-3)
pv_gap = 1.5;      // intercapedine dietro il pannello: fili saldati ai contatti e saldature d'angolo (fino a 1-1,5)
pv_ledge = 1.5;    // gradino sui lati che regge il retro del pannello sopra l'intercapedine
pv_lip = 1.8;      // di quanto il labbro copre il bordo del pannello (anche col pannello tutto da un lato resta 1,4)
pv_lip_t = 1.0;    // spessore del labbro
pv_clr = 0.2;      // gioco per lato e sopra il bordo
snap_l = 9;        // linguette a scatto sopra il pannello: lunghezza (si flettono verso fuori)
snap_t = 0.9;      // spessore della linguetta
snap_hook = 0.6;   // quanto il dentino scende sotto il bordo alto del pannello (calcolato sul bordo piu' sottile)
snap_room = 11;    // spazio tra il bordo alto del pannello e il bordo alto del coperchio
pv_t = pv_dome;
visor = 12;        // 1 pannello: quanto scende il coperchio sotto la scatola, davanti alla feritoia (0 = niente)
ext_rib = 6;       // nervature dietro il tratto che scende: profonde 6 in alto, 2 in fondo
pv_hole_d = 5;     // fori per i fili dei pannelli, nel fondo della sede (si sigillano con silicone)

/* [Alette a muro] */
ears = true;
ear_len = 12;
ear_w   = 20;
ear_t   = 4;
ear_slot_d = 4.5;

/* [Etichette] */
labels = true;
label_depth = 0.6;
label_font = "Liberation Sans:style=Bold";

// ---------------------------------------------------------------------
//  Quote derivate
// ---------------------------------------------------------------------
tk = tan(box_tilt);
function rim(y) = inner_h - y * tk;    // altezza del bordo (= faccia interna del coperchio) alla quota y
col_pos = [[col_c, col_c], [inner_l - col_c, col_c],
           [col_c, inner_w - col_c], [inner_l - col_c, inner_w - col_c]];
usb_y = h_y + h_wid/2;
usb_z = h_so_h + h_pcb + usb_up;
ch_in_x0 = ch_x + ch_wall;
sw_y = ch_depth;
scd_x = inner_l - 0.6 - scd_w;
vent_x = (ch_in_x0 + inner_l)/2 - vent_len/2;
bms_in_w = bms_t + 2*bms_clr;
bms_in_l = bms_l + 1.2;
// coperchio: riferimento sul piano del bordo (x = X, v = lungo la pendenza verso l'alto, w = uscente)
LM = [[1, 0, 0, 0], [0, cos(box_tilt), sin(box_tilt), 0], [0, -sin(box_tilt), cos(box_tilt), inner_h], [0, 0, 0, 1]];
LMinv = [[1, 0, 0, 0], [0, cos(box_tilt), -sin(box_tilt), 0], [0, sin(box_tilt), cos(box_tilt), 0], [0, 0, 0, 1]];
v_lo  = -wall / cos(box_tilt);                  // bordo basso del coperchio (lungo la pendenza)
v_hi  = (inner_w + wall) / cos(box_tilt);       // bordo alto
// pannelli: 1 coricato (80 in X) sotto il GPS, oppure 2 in piedi affiancati (45 in X, 80 in pendenza)
pv_shift = 0;                                   // il GPS sta contro la parete in alto: il pannello resta centrato
pv_xs  = pv_count == 2 ? [inner_l/2 - pv_w/2 - 1, inner_l/2 + pv_w/2 + 1] : [inner_l/2 + pv_shift];
pv_dx  = pv_count == 2 ? pv_w : pv_l;           // larghezza (X) di un pannello
pv_dv  = pv_count == 2 ? pv_l : pv_w;           // lunghezza (pendenza) di un pannello
pv_vtop = v_hi - 1;                             // bordo alto dei pannelli (v)
ext_x0 = pv_count == 2 ? inner_l/2 - pv_w - 4 : inner_l/2 + pv_shift - pv_l/2 - 3;
ext_w  = pv_count == 2 ? 2*pv_w + 8 : pv_l + 6;
ext_v0 = pv_count == 1 ? v_lo - visor / cos(box_tilt)   // fondo del coperchio (v)
                       : v_hi - snap_room - pv_dv - pv_clr - 3;   // 2 pannelli: 3 mm di bordo sotto la sede
pv_hole_v = 25;                                 // fori dei fili: sopra la Heltec
pad_h = pv_count > 0 ? pv_gap + pv_edge + pv_groove + pv_lip_t : 0;   // spessore aggiunto al coperchio per la sede (5,1)
lid_T = lid_t + pad_h;                          // spessore totale del coperchio
pv_v0 = v_hi - snap_room - pv_dv - pv_clr;      // fondo della sede: il pannello ci appoggia (chiusa in basso, aperta in alto)
pv_vt = pv_v0 + pv_clr + pv_dv;                 // bordo alto del pannello inserito
pv_hole_vs = pv_count == 1 ? pv_v0 + pv_clr + pv_dv/2 : pv_hole_v;   // 1 pannello: foro al centro, sotto i contatti

function hpos(p) = [h_x + p[0], h_y + p[1]];

module rbox(size, r) {
    rr = max(min(r, size[0]/2 - 0.01, size[1]/2 - 0.01), 0.01);
    hull() for (x = [rr, size[0] - rr], y = [rr, size[1] - rr])
        translate([x, y, 0]) cylinder(r = rr, h = size[2]);
}
module ltext(s, size) {
    linear_extrude(label_depth + 1)
        text(s, size = size, font = label_font, halign = "center", valign = "center");
}
module floor_label(x, y, s, size, rot = 0) {
    translate([x, y, -label_depth]) rotate([0, 0, rot]) ltext(s, size);
}
// semispazi sotto / sopra il piano del bordo, spostato in verticale di dz
module below_rim(dz = 0) {
    translate([0, 0, inner_h + dz]) rotate([-box_tilt, 0, 0]) translate([-500, -500, -1000]) cube([1000, 1000, 1000]);
}
module above_rim(dz = 0) {
    translate([0, 0, inner_h + dz]) rotate([-box_tilt, 0, 0]) translate([-500, -500, 0]) cube([1000, 1000, 1000]);
}

// ---------------------------------------------------------------------
//  Elementi interni (uguali alla v2)
// ---------------------------------------------------------------------
module heltec_standoffs() {
    // colonnine rastremate: base larga (raccordo conico) e fusto sottile sotto la scheda
    for (p = h_holes) translate(concat(hpos(p), 0))
        difference() {
            union() {
                cylinder(d = h_so_d, h = h_so_h);
                cylinder(d1 = h_so_base_d, d2 = h_so_d, h = h_so_flare_h);
            }
            translate([0, 0, h_so_h - h_pilot_depth]) cylinder(d = h_pilot_d, h = h_pilot_depth + 1);
        }
}

module battery_cradle() {
    zc = batt_d/2 + 1;
    x0 = batt_x + stop_t;                 // inizio della cella
    for (xs = [x0 + 6, x0 + batt_l - 6 - saddle_w])
        translate([xs, 0, 0]) difference() {
            // sella a scatto: l'apertura in cima e' piu' stretta della cella (14,1 contro 14,5)
            cube([saddle_w, batt_span, zc + saddle_snap]);
            translate([-1, batt_yc, zc]) rotate([0, 90, 0]) cylinder(d = batt_d + 0.6, h = saddle_w + 2);
        }
    for (xs = [batt_x, x0 + batt_l])
        translate([xs, batt_yc - 7.15, 0]) cube([stop_t, 14.3, batt_d * 0.6]);
}

// BMS in piedi, parallelo alla batteria, sotto il bordo della Heltec (che sta a Z 5,5):
// parete verso la batteria tra le due selle, pareti verso la Heltec a tratti (lasciano
// libere le basi coniche delle colonnine della Heltec), due fermi alle estremita'
module bms_slot() {
    hh = 4;
    y_out = bms_y + bms_t + 0.2;
    translate([batt_x + stop_t + batt_l - 6 - saddle_w - 25, bms_y - 1.0, 0]) cube([25, 0.8, hh]);   // tra le selle
    for (xr = [[bms_x - 0.2, 19.0], [27.0, 44.0]]) translate([xr[0], y_out, 0]) cube([xr[1] - xr[0], 0.8, hh]);
    translate([bms_x - 0.9, bms_y - 0.2, 0]) cube([0.7, bms_t + 0.4, 3]);
    translate([bms_x + bms_l + 0.2, bms_y - 0.2, 0]) cube([0.7, bms_t + 0.2, 3]);
}

// GPS in piedi, antenna verso la parete in alto (il cielo): entra dall'alto in due
// guide a U che lo prendono per l'ingombro esterno (una sul fondo, una verticale a
// destra); col coperchio chiuso resta bloccato. Il lato sinistro resta libero per il cavo.
module gps_cradle() {
    y0 = gps_y - gps_clr - gps_wall;  y1 = gps_y + gps_mod[2] + gps_clr;   // pareti dietro / davanti
    x1 = gps_x + gps_mod[0] + gps_clr;
    // guida sul fondo (alta 1,5): pareti dietro e davanti
    for (y = [y0, y1]) translate([gps_x + 1.5, y, 0]) cube([x1 - gps_x - 1.5, gps_wall, 1.5]);
    // guida verticale a destra: fondo della U + pareti dietro e davanti, alta quanto il modulo
    translate([x1, y0, 0]) cube([gps_wall, y1 + gps_wall - y0, gps_mod[1]]);
    for (y = [y0, y1]) translate([x1 - 2.5, y, 0]) cube([2.5, gps_wall, gps_mod[1]]);
}
module gps_dummy() {
    translate([gps_x, gps_y, 0]) cube([gps_mod[0], gps_mod[2], gps_mod[1]]);
}

// pareti del vano: fino a 0,6 sotto il coperchio (cordone di silicone),
// abbassate vicino alle pareti esterne dove passa il labbro
module sensor_chamber() {
    intersection() {
        union() {
            translate([ch_x, 0, 0]) cube([ch_wall, sw_y + ch_wall, inner_h]);
            translate([ch_x, sw_y, 0]) cube([inner_l - ch_x, ch_wall, inner_h]);
        }
        below_rim(-seal_gap);
    }
    for (h = scd_holes) sensor_boss(scd_x + h[0], board_z + h[1]);
    for (h = mpl_holes) mpl_boss(mpl_y0 + h[0], board_z + h[1]);
    translate([ch_in_x0, mpl_y0, 0]) cube([so_h + 2.1, mpl_w, board_z]);
}
module chamber_notches() {
    intersection() {
        union() {
            translate([ch_x - 1, -1, 0]) cube([ch_wall + 2, 2.2 + 1, 40]);
            translate([inner_l - 2.2, sw_y - 1, 0]) cube([3.2, ch_wall + 2, 40]);
        }
        above_rim(-lid_lip_h - 0.3);
    }
}
module sensor_boss(x, z) {
    translate([x, sw_y, z]) rotate([90, 0, 0]) cylinder(d = so_d, h = so_h);
}
module mpl_boss(y, z) {
    translate([ch_in_x0, y, z]) rotate([0, 90, 0]) cylinder(d = so_d, h = so_h);
}
module sensor_boss_holes() {
    for (h = scd_holes) boss_hole(scd_x + h[0], board_z + h[1]);
    for (h = mpl_holes)
        translate([ch_in_x0 + so_h + 0.01, mpl_y0 + h[0], board_z + h[1]])
            rotate([0, -90, 0]) cylinder(d = so_pilot_d, h = so_pilot_depth);
}
module boss_hole(x, z) {
    translate([x, sw_y - so_h - 0.01, z]) rotate([-90, 0, 0]) cylinder(d = so_pilot_d, h = so_pilot_depth);
}

module ear(side) {
    x_wall = side < 0 ? -wall : inner_l + wall;
    x_tip  = x_wall + side * ear_len;
    translate([0, inner_w/2, -floor_t]) difference() {
        hull() {
            translate([x_wall - side * 2, -ear_w/2, 0]) cube([0.01, ear_w, ear_t]);
            translate([x_tip - side * ear_w/4, 0, 0]) cylinder(d = ear_w/2, h = ear_t);
        }
        translate([x_wall + side * ear_len/2, 0, -1]) cylinder(d = ear_slot_d, h = ear_t + 2);
    }
}


// ---------------------------------------------------------------------
//  Base
// ---------------------------------------------------------------------
module base() {
    difference() {
        union() {
            intersection() {
                union() {
                    difference() {
                        translate([-wall, -wall, -floor_t])
                            rbox([inner_l + 2*wall, inner_w + 2*wall, inner_h + floor_t + 5], corner_r);
                        rbox([inner_l, inner_w, inner_h + 10], corner_r - wall);
                    }
                    for (p = col_pos) translate([p[0], p[1], 0]) cylinder(d = col_d, h = inner_h + 5);
                }
                below_rim(0);
            }
            heltec_standoffs();
            battery_cradle();
            bms_slot();
            if (gps) gps_cradle();
            sensor_chamber();
            if (ears) { ear(-1); ear(1); }
        }
        chamber_notches();
        // fori pilota verticali nelle colonnine d'angolo
        for (p = col_pos) translate([p[0], p[1], rim(p[1]) - screw_pilot_depth])
            cylinder(d = screw_pilot_d, h = screw_pilot_depth + 5);
        sensor_boss_holes();
        // passacavi del vano (da sigillare)
        translate([scd_x + 5, sw_y - 1, board_z + scd_h/2])
            rotate([-90, 0, 0]) cylinder(d = wire_d, h = ch_wall + 2);
        translate([ch_x - 1, mpl_y0 + 3, 4])
            rotate([0, 90, 0]) cylinder(d = wire_d, h = ch_wall + 2);
        // finestra USB (parete sinistra)
        translate([0, usb_y, usb_z]) hull()
            for (dy = [-(usb_w - usb_h)/2, (usb_w - usb_h)/2])
                translate([-wall - 1, dy, 0]) rotate([0, 90, 0]) cylinder(d = usb_h, h = wall + 2);
        // UNICA feritoia del vano sensori (parete in basso)
        translate([vent_x, -wall - 1, vent_z]) cube([vent_len, wall + 2, vent_h]);
        // foro sigillato per il cavo DS18B20 (parete in basso)
        translate([ds_x, 1, ds_z]) rotate([90, 0, 0]) cylinder(d = ds_d, h = wall + 2);
        // antenna SMA (parete destra)
        translate([sma_top[0], inner_w - 1, sma_top[1]]) rotate([-90, 0, 0]) cylinder(d = sma_d, h = wall + 2);
        if (labels) {
            floor_label(h_x + 31.5, h_y + h_wid/2, "HELTEC V4", 3);
            floor_label(batt_x + stop_t + batt_l/2, batt_yc, "14500", 4);
            if (gps) floor_label(gps_x + gps_mod[0]/2, 32, "GPS", 3);
            floor_label(bms_x + bms_l/2, bms_y + bms_t/2, "BMS", 1.6);
            floor_label((ch_in_x0 + inner_l)/2, 4.5, "SENSORI", 3);
            floor_label(ds_x, 6, "DS18B20", 2.2, 90);
        }
    }
}

// ---------------------------------------------------------------------
//  Coperchio inclinato (coordinate della scatola)
// ---------------------------------------------------------------------
module lid() {
  union() {
    difference() {
        union() {
            // piastra sul piano del bordo, con il profilo della scatola visto in pendenza
            multmatrix(LM) translate([-wall, v_lo, 0])
                scale([1, 1/cos(box_tilt), 1]) rbox([inner_l + 2*wall, inner_w + 2*wall, lid_T], corner_r);
            // tratto che scende davanti alla feritoia e porta i pannelli, con due nervature dietro
            if (pv_count > 0 && ext_v0 < v_lo - 1) {
                multmatrix(LM) translate([ext_x0, ext_v0, 0]) cube([ext_w, v_lo - ext_v0 + 1, lid_T]);
                for (xc = [ext_x0, ext_x0 + ext_w - 2])
                    multmatrix(LM) translate([xc, 0, 0]) rotate([90, 0, 90]) linear_extrude(2)
                        polygon([[v_lo - 0.6, 0.01], [v_lo - 0.6, -ext_rib], [ext_v0, -2], [ext_v0, 0.01]]);
            }
            // labbro verticale, tra il bordo e 3 mm sotto
            intersection() {
                difference() {
                    translate([lid_clear, lid_clear, 0])
                        rbox([inner_l - 2*lid_clear, inner_w - 2*lid_clear, 40], corner_r - wall - lid_clear);
                    translate([lid_clear + lid_lip_t, lid_clear + lid_lip_t, -1])
                        rbox([inner_l - 2*(lid_clear + lid_lip_t), inner_w - 2*(lid_clear + lid_lip_t), 42], 0.01);
                }
                below_rim(0.01);
                above_rim(-lid_lip_h);
            }
        }
        // scarichi del labbro: colonnine d'angolo, sopra il SCD41 (parete destra), sopra il GPS (parete in alto)
        // (solo sotto la faccia interna del coperchio: tagliano il labbro, non la piastra)
        intersection() {
            union() {
                for (p = col_pos) translate([p[0], p[1], 0]) cylinder(d = col_d + 1, h = 40);
                translate([inner_l - 3, 5.5, 0]) cube([4, sw_y + ch_wall - 5.5, 40]);
            }
            below_rim(0);
        }
        // viti M3 verticali, con sede piana per la testa (il coperchio e' inclinato)
        for (p = col_pos) {
            translate([p[0], p[1], 0]) cylinder(d = screw_clear_d, h = 40);
            translate([p[0], p[1], rim(p[1] + screw_head_d/2) + lid_t/cos(box_tilt) - 0.3])
                cylinder(d = screw_head_d, h = 20);   // pozzetto attraverso lo spessore aggiunto
        }
        // fori per i pulsanti della Heltec (coperti solo con 2 pannelli)
        if (pv_count < 2)
            for (b = h_btns) translate([hpos(b)[0], hpos(b)[1], 0]) cylinder(d = 3, h = 40);
        // fori per i fili dei pannelli, perpendicolari al coperchio
        if (pv_count > 0) for (x = pv_xs)
            multmatrix(LM) translate([x, pv_hole_vs, -1]) cylinder(d = pv_hole_d, h = lid_t + 2);
        // sedi a incastro dei pannelli
        if (pv_count > 0) for (x = pv_xs) multmatrix(LM) { pv_seat(x); pv_snap_cuts(x); }
    }
    // dentini delle linguette a scatto
    if (pv_count > 0) for (x = pv_xs) multmatrix(LM) pv_teeth(x);
  }
}

// linguette a scatto sui due lati di ogni sede, sopra il bordo alto del pannello:
// attaccate in alto, libere in basso; il dentino scende dietro il bordo del pannello
// (vicino al lato, dove la resina e' ancora sottile) e lo blocca. Per togliere il
// pannello si solleva la linguetta con un cacciavite.
function snap_xs(xc) = let(x0 = xc - pv_dx/2 - pv_side, x1 = xc + pv_dx/2 + pv_side)
    [[x0 - 0.6, x0 + 3.0, x0 - 1.2], [x1 - 3.0, x1 + 0.6, x1 + 0.6]];   // [inizio, fine, fessura esterna]
module pv_snap_cuts(xc) {
    for (t = snap_xs(xc)) {
        // fessura esterna, fessura in fondo e luce sotto la linguetta (cosi' si flette)
        translate([t[2], pv_vt - 0.3, lid_T - snap_t - 0.4]) cube([0.6, snap_l + 0.6, snap_t + 2]);
        translate([min(t[0], t[2]), pv_vt - 0.3, lid_T - snap_t - 0.4]) cube([max(t[1], t[2] + 0.6) - min(t[0], t[2]), 0.6, snap_t + 2]);
        translate([t[0], pv_vt + 0.3, lid_T - snap_t - 0.4]) cube([t[1] - t[0], snap_l, 0.41]);
    }
}
module pv_teeth(xc) {
    w_bot = lid_t + pv_gap + pv_edge_min - snap_hook;        // fondo del dentino: scende sotto anche il bordo piu' sottile
    for (t = snap_xs(xc)) {
        xa = max(t[0], xc - pv_dx/2) + 0.2;  xb = min(t[1], xc + pv_dx/2) - 0.2;
        hull() {   // faccia di arresto verso il basso, rampa verso l'alto (dove entra il pannello)
            translate([xa, pv_vt + 0.3, w_bot]) cube([xb - xa, 0.6, lid_T - snap_t - w_bot]);
            translate([xa, pv_vt + 0.3, lid_T - snap_t - 0.01]) cube([xb - xa, 2.2, 0.01]);
        }
        // la linguetta stessa (ricostruita sopra la luce, attaccata in alto)
        translate([t[0], pv_vt + 0.3, lid_T - snap_t]) cube([t[1] - t[0], snap_l, snap_t]);
    }
}

// sede di un pannello (riferimento del coperchio): dal fondo verso l'esterno
//   intercapedine (piu' stretta: gradini laterali)  ->  bordo del pannello (larghezza piena)
//   ->  labbro che copre il bordo sottile, smussato verso fuori (la resina bombata resta libera)
// aperta in alto (il pannello entra scivolando), chiusa in basso (ci appoggia)
module pv_seat(xc) {
    x0 = xc - pv_dx/2 - pv_side;  w = pv_dx + 2*pv_side;
    len = v_hi - pv_v0 + 10;
    w_lip = lid_t + pv_gap + pv_edge + pv_groove;         // sotto del labbro: gola profonda
    li = pv_side + pv_lip;                                // rientro del labbro sui lati
    lb = pv_clr + pv_lip;                                 // rientro del labbro sul fondo
    translate([x0 + pv_side + pv_ledge, pv_v0 + pv_ledge, lid_t]) cube([w - 2*(pv_side + pv_ledge), len, pv_gap + 0.01]);
    translate([x0, pv_v0, lid_t + pv_gap]) cube([w, len, pv_edge + pv_groove + 0.01]);   // gola
    hull() {   // labbro smussato a 45 gradi verso fuori: la resina bombata resta libera
        translate([x0 + li, pv_v0 + lb, w_lip - 0.01]) cube([w - 2*li, len, 0.01]);
        translate([x0 + li - pv_lip_t, pv_v0 + lb - pv_lip_t, lid_T - 0.01]) cube([w - 2*(li - pv_lip_t), len, 1]);
    }
    // due scoli dal fondo dell'intercapedine fino al bordo basso del coperchio (l'acqua esce)
    v_bot = min(ext_v0, v_lo) - 1;
    for (f = [0.25, 0.75]) translate([x0 + w*f - 2, v_bot, lid_t]) cube([4, pv_v0 + 0.5 - v_bot, pv_gap]);
}

// pannelli (solo vista assembly): nella sede, a filo della faccia esterna
module panels() {
    if (pv_count > 0) for (x = pv_xs)
        multmatrix(LM) translate([x - pv_dx/2, pv_v0 + pv_clr, lid_t + pv_gap]) cube([pv_dx, pv_dv, pv_t]);
}

// ---------------------------------------------------------------------
//  Sagome dei componenti (solo vista assembly)
// ---------------------------------------------------------------------
module dummies() {
    color("RoyalBlue", 0.6) translate([h_x, h_y, h_so_h]) cube([h_len, h_wid, h_pcb]);
    color("Silver", 0.6) translate([batt_x + stop_t, batt_yc, batt_d/2 + 1]) rotate([0, 90, 0]) cylinder(d = batt_d, h = batt_l);
    color("Black", 0.7) translate([bms_x, bms_y, 0.5]) cube([bms_l, bms_t, bms_h]);
    if (gps) color("DarkGreen", 0.8) gps_dummy();
    color("Purple", 0.6) translate([scd_x, sw_y - so_h - 1.6, board_z]) cube([scd_w, 1.6, scd_h]);
    color("Red", 0.6) translate([ch_in_x0 + so_h, mpl_y0, board_z]) cube([1.6, mpl_w, mpl_h]);
}

// coperchio in posizione di stampa: faccia esterna sul piatto
// tappo a scatto per l'asola USB: aletta fuori, corpo cavo nella parete,
// dentino che scatta dietro la parete; linguetta per tirarlo via
module usb_plug() {
    sp = usb_w - usb_h;
    module stadium(d, h) { hull() for (x = [-sp/2, sp/2]) translate([x, 0, 0]) cylinder(d = d, h = h); }
    difference() {
        union() {
            stadium(usb_h + 2.6, 1.2);                                   // aletta (resta fuori)
            stadium(usb_h - 0.2, 1.2 + wall + 1.0);                      // corpo nella parete
            translate([0, 0, 1.2 + wall + 0.15]) hull() {                // dentino a scatto
                stadium(usb_h + 0.4, 0.3);
                translate([0, 0, 0.6]) stadium(usb_h - 0.2, 0.01);
            }
            translate([sp/2 + usb_h/2, -2.5, 0]) cube([5, 5, 1.2]);      // linguetta
        }
        translate([0, 0, 1.2]) stadium(usb_h - 1.8, 10);                 // corpo cavo: si stringe entrando
        translate([-sp/2 - usb_h, -0.4, 2.6]) cube([sp + 2*usb_h, 0.8, 10]);   // taglio: le due meta' flettono
    }
}

module lid_print() {
    translate([0, 0, lid_T]) mirror([0, 0, 1]) multmatrix(LMinv) translate([0, 0, -inner_h]) lid();
}

if (part == "base") {
    base();
} else if (part == "lid") {
    lid_print();
    translate([inner_l + 25, 20, 0]) usb_plug();      // il tappo si stampa sullo stesso piatto
} else if (part == "print") {
    base();
    translate([0, inner_w + 2*wall + 14 - ext_v0, 0]) lid_print();
    translate([inner_l + 25, 20, 0]) usb_plug();
} else {
    base();
    %dummies();
    translate([0, 0, explode]) { lid(); color("MidnightBlue") panels(); }
}
