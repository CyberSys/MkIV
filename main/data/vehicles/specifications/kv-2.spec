ARTIST kodo_day, mod. Anton Gerdelan
DESIGNATION KV-2 Heavy Tank
NICKNAME Marshal Klimenti Voroshilov 2

MESHFILE kv-2.mesh

ANIM_IDLE idle
ANIM_MOVE move

SOUND_IDLE crusader_idle.wav
SOUND_MOVE churchill_drive.wav

#CREW 6

FUZZY_SETS ai/carnav
FUZZY_RULES ai/default-fuzzy-rules.txt

HULL_LENGTH_M 6.79
HULL_WIDTH_M 3.32
AUTO_SCALE 1

FULL_TURN_TIME_S 15
MAX_SLOPE_DEG 30
MAX_ROAD_KPH 26
MAX_OFFROAD_KPH 16
ZERO_TO_TOP_SPEED_S 10

CANNON_SOUND_FIRE tankfire1.wav
CANNON_ANIM_FIRE fire
CANNON_BONE_MUZZLE gun.muzzle
#CANNON_BONE_PIVOT gun.pivot
#CANNON_RELOAD_TIME_S

DUST_EMITTER dust0
DUST_EMITTER dust1
EXHAUST_EMITTER exhaust0
EXHAUST_EMITTER exhaust1

TURRET_TRAVERSE_BONE turret
TURRET_TIME_TO_TRAVERSE_360_S 240
# appears to be manually operated so guessed ~240s although KV-1 is 30s http://www.armchairgeneral.com/rkkaww2/weapons/art_tanks.htm

HULL_ARMOUR 75@60 70@90 75@90 30@0
TURRET_ARMOUR 75@90 75@90 75@90 40@0
PENETRATION_mm 73 71 67 64 60 52 45
# note: this is actually a 40kg anti-concrete shell (36 rounds carried)

### Not loaded by specloader yet ###
#CREWMAN MESH ATTACH_TO_BONE
DESCRIPTION_START
A specialised variant of the KV-1 Heavy Tank, the
KV-2 mounted a colossal turret containing a 152mm
howitzer, firing a gigantic 40kg anti-concrete shell.
The KV-2 was intended as a weapon for destroying
pillboxes and fortifications. It used the same
chassis and engine as the KV-1, but given the extra
weight it was slower and even more unreliable. The
Germans found the KV-2 a large target that was easy
to immobilise.

References:
* George Forty, "Tanks of World Wars I and II", Southwater, 2006
* kodo_day http://www.turbosquid.com/FullPreview/Index.cfm/ID/306891
DESCRIPTION_END
