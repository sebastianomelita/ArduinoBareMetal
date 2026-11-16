// =====================================================================
//  TESTINA PER IL SENSORE UV (VEML6075, scheda CJMCU-6075 18,2 x 10,5 x 2)
//  Piccola scatola separata, da fissare all'aperto con il sensore verso il
//  cielo e collegare alla scatola principale con un cavetto a 4 fili
//  (3,3 V, GND, SCL, SDA) che entra dal pressacavo.
//  Tre pezzi:
//    - tazza (cup): contiene la scheda, con due alette per le viti M3;
//    - coperchio (lid): ha la finestra sopra il sensore;
//    - finestra: un pezzetto di materiale TRASPARENTE AGLI UV 19 x 11,5,
//      spesso fino a 1 mm (PMMA tipo UV-T, oppure velo di PTFE), che NON si
//      stampa: si incolla con silicone sotto la finestra del coperchio.
//  Montaggio: salda i 4 fili sui pin (tagliando la barretta), metti la
//  scheda con le scritte in basso e il sensore in alto, sigilla con
//  silicone l'uscita del cavo, incolla la finestra, poi il coperchio.
//  ATTENZIONE: la posizione del chip sulla scheda non l'ho vista (la foto
//  mostrava il retro): guarda dove sta e regola win_c (centro della
//  finestra, in mm dallo spigolo in basso a sinistra della scheda).
// =====================================================================

part = "assembly";   // [assembly, cup, lid, print]

board  = [18.2, 10.5, 2];     // scheda misurata con il calibro: lunghezza (con i pin a destra), larghezza, spessore
clr    = 0.3;             // gioco intorno alla scheda
wall   = 1.6;
floor_t = 1.6;
under  = 3;               // spazio sotto la scheda per le saldature dei fili
rail   = 1.2;             // appoggio della scheda sui due lati lunghi
free_r = 6;               // spazio a destra della scheda per i fili
cable_d = 4.6;            // cavetto a 4 fili, diametro
win    = [11, 7.5];       // apertura della finestra
win_c  = [9.5, 5.5];      // centro dell'apertura, dallo spigolo in basso a sinistra della scheda
pane   = [19, 11.5];       // il pezzetto trasparente agli UV
pane_t = 1.2;             // profondita' della sede del vetrino
lid_t  = 2.4;
ear_r  = 4;  ear_d = 3.4; ear_t = 3;   // alette per le viti M3
$fn = 40;

px = board[0] + 2*clr;                // pocket X della scheda
ox = px + free_r + 2*wall;            // esterno X
oy = board[1] + 2*clr + 2*wall;       // esterno Y
h  = floor_t + under + board[2] + clr; // altezza della tazza
bx = wall + clr;                      // dove comincia la scheda (X)
by = wall + clr;                      // e (Y)

module cup() {
    difference() {
        union() {
            cube([ox, oy, h]);
            for (s = [0, 1]) hull() {        // alette sui due lati lunghi, a meta'
                translate([ox/2 - ear_r, s == 0 ? 1 : oy - 1, 0]) cube([2*ear_r, 0.01, ear_t]);
                translate([ox/2, s == 0 ? -3.5 : oy + 3.5, 0]) cylinder(r = ear_r, h = ear_t);
            }
        }
        // tasca superiore (scheda + spazio fili)
        translate([wall, wall, floor_t + under]) cube([ox - 2*wall, oy - 2*wall, h]);
        // cavita' sotto la scheda per le saldature: lascia gli appoggi di 'rail' sui lati lunghi
        translate([wall, wall + rail, floor_t]) cube([ox - 2*wall, oy - 2*wall - 2*rail, under + 0.01]);
        // uscita del cavo nella parete destra
        translate([ox - wall - 1, oy/2, floor_t + 2.7]) rotate([0, 90, 0]) cylinder(d = cable_d, h = wall + 2);
        // viti M3
        for (y = [-3.5, oy + 3.5]) translate([ox/2, y, -1]) cylinder(d = ear_d, h = ear_t + 2);
    }
}
module lid() {
    difference() {
        translate([0, 0, h]) cube([ox, oy, lid_t]);
        // apertura sopra il sensore
        translate([bx + win_c[0] - win[0]/2, by + win_c[1] - win[1]/2, h - 1])
            cube([win[0], win[1], lid_t + 2]);
        // sede del pezzetto trasparente, sotto
        translate([bx + board[0]/2 - pane[0]/2, oy/2 - pane[1]/2, h - 0.01]) cube([pane[0], pane[1], pane_t + 0.01]);
    }
}
module lid_print() { translate([0, 0, h + lid_t]) mirror([0, 0, 1]) lid(); }   // a faccia in giu', senza supporti
module board_dummy() { color("Green") translate([bx, by, floor_t + under]) cube(board); }

if (part == "cup") cup();
else if (part == "lid") lid_print();
else if (part == "print") { cup(); translate([ox + 12, 0, -h]) lid_print(); }
else { cup(); color("SteelBlue") lid(); %board_dummy(); }

echo(str("testina UV: ", ox, " x ", oy, " mm (", oy + 2*(3.5 + ear_r), " con le alette) x ", h + lid_t, " mm"));
