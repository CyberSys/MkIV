ARTIST kodo_day, mod. Anton Gerdelan
DESIGNATION T-70 Model 1942
NICKNAME T-70 Light Tank
MESHFILE t-70.mesh

#CREW 2

#ANIM_IDLE idle
#ANIM_MOVE move
#SOUND_IDLE crusader_idle.wav
SOUND_MOVE t-70drive.wav
FUZZY_SETS ai/carnav
FUZZY_RULES ai/default-fuzzy-rules.txt

HULL_LENGTH_M 4.29
HULL_WIDTH_M 2.32
AUTO_SCALE 1

FULL_TURN_TIME_S 15
MAX_SLOPE_DEG 34
MAX_ROAD_KPH 45
MAX_OFFROAD_KPH 29.93
ZERO_TO_TOP_SPEED_S 10
EXHAUST_EMITTER exhaust
DUST_EMITTER dust0
DUST_EMITTER dust1
CANNON_SOUND_FIRE tankfire1.wav
PENETRATION_mm 48 35 28 23 19 15 10
# this is for 30 degrees. Figures from sources vary widly. this is http://www.freeweb.hu/gva/weapons/soviet_guns3.html
# ammo dist '43-'44: HE:56, AP:22, APCR: 12
HULL_ARMOUR 45@60 35@50 45@90 10@0
TURRET_ARMOUR 60@55 35@60 35@65 10@0

CANNON_SOUND_FIRE tankfire1.wav
#CANNON_ANIM_FIRE fire_cannon
CANNON_BONE_MUZZLE muzzle
CANNON_BONE_PIVOT mantlet
#CANNON_RELOAD_TIME_S
TURRET_TRAVERSE_BONE turret
#manual turret traverse
TURRET_TIME_TO_TRAVERSE_360_S 240

### Not loaded by specloader yet ###
#CREWMAN MESH ATTACH_TO_BONE
DESCRIPTION_START
Soviet light tank with 2-man crew. More than 8000
produced '42-'43. The Soviets realised that their
light tanks were obsolete versus German armour,
however smaller factories were not able to produce the
T-34 tank. The T-70 is was a balanced engineering
decision to maximise tank production by producing a
light tank with a more powerful gun that the obsolete
T-60 scout, and at lower cost than the T-50 light
infantry tank. The T-70's commander was also the
gunner and loader, which made group co-ordination
impossible. The T-70 was also developed into an anti-
aircraft vehicle (the T-90) and a widened version of
the chassis was used for the SU-76 anti-tank vehicle.
The Soviets cancelled all light tank production in
1943 and shifted production to the SU-76.

References:
* http://en.wikipedia.org/wiki/T-70
* http://www.wwiivehicles.com/ussr/tanks-light/t-70.asp
* http://www.figuras.miniatures.de/shells-russian.html
* kodo_day http://www.turbosquid.com/3d-models/3dsmax-t-70/306256
DESCRIPTION_END
