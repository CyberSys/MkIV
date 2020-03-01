# Name
ARTIST www.katsbits.com, mod. Anton Gerdelan
DESIGNATION Universal Carrier MKII
NICKNAME Bren Gun Carrier

# Model
MESHFILE unicarrier.mesh
#DUST_EMITTER dust_l
#EXHAUST_EMITTER exhaust

# Animations

# Audio
SOUND_IDLE sherman-idle.mp3
SOUND_MOVE sherman-drive.mp3

# Scale
HULL_LENGTH_M 3.65 # Wikipedia article "Universal Carrier" - Livesey's is even longer.
HULL_WIDTH_M 2.11 # Wikipedia article "Universal Carrier"
AUTO_SCALE 1

# Performance
FULL_TURN_TIME_S 9 # made up
MAX_SLOPE_DEG 25.5 # http://www.wwiiequipment.com/
MAX_ROAD_KPH 47.07 # http://www.wwiiequipment.com/
MAX_OFFROAD_KPH 25.75 # http://www.wwiiequipment.com/
ZERO_TO_TOP_SPEED_S 10 # made up

# Armour
HULL_ARMOUR 10@90 7@90 5@90 0@0 # http://www.wwiiequipment.com/

# Armament

# Artificial Intelligence
FUZZY_SETS ai/carnav
FUZZY_RULES ai/default-fuzzy-rules.txt
#LOG_FITNESS_TO_FILE data/ai/logs/m4a2-log.txt
#TRAINING_WPS {1174.0, 278.0} {338.0, 359.0} {283.0,910.0}

CREW 2 # http://www.wwiiequipment.com/, Livesey gives 3
MAX_CREW 4 # http://www.wwiiequipment.com/

# Unused Extras
DESCRIPTION_START
Infantry carriers were designed to take an infantry
machine gun section into battle. In 1939 the
Universal Carrier replaced all existing models, and
35,000 would be produced by Britain alone during the
war. The carrier had an exposed V8 engine which was
incredible noisy. The driver and gunner have basic
front armour protection, but passengers were very
exposed. Numerous modifications were made officially,
and in the field. Carriers with heavier machine-guns,
mortar-launchers, and command vehicles were produced.
The Wasp was a very successful variant, mounting a
flamethrower with a range of 91.4m, and storing two
large flame tanks in the passenger compartment.
An amphibious flamethrower variant called the
"Dragonfly" used inflatable screens for buoyancy.
Thousands of these were produced. The carrier could
also tow the 57mm anti-tank gun (6pdr).
German-captured carriers were designated "Panzerjager
Bren".
References:
* Wikipedia article "Universal Carrier"
* http://www.wwiiequipment.com/
DESCRIPTION_END

#CREWMAN MESH ATTACH_TO_BONE
