// =====================================================================
//  Versione 4 CORTA da esterno CON MODULO RS485
//  E' la v4 corta con una striscia in piu' di 22 mm tra il vano sensori e
//  la pila: li' sta coricato il classico modulo MAX485 con il morsetto
//  verde (basetta 44 x 14), tra quattro battute, con i pin in giu' (entrano in due incavi del fondo),
//  il morsetto a destra con le bocche verso la pila: davanti resta un canale
//  di 4 mm per i fili A/B, che arrivano dal pressacavo PG7 li' accanto.
//  I fili verso la Heltec si saldano sui pin dal lato dei componenti
//  (sotto non c'e' posto per i connettori Dupont).
//  Pila, BMS, Heltec e GPS sono gli stessi, spostati 22 mm piu' in alto.
//  La scatola diventa 82 x 83 dentro (era 82 x 61) e profonda 25,6 in basso
//  (era 22,5), per tenere la pendenza di 8 gradi: scatola e coperchio
//  NON sono intercambiabili con quelli della corta normale.
//  La piastra dei sensori e' la stessa, ma 3,1 mm piu' alta: ristampala.
// =====================================================================

include <nodo_lorawan_case_v4.scad>
assert(!is_undef(rs485_size), "MANCA nodo_lorawan_case_v4.scad aggiornato: mettilo nella STESSA cartella di questo file");

part = "assembly"; // [assembly, base, lid, plate, print]
coperchio = "A";   // [A, B]  A = 1 pannello coricato, B = 2 pannelli in piedi
rs485   = true;
baccello_uv = "no";  // [no, si] "si" = baccello del sensore UV sulla parete in alto
uv_x    = 41;      // centro del baccello lungo X
uv_pod  = (baccello_uv == "si");   // non toccare: la comanda la voce qui sopra
gland2  = true;    // secondo pressacavo PG7 sulla parete sinistra, per il sensore capacitivo del terreno
strip_h = 22;      // striscia: 18 per il modulo + 4 di canale per i fili davanti al morsetto
inner_w = 83;      // 61 + 22
inner_h = 25.6;     // in alto restano 13,9 come nella corta (la Heltec ne vuole 12,9)
