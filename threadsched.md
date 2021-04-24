
>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  >[Versione in Python](threadschedpy.md)
>
# **SCHEDULAZIONE CON I THREAD**

I thread, detti anche processi leggeri, sono dei **flussi di esecuzione** separati da quello principale (il programma main) che procedono **indipendentemente** l'uno dall'altro e soprattutto in maniera **paralllela** cioè **contemporaneamente** l'uno con l'altro. Il **parallelismo** può essere:
- **reale** se flussi di esecuzione diversi sono eseguiti da core (o CPU) diversi. Possiede la proprietà di effettiva **simultaneità** nell'esezuzione di più istruzioni.
- **emulato** se flussi di esecuzione diversi sono eseguiti dallo stesso core della stessa CPU. La proprietà di **simultaneità** è relativa all'esezuzione di più **programmi** nello stesso momento ma con **istruzioni** dell'uno e dell'altro eseguite in momenti diversi (tenica dell'interleaving).

Normalmente una **istruzione delay(x)** fa attendere per x secondi non solo l'esecuzione di un certo task ma anche quella di tutti gli altri che quindi, in quel frattempo, sono bloccati. Il motivo risiede nel fatto che tutti i task condividono **il medesimo flusso** di esecuzione (o thread) e, se questo viene fermato, viene fermato per tutti i task.

Se però due o più task vengono eseguiti su thread differenti è possibile **bloccarne uno** con un delay impedendo temporaneamente ad un suo task di andare avanti, ma **laciando liberi tutti gli altri** task sugli altri thread di proseguire la loro esecuzione senza interruzioni di sorta. 

Abbiamo visto che usare i delay per progettare i tempi di un task **è più semplice** perchè la programmazione rimane quella lineare a cui è solito ricorrere un programmatore per pensare gli algoritmi ma, in questo caso, **è anche molto meno costosa** in termini di efficienza che in un programma a singolo thread dato che la CPU può sempre servire tutti i task nello stesso momento (in maniera reale o simulata). 

Un **altro vantaggio** per il programmatore è che la gestione della schedulazione è trasparente alla applicazione che sta realizzando nel senso che l'algoritmo che deve gestirla non deve essere incluso nel codice dell'applicazione. Qualcuno deve comunque gestire la schedulazione nel tempo dei thread e questo qualcuno è un **modulo SW** diverso dall'applicazione che può:
- essere fornito da una **libreria** apposita
- essere incluso in framework di terze parti (**middleware**)
- essere fornito dal **sistema operativo** presente sulla macchina

Il **costo da pagare** è una certa dose di **inefficienza residua** perchè la shedulazione dei thread può essere fatta in maniera più o meno sofisticata ma comunque richiede l'utilizzo di un certo ammontare della risorsa CPU. Il peso di queste inefficienze potrebbe diventare insostenibile in presenza di **molti task** che girano su sistemi con limitate risorse di calcolo, come sono tipicamente i **sistemi embedded**.

Riassumendo, la **schedulazione mediante thread** comporta:
- **vantaggio**.  Maggiore semplicità nella progettazione dei programmi, grazie alla possibilità di utilizzare uno stile lineare di programmazione.
- **vantaggio**. Maggiore semplicità nella progettazione dei programmi che non devono realizzare la logica della schedulazione dei task ma solo quella interna al singolo task.
- **svantaggio**. Maggiore inefficienza nell'uso della risorsa CPU che deve comunque eseguire un thread a parte di gestione delle schedulazioni sugli altri thread.
- **svantaggio**. Bisogna conoscere le API di libreria con cui viene realizzato il multithreading in un certo sistema con un certo linguaggio specifico. Le maniere possono essere parecchie per cui potrebbe essere utile ricorrere ad interfacce standard consolidate (ad esempio Thread POSIX)
- **svantaggio**. la schedulazione, cioè il passaggio da un thread all'altro, è comandata dallo schedulatore secondo una tempistica che, di base, **non è governata dal programma principale**, per effetto di ciò potrebbe diventare molto probematica la gestione delle **risorse condivise**  tra un thread e l'altro. Cosa accade se due istruzioni di due task differenti scrivono contemporaneamente sullo stessa variabile globale? E se modificano contemporaneamente lo stesso registro di una periferica? 

L'ultimo svantaggio è **particolarmente critico** e può comportare l'introduzione di errori difficilmente rilevabili, anche dopo innumerevoli prove sistematiche. La progettazione della gestione delle **risorse condivise**, e della gestione della **comunicazione tra i thread** in generale, deve essere molto accurata e ben ponderata. Vari strumenti SW e metodologie ad hoc permettono di affrontare più o meno efficacemente il problema.

Esempio di realizzazione di due task che eseguono un blink mediante delay() insieme ad altre generiche operazioni svolte nel main:

```C++
#include <pthread.h>

pthread_t t1;
pthread_t t2;
int delay ;
bool blink1_running = true;
bool blink2_running = true;
led1 = 13;
led2 = 12;

void * blink1(void * d)
{
    int time;
    time = (int) d;
    while(blink1_running){
	digitalWrite(led1, !digitalRead(led1));
	delay(time);
    }
    digitalWrite(led1, LOW);
    return NULL;
}

void * blink2(void * d)
{
    int time;
    time = (int) d;
    while(blink2_running){
	digitalWrite(led2, !digitalRead(led2));
	delay(time);
    }
    digitalWrite(led2, LOW);
    return NULL;
}

void setup() {
  Serial.begin(115200);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  delay = 500;
  if (pthread_create(&t1, NULL, blink1, (void *)delay)) {
         Serial.println("Errore crezione thread 1");
  }
  delay = 1000;
  if (pthread_create(&t2, NULL, blink2, (void *)delay)) {
         Serial.println("Errore crezione thread 2");
  } 
}

void loop() {
	int count = 0
	while(true){
		print("Doing stuff... " + str(count))
		Serial.print("Doing stuff... ");
		Serial.println(count);
		count += 1;
		if(count >= 10){
			break
		delay(1000);
	}
	Serial.print("Ending threads...");
	blink1_running = False
	blink2_running = False
}
```

>[Torna all'indice generazione tempi](indexgenerazionetempi.md)  >[Versione in Python](threadschedpy.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbMTEwNDI2NDY0N119
-->
