# Template vehicle specification file

# Commented lines in this type of file start with '#'
# All other fields are optional. Defaults will be used if line removed

# Name
DESIGNATION Mk. IV
NICKNAME Big Willie

# Model
MESHFILE panzer3.mesh

# Special Effects
DUST_EMITTER bone_name
EXHAUST_EMITTER bone_name

# Animations
ANIM_IDLE animation_name
ANIM_MOVE animation_name
ANIM_DEATH animation_name

# Audio
SOUND_IDLE file.wav
SOUND_MOVE file.wav

# Dimensions
HULL_LENGTH_M meters
HULL_WIDTH_M meters
AUTO_SCALE 0 (no) or 1 (yes)

# Performance
FULL_TURN_TIME_S seconds
MAX_ROAD_KPH kilometers
MAX_OFFROAD_KPH kilometers
MAX_WATER_KPH kilometers
MAX_SLOPE_DEG degrees
ZERO_TO_TOP_SPEED_S seconds

# Main Cannon
CANNON_SOUND_FIRE file.wav
CANNON_ANIM_FIRE animation_name
CANNON_BONE_MUZZLE bone_name
CANNON_BONE_PIVOT bone_name
CANNON_RELOAD_TIME_S seconds
CANNON_ACCURACY 500m% 1000m% 1500m% 2000m% 2500m% 3000m% # base accuracy to hit normal sized target 2*2.5m
# mm of armour plate at 60 degrees from HORIZONTAL (30 degrees from vertical)
PENETRATION_mm 100m_mm 500m_mm 1000m_mm 1500m_mm 2000m_mm 2500m_mm 3000m_mm

# Turret
TURRET_TRAVERSE_BONE bone_name
TURRET_TIME_TO_TRAVERSE_360_S seconds

# Armour
# mm of thickness and degrees FROM VERTICAL must be given
# In our simulation a flat roof is mm@90 and a flat side is mm@0
HULL_ARMOUR front_mm@degrees rear_mm@degrees side_mm@degrees top_mm@degrees
TURRET_ARMOUR front_mm@degrees rear_mm@degrees side_mm@degrees top_mm@degrees

# Artificial Intelligence
FUZZY_SETS file
FUZZY_RULES file

DESCRIPTION_START
Try to limit lines to 55 characters
DESCRIPTION_END
