// =====================================================================
//  Contenitore COMPATTO per nodo LoRaWAN  (versione 2)
//  Heltec WiFi LoRa 32 V4 + batteria 14500 + BMS 1S + GPS
//  + vano sensori STAGNO con SCD41 e MPL3115A2 montati in verticale
//
//  Installazione: a muro, lato lungo orizzontale.
//    parete frontale (Y = 0)        -> guarda in BASSO
//    parete posteriore (Y = inner_w) -> guarda in ALTO
//
//  Pianta (vista dall'alto, Y = 0 in basso):
//    - batteria contro la parete in basso, a sinistra
//    - Heltec dietro la batteria, su 4 colonnine, USB a sinistra
//    - BMS in verticale in una fessura tra Heltec e GPS
//    - GPS a destra, in alto
//    - vano sensori stagno a destra, in basso, con UNA feritoia
//      verso il basso: SCD41 coricato sulla parete del vano,
//      MPL3115A2 in piedi sulla parete sinistra del vano
//
//  Misure in mm. Origine: angolo interno sinistro-anteriore del fondo.
//  X = lunghezza, Y = larghezza, Z = altezza.
//
//  Quote Heltec V4: ingombri dal modello 3D di Printables (1644103);
//  posizione dei 4 fori di fissaggio ricavata da una foto, usando il
//  passo dei pin (2,54) come riferimento -> CONTROLLARE con il calibro.
// =====================================================================

/* [Cosa visualizzare] */
part = "assembly"; // [assembly, base, lid, roof, visor, print]
explode = 20;

/* [Scatola] */
inner_l = 105.5;   // lunghezza interna (X)
inner_w = 44.5;    // larghezza interna (Y): culla batteria 17 + Heltec 25,6 + margini
inner_h = 22.5;    // altezza interna (Z): SCD41 coricato (17,8) + labbro del coperchio
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
screw_pilot_depth = 14;
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
usb_w = 13;
usb_h = 7.5;
usb_up = 1.6;      // centro della presa sopra la faccia superiore della scheda

/* [Batteria 14500] */
batt_d = 14.5;
batt_l = 53;       // misura la tua cella: le protette arrivano a 53
batt_x = 6;        // inizio del primo fermo (prima c'e' la colonnina d'angolo)
batt_yc = 8.15;    // asse della cella: a 0,9 dalla parete in basso
batt_span = 17;    // larghezza delle selle (da Y = 0)
stop_t = 2;
saddle_w = 8;
tie_w = 4;
tie_h = 1.8;

/* [BMS 1S in verticale] */
bms_l = 35.2;
bms_h = 4.2;       // la larghezza della scheda diventa l'altezza
bms_t = 2.0;       // spessore con i componenti
bms_clr = 0.4;
bms_wall = 1.2;
bms_slot_h = 6;
bms_x = 63.9;      // faccia esterna sinistra della fessura
bms_y = inner_w - 0.1 - (bms_l + 1.2);   // fessura appoggiata alla parete in alto

/* [GPS (misure da verificare)] */
gps_l = 30;
gps_w = 25;
gps_x = 69.5;
gps_y = 18;
gps_inset = 2.5;
gps_so_h = 5;
gps_so_d = 5;

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
sma_d = 6.6;       // antenna SMA (parete in alto)
sma_x = 63.5;      // tra Heltec e GPS: il dado interno non tocca nulla
sma_z = 12;

/* [Pannelli solari: due da 80 x 45] */
pv_mount = "tilt";     // [roof: tettoia sopra la scatola, lid: portapannelli inclinato sul coperchio, flat: coperchio prolungato in basso con pannelli verticali, tilt: coperchio a cuneo in un pezzo unico, pannelli in piedi e inclinati, top: un pannello coricato sul bordo superiore, none]
pv_count = 2;          // [1, 2] coperchio a cuneo: 2 pannelli in piedi affiancati, oppure 1 pannello coricato (80 in orizzontale)
tilt_visor = 15;       // coperchio a cuneo con 1 pannello: tratto senza pannello sotto il pannello, davanti alla feritoia
vis_tilt = 15;         // portapannelli: inclinazione verso l'alto rispetto alla verticale (gradi)
vis_t = 3;             // spessore della piastra (i pannelli si incollano sulla faccia esterna)
vis_feet_y = [20, 36]; // quote Y dei piedini, avvitati da dentro il coperchio (viti M2x6)
/* [Tettoia] */
roof = true;           // mostra i pannelli nella vista assembly
pv_w = 45;             // pannello: lato lungo X (affiancati); per l'altra disposizione scambia pv_w e pv_l
pv_l = 80;             // pannello: lato lungo la pendenza
pv_t = 3;              // spessore pannello -> MISURARE (= profondita' della tasca)
pv_tilt = 45;          // inclinazione sull'orizzontale (gradi): stampabile senza supporti
pv_rim = 2;            // bordi della vaschetta
pv_base = 2;           // fondo della vaschetta
pv_slot = 14;          // fessura tra i pannelli: ci passa l'antenna
roof_plate_t = 4;      // piastra a muro
roof_clear = 3;        // aria tra il bordo della scatola e la tettoia
rib_t = 2;
rib_len = 23;          // la nervatura resta sopra il bordo della scatola
pv_hole_d = 4.5;       // foro sigillato per il cavo dei pannelli (parete in alto)
pv_hole_x = 10;        // sopra lo spazio degli spinotti della Heltec (connettore Sol)
pv_hole_z = 14;

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
col_pos = [[col_c, col_c], [inner_l - col_c, col_c],
           [col_c, inner_w - col_c], [inner_l - col_c, inner_w - col_c]];
usb_y = h_y + h_wid/2;
usb_z = h_so_h + h_pcb + usb_up;
ch_in_x0 = ch_x + ch_wall;            // inizio interno del vano
sw_y = ch_depth;                      // faccia della parete sensori verso il vano
ch_top = inner_h - seal_gap;          // altezza delle pareti del vano
ch_top_edge = inner_h - lid_lip_h - 0.3;  // vicino alle pareti esterne passa il labbro
scd_x = inner_l - 0.6 - scd_w;        // SCD41 accostato alla parete destra
vent_x = (ch_in_x0 + inner_l)/2 - vent_len/2;
bms_in_w = bms_t + 2*bms_clr;
bms_in_l = bms_l + 1.2;

// tettoia
pw = pv_w + 0.6;  pl = pv_l + 0.6;
roof_len = 4*pv_rim + 2*pw + pv_slot;
roof_dep = pl + 2*pv_rim;
roof_x0  = sma_x - (2*pv_rim + pw + pv_slot/2);       // fessura centrata sull'antenna
roof_y0  = inner_w + wall + 1.1;                       // piede della piastra, sopra la scatola
roof_zf  = -floor_t + roof_plate_t;                    // faccia anteriore della piastra
roof_ya  = inner_w + wall + roof_clear
         + (floor_t + inner_h + lid_t - roof_plate_t) * tan(pv_tilt);  // attacco della vaschetta
roof_top = roof_ya + 24;
// portapannelli sul coperchio
lid_z  = inner_h + lid_t;                       // faccia superiore del coperchio
vis_px = pv_l + 2;                              // larghezza (X): pannelli col lato lungo orizzontale
vis_s  = 2*pv_w + 4;                            // lunghezza lungo la pendenza: due pannelli uno sotto l'altro
vis_x0 = inner_l/2 - vis_px/2;
vis_y0 = inner_w + wall;                        // il bordo alto poggia sul bordo alto del coperchio
vis_feet_x = [vis_x0 + 3.5, vis_x0 + vis_px - 3.5];
vis_cable = [inner_l/2, 30];                    // foro del cavo nel coperchio, riparato dalla piastra
tilt_ang = 15;                                  // tilt: inclinazione verso l'alto rispetto alla verticale (gradi)
tilt_t = 3;                                     // tilt: spessore della piastra dei pannelli
tilt_s = pv_count == 2 ? pv_l + 2               // tilt, 2 pannelli: lato lungo (80) lungo la pendenza
                       : pv_w + 2 + tilt_visor; // tilt, 1 pannello: 45 in pendenza + parte bassa che ripara la feritoia
tilt_w = inner_l + 2*wall;                      // tilt: piastra larga quanto il coperchio
tilt_px = pv_count == 2 ? [inner_l/2 - pv_w/2 - 1, inner_l/2 + pv_w/2 + 1] : [inner_l/2];   // centri X dei pannelli
tilt_groove = pv_count == 2 ? tilt_s - 10 : pv_w - 8;          // canalina dietro il pannello
tilt_hole_u = 15;                               // fori dei fili: a 15 mm dal bordo alto della piastra
tilt_M = [[1, 0, 0, -wall], [0, -cos(tilt_ang), sin(tilt_ang), vis_y0], [0, sin(tilt_ang), cos(tilt_ang), lid_z], [0, 0, 0, 1]];
tilt_Minv = [[1, 0, 0, 0], [0, -cos(tilt_ang), sin(tilt_ang), 0], [0, sin(tilt_ang), cos(tilt_ang), 0], [0, 0, 0, 1]];
// top: un pannello 80 x 45 sul bordo superiore (la parete in alto prosegue oltre il coperchio)
top_x0 = inner_l/2 - pv_l/2 - 1;                // piazzola del pannello (X), 1 mm di margine
top_len = pv_l + 2;
top_zmax = -floor_t + pv_w;                     // l'aletta arriva dove finisce il pannello (45 dal muro)
top_pad = 1.5;                                  // piazzola rialzata: dentro corre la canalina dei fili
top_hole = [inner_l/2 - pv_l/2 + 3, 14];        // foro dei fili nella parete in alto (X, Z), sotto il pannello
sma_right = [30, 12];                           // top: antenna sulla parete destra (Y, Z), antenna a gomito verso l'alto
flat_rib = 6;                                   // nervature dietro il prolungamento: profonde 6 in alto, 2 in fondo
flat_hole_top = [inner_l/2, vis_y0 - 1 - pv_w/2];          // fili del pannello alto: nel coperchio, sotto il pannello
flat_hole_bot = [inner_l/2, vis_y0 - 3 - 1.5*pv_w];        // fili del pannello basso: nel prolungamento
flat_wall_hole_z = 16.5;                                   // e poi nella parete in basso, al riparo del prolungamento
vis_M = [[1, 0, 0, vis_x0], [0, -cos(vis_tilt), sin(vis_tilt), vis_y0], [0, sin(vis_tilt), cos(vis_tilt), lid_z], [0, 0, 0, 1]];
vis_Minv = [[1, 0, 0, 0], [0, -cos(vis_tilt), sin(vis_tilt), 0], [0, sin(vis_tilt), cos(vis_tilt), 0], [0, 0, 0, 1]];

function hpos(p) = [h_x + p[0], h_y + p[1]];   // da coordinate scheda a coordinate scatola

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

// ---------------------------------------------------------------------
//  Elementi interni
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
            cube([saddle_w, batt_span, zc + 1]);
            translate([-1, batt_yc, zc]) rotate([0, 90, 0]) cylinder(d = batt_d + 0.6, h = saddle_w + 2);
            translate([(saddle_w - tie_w)/2, -1, 0]) cube([tie_w, batt_span + 2, tie_h]);
        }
    for (xs = [batt_x, x0 + batt_l])
        translate([xs, batt_yc - 7.15, 0]) cube([stop_t, 14.3, batt_d * 0.6]);
}

module bms_slot() {
    // due pareti parallele lungo Y: la scheda sta in piedi tra le due, estremita' aperte
    for (xs = [bms_x, bms_x + bms_wall + bms_in_w])
        translate([xs, bms_y, 0]) cube([bms_wall, bms_in_l, bms_slot_h]);
}

module gps_standoffs() {
    for (x = [gps_x + gps_inset, gps_x + gps_l - gps_inset], y = [gps_y + gps_inset, gps_y + gps_w - gps_inset])
        translate([x, y, 0]) difference() {
            cylinder(d = gps_so_d, h = gps_so_h);
            translate([0, 0, 1]) cylinder(d = h_pilot_d, h = gps_so_h);
        }
}

// pareti del vano: piene fino a ch_top, abbassate vicino alle pareti esterne
// (li' passa il labbro del coperchio)
module sensor_chamber() {
    // parete sinistra del vano (lungo Y)
    difference() {
        translate([ch_x, 0, 0]) cube([ch_wall, sw_y + ch_wall, ch_top]);
        translate([ch_x - 1, -1, ch_top_edge]) cube([ch_wall + 2, 2.2 + 1, inner_h]);
    }
    // parete dei sensori (lungo X)
    difference() {
        translate([ch_x, sw_y, 0]) cube([inner_l - ch_x, ch_wall, ch_top]);
        translate([inner_l - 2.2, sw_y - 1, ch_top_edge]) cube([3.2, ch_wall + 2, inner_h]);
    }
    // distanziali del SCD41 sulla parete dei sensori (verso -Y)
    for (h = scd_holes) sensor_boss(scd_x + h[0], board_z + h[1]);
    // distanziale dell'MPL sulla parete sinistra del vano (verso +X)
    for (h = mpl_holes) mpl_boss(mpl_y0 + h[0], board_z + h[1]);
    // battuta d'appoggio sotto l'MPL (ha un solo foro: non deve ruotare)
    translate([ch_in_x0, mpl_y0, 0]) cube([so_h + 2.1, mpl_w, board_z]);
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
            difference() {
                translate([-wall, -wall, -floor_t])
                    rbox([inner_l + 2*wall, inner_w + 2*wall, inner_h + floor_t], corner_r);
                rbox([inner_l, inner_w, inner_h + 1], corner_r - wall);
            }
            // colonnine d'angolo, concentriche con gli spigoli esterni
            for (p = col_pos) translate([p[0], p[1], 0]) cylinder(d = col_d, h = inner_h);
            heltec_standoffs();
            battery_cradle();
            bms_slot();
            gps_standoffs();
            sensor_chamber();
            if (pv_mount == "top") {
                // la parete in alto continua oltre il coperchio: aletta + piazzola del pannello
                translate([top_x0 - 1, inner_w, inner_h - 0.01]) cube([top_len + 2, wall, top_zmax - inner_h]);
                translate([top_x0, inner_w + wall - 0.01, -floor_t]) cube([top_len, top_pad, top_zmax + floor_t]);
            }
            if (ears) { ear(-1); ear(1); }
        }
        // fori pilota nelle colonnine d'angolo
        for (p = col_pos) translate([p[0], p[1], inner_h - screw_pilot_depth])
            cylinder(d = screw_pilot_d, h = screw_pilot_depth + 1);
        // fori ciechi dei distanziali dei sensori
        sensor_boss_holes();
        // passacavi (da sigillare): SCD41 nella parete dei sensori, MPL nella parete sinistra del vano
        translate([scd_x + 5, sw_y - 1, board_z + scd_h/2])
            rotate([-90, 0, 0]) cylinder(d = wire_d, h = ch_wall + 2);
        translate([ch_x - 1, mpl_y0 + 3, 4])      // in basso: sotto il cavo del DS18B20, prima della fessura BMS
            rotate([0, 90, 0]) cylinder(d = wire_d, h = ch_wall + 2);
        // finestra USB (parete sinistra)
        translate([0, usb_y, usb_z]) hull()
            for (dy = [-(usb_w - usb_h)/2, (usb_w - usb_h)/2])
                translate([-wall - 1, dy, 0]) rotate([0, 90, 0]) cylinder(d = usb_h, h = wall + 2);
        // UNICA feritoia del vano sensori (parete che guarda in basso)
        translate([vent_x, -wall - 1, vent_z]) cube([vent_len, wall + 2, vent_h]);
        // foro sigillato per il cavo DS18B20 (parete in basso)
        translate([ds_x, 1, ds_z]) rotate([90, 0, 0]) cylinder(d = ds_d, h = wall + 2);
        // foro sigillato per il cavo dei pannelli solari (parete in alto, riparata dalla tettoia)
        // pannelli verticali: il cavo del pannello basso entra dalla parete in basso, dietro il prolungamento
        if (pv_mount == "flat")
            translate([flat_hole_bot[0], 1, flat_wall_hole_z]) rotate([90, 0, 0]) cylinder(d = pv_hole_d, h = wall + 2);
        if (pv_mount == "roof") translate([pv_hole_x, inner_w - 1, pv_hole_z]) rotate([-90, 0, 0]) cylinder(d = pv_hole_d, h = wall + 2);
        // antenna SMA: parete in alto, oppure parete destra se in alto c'e' il pannello
        if (pv_mount == "top")
            translate([inner_l - 1, sma_right[0], sma_right[1]]) rotate([0, 90, 0]) cylinder(d = sma_d, h = wall + 2);
        else
            translate([sma_x, inner_w - 1, sma_z]) rotate([-90, 0, 0]) cylinder(d = sma_d, h = wall + 2);
        if (pv_mount == "top") {
            // foro dei fili sotto il pannello e canaline nella piazzola (orizzontale + verticale al centro)
            translate([top_hole[0], inner_w - 1, top_hole[1]]) rotate([-90, 0, 0]) cylinder(d = 4.5, h = wall + top_pad + 2);
            translate([top_hole[0] - 2, inner_w + wall, top_hole[1] - 2]) cube([inner_l/2 - top_hole[0] + 4, top_pad + 1, 4]);
            translate([inner_l/2 - 2, inner_w + wall, 1]) cube([4, top_pad + 1, top_zmax - 4]);
        }
        if (labels) {
            floor_label(h_x + 31.5, h_y + h_wid/2, "HELTEC V4", 3);
            floor_label(batt_x + stop_t + batt_l/2, batt_yc, "14500", 4);
            floor_label(gps_x + gps_l/2, gps_y + gps_w/2, "GPS", 4);
            floor_label(bms_x - 1.4, bms_y + bms_in_l/2, "BMS", 2.2, 90);
            floor_label((ch_in_x0 + inner_l)/2, 4.5, "SENSORI", 3);
            floor_label(ds_x, 6, "DS18B20", 2.2, 90);
        }
    }
}

// ---------------------------------------------------------------------
//  Coperchio (origine: faccia inferiore della piastra)
// ---------------------------------------------------------------------
module lid() {
    lip_r = corner_r - wall - lid_clear;
    difference() {
        union() {
            difference() {
                translate([-wall, -wall, 0]) rbox([inner_l + 2*wall, inner_w + 2*wall, lid_t], corner_r);
                // top: tacca nel bordo alto del coperchio, dove la parete prosegue come aletta
                if (pv_mount == "top") translate([top_x0 - 1.3, inner_w - lid_clear, -1]) cube([top_len + 2.6, wall + 1, lid_t + 2]);
            }
            // pannelli verticali: il coperchio continua verso il basso, davanti alla feritoia
            if (pv_mount == "flat") {
                translate([vis_x0, vis_y0 - vis_s, 0]) cube([vis_px, vis_s - vis_y0 - wall + 1, lid_t]);
                for (xc = [vis_x0, vis_x0 + vis_px - 2])
                    translate([xc, 0, 0]) rotate([90, 0, 90]) linear_extrude(2)
                        polygon([[-wall - 0.5, 0.01], [-wall - 0.5, -flat_rib], [vis_y0 - vis_s, -2], [vis_y0 - vis_s, 0.01]]);
            }
            // tilt: cuneo pieno sopra il coperchio + piastra inclinata che scende davanti alla feritoia
            if (pv_mount == "tilt") translate([0, 0, -inner_h]) {
                intersection() {
                    translate([-wall, -wall, lid_z - 0.01]) rbox([inner_l + 2*wall, inner_w + 2*wall, 40], corner_r);
                    multmatrix(tilt_M) translate([-50, -50, -100]) cube([tilt_w + 100, tilt_s + 100, 100.5]);
                }
                multmatrix(tilt_M) rbox([tilt_w, tilt_s, tilt_t], corner_r);
            }
            translate([0, 0, -lid_lip_h]) difference() {
                translate([lid_clear, lid_clear, 0])
                    rbox([inner_l - 2*lid_clear, inner_w - 2*lid_clear, lid_lip_h + 0.01], lip_r);
                translate([lid_clear + lid_lip_t, lid_clear + lid_lip_t, -1])
                    rbox([inner_l - 2*(lid_clear + lid_lip_t), inner_w - 2*(lid_clear + lid_lip_t), lid_lip_h + 2], lip_r - lid_lip_t);
            }
        }
        // scarichi del labbro attorno alle colonnine d'angolo
        for (p = col_pos) translate([p[0], p[1], -lid_lip_h - 1]) cylinder(d = col_d + 1, h = lid_lip_h + 1);
        for (p = col_pos) {
            translate([p[0], p[1], -1]) cylinder(d = screw_clear_d, h = lid_t + 2);
            translate([p[0], p[1], lid_t - screw_head_h]) cylinder(d = screw_head_d, h = screw_head_h + 1);
        }
        if (pv_mount == "tilt") translate([0, 0, -inner_h]) {
            // pozzetti per raggiungere le viti del coperchio attraverso il cuneo
            for (p = col_pos) translate([p[0], p[1], lid_z - screw_head_h]) cylinder(d = 7, h = 40);
            // fori dei fili (dritti nella scatola, sopra Heltec e GPS) e canaline dietro ai pannelli
            for (x = tilt_px) {
                ph = [x, vis_y0 - tilt_hole_u*cos(tilt_ang)];
                translate([ph[0], ph[1], inner_h - 1]) cylinder(d = 5, h = 40);
                multmatrix(tilt_M) translate([x + wall - 2, 5, tilt_t - 1.5]) cube([4, tilt_groove, 2]);
            }
        }
        if (pv_mount == "flat")
            for (h = [flat_hole_top, flat_hole_bot]) translate([h[0], h[1], -flat_rib - 1]) cylinder(d = 4.5, h = flat_rib + lid_t + 2);
        if (pv_mount == "lid") {
            // fori per le viti del portapannelli e per il cavo dei pannelli (da sigillare)
            for (x = vis_feet_x, y = vis_feet_y) translate([x, y, -1]) cylinder(d = 2.4, h = lid_t + 2);
            translate([vis_cable[0], vis_cable[1], -1]) cylinder(d = 4.5, h = lid_t + 2);
        } else if (pv_mount != "tilt")
        // fori per premere i due pulsanti della Heltec con una graffetta
        for (b = h_btns) translate(concat(hpos(b), -1)) cylinder(d = 3, h = lid_t + 2);
        if (labels && pv_mount != "lid" && pv_mount != "flat" && pv_mount != "tilt") for (b = h_btns)
            translate([hpos(b)[0] + 7, hpos(b)[1], lid_t - label_depth]) ltext("BTN", 3);
    }
}

// ---------------------------------------------------------------------
//  Tettoia per due pannelli solari (corpo separato, si avvita al muro
//  sopra la scatola). Coordinate della scatola: Z = distanza dal muro.
// ---------------------------------------------------------------------
module pv_tray() {
    // coordinate locali: x lungo X, y lungo la pendenza (0 alla piastra), z dal sotto della vaschetta
    difference() {
        translate([0, -8, 0]) cube([roof_len, roof_dep + 8, pv_base + pv_t]);
        for (i = [0, 1]) translate([pv_rim + i*(pw + 2*pv_rim + pv_slot), pv_rim, pv_base]) {
            cube([pw, pl, pv_t + 1]);                                   // tasca del pannello
            for (f = [0.25, 0.75]) translate([pw*f - 3, pl - 1, 0]) cube([6, pv_rim + 2, pv_t + 1]);  // scolo
            translate([pw/2, 15, -pv_base - 1]) cylinder(d = 5, h = pv_base + 2);  // fili del pannello
        }
        translate([2*pv_rim + pw, -10, -1]) cube([pv_slot, roof_dep + 12, pv_base + pv_t + 2]);  // antenna
    }
}
module roof() {
    difference() {
        union() {
            translate([roof_x0, roof_y0, -floor_t]) cube([roof_len, roof_top - roof_y0, roof_plate_t]);
            intersection() {
                translate([roof_x0, roof_ya, roof_zf]) rotate([90 + pv_tilt, 0, 0]) mirror([0, 0, 1]) pv_tray();
                translate([-500, -500, -floor_t]) cube([1000, 1000, 1000]);   // niente dietro al muro
            }
            // nervature triangolari sotto la vaschetta: alle estremita' e ai lati della fessura
            for (xr = [roof_x0, roof_x0 + roof_len - rib_t,
                       roof_x0 + 2*pv_rim + pw - rib_t, roof_x0 + 2*pv_rim + pw + pv_slot])
                translate([xr, 0, 0]) rotate([90, 0, 90]) linear_extrude(rib_t)
                    polygon([[roof_y0, roof_zf - 0.01], [roof_ya + 0.5, roof_zf - 0.01],
                             [roof_ya - rib_len*tan(pv_tilt) + 0.5, roof_zf + rib_len]]);
        }
        // due asole a buco di serratura sopra la vaschetta: la tettoia si appende a due viti
        // lasciate sporgere 4,5 mm e si sfila verso l'alto per aprire il coperchio
        for (xs = [roof_x0 + 20, roof_x0 + roof_len - 20]) translate([xs, 0, -floor_t - 1]) {
            translate([0, roof_ya + 12, 0]) cylinder(d = 8.5, h = roof_plate_t + 2);
            hull() for (yy = [roof_ya + 12, roof_ya + 18])
                translate([0, yy, 0]) cylinder(d = 4.5, h = roof_plate_t + 2);
        }
    }
}

// ---------------------------------------------------------------------
//  ALTERNATIVA: portapannelli avvitato sul coperchio. Scende oltre la
//  parete in basso (ripara la feritoia) ed e' inclinato verso l'alto.
//  Coordinate della scatola; coordinate locali della piastra:
//  x lungo X, u lungo la pendenza verso il basso, n uscente.
// ---------------------------------------------------------------------
module visor() {
    c = cos(vis_tilt); sn = sin(vis_tilt);
    p1 = [vis_y0, lid_z];                                   // (Y,Z) bordo alto, sul coperchio
    p2 = [-wall, lid_z];                                    // spigolo della parete in basso
    p3 = [vis_y0 - vis_s*c, lid_z + vis_s*sn];              // fondo della piastra (lato interno)
    ov = [0.5*sn, 0.5*c];
    difference() {
        union() {
            multmatrix(vis_M) cube([vis_px, vis_s, vis_t]);
            // due fianchi triangolari: poggiano sul coperchio e reggono la piastra
            for (xc = [vis_x0, vis_x0 + vis_px - 2])
                translate([xc, 0, 0]) rotate([90, 0, 90]) linear_extrude(2)
                    polygon([p1, p2, p3, p3 + ov, p1 + ov]);
            // piedini sotto la piastra
            for (x = vis_feet_x, y = vis_feet_y)
                translate([x, y, lid_z]) cylinder(d = 7, h = (vis_y0 - y)*sn/c + 1.5);
        }
        for (x = vis_feet_x, y = vis_feet_y)
            translate([x, y, lid_z - 0.01]) cylinder(d = h_pilot_d, h = 4);   // viti M2x6 da dentro il coperchio
        // fori per i fili dei due pannelli (al centro di ciascuno)
        for (u = [1 + pv_w/2, 3 + 1.5*pv_w])
            multmatrix(vis_M) translate([vis_px/2, u, -1]) cylinder(d = 6, h = vis_t + 2);
    }
}

// ---------------------------------------------------------------------
//  Sagome dei componenti (solo vista assembly)
// ---------------------------------------------------------------------
module dummies() {
    color("RoyalBlue", 0.6) translate([h_x, h_y, h_so_h]) cube([h_len, h_wid, h_pcb]);
    color("Silver", 0.6) translate([batt_x + stop_t, batt_yc, batt_d/2 + 1]) rotate([0, 90, 0]) cylinder(d = batt_d, h = batt_l);
    color("Black", 0.7) translate([bms_x + bms_wall + bms_clr, bms_y + 0.6, 0.5]) cube([bms_t, bms_l, bms_h]);
    color("DarkGreen", 0.6) translate([gps_x, gps_y, gps_so_h]) cube([gps_l, gps_w, 1.6]);
    color("Purple", 0.6) translate([scd_x, sw_y - so_h - 1.6, board_z]) cube([scd_w, 1.6, scd_h]);
    color("Red", 0.6) translate([ch_in_x0 + so_h, mpl_y0, board_z]) cube([1.6, mpl_w, mpl_h]);
}

if (part == "base") {
    base();
} else if (part == "lid" && pv_mount == "tilt") {
    // faccia dei pannelli sul piatto: il cuneo pieno sostiene il coperchio, niente supporti
    translate([0, 0, tilt_t]) mirror([0, 0, 1]) multmatrix(tilt_Minv)
        translate([wall, -vis_y0, -lid_z + inner_h]) lid();
} else if (part == "lid") {
    translate([0, 0, lid_t]) rotate([180, 0, 0]) lid();
} else if (part == "visor") {
    // faccia esterna della piastra sul piatto: fianchi e piedini salgono, niente supporti
    translate([0, 0, vis_t]) mirror([0, 0, 1]) multmatrix(vis_Minv)
        translate([-vis_x0, -vis_y0, -lid_z]) visor();
} else if (part == "roof") {
    translate([0, 0, floor_t]) roof();     // piastra sul piatto: la vaschetta sale a 45 gradi, senza supporti
} else if (part == "print") {
    base();
    if (pv_mount == "tilt")
        translate([0, inner_w + 2*wall + 14 + tilt_s, tilt_t]) mirror([0, 0, 1]) multmatrix(tilt_Minv)
            translate([wall, -vis_y0, -lid_z + inner_h]) lid();
    else
        translate([0, 2*inner_w + 4*wall + 14, lid_t]) rotate([180, 0, 0]) lid();
} else {
    base();
    %dummies();
    translate([0, 0, inner_h + explode]) lid();
    if (roof && pv_mount == "roof") color("SlateGray", 0.8) roof();
    if (roof && pv_mount == "lid") color("SlateGray", 0.8) translate([0, 0, explode]) visor();
}
