/*################### File version: 4.3 ##################*/
/*########################################################*/
/* Controllo stepmotor per il nastro trasportatore a      */
/* scopo didattico (il docente dovrà spiegarne il         */
/* funzionamento).                                        */
/* vengono utilizzati tre pulsanti:                       */
/*  - pulsante rosso P1 per halt e stop del motore;       */
/*  - pulsante verde P2 per controllare il senso          */
/*    di rotazione;                                       */
/*  - pulsante grigio per controllare il microstepping    */
/*    (quindi la velocità) del motore.                    */
/* I tre pulsanti sono letti in polling con la struttura  */
/* Button: un cambio è accettato solo quando il nuovo     */
/* livello resta stabile per DEBOUNCETIME ms (filtra      */
/* rimbalzi e disturbi brevi). P1 e P2 emulano i comandi */
/* a livello del robot UR: il motore gira finché P1 è     */
/* alto, il verso segue il livello di P2. Gli step del    */
/* motore sono generati dall'interrupt del Timer1.        */
/*                                                        */
/* Con il robot UR, marcia e direzione arrivano da una    */
/* scheda d'interfaccia optoisolata 24 V -> 5 V. Se la    */
/* scheda ha l'uscita NPN (attiva bassa), togliere il     */
/* commento a #define INGRESSI_ATTIVI_BASSI.              */
/*                                                        */
/*     driver A4988 (tra parentesi i nomi sul DRV8825)    */
/*         ,-----------,                                  */
/*  ENABLE |o         o| V_MOTORE                         */
/* MS1(M0) |o ____    o| GND                              */
/* MS2(M1) |o|    |   o| 2B                               */
/* MS3(M2) |o|____|   o| 2A                               */
/*   RESET |o         o| 1A                               */
/*   SLEEP |o     _   o| 1B                               */
/*    STEP |o    |_|  o| VDD (FAULT sul DRV8825)          */
/*     DIR |o         o| GND                              */
/*         `-----------'                                  */
/*                                                        */
/*   ===================================================  */
/*    MS1 | MS2  | MS3  |  A4988     |  DRV8825           */
/*   -----|------|------|------------|-----------------   */
/*    Low | Low  | Low  |  Full step |  Full step         */
/*    High| Low  | Low  |  1/2 step  |  1/2 step          */
/*    Low | High | Low  |  1/4 step  |  1/4 step          */
/*    High| High | Low  |  1/8 step  |  1/8 step          */
/*    Low | Low  | High |  -         |  1/16 step         */
/*    High| High | High |  1/16 step |  1/32 step         */
/*   ===================================================  */
/* Il quinto livello (SIXTEENTH) usa la riga giusta per   */
/* il driver scelto con #define DRIVER_DRV8825.           */
/*########################################################*/
// https://wokwi.com/projects/426024956545802241
#include <TimerOne.h>  // Libreria per gestire interrupt basati su timer
#define dirPin 6 //2
#define stepPin 3
// Nota: il pin 9 (MS1) è anche l'uscita PWM del Timer1: va bene finché
// non si usa Timer1.pwm().
#define button_ForwardReverse 5
#define button_HaltStart 4
#define button_Microstep 2 //6
#define  DEBOUNCETIME 50
// #define DRIVER_DRV8825   // togliere il commento se si usa il DRV8825

// Comandi del robot (marcia e direzione) attraverso la scheda d'interfaccia:
// - uscita PNP, o pulsanti in pull-down: pin ALTO quando il comando è attivo (default);
// - uscita NPN o modulo PC817 con pull-up: pin BASSO quando il comando è attivo.
// #define INGRESSI_ATTIVI_BASSI   // togliere il commento con schede a uscita NPN
#ifdef INGRESSI_ATTIVI_BASSI
  #define ATTIVO LOW                  // livello del comando attivo
  #define RIPOSO HIGH                 // livello a riposo
  #define MODO_COMANDI INPUT_PULLUP   // pull-up interno per le uscite a collettore aperto
#else
  #define ATTIVO HIGH
  #define RIPOSO LOW
  #define MODO_COMANDI INPUT          // il pull-down è sulla scheda (o la resistenza esterna)
#endif

volatile bool directionChange = false;  // Flag per prevenire step durante cambio dir:
                                        // ridondante finché il Timer1 viene fermato
                                        // durante il cambio, ma protegge se lo si toglie
volatile bool direction = LOW;    // HIGH quando il comando di direzione è attivo
volatile bool stato = false;
uint8_t stepLevel = 0;
uint16_t stepInterval = 1200; // Microsecond (600µs HIGH + 600µs LOW)

// Variabili per gestione non bloccante del cambio di direzione
unsigned long dirChangeStartTime = 0;
bool waitingForDirChange = false;

int MS[3] = {9, 8, 7}; // pin microstepper MS1=MS[0]->9, MS2=MS[1]->8, MS3=MS[2]->7
boolean microstepArray[5][3] = {
 {LOW, LOW, LOW},     // full step
 {HIGH, LOW, LOW},    // 1/2
 {LOW, HIGH, LOW},    // 1/4
 {HIGH, HIGH, LOW},   // 1/8
#ifdef DRIVER_DRV8825
 {LOW, LOW, HIGH}     // 1/16 sul DRV8825 (HIGH, HIGH, HIGH sarebbe 1/32)
#else
 {HIGH, HIGH, HIGH}   // 1/16 sull'A4988
#endif
};

enum resolution{
	FULL,
	HALF,
	QUARTER,
	EIGHTH,
	SIXTEENTH
};

/**
 * Struttura Button per la gestione del debounce
 * 
 * Ogni variazione letta (rimbalzi e disturbi compresi) fa ripartire il timer.
 * Un cambio è accettato solo quando il nuovo livello è rimasto stabile per debtime ms:
 * un'apertura breve del contatto a pulsante premuto non viene presa per un rilascio.
 */
struct Button {
  uint8_t pin;           // Pin di Arduino collegato al pulsante
  uint8_t val0;          // Ultimo valore letto
  unsigned long debtime; // Tempo di debounce in millisecondi
  uint8_t val;           // Valore attuale letto
  uint8_t val00;         // Ultimo stato stabile (accettato)
  unsigned long last;    // Timestamp ultima variazione letta
  
  /**
   * Verifica se c'è stato un cambiamento valido dello stato del pulsante.
   * 
   * @return true quando il nuovo livello è stabile da debtime ms, false altrimenti
   */
  bool changed() {
    val = digitalRead(pin);
    if (val != val0) {              // ogni variazione, rimbalzi e disturbi compresi,
      val0 = val;                   // fa ripartire il timer
      last = millis();
    } else if (val != val00 && (unsigned long)(millis() - last) > debtime) {
      val00 = val;                  // livello stabile da debtime: cambio vero
      return true;
    }
    return false;
  }
};

// Inizializzazione dei pulsanti: pin, val0, debtime, val, val00, last.
// I comandi del robot partono dal livello di RIPOSO; il pulsante del microstepping
// resta un pulsante locale in pull-down (riposo LOW). Se all'accensione un comando
// del robot è già attivo, viene accettato dopo DEBOUNCETIME ms.
Button buttonStart = {button_HaltStart, RIPOSO, DEBOUNCETIME, RIPOSO, RIPOSO, 0};    // start/stop
Button buttonDir = {button_ForwardReverse, RIPOSO, DEBOUNCETIME, RIPOSO, RIPOSO, 0};  // direzione
Button buttonStep = {button_Microstep, LOW, DEBOUNCETIME, LOW, LOW, 0};              // microstepping

/**
 * Imposta la risoluzione del microstepping
 * 
 * @param level Livello di microstepping (0-4)
 * @return true se il livello è valido, false altrimenti
 */
bool setResolution(uint8_t level){
  if (level > 4) return false;
  
  Serial.print("Res: ");
  for (int i = 0; i < 3; i++) {
    digitalWrite(MS[i],microstepArray[level][i]);
    Serial.print(microstepArray[level][i]);
  }
  Serial.println();
  return true;
}

/**
 * Funzione di interrupt per controllare gli step del motore
 * Viene eseguita a ogni interrupt del timer
 */
void stepMotor() {
  static bool stepState = false;
  
  if (directionChange) {
    return;  // Non generare step durante il cambio di direzione
  }
  
  stepState = !stepState;
  digitalWrite(stepPin, stepState);
}

void setup() {
  Serial.begin(115200);
 
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(MS[0], OUTPUT);
  pinMode(MS[1], OUTPUT);
  pinMode(MS[2], OUTPUT);
  pinMode(button_ForwardReverse, MODO_COMANDI);
  pinMode(button_HaltStart, MODO_COMANDI);
  pinMode(button_Microstep, INPUT);
   
  setResolution(FULL);
  stato = false;
  direction = LOW;                    // comando di direzione a riposo
  directionChange = false;
  digitalWrite(dirPin, direction);    // verso di rotazione iniziale definito
  
  // Configura l'interrupt di timer per gli step del motore
  Timer1.initialize(stepInterval/2);  // Metà del periodo totale
  Timer1.attachInterrupt(stepMotor);
  Timer1.stop();  // Ferma il timer finché non è necessario
}

void loop() {
  // Gestione non bloccante del cambio di direzione
  if (waitingForDirChange && ((unsigned long) millis() - dirChangeStartTime >= 1000)) {
    waitingForDirChange = false;
    digitalWrite(dirPin, direction);
    directionChange = false;
    
    // Se il motore era in movimento prima del cambio direzione, riprendi
    if (stato) {
      Timer1.resume();
    }
  }
  
  // Controlla il pulsante di microstepping
  if (buttonStep.changed() && buttonStep.val) {
    stepLevel = (stepLevel + 1) % 5;
    setResolution(stepLevel);
  }

  // Comando di direzione (livello): cambio del verso a ogni cambio di livello stabile
  if (buttonDir.changed()) {
    direction = (buttonDir.val == ATTIVO);  // comando attivo: verso HIGH
    directionChange = true;
    Serial.print("Direction: ");
    Serial.println(direction);
    if (stato) Timer1.stop();          // ferma il motore durante il cambio
    waitingForDirChange = true;        // cambio di direzione non bloccante
    dirChangeStartTime = millis();     // un nuovo cambio durante l'attesa la fa ripartire
  }

  // Comando di avvio/arresto (livello): il motore gira finché il comando è alto
  if (buttonStart.changed()) {
    stato = (buttonStart.val == ATTIVO);    // il motore gira finché il comando è attivo
    if (stato) {
      if (!waitingForDirChange) Timer1.start();   // durante il cambio direzione riparte dopo
      Serial.println("Timer started");
    } else {
      Timer1.stop();
      Serial.println("Timer stopped");
    }
  }
  
  delay(1); // non serve al debounce né al motore: si può togliere
}
// https://wokwi.com/projects/426024956545802241