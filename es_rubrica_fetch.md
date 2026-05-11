# Progetto: Rubrica Telefonica Web

**Materia:** Informatica / Tecnologie Web
**Tempo previsto:** 6–10 ore di lavoro
**Modalità:** Individuale o a coppie

---

## Obiettivi didattici

Al termine del progetto lo studente sarà in grado di:

- Comprendere l'architettura **client–server** di un'applicazione web.
- Realizzare una pagina HTML che comunica con un server tramite **API REST**.
- Utilizzare la funzione **`fetch`** con **`async/await`** per inviare e ricevere dati in formato **JSON**.
- Implementare le quattro operazioni fondamentali **CRUD** (Create, Read, Update, Delete) su un database.
- Gestire la **manipolazione del DOM** per aggiornare dinamicamente la pagina.

---

## Descrizione

È fornito un server Node.js già funzionante che espone delle API per gestire una rubrica di contatti su database MongoDB. Lo studente deve realizzare un **client HTML** (una singola pagina web con HTML + CSS + JavaScript) che permetta all'utente di gestire la rubrica.

L'applicazione deve consentire di:

1. **Visualizzare** l'elenco dei contatti già salvati.
2. **Inserire** un nuovo contatto.
3. **Modificare** un contatto esistente.
4. **Cancellare** un contatto.
5. **Cercare** un contatto per nome, cognome, telefono o email.

Ogni contatto è composto da: `nome`, `cognome`, `telefono` (obbligatori) e `email` (opzionale).

---

## API a disposizione

Il server espone i seguenti endpoint sotto il percorso base `/api/contatti`:

| Metodo | Percorso | Funzione | Body / Query |
|--------|----------|----------|--------------|
| GET    | `/api/contatti` | Elenca tutti i contatti | — |
| GET    | `/api/contatti/cerca?q=testo` | Cerca contatti | query string `q` |
| GET    | `/api/contatti/:id` | Legge un singolo contatto | — |
| POST   | `/api/contatti` | Inserisce un nuovo contatto | JSON `{nome, cognome, telefono, email}` |
| PUT    | `/api/contatti/:id` | Modifica un contatto | JSON con i campi da aggiornare |
| DELETE | `/api/contatti/:id` | Cancella un contatto | — |

Tutte le risposte sono in formato JSON. In caso di successo si riceve `{ok: true, ...}`; in caso di errore `{error: "messaggio"}` con codice HTTP appropriato (400, 404, 500).

---

## Requisiti tecnici

### Obbligatori

- **Un solo file HTML** che contiene HTML, CSS e JavaScript (no librerie esterne come jQuery, React, ecc.).
- Tutte le chiamate al server devono usare **`fetch` con `async/await`** (non callback né `.then()` a catena lunga).
- **Gestione degli errori** con `try/catch` su ogni chiamata.
- **Validazione lato client** prima di inviare i dati: i campi obbligatori non devono essere vuoti.
- **Aggiornamento automatico** dell'elenco dopo ogni inserimento, modifica o cancellazione.
- **Conferma** prima di cancellare un contatto (es. con `confirm()`).
- Il codice JavaScript deve essere **commentato** nei punti significativi.

### Facoltativi (per chi vuole un voto più alto)

- **Ricerca live**: la lista si aggiorna mentre l'utente digita (con un piccolo ritardo / debounce).
- **Ordinamento** delle colonne cliccando sull'intestazione della tabella.
- **Validazione dell'email** con espressione regolare.
- **Protezione XSS**: escape dei caratteri HTML nei dati visualizzati.
- **Layout responsive** (utilizzabile anche da smartphone).
- **Esportazione** della rubrica in formato CSV.

---

## Consegna

Lo studente dovrà consegnare:

1. Il file **`rubrica.html`** funzionante.
2. Una breve **relazione** (max 2 pagine) che spieghi:
   - Come è strutturato il codice (parti principali del JS).
   - Quali endpoint vengono chiamati e in che momento.
   - Eventuali difficoltà incontrate e come sono state risolte.
3. Almeno **2 screenshot**: uno con la lista popolata, uno con il form di modifica attivo.

**Modalità di consegna:** caricamento su classroom entro la data stabilita.

---

## Criteri di valutazione

| Criterio | Punti |
|----------|-------|
| Inserimento contatti funzionante | 2 |
| Visualizzazione elenco funzionante | 2 |
| Modifica contatti funzionante | 2 |
| Cancellazione contatti funzionante | 1 |
| Ricerca funzionante | 1 |
| Uso corretto di `async/await` e `try/catch` | 1 |
| Qualità del codice e commenti | 0,5 |
| Cura grafica del client | 0,5 |
| **Totale** | **10** |

I requisiti facoltativi possono aggiungere fino a **+1 punto** (lode).

---

## Suggerimenti per iniziare

1. Parti scrivendo l'HTML statico con il form e una tabella vuota.
2. Implementa per prima la funzione `caricaContatti()` che chiama `GET /api/contatti` e mostra i dati.
3. Aggiungi l'inserimento con `POST`.
4. Aggiungi modifica e cancellazione.
5. Per ultima, implementa la ricerca.

Esempio di chiamata `fetch` con `async/await`:

```javascript
async function caricaContatti() {
    try {
        const res = await fetch('/api/contatti');
        if (!res.ok) throw new Error('Errore HTTP ' + res.status);
        const dati = await res.json();
        console.log(dati);
    } catch (err) {
        console.error('Errore:', err.message);
    }
}
```

---

## Note importanti

- Le API restituiscono il campo `_id` (con underscore) come identificatore univoco di ogni contatto, non `id`. Usalo per le operazioni di modifica e cancellazione.
- Per testare il server localmente, dopo `npm install` esegui `node server.js`.
- Tutti i metodi `fetch` con corpo JSON devono includere l'header `Content-Type: application/json`.

Buon lavoro!
