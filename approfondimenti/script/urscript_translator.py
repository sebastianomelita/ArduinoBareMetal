"""
Wrapper "URScript-like" per un controller Webots di un Universal Robots (UR3e).

ATTENZIONE: NON è URScript vero. Imita la SINTASSI di URScript, non la
SEMANTICA del controller UR. Utile per prototipare in simulazione, ma il
comportamento (timing, dinamica, IK) non è identico al robot reale.
"""

import math
from controller import Robot


class URScriptTranslator:
    JOINT_NAMES = [
        "shoulder_pan_joint", "shoulder_lift_joint", "elbow_joint",
        "wrist_1_joint", "wrist_2_joint", "wrist_3_joint",
    ]

    def __init__(self):
        self.robot = Robot()
        self.timestep = int(self.robot.getBasicTimeStep())

        self.joints = []
        self.sensors = []
        for name in self.JOINT_NAMES:
            motor = self.robot.getDevice(name)
            self.joints.append(motor)

            # NB: verifica il nome esatto del sensore con "View PROTO Source".
            # Nei PROTO UR di Webots la convenzione è di solito "<joint>_sensor".
            sensor = self.robot.getDevice(name + "_sensor")
            sensor.enable(self.timestep)
            self.sensors.append(sensor)

        # un primo step per avere letture valide dai sensori
        self.robot.step(self.timestep)

    def _step(self):
        """Avanza di uno step. Ritorna False quando la simulazione termina."""
        return self.robot.step(self.timestep) != -1

    def movej(self, q, a=1.4, v=1.05, tol=1e-3, timeout=10.0):
        """
        Imita movej(q, a, v) di URScript: movimento nello spazio dei giunti.
          q       : lista di 6 angoli target [rad]
          a       : accelerazione [rad/s^2]  -> mappata su setAcceleration
          v       : velocità      [rad/s]    -> mappata su setVelocity
          tol     : tolleranza di arrivo [rad]
          timeout : tempo massimo di attesa [s] (guardia di sicurezza)
        Blocca finché tutti i giunti sono entro 'tol' dal target o scade il timeout.
        """
        for i, motor in enumerate(self.joints):
            motor.setAcceleration(a)
            motor.setVelocity(v)
            motor.setPosition(q[i])

        dt = self.timestep / 1000.0
        elapsed = 0.0
        while elapsed < timeout:
            if not self._step():
                return  # simulazione chiusa
            err = max(abs(self.sensors[i].getValue() - q[i]) for i in range(6))
            if err < tol:
                return
            elapsed += dt

    def get_actual_joint_positions(self):
        """Equivalente di get_actual_joint_positions() di URScript."""
        return [s.getValue() for s in self.sensors]

    def movel(self, pose, a=1.2, v=0.25):
        """
        movel di URScript = movimento LINEARE nello spazio cartesiano.
        Webots NON fornisce cinematica inversa: serve un solver esterno
        (es. ikpy) per convertire 'pose' -> angoli di giunto, poi chiamare movej.
        """
        raise NotImplementedError(
            "movel richiede un solver IK (es. ikpy): converti la posa in angoli "
            "e poi usa movej(). Webots non risolve la cinematica inversa da solo."
        )

    def set_digital_out(self, pin, value):
        """
        In Webots non esiste un 'pin digitale' generico. Un'uscita UR di solito
        pilota una pinza (motori del gripper) o un Connector/LED. Mappa qui il
        dispositivo reale della tua scena.
        """
        print(f"[URScript] set_digital_out({pin}, {value})")
        # Esempio:
        #   grip = self.robot.getDevice("gripper_motor")
        #   grip.setPosition(0.0 if value else 0.04)


if __name__ == "__main__":
    ur = URScriptTranslator()
    ur.movej([0.0, -math.pi / 2, math.pi / 2, -math.pi / 2, -math.pi / 2, 0.0])
    print("Giunti attuali:", ur.get_actual_joint_positions())
