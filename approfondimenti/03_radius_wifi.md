# Approfondimento 03 — Wi-Fi aziendale: RADIUS e 802.1X

← [Torna al documento principale](00_dispensa_principale.md)

---

Il protocollo **RADIUS** (RFC 2865) opera su **UDP 1812** (autenticazione) e
**1813** (accounting). Nel contesto **802.1X / WPA2-Enterprise** il flusso
coinvolge tre attori distinti.

![Flusso 802.1X / RADIUS con VLAN dinamica](../img/radius_8021x.svg)

## I tre attori

| Attore | Termine tecnico | Ruolo |
| --- | --- | --- |
| Dispositivo utente | **Supplicant** | Avvia la connessione e presenta le credenziali tramite EAP. |
| Access Point | **Authenticator / NAS** | Incapsula le credenziali EAP in pacchetti RADIUS. Non conosce le credenziali: è solo un relay. |
| Server RADIUS (NPS / FreeRADIUS) | **Authentication Server** | Valida le credenziali contro AD e risponde `Access-Accept` o `Access-Reject`. |

## I metodi EAP

| Metodo | Caratteristiche |
| --- | --- |
| **PEAP-MSCHAPv2** | Il più comune in ambienti AD. Username+password dentro un tunnel TLS; richiede un certificato sul server RADIUS. |
| **EAP-TLS** | Il più sicuro: autenticazione reciproca con certificati su client e server, nessuna password sulla rete. Richiede una PKI. |
| **PEAP-EAP-TLS** | Combina il tunnel TLS esterno con la verifica via certificato client interno; per requisiti di sicurezza elevati. |

## Le Network Policy (NPS)

In Microsoft AD il server RADIUS è il ruolo **NPS** (Network Policy Server). Le
Network Policy definiscono le condizioni d'accesso: gruppo AD di appartenenza,
tipo di connessione (Wi-Fi / VPN / 802.1X cablato), orario, e — caso d'uso più
potente — l'**assegnazione di VLAN dinamica** tramite attributi RADIUS
(`Tunnel-Type`, `Tunnel-Medium-Type`, `Tunnel-Private-Group-ID`).

> **🌐 VLAN dinamica.** Docenti e studenti si connettono alla stessa SSID
> *“Scuola”* ma finiscono automaticamente in reti separate, con firewall e
> policy diverse — tutto pilotato dai gruppi AD, senza configurare nulla sull'AP.

## Esempio reale: FreeRADIUS su Univention (Istituto Marconi)

A seconda del gruppo LDAP dell'utente, l'AP lo mette in una VLAN diversa:

| Gruppo LDAP | VLAN | Significato |
| --- | --- | --- |
| `nointernet-sc1` | 300 | rete senza Internet (dispositivi gestiti, IoT) |
| `lehrer-sc1` | 120 | rete docenti |
| `schueler-sc1` | 130 | rete studenti |
| *nessun gruppo valido* | — | accesso rifiutato (Reject) |

Estratto del file `/etc/freeradius/3.0/users`:

```text
DEFAULT Ldap-Group == "cn=nointernet-sc1,cn=groups,ou=sc1,dc=univention,dc=marconicloud,dc=it"
        Tunnel-Type = VLAN,
        Tunnel-Medium-Type = IEEE-802,
        Tunnel-Private-Group-Id = "300"

DEFAULT Ldap-Group == "cn=lehrer-sc1,cn=groups,ou=sc1,dc=univention,dc=marconicloud,dc=it"
        Tunnel-Type = VLAN,
        Tunnel-Medium-Type = IEEE-802,
        Tunnel-Private-Group-Id = "120"

DEFAULT Ldap-Group == "cn=schueler-sc1,cn=groups,ou=sc1,dc=univention,dc=marconicloud,dc=it"
        Tunnel-Type = VLAN,
        Tunnel-Medium-Type = IEEE-802,
        Tunnel-Private-Group-Id = "130"

DEFAULT Auth-Type := Reject
        # nessuna delle regole precedenti è scattata: accesso negato
```

### Anatomia delle righe

| Elemento | Significato |
| --- | --- |
| `DEFAULT` | Regola generica, valutata per qualsiasi username. |
| `Ldap-Group ==` | Verifica l'appartenenza al gruppo LDAP indicato (query sul DC nell'attributo `memberOf`). |
| `cn=lehrer-sc1, … ,dc=it` | DN completo del gruppo; si legge dal basso: `dc=it → dc=marconicloud → dc=univention` = dominio, `ou=sc1` = sede, `cn=groups` = contenitore, `cn=lehrer-sc1` = il gruppo. |
| `Tunnel-Type = VLAN` | Attributo RFC 2868: il tunneling è di tipo VLAN (802.1Q). |
| `Tunnel-Medium-Type = IEEE-802` | Mezzo fisico (Ethernet o Wi-Fi 802.11). |
| `Tunnel-Private-Group-Id` | Il numero di VLAN da assegnare; deve già esistere su switch e AP. |
| `Auth-Type := Reject` | Fallback: se nessuna condizione è vera, accesso negato. |

> **⚠️ L'ordine conta.** FreeRADIUS valuta le regole dall'alto verso il basso e
> si ferma alla **prima** corrispondenza. Se un utente è in più gruppi, finisce
> nella VLAN della prima regola che combacia. Mettere `nointernet-sc1` per prima
> è deliberato: ha priorità su tutto (dispositivi gestiti o quarantena).

> **📝 Nota sui nomi.** Univention nasce in Germania: i gruppi predefiniti di
> UCS@school usano il tedesco — *Lehrer* = docenti, *Schüler* = studenti.

In Univention lo stesso risultato si ottiene tramite il modulo **UCS@school** /
il pacchetto `univention-radius`, configurabile interamente dalla **Univention
Management Console** senza editare file a mano — comodo dove il personale tecnico
è limitato.

---

← [02 · Il PAC](02_pac.md) ·
➡️ Prossimo: [04 · LDAP e la directory](04_ldap.md)
