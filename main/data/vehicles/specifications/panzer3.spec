ARTIST Anton Gerdelan
DESIGNATION PzKpfw III Ausf F (late)
NICKNAME Panzer III
MESHFILE panzer3.mesh

# CREW 5

#ANIM_IDLE idle
#ANIM_MOVE move
SOUND_IDLE panther-idle.mp3
SOUND_MOVE panther.wav
FUZZY_SETS ai/carnav
FUZZY_RULES ai/default-fuzzy-rules.txt
HULL_LENGTH_M 5.38
HULL_WIDTH_M 2.91
AUTO_SCALE 1
FULL_TURN_TIME_S 15
MAX_ROAD_KPH 40
MAX_OFFROAD_KPH 19
MAX_SLOPE_DEG 30
ZERO_TO_TOP_SPEED_S 10

CANNON_SOUND_FIRE tankfire1.wav

HULL_ARMOUR 30@69 21@80 30@90 16@0
TURRET_ARMOUR 30@75 30@90 30@65 12@0
#mantlet 30
# PENETRATION_mm 54 46 36 28 22 16 12 # with the 39 ammo
# PENETRATION_mm 96 80 58 0 0 0 # with the 40 ammo
PENETRATION_mm 96 80 58 28 22 16 12 # mixed ammo
ACCURACY 100 100 96 26 10 3 1

### Not loaded by specloader yet ###
#CREWMAN MESH ATTACH_TO_BONE
DESCRIPTION_START
German medium tank with anti-tank gun.
435 of the Ausf F model were produced from 1939-1940.
Designed to work in complement with the Panzer IV
infantry-support tank. The roles were reversed later
in the war.
Later models of the Ausf F were mounted with a 50mm
main gun to replace to 37mm gun of earlier models.
The tank was superior to most enemy armour
encountered, but the gun was not effective against the
T-34 and KV-1.
The front of the Panzer III was almost impervious to
1941-1942 British and Russian AT rounds, except at
very close range.
Early-war models had two co-axial MG34s and a hull MG34.
References:
* Tanks of World Wars I and II - George Forty
* Fighting Vehicles of the World, P.Trewhitt & C.McNab ISBN 1-904687-03-2
* Wikipedia
* http://www.onwar.com/tanks/germany/data/pz3f.htm
* http://www.achtungpanzer.com/pz8.htm
* Accuracy data: http://boards.avalonhill.com/showpost.php?p=95080&postcount=15
DESCRIPTION_END

