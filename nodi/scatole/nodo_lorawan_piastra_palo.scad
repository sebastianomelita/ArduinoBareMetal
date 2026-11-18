// ============================================================
// PIASTRA DA PALO per le scatole da ESTERNO del nodo LoRaWAN v4
// Fa da "muro" tra il palo e la scatola: la scatola si avvita con le sue
// due alette, come se fosse appesa a una parete.
// File indipendente: non include nessun altro .scad.
//
// COME SI AGGANCIA: la fascetta fa un anello attorno al PALO E ALLA PIASTRA
// INSIEME, passando tutta DIETRO, dentro una cava scavata nel retro.
// Le due aste su cui appoggia il palo scavalcano la cava e fanno da fermo:
// la fascetta ci passa sotto e non puo' uscire.
// Niente sporge dal retro oltre alle due aste: il tondo sta tutto DENTRO la
// cava, dove la fascetta scorre davvero. Verso le due uscite il fondo della
// cava risale con un raccordo, cosi' la fascetta esce gia' puntata verso il
// palo, senza scavalcare nessuno spigolo.
// La faccia davanti e' liscia: fra scatola e piastra non passa niente, e una
// fascetta si cambia senza smontare la scatola.
//
// STAMPA: faccia davanti (liscia) sul piatto, cava e aste in alto.
//         Niente supporti. 3 perimetri, riempimento 40%, PETG o ASA.
// SERVE:  2 viti M4 x 16 a testa bombata + 2 dadi M4 (tasche esagonali)
//         2 fascette: stringitubo in acciaio (nastro max 13 mm) oppure
//         nylon da 9 mm, NERE stabilizzate ai raggi UV.
//         SEMPRE DUE: una sola lascia scivolare e girare la scatola.
// ============================================================

/* [Cosa visualizzare] */
part = "piastra";   // [piastra, con_palo]

/* [Scatola] */
scatola = "corta";  // [corta, corta RS485, lunga, lunga RS485]

/* [Piastra] */
h_piastra = 66;     // altezza del corpo centrale
corpo_w   = 46;     // larghezza del corpo centrale
t         = 6;      // spessore della piastra
pad_d     = 22;     // diametro della piazzola forata in punta al rombo
vite_d    = 4.4;    // foro di passaggio della vite M4
dado_e    = 8.1;    // chiave del dado M4 (esagono, da piano a piano)
dado_h    = 3.6;    // profondita' della tasca del dado
raggio    = 4;      // raccordo degli spigoli

/* [Aste di appoggio sul palo] */
cost_x = 7.5;       // meta' distanza fra le due aste (va bene da 18 mm in su)
cost_h = 3.5;       // altezza delle aste
cost_w = 5;         // larghezza delle aste
// Fra le due aste il retro e' scavato di poco: un palo sottile scende li' in
// mezzo e senza questo scavo toccherebbe la piastra, perdendo l'appoggio sulle
// due aste. 1,5 basta per i tubi da 16 mm in su e lascia 4,5 di piastra.
// Dove passano le cave non cambia niente: li' sono gia' piu' profonde.
canale_d = 1.5;     // 0 = niente scavo

/* [Cava della fascetta] */
fascia_w = 13;      // larghezza utile (nastro d'acciaio da 13 o nylon da 9)
cava_z   = 3;       // quota del fondo piatto (la cava e' profonda t - cava_z)
fascia_y = 22;      // distanza delle due cave dal centro
piano_x  = 13;      // meta' del tratto a fondo piatto (copre le due aste)
rac_r    = 10;      // raggio del raccordo che risale verso le uscite
asta_r   = 1.5;     // raggio con cui le aste sono arrotondate dentro la cava

/* [Nervature] */
// Sono due creste basse, alte quanto le aste: partono dalle aste e arrivano
// fino alle viti, cosi' aste, creste e piazzole fanno un telaio unico.
nerv_w  = 6;        // larghezza delle creste

/* [Solo per l'anteprima] */
palo_d = 48;        // [12:1:120] palo da mostrare in "con_palo": non cambia il pezzo

$fn = 64;

// ---------- misure prese dalle scatole ----------
// interasse dei fori delle alette:
//   corta e corta RS485  98,8    lunga  199,2    lunga RS485  221,2
interasse = scatola == "lunga"       ? 199.2 :
            scatola == "lunga RS485" ? 221.2 : 98.8;
h_scatola = scatola == "corta RS485" ? 87.8 : 83.3;   // solo per l'anteprima
pad_x     = interasse / 2;

module rbox2d(w, h, r) {
    offset(r = r) offset(r = -r) square([w, h], center = true);
}

// sagoma in piano: rombo allungato, dal corpo centrale alle due piazzole
module corpo_2d() {
    hull() {
        rbox2d(corpo_w, h_piastra, raggio);
        for (s = [-1, 1]) translate([s * pad_x, 0]) circle(d = pad_d);
    }
}

// cresta bassa, stessa sezione delle aste, da x0 a x1
module cresta(x0, x1) {
    L = x1 - x0;
    hull() {
        translate([x0, -nerv_w/2, t - 0.01]) cube([L, nerv_w, 0.01]);
        translate([x0, 0, t + cost_h - 1]) rotate([0, 90, 0]) cylinder(r = 1, h = L);
    }
}

// smusso tondo su uno spigolo di sotto dell'asta (e = -1 sinistro, +1 destro)
module smusso_2d(xc, e) {
    difference() {
        translate([xc + (e < 0 ? 0 : -asta_r), t]) square([asta_r, asta_r]);
        translate([xc - e * asta_r, t + asta_r]) circle(r = asta_r);
    }
}

// sezione della cava, nel piano X-Z: fondo piatto al centro, poi risale con
// un raccordo tondo fino a sfiorare il retro. Tutto il tondo sta qui dentro.
// In piu', DENTRO la cava le due aste sono arrotondate di sotto: la fascetta
// che ci passa sotto trova un tondo, non uno spigolo.
module cava_2d() {
    union() {
        intersection() {
            translate([0, (cava_z + t)/2])
                square([corpo_w + 8, t - cava_z], center = true);
            union() {
                square([2 * piano_x, 4 * t], center = true);
                for (s = [-1, 1])
                    translate([s * piano_x, cava_z + rac_r]) circle(r = rac_r);
            }
        }
        for (s = [-1, 1], e = [-1, 1])
            smusso_2d(s * cost_x + e * cost_w/2, e);
    }
}

module piastra() {
    difference() {
        union() {
            linear_extrude(t) corpo_2d();                 // lastra piatta
            // le due aste su cui appoggia il palo: scavalcano la cava e
            // fanno da fermo alla fascetta
            for (s = [-1, 1]) translate([s * cost_x, 0, t - 0.01])
                hull() {
                    translate([-cost_w/2, -h_piastra/2, 0])
                        cube([cost_w, h_piastra, 0.01]);
                    translate([0, -h_piastra/2, cost_h - 1])
                        rotate([-90, 0, 0]) cylinder(r = 1, h = h_piastra);
                }
            // creste: dalle viti fino alla CRESTA dell'asta, dentro la quale
            // entrano per meta' larghezza: si compenetrano e fanno un pezzo
            // solo, ma non scendono nel canale fra le due aste, dove deve
            // restare solo l'asta (li' appoggia il palo).
            for (s = [-1, 1]) scale([s, 1, 1])
                cresta(cost_x, pad_x - 6);
        }
        // scavo poco profondo fra le due aste, per i pali sottili.
        // E' un arco di cerchio, non un incavo squadrato: niente spigoli vivi
        // dove partirebbe una crepa, e un palo sottile che scende fin li'
        // trova una superficie curva come la sua.
        if (canale_d > 0) {
            w = 2 * (cost_x - cost_w/2);                  // corda dello scavo
            R = (w * w) / (8 * canale_d) + canale_d / 2;  // raggio dell'arco
            intersection() {
                translate([0, -h_piastra/2 - 1, t - canale_d + R])
                    rotate([-90, 0, 0]) cylinder(r = R, h = h_piastra + 2, $fn = 160);
                // l'arco si ferma al retro e fra le due aste: le aste non si toccano
                translate([-w/2, -h_piastra/2 - 1, t - canale_d - 1])
                    cube([w, h_piastra + 2, canale_d + 1]);
            }
        }
        // le due cave della fascetta
        for (sy = [-1, 1])
            translate([0, sy * fascia_y + (fascia_w + 1)/2, 0])
                rotate([90, 0, 0]) linear_extrude(fascia_w + 1) cava_2d();
        // fori delle viti e tasche dei dadi (il dado sta dietro)
        for (s = [-1, 1]) translate([s * pad_x, 0, 0]) {
            translate([0, 0, -1]) cylinder(d = vite_d, h = t + 2);
            translate([0, 0, t - dado_h])
                cylinder(d = dado_e / cos(30), h = dado_h + 1, $fn = 6);
        }
    }
}

// palo e ingombro della scatola, solo per l'anteprima
module anteprima() {
    r   = palo_d / 2;
    sag = r > cost_x ? r - sqrt(r*r - cost_x*cost_x) : 1e6;
    z   = sag < cost_h ? t + cost_h + sqrt(r*r - cost_x*cost_x) : t + r;
    l_scatola = interasse - 12;     // il foro dell'aletta sta a 6 mm dal fianco
    color("Gray", 0.35)
        translate([0, -h_scatola/2 - 40, z])
            rotate([-90, 0, 0]) cylinder(d = palo_d, h = h_scatola + 80);
    color("SteelBlue", 0.30)
        translate([-l_scatola/2, -h_scatola/2, -46]) cube([l_scatola, h_scatola, 46]);
}

piastra();
if (part == "con_palo") anteprima();

echo(str("PIASTRA ", scatola, ": ", interasse + pad_d, " x ", h_piastra,
         " mm, spessore ", t + cost_h, " mm (niente sporge oltre le aste)"));
