// =====================================================================
//  Sezione SDS011 da avvitare a destra di una scatola (v4 o v5).
//  Non si apre da sola: la includono nodo_lorawan_case_v4_sds.scad e
//  nodo_lorawan_case_v5_sds.scad, DOPO il file della scatola, di cui usa
//  le misure (inner_l, inner_w, wall, viti, sede del pannello...).
//
//  SDS011 coricato, ventola in basso e beccuccio a destra:
//    - ingresso: tubo di silicone 6 x 8 dal beccuccio, curva a L verso il
//      basso, esce da un foro nella parete in BASSO (in fondo a destra);
//    - uscita: l'aria della ventola esce dal lato sinistro del sensore,
//      scende nel canale di 12 mm e va fuori dalla griglia nella parete in
//      BASSO (in fondo a sinistra): i due fori distano circa 80 mm;
//    - vano elettronica a sinistra: millefori 20 x 60 col boost 5 V e il
//      condensatore d'uscita vicino al J34; dalla scatola arrivano Vext 3,3 V,
//      GND, TX e RX (piu' i 3 fili del DS18B20 e i 2 del secondo pannello);
//    - il coperchio porta il SECONDO pannello 80 x 45, con la stessa sede a
//      incastro dell'altro coperchio.
//  Le quote dell'SDS011 vengono dalle foto e dal datasheet: misurale col
//  calibro (sds_mod, sds_inlet_top, sds_inlet_z, sds_inlet_out).
// =====================================================================

/* [SDS011] */
sds_mod = [71, 70, 24];   // ingombro: largo (X), alto (Y), spesso (Z) = parte con la ventola, misurata 24
sds_inlet_top = 16.7;     // asse del beccuccio dal bordo ALTO del sensore (lato destro)
sds_inlet_z = 12;         // asse del beccuccio sopra la faccia d'appoggio del sensore
sds_inlet_out = 3;        // quanto il beccuccio sporge a destra
sds_pad_h = 3;            // piedini sotto gli angoli (sotto c'e' il circuito)
sds_t_top = 15;           // spessore della parte ALTA (senza ventola), misurato 15
sds_fan_h = 35;           // altezza della parte con la ventola, dal bordo basso del sensore
tube_od = 8;              // tubo di silicone 6 x 8 (massimo 1 m)

/* [Sezione] */
sds_fused = false;        // true = la sezione e la scatola sono UN pezzo solo (parete interna doppia, niente viti)
s_gap = 0.3;              // spazio tra le due scatole: il coperchio della scatola si alza senza strisciare
s_bay = 24;               // vano ELETTRONICA a sinistra: boost 5 V (es. Pololu U1V11F5, da Vext) + condensatore 100-220 uF;
                          // c'e' posto anche per AO3401 + 2N7002 se il boost lo prendi dalla batteria
s_chim = 12;              // canale di scarico tra il vano elettronica e il sensore
s_tube = 20;              // canale del tubo a destra del sensore (curva a L, raggio circa 7)
bay_board = [20, 60];     // millefori (taglia una 2 x 8 cm a 60): su 4 colonnine alte bay_so_h
bay_so_h = 15;            // sotto passano i fili dal passacavo e i dadi delle viti di unione
s_top_gap = 2;            // sopra il sensore
s_bot_gap = 5;            // sotto il sensore: l'aria di scarico gira verso la griglia
s_in_h = 28;              // profondita' interna (piatta): piedini 3 + sensore 24 + 1

// ---------------------------------------------------------------------
//  Quote derivate (stessi assi della scatola)
// ---------------------------------------------------------------------
s_x0  = inner_l + wall + s_gap;                 // faccia esterna sinistra della sezione
s_ix0 = s_x0 + wall;                            // interno
s_il  = s_bay + s_chim + sds_mod[0] + s_tube;   // 127
s_ix1 = s_ix0 + s_il;
s_iy1 = inner_w;                                // in alto a filo con la scatola
s_iw  = s_top_gap + sds_mod[1] + s_bot_gap;     // 77
s_iy0 = s_iy1 - s_iw;                           // sotto, scende piu' della scatola
m_x0 = s_ix0 + s_bay + s_chim;                  // sensore
m_y1 = s_iy1 - s_top_gap;
m_y0 = m_y1 - sds_mod[1];
m_z1 = sds_pad_h + sds_mod[2];
in_y = m_y1 - sds_inlet_top;                    // asse del beccuccio
in_z = sds_pad_h + sds_inlet_z;
tube_x = s_ix1 - 10;                            // tratto verticale del tubo
ds_sx = tube_x - 10;                            // DS18B20: accanto all'ingresso dell'aria, all'ombra sotto la scatola
bay_x0 = s_ix0 + 3.5;                           // millefori
bay_y0 = (s_iy0 + s_iy1)/2 - bay_board[1]/2;
bay_holes = [for (x = [2.5, bay_board[0] - 2.5], y = [2.5, bay_board[1] - 2.5]) [bay_x0 + x, bay_y0 + y]];
s_col = [[s_ix0 + col_c, s_iy0 + col_c], [s_ix1 - col_c, s_iy0 + col_c],
         [s_ix0 + col_c, s_iy1 - col_c], [s_ix1 - col_c, s_iy1 - col_c]];
grill_x = [s_ix0 + s_bay + 0.5, m_x0 + 12];      // griglia di scarico (X): sotto il canale e un po' sotto il sensore
// pannello sul coperchio della sezione (v = Y, coperchio piatto)
s_pv_xc  = s_ix0 + s_il/2;
s_snap_end = s_iy1 + wall - 1.5;
s_pv_vt  = s_snap_end - (is_undef(snap_l) ? snap_len : snap_l) - 0.3;
s_pv_v0  = s_pv_vt - pv_dv - pv_clr;
s_pv_hole = [m_x0 - s_chim/2, s_pv_v0 + pv_clr + pv_dv/2];   // foro fili sopra il canale di scarico (non sopra il sensore)

assert(s_col[1][0] - col_d/2 > tube_x + tube_od/2, "il tubo tocca la colonnina in basso a destra");
assert(m_x0 + sds_mod[0] + sds_inlet_out < tube_x - 5, "curva del tubo troppo stretta");
assert(s_bay == 0 || bay_y0 > s_iy0 + col_c + col_d/2 && bay_y0 + bay_board[1] < s_iy1 - col_c - col_d/2, "millefori contro le colonnine");

// ---------------------------------------------------------------------
//  Sezione: scatola
// ---------------------------------------------------------------------
module sds_base() {
    difference() {
        union() {
            difference() {
                translate([s_x0, s_iy0 - wall, -floor_t])
                    rbox([s_il + 2*wall, s_iw + 2*wall, s_in_h + floor_t], corner_r);
                translate([s_ix0, s_iy0, 0]) rbox([s_il, s_iw, s_in_h + 1], corner_r - wall);
            }
            for (p = s_col) translate([p[0], p[1], 0]) cylinder(d = col_d, h = s_in_h);
            // rondelle da 0,3 sulla faccia verso la scatola (tengono lo spazio tra le scatole)
            if (!sds_fused) for (y = sds_bolt_ys) translate([s_x0 + 0.01, y, sds_bolt_z]) rotate([0, -90, 0]) cylinder(d = 9, h = s_gap + 0.01);
            // un pezzo solo: riempie tra le due pareti fino al bordo inclinato (sopra resta la fessura per il coperchio)
            if (sds_fused) intersection() {
                translate([inner_l + 0.01, max(s_iy0, 0) - wall, -floor_t]) cube([s_x0 - inner_l + wall - 0.02, min(s_iy1, inner_w) - max(s_iy0, 0) + 2*wall, 60]);
                below_rim(0);
            }
            sds_inner_parts();
            sds_ear();
        }
        sds_cuts();
        // viti M3 d'angolo (verticali) per il coperchio
        for (p = s_col) translate([p[0], p[1], s_in_h - screw_pilot_depth]) cylinder(d = screw_pilot_d, h = screw_pilot_depth + 1);
        // unione con la scatola: due viti M3 passanti e il passacavo
        if (!sds_fused) for (y = sds_bolt_ys) translate([s_x0 - 1, y, sds_bolt_z]) rotate([0, 90, 0]) cylinder(d = screw_clear_d, h = wall + 2);
        translate([inner_l - 1, sds_cable[0], sds_cable[1]]) rotate([0, 90, 0]) cylinder(d = sds_cable_d, h = s_x0 - inner_l + wall + 2);
    }
}
// interno del vano SDS011 (lo usa anche la scatola lunga in un pezzo)
module sds_inner_parts() {
    sds_seat();
    // colonnine della millefori (viti M2)
    if (s_bay > 0) for (h = bay_holes) translate([h[0], h[1], 0]) cylinder(d = 5, h = bay_so_h);
    tube_clips();
    // collare sotto il foro d'ingresso: il tubo esce protetto e l'acqua gocciola via
    translate([tube_x, s_iy0 - wall + 0.01, in_z]) rotate([90, 0, 0]) cylinder(d = tube_od + 5, h = 4);
}
module sds_cuts() {
    if (s_bay > 0) for (h = bay_holes) translate([h[0], h[1], bay_so_h - h_pilot_depth]) cylinder(d = h_pilot_d, h = h_pilot_depth + 1);
    // DS18B20 (parete in BASSO, accanto all'ingresso dell'aria)
    translate([ds_sx, s_iy0 + 1, ds_z]) rotate([90, 0, 0]) cylinder(d = ds_d, h = wall + 2);
    // ingresso aria: foro per il tubo nella parete in BASSO (passa anche il collare)
    translate([tube_x, s_iy0 + 1, in_z]) rotate([90, 0, 0]) cylinder(d = tube_od + 0.4, h = wall + 6);
    // uscita aria: griglia nella parete in BASSO, a sinistra (asole verticali, barrette da 2)
    for (x = [grill_x[0] : 5 : grill_x[1] - 3])
        translate([x, s_iy0 - wall - 1, 4]) cube([3, wall + 2, s_in_h - 8]);
    if (labels) {
        floor_label(m_x0 + sds_mod[0]/2, (m_y0 + m_y1)/2, "SDS011", 6);
        floor_label(m_x0 - s_chim/2, 30, "J34", 2.5, 90);
        if (s_bay > 0) floor_label(bay_x0 + bay_board[0]/2, bay_y0 + bay_board[1]/2, "BOOST 5V", 3, 90);
        floor_label(ds_sx, s_iy0 + 4, "DS", 2.2);
    }
}
// piedini sotto gli angoli + battute basse (solo 2 mm sopra i piedini: non coprono le aperture sui lati)
module sds_seat() {
    c = 0.4;  hb = sds_pad_h + 2;
    for (x = [m_x0, m_x0 + sds_mod[0] - 8], y = [m_y0, m_y1 - 8]) translate([x, y, 0]) cube([8, 8, sds_pad_h]);
    for (x = [m_x0 + 6, m_x0 + sds_mod[0] - 18]) {                   // sotto e sopra (Y)
        translate([x, m_y0 - c - 1.6, 0]) cube([12, 1.6, hb]);
        translate([x, m_y1 + c, 0]) cube([12, 1.6, hb]);
    }
    for (y = [m_y0 + 6, m_y1 - 18]) {                                // a sinistra e a destra (X)
        translate([m_x0 - c - 1.6, y, 0]) cube([1.6, 12, hb]);
        translate([m_x0 + sds_mod[0] + c, y, 0]) cube([1.6, 12, hb]);
    }
}
// due mollette a U per il tratto verticale del tubo (il tubo entra dall'alto a pressione)
module tube_clips() {
    for (y = [s_iy0 + 8, in_y - 20]) translate([tube_x, y, 0]) difference() {
        translate([-tube_od/2 - 2, -2, 0]) cube([tube_od + 4, 4, in_z + 2]);
        translate([0, -3, in_z]) rotate([-90, 0, 0]) cylinder(d = tube_od + 0.2, h = 6);
        translate([-(tube_od - 1.2)/2, -3, in_z]) cube([tube_od - 1.2, 6, 10]);
    }
}
module sds_ear() {
    x_wall = s_ix1 + wall;
    translate([0, (s_iy0 + s_iy1)/2, -floor_t]) difference() {
        hull() {
            translate([x_wall - 2, -ear_w/2, 0]) cube([0.01, ear_w, ear_t]);
            translate([x_wall + ear_len - ear_w/4, 0, 0]) cylinder(d = ear_w/2, h = ear_t);
        }
        translate([x_wall + ear_len/2, 0, -1]) cylinder(d = ear_slot_d, h = ear_t + 2);
    }
}

// ---------------------------------------------------------------------
//  Sezione: coperchio piatto con la sede del secondo pannello
// ---------------------------------------------------------------------
module sds_lid() {
    translate([0, 0, s_in_h]) union() {
        difference() {
            union() {
                translate([s_x0, s_iy0 - wall, 0]) rbox([s_il + 2*wall, s_iw + 2*wall, lid_T], corner_r);
                // labbro dentro le pareti
                translate([0, 0, -lid_lip_h]) difference() {
                    translate([s_ix0 + lid_clear, s_iy0 + lid_clear, 0])
                        rbox([s_il - 2*lid_clear, s_iw - 2*lid_clear, lid_lip_h + 0.01], corner_r - wall - lid_clear);
                    translate([s_ix0 + lid_clear + lid_lip_t, s_iy0 + lid_clear + lid_lip_t, -1])
                        cube([s_il - 2*(lid_clear + lid_lip_t), s_iw - 2*(lid_clear + lid_lip_t), lid_lip_h + 2]);
                }
                // quattro tamponi che tengono giu' il sensore (0,3 di gioco: metti un velo di gomma)
                for (x = [m_x0 + 10, m_x0 + sds_mod[0] - 10], y = [m_y0 + 10, m_y1 - 10])
                    translate([x, y, -(s_in_h - m_z1) + 0.3]) cylinder(d = 8, h = s_in_h - m_z1 - 0.3 + 0.01);
            }
            for (p = s_col) {
                translate([p[0], p[1], -lid_lip_h - 1]) cylinder(d = col_d + 1, h = lid_lip_h + 1);   // scarico del labbro
                translate([p[0], p[1], -5]) cylinder(d = screw_clear_d, h = 20);
                translate([p[0], p[1], lid_t - 0.3]) cylinder(d = screw_head_d, h = 20);
            }
            // sede del pannello (stessa dell'altro coperchio) e linguette
            pv_seat(s_pv_xc, s_pv_v0, s_iy1 + wall, s_iy0 - wall - 1);
            pv_snap_cuts(s_pv_xc, s_pv_vt);
            // fili del pannello: foro nell'intercapedine, sopra il canale di scarico
            translate([s_pv_hole[0], s_pv_hole[1], -1]) cylinder(d = pv_hole_d, h = lid_t + 2);
            // se il foro e' fuori dalla sede: galleria nello spessore fino all'intercapedine dietro il pannello
            if (s_pv_hole[0] < s_pv_xc - pv_dx/2 + pv_side + pv_ledge + 3)
                translate([s_pv_hole[0], s_pv_hole[1] - 2, lid_t]) cube([s_pv_xc - pv_dx/2 + pv_side + pv_ledge + 4 - s_pv_hole[0], 4, pv_gap + 0.01]);
        }
        pv_teeth(s_pv_xc, s_pv_vt);
    }
}
module sds_lid_print() {
    translate([0, 0, lid_T]) mirror([0, 0, 1]) translate([0, 0, -s_in_h]) sds_lid();
}

// ---------------------------------------------------------------------
//  Sagome (solo vista assembly)
// ---------------------------------------------------------------------
module sds_dummies() {
    color("DimGray", 0.6) translate([m_x0, m_y0, sds_pad_h]) cube([sds_mod[0], sds_fan_h, sds_mod[2]]);              // parte con la ventola
    color("DimGray", 0.6) translate([m_x0, m_y0 + sds_fan_h, sds_pad_h]) cube([sds_mod[0], sds_mod[1] - sds_fan_h, sds_t_top]);
    color("DimGray", 0.6) translate([m_x0 + sds_mod[0], in_y, in_z]) rotate([0, 90, 0]) cylinder(d = 6, h = sds_inlet_out);
    // tubo: tratto orizzontale, curva, tratto verticale fino al foro
    color("Orange", 0.7) {
        r = tube_x - (m_x0 + sds_mod[0] + sds_inlet_out);
        translate([m_x0 + sds_mod[0] + sds_inlet_out, in_y - r, in_z])
            rotate_extrude(angle = 90) translate([r, 0]) circle(d = tube_od);
        translate([tube_x, s_iy0 - wall - 2, in_z]) rotate([-90, 0, 0]) cylinder(d = tube_od, h = in_y - r - s_iy0 + wall + 2);
    }
}
module sds_panel() {
    translate([s_pv_xc - pv_dx/2, s_pv_v0 + pv_clr, s_in_h + lid_t + pv_gap]) cube([pv_dx, pv_dv, pv_t]);
}

