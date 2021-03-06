

## **GESTIONE DEI TASK/THREAD**

### **Diagramma temporale dei task/thread**

<img src="suspending.gif" alt="alt text" width="700">

Facendo riferimento ai numeri nel diagramma sopra:

- A (1) l'attività 1 è in esecuzione.
- In (2) il kernel sospende (scambia) l'attività 1 ...
- ... e in (3) riprende l'attività 2.
- Mentre l'attività 2 è in esecuzione (4), blocca una periferica del processore per il proprio accesso esclusivo.
- In (5) il kernel sospende l'attività 2 ...
- ... e in (6) riprende l'attività 3.
- L'attività 3 tenta di accedere alla stessa periferica del processore, trovandola bloccata, l'attività 3 non può continuare, quindi si sospende in (7).
- In (8) il kernel riprende l'attività 1.
- Eccetera.
- La prossima volta che l'attività 2 viene eseguita (9), termina con la periferica del processore e la sblocca.
- La prossima volta che l'attività 3 viene eseguita (10) scopre che ora può accedere alla periferica del processore e questa volta viene eseguita fino alla sospensione dal kernel.

### **Diagramma di stato dei task/thread**

Un'attività può esistere in uno dei seguenti stati:

- **Esecuzione (running)**
Quando un'attività è effettivamente in esecuzione, si dice che si trova nello stato In esecuzione. Attualmente sta utilizzando il processore. Se il processore su cui è in esecuzione RTOS ha un solo core, può esserci solo un'attività nello stato In esecuzione alla volta.
- **Pronto (ready)**
Le attività pronte sono quelle che possono essere eseguite (non sono nello stato Bloccato o Sospeso) ma non sono attualmente in esecuzione perché un'attività diversa con priorità uguale o superiore è già nello stato In esecuzione.
- **Bloccato (blocked)**
Si dice che un'attività si trova nello stato Bloccato se è attualmente in attesa di un evento temporale o esterno. Ad esempio, se un'attività chiama vTaskDelay() si bloccherà (verrà posta nello stato Bloccato) fino alla scadenza del periodo di ritardo, un evento temporale. Le attività possono anche bloccare l'attesa per la coda, il semaforo, il gruppo di eventi, la notifica o l'evento del semaforo. Le attività nello stato Bloccato normalmente hanno un periodo di "timeout", dopo il quale l'attività andrà in timeout e verrà sbloccata, anche se l'evento che l'attività stava aspettando non si è verificato.
Le attività nello stato Bloccato non utilizzano alcun tempo di elaborazione e non possono essere selezionate per entrare nello stato In esecuzione.
- **Sospeso (suspended)**
Come le attività che si trovano nello stato Bloccato, le attività nello stato Sospeso non possono essere selezionate per entrare nello stato In esecuzione, ma le attività nello stato Sospeso non hanno un timeout. Al contrario, le attività entrano o escono dallo stato Sospeso solo quando esplicitamente richiesto tramite le chiamate API vTaskSuspend() e xTaskResume() rispettivamente.


<img src="5-Figure1-1.png" alt="alt text" width="700">



### **Sitografia**:
- https://www.freertos.org/implementation/a00005.html
- https://www.freertos.org/RTOS-task-states.html
- https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/pthread.html
- http://www.cs.fsu.edu/~baker/opsys/notes/threads.html
- https://faculty.cc.gatech.edu/~rama/CS2200-External/projects/p4/prj4.html
- https://www.embedded.com/effective-use-of-pthreads-in-embedded-linux-designs-part-1-the-multitasking-paradigm/
- https://superuser.com/questions/1068315/thread-states-linux-vs-windows
- https://docs.espressif.com/projects/esp-idf/en/v4.3-beta2/esp32/api-reference/system/esp_pthread.html
