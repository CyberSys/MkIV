# Name
ARTIST Anton Gerdelan
DESIGNATION Duplex Drive M4
NICKNAME Donald Duck tank
UNIT_TYPE AMPHIBIAN
WATERLINE_OFFSET_M 1.5

#MAX_OCCUPANTS 23 # (5 crew + 18 passengers)

# Model
MESHFILE dd-sherman.mesh
#CANNON_BONE_MUZZLE main_gun.muzzle
#CANNON_BONE_PIVOT main_gun.pivot
DUST_EMITTER dust_l
DUST_EMITTER dust_r
WAKE_EMITTER exhaust
EXHAUST_EMITTER exhaust

# Animations
#ANIM_IDLE idle
#ANIM_IDLE bob
#ANIM_MOVE move
#ANIM_MOVE swim
#CANNON_ANIM_FIRE fire

# Audio
SOUND_IDLE sherman-idle.mp3
SOUND_MOVE sherman-drive.mp3
CANNON_SOUND_FIRE tankfire1.wav

# Scale
HULL_LENGTH_M 5.92 # Forty gives 5.88 but we also have the cowling on this model
HULL_WIDTH_M 2.68 # Forty
AUTO_SCALE 1

# Performance
FULL_TURN_TIME_S 9 # based on M4A2
MAX_SLOPE_DEG 31 # based on M4A2
MAX_WATER_KPH 7.41 # 4 knots in kph, although another source gave 7kph
MAX_ROAD_KPH 39 # Forty
MAX_OFFROAD_KPH 34 # based on the M4A2 - although this sounds very very fast
TURRET_TIME_TO_TRAVERSE_360_S 15.9 # based on M4A2
TURRET_TRAVERSE_BONE turret
ZERO_TO_TOP_SPEED_S 10 # just made up - probably a lot longer than this

# Armour
HULL_ARMOUR 51@34 38@90 38@90 19@0 # based on mid-production M4
TURRET_ARMOUR 76@60 51@90 51@85 25@0 # based on mid-production M4

# Armament
PENETRATION_mm 76 63 51 43 35 25 12

# Artificial Intelligence
FUZZY_SETS ai/carnav
FUZZY_RULES ai/default-fuzzy-rules.txt
LOG_FITNESS_TO_FILE log-ddsherman-fitness.txt

# Unused Extras
DESCRIPTION_START
The Duplex Drive (DD) was a modified American M4 tank
designed by General Percy Hobart, who was tasked with
creating specialised tanks ("Hobart's Funnies").
The DD was fitted with collapsable floatation screens
and twin propellers. It could move at 4 knots in the
water. The Nicholas Straussler-designed screens took
15 minutes to inflate but collapsed quickly on arrival
at shore.
The DD was assigned to 10 Allied tank battalions during
Operation Overlord on June 6th. Of 29 DD tanks launched
into the water at Omaha only two examples successfully
made it to the beach. The rest sank in the choppy
chanel waters during their 6km ship-to-land voyage and
remain as grim curiosities for divers.

References:
* George Forty, "Tanks of World Wars I and II", Southwater.
* http://www.sproe.com/t/tank-dd.html
* Wikipedia.org, "DD Tank".
* AFV DB http://afvdb.50megs.com/usa/m4sherman.html
DESCRIPTION_END

#CREWMAN MESH ATTACH_TO_BONE
