#ifndef WATERPLANE_H_
#define WATERPLANE_H_

#include "Ogre.h"
#include "OgreConfigFile.h"
#include "OgreException.h"

using namespace Ogre;
using namespace std;

#define WATER_TYPE_NONE 0
#define WATER_TYPE_PLANE 1
#define WATER_TYPE_HYDRAX 2

struct WaterPlane;

class WaterManager {
public:
	WaterManager();
	bool free();
	bool createWaterPlane(const float& level);
	bool freeWaterPlane();
	bool updateWater(const unsigned long& elapsed);
	float getCurrentWaterHeight();
	
	float mMaxWaterLevel;
	int mWaterType;
private:
	WaterPlane* mpWaterPlane;
};

/* basic water plane -
 * largely based on this tutorial http://www.ogre3d.org/wiki/index.php/Tutorial_10
*/
struct WaterPlane {
	float mCurrentHeight;
	float mBaseHeight;
	float mFlowLimit;
	float mFlowSpeed;
	float mFlowAmount;
	bool mIsRising;
	bool mIsActive;
	
	WaterPlane();
	bool addWaterPlane(float height_m); // create a water plane/scene node
	bool destroyPlane();
	bool updateWater(unsigned long elapsed);
};

#endif

