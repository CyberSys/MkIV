ARTIST Anton Gerdelan
DESIGNATION T-28E
NICKNAME Stalin's Monster
MESHFILE t-28.mesh
MAX_OCCUPANTS 4 # And 2 in the aux turrets
MAX_AUX_TURRETS 2

TURRET_TRAVERSE_BONE turret
#ANIM_IDLE idle
#ANIM_MOVE move
#ANIM_DEATH death
#CANNON_ANIM_FIRE fire_cannon
#CANNON_BONE_MUZZLE main_gun.muzzle
#CANNON_BONE_PIVOT main_gun.pivot

SOUND_IDLE crusader_idle.wav
SOUND_MOVE churchill_drive.wav
CANNON_SOUND_FIRE tankfire1.wav

FUZZY_SETS ai/carnav
FUZZY_RULES ai/default-fuzzy-rules.txt

HULL_LENGTH_M 7.44
HULL_WIDTH_M 2.81
AUTO_SCALE 1

FULL_TURN_TIME_S 15
MAX_ROAD_KPH 37
MAX_OFFROAD_KPH 22
MAX_SLOPE_DEG 45
ZERO_TO_TOP_SPEED_S 10

#DUST_EMITTER dust.003
#DUST_EMITTER dust.004
#EXHAUST_EMITTER smoke0
#EXHAUST_EMITTER smoke1

#CANNON_RELOAD_TIME_S

TURRET_TIME_TO_TRAVERSE_360_S 23
# this is electric, manually was ~240s

HULL_ARMOUR 80@65 40@85 40@90 15@0
TURRET_ARMOUR 80@90 40@90 40@90 15@0
PENETRATION_mm 63 53 51 40 30 20 10

### Not loaded by specloader yet ###
#TURRET_TRAVERSE_AUDIO 

DESCRIPTION_START
Russian medium tank intended for infantry support.
503 T-28s were built from 1932-1941.	
The T-28 is	a pre-war design of the 'land battleship'
philosophy; multiple turrets, with 4 or 5	machine guns
to enagage and occupy large groups of enemy infantry,
allowing friendly troops to make a break-through. The
T-28 also had a low-velocity 76.2mm main gun for
destroying enemy fortifications. The T-28 had some
advanced features for tanks of its era; a radio as
standard, and anti-aircraft machine gun mountings.
The vehicle first fought against the Japanese, and was
sent to action in Spain and Finland prior to the
Second World War. German-captured examples were
designated T-28 746(r). Suspension, transmission, and
engine were unreliable, and many T-28s were abandoned
due to breakdowns. Early models were vulnerable to
anti-tank rifles, light AT guns, and mines. This is the
up-gunned, up-armoured "ekanirovki" (E) version, which
was developed as a reaction to poor performance against
the Finns. The Soviets had 411 T-28s ready by the time
of the German invasion	in 1941, and by this stage
they were roughly equivalent to the German Panzer IV,
which was in 1941 also an infantry- support tank at
that time.

References:
* Fighting Vehicles of the World, P.Trewhitt&C.McNab ISBN 1-904687-03-2
* Wikipedia
* http://www.russianwarrior.com/STMMain.htm?1930vecindex_T28history.htm&1
* http://www.saunalahti.fi/~ejuhola/7.62/t28.html
* OnWar.com http://www.onwar.com/tanks/ussr/data/t28m34.htm
DESCRIPTION_END
