"""
Modulo PINZA per il wrapper UR3 in Webots.

Un solo metodo, grip(close=True/False), con due modalita' selezionabili:

  MODE = "connector"  -> presa "magnetica" via nodo Connector di Webots.
                         Robusta, zero taratura di attrito. Ideale per imparare
                         la SEQUENZA di pick & place. L'oggetto si aggancia
                         rigidamente all'utensile quando chiudi.

  MODE = "robotiq"    -> presa per CONTATTO reale muovendo i motori delle dita.
                         Piu' fedele al robot vero (l'oggetto puo' scivolare se
                         stringi male), ma va tarata (attrito, forza, masse).

I nomi dei dispositivi vanno verificati con "View PROTO Source" sulla tua scena.
"""

# --- CONFIG --------------------------------------------------------------
GRIPPER_MODE = "connector"          # "connector" oppure "robotiq"

CONNECTOR_NAME = "connector"        # nome del nodo Connector sul toolSlot

ROBOTIQ_FINGER_MOTORS = [           # motori delle dita (adatta alla tua pinza)
    "finger_1_joint_1",
    "finger_2_joint_1",
    "finger_middle_joint_1",
]
ROBOTIQ_CLOSED = 0.8                # posizione dita "chiuse" [rad]
ROBOTIQ_OPEN = 0.0                  # posizione dita "aperte" [rad]
ROBOTIQ_SETTLE_S = 0.4             # tempo per far chiudere/aprire le dita


class Gripper:
    def __init__(self, robot, timestep, sleep_fn):
        self.robot = robot
        self.timestep = timestep
        self._sleep = sleep_fn          # riusa lo sleep del wrapper (avanza la sim)
        self.mode = GRIPPER_MODE

        if self.mode == "connector":
            self.connector = robot.getDevice(CONNECTOR_NAME)
            if self.connector is not None:
                # abilita il rilevamento di presenza, cosi' lock() aggancia se c'e' contatto
                self.connector.enablePresence(timestep)
            self.fingers = []
        else:  # robotiq
            self.connector = None
            self.fingers = [robot.getDevice(n) for n in ROBOTIQ_FINGER_MOTORS]

    def grip(self, close=True):
        """close=True -> chiudi/afferra ; close=False -> apri/rilascia."""
        if self.mode == "connector":
            self._grip_connector(close)
        else:
            self._grip_robotiq(close)

    # -- implementazioni ----------------------------------------------------
    def _grip_connector(self, close):
        if self.connector is None:
            print("[grip] Connector non trovato:", CONNECTOR_NAME)
            return
        if close:
            self.connector.lock()       # aggancia l'oggetto in presa
            print("[grip] lock (presa)")
        else:
            self.connector.unlock()     # rilascia
            print("[grip] unlock (rilascio)")

    def _grip_robotiq(self, close):
        target = ROBOTIQ_CLOSED if close else ROBOTIQ_OPEN
        for m in self.fingers:
            if m is not None:
                m.setPosition(target)
        print(f"[grip] dita -> {'chiuse' if close else 'aperte'} ({target} rad)")
        self._sleep(ROBOTIQ_SETTLE_S)   # lascia il tempo alle dita di muoversi
