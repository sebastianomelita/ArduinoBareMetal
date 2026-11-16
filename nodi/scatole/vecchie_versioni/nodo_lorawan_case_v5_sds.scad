// =====================================================================
//  Versione 5 + SDS011 (versione lunga) (polveri sottili PM2.5 / PM10)
//
//  La scatola v5 resta IDENTICA (la prende dal suo file); in piu':
//    - due fori M3 e un passacavo nella sua parete destra;
//    - niente aletta destra (la porta la sezione nuova).
//  A destra si avvita la sezione piatta con l'SDS011 (file sds011_sezione.scad).
//  Stessa origine e stessi assi della v5.
// =====================================================================

include <nodo_lorawan_case_v5.scad>
include <sds011_sezione.scad>

// la v5 non disegna niente da sola; si accende il lato SDS
v5_render  = false;
sds_side   = true;
ear_right  = false;
sds_cable_d = 7;          // Vext 3,3 V, GND, TX e RX dell'SDS011, 3 fili del DS18B20, 2 del secondo pannello
ds_hole = false;          // il DS18B20 entra dalla sezione SDS011

/* [Cosa visualizzare] */
part = "assembly"; // [assembly, sds_base, sds_lid, v5_base, v5_lid, print_sds]

// ---------------------------------------------------------------------
if (part == "sds_base") {
    sds_base();
} else if (part == "sds_lid") {
    sds_lid_print();
} else if (part == "v5_base") {
    base();
} else if (part == "v5_lid") {
    lid_print();
    translate([inner_l + 25, 20, 0]) usb_plug();
} else if (part == "print_sds") {
    translate([-s_x0, -s_iy0 + wall, 0]) sds_base();
    translate([-s_x0 + s_il + 2*wall + 10, -s_iy0 + wall, 0]) sds_lid_print();
} else {
    base(); %dummies();
    sds_base(); %sds_dummies();
    translate([0, 0, explode]) {
        lid(); color("MidnightBlue") panels();
        sds_lid(); color("MidnightBlue") sds_panel();
    }
}
