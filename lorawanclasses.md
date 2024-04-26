>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)

Di seguito è riportata l'architettura generale di una rete Lorawan. Essa è composta essenzialmente di una rete di accesso ai sensori e dau na rete di distribuzione che fa da collante di ciascuna rete di sensori.

La rete di sensori fisica è a stella dove il centro stella è il gateway. Un sensore può anche essere associao a più gateway ed inviare dati a tutti i gateway a cui esso è associato. I dati normalmente arrivano ad un certo dispositivo attraverso un solo gateway.

I gateway utilizzano la rete internet (o una LAN) per realizzare il collegamento con il network server, per cui, in definitiva, la topologia risultante è quella di una stella di reti a stella. Il centrostella di livello gerarchico più alto è il network server, gli altri sono realizzati dai gateway.


<img src="img/lorawanArchitecture.png" alt="alt text" width="800">














>[Torna a reti di sensori](sensornetworkshort.md#reti-di-sensori-e-attuatori)
