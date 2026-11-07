# MCP vs ACP nella gestione degli agenti AI
*Dispensa — aggiornata a novembre 2026*

---

## 0. Attenzione alla sigla: quale "ACP"?

Prima di confrontare, va chiarito un equivoco frequente. **"ACP" indica due protocolli diversi:**

| Sigla | Nome completo | Origine | Livello | Stato oggi |
|---|---|---|---|---|
| ACP | **Agent Client Protocol** | Zed Industries (+ JetBrains), 2025 | Editor/IDE ↔ agente | **Attivo**, in forte crescita [1][7] |
| ACP | **Agent Communication Protocol** | IBM Research (BeeAI), marzo 2025 | Agente ↔ agente | **Confluito in A2A** (ago 2025), archiviato [8][11] |

Oggi, quando si parla di "ACP" nel contesto degli agenti di coding, si intende quasi sempre **l'Agent Client Protocol di Zed** [7][8]. La dispensa si concentra su questo, ma la sezione 5 tratta anche l'ACP di IBM per completezza.

> Esiste anche un terzo "ACP", l'*Agentic Commerce Protocol* (pagamenti/e-commerce agentico), che non c'entra con questo confronto [9].

---

## Prima di tutto: che cos'è un agente?

### Definizione

> Un **agente AI** è un sistema software in cui un **modello linguistico (LLM)** riceve un **obiettivo** e lo persegue **in autonomia**, decidendo da sé, passo dopo passo, quali **azioni** compiere (usando **strumenti**), osservandone i **risultati** e continuando finché l'obiettivo non è raggiunto o serve l'intervento di un umano.

In una formula: **agente = modello + strumenti + ciclo decisionale + obiettivo**.

### Il ciclo dell'agente

<p align="center">
  <img src="img/ciclo_agente.svg" alt="Il ciclo dell'agente: percepisce, ragiona, agisce, osserva e ripete finché l'obiettivo è raggiunto; l'utente supervisiona via ACP, gli strumenti sono raggiunti via MCP e le skill guidano le scelte del modello" width="720">
</p>

<p align="center"><em>Figura 1 — Il ciclo dell'agente. Il ciclo ragiona → agisce → osserva si ripete finché l'obiettivo è raggiunto; l'utente affida il compito e supervisiona (ACP), gli strumenti sono raggiunti tramite MCP; le skill forniscono al modello le competenze per scegliere come agire.</em></p>

### I componenti

| Componente | Cosa fa | Esempio |
|---|---|---|
| **Modello (LLM)** | Il "cervello": interpreta, ragiona, decide | Claude, Gemini, GPT |
| **Obiettivo** | Il compito da portare a termine | "Correggi il test che fallisce" |
| **Strumenti (tool)** | Le "mani": permettono di agire sul mondo | leggere file, eseguire comandi, interrogare un DB |
| **Memoria / contesto** | Ciò che l'agente sa in quel momento | cronologia, file aperti, risultati precedenti |
| **Ciclo (loop)** | Ripete ragiona → agisce → osserva | più passi senza nuovi input dell'utente |
| **Supervisione umana** | Permessi, approvazioni, interruzioni | "Posso eseguire `rm`?" → l'utente approva |

### Agente vs chatbot vs workflow

| | **Chatbot** | **Workflow automatizzato** | **Agente** |
|---|---|---|---|
| Chi decide i passi | Nessuno: una domanda, una risposta | Il programmatore, in anticipo | **Il modello**, durante l'esecuzione |
| Usa strumenti | No (o al massimo uno) | Sì, in sequenza fissa | Sì, scelti dinamicamente |
| Gestisce imprevisti | No | Solo quelli previsti | Sì, si adatta agli esiti |
| Esempio | Risponde "come si scrive un for in Python?" | Pipeline fissa: riassumi → traduci → invia | Trova il bug, lo corregge, lancia i test, riprova se falliscono |

### Perché serve per capire MCP e ACP

Con questa definizione i due protocolli si collocano subito:

- **MCP** standardizza le **mani** dell'agente: come accede agli *strumenti* e al *contesto*.
- **ACP** standardizza la **postazione** dell'agente: come l'utente gli affida l'*obiettivo*, vede le sue azioni ed esercita la *supervisione* (permessi, diff, interruzioni) dall'editor.
- **A2A** (sezione 5) standardizza i **colleghi**: come un agente delega compiti a *un altro agente*.

### E le skill?

Accanto agli strumenti, un agente ha bisogno di **sapere come usarli bene**. A questo servono le skill.

> Una **skill** (*Agent Skill*) è un **pacchetto di istruzioni e conoscenze riutilizzabili** che insegna a un agente a svolgere un certo tipo di compito: procedure, convenzioni, esempi e, se servono, script e file di supporto. L'agente la **carica solo quando il compito la richiede** [15].

È un **concetto generico**, non legato a un solo prodotto. Nata in Claude, è stata pubblicata da Anthropic come **standard aperto** il 18 dicembre 2025 (specifica su agentskills.io) ed è adottata da molti agenti: Claude Code, Codex CLI, Gemini CLI, GitHub Copilot, Cursor, VS Code e altri [15][16].

#### Com'è fatta

Una skill è semplicemente una **cartella** [15][16]:

```
nome-skill/
├── SKILL.md        ← obbligatorio: intestazione YAML (name, description) + istruzioni in Markdown
├── scripts/        ← facoltativo: script eseguibili (Python, Bash, JS)
├── references/     ← facoltativo: documentazione di approfondimento
└── assets/         ← facoltativo: modelli, schemi, dati
```

Esempio minimo di `SKILL.md`:

```markdown
---
name: modellazione-contenitori
description: Usala quando l'utente chiede di modellare un contenitore stampabile in 3D (base + coperchio).
---
# Modellare un contenitore
1. Crea la base come corpo separato chiamato "Base".
2. Usa uno spessore di parete di almeno 2 mm.
3. Dopo ogni blocco di lavorazioni, fai uno screenshot e verifica le misure.
```

#### Caricamento progressivo (*progressive disclosure*)

Il punto chiave delle skill è che **non riempiono il contesto del modello** [15]:

1. **All'avvio** l'agente legge solo `name` e `description` di ogni skill (poche decine di token ciascuna).
2. **Quando un compito corrisponde** alla descrizione, carica il testo completo di `SKILL.md`.
3. **Solo se servono**, apre i file in `references/` o esegue gli `scripts/`.

Per questo la `description` è la parte più importante: è ciò che l'agente usa per decidere se attivare la skill.

#### Skill vs tool MCP

| | **Tool MCP** | **Skill** |
|---|---|---|
| Cos'è | Una **capacità**: una funzione che l'agente può invocare | Una **competenza**: istruzioni su *come* e *quando* fare le cose |
| Dove gira | In un server esterno (processo separato) | Dentro il contesto dell'agente (testo + eventuali script locali) |
| Formato | Server che parla JSON-RPC | Cartella con `SKILL.md` |
| Metafora | Le **mani** (gli attrezzi) | Il **manuale di mestiere** (come usare gli attrezzi) |
| Esempio | `execute("extrude", …)` | "Per un contenitore: prima la base, poi il coperchio, controlla con uno screenshot" |

Le due cose si **combinano**: la skill dice all'agente *quale sequenza di tool* usare e con quali convenzioni; il tool MCP esegue materialmente l'azione.

> **Esempio concreto — SolidCraft.** Il CAD SolidCraft espone i suoi comandi tramite un server MCP (`execute`, `batch`, `screenshot`, `export`...) e affianca una skill, `skills/solidcraft-modeling/reference.md`, che documenta parametri, convenzioni di nome ed espressioni: l'MCP dà all'agente la *possibilità* di modellare, la skill gli insegna a *farlo correttamente* [17].

In sintesi, completando la formula di prima: **agente = modello + strumenti (MCP) + competenze (skill) + ciclo decisionale + obiettivo**, ospitato in un'interfaccia (ACP).

---

## 1. L'idea in una frase

- **MCP (Model Context Protocol)** → collega un **agente ai suoi strumenti e dati** (database, file, API, servizi).
- **ACP (Agent Client Protocol)** → collega un **editor/IDE a un agente**, così l'utente può usare qualsiasi agente dentro qualsiasi editor.

Non sono concorrenti: **lavorano a livelli diversi e si usano insieme** [5][13].

```
  ┌──────────────┐   ACP    ┌──────────────┐   MCP    ┌──────────────────┐
  │  Editor/IDE  │ ◄──────► │   Agente AI  │ ◄──────► │ Tool / dati      │
  │ (Zed, IntelliJ│          │ (Claude Code,│          │ (DB, GitHub, FS, │
  │  Neovim...)  │          │  Gemini CLI) │          │  API, Slack...)  │
  └──────────────┘          └──────────────┘          └──────────────────┘
     "dove vive                "chi ragiona              "con cosa
      l'agente"                 e agisce"                 lavora"
```

---

## 2. MCP — Model Context Protocol

### Cos'è
Standard aperto introdotto da **Anthropic a fine 2024** per dare ai modelli/agenti un modo uniforme di accedere a contesto esterno [4][5]. Oggi è governato in modo neutrale (Agentic AI Foundation, Linux Foundation) [9].

### Problema che risolve
Il problema **N×M** delle integrazioni: senza standard, ogni applicazione AI doveva scrivere un connettore ad hoc per ogni servizio. Con MCP, un servizio espone **un server MCP** e qualsiasi client compatibile lo può usare.

### Ruoli
| Ruolo | Chi è |
|---|---|
| **Host / Client MCP** | L'applicazione AI o l'agente (Claude, Claude Code, un IDE con AI...) |
| **Server MCP** | Il "wrapper" di uno strumento o di una sorgente dati |

### Primitive principali
- **Tools** — funzioni che l'agente può invocare (es. `query_db`, `create_issue`).
- **Resources** — dati leggibili (file, record, documenti).
- **Prompts** — template di prompt riutilizzabili esposti dal server.
- Lato client: *sampling*, *elicitation* (richiesta di input all'utente), *roots*.

### Trasporto
JSON-RPC 2.0 su **stdio** (server locali) o **Streamable HTTP** (server remoti), con autenticazione OAuth 2.1 per i server HTTP [4][9].

### Ruolo nella gestione degli agenti
MCP definisce **cosa l'agente può fare e vedere**: è il livello delle *capacità*. Gestire un agente via MCP significa decidere quali server/tool abilitare, con quali permessi e su quali dati.

---

## 3. ACP — Agent Client Protocol

### Cos'è
Standard aperto creato da **Zed Industries (agosto 2025)**, licenza Apache 2.0, sviluppato poi insieme a **JetBrains** [1][3][12]. È esplicitamente modellato sul **LSP (Language Server Protocol)**: come LSP ha separato l'"intelligenza del linguaggio" dall'editor, ACP separa l'**agente AI** dall'**interfaccia dell'editor** [6][7].

### Problema che risolve
Anche qui un problema N×M: ogni agente (Claude Code, Gemini CLI, Codex, Copilot CLI...) doveva costruire un'integrazione dedicata per ogni editor, e ogni editor doveva sviluppare un pannello per ogni agente. Con ACP: **l'agente implementa ACP una volta e gira in tutti gli editor compatibili** [1][7].

### Ruoli (attenzione: invertiti rispetto a MCP!)
| Ruolo | Chi è |
|---|---|
| **Client ACP** | L'editor/IDE (Zed, IDE JetBrains, Neovim, Emacs...) |
| **Agente ACP** | L'agente di coding, spesso avviato come sottoprocesso |

### Cosa standardizza [3][5]
- **Inizializzazione** e negoziazione delle *capabilities*.
- **Sessioni** di lavoro e invio dei prompt dell'utente.
- **Streaming** degli aggiornamenti (messaggi, piani, tool call in corso).
- **Richieste di permesso** all'utente (es. "posso eseguire questo comando?").
- Lettura/scrittura file tramite l'editor, **visualizzazione dei diff**, terminali.
- **Cancellazione** di un'operazione in corso.

### Trasporto
JSON-RPC 2.0, tipicamente su **stdio** (agente come processo figlio dell'editor); supporto per agenti remoti via HTTP [6][9].

### Ecosistema (2025–2026)
- Ago 2025: lancio con **Gemini CLI** come prima integrazione esterna [1][13].
- Ott–dic 2025: partnership e supporto nativo negli **IDE JetBrains** [1][7].
- 28 gen 2026: **ACP Registry**, directory di agenti installabili con un clic da Zed e JetBrains [2][7].
- Gen 2026: **GitHub Copilot CLI** aggiunge ACP (public preview) [13]; in seguito anche Cursor, Goose, OpenHands, Codex e molti altri [1][13].
- Apr 2026: ACP è la funzionalità di punta di **Zed 1.0** [7].

### Ruolo nella gestione degli agenti
ACP definisce **come l'utente interagisce con l'agente e lo supervisiona**: è il livello dell'*interfaccia e del controllo umano* (approvazioni, diff, sessioni, interruzioni).

---

## 4. Confronto diretto MCP vs ACP (Agent Client Protocol)

*Fonti della tabella: [3][4][5][6].*

| Dimensione | **MCP** | **ACP** (Agent Client Protocol) |
|---|---|---|
| Domanda a cui risponde | *Con quali strumenti/dati lavora l'agente?* | *In quale interfaccia vive l'agente?* |
| Confine che standardizza | Agente ↔ tool/dati | Editor ↔ agente |
| Chi è il client | L'agente / app AI | L'editor / IDE |
| Chi è il server | Il wrapper del tool | L'agente |
| Origine | Anthropic, fine 2024 | Zed Industries, ago 2025 (+ JetBrains) |
| Ispirazione | "USB-C per l'AI" | LSP per gli agenti |
| Formato messaggi | JSON-RPC 2.0 | JSON-RPC 2.0 |
| Trasporto tipico | stdio / Streamable HTTP | stdio (sottoprocesso) / HTTP |
| Concetti chiave | tools, resources, prompts | sessioni, prompt turn, permessi, diff, piani |
| Dominio | Generale (chat, agenti, automazioni) | Soprattutto coding agent e IDE |
| Human-in-the-loop | Indiretto (elicitation, decide l'host) | Centrale (permessi e approvazioni in UI) |
| Distribuzione | Directory/registry di server MCP | ACP Registry (agenti) |

### Il punto chiave
Sono **complementari e componibili**. Esempio tipico:

1. **Zed** (client ACP) avvia **Claude Code** (agente ACP).
2. Claude Code si collega a un **server MCP** per accedere al database o a GitHub.
3. L'utente vede in Zed i diff e approva le modifiche (ACP); l'agente interroga il DB (MCP) [6].

Anzi, ACP prevede che l'editor possa **passare all'agente la configurazione dei server MCP** dell'utente: i due protocolli sono pensati per incastrarsi [3].

### Caso di studio: SolidCraft (chat, prodotto software e agente)

**SolidCraft** è un CAD 3D open source con una chat in cui si può collegare un agente esterno (Claude Code, Antigravity, Codex...). È un ottimo esempio perché **lo stesso programma ricopre due ruoli** [17][18]:

- **client ACP**: la chat avvia l'agente come sottoprocesso e gli parla via ACP;
- **server MCP**: il motore CAD espone i suoi comandi (`list_commands`, `execute`, `batch`, `inspect_design`, `screenshot`, `export`...) tramite il server `solidcraft-cli mcp --connect`, che SolidCraft passa all'agente all'apertura della sessione (`session/new`).

L'agente, dunque, non "sa disegnare": riceve il prompt dalla chat (ACP), chiede al modello cosa fare (API del vendor) e **richiama SolidCraft** per eseguire le operazioni (MCP). Il cerchio si chiude sullo stesso prodotto.

<p align="center">
  <img src="img/solidcraft_sequenza.svg" alt="Diagramma di sequenza: l'utente scrive nella chat di SolidCraft, la chat inoltra il prompt all'agente via ACP, l'agente consulta il modello via API, chiama il motore CAD di SolidCraft via MCP, il gate chiede l'approvazione all'utente, il risultato torna all'agente e infine alla chat" width="820">
</p>

<p align="center"><em>Figura 2 — Un turno di conversazione in SolidCraft (semplificato da [17]). Blu: ACP; arancio: API del modello; viola: MCP; grigio: utente.</em></p>

**Lettura passo per passo**

| Passi | Chi parla con chi | Protocollo | Cosa succede |
|---|---|---|---|
| 1 | Utente → chat | — | L'utente scrive la richiesta nel pannello di SolidCraft |
| 2 | Chat → agente | **ACP** | La chat inoltra il prompt (`session/prompt`) |
| 3–4 | Agente ↔ modello | **API del vendor** | L'agente manda prompt, elenco dei tool e skill; il modello decide quale tool chiamare |
| 5 | Agente → motore CAD | **MCP** | L'agente invoca il tool (es. `execute` con un'estrusione) |
| 6–7 | Motore (gate) ↔ utente | — | Il controllo permessi (`gate.rs`) chiede conferma se la modalità è *Ask* (le altre sono *Edits* e *All*) |
| 8 | Motore → agente | **MCP** | Il risultato torna all'agente, eventualmente con uno screenshot |
| 9–10 | Agente ↔ modello | **API del vendor** | Il modello valuta l'esito: nuovo tool (si ripete il ciclo) o risposta finale |
| 11–12 | Agente → chat → utente | **ACP** | Messaggi, `tool_call` e fine turno arrivano in streaming; la chat aggiorna conversazione e vista 3D |

**Cosa notare**

- I passi 3–10 sono il **ciclo dell'agente** della Figura 1: ragiona (3–4), agisce (5), osserva (8–9).
- Nell'inizializzazione ACP SolidCraft **non concede all'agente accesso diretto a file system e terminale**: ogni modifica al modello passa per MCP, quindi per il gate dei permessi [17].
- La **skill** `solidcraft-modeling` insegna all'agente parametri e convenzioni dei comandi: senza, l'agente avrebbe i tool ma non saprebbe usarli bene [17].
- Il **modello** (Gemini, Claude...) non parla mai direttamente con SolidCraft: tutto passa per il programma agente.

> **Nota — dove nasce la richiesta di permesso.** Nella figura, come nel diagramma originale di SolidCraft [17], la richiesta di approvazione (passi 6–7) parte dal **gate del motore CAD**, cioè dal lato MCP: è SolidCraft a controllare ogni tool call prima di eseguirla. Nella **specifica ACP generica**, invece, è l'**agente** a chiedere il permesso al client prima di un'azione delicata, con il metodo `session/request_permission`: la chat mostra la richiesta e restituisce la scelta dell'utente (consenti una volta, consenti sempre, rifiuta) [3].
>
> | | Permesso via ACP (generico) | Permesso via gate MCP (SolidCraft) |
> |---|---|---|
> | Chi decide di chiedere | L'agente | Il prodotto che esegue il tool |
> | Messaggio | `session/request_permission` (agente → client) | Controllo interno (`gate.rs`) prima dell'esecuzione |
> | Vantaggio | Funziona con qualsiasi editor ACP | Il controllo non dipende dalla "buona volontà" dell'agente |
>
> I due meccanismi non si escludono: un prodotto può usarli entrambi. Il gate lato esecuzione è però la difesa più robusta, perché si applica anche a un agente che non chiede il permesso.

---

## 5. E l'ACP di IBM (Agent Communication Protocol)?

### Cos'era
Protocollo lanciato da **IBM Research a marzo 2025** per la piattaforma open source **BeeAI**, con l'ambizione di diventare "l'HTTP della comunicazione tra agenti". Basato su **REST**, messaggi multimodali, supporto sincrono/asincrono/streaming [8][9][10].

### Che fine ha fatto
Poche settimane dopo, Google ha lanciato **A2A (Agent2Agent)**, che risolveva lo stesso problema [10]. Ad **agosto 2025** IBM ha **fuso ACP dentro A2A** sotto la Linux Foundation: il repository ACP è archiviato, BeeAI ora usa A2A e IBM ha pubblicato una guida di migrazione [8][9][11][14]. A2A ha rilasciato la v1.0 stabile nel 2026 [9].

### Confronto con MCP
| | MCP | ACP IBM → oggi A2A |
|---|---|---|
| Livello | Agente ↔ strumento | Agente ↔ agente |
| Controparte | Un tool "passivo", che esegue | Un altro agente autonomo, con modello e obiettivi propri |
| Uso | Dare capacità a un agente | Delegare compiti e far collaborare agenti (multi-agent) |

**Indicazione pratica:** per progetti nuovi di comunicazione agente-agente, usa **A2A**, non l'ACP di IBM.

---

## 6. La mappa completa dello "stack" agentico (2026)

Ogni protocollo possiede **un confine** [5][9]:

| Confine | Protocollo |
|---|---|
| Agente ↔ strumenti e dati | **MCP** |
| Editor/IDE ↔ agente | **ACP** (Agent Client Protocol) |
| Agente ↔ agente | **A2A** (che ha assorbito l'ACP di IBM) |
| Agente ↔ interfaccia utente (app web) | **AG-UI** |

```
                 ┌──────────── A2A ────────────┐
                 ▼                             ▼
 Editor ─ACP─► Agente A ─MCP─► tool       Agente B ─MCP─► tool
```

---

## 7. Quale usare? Guida decisionale

- **Vuoi dare al tuo agente accesso a un servizio (DB, CRM, filesystem, API)?** → scrivi/usa un **server MCP**.
- **Hai costruito un agente di coding e vuoi che giri in Zed, JetBrains, Neovim...?** → implementa **ACP** (Agent Client Protocol).
- **Stai costruendo un editor e vuoi supportare tanti agenti senza integrazioni custom?** → diventa **client ACP**.
- **Vuoi che più agenti autonomi (anche di vendor diversi) si scambino compiti?** → **A2A** (non l'ACP di IBM, ormai archiviato).

---

## 8. Riepilogo da ricordare

1. **MCP = mani e occhi dell'agente** (strumenti e contesto).
2. **ACP (Zed) = la "scrivania" dell'agente** (l'editor dove lavora sotto la supervisione dell'utente).
3. Ruoli **invertiti**: in MCP l'agente è *client*; in ACP l'agente è *server/agent* e l'editor è *client*.
4. Entrambi usano **JSON-RPC 2.0**, ma risolvono problemi diversi e **si usano insieme**.
5. **ACP di IBM ≠ ACP di Zed**: quello di IBM era agente-agente ed è confluito in **A2A**.
6. **Skill = manuale di mestiere dell'agente**: istruzioni riutilizzabili (standard aperto `SKILL.md`), caricate solo quando servono; non sono un protocollo ma si combinano con i tool MCP.

---

## Riferimenti

I numeri tra parentesi quadre nel testo rimandano a questo elenco.

| # | Fonte | Tipo |
|---|---|---|
| [1] | Zed Industries — [*Agent Client Protocol*](https://zed.dev/acp) (pagina ufficiale, annunci e timeline) | Ufficiale |
| [2] | Zed Industries — [*The ACP Registry is Live*](https://zed.dev/blog/acp-registry) (28 gen 2026) | Ufficiale |
| [3] | [*Agent Client Protocol* — specifica e documentazione](https://agentclientprotocol.com) (Apache 2.0) | Specifica |
| [4] | [*Model Context Protocol* — specifica e documentazione](https://modelcontextprotocol.io) | Specifica |
| [5] | mcp.directory — [*Agent Client Protocol vs MCP (2026)*](https://mcp.directory/blog/agent-client-protocol-vs-mcp-2026) | Analisi |
| [6] | M. Danilchenko — [*ACP: Connect Any AI Agent to Any Editor*](https://dev.to/dmaxdev/agent-client-protocol-acp-connect-any-ai-agent-to-any-editor-2p0m), DEV Community | Articolo |
| [7] | R. Walker — [*Zed Agent Client Protocol*](https://rywalker.com/research/zed-agent-client-protocol) (scheda di ricerca) | Analisi |
| [8] | R. Walker — [*ACP (Agent Communication Protocol)*](https://rywalker.com/research/acp-agent-communication-protocol) (ACP di IBM) | Analisi |
| [9] | Maxim AI — [*Agent Communication Protocols Compared: A2A, MCP and ACP*](https://www.getmaxim.ai/articles/agent-communication-protocols/) | Analisi |
| [10] | IBM Research — [*The simplest protocol for AI agents to work together*](https://research.ibm.com/blog/agent-communication-protocol-ai) | Ufficiale |
| [11] | LF AI & Data Foundation — [confluenza di ACP (IBM) in A2A](https://lfaidata.foundation/?p=7689) | Ufficiale |
| [12] | Tessl — [*Zed debuts Agent Client Protocol*](https://tessl.io/blog/zed-debuts-agent-client-protocol-to-connect-ai-coding-agents-to-any-editor/) (set 2025) | Articolo |
| [13] | Morph — [*What Is the Agent Client Protocol?*](https://www.morphllm.com/agent-client-protocol) (adozione per agente/editor) | Analisi |
| [14] | Olostep — [*A2A Protocol (Agent2Agent)*](https://www.olostep.com/blog/a2a-protocol-agent2agent) (timeline A2A) | Articolo |
| [15] | Firecrawl — [*Agent Skills Explained: How SKILL.md Files Work*](https://www.firecrawl.dev/blog/agent-skills) | Articolo |
| [16] | [*Agent Skills* — specifica aperta](https://agentskills.io) | Specifica |
| [17] | DeepWiki — [*SolidCraft: In-App Agent Chat and Skills*](https://deepwiki.com/tangency-labs/solidcraft/7.3-in-app-agent-chat-and-skills) | Documentazione |
| [18] | DeepWiki — [*SolidCraft: Automation and Agent Integration*](https://deepwiki.com/tangency-labs/solidcraft/7-automation-and-agent-integration) | Documentazione |

*Ultima consultazione delle fonti: 7 novembre 2026.*
