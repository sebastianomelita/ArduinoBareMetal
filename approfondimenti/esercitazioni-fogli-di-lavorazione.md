# Esercitazioni – Comporre un foglio di lavorazione per scatole

*Documento collegato alla [dispensa del contenitore LoRaWAN](dispensa-contenitore-lorawan.md) e al [Lessico tecnico](lessico-tecnico-contenitori.md) · 6 novembre 2026*

Sei esercizi di difficoltà crescente. Ognuno aggiunge **una** idea nuova a quelle precedenti, e l'ultimo chiede di trovare gli errori in un foglio scritto male. Prova a scrivere il foglio da solo prima di aprire la soluzione: si apre cliccando su **▸ Soluzione**.

## Indice

| N° | Esercizio | Idea nuova | Livello |
| --- | --- | --- | --- |
| 1 | [Scatola semplice](#esercizio-1--scatola-semplice) | origine, guscio, svuotamento | ●○○○○○ |
| 2 | [Coperchio con labbro e viti](#esercizio-2--coperchio-con-labbro-e-viti) | gioco, colonnine, ordine aggiungi→togli | ●●○○○○ |
| 3 | [Scheda e presa USB](#esercizio-3--scheda-e-presa-usb) | quote dei fori, catena delle altezze, verifica di interferenza | ●●●○○○ |
| 4 | [Aperture e installazione](#esercizio-4--aperture-e-installazione) | orientamento, serie lineare, ingombro dei dadi | ●●●●○○ |
| 5 | [Due vani, batteria e alette](#esercizio-5--due-vani-batteria-e-alette) | progetto completo | ●●●●●○ |
| 6 | [Trova gli errori](#esercizio-6--trova-gli-errori) | revisione critica di un foglio | ●●●●●● |

---

## Il metodo in sette passi

Vale per tutti gli esercizi, e per qualunque scatola.

1. **Requisiti in frasi semplici.** Cosa contiene, dove sta, cosa deve restare accessibile, che ambiente (interno, esterno, calore).
2. **Componenti e misure.** Per ogni pezzo: ingombro, fori, connettori, e da dove si misura.
3. **Orientamento da installata.** Quale lato guarda in basso, quale in alto, quale verso il muro.
4. **Origine e assi.** Un punto solo, da cui partono tutte le quote (qui: angolo interno del fondo).
5. **Elenco delle lavorazioni**, dal generale al particolare: involucro → interni → aperture → coperchio → fissaggi → finiture.
6. **Quote per coordinate**, mai in catena.
7. **Verifiche**: interferenze tra pezzi, giochi, spazi per dita, cavi e dadi.

**Modello vuoto del foglio**

| N° | Lavorazione | Corpo | Quote (mm) | Perché |
| --- | --- | --- | --- | --- |
| 10 | | | | |
| 20 | | | | |

**Convenzioni usate in tutti gli esercizi**: misure in mm; origine (0; 0; 0) nell'angolo interno sinistro-anteriore del fondo; X lungo il lato lungo, Y lungo il lato corto, Z verso l'alto; pareti, fondo e coperchio da 2 mm salvo diversa indicazione.

---

## Esercizio 1 – Scatola semplice

**Livello ●○○○○○ · Idea nuova:** misure interne contro esterne; guscio pieno + svuotamento.

**Richiesta.** "Mi serve una scatolina per tenere dei componenti. Dentro deve misurare 60 × 40 × 25 mm. Pareti e fondo da 2 mm, spigoli esterni arrotondati con raggio 3. Il coperchio è una piastra piatta da 2 mm appoggiata sopra."

**Da fare:** scrivi il foglio di lavorazione per base e coperchio.

<details>
<summary><b>▸ Soluzione</b></summary>

**Ragionamento.** Le misure date sono **interne**: le pareti si aggiungono fuori. Con l'origine nell'angolo interno, le pareti stanno a coordinate negative (da −2 a 0) e oltre 60 e 40. Fuori la base misura 60 + 2×2 = **64** × 40 + 2×2 = **44** × 25 + 2 = **27** mm.

| N° | Lavorazione | Corpo | Quote (mm) | Perché |
| --- | --- | --- | --- | --- |
| 10 | Guscio pieno | Base | rettangolo (−2; −2)–(62; 42), spigoli R3, estruso da Z −2 a 25 | è l'ingombro esterno |
| 20 | Svuotamento | Base | tasca (0; 0)–(60; 40), spigoli R1, da Z 0 verso l'alto, passante | raggio interno = 3 − 2: la parete resta spessa 2 anche negli angoli |
| 30 | Piastra | Coperchio | stesso profilo del guscio, (−2; −2)–(62; 42) R3, da Z 25 a 27 | il coperchio deriva dalla base |

**Errori tipici**
- Usare 60 × 40 come misura esterna: la cavità verrebbe 56 × 36.
- Raggio interno uguale all'esterno (R3 anche dentro): negli angoli la parete si assottiglia.
- Coperchio senza niente che lo centri: scivola. È il problema che risolve l'esercizio 2.

</details>

> **Box – Come si scelgono le quote**
>
> **Misure esterne.** Si parte sempre dall'interno e si aggiunge la parete: **esterno = interno + 2 × spessore**. Con l'origine nell'angolo interno, la cavità va da 0 a 60 e da 0 a 40; la parete da 2 la circonda, quindi il rettangolo esterno va da −2 a 62 e da −2 a 42. Un rettangolo si descrive con i due **angoli opposti**: dicono insieme posizione e misure.
>
> **Altezze.** Z = 0 sta sulla faccia interna del fondo, dove appoggiano i componenti: ogni altezza interna si legge direttamente, e il fondo resta sotto, da −2 a 0. L'altezza interna si ricava dalla catena delle altezze: **distanziali + scheda + componente più alto + margine per i cavi (3–5 mm) + labbro del coperchio**.
>
> **Raggi.** **Raggio interno = raggio esterno − spessore**: così la parete resta spessa uguale anche negli angoli. Il raggio esterno va scelto **almeno pari allo spessore** (con R = s l'angolo interno resta vivo), di solito **2–5 mm** per scatole di queste dimensioni, e non troppo grande, perché "mangia" l'angolo interno, dove spesso vanno le colonnine.
>
> **Origine.** Si mette dove partono la maggior parte delle quote:
>
> | Origine | Vantaggio | Quando |
> | --- | --- | --- |
> | Angolo interno del fondo | posizioni dei componenti lette direttamente | contenitori per elettronica (il caso tipico) |
> | Angolo esterno | tutte le coordinate positive | pezzi visti "da fuori", lavorazioni meccaniche |
> | Centro della scatola | simmetrie immediate con il comando *specchio* | scatole simmetriche, con fori o alette uguali |

---

## Esercizio 2 – Coperchio con labbro e viti

**Livello ●●○○○○ · Idea nuova:** gioco tra parti accoppiate; colonnine; prima si aggiunge, poi si toglie.

**Richiesta.** "La stessa scatola dell'esercizio 1, ma il coperchio deve centrarsi da solo e restare chiuso con 4 viti M3 autofilettanti negli angoli."

**Dati di progetto:** colonnine Ø 7 tangenti alle pareti; foro pilota Ø 2,6 profondo 10; labbro spesso 1,2 e alto 3, gioco 0,3 per lato; foro passante nel coperchio Ø 3,4; viti a testa bombata.

<details>
<summary><b>▸ Soluzione</b></summary>

**Ragionamento.**
- Una colonnina Ø 7 tangente alle due pareti ha il centro a 3,5 mm da ciascuna: (3,5; 3,5), (56,5; 3,5), (3,5; 36,5), (56,5; 36,5).
- Il foro pilota si fa **dopo** la colonnina: se lo fai prima, la colonnina lo riempie.
- Il labbro sta 0,3 mm dentro le pareti su ogni lato: profilo esterno da (0,3; 0,3) a (59,7; 39,7), raggio 1 − 0,3 = 0,7.
- Il labbro passerebbe proprio sopra le colonnine: va interrotto con uno **scarico** attorno a ciascuna, Ø 7 + 2 × 0,5 = Ø 8.
- Con un coperchio da 2 mm una svasatura lascerebbe troppo poco materiale: meglio viti a testa bombata e foro dritto.

| N° | Lavorazione | Corpo | Quote (mm) | Perché |
| --- | --- | --- | --- | --- |
| 10 | Guscio pieno | Base | come esercizio 1 | ingombro esterno |
| 20 | Svuotamento | Base | come esercizio 1 | cavità |
| 30 | Colonnine angolari | Base | Ø 7, da Z 0 a 25, centri (3,5; 3,5) (56,5; 3,5) (3,5; 36,5) (56,5; 36,5) | sostegno delle viti, a filo con il bordo |
| 40 | Fori pilota | Base | Ø 2,6, profondi 10 dall'alto (Z 15…25), nei 4 centri | la vite autofilettante si crea il filetto |
| 50 | Piastra | Coperchio | (−2; −2)–(62; 42) R3, da Z 25 a 27 | chiusura |
| 60 | Labbro | Coperchio | anello da Z 22 a 25, esterno (0,3; 0,3)–(59,7; 39,7) R0,7, spesso 1,2 | centra il coperchio; 0,3 di gioco per lato |
| 70 | Scarichi del labbro | Coperchio | cilindri Ø 8 ai 4 centri delle colonnine, da Z 22 a 25 | il labbro non urta le colonnine |
| 80 | Fori viti | Coperchio | Ø 3,4 passanti ai 4 centri | la vite passa libera |

**Errori tipici**
- Labbro con profilo (0; 0)–(60; 40): gioco zero, il coperchio non entra.
- Dimenticare gli scarichi: il labbro appoggia sulle colonnine e il coperchio resta sollevato di 3 mm.
- Foro pilota largo come la vite (Ø 3): la vite gira a vuoto.

</details>

---

## Esercizio 3 – Scheda e presa USB

**Livello ●●●○○○ · Idea nuova:** posizionare i fori di una scheda; calcolare la quota Z di un connettore sommando le altezze; controllare le interferenze.

**Richiesta.** "Ci devo mettere dentro una scheda e collegarla con un cavo USB-C dal lato corto sinistro."

**Dati della scheda:** 50 × 30 × 1,6; 4 fori M2,5 a 2,5 mm dai bordi; presa USB-C al centro del lato corto, con il centro 1,6 mm sopra la faccia superiore della scheda.
**Dati della scatola:** interno 70 × 46 × 25, coperchio e colonnine angolari come nell'esercizio 2; scheda su distanziali Ø 5 alti 5, foro pilota Ø 2,1; finestra USB ad asola 12 × 7.

<details>
<summary><b>▸ Soluzione</b></summary>

**Ragionamento.**
1. **Dove mettere la scheda.** In Y la centro: (46 − 30) / 2 = 8, quindi Y da 8 a 38. In X la avvicino alla parete della USB, ma deve evitare la colonnina d'angolo: la metto da X 4 a 54.
2. **Fori.** Ognuno a 2,5 dai bordi della scheda: X 4 + 2,5 = **6,5** e 54 − 2,5 = **51,5**; Y 8 + 2,5 = **10,5** e 38 − 2,5 = **35,5**.
3. **Altezza della presa (catena delle altezze).** Distanziale 5 + scheda 1,6 + centro presa 1,6 = **Z 8,2**. In Y il centro è a metà scheda: 8 + 15 = **23**.
4. **Verifica di interferenza** con la colonnina d'angolo (centro (3,5; 3,5), raggio 3,5):
    - spigolo della scheda (4; 8): distanza dal centro √(0,5² + 4,5²) ≈ 4,5 > 3,5 → **libero, con 1 mm di margine**;
    - distanziale in (6,5; 10,5), raggio 2,5: distanza √(3² + 7²) ≈ 7,6 > 3,5 + 2,5 = 6 → **libero**.
5. **Nota pratica.** Tra la scheda e la parete restano 4 mm: lo spinotto deve entrare un po' più a fondo. Con un cavo dalla testa grossa, avvicina la scheda.

| N° | Lavorazione | Corpo | Quote (mm) | Perché |
| --- | --- | --- | --- | --- |
| 10 | Guscio pieno | Base | (−2; −2)–(72; 48) R3, da Z −2 a 25 | ingombro esterno |
| 20 | Svuotamento | Base | (0; 0)–(70; 46) R1, da Z 0 in su | cavità |
| 30 | Colonnine angolari + fori pilota | Base | Ø 7, Z 0…25, centri (3,5; 3,5) (66,5; 3,5) (3,5; 42,5) (66,5; 42,5); foro Ø 2,6 prof. 10 | viti del coperchio |
| 40 | Distanziali | Base | Ø 5, Z 0…5, centri (6,5; 10,5) (51,5; 10,5) (6,5; 35,5) (51,5; 35,5); foro Ø 2,1 prof. 4 | appoggio e fissaggio della scheda |
| 50 | Finestra USB | Base | asola passante 12 × 7 sulla parete sinistra (X −2…0), centro Y 23, Z 8,2 | larga per lo spinotto, non solo per la presa |
| 60 | Coperchio | Coperchio | come esercizio 2, con labbro (0,3; 0,3)–(69,7; 45,7) e fori ai nuovi centri | chiusura |

**Errori tipici**
- Mettere la presa a Z 1,6, dimenticando distanziale e scheda.
- Quotare i fori "a 45 mm l'uno dall'altro" (in catena) invece che per coordinate.
- Non controllare le colonnine d'angolo: è l'interferenza più frequente nelle scatole piccole.

</details>

---

## Esercizio 4 – Aperture e installazione

**Livello ●●●●○○ · Idea nuova:** decidere le aperture in base a come la scatola sarà montata; serie lineari; ingombro dei dadi all'interno.

**Richiesta.** "La scatola dell'esercizio 3, alta 30 dentro invece di 25, va appesa al muro con il lato lungo orizzontale. C'è un sensore d'aria, quindi serve ventilazione, e un cavo per una sonda esterna."

**Dati:** feritoie ad asola alte 2, passo 4; pressacavo PG7 (foro Ø 12,5, dado esagonale da 19 mm, ingombro circa Ø 22); labbro del coperchio alto 3, quindi da Z 27 a 30.

<details>
<summary><b>▸ Soluzione</b></summary>

**Ragionamento.**
1. **Orientamento.** Appesa con il lato lungo orizzontale, la parete frontale (Y = 0) guarda in basso e la posteriore (Y = 46) in alto. Regola: aperture in basso o di lato, **mai in alto**, dove cade la pioggia.
2. **Altezza utile delle feritoie.** Sopra Z 27 c'è il labbro, sotto Z 4 conviene lasciare il fondo pieno: 5 asole con il bordo inferiore a Z 6, 10, 14, 18, 22 (l'ultima arriva a 24).
3. **Larghezza delle feritoie sulla parete frontale.** Devono evitare la colonnina d'angolo (fino a X 7) e il dado del pressacavo: X da 10 a 38.
4. **Pressacavo.** Sulla parete frontale, in X 50, Z 12. Il dado (Ø 22) occupa X 39…61: non tocca le feritoie (finiscono a 38) né la colonnina in (66,5; 3,5), che parte da X 63. In Z occupa 1…23: non tocca il fondo.
5. **Il dado contro la scheda.** Il dado sporge verso l'interno di circa 6 mm (Y 0…6); la scheda parte da Y 8 → libero.
6. **Feritoie laterali.** Sulla parete destra (X 70…72), tra le colonnine d'angolo: Y 12…34.

| N° | Lavorazione | Corpo | Quote (mm) | Perché |
| --- | --- | --- | --- | --- |
| 10–50 | Come esercizio 3 | Base | stesse quote, ma guscio da Z −2 a 30 e colonnine fino a Z 30 | base di partenza |
| 60 | Feritoie frontali | Base | serie lineare di 5 asole passanti sulla parete frontale (Y −2…0), X 10…38, alte 2, bordo inferiore a Z 6, 10, 14, 18, 22 | aria dal basso |
| 70 | Feritoie laterali | Base | stessa serie sulla parete destra (X 70…72), Y 12…34 | aria dal lato |
| 80 | Pressacavo PG7 | Base | foro passante Ø 12,5 sulla parete frontale, centro X 50, Z 12 | cavo verso il basso: l'acqua non risale il cavo |
| 90 | Coperchio | Coperchio | piastra Z 30…32, labbro Z 27…30, come esercizio 3 | chiusura |
| — | Parete posteriore | Base | **nessuna apertura** | installata guarda in alto |

**Errori tipici**
- Feritoie che entrano nella zona del labbro (sopra Z 27): il coperchio le copre o il labbro si indebolisce.
- Pressacavo posizionato guardando solo il foro e non il dado: il dado urta la colonnina o copre le feritoie.
- Aperture sulla parete che, una volta montata, guarda in alto.

</details>

---

## Esercizio 5 – Due vani, batteria e alette

**Livello ●●●●●○ · Idea nuova:** mettere insieme tutto in un progetto completo.

**Richiesta.** "Un nodo a batteria da appendere al muro, con il lato lungo orizzontale. Dentro: la scheda dell'esercizio 3, una batteria 18650 e un sensore d'aria che non deve sentire il calore della scheda. Un cavo per una sonda esce in basso. Si programma a scatola aperta, quindi niente USB."

**Dati:**
- interno 120 × 66 × 30; paratia spessa 2 in X 80…82 (vano A a sinistra, vano B a destra);
- batteria 18650: Ø 18,4 × 65, in culla con due selle larghe 8 e gioco 0,6 sul diametro;
- sensore 20 × 20 con 4 fori M2 a interasse 15 × 15;
- coperchio come esercizio 2 (labbro alto 3), più una nervatura che prosegue la paratia;
- alette a muro sui lati corti: spesse 4, larghe 20, sporgenza 12, foro Ø 4,5 a 6 dalla parete.

<details>
<summary><b>▸ Soluzione</b></summary>

**Ragionamento, in ordine.**
1. **Disposizione in pianta.** Vano A: batteria lungo X vicino alla parete frontale, scheda dietro. Vano B: sensore al centro, lontano dalla paratia.
2. **Batteria.** Asse lungo X, da X 8 a 73. Asse a Y 18,2 (lascia spazio al dado del pressacavo davanti) e a Z = 18,4 / 2 + 1 = 10,2 (1 mm sopra il fondo). Selle a Y 7…29,4, alte 11,2, scavo Ø 19 (18,4 + 0,6). Fermi alle estremità.
3. **Scheda.** X 8…58, Y 33…63: fori in (10,5; 35,5) (55,5; 35,5) (10,5; 60,5) (55,5; 60,5). Verifica con la colonnina in (3,5; 62,5): spigolo scheda (8; 63) a 4,5 > 3,5 → libero.
4. **Paratia.** Si ferma sotto il labbro: 30 − 3 − 0,3 = Z 26,7. Passaggio cavi in basso, Y 28…38, alto 6.
5. **Sensore nel vano B.** X 91…111, Y 23…43: fori in (93,5; 25,5) (108,5; 25,5) (93,5; 40,5) (108,5; 40,5).
6. **Aperture.** Feritoie solo nel vano B (destra + frontale); pressacavo sulla frontale del vano A in X 40. Il dado (Ø 22, X 29…51, sporge fino a Y 6) non tocca la batteria, che parte da Y 9, né le selle, in X 15…23 e 58…66.
7. **Alette.** Centrate in Y 33, a filo con il fondo esterno (Z −2…2).

| N° | Lavorazione | Corpo | Quote (mm) | Perché |
| --- | --- | --- | --- | --- |
| 10 | Guscio pieno | Base | (−2; −2)–(122; 68) R3, Z −2…30 | ingombro esterno |
| 20 | Svuotamento | Base | (0; 0)–(120; 66) R1, Z 0 in su | cavità |
| 30 | Colonnine angolari + fori pilota | Base | Ø 7, Z 0…30, centri (3,5; 3,5) (116,5; 3,5) (3,5; 62,5) (116,5; 62,5); foro Ø 2,6 prof. 10 | viti del coperchio |
| 40 | Paratia | Base | X 80…82, Y 0…66, Z 0…26,7; passaggio cavi Y 28…38, alto 6 | separa calore e sensore |
| 50 | Culla batteria | Base | selle X 15…23 e 58…66, Y 7…29,4, alte 11,2, scavo Ø 19 con asse in Y 18,2, Z 10,2; fermi X 6…8 e 73…75, Y 9…27,4, alti 11 | tiene la 18650 senza viti |
| 60 | Distanziali scheda | Base | Ø 5, Z 0…5, centri (10,5; 35,5) (55,5; 35,5) (10,5; 60,5) (55,5; 60,5); foro Ø 2,1 prof. 4 | fissaggio scheda |
| 70 | Distanziali sensore | Base | Ø 5, Z 0…5, centri (93,5; 25,5) (108,5; 25,5) (93,5; 40,5) (108,5; 40,5); foro Ø 1,7 prof. 4 | fissaggio sensore (M2) |
| 80 | Feritoie | Base | asole alte 2, bordo inferiore Z 6, 10, 14, 18, 22; parete destra Y 15…51; parete frontale X 88…110 | aria solo al sensore, dal lato e dal basso |
| 90 | Pressacavo PG7 | Base | foro Ø 12,5 sulla parete frontale, X 40, Z 12 | cavo verso il basso |
| 100 | Alette | Base | 2 alette spesse 4 (Z −2…2), centrate in Y 33, oltre X −2 e oltre X 122; larghe 20, sporgenza 12, foro Ø 4,5 a 6 dalla parete | fissaggio a muro |
| 110 | Piastra | Coperchio | (−2; −2)–(122; 68) R3, Z 30…32 | chiusura |
| 120 | Labbro + scarichi | Coperchio | anello Z 27…30, esterno (0,3; 0,3)–(119,7; 65,7) R0,7, spesso 1,2; scarichi Ø 8 ai centri delle colonnine | centraggio |
| 130 | Nervatura | Coperchio | X 80…82, Y 0,3…65,7, Z 27…30 | prosegue la paratia fino al coperchio |
| 140 | Fori viti | Coperchio | Ø 3,4 passanti ai 4 centri | chiusura con viti |
| 150 | Etichette | Base | incise 0,6 sul fondo: "A", "B", "18650", "SENSORE" | aiutano il montaggio; sempre per ultime |

**Verifiche finali**
- [ ] Dado del pressacavo libero da batteria e selle (punto 6).
- [ ] Scheda e distanziali liberi dalle colonnine d'angolo (punto 3).
- [ ] Paratia (fino a Z 26,7) e nervatura (da Z 27) allineate, con 0,3 di distacco.
- [ ] Nessuna apertura sulla parete posteriore.

</details>

---

## Esercizio 6 – Trova gli errori

**Livello ●●●●●● · Idea nuova:** rileggere criticamente un foglio scritto da altri (o da un assistente AI).

**Richiesta.** Questo foglio descrive la scatola dell'esercizio 2, installata a muro con la parete frontale in basso. Contiene **7 errori**: trovali e correggili.

| N° | Lavorazione | Corpo | Quote (mm) |
| --- | --- | --- | --- |
| 10 | Guscio pieno | Base | (−2; −2)–(62; 42) R3, Z −2…25 |
| 15 | Etichetta "ALTO" incisa | Base | sul fondo interno, al centro |
| 20 | Svuotamento | Base | (0; 0)–(60; 40) R3, Z 0 in su |
| 30 | Fori pilota | Base | Ø 2,6, prof. 10, ai centri delle colonnine |
| 40 | Colonnine | Base | Ø 7, la prima in (3,5; 3,5), la seconda 53 mm più a destra, la terza 33 mm più indietro della seconda, la quarta in fondo a sinistra |
| 50 | Foro per il cavo | Base | sul lato |
| 60 | Feritoie | Base | 3 asole 20 × 2 sulla parete posteriore |
| 70 | Labbro | Coperchio | anello Z 22…25, esterno (0; 0)–(60; 40), spesso 1,2 |
| 80 | Fori viti | Coperchio | Ø 3,4 ai centri delle colonnine |

<details>
<summary><b>▸ Soluzione</b></summary>

| N° | Errore | Perché è sbagliato | Correzione |
| --- | --- | --- | --- |
| 15 | Etichetta fatta all'inizio | le finiture vanno per ultime; qui lo svuotamento (20) la cancella | spostarla in fondo, dopo tutti i tagli (es. 90) |
| 20 | Svuotamento con R3 | raggio interno uguale all'esterno: parete più sottile negli angoli | R1 (= 3 − 2) |
| 30 | Fori pilota prima delle colonnine | si toglie materiale che non c'è ancora; la colonnina (40) richiude i fori | prima le colonnine, poi i fori |
| 40 | Colonnine quotate in catena | ogni errore si somma al successivo; "in fondo a sinistra" non è una quota | coordinate: (3,5; 3,5) (56,5; 3,5) (3,5; 36,5) (56,5; 36,5) |
| 50 | "Foro per il cavo sul lato" | mancano dove (quale parete, quale punto), quanto (Ø) e perché | es.: foro passante Ø 12,5 sulla parete frontale, centro X 30, Z 12, per pressacavo PG7 (il dado, Ø 22, resta lontano dalle colonnine) |
| 60 | Feritoie sulla parete posteriore | installata, guarda in alto: entra la pioggia | spostarle sulla parete frontale o laterale, con posizione e passo |
| 70 | Labbro senza gioco e senza scarichi | (0; 0)–(60; 40) ha gioco zero; senza scarichi il labbro urta le colonnine | esterno (0,3; 0,3)–(59,7; 39,7) R0,7 + scarichi Ø 8 ai centri delle colonnine |

**Come si trovano errori così.** Rileggi il foglio con quattro domande, una alla volta:
1. **Ordine** – c'è qualcosa che viene tolto prima di essere aggiunto, o una finitura fatta troppo presto?
2. **Completezza** – ogni riga dice cosa, a cosa, dove, quanto e perché?
3. **Accoppiamenti** – dove due parti si toccano c'è un gioco? Ci sono interferenze?
4. **Uso reale** – una volta installata, che lato guarda in alto? Dove va l'acqua, dove scalda?

</details>

---

*Tutti i valori sono pensati per stampa FDM con ugello da 0,4 mm; per giochi e fori di partenza vedi la tabella "Valori di partenza per la stampa FDM" nel [Lessico tecnico](lessico-tecnico-contenitori.md).*
