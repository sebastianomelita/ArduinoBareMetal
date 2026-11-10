# Dispense di informatica: array, ordinamenti ed elenchi

Indice di tutte le dispense. Ogni titolo apre la dispensa; le voci rientrate sotto di esso portano direttamente alle sue sezioni.

## Array e shift

- **[Gli shift negli array](dispensa_shift.md)**  
    *Spostare gli elementi di un array di una o più posizioni, con valore di default o in modo circolare; il verso giusto da cui partire.*

    1. [Che cos'è uno shift](dispensa_shift.md#s1)
    2. [Shift a destra con valore di default](dispensa_shift.md#s2)
    3. [Shift a sinistra con valore di default](dispensa_shift.md#s3)
    4. [Shift di più posizioni](dispensa_shift.md#s4)
    5. [Shift circolari](dispensa_shift.md#s5)
    6. [Applicazione: l'inserimento ordinato](dispensa_shift.md#s6)
    7. [Errori tipici](dispensa_shift.md#s7)

## Algoritmi di ordinamento

- **[Insertion sort](dispensa_insertion_sort.md)**  
    *Ogni elemento viene inserito al suo posto nella parte già ordinata, con il buco che scende.*

    1. [L'idea, dall'animazione](dispensa_insertion_sort.md#s1)
    2. [Passo per passo](dispensa_insertion_sort.md#s2)
    3. [Il loop esterno: l'indice separatore](dispensa_insertion_sort.md#s3)
    4. [Il loop interno: l'inserimento ordinato](dispensa_insertion_sort.md#s4)
    5. [Il programma completo e le varianti](dispensa_insertion_sort.md#s5)

- **[Bubble sort](dispensa_bubble_sort.md)**  
    *Confronti e scambi tra vicini: a ogni passata il massimo sale in cima. Separatore e bandierina.*

    1. [L'idea](dispensa_bubble_sort.md#s1)
    2. [Passo per passo](dispensa_bubble_sort.md#s2)
    3. [Il loop esterno: `i` conta le passate](dispensa_bubble_sort.md#s3)
    4. [Il loop interno: confronto tra vicini e scambio](dispensa_bubble_sort.md#s4)
    5. [Il programma completo e le varianti](dispensa_bubble_sort.md#s5)
    6. [Ottimizzazione con la bandierina: fermarsi quando una passata non scambia](dispensa_bubble_sort.md#s6)
    7. [Bubble sort e insertion sort a confronto](dispensa_bubble_sort.md#s7)
    8. [Errori tipici](dispensa_bubble_sort.md#s8)

- **[Selection sort](dispensa_selection_sort.md)**  
    *A ogni passata si cerca il minimo e lo si porta al suo posto. Confronto tra i tre algoritmi.*

    1. [L'idea](dispensa_selection_sort.md#s1)
    2. [Passo per passo](dispensa_selection_sort.md#s2)
    3. [Il loop esterno: `i` è il separatore](dispensa_selection_sort.md#s3)
    4. [Il loop interno: la ricerca del minimo](dispensa_selection_sort.md#s4)
    5. [Il programma completo e la variante con il massimo in alto](dispensa_selection_sort.md#s5)
    6. [Perché non ci si può fermare prima](dispensa_selection_sort.md#s6)
    7. [Il selection sort non è stabile](dispensa_selection_sort.md#s7)
    8. [I tre algoritmi a confronto](dispensa_selection_sort.md#s8)
    9. [Errori tipici](dispensa_selection_sort.md#s9)

## Elenchi

- **[La ricerca in un elenco](dispensa_ricerca.md)**  
    *Ricerca esaustiva, sequenziale su elenco ordinato e binaria; prima e ultima occorrenza.*

    1. [Mappa dei casi](dispensa_ricerca.md#s1)
    2. [La ricerca esaustiva (elenco non ordinato)](dispensa_ricerca.md#s2)
    3. [La ricerca sequenziale su un elenco ordinato](dispensa_ricerca.md#s3)
    4. [La ricerca binaria](dispensa_ricerca.md#s4)
    5. [Prima e ultima occorrenza](dispensa_ricerca.md#s5)
    6. [Riepilogo](dispensa_ricerca.md#s6)
    7. [Errori tipici](dispensa_ricerca.md#s7)

- **[L'inserimento in un elenco ordinato](dispensa_inserimento.md)**  
    *Punto di inserimento (lower e upper bound), inserimento ordinato, inserimento in coda e riordino.*

    1. [Mappa: come si inserisce un valore](dispensa_inserimento.md#s1)
    2. [Trovare il punto di inserimento](dispensa_inserimento.md#s2)
    3. [L'inserimento ordinato](dispensa_inserimento.md#s3)
    4. [Inserimento in coda e riordino con un sort](dispensa_inserimento.md#s4)
    5. [Ordinato o non ordinato?](dispensa_inserimento.md#s5)
    6. [Riepilogo](dispensa_inserimento.md#s6)
    7. [Errori tipici](dispensa_inserimento.md#s7)

- **[Cancellazione e modifica in un elenco ordinato](dispensa_cancellazione_modifica.md)**  
    *Cancellazione di una o di tutte le occorrenze, con lo shift o con un valore massimo; modifica con il buco che si sposta.*

    1. [Mappa](dispensa_cancellazione_modifica.md#s1)
    2. [La cancellazione ordinata](dispensa_cancellazione_modifica.md#s2)
    3. [Cancellare tutte le occorrenze](dispensa_cancellazione_modifica.md#s3)
    4. [Cancellazione con un valore massimo e riordino](dispensa_cancellazione_modifica.md#s4)
    5. [La modifica mediante shift](dispensa_cancellazione_modifica.md#s5)
    6. [E in un elenco non ordinato?](dispensa_cancellazione_modifica.md#s6)
    7. [Riepilogo](dispensa_cancellazione_modifica.md#s7)
    8. [Errori tipici](dispensa_cancellazione_modifica.md#s8)

- **[Creare nuovi elenchi da elenchi esistenti](dispensa_nuovi_elenchi.md)**  
    *Filtraggio, suddivisione, operazioni sugli insiemi con la ricerca o con due indici, fusione.*

    1. [Mappa](dispensa_nuovi_elenchi.md#s1)
    2. [Il filtraggio](dispensa_nuovi_elenchi.md#s2)
    3. [La suddivisione](dispensa_nuovi_elenchi.md#s3)
    4. [La ricerca di un elemento in un altro elenco](dispensa_nuovi_elenchi.md#s4)
    5. [La fusione di due elenchi ordinati](dispensa_nuovi_elenchi.md#s5)
    6. [Unione, intersezione e differenza di elenchi ordinati](dispensa_nuovi_elenchi.md#s6)
    7. [Riepilogo](dispensa_nuovi_elenchi.md#s7)
    8. [Errori tipici](dispensa_nuovi_elenchi.md#s8)
