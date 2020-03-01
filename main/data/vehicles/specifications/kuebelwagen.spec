# Name
ARTIST www.katsbits.com, mod. Anton Gerdelan
DESIGNATION Volkswagen Type 82
NICKNAME Kübelwagen

# Model
MESHFILE kuebelwagen.mesh
#CANNON_BONE_MUZZLE main_gun.muzzle
#CANNON_BONE_PIVOT main_gun.pivot
#DUST_EMITTER dust_l
#DUST_EMITTER dust_r
#EXHAUST_EMITTER exhaust

UNIT_TYPE LIGHT_VEHICLE

# Animations
#ANIM_IDLE idle
#ANIM_MOVE move
#CANNON_ANIM_FIRE fire

# Audio
SOUND_IDLE sherman-idle.mp3
SOUND_MOVE sherman-drive.mp3
#CANNON_SOUND_FIRE tankfire1.wav

# Scale
HULL_LENGTH_M 3.74 # Wikipedia article "Volkswagen Kübelwagen"
HULL_WIDTH_M 1.6 # Wikipedia article "Volkswagen Kübelwagen"
AUTO_SCALE 1

# Performance
FULL_TURN_TIME_S 9
MAX_SLOPE_DEG 25.5 # http://www.wwiiequipment.com/
MAX_ROAD_KPH 80 # http://www.wwiiequipment.com/
MAX_OFFROAD_KPH 35 # http://www.wwiiequipment.com/
#MAX_WATER_KPH 0
#TURRET_TIME_TO_TRAVERSE_360_S 15.9
#TURRET_TRAVERSE_BONE turret
ZERO_TO_TOP_SPEED_S 10

# Armour
#HULL_ARMOUR 10@90 7@90 5@90 0@0 # http://www.wwiiequipment.com/

# Armament
#PENETRATION_mm 76 63 51 43 35 25 12

# Artificial Intelligence
FUZZY_SETS ai/carnav
FUZZY_RULES ai/default-fuzzy-rules.txt
#LOG_FITNESS_TO_FILE data/ai/logs/m4a2-log.txt
#TRAINING_WPS {1174.0, 278.0} {338.0, 359.0} {283.0,910.0}

#CREW 2 # http://www.wwiiequipment.com/
#MAX_CREW 4 # http://www.wwiiequipment.com/
MAX_OCCUPANTS 8

# Unused Extras
DESCRIPTION_START
The original Kuebelwagen ("bucket car") was designed
by Ferdinand Porsche in only a month to meet Hitler's
requirement for a rugged off-road military varient of
the VW Beetle. Kuebelwagen was the German equivalent
of the Jeep, and altough it was only 2-wheel drive
it proved to be a robust, reliable off-road vehicle.
Over 50,000 were produced 1940-1945.

References:
* Wikipedia article "Volkswagen Kübelwagen"
* 1944 Technical Manual at http://www.allworldwars.com/
DESCRIPTION_END

#CREWMAN MESH ATTACH_TO_BONE
