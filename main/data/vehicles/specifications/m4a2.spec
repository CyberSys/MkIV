# Name
ARTIST Anton Gerdelan
DESIGNATION Medium Tank M4A2
NICKNAME Sherman III

#CREW 5 + 18 passengers

# Model
MESHFILE m4a2.mesh
#CANNON_BONE_MUZZLE main_gun.muzzle
#CANNON_BONE_PIVOT main_gun.pivot
#DUST_EMITTER dust_l
#DUST_EMITTER dust_r
#EXHAUST_EMITTER exhaust

# Animations
ANIM_IDLE idle
ANIM_MOVE move
#CANNON_ANIM_FIRE fire

MAX_OCCUPANTS 23

# Audio
SOUND_IDLE sherman-idle.mp3
SOUND_MOVE sherman-drive.mp3
CANNON_SOUND_FIRE tankfire1.wav

# Scale
HULL_LENGTH_M 5.92
HULL_WIDTH_M 2.62
AUTO_SCALE 1

# Performance
FULL_TURN_TIME_S 9
MAX_SLOPE_DEG 31
MAX_ROAD_KPH 39
MAX_OFFROAD_KPH 34
MAX_WATER_KPH 0
TURRET_TIME_TO_TRAVERSE_360_S 15.9
TURRET_TRAVERSE_BONE turret
ZERO_TO_TOP_SPEED_S 10

# Armour
HULL_ARMOUR 80@34 38@90 38@90 13@0
TURRET_ARMOUR 80@60 51@90 51@85 25@0

# Armament
PENETRATION_mm 76 63 51 43 35 25 12

# Artificial Intelligence
FUZZY_SETS ai/carnav
FUZZY_RULES ai/g250rules.txt
LOG_FITNESS_TO_FILE data/ai/logs/log.txt
#TRAINING_WPS {1174.0, 278.0} {338.0, 359.0} {283.0,910.0}
TRAINING_WPS {1251.0, 357.0} {192.0, 366.0} {259.0,976.0}

# Unused Extras
DESCRIPTION_START
The M4 Medium was the most widely-used Allied tank,
with 49,234 produced by the USA; more than the
combined total wartime production of all German and
British tanks. It was the clear offspring of the M3 
Medium, using the same basic hull and armament, except
mounted in a fully traversing main turret. The M4
Medium entered service first with the British in
North Africa in 1941 who designated it the "General
Sherman". The M4 was extremely reliable, and was
produced until 1944 with 6 basic models {M4-M4A6}
which were increasingly up-armoured and up-gunned. M4
caught fire easily when hit, and was nicknamed  the
"Ronson Lighter" by troops - "Guaranteed to Light
First Time". The Germans called it the "Tommy Cooker".
Later versions attempted to remedy this. A number of
M4s were even exported to the Soviet Union under the
lend-lease programme.

The M4A2 was known as the "Sherman III" by the
British, and had a diesel engine. This model also had
much thicker hull front armour. The US Marine	Corps
used the M4A2 in the Pacific, but it was largely
intended to be a Lend-Lease export. 8053 were produced
from 1942-1944.

References:
* George Forty, "Tanks of World Wars I and II", Southwater, 2006
* Trewhitt, P. & McNab, C., "Fighting Vehicles of the World", Amber Books, 2004
* AFV Database http://afvdb.50megs.com/usa/m4sherman.html
* http://en.wikipedia.org/wiki/M4_Sherman
* OnWar.com http://www.onwar.com/tanks/usa/fm4.htm
* Wikipedia article on 75mm guns http://en.wikipedia.org/wiki/75_mm_gun_(US)
* freeweb.hu http://gva.freeweb.hu/weapons/usa_guns5.html
* http://www.armchairgeneral.com/rkkaww2/weapons/art_tanks.htm

Interesting Links
* M4 battle stats http://www.weaponsofwwii.com/forum/viewtopic.php?f=10&t=1961
DESCRIPTION_END

#CREWMAN MESH ATTACH_TO_BONE
