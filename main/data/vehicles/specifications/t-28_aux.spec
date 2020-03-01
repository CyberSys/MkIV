ARTIST Anton Gerdelan
DESIGNATION T-28 Aux. Turret
NICKNAME T-28 Auxilliary Turret
MESHFILE t-28_aux.mesh
MAX_OCCUPANTS 1
UNIT_TYPE AUX_TURRET

#CREW 6

#ANIM_IDLE idle
#ANIM_MOVE move
#ANIM_DEATH death

#SOUND_IDLE crusader_idle.wav
#SOUND_MOVE churchill_drive.wav

#FUZZY_SETS ai/carnav
#FUZZY_RULES ai/default-fuzzy-rules.txt

HULL_LENGTH_M 2
HULL_WIDTH_M 1
AUTO_SCALE 1

#FULL_TURN_TIME_S 15
#MAX_ROAD_KPH 37
#MAX_OFFROAD_KPH 22
#MAX_SLOPE_DEG 45
#ZERO_TO_TOP_SPEED_S 10

#DUST_EMITTER dust.003
#DUST_EMITTER dust.004
#EXHAUST_EMITTER smoke0
#EXHAUST_EMITTER smoke1

#CANNON_SOUND_FIRE tankfire1.wav
#CANNON_ANIM_FIRE fire_cannon
#CANNON_BONE_MUZZLE main_gun.muzzle
#CANNON_BONE_PIVOT main_gun.pivot
#CANNON_RELOAD_TIME_S

#TURRET_TRAVERSE_BONE main_turret
#TURRET_TIME_TO_TRAVERSE_360_S 23
# this is electric, manually was ~240s

#HULL_ARMOUR 80@65 40@85 40@90 15@0
#TURRET_ARMOUR 80@90 40@90 40@90 15@0
#PENETRATION_mm 63 53 51 40 30 20 10

### Not loaded by specloader yet ###
#TURRET_TRAVERSE_AUDIO 
#CREWMAN MESH ATTACH_TO_BONE
#CREWMAN crewman_ant.mesh commander
#CREWMAN crewman_dan.mesh gunner
#CREWMAN crewman_arno.mesh driver
#CREWMAN crewman_ken.mesh left_aux_gunner
#CREWMAN crewman_guy.mesh right_aux_gunner
DESCRIPTION_START
Auxilliary turret for the T-28 tank. These were
removed in the field on later models.

References:
* Fighting Vehicles of the World, P.Trewhitt&C.McNab ISBN 1-904687-03-2
* Wikipedia
* http://www.russianwarrior.com/STMMain.htm?1930vecindex_T28history.htm&1
* http://www.saunalahti.fi/~ejuhola/7.62/t28.html
* OnWar.com http://www.onwar.com/tanks/ussr/data/t28m34.htm
DESCRIPTION_END
