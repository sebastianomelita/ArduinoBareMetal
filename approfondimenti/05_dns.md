# Approfondimento 05 — DNS: il sistema nervoso di Active Directory

← [Torna al documento principale](00_dispensa_principale.md)

---

Active Directory dipende dal DNS per due categorie di record: i record di
**localizzazione dei servizi (SRV)** e i record **host (A/AAAA)** dei Domain
Controller. Tutti vengono registrati automaticamente dal DC all'avvio, tramite
il servizio **Netlogon**.

![Come il client trova il Domain Controller via SRV](../img/dns_srv.svg)

## I record SRV

I record SRV (Service Locator, **RFC 2782**) hanno il formato:

```text
_servizio._protocollo.dominio   TTL   IN   SRV   priorità  peso  porta  hostname
```

| Record | Scopo |
| --- | --- |
| `_ldap._tcp.<dominio>` | Localizza i DC che espongono LDAP (389). |
| `_kerberos._tcp.<dominio>` | Localizza il KDC per l'autenticazione (88 TCP). Usato al logon per il TGT. |
| `_kerberos._udp.<dominio>` | Stessa funzione su UDP 88 (messaggi piccoli; TCP per quelli grandi, es. PAC con molti gruppi). |
| `_kpasswd._tcp.<dominio>` | Servizio di cambio password Kerberos (464). |
| `_gc._tcp.<dominio>` | Global Catalog (3268): copia parziale di tutti gli oggetti della foresta, per ricerche cross-dominio e risoluzione degli UPN. |
| `_ldap._tcp.dc._msdcs.<dominio>` | Sottozona `_msdcs`: localizza specificamente i DC (non tutti i server LDAP). |
| `_ldap._tcp.pdc._msdcs.<dominio>` | Localizza il **PDC Emulator** (ruolo FSMO: cambi password, sincronizzazione orario, compatibilità legacy). |
| `_ldap._tcp.<site>._sites.<dominio>` | Record *site-aware*: localizza il DC nel **sito più vicino**, fondamentale in reti multi-sede per non passare per la WAN. |

## Record host (A/AAAA) e PTR

Oltre agli SRV, ogni DC deve avere:

- **Record A / AAAA** — FQDN del DC (es. `dc01.scuola.local`) → indirizzo IP.
  Senza, gli SRV puntano a un nome che non si risolve.
- **Record PTR (reverse DNS)** — IP → FQDN. Necessario per Kerberos (in alcuni
  scenari il KDC verifica il reverse DNS del client) e per log leggibili.

> **⚠️ Problemi DNS = problemi AD.** Sintomi tipici di un DNS mal configurato:
> - logon lento o fallito (*“Impossibile connettersi a un controller di dominio”*);
> - Kerberos fallisce e il sistema **ricade su NTLM** (più lento, meno sicuro);
> - le GPO non si applicano (*“Impossibile caricare le impostazioni utente”*);
> - la replica tra DC si interrompe;
> - errori SPN: i Service Ticket non vengono emessi perché il nome non si risolve.

## DNS integrato in AD

In ambienti enterprise le zone DNS sono memorizzate **dentro il database di AD**
(*AD-Integrated DNS*) anziché in file di testo. Vantaggi:

- **Replica automatica** dei record DNS insieme ai dati AD, senza zone secondarie.
- **Aggiornamenti sicuri** (*Secure Dynamic Update*): solo i computer membri del
  dominio, autenticati via Kerberos, possono registrare i propri record.
- **Alta disponibilità**: ogni DC è anche DNS autorevole, niente singolo punto di
  fallimento.

> **🔧 Verifica rapida da riga di comando**
> ```text
> nslookup -type=SRV _ldap._tcp.scuola.local   # tutti i record SRV del dominio
> ipconfig /all | findstr "DNS Servers"         # il client usa il DC come DNS?
> nltest /dsregdns                              # forza la registrazione dei record del DC
> ```

---

← [04 · LDAP e la directory](04_ldap.md) ·
➡️ Prossimo: [06 · Le GPO](06_gpo.md)
