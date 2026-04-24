# Form, fetch e sicurezza lato client

Dispensa pratica sui concetti di base per inviare dati da un form HTML a un database, con particolare attenzione ai problemi di sicurezza che emergono quando si lavora solo lato client.

---

## Indice

1. [Il form HTML con FormData](#1-il-form-html-con-formdata)
2. [Invio dei dati con fetch in POST](#2-invio-dei-dati-con-fetch-in-post)
3. [Esempio minimo funzionante (senza autenticazione)](#3-esempio-minimo-funzionante-senza-autenticazione)
4. [Il problema dei segreti nel codice client](#4-il-problema-dei-segreti-nel-codice-client)
5. [La soluzione: il backend come intermediario](#5-la-soluzione-il-backend-come-intermediario)
6. [Autenticazione degli utenti: OAuth e i suoi flow](#6-autenticazione-degli-utenti-oauth-e-i-suoi-flow)
7. [Il quadro completo](#7-il-quadro-completo)
8. [Glossario](#8-glossario)

---

## 1. Il form HTML con FormData

### FormData in breve

`FormData` è un oggetto JavaScript nativo che raccoglie automaticamente tutti i campi di un form che hanno un attributo `name`. Evita di dover leggere uno per uno i valori con `document.getElementById` o simili.

```javascript
const form = document.getElementById("formAnagrafica");

form.addEventListener("submit", (event) => {
  event.preventDefault();

  // Raccoglie tutti i campi del form in un colpo solo
  const formData = new FormData(form);

  // Conversione a oggetto JS (utile per JSON)
  const dati = Object.fromEntries(formData.entries());

  console.log(dati);
  // { nome: "Mario", cognome: "Rossi", email: "mario@example.com", ... }
});
```

### Perché `event.preventDefault()`

Senza questa riga, il browser esegue il submit "classico": ricarica la pagina e invia i dati come richiesta tradizionale. Con `preventDefault()` blocchiamo quel comportamento e gestiamo noi l'invio via JavaScript.

### Due formati possibili

`FormData` può essere inviato in due modi:

- **Così com'è** (`multipart/form-data`): utile se il form contiene file upload.
- **Convertito in JSON**: il formato standard per le API moderne, più naturale per MongoDB che è un database document-oriented.

Nella maggior parte dei casi conviene il JSON.

---

## 2. Invio dei dati con fetch in POST

### Struttura della chiamata

```javascript
const response = await fetch(API_URL, {
  method: "POST",
  headers: {
    "Content-Type": "application/json"
  },
  body: JSON.stringify(dati)
});

if (!response.ok) {
  throw new Error("Errore HTTP " + response.status);
}

const risultato = await response.json();
```

### Elementi chiave

- **`method: "POST"`**: stiamo creando una risorsa nuova sul server.
- **`Content-Type: application/json`**: dichiariamo al server che il body è JSON.
- **`body: JSON.stringify(dati)`**: il payload va serializzato, `fetch` non lo fa da solo.
- **`response.ok`**: è `true` solo se lo status HTTP è 2xx. Un 404 o 500 non lancia un'eccezione automatica, bisogna controllarlo.

### Gestione errori

Conviene sempre avvolgere la chiamata in try/catch e dare feedback all'utente. Durante l'invio, disabilitare il bottone di submit previene doppi invii accidentali.

```javascript
try {
  bottone.disabled = true;
  const response = await fetch(API_URL, { /* ... */ });
  if (!response.ok) throw new Error("HTTP " + response.status);
  // successo
} catch (err) {
  console.error(err);
  // mostra messaggio di errore
} finally {
  bottone.disabled = false;
}
```

---

## 3. Esempio minimo funzionante (senza autenticazione)

Prima di introdurre qualunque complicazione, vediamo un esempio completo e funzionante: un form di anagrafica che raccoglie i dati e li invia via POST a un endpoint. Questo è il punto di partenza didattico: funziona, fa quello che deve fare, è leggibile. Nelle sezioni successive vedremo *perché* in un contesto reale non basta, e come va evoluto.

### Il codice

```html
<!DOCTYPE html>
<html lang="it">
<head>
  <meta charset="UTF-8">
  <title>Anagrafica</title>
</head>
<body>

  <h1>Anagrafica</h1>

  <form id="formAnagrafica">
    <label>Nome <input type="text" name="nome" required></label>
    <label>Cognome <input type="text" name="cognome" required></label>
    <label>Email <input type="email" name="email" required></label>
    <label>Data di nascita <input type="date" name="dataNascita"></label>
    <button type="submit">Salva</button>
  </form>

  <div id="esito"></div>

  <script>
    const API_URL = "https://tuo-servizio.example.com/api/anagrafica";

    const form  = document.getElementById("formAnagrafica");
    const esito = document.getElementById("esito");

    form.addEventListener("submit", async (event) => {
      event.preventDefault();

      // 1. Raccolta dati dal form
      const formData = new FormData(form);
      const dati = Object.fromEntries(formData.entries());

      // 2. Invio via POST
      try {
        const response = await fetch(API_URL, {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify(dati)
        });

        if (!response.ok) throw new Error("HTTP " + response.status);

        const risultato = await response.json();
        esito.textContent = "Salvato! ID: " + risultato.id;
        form.reset();

      } catch (err) {
        esito.textContent = "Errore: " + err.message;
      }
    });
  </script>

</body>
</html>
```

### Cosa fa, passo per passo

1. L'utente compila i campi e preme "Salva"
2. `event.preventDefault()` blocca il submit classico (che ricaricherebbe la pagina)
3. `FormData` legge tutti i campi con `name` dal form
4. `Object.fromEntries` li trasforma in un oggetto JS
5. `fetch` invia l'oggetto come JSON al server, in POST
6. La risposta viene letta e mostrata all'utente

### Cosa manca in questo esempio

Questa versione funziona, ma è volutamente **priva di controlli di sicurezza**:

- l'endpoint `API_URL` è chiamato direttamente, senza alcuna autenticazione
- chiunque conosca l'URL può mandare dati
- non c'è verifica di chi sia l'utente
- non c'è protezione contro invii massivi automatizzati

In un contesto reale, un endpoint di scrittura così esposto verrebbe abusato nel giro di poche ore. Nelle prossime sezioni vediamo perché, e come si affronta il problema.

---

## 4. Il problema dei segreti nel codice client

### Tutto ciò che è nel browser è pubblico

Qualunque cosa scritta nel JavaScript che gira nel browser è **visibile a chiunque**. Basta:

- premere F12 (DevTools)
- aprire il tab Sources o Network
- leggere il codice

Questo include:
- token di autenticazione
- chiavi API
- stringhe di connessione al database
- endpoint "nascosti"

### Cosa può fare un malintenzionato

Se un token o una chiave API sono nel codice frontend:

- li copia e chiama la tua API fingendosi la tua app
- scrive dati spazzatura nel database
- satura il tuo piano del servizio cloud
- se il token ha permessi ampi, legge o cancella dati

Vale per qualunque credenziale: Bearer token, API key di MongoDB Atlas Data API, chiavi Firebase, webhook secret, etc.

### Cosa NON funziona (anche se sembra)

| Tecnica | Perché non basta |
|---|---|
| Offuscare/minificare il JS | Rallenta un curioso, non ferma un attaccante |
| Nascondere in variabili "private" | Nel browser non esiste privacy: il debugger vede tutto |
| Controllare `Origin` o `Referer` | Il browser li imposta, ma `curl` o Postman scrivono quello che vogliono |
| CORS da solo | Protegge altri *siti web*, non strumenti server-to-server |

### Principio generale

> Il frontend è territorio ostile. Tutto ciò che deve restare segreto sta dietro un backend.

---

## 5. La soluzione: il backend come intermediario

### L'analogia della banca

Immagina MongoDB come il **caveau di una banca**. Per aprirlo serve una chiave (le credenziali).

- **Modo sbagliato**: dare una copia della chiave a ogni cliente. Prima o poi qualcuno ne approfitta.
- **Modo giusto**: c'è un **cassiere** (lo sportello). Il cliente gli consegna i soldi, il cassiere controlla e poi usa lui la chiave per aprire il caveau. La chiave non esce mai dalla banca.

Il cassiere è il **backend**. Il caveau è MongoDB. Il cliente è il browser.

### Il flusso corretto

```
[Browser]  ──POST /api/anagrafica──▶  [Backend]  ──driver mongo──▶  [MongoDB]
                                      (le credenziali
                                       stanno qui)
```

Il browser non sa nemmeno che esiste MongoDB. Parla solo col backend.

### Cos'è "il backend"

Un programma che **tu** scrivi e che gira su un server (non nel browser). Può essere scritto in:

- Node.js con Express, Fastify, Hono
- Python con Flask, FastAPI, Django
- PHP, Java, Go, Rust, C#…

Il suo compito:

1. Resta in ascolto su un URL (es. `https://tuosito.com/api/anagrafica`)
2. Riceve la richiesta POST dal browser
3. Valida i dati
4. Si connette lui a MongoDB con le credenziali che stanno **sul server**
5. Scrive i dati
6. Risponde al browser "ok" o "errore"

### Esempio minimo in Node.js/Express

```javascript
// server.js — GIRA SUL SERVER, NON NEL BROWSER
const express = require("express");
const { MongoClient } = require("mongodb");

const app = express();
app.use(express.json());

// Credenziali in variabile d'ambiente: non nel codice, non su Git
const MONGO_URI = process.env.MONGO_URI;
const client = new MongoClient(MONGO_URI);

app.post("/api/anagrafica", async (req, res) => {
  try {
    // Validazione minima
    const { nome, cognome, email } = req.body;
    if (!nome || !cognome || !email) {
      return res.status(400).json({ error: "Campi obbligatori mancanti" });
    }

    await client.connect();
    const collection = client.db("miodb").collection("anagrafica");
    const result = await collection.insertOne(req.body);

    res.json({ ok: true, id: result.insertedId });
  } catch (err) {
    console.error(err);
    res.status(500).json({ error: "Errore server" });
  }
});

app.listen(3000, () => console.log("Server su porta 3000"));
```

### Cose da fare comunque sul backend

Il backend non ti rende automaticamente sicuro. Vanno aggiunti:

- **Validazione input**: non fidarti mai di ciò che arriva dal client
- **Rate limiting**: limitare richieste per IP/utente
- **CORS**: accettare richieste solo dai domini che controlli
- **Variabili d'ambiente** per i segreti, mai hardcoded
- **Logging** degli errori e degli abusi
- **HTTPS** sempre in produzione

### "Ma io volevo fare solo frontend…"

Se i dati devono finire in un database, **serve sempre un lato server**. Non esiste un modo sicuro di far parlare direttamente il browser col database. Servizi come MongoDB Atlas Data API sembrano aggirare la cosa ma richiedono comunque una chiave che, se messa nel browser, ricrea lo stesso problema.

---

## 6. Autenticazione degli utenti: OAuth e i suoi flow

Finora abbiamo parlato di come proteggere le credenziali *della tua app*. Un discorso diverso è autenticare gli *utenti*, cioè capire chi sta usando la tua app.

### Il problema che OAuth risolve

Hai un'app e vuoi che solo utenti registrati possano inviare dati. Invece di implementare login/password da zero, deleghi l'autenticazione a un **provider di identità** (Google, GitHub, Auth0, Okta…). L'utente fa login lì, e il provider ti conferma "sì, è davvero Mario Rossi" consegnandoti un **access token** che rappresenta quella sessione.

OAuth 2.0 definisce vari **flow** (modi di ottenere il token) a seconda del tipo di app.

### Implicit Flow (deprecato)

Era il flow storico per le Single Page Application. Nato nel 2012.

**Come funziona:**

1. L'utente clicca "Login con Google"
2. Viene reindirizzato al provider
3. Si autentica
4. Il provider lo rimanda alla tua app con il token **direttamente nell'URL** (nel fragment `#`)
5. Il tuo JS legge il token dall'URL

**Perché è deprecato** (IETF, OAuth 2.0 Security Best Current Practice, 2020):

- Il token passa nell'URL, finisce nei log dei server, nella cronologia del browser, nell'header `Referer`
- Non c'è modo di verificare che chi richiede il token sia davvero il client legittimo
- I refresh token non sono gestibili in sicurezza
- Nessuna mitigazione contro XSS

### Authorization Code Flow con PKCE (raccomandato)

PKCE = Proof Key for Code Exchange. È il pattern standard oggi per SPA e app mobile.

**Come funziona:**

1. L'app genera un valore casuale (`code_verifier`) e il suo hash SHA-256 (`code_challenge`)
2. Manda l'utente al provider col code challenge
3. L'utente si autentica
4. Il provider rimanda alla tua app un **codice** (non il token)
5. La tua app scambia il codice + il `code_verifier` originale contro il token, via POST

**Il vantaggio:** anche se un attaccante intercetta il codice nell'URL, senza il `code_verifier` (che non è mai passato in chiaro) non può scambiarlo contro un token. E il token viaggia in una POST, non in un redirect.

Tutti i provider seri supportano PKCE. Le loro SDK lo implementano sotto il cofano, tu non devi gestire i dettagli a mano.

### Altri flow (per completezza)

- **Authorization Code Flow classico**: per app con backend tradizionale (web app server-side). Il backend custodisce un `client_secret`.
- **Client Credentials Flow**: per chiamate server-to-server, senza utente umano coinvolto.
- **Device Authorization Flow**: per dispositivi senza browser (smart TV, CLI).

### Attenzione: OAuth non sostituisce il backend

Questo è il punto cruciale che spesso confonde.

OAuth risolve: **"chi è l'utente che sta parlando con la mia app?"**

OAuth NON risolve: **"come faccio a scrivere su MongoDB dal browser in sicurezza?"**

Anche con un bel token utente ottenuto via PKCE, ti serve comunque:

1. Un backend che riceva la chiamata dal browser col token
2. Il backend verifica che il token è valido (firma, scadenza, issuer)
3. Il backend decide se quell'utente può fare quell'operazione
4. Il backend scrive su MongoDB con le **sue** credenziali

MongoDB non accetta access token OAuth come credenziali di connessione. Ha meccanismi di auth pensati per connessioni server-to-server.

---

## 7. Il quadro completo

Mettendo insieme tutti i pezzi, un'architettura sicura per un form che scrive su MongoDB con utenti autenticati è questa:

```
┌─────────────┐
│   Browser   │
│             │
│  1. Login   │──────────────┐
│             │              ▼
│             │       ┌──────────────┐
│             │       │  Provider    │
│             │       │  OAuth       │
│             │◀──────│  (Auth0,     │
│  2. token   │       │   Google...) │
│     utente  │       └──────────────┘
│             │
│  3. POST    │──────────────┐
│  /api/...   │              ▼
│  con token  │       ┌──────────────┐
│             │       │   Backend    │
│             │       │              │
│             │       │  - verifica  │
│             │       │    token     │
│             │       │  - valida    │
│             │       │    dati      │
│             │       │  - scrive    │
│             │       └──────┬───────┘
│             │              │
│             │              ▼
│             │       ┌──────────────┐
│  4. esito   │◀──────│   MongoDB    │
└─────────────┘       └──────────────┘
```

### Le responsabilità

| Componente | Cosa fa | Cosa custodisce |
|---|---|---|
| Browser | UI, raccolta dati, chiamate al backend | Token dell'utente (in memoria o cookie httpOnly) |
| Provider OAuth | Autentica gli utenti, emette token | Password, identità degli utenti |
| Backend | Valida, autorizza, scrive | Credenziali MongoDB, chiavi API, logica di business |
| MongoDB | Persiste i dati | I dati |

### Checklist minima per una messa in produzione

- [ ] Nessuna credenziale nel codice frontend
- [ ] Backend con validazione input
- [ ] Credenziali DB in variabili d'ambiente
- [ ] HTTPS ovunque
- [ ] CORS configurato per i soli domini legittimi
- [ ] Rate limiting sul backend
- [ ] Logging degli errori
- [ ] Se serve autenticazione utenti: Authorization Code Flow con PKCE
- [ ] Token utente con scadenza breve
- [ ] Gestione della scadenza del token (refresh)

---

## 8. Glossario

**Access token**: stringa che rappresenta un'autorizzazione ad accedere a una risorsa. Breve durata (tipicamente 1 ora).

**API key**: chiave statica che identifica un'applicazione verso un servizio. Se esposta, l'applicazione è compromessa.

**Backend**: codice che gira su un server, in un ambiente controllato, non accessibile direttamente dagli utenti.

**Bearer token**: token che dà accesso a chiunque lo possieda (come una banconota). Va trattato con cura: chi lo ha, lo usa.

**CORS** (Cross-Origin Resource Sharing): meccanismo del browser per controllare quali origini web possono chiamare una API. Protegge dagli abusi cross-site, non da chi usa strumenti esterni.

**FormData**: oggetto JavaScript nativo per raccogliere i campi di un form.

**Frontend**: codice che gira nel browser dell'utente. Pubblicamente visibile, non un posto sicuro per segreti.

**Implicit Flow**: vecchio flow OAuth per SPA, oggi deprecato a favore di Authorization Code + PKCE.

**JWT** (JSON Web Token): formato comune per gli access token. Contiene claim firmati (es. `sub`, `exp`, `iss`) che il backend può verificare senza chiamare il provider.

**PKCE** (Proof Key for Code Exchange): estensione di OAuth 2.0 che rende sicuro l'Authorization Code Flow anche per client che non possono custodire un segreto.

**Refresh token**: token a lunga durata usato per ottenere nuovi access token senza far rifare il login all'utente. Va custodito con cura.

**Rate limiting**: limitare il numero di richieste che un client può fare in un intervallo di tempo. Difesa fondamentale contro abusi e brute force.

**SPA** (Single Page Application): app web che gira interamente nel browser caricando dati via API, senza ricaricare la pagina.

**XSS** (Cross-Site Scripting): vulnerabilità in cui un attaccante inietta JavaScript malevolo nella tua pagina. Se succede, ogni token in memoria JS è compromesso.

---

## Risorse per approfondire

- **MDN Web Docs** — documentazione di riferimento per `FormData`, `fetch`, HTTP, CORS
- **OAuth 2.0 Security Best Current Practice** (RFC 9700) — raccomandazioni ufficiali IETF
- **OWASP Top 10** — le vulnerabilità più comuni nelle web app
- **MongoDB University** — corsi gratuiti su pattern di accesso e sicurezza
- **Auth0 Blog** — articoli molto didattici su OAuth, OIDC, PKCE

---

*Dispensa redatta come sintesi di una conversazione sullo sviluppo di un form di anagrafica con scrittura su MongoDB.*
