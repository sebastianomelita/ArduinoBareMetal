"""
Wrapper UR3 "URScript-fedele" per Webots (UR3 / UR3e) - VERSIONE FINALE.

Riproduce in modo APPROSSIMATO i movimenti progettati in URScript / PolyScope.
Fedele alla SINTASSI e alle CONVENZIONI UR; non alla fisica (vedi TUTORIAL).

Questo file integra gia' la PINZA (modulo gripper.py) e usa di default l'IK
numerica ikpy. Per passare all'IK analitica ur_ikfast basta cambiare una riga
(vedi la classe IKBackendFast in fondo e la nota nel costruttore).

DIPENDENZE
  - giunti + I/O:            solo Webots                -> UR3(use_ik=False)
  - movel / get_inverse_kin: numpy + ikpy + URDF UR3e   -> pip install numpy ikpy
"""

import math

try:
    import numpy as np
except ImportError:
    np = None

from controller import Robot


# ===========================================================================
# CONFIG - verifica i nomi con "View PROTO Source" nella tua scena.
# ===========================================================================
JOINT_NAMES = [
    "shoulder_pan_joint", "shoulder_lift_joint", "elbow_joint",
    "wrist_1_joint", "wrist_2_joint", "wrist_3_joint",
]
URDF_PATH = "ur3e.urdf"
IKPY_BASE_ELEMENTS = ["base_link"]
DIGITAL_IN_SENSORS = {0: ("distance sensor", 500.0)}   # pin -> (device, soglia)
DIGITAL_OUT_DEVICES = {}                                # pin -> device (il pin 0 = pinza)


# ===========================================================================
# Conversioni pose UR (rotazione = vettore asse-angolo)
# ===========================================================================
def rotvec_to_matrix(rx, ry, rz):
    theta = math.sqrt(rx*rx + ry*ry + rz*rz)
    if theta < 1e-12:
        return np.eye(3)
    kx, ky, kz = rx/theta, ry/theta, rz/theta
    K = np.array([[0, -kz, ky], [kz, 0, -kx], [-ky, kx, 0]])
    return np.eye(3) + math.sin(theta)*K + (1-math.cos(theta))*(K @ K)


def matrix_to_rotvec(R):
    angle = math.acos(max(-1.0, min(1.0, (np.trace(R) - 1) / 2)))
    if abs(angle) < 1e-9:
        return (0.0, 0.0, 0.0)
    rx, ry, rz = R[2, 1]-R[1, 2], R[0, 2]-R[2, 0], R[1, 0]-R[0, 1]
    n = math.sqrt(rx*rx + ry*ry + rz*rz)
    s = angle / n
    return (rx*s, ry*s, rz*s)


def _mat_to_quat(R):
    t = np.trace(R)
    if t > 0:
        s = math.sqrt(t + 1.0) * 2
        return np.array([0.25*s, (R[2, 1]-R[1, 2])/s, (R[0, 2]-R[2, 0])/s, (R[1, 0]-R[0, 1])/s])
    i = int(np.argmax([R[0, 0], R[1, 1], R[2, 2]]))
    if i == 0:
        s = math.sqrt(1.0 + R[0, 0] - R[1, 1] - R[2, 2]) * 2
        return np.array([(R[2, 1]-R[1, 2])/s, 0.25*s, (R[0, 1]+R[1, 0])/s, (R[0, 2]+R[2, 0])/s])
    if i == 1:
        s = math.sqrt(1.0 + R[1, 1] - R[0, 0] - R[2, 2]) * 2
        return np.array([(R[0, 2]-R[2, 0])/s, (R[0, 1]+R[1, 0])/s, 0.25*s, (R[1, 2]+R[2, 1])/s])
    s = math.sqrt(1.0 + R[2, 2] - R[0, 0] - R[1, 1]) * 2
    return np.array([(R[1, 0]-R[0, 1])/s, (R[0, 2]+R[2, 0])/s, (R[1, 2]+R[2, 1])/s, 0.25*s])


def _quat_slerp(q0, q1, t):
    d = np.dot(q0, q1)
    if d < 0:
        q1, d = -q1, -d
    if d > 0.9995:
        r = q0 + t*(q1-q0)
        return r / np.linalg.norm(r)
    th = math.acos(d) * t
    q2 = q1 - q0*d
    q2 /= np.linalg.norm(q2)
    return q0*math.cos(th) + q2*math.sin(th)


def _quat_to_mat(q):
    w, x, y, z = q
    return np.array([
        [1-2*(y*y+z*z), 2*(x*y-z*w), 2*(x*z+y*w)],
        [2*(x*y+z*w), 1-2*(x*x+z*z), 2*(y*z-x*w)],
        [2*(x*z-y*w), 2*(y*z+x*w), 1-2*(x*x+y*y)],
    ])


# ===========================================================================
# Backend IK - DEFAULT: ikpy (numerica). Vedi IKBackendFast per ur_ikfast.
# ===========================================================================
class IKBackend:
    def __init__(self, urdf_path=URDF_PATH, base_elements=IKPY_BASE_ELEMENTS):
        from ikpy.chain import Chain
        self.chain = Chain.from_urdf_file(urdf_path, base_elements=base_elements)
        self.active = [i for i, l in enumerate(self.chain.links)
                       if getattr(l, "joint_type", None) in ("revolute", "continuous")]

    def _full(self, q6):
        v = np.zeros(len(self.chain.links))
        for idx, qi in zip(self.active, q6):
            v[idx] = qi
        return v

    def fk(self, q6):
        T = self.chain.forward_kinematics(self._full(q6))
        return [*T[:3, 3], *matrix_to_rotvec(T[:3, :3])]

    def ik(self, pose, q_near):
        target = np.eye(4)
        target[:3, 3] = pose[:3]
        target[:3, :3] = rotvec_to_matrix(pose[3], pose[4], pose[5])
        sol = self.chain.inverse_kinematics_frame(
            target, initial_position=self._full(q_near), orientation_mode="all")
        return [sol[i] for i in self.active]


# ===========================================================================
# Wrapper principale
# ===========================================================================
class UR3:
    def __init__(self, use_ik=True):
        self.robot = Robot()
        self.timestep = int(self.robot.getBasicTimeStep())
        self.dt = self.timestep / 1000.0

        self.joints, self.sensors = [], []
        for name in JOINT_NAMES:
            m = self.robot.getDevice(name)
            m.setPosition(float("inf")); m.setVelocity(0.0)
            self.joints.append(m)
            s = self.robot.getDevice(name + "_sensor")
            s.enable(self.timestep)
            self.sensors.append(s)

        self.din = {}
        for pin, (dev, thr) in DIGITAL_IN_SENSORS.items():
            d = self.robot.getDevice(dev)
            if d is not None:
                d.enable(self.timestep)
            self.din[pin] = (d, thr)
        self.dout = {pin: self.robot.getDevice(dev) for pin, dev in DIGITAL_OUT_DEVICES.items()}

        # --- PINZA integrata (modulo gripper.py) ---
        from gripper import Gripper
        self.gripper = Gripper(self.robot, self.timestep, self.sleep)

        self.tcp = [0, 0, 0, 0, 0, 0]

        # --- IK: ikpy di default. Per ur_ikfast: self.ik = IKBackendFast() ---
        self.ik = IKBackend() if (use_ik and np is not None) else None

        self.robot.step(self.timestep)

    def _step(self):
        return self.robot.step(self.timestep) != -1

    # -- cinematica ---------------------------------------------------------
    def get_actual_joint_positions(self):
        return [s.getValue() for s in self.sensors]

    def get_forward_kin(self, q=None):
        if self.ik is None:
            raise RuntimeError("IK non disponibile (installa numpy + ikpy).")
        return self.ik.fk(q if q is not None else self.get_actual_joint_positions())

    def get_actual_tcp_pose(self):
        return self.get_forward_kin()

    def get_inverse_kin(self, pose, q_near=None):
        if self.ik is None:
            raise RuntimeError("IK non disponibile (installa numpy + ikpy).")
        return self.ik.ik(pose, q_near or self.get_actual_joint_positions())

    def set_tcp(self, pose):
        self.tcp = list(pose)

    # -- movimenti ----------------------------------------------------------
    def movej(self, q, a=1.4, v=1.05, t=0.0, r=0.0):
        """
        movej(q, a, v, t, r): giunti sincronizzati (finiscono insieme).
        r > 0 attiva un blend APPROSSIMATO -- SOLO EFFETTO VISIVO, NON FEDELE:
        riparte col comando successivo appena si e' entro r dal target, cosi'
        il moto non si ferma sul waypoint. Non riproduce geometria/tempi reali.
        """
        q0 = self.get_actual_joint_positions()
        dq = [q[i] - q0[i] for i in range(6)]
        duration = t if t > 0 else max((abs(d)/v for d in dq), default=0.0)
        duration = max(duration, self.dt)
        for i, m in enumerate(self.joints):
            m.setVelocity(min(v, abs(dq[i])/duration) if abs(dq[i]) > 1e-9 else 0.0001)
            m.setAcceleration(a)
            m.setPosition(q[i])
        if r > 0:
            self._wait_blend(q, r)
        else:
            self._wait_reached(q)

    def movel(self, pose, a=1.2, v=0.25, t=0.0, r=0.0):
        """
        movel(pose, a, v, t, r): retta cartesiana (posizione lineare + slerp).
        r > 0 attiva un blend APPROSSIMATO -- SOLO EFFETTO VISIVO, NON FEDELE:
        interrompe il segmento quando manca meno di r [m] al punto finale, cosi'
        il comando successivo 'taglia l'angolo'. Non e' la geometria del blend reale.
        Tieni r piu' piccolo della lunghezza del segmento.
        """
        if self.ik is None:
            raise RuntimeError("movel richiede l'IK: installa numpy + ikpy e imposta URDF_PATH.")
        start = self.get_actual_tcp_pose()
        p0, p1 = np.array(start[:3]), np.array(pose[:3])
        q0 = _mat_to_quat(rotvec_to_matrix(*start[3:]))
        q1 = _mat_to_quat(rotvec_to_matrix(*pose[3:]))
        dist = np.linalg.norm(p1 - p0)
        n = max(2, int(math.ceil((t*v if t > 0 else dist) / max(v*self.dt, 1e-6))))
        seed = self.get_actual_joint_positions()
        for k in range(1, n + 1):
            s = k / n
            if r > 0 and (1 - s)*dist <= r:      # blend visivo: taglia il finale
                return
            pos = (1 - s)*p0 + s*p1
            R = _quat_to_mat(_quat_slerp(q0, q1, s))
            seed = self.ik.ik([pos[0], pos[1], pos[2], *matrix_to_rotvec(R)], seed)
            for i, m in enumerate(self.joints):
                m.setVelocity(6.0); m.setPosition(seed[i])
            if not self._step():
                return

    def speedj(self, qd, a=1.0, t=0.0):
        for i, m in enumerate(self.joints):
            m.setPosition(float("inf")); m.setAcceleration(a); m.setVelocity(qd[i])
        for _ in range(max(1, int(t/self.dt) if t > 0 else 1)):
            if not self._step():
                return

    def stopj(self, a=2.0):
        self.speedj([0]*6, a=a, t=self.dt)

    def _wait_reached(self, q, tol=1e-3, timeout=15.0):
        elapsed = 0.0
        while elapsed < timeout:
            if not self._step():
                return
            if max(abs(self.sensors[i].getValue() - q[i]) for i in range(6)) < tol:
                return
            elapsed += self.dt

    def _wait_blend(self, q_target, r, timeout=15.0):
        """
        Uscita anticipata entro r dal target -> effetto blend VISIVO, non fedele.
        Con IK: r = distanza cartesiana [m] (come in URScript). Senza IK: r e'
        trattato come raggio nello spazio dei giunti [rad] (approssimazione).
        """
        there = self.ik.fk(q_target)[:3] if self.ik is not None else None
        elapsed = 0.0
        while elapsed < timeout:
            if not self._step():
                return
            if there is not None:
                here = self.ik.fk(self.get_actual_joint_positions())[:3]
                d = math.sqrt(sum((here[i]-there[i])**2 for i in range(3)))
            else:
                d = max(abs(self.sensors[i].getValue() - q_target[i]) for i in range(6))
            if d <= r:
                return
            elapsed += self.dt

    # -- pinza --------------------------------------------------------------
    def grip(self, close=True):
        """grip(close=True/False): chiudi/afferra oppure apri/rilascia."""
        self.gripper.grip(close=close)

    # -- I/O e utilita' -----------------------------------------------------
    def get_digital_in(self, pin):
        dev, thr = self.din.get(pin, (None, 0))
        return bool(dev and dev.getValue() < thr)

    def set_digital_out(self, pin, value):
        """pin 0 = pinza; altri pin -> dispositivi in DIGITAL_OUT_DEVICES."""
        print(f"[URScript] set_digital_out({pin}, {value})")
        if pin == 0:
            self.gripper.grip(close=bool(value))
        elif pin in self.dout and self.dout[pin] is not None:
            self.dout[pin].setPosition(0.0 if value else 0.04)

    def sleep(self, seconds):
        for _ in range(max(1, int(seconds/self.dt))):
            if not self._step():
                return

    def textmsg(self, *args):
        print("[URScript]", *args)


# ===========================================================================
# UPGRADE OPZIONALE: IK analitica ur_ikfast.
# Installazione: pip install git+https://github.com/cambel/ur_ikfast
#   (richiede compilatore + Cython; verifica che includa il modello 'ur3e').
# Per attivarla: nel costruttore usa  self.ik = IKBackendFast()  al posto di IKBackend().
# Nota: verifica la firma di forward()/inverse() della tua versione di ur_ikfast.
# ===========================================================================
class IKBackendFast:
    def __init__(self, robot_name="ur3e"):
        from ur_ikfast import ur_kinematics
        self.k = ur_kinematics.URKinematics(robot_name)

    def fk(self, q6):
        T = np.array(self.k.forward(np.array(q6), "matrix"))    # 3x4
        return [*T[:3, 3], *matrix_to_rotvec(T[:3, :3])]

    def ik(self, pose, q_near):
        T = np.eye(4)
        T[:3, 3] = pose[:3]
        T[:3, :3] = rotvec_to_matrix(pose[3], pose[4], pose[5])
        sol = self.k.inverse(T[:3, :].reshape(-1), q_guess=np.array(q_near))
        if sol is None:
            raise RuntimeError("ur_ikfast: nessuna soluzione IK per la posa richiesta.")
        return list(sol)


# ===========================================================================
# Esempio minimo
# ===========================================================================
if __name__ == "__main__":
    ur = UR3(use_ik=True)     # use_ik=False per provare solo giunti + I/O + pinza
    HOME = [0.0, -math.pi/2, math.pi/2, -math.pi/2, -math.pi/2, 0.0]
    ur.movej(HOME)
    ur.textmsg("TCP:", ur.get_actual_tcp_pose())
