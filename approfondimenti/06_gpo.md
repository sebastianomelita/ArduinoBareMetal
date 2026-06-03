# Approfondimento 06 — Le GPO (Group Policy Objects)

← [Torna al documento principale](00_dispensa_principale.md)

---

## Struttura e memorizzazione

Una GPO è composta da **due parti fisicamente separate** sul Domain Controller:

| Parte | Dove | Contenuto |
| --- | --- | --- |
| **GPC — Group Policy Container** | Oggetto LDAP in `CN=Policies,CN=System,DC=…` | Metadati: GUID univoco, versione, link alle OU. Replicato dalla normale replica AD. |
| **GPT — Group Policy Template** | Cartella in `\\dominio\SYSVOL\dominio\Policies\{GUID}\` | I file delle impostazioni (`.pol`, `.inf`, script, `.msi`). Replicato tramite DFS-R. |

Il **SYSVOL** è una cartella condivisa presente su ogni DC e accessibile da tutti
i client. Se GPC e GPT sono disallineati (versione diversa), i client ricevono
GPO **corrotte o incomplete**.

## Ordine di applicazione e precedenza (LSDOU)

![L'ordine LSDOU di applicazione delle GPO](../img/gpo_lsdou.svg)

| Ordine | Livello | Descrizione |
| --- | --- | --- |
| 1° | **L** — Local | Policy locali del singolo PC (senza AD). Applicate per prime, quindi sovrascrivibili da tutto il resto. |
| 2° | **S** — Site | Legate al sito AD del DC più vicino (es. proxy diverso per Milano e Roma). |
| 3° | **D** — Domain | Per tutto il dominio (es. policy password, screensaver, antivirus). |
| 4° | **OU** — Organizational Unit | **Vince su tutto.** Più la OU è specifica (annidata in profondità), più ha priorità. |

> **📌 Regola d'oro.** L'**ultima** GPO applicata vince. Si applicano in ordine
> 1 → 4, quindi le impostazioni della OU prevalgono sul dominio, che prevale sul
> sito, che sovrascrive il locale. All'interno dello stesso livello, se ci sono
> più GPO, quella con *link order* più basso viene applicata per ultima e quindi
> vince.

## GPO Computer vs GPO Utente

Ogni GPO ha due sezioni indipendenti:

| Sezione | Quando si applica | Esempi |
| --- | --- | --- |
| **Computer Configuration** | All'avvio del PC, prima del login; vale a prescindere da chi si logga. | Installazione software, firewall Windows, disabilitazione porte USB. |
| **User Configuration** | Al logon dell'utente; vale a prescindere da quale PC usa. | Sfondo, mappatura stampanti/cartelle, restrizioni del Pannello di Controllo. |

La separazione è potente: un docente che usa **qualsiasi** PC del laboratorio
riceve sempre le sue impostazioni utente, mentre il PC mantiene le sue
impostazioni computer fisse.

## Come il client scarica e applica le GPO

È la sintesi di tutta la dispensa:

1. **DNS** — il client trova il DC tramite i record SRV `_ldap._tcp` e `_kerberos._tcp`.
2. **Kerberos** — l'utente si autentica → ottiene il TGT → ottiene il ST per CIFS/SYSVOL.
3. **LDAP** — il client chiede ad AD quali GPO si applicano alla sua OU (attributo `gPLink`).
4. **SYSVOL** — scarica i file delle GPO da `\\dominio\SYSVOL\dominio\Policies\{GUID}\`.
5. **Applicazione** — Windows applica nell'ordine LSDOU. Le GPO computer si
   rieseguono ogni **90 ± 30 min** in background; le GPO utente a ogni logon o
   dopo lo stesso intervallo.

> **🔧 Comandi utili**
> ```text
> gpupdate /force            forza il ricalcolo immediato di tutte le GPO
> gpresult /r                mostra le GPO applicate all'utente/computer corrente
> gpresult /h report.html    report HTML dettagliato con le GPO "vincitrici"
> ```

---

← [05 · DNS](05_dns.md) ·
➡️ Prossimo: [07 · Il Single Sign-On nei prodotti Microsoft](07_sso_microsoft.md)
