# Approfondimento 02 — Il PAC (Privilege Attribute Certificate)

← [Torna al documento principale](00_dispensa_principale.md)

---

Il PAC è una **struttura dati proprietaria Microsoft** inserita nel campo
`AuthorizationData` dei ticket Kerberos. È ciò che trasforma un'autenticazione
*“chi sei”* in qualcosa di immediatamente utile per l'autorizzazione *“cosa puoi
fare”*.

![Struttura del PAC dentro il ticket](../img/pac_struttura.svg)

## Cosa contiene

| Campo | Contenuto |
| --- | --- |
| **SID utente** | Identificatore unico e immutabile dell'account (es. `S-1-5-21-…-1013`). |
| **SID dei gruppi** | Tutti i gruppi AD a cui l'utente appartiene (globali, locali di dominio, universali, impliciti). |
| **UAC flags** | User Account Control: tipo di account, stato (abilitato/disabilitato), requisiti password. |
| **Info di logon** | Ora del logon, scadenza password, percorso profilo, home directory. |
| **Server Signature** | HMAC con la chiave del server destinatario. |
| **KDC Signature** | HMAC con la chiave del KDC. |

## Le due firme: ruoli diversi

- La **KDC Signature** dimostra l'**origine**: solo il KDC conosce la propria
  chiave, quindi nessuno può costruire un PAC valido senza passare dal KDC.
- La **Server Signature** è **specifica per ogni Service Ticket**: il KDC la
  ricalcola con la chiave condivisa con il server di destinazione, impedendo che
  un ticket valido per il file server venga riutilizzato sul database SQL.

## Implicazioni di sicurezza

> **⚠️ MS14-068 (2014).** A causa di un bug, il KDC accettava PAC con **firma non
> valida**: un utente normale poteva forgiare un PAC che lo dichiarava *Domain
> Admin* e auto-elevarsi. È uno dei motivi per cui la verifica delle firme del
> PAC è un punto critico.

> **⚠️ Golden Ticket.** Se un attaccante ottiene la chiave dell'account speciale
> **`krbtgt`** (quello con cui il KDC cifra i TGT), può **forgiare TGT
> arbitrari** con qualsiasi PAC — inclusi privilegi di Domain Admin — validi fino
> alla rotazione della chiave `krbtgt`. È la ragione per cui questa chiave va
> ruotata periodicamente, e due volte di seguito in caso di compromissione.

## Il legame con LDAP

Quando l'utente esegue il logon, il KDC usa **LDAP internamente** per recuperare
dal database AD gli attributi dell'utente (SID, gruppi, UAC flags, scadenza
password) e li inserisce nel PAC. Da quel momento le risorse di rete **non hanno
più bisogno di interrogare LDAP**: tutto ciò che serve è già dentro il ticket.
È questo che rende il sistema scalabile (vedi [approfondimento 04](04_ldap.md)).

---

← [01 · Kerberos e il KDC](01_kerberos_kdc.md) ·
➡️ Prossimo: [03 · Wi-Fi, RADIUS e 802.1X](03_radius_wifi.md)
