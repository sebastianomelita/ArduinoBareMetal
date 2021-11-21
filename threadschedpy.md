>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  >[Versione in C++](threadsched.md)

# **SCHEDULAZIONE CON I THREAD**

I thread, detti anche processi leggeri, sono dei **flussi di esecuzione** separati da quello principale (il programma main) che procedono **indipendentemente** l'uno dall'altro e soprattutto in maniera **parallela** cioè **contemporaneamente** l'uno con l'altro. Il **parallelismo** può essere:
- **reale** se flussi di esecuzione diversi sono eseguiti da core (o CPU) diversi. Possiede la proprietà di effettiva **simultaneità** nell'esecuzione di più istruzioni.
- **emulato** se flussi di esecuzione diversi sono eseguiti dallo stesso core della stessa CPU. La proprietà di **simultaneità** è relativa all'esecuzione di più **programmi** nello stesso momento ma con **istruzioni** dell'uno e dell'altro eseguite in momenti diversi (tecnica dell'interleaving).

Normalmente una **istruzione delay(x)** fa attendere per x secondi non solo l'esecuzione di un certo task ma anche quella di tutti gli altri che quindi, in quel frattempo, sono bloccati. Il motivo risiede nel fatto che tutti i task condividono **il medesimo flusso** di esecuzione (o thread) e, se questo viene fermato, viene fermato per tutti i task.

Se però due o più task vengono eseguiti su thread differenti è possibile **bloccarne soltanto uno** con un delay impedendo temporaneamente ad uno dei suoi task di andare avanti, ma **lasciando liberi tutti gli altri** task sugli altri thread di proseguire la loro esecuzione. Questo perchè thread differenti sono assimilabili a **flussi di esecuzione** differenti eseguiti su CPU (logiche) differenti. In realtà le diverse CPU logiche condividono un'unica CPU fisica.

Avere più **flussi di esecuzione paralleli** fornisce quindi il **vantaggio** di poter realizzare gli algoritmi in **maniera lineare** suddividendoli in **fasi successive** la cui **tempistica** può essere stabilita in **maniera semplice** ed intuitiva impostando dei **ritardi**, cioè dei delay, tra una fase e l'altra. La **separazione** dei flussi permette una **progettazione indipendente** degli algoritmi eccetto che per i **dati comuni** a più flussi (thread), per i quali deve essere **sincronizzato l'accesso** con opportuni meccanismi. 

Ma come è possibile che thread diversi possano essere mandati in esecuzione contemporaneamente su un'unica CPU fisica?

In realtà ad essere eseguiti **contemporaneamente** sono soltanto i **task**, cioè i programmi ed i relativi algoritmi, le **istruzioni** in linguaggio macchina che li compongono vengono invece eseguite **a turno**, un blocco di istruzioni alla volta. La durata del turno viene detta **quanto di tempo**. Terminato il quanto di tempo di un thread si passa ad eseguire le istruzioni di un altro thread nel quanto di tempo successivo. Ciò accade a patto che i thread siano **"preemptive"** cioè supportino il **prerilascio** della risorsa CPU **prima** del termine naturale del task (il comando return o il completamento del task). Le istruzioni accorpate in un quanto potrebbero non coincidere esattamente con un multiplo intero delle istruzioni ad alto livello, ci potrebbe essere, ad esempio, metà di un'assegnazione in un quanto e l'altra metà nell'altro. Le istruzioni **atomiche**, cioè indivisibili, sono soltanto quelle in **linguaggio macchina**.

Normalmente i thread possono lavorare in due **modalità operative**:
- **prempitive o competitiva**. Se un task di un thread possiede una risorsa (ad esempio la CPU) ed è bloccato in attesa di un input o di un un delay(), viene marcato come **interrompibile** in **maniera trasparente al task**, senza che questo se ne accorga. Allo scadere di un timer HW, **il task** viene interrotto da un segnale di **interrupt** che blocca il flusso di esecuzione corrente e assegna la risorsa CPU ad **un altro thread** tra quelli che, in quel momento, aspettano di andare in esecuzione (**stato ready**).
- **"cooperative" cioè cooperativa**. Se un task di un thread possiede una risorsa (ad esempio la CPU) ed è bloccato in attesa di un input o di un una qualsiasi altra risorsa, esso stesso, con un comando **dello schedulatore** inserito **nel codice del task** (ad es. yeld()), decide **spontaneamente** di cedere il controllo della CPU allo schedulatore.  Lo **schedulatore** assegna la risorsa CPU ad **un altro thread** tra quelli che, in quel momento, aspettano di andare in esecuzione (**stato ready**).

Abbiamo visto che **usare i delay** per progettare i tempi di un task **è più semplice** perchè la programmazione rimane quella **lineare** a cui è solito ricorrere un programmatore per pensare gli algoritmi ma, in questo caso, **è anche molto meno costosa** in termini di efficienza che in un programma a singolo thread dato che la CPU può sempre servire tutti i task nello stesso momento (in maniera reale o simulata). 

Può capitare, specie in **dispositivi con risorse HW molto ristrette**, che i thread non siano interrompibili ma possano lavorare solo in modo "cooperativo", allora, in questo caso, alcune librerie forniscono una funzione delay() alternativa che contiene al suo interno il comando yeld() di rilascio "spontaneo" della CPU. In questo modo **è preservata**, anche su quei dispositivi, la possibilità di adottare uno stile di **programmazione sequenziale** degli algoritmi.

Un **altro vantaggio** per il programmatore è che la gestione della schedulazione è trasparente alla applicazione che deve essere schedulata, nel senso che l'algoritmo che deve gestire la schedulazione non deve essere incluso nel codice dell'applicazione. Qualcuno deve comunque gestire la schedulazione nel tempo dei thread e questo qualcuno è un **modulo SW** diverso dall'applicazione che può:
- essere fornito da una **libreria** apposita
- essere incluso in framework di terze parti (**middleware**)
- essere fornito dal **sistema operativo** presente sulla macchina

Il **costo da pagare** è una certa dose di **inefficienza residua** perchè la shedulazione dei thread può essere fatta in maniera più o meno sofisticata ma comunque richiede l'utilizzo di un certo ammontare della risorsa CPU. Il peso di queste inefficienze potrebbe diventare insostenibile in presenza di **molti task** che girano su sistemi con limitate risorse di calcolo, come sono tipicamente i **sistemi embedded**.

Riassumendo, la **schedulazione mediante thread** comporta:
- **vantaggio**.  Maggiore semplicità nella progettazione dei programmi, grazie alla possibilità di utilizzare uno **stile lineare** di programmazione.
- **vantaggio**. Maggiore semplicità nella progettazione dei programmi che non devono realizzare la logica della schedulazione dei task ma solo quella interna al singolo task.
- **svantaggio**. Minore efficienza nell'uso della risorsa CPU che deve comunque eseguire un thread a parte di gestione delle schedulazioni sugli altri thread.
- **svantaggio**. Bisogna conoscere le API di libreria con cui viene realizzato il multithreading in un certo sistema con un certo linguaggio specifico. Le maniere possono essere parecchie per cui potrebbe essere utile ricorrere ad interfacce standard consolidate (ad esempio Thread POSIX)
- **svantaggio**. la schedulazione, cioè il passaggio da un thread all'altro, è comandata dallo schedulatore secondo una tempistica che, di base, **non è governata dal programma principale**, per effetto di ciò potrebbe diventare molto probematica la gestione delle **risorse condivise**  tra un thread e l'altro. Cosa accade se due istruzioni di due task differenti scrivono contemporaneamente sullo stessa variabile globale? E se modificano contemporaneamente lo stesso registro di una periferica? 

L'ultimo svantaggio è **particolarmente critico** e può comportare l'introduzione di errori difficilmente rilevabili, anche dopo innumerevoli prove sistematiche. La progettazione della gestione delle **risorse condivise**, e della gestione della **comunicazione tra i thread** in generale, deve essere molto accurata e ben ponderata. Vari strumenti SW e metodologie ad hoc permettono di affrontare più o meno efficacemente il problema.

Esempio di realizzazione di due task che eseguono un blink mediante delay() insieme ad altre generiche operazioni svolte nel main (piattaforma **Espress if ESP32**, **IDE Arduino** e librerie thread **preemptive**):

```Python
#
# example ESP32 multitasking
# phil van allen
#
# thanks to https://youtu.be/iyoS9aSiDWg
#
import _thread as th
import time
from machine import Pin

blink1_running = True
blink2_running = True

led1 = Pin(4, Pin.OUT)
led2 = Pin(13, Pin.OUT)

def blink1(delay):
     while blink1_running:
         led1.value(not led1.value())
         time.sleep(delay)
     led1.value(0)

def blink2(delay):
     while blink2_running:
         led2.value(not led2.value())
         time.sleep(delay)
     led2.value(0)

print("Starting other tasks...")
th.start_new_thread(blink1, (0.5,))
th.start_new_thread(blink2, (0.25,))

count = 0
while True:
     print("Doing stuff... " + str(count))
     count += 1
     if count >= 10:
  	     break
     time.sleep(1)

print("Ending threads...")
blink1_running = False
blink2_running = False
```

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  >[Versione in C++](threadsched.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbMTEwNDI2NDY0N119
-->
