Analogamente si potrebbe eseguire un blocco di codice continuamente solo all’interno di un tempo massimo prestabilito, cioè solo prima dello scadere del timer:

```C++
//Timer aperiodico 2
#define TLIMITE	1000
unsigned long atimer2;

void loop()
{
// condizione che valuta l’accadere di un certo evento 
// di attivazione del conteggio del tempo
if(condA){
    atimer2 = millis();
}

if ((millis() – atimer2) < (unsigned long) TLIMITE2)
{
    .... 
    // istruzioni eseguite finchè NON scade il timer 2
}
}
```
Molti timers possono anche essere attivi nascosti all’interno di funzioni che sono richiamate all’interno del loop(). Possono realizzare una codice che, benchè sia eseguito **linearmente**, viene eseguito aggiungendo nuove parti in istanti via via **successivi**, man mano che passa il tempo. 
Ogni evento che scade prima viene ritenuto **disabilitante** per tutto il resto della funzione, realizzano una catena di eventi:

```C++
void loop()
{
poll();  // funzione con blocchi di codice eseguiti prima o dopo di certi eventi
…….
}

void poll()
{
if(condA){ // evento che attiva il timer (può essere ricorrente ma, in generale, è aperiodico)
    atimer1 = millis();
}

// finchè non si raggiunge TLIMITE1 ritorna senza fare nulla
if ((millis() – atime1) < (unsigned long) TLIMITE1) return;

//BLOCCO_A

// finchè non si raggiunge TLIMITE2 ritorna dopo avere eseguito il blocco di istruzioni A
if ((millis() – atimer2) < (unsigned long) TLIMITE2) return;

//BLOCCO_B

// finchè non si raggiunge TLIMITE3 ritorna dopo avere eseguito il blocco di istruzioni 
// A e B
if ((millis() – atimer1) < (unsigned long) TLIMITE3) return;
}
```

```C++
Attenzione ad un errore insidioso:
void poll()
{
if ((millis() – atimer1) < (unsigned long) TLIMITE1)
{
    .... 
    // istruzioni eseguite finchè NON scade il timer 1
}

if ((millis() – atimer2) < (unsigned long) TLIMITE2)
{
    .... 
    // istruzioni eseguite finchè NON scade il timer 2
}

}
```

In questa situazione se scade il primo timer viene comunque controllato lo scadere del secondo. La cascata degli if equivale ad un OR logico sulle condizioni di scadenza.
Se voglio che ne accada solo una posso scrivere così:

```C++
void poll()
{
if ((millis() – atimer1) < (unsigned long) TLIMITE1)
{
    .... 
    // istruzioni eseguite finchè NON scade il timer 1
}else if ((millis() – atimer2) < (unsigned long) TLIMITE2)
{
    .... 
    // istruzioni eseguite finchè NON scade il timer 2
}
}

```
Dove l’ordine stabilisce la priorità di valutazione.
