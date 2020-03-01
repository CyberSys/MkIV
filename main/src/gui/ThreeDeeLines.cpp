#include "ThreeDeeLines.h"

ThreeDeeLines::ThreeDeeLines() {
	pSceneManager = NULL;
	mNumberOfLines = 0;
}

bool ThreeDeeLines::init(SceneManager* sceneMgr, const char* uniqueName) {
	printf("Initialising 3D Lines...\n");
	if (!sceneMgr) { // validate
		printf("ERROR: SceneManager* = NULL in ThreeDeeLines::init()\n");
		return false; // failure
	}
	pSceneManager = sceneMgr;
	strcpy(mUniqueName, uniqueName);
	mNumberOfLines = 0; // ERROR: for some reason constructor didn't do this!!
	return true; // success
}

bool ThreeDeeLines::free() {
	printf("ThreeDeeLines::free()...");
	if (!pSceneManager) {
		printf("ERROR: no scene manager in ThreeDeeLines::free3DLines()\n");
		return false; // fail
	}
	for (int i = 0; i < mNumberOfLines; i++) {
		char tmp[256];
		sprintf(tmp, "%s3DLineMaterial%i", mUniqueName, i);
		MaterialManager::getSingleton().remove(tmp); // remove material
		if (!mp3DLineNode[i]) {
			printf("ERROR: line node %i not found to destroy\n", i);
		}
		mp3DLineNode[i] = NULL;
		pSceneManager->destroyManualObject(mp3DLineObject[i]); // remove object
		sprintf(tmp, "%s3DLineNode%i", mUniqueName, i);
		pSceneManager->destroySceneNode(tmp); // remove node
		sprintf(tmp, "%s3DLineMaterial%i", mUniqueName, mNumberOfLines);
		MaterialManager::getSingleton().remove(tmp);
	}
	mNumberOfLines = 0;
	printf("[OK]\n");
	return true; // succeed
}

// construct a new 3D line; returns index of new line (-1 if error)
int ThreeDeeLines::constructLine(float r, float g, float b, float a) {
	printf("DB: ThreeDeeLines::constructLine() - mNumberOfLines = %i\n", mNumberOfLines);
	if (mNumberOfLines >= MAX_3D_LINES - 1) {
		printf("FATAL ERROR: too many 3D lines (%i). Can not exec ThreeDeeLines::constructLine().\n", MAX_3D_LINES);
		exit(1);
	}
	char tmp[256];
	sprintf(tmp, "%s3DLineObject%i", mUniqueName, mNumberOfLines);
	mp3DLineObject[mNumberOfLines] = pSceneManager->createManualObject(tmp);
	sprintf(tmp, "%s3DLineNode%i", mUniqueName, mNumberOfLines);
	mp3DLineNode[mNumberOfLines] = pSceneManager->getRootSceneNode()->createChildSceneNode(tmp);
	sprintf(tmp, "%s3DLineMaterial%i", mUniqueName, mNumberOfLines);
	mp3DLineMaterial[mNumberOfLines] = MaterialManager::getSingleton().create(tmp, "Two");
	mp3DLineMaterial[mNumberOfLines]->setReceiveShadows(false);
	mp3DLineMaterial[mNumberOfLines]->getTechnique(0)->setLightingEnabled(true);
	mp3DLineMaterial[mNumberOfLines]->getTechnique(0)->getPass(0)->setDiffuse(r, g, b, a);
	mp3DLineMaterial[mNumberOfLines]->getTechnique(0)->getPass(0)->setAmbient(r, g , b);
	mp3DLineMaterial[mNumberOfLines]->getTechnique(0)->getPass(0)->setSelfIllumination(r, g, b);
	mp3DLineNode[mNumberOfLines]->attachObject(mp3DLineObject[mNumberOfLines]);
	mp3DLineObject[mNumberOfLines]->setRenderQueueGroup(RENDER_QUEUE_OVERLAY - 1); // render just before overlays
	int rValue = mNumberOfLines;
	mNumberOfLines++;
	return rValue;
}

bool ThreeDeeLines::start3DLineStrip(int lineIndex) {
	if ((lineIndex < 0) || (lineIndex >= MAX_3D_LINES)) {
		printf("ERROR: 3D line lineIndex %i is out of range in ThreeDeeLines::show3DLineStrip()\n", lineIndex);
		return false;
	}
	mp3DLineObject[lineIndex]->clear(); // reset
	char tmp[256];
	sprintf(tmp, "%s3DLineMaterial%i", mUniqueName, lineIndex);
	mp3DLineObject[lineIndex]->begin(tmp, Ogre::RenderOperation::OT_LINE_STRIP);
	return true;
}

bool ThreeDeeLines::addPointTo3DLineStrip(int lineIndex, Vector3 position) {
	mp3DLineObject[lineIndex]->position(position); // add position to line
	return true;
}

bool ThreeDeeLines::end3DLineStrip(int lineIndex) {
	if ((lineIndex < 0) || (lineIndex >= mNumberOfLines)) {
		printf("ERROR: 3D line lineIndex %i is out of range in ThreeDeeLines::show3DLineStrip()\n", lineIndex);
		return false;
	}
	mp3DLineObject[lineIndex]->end();
	return true;
}

// stop drawing a line
bool ThreeDeeLines::hide3DLine(int lineIndex) {
	if ((lineIndex < 0) || (lineIndex >= mNumberOfLines)) {
		printf("ERROR: 3D line lineIndex %i is out of range in ThreeDeeLines::hide3DLine()\n", lineIndex);
		return false;
	}
	mp3DLineObject[lineIndex]->clear(); // reset
	return true;
}

// draw a line from self to the current obstacle (if one exists)
bool ThreeDeeLines::show3DLineFromHereToThere(int lineIndex, const Vector3 &here, const Vector3 &there) {
	if ((lineIndex < 0) || (lineIndex >= mNumberOfLines)) {
		printf("ERROR: 3D line lineIndex %i is out of range in ThreeDeeLines::show3DLineFromHereToThere()\n", lineIndex);
		return false;
	}
	mp3DLineObject[lineIndex]->clear(); // reset
	char tmp[256];
	sprintf(tmp, "%s3DLineMaterial%i", mUniqueName, lineIndex);
	mp3DLineObject[lineIndex]->begin(tmp, Ogre::RenderOperation::OT_LINE_LIST);
	mp3DLineObject[lineIndex]->position(here); // position of start of line (should be our own node)
	mp3DLineObject[lineIndex]->position(there); // next vertex in line
	mp3DLineObject[lineIndex]->end();
	return true;
}


