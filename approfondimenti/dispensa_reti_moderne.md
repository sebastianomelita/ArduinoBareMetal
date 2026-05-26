# Dispensa sintetica — Modelli emergenti di rete sicura

> Dal modello "Trusted Network" classico ai paradigmi moderni: Zero Trust, ZTNA, SSE e SASE.
> Una guida sintetica per orientarsi nel vocabolario della cybersecurity di rete contemporanea.

---

## 1. Il punto di partenza: il vecchio modello "Castello e Fossato"

Per decenni la sicurezza di rete si è basata sul concetto di **perimetro fidato**: dentro la rete aziendale ci si fidava, fuori no. Una volta dentro (via VPN o fisicamente in sede), avevi accesso ampio a tutto.

<svg viewBox="0 0 700 320" xmlns="http://www.w3.org/2000/svg" style="max-width:100%;height:auto;font-family:system-ui,sans-serif">
  <defs>
    <linearGradient id="castle" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0%" stop-color="#fef3c7"/>
      <stop offset="100%" stop-color="#fcd34d"/>
    </linearGradient>
  </defs>
  <rect width="700" height="320" fill="#f8fafc"/>
  <!-- Outside threats -->
  <text x="40" y="40" font-size="13" fill="#991b1b" font-weight="600">Esterno = ostile</text>
  <circle cx="60" cy="80" r="18" fill="#fecaca" stroke="#dc2626" stroke-width="2"/>
  <text x="60" y="85" font-size="16" text-anchor="middle">👤</text>
  <circle cx="60" cy="140" r="18" fill="#fecaca" stroke="#dc2626" stroke-width="2"/>
  <text x="60" y="145" font-size="16" text-anchor="middle">🦠</text>
  <circle cx="60" cy="200" r="18" fill="#fecaca" stroke="#dc2626" stroke-width="2"/>
  <text x="60" y="205" font-size="16" text-anchor="middle">⚠️</text>
  <!-- Moat -->
  <rect x="120" y="60" width="40" height="220" fill="#60a5fa" opacity="0.5"/>
  <text x="140" y="305" font-size="11" text-anchor="middle" fill="#1e40af">Firewall</text>
  <!-- Castle walls -->
  <rect x="160" y="60" width="500" height="220" fill="url(#castle)" stroke="#92400e" stroke-width="3" rx="8"/>
  <text x="410" y="90" font-size="15" font-weight="700" text-anchor="middle" fill="#78350f">RETE FIDATA (Trusted)</text>
  <text x="410" y="108" font-size="11" text-anchor="middle" fill="#78350f">"se sei dentro, ti fido"</text>
  <!-- Internal resources -->
  <g>
    <rect x="200" y="140" width="80" height="60" fill="#fff" stroke="#92400e" rx="4"/>
    <text x="240" y="170" font-size="22" text-anchor="middle">🖥️</text>
    <text x="240" y="192" font-size="10" text-anchor="middle">Server</text>
  </g>
  <g>
    <rect x="310" y="140" width="80" height="60" fill="#fff" stroke="#92400e" rx="4"/>
    <text x="350" y="170" font-size="22" text-anchor="middle">💾</text>
    <text x="350" y="192" font-size="10" text-anchor="middle">Database</text>
  </g>
  <g>
    <rect x="420" y="140" width="80" height="60" fill="#fff" stroke="#92400e" rx="4"/>
    <text x="460" y="170" font-size="22" text-anchor="middle">📁</text>
    <text x="460" y="192" font-size="10" text-anchor="middle">File share</text>
  </g>
  <g>
    <rect x="530" y="140" width="80" height="60" fill="#fff" stroke="#92400e" rx="4"/>
    <text x="570" y="170" font-size="22" text-anchor="middle">👨‍💼</text>
    <text x="570" y="192" font-size="10" text-anchor="middle">User PC</text>
  </g>
  <!-- Lateral movement arrow -->
  <path d="M 280 230 Q 350 250 420 230" stroke="#dc2626" stroke-width="2" fill="none" stroke-dasharray="5,3" marker-end="url(#arr)"/>
  <text x="350" y="265" font-size="10" text-anchor="middle" fill="#dc2626" font-style="italic">una volta dentro → movimento laterale libero</text>
  <defs>
    <marker id="arr" markerWidth="10" markerHeight="10" refX="9" refY="3" orient="auto">
      <polygon points="0 0, 10 3, 0 6" fill="#dc2626"/>
    </marker>
  </defs>
</svg>

**Problemi del modello perimetrale**:
- Una volta che un attaccante entra (phishing, credenziali rubate, supply chain), può muoversi lateralmente
- Il "perimetro" non esiste più: cloud, SaaS, smart working hanno dissolto il confine aziendale
- Le VPN tradizionali concedono accesso troppo ampio (network-level, non application-level)

---

## 2. Il cambio di paradigma: Zero Trust

Nel 2010 Forrester (John Kindervag) propone il modello **Zero Trust**, formalizzato poi dal NIST con lo standard **SP 800-207**.

> **Principio guida**: *"Never trust, always verify"* — non fidarti mai, verifica sempre.

<svg viewBox="0 0 700 360" xmlns="http://www.w3.org/2000/svg" style="max-width:100%;height:auto;font-family:system-ui,sans-serif">
  <rect width="700" height="360" fill="#f8fafc"/>
  <text x="350" y="30" font-size="16" font-weight="700" text-anchor="middle" fill="#1e293b">I 5 pilastri del Zero Trust</text>

  <!-- Center hub -->
  <circle cx="350" cy="190" r="55" fill="#1e40af" stroke="#1e3a8a" stroke-width="3"/>
  <text x="350" y="185" font-size="13" font-weight="700" text-anchor="middle" fill="#fff">ZERO</text>
  <text x="350" y="202" font-size="13" font-weight="700" text-anchor="middle" fill="#fff">TRUST</text>

  <!-- Pillar 1: Identity -->
  <g transform="translate(120,80)">
    <circle cx="0" cy="0" r="42" fill="#dbeafe" stroke="#3b82f6" stroke-width="2"/>
    <text x="0" y="-5" font-size="22" text-anchor="middle">👤</text>
    <text x="0" y="15" font-size="11" font-weight="600" text-anchor="middle" fill="#1e40af">Identità</text>
  </g>
  <text x="120" y="145" font-size="10" text-anchor="middle" fill="#475569">SSO + MFA</text>

  <!-- Pillar 2: Device -->
  <g transform="translate(580,80)">
    <circle cx="0" cy="0" r="42" fill="#dcfce7" stroke="#22c55e" stroke-width="2"/>
    <text x="0" y="-5" font-size="22" text-anchor="middle">💻</text>
    <text x="0" y="15" font-size="11" font-weight="600" text-anchor="middle" fill="#15803d">Dispositivo</text>
  </g>
  <text x="580" y="145" font-size="10" text-anchor="middle" fill="#475569">Posture check</text>

  <!-- Pillar 3: Network -->
  <g transform="translate(80,250)">
    <circle cx="0" cy="0" r="42" fill="#fef3c7" stroke="#f59e0b" stroke-width="2"/>
    <text x="0" y="-5" font-size="22" text-anchor="middle">🌐</text>
    <text x="0" y="15" font-size="11" font-weight="600" text-anchor="middle" fill="#b45309">Rete</text>
  </g>
  <text x="80" y="315" font-size="10" text-anchor="middle" fill="#475569">Microsegmentazione</text>

  <!-- Pillar 4: Application -->
  <g transform="translate(620,250)">
    <circle cx="0" cy="0" r="42" fill="#fce7f3" stroke="#ec4899" stroke-width="2"/>
    <text x="0" y="-5" font-size="22" text-anchor="middle">⚙️</text>
    <text x="0" y="15" font-size="11" font-weight="600" text-anchor="middle" fill="#be185d">App / Workload</text>
  </g>
  <text x="620" y="315" font-size="10" text-anchor="middle" fill="#475569">Least privilege</text>

  <!-- Pillar 5: Data -->
  <g transform="translate(350,330)">
    <circle cx="0" cy="0" r="38" fill="#ede9fe" stroke="#8b5cf6" stroke-width="2"/>
    <text x="0" y="-5" font-size="20" text-anchor="middle">🔐</text>
    <text x="0" y="13" font-size="11" font-weight="600" text-anchor="middle" fill="#6d28d9">Dati</text>
  </g>

  <!-- Connections -->
  <line x1="155" y1="105" x2="305" y2="170" stroke="#94a3b8" stroke-width="1.5"/>
  <line x1="545" y1="105" x2="395" y2="170" stroke="#94a3b8" stroke-width="1.5"/>
  <line x1="115" y1="240" x2="298" y2="210" stroke="#94a3b8" stroke-width="1.5"/>
  <line x1="585" y1="240" x2="402" y2="210" stroke="#94a3b8" stroke-width="1.5"/>
  <line x1="350" y1="245" x2="350" y2="292" stroke="#94a3b8" stroke-width="1.5"/>
</svg>

**Cambia tutto rispetto al modello trusted**:

| Aspetto | Trusted Network classica | Zero Trust |
|---|---|---|
| Fiducia | Implicita (basata sulla posizione) | Esplicita (basata su identità + contesto) |
| Verifica | Una volta al login | Continua, per ogni richiesta |
| Accesso post-login | Ampio (intera rete) | Granulare (singola app/risorsa) |
| Movimento laterale | Libero | Impedito (microsegmentazione) |
| Perimetro | Rete aziendale | L'identità è il nuovo perimetro |

Zero Trust è un **modello concettuale**, non un prodotto. Le tecnologie che lo implementano si chiamano **ZTNA, SSE, SASE**.

---

## 3. ZTNA — Zero Trust Network Access

**ZTNA** è la categoria di soluzioni che implementa Zero Trust per **l'accesso alle applicazioni**. È il sostituto moderno della VPN.

<svg viewBox="0 0 700 380" xmlns="http://www.w3.org/2000/svg" style="max-width:100%;height:auto;font-family:system-ui,sans-serif">
  <rect width="700" height="380" fill="#f8fafc"/>
  <text x="350" y="28" font-size="15" font-weight="700" text-anchor="middle" fill="#1e293b">VPN tradizionale vs ZTNA</text>

  <!-- VPN side (left) -->
  <text x="175" y="60" font-size="13" font-weight="700" text-anchor="middle" fill="#991b1b">VPN classica</text>
  <g transform="translate(60,90)">
    <rect width="40" height="40" rx="5" fill="#e0e7ff" stroke="#6366f1"/>
    <text x="20" y="27" font-size="20" text-anchor="middle">👤</text>
    <text x="20" y="55" font-size="9" text-anchor="middle">Utente</text>
  </g>
  <!-- Tunnel to whole network -->
  <path d="M 105 110 L 165 110" stroke="#6366f1" stroke-width="3"/>
  <text x="135" y="100" font-size="9" text-anchor="middle" fill="#475569">tunnel</text>
  <rect x="165" y="80" width="160" height="200" fill="#fef2f2" stroke="#dc2626" stroke-width="2" rx="6" stroke-dasharray="5,3"/>
  <text x="245" y="100" font-size="10" font-weight="600" text-anchor="middle" fill="#991b1b">tutta la rete esposta</text>
  <g transform="translate(180,115)">
    <rect width="50" height="35" fill="#fff" stroke="#dc2626" rx="3"/>
    <text x="25" y="24" font-size="16" text-anchor="middle">🖥️</text>
  </g>
  <g transform="translate(245,115)">
    <rect width="50" height="35" fill="#fff" stroke="#dc2626" rx="3"/>
    <text x="25" y="24" font-size="16" text-anchor="middle">💾</text>
  </g>
  <g transform="translate(180,160)">
    <rect width="50" height="35" fill="#fff" stroke="#dc2626" rx="3"/>
    <text x="25" y="24" font-size="16" text-anchor="middle">📁</text>
  </g>
  <g transform="translate(245,160)">
    <rect width="50" height="35" fill="#fff" stroke="#dc2626" rx="3"/>
    <text x="25" y="24" font-size="16" text-anchor="middle">⚙️</text>
  </g>
  <g transform="translate(180,205)">
    <rect width="50" height="35" fill="#fff" stroke="#dc2626" rx="3"/>
    <text x="25" y="24" font-size="16" text-anchor="middle">🗄️</text>
  </g>
  <g transform="translate(245,205)">
    <rect width="50" height="35" fill="#fff" stroke="#dc2626" rx="3"/>
    <text x="25" y="24" font-size="16" text-anchor="middle">🔧</text>
  </g>
  <text x="245" y="265" font-size="10" text-anchor="middle" fill="#991b1b" font-style="italic">"accesso a tutto"</text>

  <!-- Divider -->
  <line x1="350" y1="60" x2="350" y2="360" stroke="#cbd5e1" stroke-width="2" stroke-dasharray="4,4"/>

  <!-- ZTNA side (right) -->
  <text x="525" y="60" font-size="13" font-weight="700" text-anchor="middle" fill="#166534">ZTNA</text>
  <g transform="translate(385,90)">
    <rect width="40" height="40" rx="5" fill="#dcfce7" stroke="#22c55e"/>
    <text x="20" y="27" font-size="20" text-anchor="middle">👤</text>
    <text x="20" y="55" font-size="9" text-anchor="middle">Utente</text>
  </g>
  <!-- Verification gateway -->
  <g transform="translate(450,93)">
    <rect width="50" height="35" fill="#fef3c7" stroke="#f59e0b" rx="4"/>
    <text x="25" y="20" font-size="9" text-anchor="middle" fill="#78350f">Policy</text>
    <text x="25" y="30" font-size="9" text-anchor="middle" fill="#78350f">Engine</text>
  </g>
  <path d="M 430 110 L 450 110" stroke="#22c55e" stroke-width="2"/>
  <path d="M 500 110 L 540 110" stroke="#22c55e" stroke-width="2" stroke-dasharray="3,2"/>
  <text x="525" y="105" font-size="8" text-anchor="middle" fill="#15803d">verifica</text>

  <!-- Granular access: single tunnel to one resource -->
  <g transform="translate(560,90)">
    <rect width="50" height="40" fill="#fff" stroke="#22c55e" stroke-width="2" rx="3"/>
    <text x="25" y="27" font-size="18" text-anchor="middle">📁</text>
  </g>
  <text x="585" y="148" font-size="9" text-anchor="middle" fill="#15803d">✓ autorizzata</text>

  <!-- Hidden resources -->
  <g transform="translate(390,180)" opacity="0.25">
    <rect width="220" height="120" fill="#f3f4f6" stroke="#9ca3af" rx="6" stroke-dasharray="3,3"/>
    <text x="110" y="20" font-size="10" font-weight="600" text-anchor="middle" fill="#6b7280">risorse nascoste ("dark")</text>
    <g transform="translate(15,30)">
      <rect width="40" height="30" fill="#fff" stroke="#9ca3af" rx="2"/>
      <text x="20" y="21" font-size="14" text-anchor="middle">🖥️</text>
    </g>
    <g transform="translate(70,30)">
      <rect width="40" height="30" fill="#fff" stroke="#9ca3af" rx="2"/>
      <text x="20" y="21" font-size="14" text-anchor="middle">💾</text>
    </g>
    <g transform="translate(125,30)">
      <rect width="40" height="30" fill="#fff" stroke="#9ca3af" rx="2"/>
      <text x="20" y="21" font-size="14" text-anchor="middle">⚙️</text>
    </g>
    <g transform="translate(180,30)">
      <rect width="40" height="30" fill="#fff" stroke="#9ca3af" rx="2"/>
      <text x="20" y="21" font-size="14" text-anchor="middle">🗄️</text>
    </g>
    <g transform="translate(15,75)">
      <rect width="40" height="30" fill="#fff" stroke="#9ca3af" rx="2"/>
      <text x="20" y="21" font-size="14" text-anchor="middle">🔧</text>
    </g>
    <g transform="translate(70,75)">
      <rect width="40" height="30" fill="#fff" stroke="#9ca3af" rx="2"/>
      <text x="20" y="21" font-size="14" text-anchor="middle">📊</text>
    </g>
  </g>
  <text x="500" y="325" font-size="10" text-anchor="middle" fill="#15803d" font-style="italic">"vedi solo ciò a cui sei autorizzato"</text>
</svg>

### Caratteristiche chiave

- **Accesso application-level**, non network-level
- **Identità + contesto** = chiave d'accesso (chi sei, da dove, con quale device, in che condizioni)
- **Dark cloud / hidden infrastructure**: le risorse non sono pubblicamente visibili
- **Verifica continua**: re-auth periodica, posture check dinamici
- **Microsegmentazione** automatica

### Due architetture ZTNA

| Tipo | Come funziona | Esempi vendor |
|---|---|---|
| **Service-initiated** | Connettore parte dalla rete privata verso un broker cloud; utente accede tramite browser/app | Zscaler ZPA, Cloudflare Access, Twingate |
| **Endpoint-initiated** | Agent sul device crea tunnel diretti P2P (spesso WireGuard) | **NetBird**, Tailscale, Cisco Duo Network Gateway |

---

## 4. SSE — Security Service Edge

Termine coniato da **Gartner nel 2021**. È la **suite di sicurezza cloud-delivered** che protegge accesso a Internet, SaaS e app private.

> SSE = **ZTNA + SWG + CASB + FWaaS + DLP** (e altri componenti)

<svg viewBox="0 0 700 380" xmlns="http://www.w3.org/2000/svg" style="max-width:100%;height:auto;font-family:system-ui,sans-serif">
  <rect width="700" height="380" fill="#f8fafc"/>
  <text x="350" y="30" font-size="16" font-weight="700" text-anchor="middle" fill="#1e293b">Architettura SSE — i componenti</text>

  <!-- Central cloud -->
  <ellipse cx="350" cy="200" rx="180" ry="100" fill="#dbeafe" stroke="#3b82f6" stroke-width="2.5"/>
  <text x="350" y="170" font-size="18" font-weight="700" text-anchor="middle" fill="#1e3a8a">SSE Platform</text>
  <text x="350" y="190" font-size="11" text-anchor="middle" fill="#1e40af">cloud-delivered</text>
  <text x="350" y="208" font-size="11" text-anchor="middle" fill="#1e40af">security stack</text>
  <text x="350" y="235" font-size="10" text-anchor="middle" fill="#475569" font-style="italic">erogata dai PoP globali</text>

  <!-- Components around -->
  <g transform="translate(80,90)">
    <rect width="110" height="55" fill="#fef3c7" stroke="#f59e0b" stroke-width="2" rx="6"/>
    <text x="55" y="22" font-size="13" font-weight="700" text-anchor="middle" fill="#78350f">ZTNA</text>
    <text x="55" y="40" font-size="9" text-anchor="middle" fill="#78350f">accesso app private</text>
  </g>
  <line x1="190" y1="118" x2="240" y2="155" stroke="#94a3b8" stroke-width="1.5"/>

  <g transform="translate(510,90)">
    <rect width="110" height="55" fill="#fce7f3" stroke="#ec4899" stroke-width="2" rx="6"/>
    <text x="55" y="22" font-size="13" font-weight="700" text-anchor="middle" fill="#831843">SWG</text>
    <text x="55" y="40" font-size="9" text-anchor="middle" fill="#831843">filtro web / proxy</text>
  </g>
  <line x1="510" y1="118" x2="460" y2="155" stroke="#94a3b8" stroke-width="1.5"/>

  <g transform="translate(40,200)">
    <rect width="110" height="55" fill="#dcfce7" stroke="#22c55e" stroke-width="2" rx="6"/>
    <text x="55" y="22" font-size="13" font-weight="700" text-anchor="middle" fill="#14532d">CASB</text>
    <text x="55" y="40" font-size="9" text-anchor="middle" fill="#14532d">controllo SaaS</text>
  </g>
  <line x1="150" y1="225" x2="190" y2="210" stroke="#94a3b8" stroke-width="1.5"/>

  <g transform="translate(550,200)">
    <rect width="110" height="55" fill="#ede9fe" stroke="#8b5cf6" stroke-width="2" rx="6"/>
    <text x="55" y="22" font-size="13" font-weight="700" text-anchor="middle" fill="#4c1d95">FWaaS</text>
    <text x="55" y="40" font-size="9" text-anchor="middle" fill="#4c1d95">firewall cloud</text>
  </g>
  <line x1="550" y1="225" x2="510" y2="210" stroke="#94a3b8" stroke-width="1.5"/>

  <g transform="translate(150,310)">
    <rect width="110" height="50" fill="#fef2f2" stroke="#dc2626" stroke-width="2" rx="6"/>
    <text x="55" y="22" font-size="13" font-weight="700" text-anchor="middle" fill="#7f1d1d">DLP</text>
    <text x="55" y="38" font-size="9" text-anchor="middle" fill="#7f1d1d">anti-esfiltrazione</text>
  </g>
  <line x1="260" y1="330" x2="290" y2="275" stroke="#94a3b8" stroke-width="1.5"/>

  <g transform="translate(440,310)">
    <rect width="110" height="50" fill="#e0f2fe" stroke="#0284c7" stroke-width="2" rx="6"/>
    <text x="55" y="22" font-size="13" font-weight="700" text-anchor="middle" fill="#0c4a6e">RBI</text>
    <text x="55" y="38" font-size="9" text-anchor="middle" fill="#0c4a6e">browser isolation</text>
  </g>
  <line x1="440" y1="330" x2="410" y2="275" stroke="#94a3b8" stroke-width="1.5"/>
</svg>

### I componenti SSE

| Componente | Cosa fa |
|---|---|
| **ZTNA** | Accesso Zero Trust ad app private (sostituisce VPN) |
| **SWG** (Secure Web Gateway) | Filtra traffico web in uscita, blocca siti malevoli, ispeziona TLS |
| **CASB** (Cloud Access Security Broker) | Visibilità e controllo su SaaS (Office 365, Salesforce, ecc.), shadow IT |
| **FWaaS** (Firewall as a Service) | Firewall erogato dal cloud, non da appliance |
| **DLP** (Data Loss Prevention) | Previene esfiltrazione dati sensibili |
| **RBI** (Remote Browser Isolation) | Esegue browser in sandbox cloud (anti-malware web) |
| **DEM** (Digital Experience Monitoring) | Monitora qualità esperienza utente |

### Vendor SSE principali

Zscaler (Zero Trust Exchange), Netskope, Palo Alto (Prisma Access), Cloudflare One, Cisco Umbrella+, Forcepoint, Skyhigh, iboss.

---

## 5. SASE — Secure Access Service Edge

Termine coniato da **Gartner nel 2019** (prima di SSE). È il framework più ampio: **converge networking + security** in un'unica piattaforma cloud-native distribuita su PoP globali.

> SASE = **SD-WAN + SSE**

<svg viewBox="0 0 700 380" xmlns="http://www.w3.org/2000/svg" style="max-width:100%;height:auto;font-family:system-ui,sans-serif">
  <rect width="700" height="380" fill="#f8fafc"/>
  <text x="350" y="28" font-size="16" font-weight="700" text-anchor="middle" fill="#1e293b">SASE = SD-WAN + SSE</text>

  <!-- Big outer SASE box -->
  <rect x="40" y="55" width="620" height="290" fill="#fef3c7" stroke="#f59e0b" stroke-width="3" rx="12" opacity="0.4"/>
  <text x="350" y="80" font-size="14" font-weight="700" text-anchor="middle" fill="#78350f">SASE Platform (cloud edge, PoP globali)</text>

  <!-- SD-WAN box -->
  <rect x="70" y="105" width="260" height="220" fill="#dbeafe" stroke="#2563eb" stroke-width="2.5" rx="10"/>
  <text x="200" y="130" font-size="14" font-weight="700" text-anchor="middle" fill="#1e3a8a">SD-WAN</text>
  <text x="200" y="148" font-size="10" text-anchor="middle" fill="#1e40af">(networking)</text>

  <g transform="translate(90,165)">
    <rect width="100" height="32" fill="#fff" stroke="#3b82f6" rx="4"/>
    <text x="50" y="20" font-size="10" font-weight="600" text-anchor="middle" fill="#1e40af">Path selection</text>
  </g>
  <g transform="translate(200,165)">
    <rect width="110" height="32" fill="#fff" stroke="#3b82f6" rx="4"/>
    <text x="55" y="20" font-size="10" font-weight="600" text-anchor="middle" fill="#1e40af">QoS / traffic shaping</text>
  </g>
  <g transform="translate(90,205)">
    <rect width="100" height="32" fill="#fff" stroke="#3b82f6" rx="4"/>
    <text x="50" y="20" font-size="10" font-weight="600" text-anchor="middle" fill="#1e40af">Link aggregation</text>
  </g>
  <g transform="translate(200,205)">
    <rect width="110" height="32" fill="#fff" stroke="#3b82f6" rx="4"/>
    <text x="55" y="20" font-size="10" font-weight="600" text-anchor="middle" fill="#1e40af">WAN optimization</text>
  </g>
  <g transform="translate(145,245)">
    <rect width="110" height="32" fill="#fff" stroke="#3b82f6" rx="4"/>
    <text x="55" y="20" font-size="10" font-weight="600" text-anchor="middle" fill="#1e40af">Multi-link failover</text>
  </g>
  <text x="200" y="305" font-size="10" text-anchor="middle" fill="#1e40af" font-style="italic">"trasporto intelligente"</text>

  <!-- Plus sign -->
  <circle cx="350" cy="215" r="18" fill="#fbbf24" stroke="#d97706" stroke-width="2"/>
  <text x="350" y="222" font-size="22" font-weight="700" text-anchor="middle" fill="#78350f">+</text>

  <!-- SSE box -->
  <rect x="370" y="105" width="260" height="220" fill="#fce7f3" stroke="#ec4899" stroke-width="2.5" rx="10"/>
  <text x="500" y="130" font-size="14" font-weight="700" text-anchor="middle" fill="#831843">SSE</text>
  <text x="500" y="148" font-size="10" text-anchor="middle" fill="#9d174d">(security)</text>

  <g transform="translate(390,165)">
    <rect width="70" height="32" fill="#fff" stroke="#ec4899" rx="4"/>
    <text x="35" y="20" font-size="10" font-weight="600" text-anchor="middle" fill="#831843">ZTNA</text>
  </g>
  <g transform="translate(470,165)">
    <rect width="70" height="32" fill="#fff" stroke="#ec4899" rx="4"/>
    <text x="35" y="20" font-size="10" font-weight="600" text-anchor="middle" fill="#831843">SWG</text>
  </g>
  <g transform="translate(550,165)">
    <rect width="70" height="32" fill="#fff" stroke="#ec4899" rx="4"/>
    <text x="35" y="20" font-size="10" font-weight="600" text-anchor="middle" fill="#831843">CASB</text>
  </g>
  <g transform="translate(390,205)">
    <rect width="70" height="32" fill="#fff" stroke="#ec4899" rx="4"/>
    <text x="35" y="20" font-size="10" font-weight="600" text-anchor="middle" fill="#831843">FWaaS</text>
  </g>
  <g transform="translate(470,205)">
    <rect width="70" height="32" fill="#fff" stroke="#ec4899" rx="4"/>
    <text x="35" y="20" font-size="10" font-weight="600" text-anchor="middle" fill="#831843">DLP</text>
  </g>
  <g transform="translate(550,205)">
    <rect width="70" height="32" fill="#fff" stroke="#ec4899" rx="4"/>
    <text x="35" y="20" font-size="10" font-weight="600" text-anchor="middle" fill="#831843">RBI</text>
  </g>
  <g transform="translate(445,245)">
    <rect width="120" height="32" fill="#fff" stroke="#ec4899" rx="4"/>
    <text x="60" y="20" font-size="10" font-weight="600" text-anchor="middle" fill="#831843">Threat prevention</text>
  </g>
  <text x="500" y="305" font-size="10" text-anchor="middle" fill="#831843" font-style="italic">"sicurezza convergente"</text>
</svg>

### Caratteristiche chiave SASE

- **Convergenza** networking + security in un unico vendor/piattaforma
- **Cloud-native**: niente appliance fisiche, tutto erogato da PoP globali
- **Identity-driven**: le policy seguono l'utente, non l'IP o la location
- **Edge-delivered**: la sicurezza è applicata al PoP più vicino all'utente
- **Single-pass architecture**: il traffico viene ispezionato una sola volta da tutti i moduli, in pipeline

### Vendor SASE single-vendor

Cato Networks, Versa Networks, Cloudflare, Netskope (con NewEdge), Palo Alto Prisma SASE, Fortinet FortiSASE.

---

## 6. La gerarchia: come si incastrano i modelli

<svg viewBox="0 0 700 320" xmlns="http://www.w3.org/2000/svg" style="max-width:100%;height:auto;font-family:system-ui,sans-serif">
  <rect width="700" height="320" fill="#f8fafc"/>
  <text x="350" y="30" font-size="15" font-weight="700" text-anchor="middle" fill="#1e293b">Le scatole cinesi: ZTNA ⊂ SSE ⊂ SASE</text>

  <!-- SASE outer -->
  <rect x="80" y="60" width="540" height="230" fill="#fef3c7" stroke="#d97706" stroke-width="3" rx="14"/>
  <text x="350" y="85" font-size="15" font-weight="700" text-anchor="middle" fill="#78350f">SASE</text>
  <text x="350" y="102" font-size="10" text-anchor="middle" fill="#78350f">networking + security convergenti</text>

  <!-- SD-WAN -->
  <rect x="105" y="125" width="160" height="145" fill="#dbeafe" stroke="#2563eb" stroke-width="2" rx="8"/>
  <text x="185" y="155" font-size="13" font-weight="700" text-anchor="middle" fill="#1e3a8a">SD-WAN</text>
  <text x="185" y="175" font-size="9" text-anchor="middle" fill="#1e40af">trasporto</text>
  <text x="185" y="200" font-size="20" text-anchor="middle">🌐</text>
  <text x="185" y="230" font-size="9" text-anchor="middle" fill="#1e40af">QoS · path selection</text>
  <text x="185" y="245" font-size="9" text-anchor="middle" fill="#1e40af">multi-link · failover</text>

  <!-- SSE box -->
  <rect x="285" y="125" width="320" height="145" fill="#fce7f3" stroke="#ec4899" stroke-width="2.5" rx="8"/>
  <text x="445" y="148" font-size="13" font-weight="700" text-anchor="middle" fill="#831843">SSE</text>
  <text x="445" y="163" font-size="9" text-anchor="middle" fill="#9d174d">security stack cloud</text>

  <!-- ZTNA inside SSE -->
  <rect x="305" y="180" width="100" height="75" fill="#dcfce7" stroke="#16a34a" stroke-width="2" rx="6"/>
  <text x="355" y="205" font-size="12" font-weight="700" text-anchor="middle" fill="#14532d">ZTNA</text>
  <text x="355" y="222" font-size="9" text-anchor="middle" fill="#15803d">access</text>
  <text x="355" y="245" font-size="18" text-anchor="middle">🔐</text>

  <!-- Other SSE components -->
  <g transform="translate(420,185)">
    <rect width="50" height="28" fill="#fff" stroke="#ec4899" rx="3"/>
    <text x="25" y="19" font-size="10" font-weight="600" text-anchor="middle" fill="#831843">SWG</text>
  </g>
  <g transform="translate(480,185)">
    <rect width="50" height="28" fill="#fff" stroke="#ec4899" rx="3"/>
    <text x="25" y="19" font-size="10" font-weight="600" text-anchor="middle" fill="#831843">CASB</text>
  </g>
  <g transform="translate(540,185)">
    <rect width="50" height="28" fill="#fff" stroke="#ec4899" rx="3"/>
    <text x="25" y="19" font-size="10" font-weight="600" text-anchor="middle" fill="#831843">FWaaS</text>
  </g>
  <g transform="translate(420,225)">
    <rect width="50" height="28" fill="#fff" stroke="#ec4899" rx="3"/>
    <text x="25" y="19" font-size="10" font-weight="600" text-anchor="middle" fill="#831843">DLP</text>
  </g>
  <g transform="translate(480,225)">
    <rect width="50" height="28" fill="#fff" stroke="#ec4899" rx="3"/>
    <text x="25" y="19" font-size="10" font-weight="600" text-anchor="middle" fill="#831843">RBI</text>
  </g>
  <g transform="translate(540,225)">
    <rect width="50" height="28" fill="#fff" stroke="#ec4899" rx="3"/>
    <text x="25" y="19" font-size="10" font-weight="600" text-anchor="middle" fill="#831843">DEM</text>
  </g>

  <text x="350" y="305" font-size="10" text-anchor="middle" fill="#475569" font-style="italic">ZTNA è un componente di SSE · SSE è la metà security di SASE</text>
</svg>

---

## 7. Confronto comparativo dei modelli

| Aspetto | VPN classica | ZTNA | SSE | SASE |
|---|---|---|---|---|
| **Modello fiducia** | Trusted (perimetrale) | Zero Trust | Zero Trust | Zero Trust |
| **Scope** | Tunnel di rete | Accesso applicazioni | Sicurezza complessiva | Sicurezza + Networking |
| **Granularità** | Network-level | App-level | App-level + traffico web/SaaS | Tutto il traffico aziendale |
| **Include networking?** | Sì (basic) | No | No | Sì (SD-WAN) |
| **Include QoS?** | No | No | No | Sì |
| **Delivery** | Appliance / agent | Cloud o self-hosted | Cloud-native | Cloud-native distribuito su PoP |
| **Sostituisce** | — | VPN | VPN + proxy + appliance security | VPN + MPLS + proxy + FW + tutto |
| **Anno termine** | anni '90 | ~2019 | 2021 | 2019 |
| **Esempio vendor** | Cisco AnyConnect, OpenVPN | NetBird, Tailscale, Twingate | Zscaler, Netskope | Cato, Cloudflare One, Prisma SASE |

---

## 8. Quale modello scegliere?

<svg viewBox="0 0 700 340" xmlns="http://www.w3.org/2000/svg" style="max-width:100%;height:auto;font-family:system-ui,sans-serif">
  <rect width="700" height="340" fill="#f8fafc"/>
  <text x="350" y="30" font-size="15" font-weight="700" text-anchor="middle" fill="#1e293b">Decision tree: quale soluzione fa per te?</text>

  <!-- Root -->
  <rect x="270" y="55" width="160" height="40" fill="#1e40af" rx="6"/>
  <text x="350" y="80" font-size="11" font-weight="600" text-anchor="middle" fill="#fff">Cosa devi risolvere?</text>

  <!-- Branch 1 -->
  <line x1="320" y1="95" x2="140" y2="135" stroke="#64748b" stroke-width="2"/>
  <rect x="40" y="135" width="200" height="60" fill="#dcfce7" stroke="#22c55e" stroke-width="2" rx="6"/>
  <text x="140" y="158" font-size="11" font-weight="700" text-anchor="middle" fill="#14532d">Sostituire la VPN</text>
  <text x="140" y="175" font-size="9" text-anchor="middle" fill="#15803d">accesso remoto sicuro</text>
  <text x="140" y="188" font-size="9" text-anchor="middle" fill="#15803d">a risorse aziendali</text>
  <rect x="50" y="215" width="180" height="32" fill="#fff" stroke="#22c55e" rx="4"/>
  <text x="140" y="235" font-size="11" font-weight="700" text-anchor="middle" fill="#14532d">→ ZTNA</text>
  <text x="140" y="265" font-size="9" text-anchor="middle" fill="#475569">NetBird, Tailscale,</text>
  <text x="140" y="278" font-size="9" text-anchor="middle" fill="#475569">Twingate, Cloudflare Access</text>

  <!-- Branch 2 -->
  <line x1="350" y1="95" x2="350" y2="135" stroke="#64748b" stroke-width="2"/>
  <rect x="250" y="135" width="200" height="60" fill="#fce7f3" stroke="#ec4899" stroke-width="2" rx="6"/>
  <text x="350" y="158" font-size="11" font-weight="700" text-anchor="middle" fill="#831843">Consolidare security</text>
  <text x="350" y="175" font-size="9" text-anchor="middle" fill="#9d174d">filtro web + SaaS + accesso</text>
  <text x="350" y="188" font-size="9" text-anchor="middle" fill="#9d174d">privato + DLP in un'unica piattaforma</text>
  <rect x="260" y="215" width="180" height="32" fill="#fff" stroke="#ec4899" rx="4"/>
  <text x="350" y="235" font-size="11" font-weight="700" text-anchor="middle" fill="#831843">→ SSE</text>
  <text x="350" y="265" font-size="9" text-anchor="middle" fill="#475569">Zscaler, Netskope,</text>
  <text x="350" y="278" font-size="9" text-anchor="middle" fill="#475569">Palo Alto Prisma Access</text>

  <!-- Branch 3 -->
  <line x1="380" y1="95" x2="560" y2="135" stroke="#64748b" stroke-width="2"/>
  <rect x="460" y="135" width="200" height="60" fill="#fef3c7" stroke="#f59e0b" stroke-width="2" rx="6"/>
  <text x="560" y="158" font-size="11" font-weight="700" text-anchor="middle" fill="#78350f">Convergere tutto</text>
  <text x="560" y="175" font-size="9" text-anchor="middle" fill="#92400e">networking (SD-WAN)</text>
  <text x="560" y="188" font-size="9" text-anchor="middle" fill="#92400e">+ security in unica piattaforma</text>
  <rect x="470" y="215" width="180" height="32" fill="#fff" stroke="#f59e0b" rx="4"/>
  <text x="560" y="235" font-size="11" font-weight="700" text-anchor="middle" fill="#78350f">→ SASE</text>
  <text x="560" y="265" font-size="9" text-anchor="middle" fill="#475569">Cato Networks, Cloudflare One,</text>
  <text x="560" y="278" font-size="9" text-anchor="middle" fill="#475569">Prisma SASE, FortiSASE</text>

  <!-- Bottom note -->
  <rect x="80" y="300" width="540" height="32" fill="#e0e7ff" stroke="#6366f1" stroke-width="1" rx="4"/>
  <text x="350" y="321" font-size="10" text-anchor="middle" fill="#3730a3">💡 Si può anche iniziare da ZTNA e crescere progressivamente verso SSE/SASE</text>
</svg>

### Linee guida pratiche

- **PMI / startup / team distribuiti** → ZTNA è quasi sempre la scelta giusta. Costa poco, si installa in minuti, sostituisce la VPN. Esempi: **NetBird** (open source, self-hostable), Tailscale, Twingate.

- **Aziende medio-grandi con molti utenti che vanno su SaaS** → SSE. Quando hai bisogno di proteggere non solo l'accesso a risorse private ma anche il traffico web in uscita, l'uso di Office 365/Salesforce/Dropbox, e prevenire esfiltrazione di dati.

- **Enterprise con molte sedi globali, traffico critico, esigenza di QoS** → SASE. Quando vuoi smantellare l'MPLS, convergere networking e security, e gestire migliaia di utenti distribuiti in tutto il mondo da un'unica console.

- **Approccio progressivo (consigliato)** → si parte da ZTNA per le esigenze immediate (sostituire VPN), poi si aggiungono progressivamente componenti SSE (SWG, CASB), e infine si converge in SASE se serve.

---

## 9. Cosa NON è in questi modelli

Per evitare confusioni terminologiche frequenti:

| ❌ Confusione | Realtà |
|---|---|
| "SASE = SD-WAN versione 2" | No. SD-WAN è solo il componente *networking* di SASE |
| "ZTNA = VPN cloud" | No. La VPN dà accesso di rete, ZTNA accesso per singola app |
| "Zero Trust = un prodotto" | No. È un modello concettuale, non una tecnologia |
| "SSE include SD-WAN" | No. SSE è solo security. SD-WAN + SSE = SASE |
| "MPLS è Zero Trust" | No. MPLS è una rete trusted/perimetrale classica |

---

## 10. Glossario rapido

- **Zero Trust** — modello di sicurezza basato su "never trust, always verify"
- **ZTNA** (Zero Trust Network Access) — accesso applicativo Zero Trust, sostituisce VPN
- **SSE** (Security Service Edge) — suite security cloud-delivered (ZTNA + SWG + CASB + FWaaS + ...)
- **SASE** (Secure Access Service Edge) — convergenza SD-WAN + SSE su PoP globali
- **SD-WAN** (Software-Defined WAN) — networking software-defined su link Internet multipli
- **SWG** (Secure Web Gateway) — proxy/filtro traffico web in uscita
- **CASB** (Cloud Access Security Broker) — visibilità e policy su SaaS
- **FWaaS** (Firewall as a Service) — firewall erogato dal cloud
- **DLP** (Data Loss Prevention) — prevenzione esfiltrazione dati
- **RBI** (Remote Browser Isolation) — esecuzione browser in sandbox cloud
- **MPLS** (Multiprotocol Label Switching) — servizio di trasporto carrier-grade legacy
- **NIST SP 800-207** — standard di riferimento per Zero Trust Architecture
- **PoP** (Point of Presence) — datacenter/edge node distribuito geograficamente
- **Posture check** — verifica delle condizioni di sicurezza di un dispositivo

---

## Riferimenti

- **NIST SP 800-207** — Zero Trust Architecture (2020)
- **Gartner** — *The Future of Network Security Is in the Cloud* (2019, definizione SASE)
- **Gartner** — *Magic Quadrant for Security Service Edge* (annuale, dal 2022)
- **Executive Order 14028** (USA, 2021) — Improving the Nation's Cybersecurity (mandato Zero Trust per agenzie federali)

---

*Dispensa sintetica · maggio 2026*
