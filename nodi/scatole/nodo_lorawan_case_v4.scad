// =====================================================================
//  Contenitore CORTO per nodo LoRaWAN  (versione 4: vano sensori in basso)
//  Heltec WiFi LoRa 32 V4 + batteria 14500 + BMS 1S + GPS L76K
//  + vano sensori STAGNO con SCD41 e MPL3115A2
//
//  Come la v3 (bordo inclinato, coperchio piatto con la sede a incastro del
//  pannello), ma con i pezzi impilati dal basso verso l'alto:
//     parete in BASSO -> vano sensori (feritoia) -> batteria + BMS
//     -> Heltec con il GPS accanto -> parete in ALTO (antenna)
//  A destra non resta spazio vuoto: 82 x 61 interni invece di 105,5 x 44,5.
//  La scatola e' piu' alta, quindi la pendenza scende a 8 gradi (a 11,5 non
//  ci starebbe la Heltec sotto il coperchio, in alto).
//  Due coperchi per la stessa scatola, si sceglie con "coperchio" qui sotto:
//    A = 1 pannello 80 x 45 coricato;  B = 2 pannelli in piedi affiancati.
//
//  Installazione: a muro, lato lungo orizzontale.
//    parete frontale (Y = 0)        -> guarda in BASSO (feritoia sensori)
//    parete posteriore (Y = inner_w) -> guarda in ALTO
//
//  Misure in mm. Origine: angolo interno sinistro-anteriore del fondo.
//  X = lunghezza, Y = larghezza, Z = profondita' dal fondo verso l'esterno.
// =====================================================================

/* [Cosa visualizzare] */
coperchio = "A"; // [A:A - 1 pannello coricato, B:B - 2 pannelli in piedi, C:C - senza pannelli (da interno)]
indoor = false;   // versione da INTERNO: feritoie larghe nel vano sensori e aerazione per il calore delle schede
aerazione = "massima"; // [massima, ridotta] da interno: "ridotta" toglie le asole del coperchio sopra le schede
part = "assembly"; // [assembly, base, lid, plate, print]   (plate = piastra estraibile dei sensori)
explode = 20;

/* [Sensore UV] */
baccello_uv = "no";  // [no, si] "si" = baccello del sensore UV (VEML6075) sulla parete in alto
uv_x   = 41;       // centro del baccello lungo X (meta' di inner_l = 82)
uv_pod = (baccello_uv == "si");   // non toccare: la comanda la voce qui sopra

/* [Scatola] */
inner_l = 82;      // lunghezza interna (X): Heltec + GPS accanto + passaggio cavi
inner_w = 61;      // larghezza interna (Y): vano 16,5 + culla batteria 17 + Heltec 25,6 + margini
inner_h = 22.5;    // profondita' interna sulla parete in BASSO (Y = 0); verso l'alto cala con box_tilt
box_tilt = 8;      // inclinazione del bordo (gradi): a 8 in alto restano 13,9 (la Heltec ne vuole 12,9)
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
screw_head_h  = 1.65;  // vite a TESTA A BOTTONE ISO 7380 M3x12 inox (testa 5,7 x 1,65): con i coperchi B
                       // i pannelli passano sopra le viti, una testa bombata ISO 7045 (alta 2,4) li tocca
screw_seat_w  = 1.8;   // fondo della sede della testa, misurato perpendicolare al coperchio: la testa
                       // resta sotto il retro dei pannelli (che sta a 3,9)

/* [Heltec WiFi LoRa 32 V4] */
h_len = 51.6;      // lungo X (lato USB a sinistra)
h_wid = 25.6;      // lungo Y
h_pcb = 1.7;
h_x   = 6;         // distanza del lato USB dalla parete sinistra: spinotti, cavi e colonnina d'angolo
strip_h = 0;       // striscia ELETTRONICA tra vano e batteria (boost 5 V, MOS, condensatore): 0 = niente (v4 semplice)
strip_board = [60, 12];   // millefori della striscia (taglia una millefori a 60 x 12), su due colonnine M2
ch_depth = 14.5;   // spazio tra parete frontale e parete dei sensori
ch_wall = 2;
stack_y = ch_depth + ch_wall + strip_h; // vano sensori + parete + striscia: batteria, BMS, Heltec e GPS salgono di tanto
h_y   = 17.6 + stack_y;
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
batt_x = 5.5;      // con le lamelle: la cella comincia a batt_x + stop_t = 7,5 (prima: molla e taschina, contro la parete)
batt_yc = 8.15 + stack_y;   // asse della cella: a 0,9 dalla parete del vano
batt_span = 17;    // larghezza delle selle (da Y = 0)
stop_t = 2;
saddle_w = 8;
saddle_snap = 2.75;
// LAMELLE DI CONTATTO a slitta (piastrine per pile AA, circa 12 x 14): ognuna entra dall'alto
// in una TASCHINA a fine culla. A sinistra la lamella con la molla (-), a destra quella piatta (+).
batt_contacts = true;   // false = i vecchi fermi pieni, senza lamelle
contact_w = 12;    // larghezza della piastrina (lungo Y)
contact_h = 14;    // altezza della piastrina (lungo Z), centrata sull'asse della cella
contact_t = 0.6;   // spessore della lamiera: la fessura e' larga contact_t + 0.4
spring_len = 4.5;  // molla compressa + piastrina, dalla faccia della taschina alla cella
nub_len = 1.0;     // piastrina piatta (+) con bottoncino, dalla cella alla taschina
holder_wall = 1.0; // pareti della taschina, davanti e dietro la fessura // le selle salgono 2,75 sopra l'asse della cella: la batteria entra a scatto (niente fascette)

/* [BMS 1S in piedi, tra batteria e Heltec] */
bms_l = 35.2;
bms_h = 4.2;       // la larghezza della scheda diventa l'altezza
bms_t = 2.0;       // spessore con i componenti
bms_clr = 0.4;
bms_wall = 1.2;
bms_slot_h = 6;
bms_x = 13.5;      // inizio della scheda: tra la batteria e la Heltec, parallela alla batteria
bms_y = 17.6 + stack_y;      // faccia verso la batteria (dietro le selle); sopra c'e' il bordo della Heltec (Z 5,5)

/* [GPS L76K: modulo 17,5 x 16,5 x 5,5 in piedi, antenna verso la parete in alto] */
gps = true;
gps_mod = [17.5, 16.5, 5.5];   // lungo X (connettore a sinistra, cavo verso la Heltec), alto, spesso (ingombro totale)
gps_x = 58.4;                  // subito a destra della Heltec: il cavo corto originale passa sotto la scheda
gps_y = 17.8 + stack_y;                  // retro (chip e connettore); l'antenna ceramica guarda +Y, cioe' il cielo a scatola appesa
gps_clr = 0.2;                 // gioco per lato nelle guide
gps_wall = 0.8;                // spessore delle guide

/* [Vano sensori stagno] */
ch_r   = 50;       // parete DESTRA del vano (X 50..52): a destra resta un corridoio per cavi e DS18B20
seal_gap = 0.6;    // spazio sotto il coperchio per il cordone di silicone
vent_len = 22;     // UNA sola feritoia, sulla parete che guarda in basso
vent_h   = 2.5;
vent_z   = 1.2;    // a filo del fondo: scarica anche la condensa
sensor_plate = true; // la parete con i sensori e' un pezzo a parte: si avvitano i sensori fuori, poi si infila dall'alto
plate_clr = 0.15;  // gioco per lato della piastra nelle guide
sensor_plate = true; // parete dei sensori ESTRAIBILE: si avvitano i sensori fuori dalla scatola, poi la si infila dall'alto
plate_clr = 0.15;  // gioco per lato della piastra nelle guide
so_h = 4;          // distanziali dei sensori sulla parete
so_d = 4.5;
mpl_x0 = 37;       // MPL3115A2 sulla stessa parete del SCD41, a destra (chip verso la parete)
scd_x0 = 8;        // SCD41: bordo sinistro
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
mpl_holes = [[6, 13.5]];
// schede avvitate sulla piastra dei sensori: [X del bordo sinistro, fori [x, z] dalla base della scheda]
plate_boards = [[scd_x0, scd_holes], [mpl_x0, mpl_holes]];
guide_l = 3;       // lunghezza (in X) delle costole di guida della piastra
ch_wire_holes = true;   // passacavi nella parete destra del vano (false se il vano arriva alla parete della scatola)
plate_notches = [];     // X delle tacche in cima alla piastra per far passare i fili (versione da interno)

/* [Passanti] */
ds_d = 4.5;        // foro sigillato per il cavo DS18B20 (parete in basso)
ds_x = 66;
ds_z = 12;
sma_d = 6.6;       // antenna SMA
sma_side = "top";  // [top, right] dove esce l'antenna LoRa: in alto, dritta (il dado sta 18 mm davanti all'antenna del GPS: effetto piccolo)
sma_right = [40, 6]; // parete DESTRA (Y, Z), se sma_side = "right": accanto al GPS; serve un'antenna con lo snodo a 90 gradi
sma_top = [70, 6]; // parete in ALTO (X, Z)

/* [Pannelli solari 80 x 45 incollati sul coperchio] */
pv_count = coperchio == "B" ? 2 : coperchio == "C" ? 0 : 1;   // lo sceglie "coperchio" in cima: A = 1 pannello coricato; B = 2 in piedi (coperchio piu' largo di 8 mm per lato, scende 32 mm sotto la scatola)
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
// LINGUETTE A SCATTO LATERALI: una lamella nella parete della sede, ai due lati del bordo alto di
// ogni pannello. Si flette di lato (nel piano del coperchio: si stampa a strati nel verso giusto)
// e il suo dentino entra di snap_hook nel canale, sopra l'angolo del pannello.
snap_len = 14;     // lunghezza della lamella (lunga = si flette senza rompersi: deformazione ~0,8 %)
snap_th = 1.0;     // spessore della lamella
snap_hook = 1.0;   // quanto il dentino entra nel canale (il pannello ha 0,4 di gioco: lo copre di 0,6)
snap_slit = 1.2;   // fessura dietro la lamella: lo spazio in cui si piega
snap_ramp = 2.0;   // rampa del dentino, dal lato da cui entra il pannello
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
ear_right = true;  // false nella versione con SDS011: l'aletta destra la porta la sezione aggiunta

/* [Sezione SDS011 (la attiva il file _sds)] */
sds_side = false;  // true = fori nella parete destra per unire la sezione SDS011
sds_bolt_ys = [11, 46];   // due viti M3 passanti (Y), a Z sds_bolt_z, dado dentro ciascuna scatola
sds_bolt_z = 6;
sds_cable = [28, 10];     // passacavo a 4 fili (Y, Z), da sigillare
sds_cable_d = 4.5;
gland = true;      // foro per un pressacavo PG7 (sensore del terreno o altri esterni), chiudibile col tappo stampato
gland_d = 12.5;    // filetto PG7 (cavi da 3 a 6,5 mm; ci passa anche il connettore PH a 3 poli)
gland2 = false;     // secondo pressacavo PG7, sulla parete SINISTRA (versione con RS485)
gland2_yz = [27, 9.5];
gland_yz = [25, 9.5];  // parete DESTRA (Y, Z): libera dentro per il dado (X 77..82) tra la pila e il GPS
soil_ds = true;    // secondo DS18B20 (temperatura del TERRENO): foro sigillato Ø ds_d sulla stessa parete del pressacavo
soil_ds_yz = [12, 9.5];   // parete DESTRA (Y, Z), sotto il pressacavo
ds_hole = true;    // false nella versione _sds: il DS18B20 entra dalla sezione, accanto all'ingresso dell'aria
v4_render = true;  // false quando il file e' incluso da un altro

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
sw_y = ch_depth;
scd_x = scd_x0;
vent_x = ch_r/2 - vent_len/2;
bms_in_w = bms_t + 2*bms_clr;
bms_in_l = bms_l + 1.2;
// coperchio: riferimento sul piano del bordo (x = X, v = lungo la pendenza verso l'alto, w = uscente)
LM = [[1, 0, 0, 0], [0, cos(box_tilt), sin(box_tilt), 0], [0, -sin(box_tilt), cos(box_tilt), inner_h], [0, 0, 0, 1]];
LMinv = [[1, 0, 0, 0], [0, cos(box_tilt), -sin(box_tilt), 0], [0, sin(box_tilt), cos(box_tilt), 0], [0, 0, 0, 1]];
v_lo  = -wall / cos(box_tilt);                  // bordo basso del coperchio (lungo la pendenza)
v_hi  = (inner_w + wall) / cos(box_tilt);       // bordo alto
// pannelli: 1 coricato (80 in X) tra le viti in basso e quelle in alto, oppure 2 o piu' in piedi affiancati
pv_shift = 0;
pv_gap2 = 6;       // pannelli in piedi: spazio tra due vicini (almeno 4: ci stanno le linguette con le loro fessure)
pv_xs  = pv_count >= 2 ? [for (i = [0 : pv_count - 1]) inner_l/2 + (i - (pv_count - 1)/2) * (pv_w + pv_gap2)]
                       : [inner_l/2 + pv_shift];
pv_dx  = pv_count >= 2 ? pv_w : pv_l;           // larghezza (X) di un pannello
pv_dv  = pv_count >= 2 ? pv_l : pv_w;           // lunghezza (pendenza) di un pannello
pv_vtop = v_hi - 1;                             // bordo alto dei pannelli (v)
// coperchio: 1 pannello = largo come la scatola; in piedi = sporge di lato quanto serve alle sedi e alle linguette
ext_x0 = pv_count >= 2 ? min(-wall, inner_l/2 - pv_count*pv_w/2 - (pv_count - 1)*pv_gap2/2 - pv_side - 3.2)
                       : max(-wall, inner_l/2 - pv_l/2 - 3);
ext_w  = pv_count >= 2 ? inner_l - 2*ext_x0 : min(inner_l + wall, inner_l/2 + pv_l/2 + 3) - ext_x0;
pv_hole_v = 25;                                 // in piedi: fori dei fili a 25 (sopra la batteria, fuori dal vano sensori)
pad_h = pv_count > 0 ? pv_gap + pv_edge + pv_groove + pv_lip_t : 0;   // spessore aggiunto al coperchio per la sede (5,1)
lid_T = lid_t + pad_h;                          // spessore totale del coperchio
// le linguette finiscono 0,5 prima del pozzetto delle viti in alto (misurato sulla faccia esterna)
// la fessura in cima alla lamella finisce 0,5 prima del pozzetto delle viti in alto (faccia esterna)
snap_end = (col_pos[2][1] - sin(box_tilt) * lid_T) / cos(box_tilt) - screw_head_d/2 / cos(box_tilt) - 0.5;
pv_v0 = snap_end - 0.6 - 0.3 - snap_ramp - 0.3 - pv_dv - pv_clr;   // fondo della sede: il pannello ci appoggia (chiusa in basso, aperta in alto)
pv_vt = pv_v0 + pv_clr + pv_dv;                 // bordo alto del pannello inserito
ext_v0 = pv_count >= 2 ? pv_v0 - 3                // in piedi: 3 mm di bordo sotto la sede
                       : v_lo - visor / cos(box_tilt);   // 1 pannello: scende davanti alla feritoia
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
        translate([xs, stack_y, 0]) difference() {
            // sella a scatto: l'apertura in cima e' piu' stretta della cella (14,1 contro 14,5)
            cube([saddle_w, batt_span, zc + saddle_snap]);
            translate([-1, batt_yc - stack_y, zc]) rotate([0, 90, 0]) cylinder(d = batt_d + 0.6, h = saddle_w + 2);
        }
    if (batt_contacts) {
        contact_holder(x0 - spring_len, -1);           // molla (-): la taschina sta a sinistra della molla
        contact_holder(x0 + batt_l + nub_len, 1);      // piatta (+)
    } else
        for (xs = [batt_x, x0 + batt_l])
            translate([xs, batt_yc - 7.15, 0]) cube([stop_t, 14.3, batt_d * 0.6]);
}
// taschina per una lamella a slitta: xf = faccia verso la cella, d = verso di uscita (-1 sinistra, +1 destra).
// Fessura aperta in alto (la lamella entra dall'alto), pareti laterali che la tengono per i bordi,
// finestra davanti per la molla o il bottoncino, scasso dietro per la linguetta e il filo.
module contact_holder(xf, d) {
    zc = batt_d/2 + 1;
    slot = contact_t + 0.4;
    t_tot = 2*holder_wall + slot;
    yw = contact_w + 0.6;                       // fessura in Y
    yb = yw + 2*1.6;                            // taschina in Y
    z0 = zc - contact_h/2 - 0.2;                // fondo della fessura
    hh = zc + contact_h/2 - 2;                  // alta fin quasi al bordo della lamella
    xa = d < 0 ? xf - t_tot : xf;               // X minima della taschina
    difference() {
        translate([xa, batt_yc - yb/2, 0]) cube([t_tot, yb, hh]);
        // fessura per la lamella
        translate([xa + holder_wall, batt_yc - yw/2, z0]) cube([slot, yw, hh]);
        // finestra davanti (verso la cella): passa la molla o il bottoncino
        translate([d < 0 ? xf - holder_wall - 0.1 : xf - 0.1, batt_yc - (contact_w - 3)/2, z0 + 1.5])
            cube([holder_wall + 0.2, contact_w - 3, hh]);
        // scasso dietro, in alto al centro: linguetta da saldare e filo
        translate([d < 0 ? xa - 0.1 : xf + holder_wall + slot - 0.1, batt_yc - 3, zc])
            cube([holder_wall + 0.2, 6, hh]);
    }
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
    // (1 mm piu' bassa del modulo: a 8 gradi il coperchio, sopra il GPS, e' a 16,8)
    translate([x1, y0, 0]) cube([gps_wall, y1 + gps_wall - y0, gps_mod[1] - 1]);
    for (y = [y0, y1]) translate([x1 - 2.5, y, 0]) cube([2.5, gps_wall, gps_mod[1] - 1]);
    // alette dietro e davanti anche all'estremita' sinistra (il lato del connettore resta libero):
    // il modulo non si inclina piu' verso la Heltec
    for (y = [y0, y1]) translate([gps_x, y, 0]) cube([2.5, gps_wall, gps_mod[1] - 1]);
}
module gps_dummy() {
    translate([gps_x, gps_y, 0]) cube([gps_mod[0], gps_mod[2], gps_mod[1]]);
}

// pareti del vano: fino a 0,6 sotto il coperchio (cordone di silicone)
module sensor_chamber() {
    intersection() {
        union() {
            if (!sensor_plate) translate([0, sw_y, 0]) cube([ch_r + ch_wall, ch_wall, inner_h]);   // parete sotto la batteria
            if (ch_r < inner_l - 0.1) translate([ch_r, 0, 0]) cube([ch_wall, sw_y + ch_wall, inner_h]);   // parete destra
        }
        below_rim(-seal_gap);
    }
    if (sensor_plate) plate_guides();
    if (ccs) ccs_holder();
    if (ze07) ze07_pocket();
    else if (!sensor_plate) {   // senza piastra estraibile: distanziali sulla parete fissa
        for (h = scd_holes) sensor_boss(scd_x + h[0], board_z + h[1]);
        for (h = mpl_holes) sensor_boss(mpl_x0 + h[0], board_z + h[1]);
        translate([mpl_x0, sw_y - so_h - 2.1, 0]) cube([mpl_w, so_h + 2.1, board_z]);   // mensola sotto la MPL
    }
}
/* [CCS811 / CJMCU-811 (solo versione da interno)] */
// Scheda 20 x 15 x 2 mm tenuta IN PIEDI in una fessura, sul fondo del vano
// sensori, davanti alle asole della parete in basso e LONTANO dalla piastra:
// il CCS811 scalda e falserebbe SCD41 e BME688. Non ha fori di fissaggio,
// quindi la si infila di taglio tra due guide a C; i pin restano in alto.
ccs      = false;
ccs_size = [20, 15, 2];   // larghezza, altezza, spessore della scheda
ccs_xy   = [9, 5];        // X del bordo sinistro, Y della faccia davanti
ccs_clr  = 0.4;           // gioco della fessura (0,2 per lato)
ccs_lift = 2;             // bordo inferiore sollevato dal fondo: aria sotto
ccs_grip = 2.5;           // quanto le guide coprono i bordi laterali
ccs_rw   = 2;             // spessore delle guide
// guida a C con l'apertura verso +X; origine sull'angolo della fessura
module ccs_rail_c() {
    sw = ccs_size[2] + 2*ccs_clr;
    rh = ccs_lift + ccs_size[1]*0.55;
    difference() {
        translate([-ccs_rw, -1.6, 0]) cube([ccs_rw + ccs_grip, sw + 3.2, rh]);
        translate([0, 0, ccs_lift]) cube([ccs_grip + 1, sw, rh]);
    }
}
module ccs_holder() {
    w = ccs_size[0]; sw = ccs_size[2] + 2*ccs_clr;
    x0 = ccs_xy[0] - ccs_clr; y0 = ccs_xy[1] - ccs_clr;
    translate([x0 - ccs_rw, y0 - 1.6, 0]) cube([w + 2*ccs_clr + 2*ccs_rw, sw + 3.2, ccs_lift]);
    translate([x0, y0, 0]) ccs_rail_c();
    translate([x0 + w + 2*ccs_clr, y0, 0]) mirror([1, 0, 0]) ccs_rail_c();
}
module ccs_dummy() {
    translate([ccs_xy[0], ccs_xy[1], ccs_lift]) cube([ccs_size[0], ccs_size[2], ccs_size[1]]);
}

// sensore di CO ZE07-CO (25,4 x 22,4 x 18,3) coricato sul fondo del vano, davanti alla piastra:
// quattro piedini (sotto ci sono i pin) e quattro battute a L agli angoli, lati liberi per i fili
ze07 = false;
ze07_xy = [50, 1];          // angolo in basso a sinistra dell'ingombro
ze07_size = [25.4, 22.4, 18.3];
ze07_pad = 3;               // altezza dei piedini
module ze07_pocket() {
    c = 0.5; lg = 5; t = 1.2; h = ze07_pad + 4;
    x0 = ze07_xy[0]; y0 = ze07_xy[1]; x1 = x0 + ze07_size[0]; y1 = y0 + ze07_size[1];
    for (x = [x0, x1 - 5], y = [y0, y1 - 5]) translate([x, y, 0]) cube([5, 5, ze07_pad]);
    for (cx = [[x0 - c - t, 1], [x1 + c, -1]], cy = [[y0 - c - t, 1], [y1 + c, -1]]) {
        translate([cx[0], cy[1] > 0 ? y0 - c - t : y1 + c - lg + t, 0]) cube([t, lg, h]);
        translate([cx[1] > 0 ? x0 - c - t : x1 + c - lg + t, cy[0], 0]) cube([lg, t, h]);
    }
}
// guide della piastra dei sensori: alle due estremita', una costola davanti e una dietro,
// alte quanto i sensori; la piastra scorre in mezzo dall'alto e appoggia sul fondo
module plate_guides() {
    gh = 12;
    for (xr = [[0, guide_l], [ch_r - guide_l, ch_r]]) {
        translate([xr[0], sw_y - 0.8 - plate_clr, 0]) cube([xr[1] - xr[0], 0.8, gh]);            // davanti (nel vano)
        translate([xr[0], sw_y + ch_wall + plate_clr, 0]) cube([xr[1] - xr[0], 0.8, gh]);        // dietro
    }
}
// PIASTRA DEI SENSORI (pezzo a parte, coordinate della scatola): prende il posto della parete
// tra il vano e il resto della scatola. SCD41 e MPL3115A2 si avvitano sui suoi distanziali,
// poi si infila dall'alto tra le guide. Dopo il cablaggio, un cordone di silicone neutro sui
// bordi (in alto e ai lati) richiude il vano.
module sensor_plate_part() {
    y0 = sw_y + plate_clr;  t = ch_wall - 2*plate_clr;
    difference() {
        union() {
            intersection() {
                translate([0.2, y0, 0]) cube([ch_r - 0.2 - plate_clr, t, inner_h]);
                below_rim(-seal_gap);
            }
            for (b = plate_boards, h = b[1]) plate_boss(b[0] + h[0], board_z + h[1], y0);
        }
        // tacca in alto a sinistra: ci passa il labbro del coperchio
        intersection() {
            translate([-1, y0 - 1, 0]) cube([2.2 + 1, t + 2, 40]);
            above_rim(-lid_lip_h - 0.3);
        }
        for (b = plate_boards, h = b[1]) plate_boss_hole(b[0] + h[0], board_z + h[1], y0);
        // tacche in cima per i fili
        for (x = plate_notches) translate([x - 3, y0 - 1, rim(y0) - seal_gap - 5]) cube([6, t + 2, 10]);
        // in fondo a destra, se la piastra arriva alla parete della scatola: ci passa il labbro
        if (ch_r >= inner_l - 0.1) intersection() {
            translate([inner_l - 2.2, y0 - 1, 0]) cube([3.2, t + 2, 40]);
            above_rim(-lid_lip_h - 0.3);
        }
    }
}
module plate_boss(x, z, y0) {
    translate([x, y0 + 0.01, z]) rotate([90, 0, 0]) cylinder(d = so_d, h = so_h + 0.01);
}
module plate_boss_hole(x, z, y0) {
    translate([x, y0 - so_h - 0.01, z]) rotate([-90, 0, 0]) cylinder(d = so_pilot_d, h = so_pilot_depth);
}
// la piastra stesa sul piatto, distanziali verso l'alto
module sensor_plate_print() {
    rotate([-90, 0, 0]) translate([0, -(sw_y + ch_wall - plate_clr), 0]) sensor_plate_part();
}
// tacche dove le pareti del vano incontrano quelle esterne: ci passa il labbro del coperchio
module chamber_notches() {
    intersection() {
        union() {
            translate([-1, sw_y - 1, 0]) cube([2.2 + 1, ch_wall + 2, 40]);
            if (ch_r < inner_l - 0.1) translate([ch_r - 1, -1, 0]) cube([ch_wall + 2, 2.2 + 1, 40]);
        }
        above_rim(-lid_lip_h - 0.3);
    }
}
module sensor_boss(x, z) {
    translate([x, sw_y, z]) rotate([90, 0, 0]) cylinder(d = so_d, h = so_h);
}
module sensor_boss_holes() {
    for (h = scd_holes) boss_hole(scd_x + h[0], board_z + h[1]);
    for (h = mpl_holes) boss_hole(mpl_x0 + h[0], board_z + h[1]);
}
module boss_hole(x, z) {
    translate([x, sw_y - so_h - 0.01, z]) rotate([-90, 0, 0]) cylinder(d = so_pilot_d, h = so_pilot_depth);
}

// striscia elettronica: due colonnine basse per la millefori (boost 5 V + AO3401/2N7002 + condensatore coricato)
strip_x0 = 8;
// millefori dei DIODI dei pannelli (uno Schottky per pannello, es. 1N5817 / SS14, + morsetto a 2 poli),
// nel corridoio cavi a destra del vano sensori
diode_board = [20, 12];
diode_xy = [56, 3];
diode_on = !indoor;   // sempre fuori (base uguale per A e B); da interno non ci sono pannelli
module diode_bosses() {
    for (x = [3, diode_board[0] - 3]) translate([diode_xy[0] + x, diode_xy[1] + diode_board[1]/2, 0]) cylinder(d = 4.5, h = 3);
}
// MODULO RS485 (il classico MAX485 con il morsetto verde, basetta 44 x 14) coricato nella striscia
// tra il vano sensori e la pila: quattro piedini e quattro battute a L agli angoli (come lo ZE07).
// Si attiva dal file nodo_lorawan_case_v4_rs485.scad, che allarga la scatola di 18 mm.
rs485 = false;
rs485_size = [44, 14];     // basetta (X, Y): misura la tua
rs485_x = 28;              // bordo sinistro; il morsetto verde va a destra, verso il pressacavo
rs485_pad = 7.5;           // altezza degli appoggi: sotto la basetta sporgono i pin (8,5), che entrano in due incavi del fondo
module rs485_pocket() {
    c = 0.5; lg = 5; t = 1.2; h = rs485_pad + 4;
    x0 = rs485_x; y0 = ch_depth + ch_wall + 2; x1 = x0 + rs485_size[0]; y1 = y0 + rs485_size[1];
    // due appoggi di traverso sotto la basetta, lontani dai pin delle due estremita'
    for (x = [x0 + 7, x1 - 9]) translate([x, y0, 0]) cube([2, rs485_size[1], rs485_pad]);
    for (ex = [[x0 - c - t, 1], [x1 + c, -1]], ey = [[y0 - c - t, 1], [y1 + c, -1]]) {
        translate([ex[0], ey[1] > 0 ? y0 - c - t : y1 + c - lg + t, 0]) cube([t, lg, h]);
        translate([ex[1] > 0 ? x0 - c - t : x1 + c - lg + t, ey[0], 0]) cube([lg, t, h]);
    }
}
// incavi ciechi nel fondo (profondi 1,4) sotto le due file di pin
module rs485_pin_cuts() {
    x0 = rs485_x; y0 = ch_depth + ch_wall + 2;
    for (x = [x0, x0 + rs485_size[0] - 5]) translate([x, y0 + 1, -1.4]) cube([5, rs485_size[1] - 2, 1.5]);
}
module strip_bosses() {
    for (x = [3, strip_board[0] - 3]) translate([strip_x0 + x, ch_depth + ch_wall + strip_h/2, 0]) cylinder(d = 4.5, h = 3);
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
// ---------------------------------------------------------------------
//  BACCELLO PER IL SENSORE UV (VEML6075, scheda 18,2 x 10,5 x 2) sulla parete in ALTO, fuori dalla scatola,
//  accanto alla Heltec: la scheda sta in piedi, col sensore verso l'alto (il cielo, a scatola appesa).
//  Strati, dalla parete in fuori: cavita' per le saldature (3), sede della scheda (2,3), coperchietto (uv_cap,
//  si stampa a parte) con apertura 16 x 9 e sede a filo per il vetrino trasparente agli UV 19 x 11,5 (max 1,2).
//  I 4 fili (3,3 V, GND, SCL, SDA) entrano nella scatola da un foro Ø5 nella parete, da sigillare con silicone.
// ---------------------------------------------------------------------
// uv_pod e uv_x stanno in cima al file, nella sezione [Sensore UV]: cosi' il
// Customizer li vede (si ferma al primo module).
uv_board = [18.2, 10.5, 2];   // scheda UV misurata col calibro (lunghezza, larghezza, spessore)
uv_clr = 0.3;  uv_wall = 1.6;  uv_under = 3;  uv_rail = 1.2;
uv_cap_t = 2.4;  uv_open = [16, 9];  uv_pane = [19, 11.5];  uv_pane_t = 1.2;
uv_hole_d = 5;
uv_pw = uv_board[0] + 2*uv_clr + 2*uv_wall;       // 22
uv_ph = uv_board[1] + 2*uv_clr + 2*uv_wall;       // 14,3
uv_z0 = -floor_t;                                 // il fondo del baccello e' a filo del fondo della scatola
uv_zc = uv_z0 + uv_ph/2;                          // centro in Z (4,75)
uv_d  = uv_under + uv_board[2] + uv_clr;          // quanto sporge dalla parete (5,3), poi il coperchietto (2,4)
uv_y0 = inner_w + wall;                           // faccia esterna della parete in alto
module uv_pod_body() {
    translate([uv_x - uv_pw/2, uv_y0 - 0.5, uv_z0]) cube([uv_pw, uv_d + 0.5, uv_ph]);
}
module uv_pod_cuts() {
    bw = uv_board[0] + 2*uv_clr;  bh = uv_board[1] + 2*uv_clr;
    translate([uv_x - bw/2, uv_y0 + uv_under, uv_zc - bh/2]) cube([bw, uv_d - uv_under + 1, bh]);               // sede della scheda
    translate([uv_x - bw/2, uv_y0, uv_zc - bh/2 + uv_rail]) cube([bw, uv_under + 0.01, bh - 2*uv_rail]);        // cavita' sotto (restano gli appoggi)
    translate([uv_x, inner_w - 1, uv_zc]) rotate([-90, 0, 0]) cylinder(d = uv_hole_d, h = wall + 2);            // fili verso la Heltec
}
module uv_cap() {         // coordinate locali: faccia interna a w = 0 (sul piatto), verso l'esterno w cresce
    difference() {
        translate([-uv_pw/2, -uv_ph/2, 0]) cube([uv_pw, uv_ph, uv_cap_t]);
        translate([-uv_open[0]/2, -uv_open[1]/2, -1]) cube([uv_open[0], uv_open[1], uv_cap_t + 2]);
        translate([-uv_pane[0]/2, -uv_pane[1]/2, uv_cap_t - uv_pane_t]) cube([uv_pane[0], uv_pane[1], uv_pane_t + 1]);
    }
}
module uv_cap_placed() { translate([uv_x, uv_y0 + uv_d, uv_zc]) rotate([-90, 0, 0]) uv_cap(); }
module uv_dummy() { color("Green") translate([uv_x - uv_board[0]/2, uv_y0 + uv_under, uv_zc - uv_board[1]/2]) cube(uv_board); }

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
            if (strip_h > 0 && !rs485) strip_bosses();
            if (rs485) rs485_pocket();
            if (diode_on) diode_bosses();
            sensor_chamber();
            if (ears) { ear(-1); if (ear_right) ear(1); }
            if (uv_pod) uv_pod_body();
        }
        chamber_notches();
        // fori pilota verticali nelle colonnine d'angolo
        for (p = col_pos) translate([p[0], p[1], rim(p[1]) - screw_pilot_depth])
            cylinder(d = screw_pilot_d, h = screw_pilot_depth + 5);
        if (!sensor_plate) sensor_boss_holes();
        // passacavi del vano (da sigillare)
        // (tutti e due nella parete destra del vano, verso il corridoio dei cavi)
        if (ch_wire_holes) {
            translate([ch_r - 1, 4, 5]) rotate([0, 90, 0]) cylinder(d = wire_d, h = ch_wall + 2);    // MPL3115A2
            translate([ch_r - 1, 4, 14]) rotate([0, 90, 0]) cylinder(d = wire_d, h = ch_wall + 2);   // SCD41
        }
        // finestra USB (parete sinistra)
        translate([0, usb_y, usb_z]) hull()
            for (dy = [-(usb_w - usb_h)/2, (usb_w - usb_h)/2])
                translate([-wall - 1, dy, 0]) rotate([0, 90, 0]) cylinder(d = usb_h, h = wall + 2);
        // UNICA feritoia del vano sensori (parete in basso)
        if (!indoor) translate([vent_x, -wall - 1, vent_z]) cube([vent_len, wall + 2, vent_h]);   // (da interno ci sono le asole verticali)
        // foro sigillato per il cavo DS18B20 (parete in basso)
        if (ds_hole) translate([ds_x, 1, ds_z]) rotate([90, 0, 0]) cylinder(d = ds_d, h = wall + 2);
        // pressacavo PG7 nella parete destra (si chiude col tappo gland_plug se non serve)
        if (gland) translate([inner_l - 1, gland_yz[0], gland_yz[1]]) rotate([0, 90, 0]) cylinder(d = gland_d, h = wall + 2);
        if (gland2) translate([-wall - 1, gland2_yz[0], gland2_yz[1]]) rotate([0, 90, 0]) cylinder(d = gland_d, h = wall + 2);
        if (soil_ds) translate([inner_l - 1, soil_ds_yz[0], soil_ds_yz[1]]) rotate([0, 90, 0]) cylinder(d = ds_d, h = wall + 2);
        if (indoor) indoor_vents_base();
        // versione SDS011: viti di unione e passacavo nella parete destra
        if (sds_side) {
            for (y = sds_bolt_ys) translate([inner_l - 1, y, sds_bolt_z]) rotate([0, 90, 0]) cylinder(d = screw_clear_d, h = wall + 2);
            translate([inner_l - 1, sds_cable[0], sds_cable[1]]) rotate([0, 90, 0]) cylinder(d = sds_cable_d, h = wall + 2);
        }
        // antenna SMA (parete in alto)
        if (sma_side == "top")
            translate([sma_top[0], inner_w - 1, sma_top[1]]) rotate([-90, 0, 0]) cylinder(d = sma_d, h = wall + 2);
        else
            translate([inner_l - 1, sma_right[0], sma_right[1]]) rotate([0, 90, 0]) cylinder(d = sma_d, h = wall + 2);
        if (rs485) rs485_pin_cuts();
        if (uv_pod) uv_pod_cuts();
        if (strip_h > 0 && !rs485) for (x = [3, strip_board[0] - 3]) translate([strip_x0 + x, ch_depth + ch_wall + strip_h/2, 3 - h_pilot_depth + 2]) cylinder(d = h_pilot_d, h = h_pilot_depth);
        if (diode_on) for (x = [3, diode_board[0] - 3]) translate([diode_xy[0] + x, diode_xy[1] + diode_board[1]/2, 0]) cylinder(d = h_pilot_d, h = h_pilot_depth);
        if (labels) {
            if (diode_on) floor_label(diode_xy[0] + diode_board[0]/2, diode_xy[1] + diode_board[1]/2, "DIODI", 2.5);
            if (strip_h > 0 && !rs485) floor_label(strip_x0 + strip_board[0]/2, ch_depth + ch_wall + strip_h/2, "BOOST  MOS  C", 3);
            if (rs485) floor_label(rs485_x + rs485_size[0]/2, ch_depth + ch_wall + 2 + rs485_size[1]/2, "RS485", 2.5);
            floor_label(h_x + 31.5, h_y + h_wid/2, "HELTEC V4", 3);
            floor_label(batt_x + stop_t + batt_l/2, batt_yc, "14500", 4);
            if (gps) floor_label(gps_x + gps_mod[0]/2, gps_y + 12, "GPS", 3);
            floor_label(bms_x + bms_l/2, bms_y + bms_t/2, "BMS", 1.6);
            floor_label(ch_r/2, 4.5, "SENSORI", 3);
            if (ds_hole) floor_label(ds_x, 6, "DS18B20", 2.2, 90);
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
            if (pv_count >= 2)   // piastra larga per tutta la lunghezza: i pannelli in piedi sporgono di lato
                multmatrix(LM) translate([ext_x0, ext_v0, 0]) rbox([ext_w, v_hi - ext_v0, lid_T], corner_r);
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
        // scarichi del labbro: colonnine d'angolo e tappo USB (parete sinistra)
        // (solo sotto la faccia interna del coperchio: tagliano il labbro, non la piastra)
        intersection() {
            union() {
                for (p = col_pos) translate([p[0], p[1], 0]) cylinder(d = col_d + 1, h = 40);
                translate([-1, usb_y - usb_w/2 - 1, 0]) cube([3, usb_w + 2, 40]);
                if (gland) translate([inner_l - 3.5, gland_yz[0] - 10, 0]) cube([4.5, 20, 40]);   // dado del pressacavo
                if (gland2) translate([-1, gland2_yz[0] - 10, 0]) cube([4.5, 20, 40]);
                if (ze07) translate([ze07_xy[0] - 1, -1, 0]) cube([ze07_size[0] + 2, 3, 40]);    // sopra lo ZE07 (alto fino a 21,3)
            }
            below_rim(0);
        }
        // viti M3 verticali, con sede piana per la testa (il coperchio e' inclinato)
        for (p = col_pos) {
            translate([p[0], p[1], 0]) cylinder(d = screw_clear_d, h = 40);
            translate([p[0], p[1], rim(p[1] + screw_head_d/2) + screw_seat_w/cos(box_tilt)])
                cylinder(d = screw_head_d, h = 20);   // pozzetto attraverso lo spessore aggiunto
        }
        // (niente fori per i pulsanti: sarebbero sotto il pannello)
        if (indoor) multmatrix(LM) indoor_vents_lid();
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
// per ogni pannello, i due bordi della sede: [x del bordo, verso l'esterno (-1 sinistra, +1 destra)]
function snap_sides(xc) = [[xc - pv_dx/2 - pv_side, -1], [xc + pv_dx/2 + pv_side, 1]];
module pv_snap_cuts(xc, vt = pv_vt) {
    v_free = vt + 0.3 + snap_ramp + 0.3;          // estremita' libera della lamella (in alto)
    v_root = v_free - snap_len;                   // radice (in basso, lungo il fianco del pannello)
    w_low = lid_t + pv_gap;                       // la lamella va da qui alla faccia esterna
    for (t = snap_sides(xc)) {
        xe = t[0];  sd = t[1];
        xb = xe + sd * (snap_th + snap_slit);     // fondo della fessura dietro la lamella
        // fessura dietro, per tutta la lunghezza
        translate([min(xe + sd*snap_th, xb), v_root, w_low - 0.6]) cube([snap_slit, snap_len + 0.6, lid_T]);
        // fessura in cima (stacca la lamella e il labbro sopra il pannello)
        translate([min(xb, xe - sd*2.6), v_free, w_low - 0.6]) cube([abs(xb - (xe - sd*2.6)), 0.6, lid_T]);
        // luce sotto la lamella: la stacca dal fondo
        translate([min(xe - sd*0.3, xb), v_root, w_low - 0.6]) cube([abs(xb - (xe - sd*0.3)), snap_len + 0.6, 0.6]);
        // il labbro sopra il fianco del pannello, lungo la lamella, si toglie (altrimenti la irrigidisce)
        translate([min(xe, xe - sd*2.6), v_root, lid_t + pv_gap + pv_edge + pv_groove - 0.1]) cube([2.6, snap_len + 0.6, lid_T]);
    }
}
module pv_teeth(xc, vt = pv_vt) {
    // dentino sulla faccia interna della lamella, sopra l'angolo alto del pannello: faccia di arresto
    // verso il pannello, rampa dal lato da cui il pannello entra (dall'alto)
    w_a = lid_t + pv_gap;  w_b = lid_t + pv_gap + pv_edge + pv_groove - 0.1;
    for (t = snap_sides(xc)) {
        xe = t[0];  sd = t[1];
        translate([0, 0, w_a]) linear_extrude(height = w_b - w_a)
            polygon([[xe + sd*0.01, vt + 0.3], [xe - sd*snap_hook, vt + 0.3], [xe - sd*snap_hook, vt + 0.6],
                     [xe + sd*0.01, vt + 0.3 + snap_ramp]]);
    }
}

// sede di un pannello (riferimento del coperchio): dal fondo verso l'esterno
//   intercapedine (piu' stretta: gradini laterali)  ->  bordo del pannello (larghezza piena)
//   ->  labbro che copre il bordo sottile, smussato verso fuori (la resina bombata resta libera)
// aperta in alto (il pannello entra scivolando), chiusa in basso (ci appoggia)
module pv_seat(xc, v0 = pv_v0, v_top = v_hi, v_bot = min(ext_v0, v_lo) - 1) {
    x0 = xc - pv_dx/2 - pv_side;  w = pv_dx + 2*pv_side;
    len = v_top - v0 + 10;
    w_lip = lid_t + pv_gap + pv_edge + pv_groove;         // sotto del labbro: gola profonda
    li = pv_side + pv_lip;                                // rientro del labbro sui lati
    lb = pv_clr + pv_lip;                                 // rientro del labbro sul fondo
    translate([x0 + pv_side + pv_ledge, v0 + pv_ledge, lid_t]) cube([w - 2*(pv_side + pv_ledge), len, pv_gap + 0.01]);
    translate([x0, v0, lid_t + pv_gap]) cube([w, len, pv_edge + pv_groove + 0.01]);   // gola
    hull() {   // labbro smussato a 45 gradi verso fuori: la resina bombata resta libera
        translate([x0 + li, v0 + lb, w_lip - 0.01]) cube([w - 2*li, len, 0.01]);
        translate([x0 + li - pv_lip_t, v0 + lb - pv_lip_t, lid_T - 0.01]) cube([w - 2*(li - pv_lip_t), len, 1]);
    }
    // due scoli dal fondo dell'intercapedine fino al bordo basso del coperchio (l'acqua esce)
    for (f = [0.25, 0.75]) translate([x0 + w*f - 2, v_bot, lid_t]) cube([4, v0 + 0.5 - v_bot, pv_gap]);
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
    for (bd = plate_boards) color(bd[1] == mpl_holes ? "Red" : "Purple", 0.6)
        translate([bd[0], sw_y - so_h - 1.6, board_z]) cube([bd[1] == mpl_holes ? mpl_w : scd_w, 1.6, bd[1] == mpl_holes ? mpl_h : scd_h]);
    if (ze07) color("SteelBlue", 0.6) translate([ze07_xy[0], ze07_xy[1], ze07_pad]) cube(ze07_size);
    if (ccs) color("ForestGreen", 0.6) ccs_dummy();
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

// tappo a scatto per il foro del pressacavo, se il sensore esterno non c'e':
// aletta fuori, corpo cavo tagliato a croce che flette entrando, dentino dietro la parete
module gland_plug() {
    d = gland_d;
    difference() {
        union() {
            cylinder(d = d + 4, h = 1.2);                                // aletta
            cylinder(d = d - 0.2, h = 1.2 + wall + 1.0);                 // corpo nella parete
            translate([0, 0, 1.2 + wall + 0.15]) cylinder(d1 = d + 0.6, d2 = d - 0.2, h = 0.6);   // dentino
            translate([d/2 + 1, -2.5, 0]) cube([5, 5, 1.2]);              // linguetta per toglierlo
        }
        translate([0, 0, 1.2]) cylinder(d = d - 1.8, h = 10);            // cavo dentro
        for (a = [0, 90]) rotate([0, 0, a]) translate([-d, -0.4, 2.6]) cube([2*d, 0.8, 10]);   // taglio a croce
    }
}

// ---------------------------------------------------------------------
//  Versione da INTERNO: aerazione (con la scatola appesa, l'aria entra in basso ed esce in alto)
// ---------------------------------------------------------------------
vent_w = 3;        // larghezza delle asole
vent_step = 5;     // passo (restano 2 mm di plastica tra un'asola e l'altra)
module indoor_vents_base() {
    // vano sensori: asole verticali in tutta la parete in basso e due sul fianco sinistro
    for (x = [8 : vent_step : ch_r - vent_w - 2]) translate([x, -wall - 1, 2]) cube([vent_w, wall + 2, rim(0) - lid_lip_h - 3]);
    for (y = [7, 10.5]) translate([-wall - 1, y, 3]) cube([wall + 2, 2.5, rim(14) - lid_lip_h - 4]);
    // zona schede: ingressi in basso (corridoio sotto il GPS e fianco destro) ...
    for (x = [ch_r + ch_wall + 3 : vent_step : inner_l - 8]) translate([x, -wall - 1, 2]) cube([vent_w, wall + 2, rim(0) - lid_lip_h - 3]);
    for (y = [stack_y + 1.5 : 4 : stack_y + 15]) translate([inner_l - 1, y, 3]) cube([wall + 2, 2.5, rim(y + 2.5) - lid_lip_h - 4]);
    // ... uscite in alto (parete in alto, lontano dall'antenna e dalle colonnine)
    for (x = [8 : vent_step : sma_top[0] - 8]) translate([x, inner_w - 1, 2]) cube([vent_w, wall + 2, rim(inner_w) - lid_lip_h - 2.5]);
}
module indoor_vents_lid() {
    // sopra il vano sensori (riferimento del coperchio: x, v, w)
    for (x = [6 : 4 : ch_r - 6]) translate([x, 2.5, -1]) cube([2, (sw_y - 2) / cos(box_tilt) - 2.5, lid_T + 2]);
    // sopra le schede: due file di asole, la seconda in alto dove esce l'aria calda
    // (con aerazione "ridotta" non ci sono: il camino resta fianco destro -> parete in alto)
    if (aerazione == "massima") for (x = [8 : 4 : inner_l - 10], vr = [[stack_y + 4, stack_y + 19], [stack_y + 22, inner_w - 5]])
        translate([x, vr[0] / cos(box_tilt), -1]) cube([2, (vr[1] - vr[0]) / cos(box_tilt), lid_T + 2]);
}

module lid_print() {
    translate([0, 0, lid_T]) mirror([0, 0, 1]) multmatrix(LMinv) translate([0, 0, -inner_h]) lid();
}

if (v4_render) {
if (part == "base") {
    base();
} else if (part == "lid") {
    lid_print();
    translate([inner_l + 25, 20, 0]) usb_plug();      // il tappo si stampa sullo stesso piatto
    if (gland) translate([inner_l + 25, 40, 0]) gland_plug();
    if (gland2) translate([inner_l + 25, 62, 0]) gland_plug();
    if (uv_pod) translate([inner_l + 36, 86, 0]) uv_cap();           // coperchietto del sensore UV: faccia interna sul piatto
} else if (part == "plate") {
    if (sensor_plate) sensor_plate_print();
} else if (part == "print") {
    base();
    translate([0, inner_w + 2*wall + 14 - ext_v0, 0]) lid_print();
    translate([inner_l + 25, 20, 0]) usb_plug();
    if (uv_pod) translate([inner_l + 36, 86, 0]) uv_cap();
    if (sensor_plate) translate([inner_l + 20, -40, 0]) sensor_plate_print();
} else {
    base();
    if (sensor_plate) color("Gold") sensor_plate_part();
    %dummies();
    translate([0, 0, explode]) { lid(); color("MidnightBlue") panels(); }
    if (uv_pod) { color("SteelBlue") translate([0, explode, 0]) uv_cap_placed(); %uv_dummy(); }
}
}
