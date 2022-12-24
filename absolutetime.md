>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in Python](absolutetimepy.md)

## **MISURE DI TEMPO ASSOLUTE**

Per ottenere una cadenza periodica precisa è necessario usare una forma diversa dal solito schedulatore più adatta a **cumulare** con precisione lunghe misure di tempo. E’essenziale che l’accumulatore tass venga aggiornato **esattamente** con il **tempo campionato**. L’accumulatore unisce i vari campionamenti per ottenere una misura unica. Con questa forma ci si può aspettare un errore di qualche secondo all’ora dipendente solo dall’imprecisione dell’oscillatore.
```C++
unsigned long tass = 0;
if ((millis() - tass) >= periodo)
{
	tass += periodo;
	....
}
```
Invece la forma seguente è **errata**. La condizione viene valutata **in ritardo** rispetto al **momento ideale**, “reimpostando” la variabile ‘tass” al tempo attuale, questo ritardo si aggiunge a tutti i ritardi precedenti. Con questa forma ci si può aspettare un errore di diversi secondi al minuto o anche peggiore.
```C++
if ((millis() - tass) >= periodo)
{
	tass = millis();
	....
}
```
>[Torna all'indice generazione tempi](indexgenerazionetempi.md)       >[Versione in Python](absolutetimepy.md)
