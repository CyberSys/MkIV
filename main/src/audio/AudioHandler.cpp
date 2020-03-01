#include "AudioHandler.h"
#include <fmod_errors.h>

#define INVALID_SOUND_CHANNEL -1

#define INITIAL_VECTOR_SIZE   100 // originally was 100
#define INCREASE_VECTOR_SIZE  20

#define DOPPLER_SCALE         1.0 //?
#define DISTANCE_FACTOR       1.0 // (was 1.0)
#define ROLLOFF_SCALE         1.0 // originally was 0.5
/* previous values:
#define INITIAL_VECTOR_SIZE   100 // originally was 100
#define INCREASE_VECTOR_SIZE  20

#define DOPPLER_SCALE         2.0 //?
#define DISTANCE_FACTOR       75.0 // (was 1.0)
#define ROLLOFF_SCALE         5.0 // originally was 0.5
*/
/* tested values
dist: 100, roll: 10 = okay but still not loud enough. close was over amped (crackling) and dropoff too steep
dist: 75, roll: 5 = good. can hear interesting parts of gears. not loud enough overall but not too bad. rolloff good.
Note: looks like many tanks in one place massively increases amplitude above what normally is (perhaps not out of synch enough)
--it feels like there is a 'full loud' band of about 20m, then a fading off abruptly. it needs to be about 2x as loud at origin,
but scale off from point zero to about twice as far.
*/

template<> SoundManager* Singleton<SoundManager>::ms_Singleton = 0;

void SoundInstance::clear() {
	strcpy(mFileName, "");
	mStreamPtr.setNull();
	mpFmodSound = NULL;
	mSoundType = SOUND_TYPE_INVALID;
}

void ChannelInstance::clear() {
	strcpy(mSceneNodeHandle, "");
	mPrevPos.x = mPrevPos.y = mPrevPos.z = 0.0;
	mAttachedToNode = false;
}

SoundManager::SoundManager() {
	mpSystem = NULL;
	mPrevListenerPosition.x = mPrevListenerPosition.y = mPrevListenerPosition.z = 0.0;
	for (int i = 0; i < MAX_SOUND_CHANNELS; i++) {
      mChannelArray[i].clear();
  }
	for (int i = 0; i < MAX_SOUND_INSTANCES; i++) {
		mSoundInstanceArray[i].clear();
	}
	mSoundInstanceArrayEnd = 0;
	
	mButtonClickSoundIndex = 0;
}

SoundManager::~SoundManager() {
	if (mpSystem) {
		FMOD_System_Release(mpSystem);
	}      
}

SoundManager* SoundManager::getSingletonPtr() {
	return ms_Singleton;
}

SoundManager& SoundManager::getSingleton() {
	assert(ms_Singleton);
	return (*ms_Singleton);
}

bool SoundManager::init() {
	printf("Sound manager initialising...\n");
	// Create the main system object.
	FMOD_RESULT result = FMOD_System_Create(&mpSystem); 
	if (result != FMOD_OK) {
		printf("ERROR: could not create sound system\n");
		printf(WHERESTR "\n", WHEREARG);
		return false;
	}
	// Initialize FMOD.
	result = FMOD_System_Init(mpSystem, MAX_SOUND_CHANNELS, FMOD_INIT_NORMAL, 0);
	if (result != FMOD_OK) {
		printf("ERROR: could not init sound system\n");
		printf(WHERESTR "\n", WHEREARG);
		return false;
	}
	// set FMOD parameters
	FMOD_System_Set3DSettings(mpSystem, DOPPLER_SCALE, DISTANCE_FACTOR, ROLLOFF_SCALE);
	mEnabled = true;
	loadMenuSounds();
	printf("Sound manager initialised\n");
	return true;
}

bool SoundManager::loadMenuSounds() {
	printf(" loading menu sounds...\n");
	mButtonClickSoundIndex = createStream("data/audio/button_click0.mp3");
	printf(" menu sounds loaded.\n");
	return true;
}

void SoundManager::stopAllSounds() {
	if (!mEnabled) {
		return;
	}
	int channelIndex;
	FMOD_RESULT result;
	FMOD_CHANNEL* nextChannel;

	for (channelIndex = 0; channelIndex < MAX_SOUND_CHANNELS; channelIndex++) {
		result = FMOD_System_GetChannel(mpSystem, channelIndex, &nextChannel);
		if ((result == FMOD_OK) && (nextChannel != NULL)) {
			FMOD_Channel_Stop(nextChannel);
	  }
		mChannelArray[channelIndex].clear();
	}
}

bool SoundManager::update(Ogre::Camera *camera, const unsigned long &timeElapsed) {
	if (!mEnabled) {
		return true;
	}
	int channelIndex;
	FMOD_CHANNEL* nextChannel;
	FMOD_VECTOR listenerPosition;
	FMOD_VECTOR listenerForward;
	FMOD_VECTOR listenerUp;
	FMOD_VECTOR listenerVelocity;
	Ogre::Vector3  vectorVelocity;
	Ogre::Vector3  vectorForward;
	Ogre::Vector3  vectorUp;

	if (timeElapsed > 0) {
	  vectorVelocity = (camera->getPosition() - mPrevListenerPosition) / timeElapsed;
	} else {
	  vectorVelocity = Vector3(0, 0, 0);
	}
	vectorForward = camera->getRealDirection();
	vectorForward.normalise();
	vectorUp = camera->getRealUp();
	vectorUp.normalise();

	listenerPosition.x = camera->getPosition().x;
	listenerPosition.y = camera->getPosition().y;
	listenerPosition.z = camera->getPosition().z;
	listenerForward.x = vectorForward.x;
	listenerForward.y = vectorForward.y;
	listenerForward.z = vectorForward.z;
	listenerUp.x = vectorUp.x;
	listenerUp.y = -vectorUp.y;
	listenerUp.z = vectorUp.z;
	listenerVelocity.x = vectorVelocity.x;
	listenerVelocity.y = vectorVelocity.y;
	listenerVelocity.z = vectorVelocity.z;

	// update 'ears'
	FMOD_System_Set3DListenerAttributes(mpSystem, 0, &listenerPosition,  &listenerVelocity, &listenerForward, &listenerUp);
	FMOD_System_Update(mpSystem);

	mPrevListenerPosition = camera->getPosition();

	for (channelIndex = 0; channelIndex < MAX_SOUND_CHANNELS; channelIndex++) {
	  if (mChannelArray[channelIndex].mAttachedToNode) {
	  	FMOD_System_GetChannel(mpSystem, channelIndex, &nextChannel);
	  	
	  	// get the scene node for the sound source
	  	if (!Ogre::Root::getSingleton().getSceneManager("mkivSceneManager")->hasSceneNode(mChannelArray[channelIndex].mSceneNodeHandle)) {
	  		printf("ERROR: sound manager can not find scene node %s for sound channel\n", mChannelArray[channelIndex].mSceneNodeHandle);
	  		printf(WHERESTR "\n", WHEREARG);
	  		return false;
	  	}
	  	Ogre::SceneNode* sn = Ogre::Root::getSingleton().getSceneManager("mkivSceneManager")->getSceneNode(mChannelArray[channelIndex].mSceneNodeHandle);
	  	
			if (timeElapsed > 0) {
				vectorVelocity = (sn->_getDerivedPosition() - mChannelArray[channelIndex].mPrevPos) / timeElapsed;
			} else {
				vectorVelocity = Vector3(0, 0, 0);
			}
			listenerPosition.x = sn->_getDerivedPosition().x;
			listenerPosition.y = sn->_getDerivedPosition().y;
			listenerPosition.z = sn->_getDerivedPosition().z;

			listenerVelocity.x = vectorVelocity.x;
			listenerVelocity.y = vectorVelocity.y;
			listenerVelocity.z = vectorVelocity.z;

			FMOD_Channel_Set3DAttributes(nextChannel, &listenerPosition, &listenerVelocity);
			mChannelArray[channelIndex].mPrevPos = sn->_getDerivedPosition();
		}
  }
  return true;
}

int SoundManager::createStream(const char* filename) {
	if (!mEnabled) {
		return -1;
	}
	int result = createSound(filename, SOUND_TYPE_2D_SOUND);
	printf("Looped 2D sound created at index [%i]\n", result);
	return result;
}

int SoundManager::createSound(const char* filename) {
	if (!mEnabled) {
		return -1;
	}
	int index = createSound(filename, SOUND_TYPE_3D_SOUND);
	set3DMinMaxDistance(index, 10.0f, 10000.0f);
	return  index;
}

int SoundManager::createLoopedSound(const char* filename) {
	if (!mEnabled) {
		return -1;
	}
	int index = createSound(filename, SOUND_TYPE_3D_SOUND_LOOPED);
	set3DMinMaxDistance(index, 10.0f, 10000.0f);
	return index;
}

int SoundManager::createLoopedStream(const char* filename) {
	printf("Creating looped 2D sound...\n");
	if (!mEnabled) {
		return -1;
	}
	int result = createSound(filename, SOUND_TYPE_2D_SOUND_LOOPED);
	printf("Looped 2D sound created at index [%i]\n", result);
	return result;
}

int SoundManager::createSound(const char* fileName, SOUND_TYPE soundType) {
	if (!mEnabled) {
		return -1;
	}
	printf("Creating sound from %s\n", fileName);
	// check if sound already exists
  int soundIndex;
  soundIndex = findSound(fileName, soundType);
	
  strcpy(mSoundInstanceArray[mSoundInstanceArrayEnd].mFileName, fileName);
  mSoundInstanceArray[mSoundInstanceArrayEnd].mSoundType = soundType;

	FMOD_RESULT result;
	if (soundType == SOUND_TYPE_3D_SOUND) {
		result = FMOD_System_CreateSound(mpSystem, fileName, FMOD_3D | FMOD_SOFTWARE, 0, &mSoundInstanceArray[mSoundInstanceArrayEnd].mpFmodSound);
	} else if (soundType == SOUND_TYPE_3D_SOUND_LOOPED) {
		result = FMOD_System_CreateSound(mpSystem, fileName, FMOD_LOOP_NORMAL | FMOD_3D | FMOD_SOFTWARE, 0, &mSoundInstanceArray[mSoundInstanceArrayEnd].mpFmodSound);
	} else if (soundType == SOUND_TYPE_2D_SOUND) {
		result = FMOD_System_CreateStream(mpSystem, fileName, FMOD_DEFAULT, 0, &mSoundInstanceArray[mSoundInstanceArrayEnd].mpFmodSound);
	} else if (soundType == SOUND_TYPE_2D_SOUND_LOOPED) {
		result = FMOD_System_CreateStream(mpSystem, fileName, FMOD_LOOP_NORMAL | FMOD_2D | FMOD_HARDWARE, 0, &mSoundInstanceArray[mSoundInstanceArrayEnd].mpFmodSound);
	} else {
		printf("ERROR: could not create sound of this type\n");
		printf(WHERESTR "\n", WHEREARG);
		return -1;
	}
	if (result != FMOD_OK) {
		printf("ERROR: could not create sound %s\n", fileName);
		printf(WHERESTR "\n", WHEREARG);
	}
	/*printf("Created sound %s at index %i\n", mSoundInstanceArray[mSoundInstanceArrayEnd].mFileName, mSoundInstanceArrayEnd);
  mSoundInstanceArrayEnd++;*/
  int indexUsed = mSoundInstanceArrayEnd;
  mSoundInstanceArrayEnd++;
  return indexUsed;
}

bool SoundManager::playSoundB(int soundIndex, Ogre::SceneNode *soundNode, int *channelIndex) {
	if (!mEnabled) {
		return true;
	}
	int	channelIndexTemp;
	if (channelIndex) {
	  channelIndexTemp = *channelIndex;
	} else {
		channelIndexTemp = INVALID_SOUND_CHANNEL;
	}
	
	FMOD_RESULT result;
	FMOD_CHANNEL* channel;
	// If the channelIndex already has sound assigned, test if same sceneNode.
	if ((channelIndexTemp != INVALID_SOUND_CHANNEL) && (mChannelArray[channelIndexTemp].mAttachedToNode)) {
		// get the scene node for the sound source
		if (!Ogre::Root::getSingleton().getSceneManager("mkivSceneManager")->hasSceneNode(mChannelArray[channelIndexTemp].mSceneNodeHandle)) {
			printf("ERROR: sound manager can not find scene node %s for sound channel\n", mChannelArray[channelIndexTemp].mSceneNodeHandle);
			printf(WHERESTR "\n", WHEREARG);
			return false;
		}
		Ogre::SceneNode* sn = Ogre::Root::getSingleton().getSceneManager("mkivSceneManager")->getSceneNode(mChannelArray[channelIndexTemp].mSceneNodeHandle);
	
    result = FMOD_System_GetChannel(mpSystem, channelIndexTemp, &channel);
    if (result == FMOD_OK) {
	    FMOD_BOOL isPlaying;
      result = FMOD_Channel_IsPlaying(channel, &isPlaying);
			if ((result == FMOD_OK) && (isPlaying == true) && (sn == soundNode)) {
				return true; // Already playing this sound attached to this node.
			}
		}
	}

  // Start the sound paused
  result = FMOD_System_PlaySound(mpSystem, FMOD_CHANNEL_FREE, mSoundInstanceArray[soundIndex].mpFmodSound, true, &channel);
	if (result != FMOD_OK) {
		printf("ERROR: sound manager can not play sound [%s] at index [%i]\n", mSoundInstanceArray[soundIndex].mFileName, soundIndex);
		printf(WHERESTR "\n", WHEREARG);
    if (channelIndex) {
      *channelIndex = INVALID_SOUND_CHANNEL;
		}
    return false;
  }

  FMOD_Channel_GetIndex(channel, &channelIndexTemp);
  
	FMOD_VECTOR initialPosition;
	if (soundNode) {
    strcpy(mChannelArray[channelIndexTemp].mSceneNodeHandle, soundNode->getName().c_str());
		mChannelArray[channelIndexTemp].mAttachedToNode = true;
    mChannelArray[channelIndexTemp].mPrevPos = soundNode->_getDerivedPosition();

    initialPosition.x = soundNode->_getDerivedPosition().x;
    initialPosition.y = soundNode->_getDerivedPosition().y;
    initialPosition.z = soundNode->_getDerivedPosition().z;
    FMOD_Channel_Set3DAttributes(channel, &initialPosition, NULL);
  } else {
  	mChannelArray[channelIndexTemp].mAttachedToNode = false;
  }

  result = FMOD_Channel_SetVolume(channel, 1.0); // where volume is 0.0 to 1.0
  result = FMOD_Channel_SetPaused(channel, false); // This is where the sound really starts.
  if (channelIndex) {
    *channelIndex = channelIndexTemp;
  }
  return true;
}

bool SoundManager::set3DMinMaxDistance(int channelIndex, float minDistance, float maxDistance) {
	if (!mEnabled) {
	return true;
	}
	FMOD_RESULT result;
	FMOD_CHANNEL* channel;
	if (channelIndex == INVALID_SOUND_CHANNEL) {
	  return false;
	}
	result = FMOD_System_GetChannel(mpSystem, channelIndex, &channel);
	if (result == FMOD_OK) {
	  FMOD_Channel_Set3DMinMaxDistance(channel, minDistance, maxDistance);
	}
	return true;
}

bool SoundManager::stopSound(int *channelIndex) {
   if (!mEnabled) {
		return true;
	}
  if (*channelIndex == INVALID_SOUND_CHANNEL) {
  	return false;
	}
  FMOD_CHANNEL *soundChannel;
  FMOD_System_GetChannel(mpSystem, *channelIndex, &soundChannel);
  FMOD_Channel_Stop(soundChannel);
  mChannelArray[*channelIndex].clear();
  *channelIndex = INVALID_SOUND_CHANNEL;
	return true;
}

int SoundManager::findSound(const char* fileName, SOUND_TYPE soundType) {
	if (!mEnabled) {
		return -1;
	}
  for (int i = 0; i < mSoundInstanceArrayEnd; i++) {
    if ((soundType == mSoundInstanceArray[i].mSoundType) && (strcmp(fileName, mSoundInstanceArray[i].mFileName) == 0)) {
    	return i;
		}
	}
  return -1;
}

float SoundManager::getSoundLength(int soundIndex) {
  if (!mEnabled) {
		return 0.0;
	}
  unsigned int soundLength;   // length in milliseconds
  FMOD_RESULT result;

	result = FMOD_Sound_GetLength(mSoundInstanceArray[soundIndex].mpFmodSound, &soundLength, FMOD_TIMEUNIT_MS);
  if (result != FMOD_OK) {
  	printf("ERROR in sndmgr\n");
  	printf(WHERESTR "\n", WHEREARG);
		return 0.0;
  }
  return (float)soundLength / 1000.0f;
}

