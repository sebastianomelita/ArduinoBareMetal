


>[Torna all'indice](indexseriale.md) >[versione in C++](serialegenerazionecmd.md)

La generazione dei messaggi sulla seriale può avvenire in qualunque punto del codice, anche all'interno di uno schedulatore, ma non dentro una ISR.
Un messaggio può essere inviato sulla seriale essenzialmente per:
- **generare dei report** sullo stato del sistema
-  **generare dei comandi** con cui richiedere un servizio su un dispositivo remoto 

La generazione dei messaggi in genere avviene:
- **periodicamente**, cioè inserita dentro uno schedulatore che la genera a la invia ad intervalli regolari. Utile per creare un report periodico dello stato del sistema.
- **triggerata da eventi**, cioè il messaggio viene generato all'accadere di un evento specifico come la pressione di un pulsante, il superamento di una soglia per il valore di un sensore, ecc.
- **su richiesta**, in seguito al polling eseguito da un master,  si potrebbe generare una risposta che descriva lo stato generale del sistema o di una sua parte specificata come parametro della richiesta.

**Esempio di generazione periodica di un comando**
```Python
#
# Scrivere un programma Arduino che manda sulla seriale ogni due secondi lo stato di due led 
# (cucina, soggiorno). 
#
from gpio import *
from time import *

def main():

	class Ambienti:
		CUCINA = 0
		SOGGIORNO = 1
		
	class Pulsanti:
		SOGGIORNO = 0
		CUCINA = 1
	
	class Lampade:
		SOGGIORNO = 2
		CUCINA  = 3

	NSTEP = 100
	TBASE = 0.1
	precval = [0,0]
	stato = [0,0]
	precm = 0
	step = 0
	pinMode(Pulsanti.SOGGIORNO, INT);
	pinMode(Pulsanti.CUCINA, INT);
	pinMode(Lampade.SOGGIORNO, OUT);
	pinMode(Lampade.CUCINA, OUT);
	usb = USB(0, 9600)
	
	while True:
		if (uptime() - precm) >= TBASE:  	   	# schedulatore (e anche antirimbalzo)
			precm = uptime()  			   		# preparo il tic successivo	
			step = (step+1) % NSTEP				# conteggio circolare
			
			# comunicazione stato led
			if not (step%20):
				usb.write("cucina:\"");
				usb.write(stato[Ambienti.CUCINA]);
				usb.write("\n");
				usb.write("\"");
				usb.write("soggiorno:\"");
				usb.write("\n");
				usb.println(stato[Ambienti.SOGGIORNO]);
				usb.write("\"");
			
if __name__ == "__main__":
	main()

```	

**Esempio di generazione triggerata da evento di un comando**
```Python
#
# Scrivere un programma Arduino che accenda due led (ingresso, sala). Accenderli con due pulsanti toggle separati. Lo stato dei led deve essere scritto sulla seriale all'avvenire (occorrenza) di ogni comando.
#
from gpio import *
from time import *

def main():

	def transizione(val, n):  # transizione di un pulsante
		cambiato = false
		cambiato = (precval[n] != val)
		precval[n] = val  
		return cambiato

	class Ambienti:
		SALA = 0
		INGRESSO = 1
		
	class Pulsanti:
		SALA = 0
		INGRESSO = 1
	
	class Lampade:
		SALA  =2
		INGRESSO = 3

	NSTEP = 100
	TBASE = 0.1
	precval = [0,0]
	stato = [0,0]
	precm = 0
	step = 0
	pinMode(Pulsanti.SALA, IN);
	pinMode(Pulsanti.INGRESSO, IN);
	pinMode(Lampade.SALA, OUT);
	pinMode(Lampade.INGRESSO, OUT);
	usb = USB(0, 9600)
	
	while True:
		if (uptime() - precm) >= TBASE:  	   	# schedulatore (e anche antirimbalzo)
			precm = uptime()  			   		# preparo il tic successivo	
			step = (step+1) % NSTEP				# conteggio circolare
			
			# polling pulsante SCALA
			in=digitalRead(Pulsanti.SALA)
			if  transizione(in,SALA):
				if in == HIGH: # se fronte di salita
					stato[Ambienti.SALA] = HIGH - stato[Ambienti.SALA];
					digitalWrite(Lampade.SALA, stato[Ambienti.SALA]);
					# generazione report ad evento pressione
					usb.write("sala:\"")
					usb.write(stato[Ambienti.SALA])
					usb.write("\n")
					usb.write("\"")

			# polling pulsante INGRESSO
			in=digitalRead(Pulsanti.INGRESSO)
			if  transizione(in,INGRESSO):
				if in == HIGH: # se fronte di salita
					stato[Ambienti.INGRESSO] = HIGH - stato[Ambienti.INGRESSO];
					digitalWrite(Lampade.INGRESSO, stato[Ambienti.INGRESSO]);
					# generazione report ad evento pressione
					usb.write("ingresso:\"")
					usb.write(stato[Ambienti.INGRESSO])
					usb.write("\n")
					usb.write("\"")
								
if __name__ == "__main__":
	main()
```	
>[Torna all'indice](indexseriale.md) >[versione in C++](serialegenerazionecmd.md)
<!--stackedit_data:
eyJoaXN0b3J5IjpbMjA4MTI3NTg5Ml19
-->
