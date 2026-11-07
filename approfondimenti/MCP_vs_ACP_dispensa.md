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

*Ultima consultazione delle fonti: 7 novembre 2026.*
