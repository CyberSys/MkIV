#include "WaterPlane.h"


WaterManager::WaterManager() {
	mpWaterPlane = NULL;
	mMaxWaterLevel = 0;
	mWaterType = WATER_TYPE_NONE;
}

bool WaterManager::free() {
	if (mWaterType == WATER_TYPE_PLANE) {
		return freeWaterPlane();
	}
	return true;
}

bool WaterManager::createWaterPlane(const float& level) {
	printf("  creating water plane...\n");
	if (mWaterType == WATER_TYPE_PLANE) {
		if (!freeWaterPlane()) {
			return false;
		}
	}
	if (mWaterType != WATER_TYPE_NONE) {
		printf("Water type was not empty - add code here to cleanup first! WaterManager::createWaterPlane\n");
		return true;
	}
	mpWaterPlane = new WaterPlane;
	if (!mpWaterPlane) {
		printf("ERROR: could not alloc water plane in Water::createWaterPlane\n");
		return false;
	}
	if (!mpWaterPlane->addWaterPlane(level)) {
		printf("ERROR: could not addWaterPlane in Water::createWaterPlane\n");
		return false;
	}
	mWaterType = WATER_TYPE_PLANE;
	mMaxWaterLevel = level + 2.0;
	printf("  water plane created.\n");
	return true;
}

bool WaterManager::freeWaterPlane() {
	if (!mpWaterPlane) {
		printf("ERROR: mpWaterPlane NULL in Water::freeWaterPlane\n");
		return false;
	}
	mpWaterPlane->destroyPlane();
	delete mpWaterPlane;
	mpWaterPlane = NULL;
	mWaterType = WATER_TYPE_NONE;
	return true;
}

bool WaterManager::updateWater(const unsigned long& elapsed) {
	if (mWaterType == WATER_TYPE_PLANE) {
		return mpWaterPlane->updateWater(elapsed);
	}
	return true;
}

float WaterManager::getCurrentWaterHeight() {
	if (mWaterType == WATER_TYPE_PLANE) {
		return mpWaterPlane->mCurrentHeight;
	}
	if (mWaterType == WATER_TYPE_HYDRAX) {
		printf("FATAL ERROR: cant get water height as no function here in manager!\n");
		exit(1);
	}
	return -1000.0;
}

WaterPlane::WaterPlane() {
	mBaseHeight = 0.0f;
	mCurrentHeight = 0.0f;
	mFlowLimit = 1.0f;
	mFlowSpeed = 0.00000025f;
	mFlowAmount = 0.0f;
	mIsRising = true;
	mIsActive = false;
}

// create a water plane/scene node
bool WaterPlane::addWaterPlane(float height_m) {
	mBaseHeight = height_m;
	Ogre::Plane waterPlane;
	waterPlane.normal = Vector3::UNIT_Y;
	waterPlane.d = -1.5;
	mCurrentHeight = mBaseHeight + 1.25f;
	Ogre::MeshManager::getSingleton().createPlane("WaterPlane", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, waterPlane, 1500, 1500, 20, 20, true, 1, 10, 10, Vector3::UNIT_Z);
	// TODO: validate the following:
	Ogre::Entity* waterEntity = Ogre::Root::getSingleton().getSceneManager("mkivSceneManager")->createEntity("WaterEntity", "WaterPlane");
	waterEntity->setMaterialName("water1");
	Ogre::SceneNode* tempNode = Ogre::Root::getSingleton().getSceneManager("mkivSceneManager")->getRootSceneNode()->createChildSceneNode("WaterNode");
	tempNode->attachObject(waterEntity);
	tempNode->translate(750, mBaseHeight, 750);
	mIsActive = true;
	return true;
}

bool WaterPlane::destroyPlane() {
	if (Ogre::MeshManager::getSingleton().resourceExists("WaterPlane")) {
		Ogre::MeshManager::getSingleton().remove("WaterPlane");
	}
	if (Ogre::Root::getSingleton().getSceneManager("mkivSceneManager")->hasEntity("WaterEntity")) {
		Ogre::Root::getSingleton().getSceneManager("mkivSceneManager")->destroyEntity("WaterEntity");
	}
	if (Ogre::Root::getSingleton().getSceneManager("mkivSceneManager")->hasSceneNode("WaterNode")) {
		Ogre::Root::getSingleton().getSceneManager("mkivSceneManager")->destroySceneNode("WaterNode");
	}
	return true;
}

bool WaterPlane::updateWater(unsigned long elapsed) {
	float levelDelta = mFlowSpeed * elapsed;
	if (mIsRising) {
		mFlowAmount += mFlowSpeed * elapsed;
	} else {
		mFlowAmount -= mFlowSpeed * elapsed;
	}
  if (mFlowAmount >= mFlowLimit) {
  	mIsRising = false;
  } else if (mFlowAmount <= 0.0f) {
		mIsRising = true;
	}
	if (mIsRising) {
		mCurrentHeight = mCurrentHeight + levelDelta;
	} else {
		mCurrentHeight = mCurrentHeight - levelDelta;
	}
	Ogre::SceneNode* waterNode = Ogre::Root::getSingleton().getSceneManager("mkivSceneManager")->getSceneNode("WaterNode");
	waterNode->translate(0, (mIsRising ? levelDelta : -levelDelta), 0);

	return true;
}

