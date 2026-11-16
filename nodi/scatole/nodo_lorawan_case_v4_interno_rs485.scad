// =====================================================================
//  Versione 4 DA INTERNO CON MODULO RS485 (per leggere / comandare
//  caldaie, condizionatori, contatori con uscita RS485 / Modbus)
//  E' la versione da interno squadrata con una striscia in piu' di 22 mm
//  tra la piastra dei sensori e la pila: li' sta coricato il modulo MAX485
//  col morsetto verde (basetta 44 x 14), pin in giu' negli incavi del
//  fondo, morsetto a destra con le bocche verso la pila (canale di 4 mm
//  per i fili A/B). Il cavo entra da un foro Ø7 nella parete destra,
//  subito sopra la piastra (in casa non serve il pressacavo: un gommino).
//  Interno 82 x 100,5 (era 82 x 78,5), sempre profonda 22,5 e piatta.
//  Scatola e coperchio NON sono intercambiabili con la versione da interno
//  normale; la piastra dei sensori e' la stessa.
// =====================================================================

include <nodo_lorawan_case_v4_interno.scad>
assert(!is_undef(rs485_size) && !is_undef(indoor),
       "MANCANO nodo_lorawan_case_v4_interno.scad e nodo_lorawan_case_v4.scad aggiornati: mettili nella STESSA cartella di questo file");

part = "assembly"; // [assembly, base, lid, plate, print]
aerazione = "massima";  // [massima, ridotta]
rs485   = true;
strip_h = 22;      // 18 per il modulo + 4 di canale per i fili davanti al morsetto
inner_w = 100.5;   // 78,5 + 22
// foro per il cavo RS485 (riusa il foro "DS18B20 del terreno", allargato a Ø7)
soil_ds = true;
soil_ds_yz = [37.5, 12];   // parete destra: tra la piastra dei sensori e l'aletta
ds_d    = 7;
