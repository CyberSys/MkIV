#ifndef __SOUNDMANAGER_H__
#define __SOUNDMANAGER_H__

#include <fmod.h>
#include "../handyMacros.h"
#include <Ogre.h>
using namespace Ogre;

#define MAX_SOUND_CHANNELS 200
#define MAX_SOUND_INSTANCES 100

enum SOUND_TYPE {
	SOUND_TYPE_INVALID = 0,
	SOUND_TYPE_3D_SOUND,
	SOUND_TYPE_3D_SOUND_LOOPED,
	SOUND_TYPE_2D_SOUND,
	SOUND_TYPE_2D_SOUND_LOOPED,
};

struct SoundInstance {
	void clear();
	char mFileName[256];
	DataStreamPtr mStreamPtr; // what is this???
	SOUND_TYPE mSoundType;
	FMOD_SOUND* mpFmodSound;
};

struct ChannelInstance {
   void clear();
   char mSceneNodeHandle[128];
   Ogre::Vector3 mPrevPos;
   bool mAttachedToNode;
};

class SoundManager : public Ogre::Singleton<SoundManager> {
public:
	SoundManager();
	~SoundManager();
	static SoundManager& getSingleton();
	static SoundManager* getSingletonPtr();
	bool init();
	bool loadMenuSounds();
	void stopAllSounds();
	bool update(Ogre::Camera* camera, const unsigned long &timeElapsed);
	
	int createStream(const char* filename); // single-shot 2D stream. returns soundIndex
	int createSound(const char* filename); // single-shot 3D sound. returns soundIndex
	int createLoopedSound(const char* filename); // looping 3D sound. returns soundIndex
	int createLoopedStream(const char* filename); // looping 2D stream. returns soundIndex
	int createSound(const char* filename, SOUND_TYPE soundType);
	
	bool playSoundB(int soundIndex, Ogre::SceneNode *soundNode, int *channelIndex);
	bool stopSound(int *channelIndex);
	int findSound(const char* fileName, SOUND_TYPE soundType); // returns soundIndex;
	bool set3DMinMaxDistance(int channelIndex, float minDistance, float maxDistance);
	float getSoundLength(int soundIndex); // returns seconds
	
	int mButtonClickSoundIndex;
private:
	FMOD_SYSTEM* mpSystem;
	Ogre::Vector3 mPrevListenerPosition;
	ChannelInstance	mChannelArray[MAX_SOUND_CHANNELS];
	SoundInstance mSoundInstanceArray[MAX_SOUND_INSTANCES];
	int mSoundInstanceArrayEnd;
	bool mEnabled;
};
#endif // __SOUNDMANAGER_H__

