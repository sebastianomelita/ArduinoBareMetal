// =====================================================================
//  Contenitore per nodo LoRaWAN low-power
//  Heltec WiFi LoRa 32 V4 + SCD41 + MPL3115A2 + GPS L76K + batteria 14500
//  + pressacavo per sonda DS18B20 + passante SMA per l'antenna
//
//  Due vani separati da una paratia:
//   - vano A (sinistra): Heltec, batteria, GPS  -> chiuso, si scalda
//   - vano B (destra):   SCD41 + MPL3115A2      -> ventilato, aria esterna
//
//  Tutte le misure sono in mm. Le misure delle schedine sono NOMINALI:
//  controllale con il calibro e correggi i parametri qui sotto.
//
//  Origine: angolo interno della cavita', sul piano del fondo.
//  X = lunghezza, Y = larghezza, Z = altezza.
// =====================================================================

/* [Cosa visualizzare] */
// assembly = base + coperchio sollevato + sagome delle schede
// print    = base e coperchio affiancati, pronti per lo slicer (esporta STL)
part = "assembly"; // [assembly, base, lid, print]
explode = 25;      // distanza del coperchio nella vista assembly

/* [Scatola] */
inner_l = 140;     // lunghezza interna
inner_w = 72;      // larghezza interna
inner_h = 30;      // altezza interna (fondo -> bordo superiore)
wall    = 2.4;     // spessore pareti
floor_t = 2.4;     // spessore fondo
lid_t   = 2.4;     // spessore coperchio
corner_r = 4;      // raggio degli spigoli esterni
$fn = 48;

/* [Coperchio e viti] */
lid_lip_h  = 4;    // altezza del labbro che entra nella base
lid_lip_t  = 1.6;  // spessore del labbro
lid_clear  = 0.3;  // gioco tra labbro e pareti
boss_d     = 8;    // diametro colonnine angolari
screw_pilot_d = 2.6;  // foro pilota per vite M3 autofilettante (4.0 per inserti a caldo)
screw_pilot_depth = 14;
screw_clear_d = 3.4;  // foro passante nel coperchio
screw_head_d  = 6.2;  // svasatura per la testa
screw_head_h  = 1.6;

/* [Paratia tra i due vani] */
partition_x = 96;  // posizione X della paratia
partition_t = 2;
cable_notch_w = 10;   // passaggio cavi alla base della paratia
cable_notch_h = 6;

/* [Heltec WiFi LoRa 32 V4] */
heltec_l = 51.7;   // lunghezza PCB (verifica: V3 = 50.2)
heltec_w = 25.5;   // larghezza PCB
heltec_t = 1.6;    // spessore PCB
heltec_x = 1;      // il lato USB guarda la parete sinistra
heltec_y = 36;
heltec_z = 10;     // altezza del PCB dal fondo (spazio per i pin degli header)
cradle_len = 5;    // lunghezza degli appoggi sotto le estremita'
cradle_band = 5;   // margine laterale libero sotto il PCB (righe dei pin)
fence_t  = 1.6;
fence_clr = 0.3;
usb_w = 13;        // finestra USB-C (larga per lo spinotto del cavo)
usb_h = 7.5;
usb_z_off = 1.7;   // centro del connettore sopra la faccia superiore del PCB
// fori "spillo" nel coperchio per RST e PRG (posizione rispetto al PCB: VERIFICA)
btn_rst = [4, 3.5];
btn_prg = [4, 22];
poke_d  = 3;
// finestra opzionale sopra il display OLED
oled_window = false;
oled_win = [14, 4, 26, 15];  // [x, y, larghezza, altezza] rispetto al PCB

/* [Batteria 14500 (o LiPo piatta: adatta le misure)] */
batt_d = 14.5;
batt_l = 53;
batt_x = 12;
batt_y = 5;
saddle_w = 8;      // larghezza delle due selle
tie_w = 4;         // fessura per fascetta
tie_h = 1.8;

/* [Modulo GPS L76K] */
gps_l = 30;        // VERIFICA sul tuo modulo
gps_w = 25;
gps_x = 64;
gps_y = 40;
gps_hole_inset = 2.5;

/* [SCD41 (breakout tipo Adafruit 4867)] */
scd_l = 25.4;
scd_w = 17.8;
scd_x = 106;
scd_y = 44;
scd_hole_inset = 2.54;

/* [MPL3115A2 (breakout)] */
mpl_l = 19.05;
mpl_w = 17.8;
mpl_x = 106;
mpl_y = 12;
mpl_hole_inset = 2.54;

/* [BMS 1S 5A (tipo HX-3450, a striscia)] */
// La scheda si infila da un'estremita' tra due guide con labbro e resta
// sotto i dentini; le estremita' sono aperte per i fili.
// Se la tua BMS e' rettangolare (es. 25 x 17) cambia solo bms_l e bms_w.
bms = true;
bms_l = 35.2;      // lunghezza (lungo Y)
bms_w = 4.2;       // larghezza
bms_t = 2;         // spessore con i componenti
bms_cx = 80;       // centro X della guida
bms_y  = 8;        // inizio della guida (lascia spazio al dado del pressacavo)
bms_clr = 0.3;     // gioco per lato
bms_rail_t = 1.2;  // spessore delle guide
bms_lip = 0.5;     // sporgenza dei dentini di ritenuta

/* [Colonnine per le schedine] */
standoff_h  = 5;
standoff_od = 5;
standoff_id = 1.8;  // vite M2 autofilettante

/* [Ventilazione vano B] */
vent_slot_h = 2;
vent_pitch  = 4;
vent_z0 = 6;
vent_z1 = 24;
vent_margin = 12;  // distanza dalle colonnine angolari

/* [Passanti] */
// La posizione dipende da mount: il pressacavo va sempre sul lato in basso,
// l'antenna sempre sul lato in alto.
gland_d = 12.5;    // PG7 per la sonda DS18B20
gland_z = 12;
gland_x_wall = 84; // a muro: parete frontale, vano A
gland_y_pole = 36; // su palo: parete destra, vano B
sma_d = 6.6;       // connettore SMA da pannello
sma_z = 21;
sma_x_wall = 75;   // a muro: parete posteriore
sma_y_pole = 20;   // su palo: parete sinistra

/* [Installazione] */
// wall = a muro, lato lungo orizzontale: parete frontale (Y = 0) in basso,
//        posteriore in alto. Alette sui lati corti per le viti.
// pole = su palo, lato lungo verticale e parallelo al palo: parete destra
//        (vano sensori) in basso, sinistra (USB) in alto. Due passanti sotto
//        il fondo per fascette metalliche che girano attorno al palo.
mount = "wall"; // [wall, pole]
strap_w = 14;      // larghezza della fascetta + gioco
strap_h = 2.5;     // spessore del passante
strap_roof = 2;    // spessore sotto il passante
strap_len = 30;    // lunghezza del ponticello lungo Y
strap_x = [25, 115];

/* [Alette di fissaggio] */
ears = true;       // usate solo con mount = "wall"
ear_len = 12;
ear_w   = 20;
ear_t   = 4;
ear_slot_d = 4.5;

/* [Etichette incise] */
labels = true;         // etichette di montaggio sul fondo interno
outer_labels = false;  // etichette sulle pareti esterne (uso in campo)
label_depth = 0.6;     // profondita' dell'incisione
label_font  = "Liberation Sans:style=Bold";

// ---------------------------------------------------------------------
//  Funzioni e moduli di base
// ---------------------------------------------------------------------
boss_pos = [
    [boss_d/2,           boss_d/2],
    [inner_l - boss_d/2, boss_d/2],
    [boss_d/2,           inner_w - boss_d/2],
    [inner_l - boss_d/2, inner_w - boss_d/2]
];

function corner_holes(l, w, i) = [[i, i], [l - i, i], [i, w - i], [l - i, w - i]];

// parallelepipedo con spigoli verticali arrotondati, origine nell'angolo
module rbox(size, r) {
    rr = max(min(r, size[0]/2 - 0.01, size[1]/2 - 0.01), 0.01);
    hull()
        for (x = [rr, size[0] - rr], y = [rr, size[1] - rr])
            translate([x, y, 0]) cylinder(r = rr, h = size[2]);
}

module standoffs(x, y, l, w, inset) {
    translate([x, y, 0])
        for (p = corner_holes(l, w, inset))
            translate([p[0], p[1], 0])
                difference() {
                    cylinder(d = standoff_od, h = standoff_h);
                    translate([0, 0, 1]) cylinder(d = standoff_id, h = standoff_h);
                }
}

// ---------------------------------------------------------------------
//  Elementi interni della base
// ---------------------------------------------------------------------
module heltec_cradle() {
    translate([heltec_x, heltec_y, 0]) {
        // appoggi sotto le due estremita', solo nella fascia centrale
        for (xe = [0, heltec_l - cradle_len])
            translate([xe, cradle_band, 0])
                cube([cradle_len, heltec_w - 2*cradle_band, heltec_z]);
        // battute laterali, fuori dal PCB
        for (xe = [0, heltec_l - cradle_len])
            for (ys = [-fence_t - fence_clr, heltec_w + fence_clr])
                translate([xe, ys, 0])
                    cube([cradle_len, fence_t, heltec_z + heltec_t + 1]);
        // battuta di fondo, lato antenna
        translate([heltec_l + fence_clr, cradle_band, 0])
            cube([fence_t, heltec_w - 2*cradle_band, heltec_z + heltec_t + 1]);
    }
}

module battery_cradle() {
    zc = batt_d/2 + 1;           // asse della batteria
    span = batt_d + 4;
    translate([batt_x, batt_y, 0]) {
        for (xs = [6, batt_l - 6 - saddle_w + 4])
            translate([xs, 0, 0])
                difference() {
                    cube([saddle_w, span, zc + 1]);
                    translate([-1, span/2, zc]) rotate([0, 90, 0])
                        cylinder(d = batt_d + 0.6, h = saddle_w + 2);
                    // fessura per la fascetta
                    translate([(saddle_w - tie_w)/2, -1, 0])
                        cube([tie_w, span + 2, tie_h]);
                }
        // fermi alle due estremita'
        for (xs = [0, batt_l + 2])
            translate([xs, 2, 0]) cube([2, span - 4, batt_d * 0.6]);
    }
}

module bms_holder() {
    gap = bms_w + 2*bms_clr;            // larghezza tra le guide
    h   = bms_t + 0.4;                  // altezza fino al dentino
    for (s = [-1, 1])
        translate([bms_cx + s * (gap/2 + bms_rail_t/2) - bms_rail_t/2, bms_y, 0]) {
            cube([bms_rail_t, bms_l + 2*bms_clr, h + 0.8]);
            // dentino verso l'interno, sopra la scheda
            translate([s < 0 ? bms_rail_t : -bms_lip, 0, h])
                cube([bms_lip, bms_l + 2*bms_clr, 0.8]);
        }
}

module partition() {
    difference() {
        translate([partition_x, 0, 0])
            cube([partition_t, inner_w, inner_h - lid_lip_h - 0.3]);
        translate([partition_x - 1, inner_w/2 - cable_notch_w/2, -1])
            cube([partition_t + 2, cable_notch_w, cable_notch_h + 1]);
    }
}

// passanti per fascette metalliche, sotto il fondo; la fascetta corre lungo X
module strap_bridges() {
    for (x = strap_x)
        translate([x - strap_w/2 - 2, inner_w/2 - strap_len/2, -floor_t - strap_h - strap_roof])
            difference() {
                cube([strap_w + 4, strap_len, strap_h + strap_roof + 0.01]);
                translate([2, -1, strap_roof]) cube([strap_w, strap_len + 2, strap_h]);
            }
}

// feritoie
vb0 = partition_x + partition_t + 4;   // tratto di parete del vano B
vb1 = inner_l - boss_d - 4;

module vents_right(y0, y1) {
    for (z = [vent_z0 : vent_pitch : vent_z1])
        translate([inner_l - 1, y0, z]) cube([wall + 2, y1 - y0, vent_slot_h]);
}
module vents_y(yface) {     // yface = -wall - 1 (frontale) o inner_w - 1 (posteriore)
    for (z = [vent_z0 : vent_pitch : vent_z1])
        translate([vb0, yface, z]) cube([vb1 - vb0, wall + 2, vent_slot_h]);
}

module gland_cut() {
    if (mount == "wall")
        translate([gland_x_wall, 1, gland_z]) rotate([90, 0, 0]) cylinder(d = gland_d, h = wall + 2);
    else
        translate([inner_l - 1, gland_y_pole, gland_z]) rotate([0, 90, 0]) cylinder(d = gland_d, h = wall + 2);
}
module sma_cut() {
    if (mount == "wall")
        translate([sma_x_wall, inner_w - 1, sma_z]) rotate([-90, 0, 0]) cylinder(d = sma_d, h = wall + 2);
    else
        translate([1, sma_y_pole, sma_z]) rotate([0, -90, 0]) cylinder(d = sma_d, h = wall + 2);
}

module ear(side) {
    // side = -1 sinistra, +1 destra
    x_wall = side < 0 ? -wall : inner_l + wall;   // faccia esterna della parete
    x_tip  = x_wall + side * ear_len;
    translate([0, inner_w/2, -floor_t])
        difference() {
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
            // guscio
            difference() {
                translate([-wall, -wall, -floor_t])
                    rbox([inner_l + 2*wall, inner_w + 2*wall, inner_h + floor_t], corner_r);
                rbox([inner_l, inner_w, inner_h + 1], corner_r - wall);
            }
            // colonnine angolari
            for (p = boss_pos) translate([p[0], p[1], 0]) cylinder(d = boss_d, h = inner_h);
            heltec_cradle();
            battery_cradle();
            if (bms) bms_holder();
            partition();
            standoffs(gps_x, gps_y, gps_l, gps_w, gps_hole_inset);
            standoffs(scd_x, scd_y, scd_l, scd_w, scd_hole_inset);
            standoffs(mpl_x, mpl_y, mpl_l, mpl_w, mpl_hole_inset);
            if (mount == "wall" && ears) { ear(-1); ear(1); }
            if (mount == "pole") strap_bridges();
        }

        // fori pilota nelle colonnine
        for (p = boss_pos)
            translate([p[0], p[1], inner_h - screw_pilot_depth])
                cylinder(d = screw_pilot_d, h = screw_pilot_depth + 1);

        // finestra USB-C (parete sinistra)
        translate([0, heltec_y + heltec_w/2, heltec_z + heltec_t + usb_z_off])
            hull()
                for (dy = [-(usb_w - usb_h)/2, (usb_w - usb_h)/2])
                    translate([-wall - 1, dy, 0]) rotate([0, 90, 0])
                        cylinder(d = usb_h, h = wall + 2);

        // feritoie del vano B: mai sul lato che guarda in alto
        if (mount == "wall") {
            vents_right(vent_margin, inner_w - vent_margin);  // destra = lato
            vents_y(-wall - 1);                               // frontale = in basso
        } else {
            vents_right(vent_margin, gland_y_pole - gland_d/2 - 3);  // destra = in basso,
            vents_right(gland_y_pole + gland_d/2 + 3, inner_w - vent_margin); // attorno al pressacavo
            vents_y(-wall - 1);                               // frontale = lato
            vents_y(inner_w - 1);                             // posteriore = lato
        }

        gland_cut();   // pressacavo PG7 per il DS18B20, sul lato in basso
        sma_cut();     // antenna SMA, sul lato in alto

        if (labels) base_labels();
    }
}

// ---------------------------------------------------------------------
//  Etichette
// ---------------------------------------------------------------------
module ltext(s, size) {
    linear_extrude(label_depth + 1)
        text(s, size = size, font = label_font, halign = "center", valign = "center");
}

// sul fondo interno: incise verso il basso a partire da Z = 0
module floor_label(x, y, s, size) {
    translate([x, y, -label_depth]) ltext(s, size);
}

module base_labels() {
    // fondo, accanto o sotto ogni alloggiamento
    floor_label(heltec_x + heltec_l/2, heltec_y + heltec_w/2, "HELTEC V4", 4);
    floor_label(batt_x + (batt_l + 4)/2, batt_y + (batt_d + 4)/2, "14500", 4);
    floor_label(gps_x + gps_l/2, gps_y + gps_w/2, "GPS L76K", 3.5);
    floor_label(scd_x + scd_l/2, scd_y + scd_w/2, "SCD41", 4);
    floor_label(mpl_x + mpl_l/2, mpl_y + mpl_w/2, "MPL3115A2", 2.5);
    floor_label(partition_x/2, inner_w - 4, "A - ELETTRONICA", 3);
    floor_label((partition_x + partition_t + inner_l)/2, inner_w - 6, "B - SENSORI", 3);

    // indicazioni di montaggio sul fondo, vicino ai passanti
    floor_label(heltec_x + 13, heltec_y - 6, "< USB", 3);          // verso della Heltec
    if (mount == "wall") {
        floor_label(sma_x_wall, inner_w - 3.5, "ANT", 3);           // pigtail SMA
        floor_label(gland_x_wall, 4, "DS18B20", 3);                 // pressacavo
    } else {
        translate([6, sma_y_pole, -label_depth]) rotate([0, 0, 90]) ltext("ANT", 3);
        translate([inner_l - 11, gland_y_pole, -label_depth]) rotate([0, 0, 90]) ltext("DS18B20", 2.5);
    }
    floor_label(partition_x - 8, inner_w/2, "I2C", 3);              // passaggio cavi
    if (bms) {
        // tra le guide, letta lungo Y
        translate([bms_cx, bms_y + bms_l/2, -label_depth]) rotate([0, 0, 90]) ltext("BMS", 3);
    }

    if (outer_labels) {
        // parete sinistra, esterno: sopra la finestra USB
        translate([-wall + label_depth, heltec_y + heltec_w/2, inner_h - 7])
            rotate([0, 0, -90]) rotate([90, 0, 0]) ltext("USB", 4);
        if (mount == "wall") {
            translate([gland_x_wall, -wall + label_depth, gland_z + gland_d/2 + 5])
                rotate([90, 0, 0]) ltext("DS18B20", 3.5);
            translate([sma_x_wall, inner_w + wall - label_depth, sma_z - sma_d/2 - 4])
                rotate([0, 0, 180]) rotate([90, 0, 0]) ltext("ANT 868", 3.5);
        } else {
            translate([inner_l + wall - label_depth, gland_y_pole, gland_z + gland_d/2 + 5])
                rotate([0, 0, 90]) rotate([90, 0, 0]) ltext("DS18B20", 3.5);
            translate([-wall + label_depth, sma_y_pole, sma_z - sma_d/2 - 4])
                rotate([0, 0, -90]) rotate([90, 0, 0]) ltext("ANT", 3.5);
        }
    }
}

// sulla faccia superiore del coperchio: RST e PRG servono a scatola chiusa
module lid_labels() {
    for (b = [[btn_rst, "RST"], [btn_prg, "PRG"]])
        translate([heltec_x + b[0][0] + poke_d/2 + 5, heltec_y + b[0][1], lid_t - label_depth])
            ltext(b[1], 3.5);
}

// ---------------------------------------------------------------------
//  Coperchio (origine: faccia inferiore della piastra; il labbro scende in Z negativa)
// ---------------------------------------------------------------------
module lid() {
    lip_r = corner_r - wall - lid_clear;
    difference() {
        union() {
            translate([-wall, -wall, 0])
                rbox([inner_l + 2*wall, inner_w + 2*wall, lid_t], corner_r);
            // labbro
            translate([0, 0, -lid_lip_h])
                difference() {
                    translate([lid_clear, lid_clear, 0])
                        rbox([inner_l - 2*lid_clear, inner_w - 2*lid_clear, lid_lip_h + 0.01], lip_r);
                    translate([lid_clear + lid_lip_t, lid_clear + lid_lip_t, -1])
                        rbox([inner_l - 2*(lid_clear + lid_lip_t),
                              inner_w - 2*(lid_clear + lid_lip_t), lid_lip_h + 2], lip_r - lid_lip_t);
                }
            // nervatura che chiude la paratia fino al coperchio
            translate([partition_x, lid_clear, -lid_lip_h])
                cube([partition_t, inner_w - 2*lid_clear, lid_lip_h + 0.01]);
        }

        // scarico del labbro attorno alle colonnine
        for (p = boss_pos)
            translate([p[0], p[1], -lid_lip_h - 1])
                cylinder(d = boss_d + 1, h = lid_lip_h + 1);

        // fori viti con svasatura
        for (p = boss_pos) {
            translate([p[0], p[1], -1]) cylinder(d = screw_clear_d, h = lid_t + 2);
            translate([p[0], p[1], lid_t - screw_head_h]) cylinder(d = screw_head_d, h = screw_head_h + 1);
        }

        // fori per premere RST e PRG con una graffetta
        for (b = [btn_rst, btn_prg])
            translate([heltec_x + b[0], heltec_y + b[1], -1])
                cylinder(d = poke_d, h = lid_t + 2);

        // finestra OLED (opzionale)
        if (oled_window)
            translate([heltec_x + oled_win[0], heltec_y + oled_win[1], -1])
                cube([oled_win[2], oled_win[3], lid_t + 2]);

        if (labels) lid_labels();
    }
}

// ---------------------------------------------------------------------
//  Sagome delle schede (solo per la vista assembly)
// ---------------------------------------------------------------------
module dummies() {
    color("RoyalBlue", 0.6)
        translate([heltec_x, heltec_y, heltec_z]) cube([heltec_l, heltec_w, heltec_t]);
    color("Silver", 0.6)
        translate([batt_x + 2, batt_y + (batt_d + 4)/2, batt_d/2 + 1]) rotate([0, 90, 0])
            cylinder(d = batt_d, h = batt_l);
    color("DarkGreen", 0.6)
        translate([gps_x, gps_y, standoff_h]) cube([gps_l, gps_w, 1.6]);
    color("Purple", 0.6)
        translate([scd_x, scd_y, standoff_h]) cube([scd_l, scd_w, 1.6]);
    color("Orange", 0.6)
        translate([mpl_x, mpl_y, standoff_h]) cube([mpl_l, mpl_w, 1.6]);
    if (bms)
        color("Black", 0.6)
            translate([bms_cx - bms_w/2, bms_y + bms_clr, 0]) cube([bms_w, bms_l, bms_t]);
}

// ---------------------------------------------------------------------
//  Selezione
// ---------------------------------------------------------------------
if (part == "base") {
    base();
} else if (part == "lid") {
    translate([0, 0, lid_t]) rotate([180, 0, 0]) lid();
} else if (part == "print") {
    base();
    translate([0, 2*inner_w + 4*wall + 10, lid_t]) rotate([180, 0, 0]) lid();
} else {
    base();
    %dummies();
    translate([0, 0, inner_h + explode]) lid();
}
