# Progettare con l'AI: dagli agenti al pezzo stampato

*Percorso di dispense sulla progettazione assistita da agenti AI — novembre 2026*

Oggi l'AI non serve più solo a rispondere a domande: un **agente** può lavorare dentro un programma (un editor di codice, un CAD come SolidCraft), usare i suoi comandi e portare a termine un compito sotto la supervisione di chi progetta. Questo percorso spiega **come funziona** e **come si usa bene**, partendo dai concetti e arrivando a un caso pratico: il contenitore stampato in 3D per un nodo LoRaWAN.

---

## Parte A — Capire l'AI agentica

| | Dispensa | Di cosa parla |
|---|---|---|
| 1 | [MCP vs ACP nella gestione degli agenti AI](MCP_vs_ACP_dispensa.md) | Agente, skill e contesto; i protocolli che collegano gli agenti agli strumenti (MCP), alle interfacce (ACP) e ad altri agenti (A2A); la programmazione agentica; il caso di studio di SolidCraft |
| 2 | [Costruire un assistente AI aziendale in 8 strati](AI_aziendale_8_strati.md) | Cosa serve intorno a un modello perché diventi un assistente affidabile: documenti (RAG), strumenti, permessi, guardrail, costi, tracce e test |

## Parte B — Progettare un contenitore con l'AI

| | Dispensa | Di cosa parla |
|---|---|---|
| 3 | [Lessico tecnico per contenitori elettronici](Lessico%20tecnico%20per%20contenitori%20elettronici.md) | Il vocabolario tecnico per descrivere un contenitore elettronico: serve anche per scrivere istruzioni precise all'agente |
| 4 | [Contenitore per nodo LoRaWAN – foglio di lavorazione](dispensa-contenitore-lorawan.md) | Progetto passo per passo del contenitore (varianti a muro e su palo), verifiche e stampa 3D, configurazione dell'assistente di SolidCraft via ACP e prompt di esempio |
| 5 | [Esercitazioni sui fogli di lavorazione](esercitazioni-fogli-di-lavorazione.md) | Esercitazioni pratiche basate sui fogli di lavorazione |

---

## Come leggere il percorso

| Se vuoi… | Parti da | Poi |
|---|---|---|
| **Capire come funziona un agente** prima di usarlo | 1 | 4, 5 |
| **Progettare subito il contenitore** con l'assistente di SolidCraft | 3, 4 | 1 (sezioni sul contesto e sul caso SolidCraft) |
| **Esercitarti** | 3 | 5, con 4 come riferimento |
| Capire come si costruisce un **assistente AI per un'azienda** | 1 | 2 |

> **Il filo conduttore.** Le dispense 1 e 4 si parlano: la 4 usa SolidCraft con un agente collegato via ACP, la 1 spiega cosa succede "dietro le quinte" (chi è client, chi è server, dove vive il contesto, perché servono le skill). Le dispense 1 e 2 condividono la stessa **mappa a strati** dei sistemi agentici.
