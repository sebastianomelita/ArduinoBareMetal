>[Torna all'indice generale](index.md)

## **Modo C/C++**

Nella maniera tradizionale di gestire i progetti in C/C++ una possibilità è quella di creare librerie di funzioni ed, eventualmente, di variabili globali pubbliche organizzate in due file aventi lo stesso nome (quello che identifica la libreria) ma estensioni diverse:
- **mialibreria.cpp (o mialibreria.c)** che contiene la definizione del corpo delle funzioni pubbliche e la dichiarazione delle variabili globali pubbliche. 
- **mialibreria.h** che contiene la segnatura cioè i prototipi delle funzioni pubbliche e gli include di cui queste hanno bisogno.

I due file sono **collegati** tra loro inserendo, in cima a tutto, nel file ```mialibreria.cpp``` la riga  ```mialibreria.h```.

Inoltre, per evitare **inclusioni multiple** dello stesso file (fenomeno che causa spreco di spazio e potenziali errori) si fa in modo che nel file ```mialibreria.h``` tutto il codice sia racchiuso tra opportune direttive di compilazione come illustrato nell'esempio di seguito: 

```C++
#ifndef __COMMON_H__
#define __COMMON_H__

#include <WebServer.h>
#include <ESPmDNS.h>

void handleFavicon();
void handleNotFound();
void handleRoot();
#endif
```
La direttiva, prima di includere il file header, in fase di compilazione verifica se esso non è già stato incluso controllando una costante flag (bandierina) che viene definita l momento della prima inclusione.






>[Torna all'indice generale](index.md)
