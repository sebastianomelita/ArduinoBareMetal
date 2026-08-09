"""
Pick & Place - traduzione dell'albero PolyScope nel wrapper UR3 (Webots).

Albero originale:
  BeforeStart
    MoveJ -> riposo
  Robot Program
    MoveJ -> approccioPick
      MoveL -> pick ; Wait 0.5 ; approccioPick
    MoveJ -> approccioPlace           (nota: e' figlio del MoveJ iniziale)
      MoveL -> place ; Wait 0.5 ; approccioPlace
"""

import math
from ur3_wrapper import UR3

# ---------------------------------------------------------------------------
# WAYPOINT - da tarare sulla tua scena.
#  - I punti raggiunti con MoveJ possono stare in GIUNTI [rad] (piu' fedele a PolyScope,
#    che memorizza i waypoint come configurazioni di giunto).
#  - I punti raggiunti con MoveL li esprimo come POSE [x,y,z,rx,ry,rz] (asse-angolo),
#    perche' il movimento lineare ragiona nello spazio cartesiano.
# ---------------------------------------------------------------------------
riposo         = [0.0, -math.pi/2,  math.pi/2, -math.pi/2, -math.pi/2, 0.0]   # giunti
approccioPick  = [0.3, -math.pi/3,  math.pi/2, -math.pi/2, -math.pi/2, 0.0]   # giunti
approccioPlace = [-0.6, -math.pi/3, math.pi/2, -math.pi/2, -math.pi/2, 0.0]   # giunti

pick  = [0.30, -0.20, 0.05, 0.0, math.pi, 0.0]   # pose cartesiana (esempio)
place = [-0.30, -0.20, 0.05, 0.0, math.pi, 0.0]  # pose cartesiana (esempio)


def before_start(ur):
    ur.movej(riposo, a=1.4, v=1.05)


def robot_program(ur):
    # --- PICK ---
    ur.movej(approccioPick, a=1.4, v=1.05)          # MoveJ -> approccioPick
    ur.movel(pick, a=1.2, v=0.25)                   # MoveL -> pick
    ur.set_digital_out(0, True)                     # (presa: chiudi pinza)
    ur.sleep(0.5)                                   # Wait: 0.5
    ur.movel(_approach_of(pick), a=1.2, v=0.25)     # MoveL -> approccioPick (risalita)

    # --- PLACE ---
    ur.movej(approccioPlace, a=1.4, v=1.05)         # MoveJ -> approccioPlace
    ur.movel(place, a=1.2, v=0.25)                  # MoveL -> place
    ur.set_digital_out(0, False)                    # (rilascio: apri pinza)
    ur.sleep(0.5)                                   # Wait: 0.5
    ur.movel(_approach_of(place), a=1.2, v=0.25)    # MoveL -> approccioPlace (risalita)


def _approach_of(pose, dz=0.10):
    """Punto di approccio = stessa pose ma alzata di dz in Z."""
    p = list(pose)
    p[2] += dz
    return p


if __name__ == "__main__":
    ur = UR3(use_ik=True)     # serve ikpy + URDF per i MoveL; per solo giunti usa use_ik=False
    before_start(ur)          # eseguito una volta sola
    while True:               # il Robot Program di PolyScope cicla
        robot_program(ur)
