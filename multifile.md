>[Torna all'indice generale](index.md)

## **Modo C/C++**

Nella maniera tradizionale di gestire i progetti in C/C++ una possibilità è quella di creare librerie di funzioni ed, eventualmente, di variabili globali pubbliche organizzate in due file aventi lo stesso nome (quello che identifica la libreria) ma estensioni diverse:
- **mialibreria.cpp (o mialibreria.c)** che contiene la definizione del corpo delle funzioni pubbliche e la dichiarazione delle variabili globali pubbliche. 
- **mialibreria.h** che contiene la segnatura cioè i prototipi delle funzioni pubbliche e gli include di cui queste hanno bisogno.

I due file sono **collegati** tra loro inserendo, in cima a tutto, nel file ```mialibreria.cpp``` la riga  ```mialibreria.h```.
nel file principale del progetto, ad esempio ```mioprogetto.ino``` in cima a tutto va incluso (comunque prima di utilizzare le variabili e le funzioni incluse) va inserita la direttiva di inclusione #include ```"mialibreria.h"```.

Inoltre, per evitare **inclusioni multiple** dello stesso file (fenomeno che causa spreco di spazio e potenziali errori) si fa in modo che nel file ```mialibreria.h``` tutto il codice sia racchiuso tra opportune **direttive** di compilazione (istruzioni per il compilatore) come illustrato nell'esempio di seguito: 

```C++
# webmanager.h

#ifndef __WEBMANAGER_H__
#define __WEBMANAGER_H__

#include <WebServer.h>
#include <ESPmDNS.h>

void handleFavicon();
void handleNotFound();
void handleRoot();
#endif
```
Una opportuna **direttiva**, prima di includere il file header, in fase di compilazione verifica se esso non è già stato incluso controllando una **costante flag** (bandierina) che viene definita al momento della prima inclusione.

Di seguito è riportato il file delle definizioni corrispondente al file delle intestazioni (header file) precedente:

```C++
# webmanager.cpp

#include "webmanager.h"
WebServer webserver(80);

const char favicon[] = {
  0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x00, 0x00, 0x01, 0x00,
  0x18, 0x00, 0x68, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void handleFavicon(){
	webserver.sendHeader("Cache-Control", "max-age=31536000");
	webserver.send_P(200, "image/x-icon", favicon, sizeof(favicon));
}

void handleNotFound() {
  String message = "File webserverNot Found\n\n";
  message += "URI: ";
  message += webserver.uri();
  message += "\nMethod: ";
  message += (webserver.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webserver.args();
  message += "\n";
  for (uint8_t i = 0; i < webserver.args(); i++) {
    message += " " + webserver.argName(i) + ": " + webserver.arg(i) + "\n";
  }
  webserver.send(404, "text/plain", message);
}

void handleRoot() {
  webserver.send(200, "text/plain", "hello from esp32!");
}
```
Si noti che il file definisce e rende **pubbliche** (cioè **globali**), cioè accessibili al file ```mioprogetto.ino``` e a tutti gli altri file di definizione di librerie di funzioni (quelli con estensione .c o .c++):
- le sue **variabili globali**, nell'esempio l'array ```favicon``` e l'oggetto ```webserver```
- le sue **funzioni**, nell'esempio ```handleFavicon()```, ```handleNotFound()```, ```handleRoot()```.

Nel file ```mioprogetto.ino``` verrà inserita la riga #include ```"mialibreria.h"```.

Si noti che l'inclusione, così come è stata realizzata, avviene in un **solo verso**, dal file della libreria a quello principale del progetto, cioè dal file ```mialibreria.cpp``` a quello in cui è incluso il file header corrispondente, cioè ```mialibreria.h```. Nel nostro caso, quindi, le funzioni e le variabili globali verranno esportate dal file ```mialibreria.cpp``` al file ```mioprogetto.ino```. Il risultato è evidentemente quello voluto nel momento che siamo interessati ad utilizzare nel file principale del progetto, ```mioprogetto.ino```, le **variabili globali** e le **funzioni** definite nella libreria .

Se volessimo uno scambio di valori nel verso opposto, ad esempio esportare nel file della libreria una variabile globale definita nel file del progetto una maniera è riorrere ad una istruzione di **assegnazione** che modfichi il valore di una variabile globale **dichiarata nel file della libreria** con il valore di una variabile istanziata **nel file del progetto**. Il **posto giusto** per effettuare questa assegnazione è la funzione di ```**setup()**``` del ```loop()``` principale. Può avvenire in maniera esplicita all'interno del file principale del progetto o all'interno di una funzione di inizializzazione della libreria (definita a sua volta nella libreria.

```C++
# webmanager.h

#ifndef __WEBMANAGER_H__
#define __WEBMANAGER_H__

#include <WebServer.h>
#include <ESPmDNS.h>

void initCommon(WebServer *);
void handleFavicon();
void handleNotFound();
void handleRoot();
#endif
```


```C++
# webmanager.cpp
#include "webmanager.h"
#define webserver (*serveru)

ESP8266WebServer *serveru; 

const char favicon[] = {
  0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x10, 0x00, 0x00, 0x01, 0x00,
  0x18, 0x00, 0x68, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void initWebmanager(WebServer *serveri){
	serveru=serveri; 
}

void handleFavicon(){
	webserver.sendHeader("Cache-Control", "max-age=31536000");
	webserver.send_P(200, "image/x-icon", favicon, sizeof(favicon));
}

void handleNotFound() {
  String message = "File webserverNot Found\n\n";
  message += "URI: ";
  message += webserver.uri();
  message += "\nMethod: ";
  message += (webserver.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webserver.args();
  message += "\n";
  for (uint8_t i = 0; i < webserver.args(); i++) {
    message += " " + webserver.argName(i) + ": " + webserver.arg(i) + "\n";
  }
  webserver.send(404, "text/plain", message);
}

void handleRoot() {
  webserver.send(200, "text/plain", "hello from esp32!");
}
```

```C++
#include "webmanager.h"
WebServer webserver(80);

void setup(){
initWebmanager(&webserver);
.....
}
```C

>[Torna all'indice generale](index.md)
