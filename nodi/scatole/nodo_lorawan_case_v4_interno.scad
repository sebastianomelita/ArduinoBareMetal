// =====================================================================
//  Versione 4 DA INTERNO, con sensori di qualita' dell'aria e di CO
//  Stessi pezzi della v4 corta (Heltec, pila con le lamelle, BMS, piastra
//  dei sensori estraibile), ma:
//    - coperchio PIATTO senza pannelli, grande esattamente come la scatola
//      (da interno l'inclinazione non serve: profonda 22,5 ovunque);
//    - niente GPS (in casa prende poco; la posizione si scrive una volta
//      nel server LoRaWAN);
//    - vano sensori largo quanto la scatola e profondo 32, molto aerato
//      (asole sulla parete in basso, sul fianco sinistro e nel coperchio):
//        piastra estraibile con tre schedine 25,4 x 17,8 (formato STEMMA QT,
//        fori a 2,54 dagli angoli): SCD41, SGP41, BME688
//        la MPL3115A2 ha il suo foro nello stesso posto del BME688, in
//        alternativa: monti l'uno o l'altro (misurano entrambi la pressione);
//        davanti, sul fondo, l'alloggio del sensore di CO ZE07-CO;
//    - zona schede aerata a camino: l'aria entra in basso dal fianco
//      destro ed esce in alto, sopra il vano sensori il caldo non passa.
//  Niente fori per DS18B20 esterni e pressacavo: rimettili a true se servono.
// =====================================================================

include <nodo_lorawan_case_v4.scad>

coperchio = "C";
indoor    = true;
aerazione = "massima";  // [massima, ridotta] "ridotta" = senza le asole del coperchio sopra le schede
ds_hole   = false;
soil_ds   = false;
gland     = false;
gps       = false;
box_tilt  = 0;         // coperchio piatto
ch_depth  = 32;        // vano sensori profondo: ZE07-CO davanti, schedine sulla piastra dietro
inner_w   = 78.5;      // 61 + 17,5
ch_r      = 82;        // il vano arriva alla parete destra della scatola
ch_wire_holes = false; // i fili escono dalle tacche in cima alla piastra
plate_notches = [27.2, 54.2];
guide_l   = 1.2;       // costole corte: i distanziali delle schedine stanno vicino ai bordi
// SCD41, SGP41, BME688 e, sullo STESSO posto del BME688, il foro alternativo della MPL3115A2
// (centrata nella sagoma del BME688: monti l'una o l'altra)
plate_boards = [[1, scd_holes], [28, scd_holes], [55, scd_holes], [55 + (25.4 - mpl_w)/2, mpl_holes]];
ccs       = true;      // CCS811 (CJMCU-811) in piedi nella fessura sul fondo, a sinistra: false se non lo monti
ze07      = true;
ze07_xy   = [50, 1];
