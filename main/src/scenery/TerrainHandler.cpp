#include "TerrainHandler.h"
#include "../globals.h"
#include <math.h>
#include "../usefulFuncs.h"
#ifndef NOPAGED
 #include "PagedGeoWrapper.h"
#endif
using namespace std;

#define DISABLE_NEAR_THRESH // stop the mal-functioning nearest neighbour angle threshold

Fog::Fog() {
	mMode = FOG_EXP; // 0 = none, 1 = linear
	mColour.r = 0.7;
	mColour.g = 0.7;
	mColour.b = 0.8; // default to white
	mDensity = 0.001;
	mLinearStart_m = 5.0;
	mLinearEnd_m = 2000.0;
}

// constructor to cover own arse
TerrainHandler::TerrainHandler() {
	mSceneMgr = NULL;
	mSelectedNode = NULL;
	mSelectedShrub = -1;
	mSelectedBuilding = -1;
	mTerrainManager = NULL;
	mSplattingManager = NULL;
	mPointer = Vector3(0.0f,0.0f,0.0f);
	mCurrentPaintTexture = 0;
	mCurrentShrubType = 0;
	mCurrentBuildingType = 0;
	mShrubDefHashEnd = 0;
	mShrubHashEnd = 0;
	mShrubEntityCount = 0;
	mShrubCullDistance = 250000.0f; // meters from camera to cull shrubs from view
	mShrubUnCullDistance = 202500.0f; // meters from camera to return shrubs to view*/
	mMaxShrubsOnScreen = 1024;
	for (int i = 0; i < SHRUB_HASH_MAX; i++) { // reset shrub hash
		mShrubHash[i] = NULL;
	}
	mBuildingDefHashEnd = 0;
	mBuildingHashEnd = 0;
	mBuildingEntityCount = 0;
	mBuildingCullDistance = 250000.0f; // meters from camera to cull Buildings from view
	mBuildingUnCullDistance = 202500.0f; // meters from camera to return Buildings to view
	mMaxBuildingsOnScreen = 1024;
	for (int i = 0; i < BUILDING_HASH_MAX; i++) { // reset Building hash
		mBuildingHash[i] = NULL;
	}
	mMinTerrainExtents = Vector3(0.0f, -300.0f, 0.0f); // size of terrain in meters
	mMaxTerrainExtents = Vector3(1500.0f, 300.0f, 1500.0f); // size of terrain in meters
	strcpy(mLastFileNameCreated, "");
	mPagedGeometryActivated = false;
}

// init fog and lighting etc
bool TerrainHandler::initScene(SceneManager* sceneMgr, RenderWindow* window, Camera* cam, bool paged) {
	if (!sceneMgr) {
		printf("ERROR: initScene, SceneManager = NULL\n");
		return false;
	}
	mSceneMgr = sceneMgr;
	pcam = cam;
	sceneMgr->setAmbientLight(ColourValue(0.5, 0.5, 0.5)); // TODO: Load these details from scenario??
	ColourValue fadeColour(0.9, 0.9, 0.9); // TODO: Load these details from scenario??
  sceneMgr->setFog(mFog.mMode, mFog.mColour, mFog.mDensity, mFog.mLinearStart_m, mFog.mLinearEnd_m); // TODO: Load these details from scenario
	window->getViewport(0)->setBackgroundColour(fadeColour); // TODO: Load these details from scenario??
	/*
	if (!mWaterManager.createWaterPlane(mSceneMgr, -10.0)) {
		printf("ERROR: could not create water plane in TerrainHandler::initScene()\n");
		return false;
	}*/
	
	mPagedGeometryActivated = paged;
#ifndef NOPAGED
	if (mPagedGeometryActivated) {
		if (!mPagedGeo.init(cam)) { // start paging system for geometry
			printf("ERROR: could not init paged geometry in TerrainHandler::initScene\n");
			return false;
		}
	}
#endif
	return true;
}

// init terrain resources
bool TerrainHandler::initET(unsigned int viewportHeight) {
	if (!mSceneMgr) {
		printf("ERROR: initET, mSceneMgr = NULL\n");
		return false;
	}
	mTerrainManager = new ET::TerrainManager(mSceneMgr); // initialise terrain manager
	mTerrainManager->setLODErrorMargin(2, viewportHeight); // set the Level of Detail Error (performance/quality balance) (unsigned int maxPixelError, unsigned int viewportHeight)
	mTerrainManager->setUseLODMorphing(true, 0.2f, "morphFactor"); // (bool lodMorph=true, float startMorphing=0.25f, const std::string &morphParamName="morphFactor")
	// IMPORTANT NOTE* if I want to make terrain extents user-chosen then I will need to auto-adjust vertex size of terrain to scale, otherwise shell holes and editor brushes will be the wrong size
	ET::TerrainInfo terrainInfo(257, 257, std::vector<float>(257*257, 0.5f)); // number of vertices in terrain
	terrainInfo.setExtents(AxisAlignedBox(mMinTerrainExtents.x, mMinTerrainExtents.y, mMinTerrainExtents.z, mMaxTerrainExtents.x, mMaxTerrainExtents.y, mMaxTerrainExtents.z)); // set position and size of the terrain
	#ifdef _GO_GRIDS_
	// set up terrain grid divider for AI
	g.mBrainGrid.init(mMinTerrainExtents.x, mMaxTerrainExtents.x, mMinTerrainExtents.z, mMaxTerrainExtents.z, 15.0f); // 15m*15m cells
	g.mBrainGrid.clearStaticObstacles();
	g.mSmallGrid.init(mMinTerrainExtents.x, mMaxTerrainExtents.x, mMinTerrainExtents.z, mMaxTerrainExtents.z, 3.0f); // 3m*3m cells
	g.mSmallGrid.clearStaticObstacles();
	printf("static obstacles cleared\n");
	#endif
	mTerrainManager->createTerrain(terrainInfo);
	mSplattingManager = new ET::SplattingManager("ETSplatting", "General", 256, 256, 3); // create the splatting manager (const std::string &baseName, const std::string &group, unsigned int width, unsigned int height, unsigned int channels=4)
	mSplattingManager->setNumTextures(7); // specify number of splatting textures we need to handle
	TexturePtr lightmapTex = TextureManager::getSingleton().createManual("ETLightmap", "General", TEX_TYPE_2D, 128, 128,1, PF_BYTE_RGB); // create a manual lightmap texture TODO: fix the warning that this causes in run-time
	Image lightmap;
	ET::createTerrainLightmap(terrainInfo, lightmap, 128, 128, Vector3(1, -0.75, 1), ColourValue(1.0, 1.0, 1.0), ColourValue(0.5, 0.5, 0.5)); // (terraininfo, imagefile, imagew, imageh, lightDir, lightCol, ambient, selfShadowed=true)
	lightmapTex->getBuffer(0, 0)->blitFromMemory(lightmap.getPixelBox(0, 0));
	MaterialPtr material(MaterialManager::getSingleton().getByName("ETTerrainMaterial")); // load the terrain material and assign it
	mTerrainManager->setMaterial(material);
	return true;
}

// init shrubs from files. returns false on error
bool TerrainHandler::initEditorScenery() {
	/*if (!mShrubDefHash[mShrubDefHashEnd].loadFromFile("firtree.txt")) { // load all the shrub defs into memory
		printf("ERROR: Could not load .def file\n");
		return false;
	}
	mShrubDefHashEnd++; // increment shrubdefinition count
	if (!mBuildingDefHash[mBuildingDefHashEnd].loadFromFile("countryhouse.txt")) { // load all the building defs into memory
		printf("ERROR: Could not load .def file\n");
		return false;
	}
	mBuildingDefHashEnd++;*/
	return true;
}

// create a brush from a picture file
bool TerrainHandler::createEditBrush(int size) {
	Image image;
	image.load("brush.png", "General"); // load the edit brush for terrain editing
	image.resize(size, size);
	mEditBrush = ET::loadBrushFromImage(image);
	return true;
}

bool TerrainHandler::setBrushSizeSmall() {
	if (!createEditBrush(4)) {
		printf("ERROR: creating small brush\n");
		return false;
	}
	return true;
}

bool TerrainHandler::setBrushSizeMed() {
	if (!createEditBrush(16)) {
		printf("ERROR: creating medium brush\n");
		return false;
	}
	return true;
}

bool TerrainHandler::setBrushSizeLarge() {
	if (!createEditBrush(64)) {
		printf("ERROR: creating large brush\n");
		return false;
	}
	return true;
}

// free terrain resources
bool TerrainHandler::free() {
#ifndef NOPAGED
	if (mPagedGeometryActivated) {
		if (!mPagedGeo.free()) {
			printf("ERROR: Could not free paged geometry in TerrainHandler::free\n");
			return false;
		}
	}
#endif	
	if (!freeAllShrubs()) { // delete shrubs
		printf("ERROR: Could not free all shrubs\n");
		return false;
	}
	if (!freeAllBuildings()) { // delete buildings
		printf("ERROR: Could not free all buildings\n");
		return false;
	}
	if(!mWaterManager.free()) {
		printf("ERROR: could not free water in TerrainHandler::free\n");
		return false;
	}
	MaterialManager::getSingleton().unload("ETTerrainMaterial"); //remove textures and materials
	TextureManager::getSingleton().remove("ETLightmap"); //remove textures and materials
	if (!mSplattingManager) { //remove textures and materials
		printf("ERROR: Could not free Splatting Manager\n");
		return false;	// nothing to free
	}
	delete mSplattingManager;
	mSplattingManager = NULL;
	if (!mTerrainManager) {
		printf("ERROR: Could not free Terrain Manager\n");
		return false;	// nothing to free
	}
	delete mTerrainManager;
	mTerrainManager = NULL;
	if (!mSceneMgr) { // get rid of all the meshes and nodes and entities
		printf("ERROR: Could not access Scene Manager\n");
		return false;	// nothing to free
	}
	mSceneMgr->clearScene();
	return true;
}

// clean the shrub hash and cancel all definitons
bool TerrainHandler::freeAllShrubs() {
	for (int i = 0; i < mShrubHashEnd; i++) { // delete data structures
		if (!mShrubHash[i]) {
			continue;
		}
		delete mShrubHash[i];
		mShrubHash[i] = NULL;
	}
	mShrubHashEnd = 0;
	return true;
}

bool TerrainHandler::freeAllBuildings() {
	for (int i = 0; i < mBuildingHashEnd; i++) { // delete data structures
		if (!mBuildingHash[i]) {
			continue;
		}
		delete mBuildingHash[i];
		mBuildingHash[i] = NULL;
	}
	mBuildingHashEnd = 0;
	return true;
}

// per frame updates (only used by scenario editor, not by sim)
bool TerrainHandler::update(const unsigned long& elapsed) {	
	if (mSelectedNode != NULL) { // look for any selected scenery
		bool found = false;
		for (int i = 0; i < mBuildingHashEnd; i++) { // look for buildings
			if (!mBuildingHash[i]) {
				continue;
			}
			if ((mBuildingHash[i]->getNode() == mSelectedNode)) {
				found = true;
				bool selected = mBuildingHash[i]->isSelected(); // it's a match - so invert the selection
				deselectAll();
				if (selected == false) {
					mBuildingHash[i]->setSelected(true);
					mSelectedBuilding = i;
				}
				break;
			}
		}
		if (!mPagedGeometryActivated) {
			if (!found) { // look through shrubs
				for (int i = 0; i < mShrubHashEnd; i++) {
					if (!mShrubHash[i]) {
						continue;
					}
					Ogre::SceneNode* ssn = NULL;
					ssn = mSceneMgr->getSceneNode(mShrubHash[i]->mNodeName);
					if (!ssn) {
						printf("ERROR: terrainhandler could not select shrub as name does not correspond to a scenenode\n");
						return false;
					}
					if ((ssn == mSelectedNode)) {
						found = true;
						bool selected = mShrubHash[i]->isSelected(); // it's a match - so invert the selection
						deselectAll();
						if (selected == false) {
							mShrubHash[i]->setSelected(true);
							if (!mPagedGeometryActivated) {
								ssn->showBoundingBox(true);
							}
							mSelectedShrub = i;
						}
						break;
					}
				}
			}
		}
	}
	if (!updateEnvironment(elapsed)) {
		printf("ERROR could not updateEnvironment from TerrainHandler::update()\n");
		return false;
	}

	return true;
}

bool TerrainHandler::updateEnvironment(const unsigned long& elapsed) {
#ifndef NOPAGED
	if (mPagedGeometryActivated) {
		if (!mPagedGeo.update()) {
			printf("ERROR could not update mPagedGeo from TerrainHandler::updateEnvironment()\n");
			return false;
		}
	}
#endif
	if (!mWaterManager.updateWater(elapsed)) {
		printf("ERROR could not update water plane from TerrainHandler::updateEnvironment()\n");
		return false;
	}
	updateShrubCulling(pcam->getPosition()); // update culling things from view
	updateBuildingCulling(pcam->getPosition()); // update culling things from view
	return true;
}

bool TerrainHandler::deselectAll() {
	mSelectedNode = NULL; // deselect previous selections
	if (mSelectedShrub >= 0) {
		mShrubHash[mSelectedShrub]->setSelected(false);
		if (!mShrubHash[mSelectedShrub]->mIsPagedGeometry) {
			Ogre::SceneNode* ssn = NULL;
			ssn = mSceneMgr->getSceneNode(mShrubHash[mSelectedShrub]->mNodeName);
			if (!ssn) {
				printf("ERROR: terrainhandler could not deselectAll shrub as name does not correspond to a scenenode\n");
				return false;
			}
			ssn->showBoundingBox(false);
		}
	}
	if (mSelectedBuilding >= 0) {
		mBuildingHash[mSelectedBuilding]->setSelected(false);
	}
	mSelectedShrub = -1;
	mSelectedBuilding = -1;
	return true;
}

// load scenario to files prefixed by 'name'
bool TerrainHandler::loadScenario(const char *name) {
	printf("Loading scenario %s\n", name);
	if (!freeAllShrubs()) { // delete shrubs
		printf("ERROR: Could not free all shrubs\n");
		return false;
	}
	if (!freeAllBuildings()) { // delete buildings
		printf("ERROR: Could not free all buildings\n");
		return false;
	}
	mSceneMgr->clearScene(); // free everything 'cos we dont want more than one copy at once
	if (!mScenarioDetails.loadFromFile(name)) { // load .sce file
		printf("ERROR: scenarioHandler could not load %s into scenarioDetails handler\n", name);
		return false;
	}
	mTerrainManager->destroyTerrain(); // recreate terrain...
	Image image;
	char filename[256];
	sprintf(filename,"%s_terrain.png", mScenarioDetails.mMapName);
	image.load(filename, "General");
	ET::TerrainInfo info(257, 257, std::vector<float>(257*257, 0.75f));
	ET::loadHeightmapFromImage(info, image);
	info.setExtents(AxisAlignedBox(mMinTerrainExtents.x, mMinTerrainExtents.y, mMinTerrainExtents.z, mMaxTerrainExtents.x, mMaxTerrainExtents.y, mMaxTerrainExtents.z)); // TODO load from .sce file
	mTerrainManager->createTerrain(info);
	for (unsigned int i = 0; i < mSplattingManager->getNumMaps(); i++) { // now load the splatting maps
		sprintf(filename,"%s_coverage_%i.png", mScenarioDetails.mMapName, i);
		image.load(filename, "General");
		mSplattingManager->loadMapFromImage(i, image);
	}
	if (!updateLightmap()) { // update the lightmap
		printf("ERROR: Could not generate Lightmap\n");
		return false;
	}
	
	if (mScenarioDetails.mWaterType == 1) {
		if (!mWaterManager.createWaterPlane(mScenarioDetails.mWaterLevel)) {
			printf("ERROR: could not create water plane in TerrainHandler::initScene()\n");
			return false;
		}
	}

	printf(" loading shrubs.....\n");
	sprintf(filename,"data/scenarios/maps/%s", mScenarioDetails.mShrubListName);
	if (!loadShrubs(filename)) { // Load shrubs and their DEFs
		printf("WARNING: Could not load shrubs using %s\n", filename);
		printf("Assuming no shrubs for this scenario.\n");
		return true;
	}
	printf(" shrubs loaded.\n");
	printf(" loading buildings.....\n");
	sprintf(filename,"data/scenarios/maps/%s", mScenarioDetails.mBuildingListName);
	if (!loadBuildings(filename)) { // Load buildings and their DEFs
		printf("ERROR: Could not load buildings using %s\n", filename);
		return false;
	}
	printf(" buildings loaded.\n");
	#ifdef _GO_GRIDS_
	g.mBrainGrid.clearStaticObstacles();
	g.mSmallGrid.clearStaticObstacles();
	#endif
	printf(" static obstacles cleared\n");
	if (!addAllToObstacleMap()) { // add shrubs and buildings to obstacle map
		printf("ERROR: could not add shrubs and buildings to obstacle map. TerrainHandler::loadScenario()\n");
		return false;
	}
	printf(" static obstacles added\n");
	printf("setting sky...\n");
	mSceneMgr->setSkyBox(true, "Examples/CloudyNoonSkyBox", 50 ); // Create a skybox
	printf(" setting lighting...\n");
	Light* l = mSceneMgr->createLight("MainLight"); // Create a light
	l->setType(Light::LT_DIRECTIONAL);
	if (g.mEnableSpecularLighting) {
		l->setSpecularColour(1.0f, 1.0f, 1.0f);
	}
	Vector3 direction(1, -3, 1); // directional light
	direction.normalise();
	l->setDirection(direction);

	#ifdef _GO_GRIDS_
	if (!storeTerrainHeightsInGrid()) {
		printf("ERROR: could not store terrain heights in grid in TH::loadSce()\n");
		return false;
	}
	#endif
	printf(" setting lightmap...\n");
	char lightmapfile[256];
	sprintf(lightmapfile, "%s_lightmap.png", mScenarioDetails.mMapName);
#ifndef NOPAGED
	if (mPagedGeometryActivated) {
		mPagedGeo.mTreeLoader->setColorMap(lightmapfile);
	}
#endif
printf("Scenario Loaded.\n");
	return true;
}

// save scenario to files prefixed by 'name'
bool TerrainHandler::saveScenario(const char* name) {
	char filename[256]; // names of actual files to save
	char temp[256];
	legaliseFilename(name, temp); // convert title into something that can be used as a filename
	strcpy(mLastFileNameCreated, temp);
	strcpy(mScenarioDetails.mMapName, temp); // store the maps' name prefix (just an OS-friendly version of title)
	sprintf(filename, "data/scenarios/%s", temp); // make a file name
	strcpy(mScenarioDetails.mFilename, filename); // set scenario file name (just an OS-friendly version of title)
	strcat(mScenarioDetails.mFilename, ".sce"); // add extension to filename
	Image image;
	ET::saveHeightmapToImage(mTerrainManager->getTerrainInfo(), image); // first save the terrain heightmap
	sprintf(filename, "data/scenarios/maps/%s_terrain.png", temp);
	image.save(filename);
	for (unsigned int i = 0; i < mSplattingManager->getNumMaps(); i++) { // now save the splatting maps
		mSplattingManager->saveMapToImage(i, image);
		sprintf(filename, "data/scenarios/maps/%s_coverage_%i.png", temp, i);
		image.save(filename);
	}
	Image lightmap; // now the lightmap
	ET::createTerrainLightmap(mTerrainManager->getTerrainInfo(), lightmap, 128, 128, Vector3(1, -0.75, 1), ColourValue(1.0, 1.0, 1.0), ColourValue(0.5, 0.5, 0.5));
	sprintf(filename, "data/scenarios/maps/%s_lightmap.png", temp);
	lightmap.save(filename);
	sprintf(filename, "data/scenarios/maps/%s_shrubs.txt", temp);
	if (!saveShrubs(filename)) { // save trees positions and DEFs name
		printf("ERROR: Could not save shrubs\n");
		return false;
	}
	sprintf(filename, "%s_shrubs.txt", temp);
	strcpy(mScenarioDetails.mShrubListName, filename); // set the name of the file that stores all the shrub locations
	sprintf(filename, "%s_units.txt", temp);
	strcpy(mScenarioDetails.mUnitListName, filename); // set the name of the file that stores all the shrub locations
	sprintf(filename, "data/scenarios/maps/%s_buildings.txt", temp);
	if (!saveBuildings(filename)) { // save house positions
		printf("ERROR: Could not save buildings\n");
		return false;
	}
	sprintf(filename, "%s_buildings.txt", temp);
	strcpy(mScenarioDetails.mBuildingListName, filename); // set the name of the file that stores all the building locations
	// TODO: save victory locations // TODO: save force pools
	if (!mScenarioDetails.saveToFile()) {
		printf("ERROR: scenarioHandler could not load %s into scenarioDetails handler\n", name);
		return false;
	}
	
	/* TODOS
	* force shield images
	* force names
	* time limits etc
	* water type
	* another other sce details
	* shadow override
	
	*/
	return true;
}

// save all the shrubs' positions and DEF to a file
bool TerrainHandler::saveShrubs(const char *name) {
	FILE* fDataFile;
	fDataFile = fopen(name, "w"); // Open scenario file and setup data members
	if (!fDataFile) {
		printf("ERROR: Could not open scenario shrub file %s for writing\n",name);
		return false;
	}
	for (int i = 0; i < mShrubHashEnd; i++) { // write all trees to file
		if (!mShrubHash[i]) {
			continue;
		}
		ShrubDef *defObj = mShrubHash[i]->getDefPtr();
		Vector3 pos = mShrubHash[i]->getPosition();
		double yaw = mShrubHash[i]->getYaw();
		fprintf(fDataFile,"SHRUB %s (%.2f,%.2f,%.2f) (%.2f)\n",defObj->getFilename(),pos.x,pos.y,pos.z,yaw);
	}
	fclose(fDataFile);
	return true;
}

// load all the shrubs' positions and DEF from a file
bool TerrainHandler::loadShrubs(const char *name) {
	FILE* fDataFile; // Open scenario file and setup data members
	fDataFile = fopen(name, "r");
	if (!fDataFile) {
		printf("ERROR: Could not open scenario shrub file %s for reading\n",name);
		return false;
	}
	if (mShrubDefHashEnd >= SHRUB_DEF_MAX) { // TEMP HACK: // load all the shrub defs into memory
		printf("ERROR: Too many shrub definitions in memory\n");
		return false;
	}/*
	if (!mShrubDefHash[mShrubDefHashEnd].loadFromFile("firtree.txt")) {
		printf("ERROR: Could not load .def file\n");
		return false;
	}
	mShrubDefHashEnd++; // increment shrubdefinition count
	printf("mShrubDefHashEnd = %i\n", mShrubDefHashEnd);*/
	char line[256];
	while (!feof(fDataFile)) { // read all trees from file
		fgets(line, 256, fDataFile);
		if (strncmp(line, "SHRUB", 5) == 0) { // scan shrub
			double x, y, z, yaw;
			char defName[256];
			sscanf(line,"SHRUB %s (%lf,%lf,%lf) (%lf)", defName, &x, &y, &z, &yaw);
			Vector3 pos; // create shrub
			pos.x = x;
			pos.y = y;
			pos.z = z;
			if (createShrub(defName, pos, yaw) < 0) {
				printf("ERROR: failed to Create shrub from scenario file.\n");
				return false;
			}

		} else if (line[0] == '#') { // skip commented lines
		} else if (strcmp(line, "") == 0) { // skip blank lines
		} else { // print warning on unreadable lines
			printf("WARNING: Unrecognised line in shrub file %s: [%s]\n", name, line);
		}
	}
	fclose(fDataFile);
	return true;
}

// save all the buildings' positions and DEF to a file
bool TerrainHandler::saveBuildings(const char *name) {
	FILE* fDataFile;
	fDataFile = fopen(name, "w"); // Open scenario file and setup data members
	if (!fDataFile) {
		printf("ERROR: Could not open scenario building file %s for writing\n",name);
		return false;
	}
	for (int i = 0; i < mBuildingHashEnd; i++) { // write all houses to file
		if (!mBuildingHash[i]) {
			continue;
		}
		BuildingDef *defObj = mBuildingHash[i]->getDefPtr();
		Vector3 pos = mBuildingHash[i]->getPosition();
		double yaw = mBuildingHash[i]->getYaw();
		fprintf(fDataFile,"BUILDING %s (%.2f,%.2f,%.2f) (%.2f)\n",defObj->getFilename(),pos.x,pos.y,pos.z,yaw);
	}
	fclose(fDataFile);
	return true;
}

// load bldngs into scen from file
bool TerrainHandler::loadBuildings(const char *name) {
	FILE* fDataFile;
	fDataFile = fopen(name, "r"); // Open scenario file and setup data members
	if (!fDataFile) {
		printf("WARNING: Could not open scenario building file %s for reading\n",name);
		printf("Assuming no buildings for this scenario\n");
		return true;
	}
	if (mBuildingDefHashEnd >= BUILDING_DEF_MAX) { // tEMP HACK:load all the building defs into memory
		printf("ERROR: Too many shrub definitions in memory\n");
		return false;
	}/*
	if (!mBuildingDefHash[mBuildingDefHashEnd].loadFromFile("countryhouse.txt")) {
		printf("ERROR: Could not load .def file\n");
		return false;
	}
	mBuildingDefHashEnd++;*/
	char line[256];
	while (!feof(fDataFile)) { // read all buildings from file
		fgets(line,256,fDataFile);
		if (strncmp(line, "BUILDING", 5) == 0) {
			double x,y,z,yaw;
			char defName[256];
			sscanf(line,"BUILDING %s (%lf,%lf,%lf) (%lf)",defName,&x,&y,&z,&yaw); // scan building
			Vector3 pos;
			pos.x = x;
			pos.y = y;
			pos.z = z;
			if (createBuilding(defName, pos, yaw) < 0) { // create building
				printf("ERROR: failed to Create building from scenario file.\n");
				return false;
			}
		} else if (line[0] == '#') { // skip commented lines
		} else if (strcmp(line, "") == 0) { // skip blank lines
		} else {
			printf("WARNING: Unrecognised line in building file %s: [%s]\n",name,line); // print warning on unreadable lines
		}
	}
	fclose(fDataFile);
	return true;
}

void TerrainHandler::setSelectedNode(SceneNode* node) {
	mSelectedNode = node;
}

bool TerrainHandler::moveSelectionWithMouse() {
	if (mSelectedShrub >= 0) {
		return moveShrubWithMouse(mSelectedShrub);
	}
	if (mSelectedBuilding >= 0) {
		return moveBuildingWithMouse(mSelectedBuilding);
	}
	return false;
}

// deletes selected scenery. returns false if no scenery selected. exits program on error
bool TerrainHandler::deleteSelection() {
	if (mSelectedShrub >= 0) {
		if (!mShrubHash[mSelectedShrub]) {
			printf("FATAL ERROR - can not delete shrub, it does not exist!\n");
			exit(1);
		}
		int selection = mSelectedShrub;
		deselectAll();
		shrubSetVisible(selection, false);	// hide, I don't bother deleting the node
		delete mShrubHash[selection];
		mShrubHash[selection] = NULL;
		if (selection == mShrubHashEnd - 1) {
			mShrubHashEnd--;
		}
		return true;
	}
	if (mSelectedBuilding >= 0) {
		if (!mBuildingHash[mSelectedBuilding]) {
			printf("FATAL ERROR - can not delete building %i, it does not exist!\n",mSelectedBuilding);
			exit(1);
		}
		int selection = mSelectedBuilding;
		deselectAll();
		mBuildingHash[selection]->setVisible(false);	// hide, I don't bother deleting the node
		delete mBuildingHash[selection];
		mBuildingHash[selection] = NULL;
		if (selection == mBuildingHashEnd - 1) {
			mBuildingHashEnd--;
		}
		return true;
	}
	return false;
}

bool TerrainHandler::shrubSetVisible(int index, bool visible) {
	if (!mShrubHash[index]->mIsPagedGeometry) {
		Ogre::SceneNode* ssn = NULL;
		ssn = mSceneMgr->getSceneNode(mShrubHash[index]->mNodeName);
		if (!ssn) {
			printf("ERROR: terrainhandler could not yawShrub as name does not correspond to a scenenode\n");
			return false;
		}
		ssn->setVisible(visible);
	}
	return true;
}

bool TerrainHandler::moveShrubTo(const int& index, const double& x, const double&y, const double& z) {
	mShrubHash[index]->setPosition(x, y, z);
	if (!mShrubHash[index]->mIsPagedGeometry) {
		Ogre::SceneNode* ssn = mSceneMgr->getSceneNode(mShrubHash[index]->mNodeName);
		ssn->setPosition(x, y, z);

	}
	return true;
}

bool TerrainHandler::yawShrub(int index, double yaw) {
	mShrubHash[index]->setYaw(yaw);
	if (!mShrubHash[index]->mIsPagedGeometry) {
		Ogre::SceneNode* ssn = NULL;
		ssn = mSceneMgr->getSceneNode(mShrubHash[index]->mNodeName);
		if (!ssn) {
			printf("ERROR: terrainhandler could not yawShrub as name does not correspond to a scenenode\n");
			return false;
		}
		ssn->resetOrientation();
		ssn->yaw(Radian(yaw));
	}
	return true;
}

bool TerrainHandler::posRotSelection(const long &elapsed) {
	if (mSelectedShrub >= 0) {
		if (!mShrubHash[mSelectedShrub]) {
			printf("ERROR - can not rotate shrub, it does not exist!\n");
			return false;
		}
		double yaw = mShrubHash[mSelectedShrub]->getYaw();
		yaw = elapsed * 0.000002f + yaw;
		yawShrub(mSelectedShrub, yaw);
		
		return true;
	}
	if (mSelectedBuilding >= 0) {
		if (!balanceBuilding(mSelectedBuilding)) {
			printf("ERROR - can not rotate building, it does not exist!\n");
			return false;
		}
		double yaw = mBuildingHash[mSelectedBuilding]->getYaw();
		yaw = yaw + elapsed * 0.000002f;
		mBuildingHash[mSelectedBuilding]->setYaw(yaw);
		if (!balanceBuilding(mSelectedBuilding)) {
			printf("ERROR: balancing a building that is being rotated\n");
		}

		return true;
	}
	return true;
}

bool TerrainHandler::negRotSelection(const long &elapsed) {
	if (mSelectedShrub >= 0) {
		if (!mShrubHash[mSelectedShrub]) {
			printf("ERROR - can not rotate shrub, it does not exist!\n");
			return false;
		}
		double yaw = mShrubHash[mSelectedShrub]->getYaw();
		yaw =  yaw - elapsed * 0.000002f;
		if(!yawShrub(mSelectedShrub, yaw)) {
			return false;
		}
		
		return true;
	}
	if (mSelectedBuilding >= 0) {
		if (!balanceBuilding(mSelectedBuilding)) {
			printf("ERROR - can not rotate building, it does not exist!\n");
			return false;
		}
		double yaw = mBuildingHash[mSelectedBuilding]->getYaw();
		yaw = yaw - elapsed * 0.000002f;
		mBuildingHash[mSelectedBuilding]->setYaw(yaw);
		return true;
	}
	return true;
}


// get nearest obstacle to a given position. false in no obstacles. also provides radius of object (~size of obstacle) TODO: {3D distances}
bool TerrainHandler::getNearestObstacleTo(const Vector3& origin, double currentHeading, double angleThresh, Vector3& obstacle, double& distanceSquared, double& sizeRadius, AxisAlignedBox& boundingBox) {
	Vector3 nearestObstaclePosition;
	nearestObstaclePosition.x = nearestObstaclePosition.y = nearestObstaclePosition.z = 0.0f; // reset
	double nearestObstacleDistance = 9876543210.987654210f;
	double nearestObstacleSizeRadius = 0.0f;
	int nearestObstacleIndex = -1;
	bool isBuilding = false;
	for (int i = 0; i < mShrubHashEnd; i++) { // first look through all the trees
		if (!mShrubHash[i]) { // skip empty entries
			continue;
		}
		Vector3 treePosition = mShrubHash[i]->getPosition(); // find out where this building is geographically
		double xDist = (double)fabs(treePosition.x - origin.x); // Calculate 2d Distance btwn points
		double zDist = (double)fabs(treePosition.z - origin.z);
		double squaredDistance = xDist * xDist + zDist * zDist; // squared distance in meters to centre of obstacle
		double radius = mShrubHash[i]->getRadiusMeters(); // radius size of obstacle (not squared)
		double sqDistInclSqRadius = squaredDistance - (double)(radius * radius); // include radius (size) in nearest obstacle calculation
		// find out if this is a new closest obstacle
		if (sqDistInclSqRadius < nearestObstacleDistance) { // compare sqDist - rad^2
			#ifndef DISABLE_NEAR_THRESH
			double chInHeading = getChangeInHeadingTo(origin, currentHeading, treePosition); // check if obst is in front of us
			if (chInHeading < angleThresh) { // only consider obst in front of us
			#endif
				nearestObstaclePosition = treePosition;
				nearestObstacleDistance = squaredDistance; // set sqDist and radius separately
				nearestObstacleSizeRadius = radius; // set sqDist and radius separately
				nearestObstacleIndex = i; // set this as nearest obst so far
			#ifndef DISABLE_NEAR_THRESH
			}
			#endif
		}
	}
	for (int i = 0; i < mBuildingHashEnd; i++) { // next look through all the buildings
		if (!mBuildingHash[i]) {
			continue;
		}
		// find out where this building is geographically
		Vector3 bldgPosition = mBuildingHash[i]->getPosition();
		// Calculate 2d Distance btwn points
		double xDist = (double)fabs(bldgPosition.x - origin.x);
		double zDist = (double)fabs(bldgPosition.z - origin.z);
		double squaredDistance = xDist * xDist + zDist * zDist; // squared distance in meters to centre of obstacle
		double radius = mBuildingHash[i]->getRadiusMeters(); // radius size of obstacle (not squared)
		double sqDistInclSqRadius = squaredDistance - (radius * radius); // include radius (size) in nearest obstacle calculation
		// find out if this is a new closest obstacle
		if (sqDistInclSqRadius < nearestObstacleDistance) { // compare sqDist - rad^2
			#ifndef DISABLE_NEAR_THRESH
			double chInHeading = getChangeInHeadingTo(origin, currentHeading, bldgPosition); // check if obst is in front of us
			if (chInHeading < angleThresh) { // only consider obst in front of us
			#endif
				nearestObstaclePosition = bldgPosition;
				nearestObstacleDistance = squaredDistance; // set sqDist and radius separately
				nearestObstacleSizeRadius = radius; // set sqDist and radius separately
				nearestObstacleIndex = i; // set this as nearest obst so far
				isBuilding = true;
			#ifndef DISABLE_NEAR_THRESH
			}
			#endif
		}
	}
	if (nearestObstacleIndex > -1) { // set required details
		obstacle = nearestObstaclePosition;
		distanceSquared = nearestObstacleDistance;
		sizeRadius = nearestObstacleSizeRadius;
		if (isBuilding) {
			boundingBox = mBuildingHash[nearestObstacleIndex]->getBoundingBox();
		} else {
			boundingBox = mShrubHash[nearestObstacleIndex]->getBoundingBox();
		}
		return true; // return success (that we have found a closest obstacle)
	}
	return false; // return failure (that no closest obstacle found)
}

bool TerrainHandler::storeTerrainHeightsInGrid() {
#ifdef _GO_GRIDS_
	for (int x = 0; x <= g.mBrainGrid.mCellsAcross; x++) {
		for (int y = 0; y <= g.mBrainGrid.mCellsDown; y++) {
			double realX = 0.0f;
			double realZ = 0.0f;
			if (!g.mBrainGrid.calcRealPos(x, y, realX, realZ)) {
				printf("ERROR: could not convert LARGEgrid cell to real 3d coords in TerrainHandler::storeTerrainHeightsInGrid()\n");
				return false;
			}
			double height = getTerrainHeightAt(realX, realZ);
			g.mBrainGrid.mGridCells[x][y].mTerrainHeight_m = height;
			if (mWaterPlane.mIsActive) {
				if (height < mWaterPlane.mBaseHeight) {
					g.mBrainGrid.mGridCells[x][y].mIsUnderWater = true;
				}
			}
		}
	}
	for (int x = 0; x <= g.mSmallGrid.mCellsAcross; x++) {
		for (int y = 0; y <= g.mSmallGrid.mCellsDown; y++) {
			double realX = 0.0f;
			double realZ = 0.0f;
			if (!g.mSmallGrid.calcRealPos(x, y, realX, realZ)) {
				printf("ERROR: could not convert SMALLgrid cell to real 3d coords in TerrainHandler::storeTerrainHeightsInGrid()\n");
				return false;
			}
			double height = getTerrainHeightAt(realX, realZ);
			g.mSmallGrid.mGridCells[x][y].mTerrainHeight_m = height;
			if (mWaterPlane.mIsActive) {
				if (height < mWaterPlane.mBaseHeight) {
					g.mBrainGrid.mGridCells[x][y].mIsUnderWater = true;
				}
			}
		}
	}
	#endif
	return true;
}

// get height of terrain in metres below given world coordinates
double TerrainHandler::getTerrainHeightAt(const double& xp, const double& zp) {
	if (!mTerrainManager) { // validate data
		return 0.0f;	// send nothing spurrious
	}
	return mTerrainManager->getTerrainInfo().getHeightAt(xp,zp);
}

// check if GUI mouse has intersected with 3D terrain
bool TerrainHandler::checkMouseIntersect(Ray &mouseRay) {
	std::pair<bool, Vector3> result = mTerrainManager->getTerrainInfo().rayIntersects(mouseRay); // since ETM is no longer a scene manager, we don't use a ray scene query, but instead query the terrain info directly
	if (result.first) {
		mPointer = result.second; // update pointer's position
		return true;
	}
	return false;
}

// get vector where mouse has intersected with 3D terrain
Vector3 TerrainHandler::getMousePosition() {
	return mPointer;
}

Vector3 TerrainHandler::getTerrainMinExtents() {
	return mMinTerrainExtents;
}

Vector3 TerrainHandler::getTerrainMaxExtents() {
	return mMaxTerrainExtents;
}

// manually set the 'mouse' position
void TerrainHandler::setPointerPosition(const Vector3 &pointer) {
	mPointer.x = pointer.x;
	mPointer.y = pointer.y;
	mPointer.z = pointer.z;
}

// build a mountain at the mouse position
bool TerrainHandler::deform(unsigned long elapsed, bool up) {
	double brushIntensity = elapsed * 0.0000001; // choose a brush intensity, this determines how extreme our brush works on the terrain
	if (!up) {
		brushIntensity *= -1;	// deform down
	}
	int x = mTerrainManager->getTerrainInfo().posToVertexX(mPointer.x); // translate our cursor position to vertex indexes
	int z = mTerrainManager->getTerrainInfo().posToVertexZ(mPointer.z); // translate our cursor position to vertex indexes
	mTerrainManager->deform(x, z, mEditBrush, brushIntensity); // now tell the ETM to deform the terrain
	reBalanceShrubsNear(mPointer, 300.0f); // rebalance nearby trees and houses // TODO: change the hardcoded 300m to something dynamic with brush size
	if (!reBalanceBuildingsNear(mPointer, 300.0f)) { // TODO: change the hardcoded 300m to something dynamic with brush size
		printf("ERROR: deform of terrain irritated buildings on terrain\n");
		return false;
	}
	return true;
}

// paint at mouse position
bool TerrainHandler::paint(unsigned long elapsed, bool positive) {
	if (mCurrentPaintTexture < 0) { // validate current paint
		printf("ERROR: paint texture was < 0 in TerrainHandler::paint()\n");
		return false;
	}
	double brushIntensity = elapsed * 0.0000025; // need to set our brush intensity larger for painting.
	if (!positive) { // for painting, all texture channels are only 1 byte large, so with a small intensity you won't get any effect at all.
		brushIntensity *= -1;	// deform down
	}
	int x = mTerrainManager->getTerrainInfo().posToVertexX(mPointer.x); // retrieve edit points
	int z = mTerrainManager->getTerrainInfo().posToVertexZ(mPointer.z); // retrieve edit points
	mSplattingManager->paint(mCurrentPaintTexture, x, z, mEditBrush, brushIntensity); // now use the splatting manager to update the coverage maps
	return true;
}

// recast light over terrain (avoid doing in real time as writes to image file)
bool TerrainHandler::updateLightmap() {
	Image lightmap;	// the lightmap to generate
	ET::createTerrainLightmap(mTerrainManager->getTerrainInfo(), lightmap, 128, 128, Vector3(1, -0.75, 1), ColourValue(1.0, 1.0, 1.0), ColourValue(0.5, 0.5, 0.5));
	TexturePtr tex = TextureManager::getSingleton().getByName("ETLightmap"); // get our dynamic texture and update its contents
	tex->getBuffer(0, 0)->blitFromMemory(lightmap.getPixelBox(0, 0));
	return true; // TODO: detect some errors
}

// set number of current paint texture
void TerrainHandler::setCurrentPaint(int num) {
	mCurrentPaintTexture = num;
}

// generate a new shrub from a particular definition (type) // returns index of shrub or < 0 on error
int TerrainHandler::createShrub(int defIndex, Vector3& pos, double yaw) {
	char shrubEntityName[32]; // create the entity (blender mesh)
	sprintf(shrubEntityName, "shrubEntity%i", mShrubEntityCount);
	Ogre::Entity* ent = Ogre::Root::getSingleton().getSceneManager("mkivSceneManager")->createEntity(shrubEntityName, mShrubDefHash[defIndex].getMeshFile());
	if (!ent) {
		printf("ERROR: could not create shrub entity from mesh file\n");
		return -2;
	}
	Ogre::Vector3 size = ent->getBoundingBox().getSize();
	double boundRad = size.x;
	if (size.y > boundRad) {
		boundRad = size.y;
	}
	if (size.z > boundRad) {
		boundRad = size.z;
	}
	
	bool belowWaterLevel = false;
	if (pos.y < mWaterManager.mMaxWaterLevel) {
		belowWaterLevel = true;
	}
	
	char nodeName[128];
	sprintf(nodeName, "%sNode", shrubEntityName);
	Ogre::SceneNode* tempNode = NULL;
	if (!mPagedGeometryActivated || belowWaterLevel) {
		tempNode = Ogre::Root::getSingleton().getSceneManager("mkivSceneManager")->getRootSceneNode()->createChildSceneNode(nodeName);
		if (!tempNode) {
			printf("ERROR: could not create shrub node\n");
			return -3;
		}
		tempNode->attachObject(ent);
	}
	
	mShrubEntityCount++;
	
	if (mShrubHashEnd >= SHRUB_HASH_MAX) { // create the tempNode as a vehicle
		printf("ERROR: Too many (%i) shrubs, could not create any more.\n", mShrubDefHashEnd);
		return -4;	// too many shrubs
	}
	int slotToUse = mShrubHashEnd;	// default slot is at the end
	for (int i = 0; i < mShrubHashEnd; i++) { // first look for empty slots
		if (mShrubHash[i] == NULL) {
			slotToUse = i; // found a free spot - make new shrub here
			break;
		}
	}
	mShrubHash[slotToUse] = new Shrub; // allocate memory
	if (!mShrubHash[slotToUse]) {
		printf("FATAL ERROR: not enough free memory to create shrub\n");
		exit(1);
	}
	if (!mShrubHash[slotToUse]->init(&mShrubDefHash[defIndex], ent, pos, boundRad)) { // initialise shrub
		printf("ERROR: failed to initialise shrub\n");
		return -5;
	}
	if (slotToUse == mShrubHashEnd) {
		mShrubHashEnd++;
	}
	if (mPagedGeometryActivated && !belowWaterLevel) {
#ifndef NOPAGED	
		mPagedGeo.mTreeLoader->addTree(ent, pos, Degree(Radian(yaw).valueDegrees()), mShrubHash[slotToUse]->mScaleFactor); // add tree to paged geometry
#endif
		mShrubHash[slotToUse]->mIsPagedGeometry = true;
	} else {
		strcpy(mShrubHash[slotToUse]->mNodeName, nodeName);
		tempNode->setScale(mShrubHash[slotToUse]->mScaleFactor, mShrubHash[slotToUse]->mScaleFactor, mShrubHash[slotToUse]->mScaleFactor);
		tempNode->setPosition(pos);
	}
	yawShrub(slotToUse, yaw); // set rotation
	if (!balanceShrub(slotToUse)) { // re-balance on terrain
		printf("ERROR: could not balance newly created shrub on terrain in TerrainHandler::createShrub()\n");
		return -6;
	}
	return slotToUse;
}

// create shrub file from def name at position and rotation
int TerrainHandler::createShrub(const char* defFile, Vector3& pos, double yaw) {
	int defIndex = -1;
	for (int i = 0; i < mShrubDefHashEnd; i++) { // first check to see if name is loaded into memory
		if (strcmp(mShrubDefHash[i].getFilename(), defFile) == 0) {
			defIndex = i; // found it so grab the index numnber
			break;
		}
	}
	if (defIndex == -1) { // if not check if it exists and load file into memory
		if (!mShrubDefHash[mShrubDefHashEnd].loadFromFile(defFile)) { // load ashrub def into memory
			printf("ERROR: Could not load .def file %s\n", defFile);
			return -1;
		}
		defIndex = mShrubDefHashEnd;
		mShrubDefHashEnd++;
	}
	return createShrub(defIndex, pos, yaw); // create shrub using this index number
}

// create the 'current' shrub selection at the mouse pointer position // return index of shrub or < 0 on error
int TerrainHandler::createShrubsAtPointer(const char* defFile, int clumpSize) {
	int result = -1;
	if (clumpSize == 0) { // create a single shrub
		int randYaw = rand() % 628;
		double dRandYaw = ((double)randYaw) / 100.0;
		result = createShrub(defFile, mPointer, dRandYaw);
		if (mPagedGeometryActivated) {
#ifndef NOPAGED
			if (!mPagedGeo.doneAddingTrees()) { // copy trees to pager
				printf("ERROR: could not finalise tree adding to pager in TerrainHandler::createShrubAtPointer\n");
				return -1;
			}
#endif
		}
	} else { // create several shrubs at once
		int treeCount = 4; // create a small clump of shrubs
		int dispersion = 15;
		if (clumpSize == 2) { // create a forest of shrubs
			treeCount = 50;
			dispersion = 100;
		}
		for (int i = 0; i < treeCount; i++) {
			int xoffset = rand() % dispersion - dispersion / 2;
			int zoffset = rand() % dispersion - dispersion / 2;
			int randYaw = rand() % 628;
			double dRandYaw = ((double)randYaw) / 100.0;
			Vector3 pos;
			pos.x = mPointer.x + (float)xoffset;
			pos.y = mPointer.y;
			pos.z = mPointer.z + (float)zoffset;
			result = createShrub(defFile, pos, dRandYaw);
			if (mPagedGeometryActivated) {
	#ifndef NOPAGED
			if (!mPagedGeo.doneAddingTrees()) { // copy trees to pager
				printf("ERROR: could not finalise tree adding to pager in TerrainHandler::createShrubAtPointer\n");
				return -1;
			}
		#endif
			}
		}
	}
	return result;
}

// create a building from a given def file index. also orient and position
int TerrainHandler::createBuilding(int defIndex, Vector3& pos, double yaw) {
	if (!mSceneMgr) {
		printf("ERROR: SceneManager = NULL\n");
		return -1;
	}
	char buildingEntityName[32];
	sprintf(buildingEntityName, "buildingEntity%i", mBuildingEntityCount);
	Entity* ent = mSceneMgr->createEntity(buildingEntityName, mBuildingDefHash[defIndex].getMeshFile()); // create the entity (blender mesh)
	if (!ent) {
		printf("ERROR: could not create building entity from mesh file\n");
		return -2;
	}
	Ogre::Vector3 size = ent->getBoundingBox().getSize();
	double boundRad = size.x;
	if (size.y > boundRad) {
		boundRad = size.y;
	}
	if (size.z > boundRad) {
		boundRad = size.z;
	}
	SceneNode* tempNode = NULL;
	tempNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(String(buildingEntityName) + "Node");
	if (!tempNode) {
		printf("ERROR: could not create building node\n");
		return -3;
	}
	tempNode->attachObject(ent);
	mBuildingEntityCount++;
	if (mBuildingHashEnd >= BUILDING_HASH_MAX) { // create the tempNode as a building
		printf("ERROR: Too many (%i) buildings, could not create any more.\n", mBuildingDefHashEnd);
		return -4;	// too many buildings
	}
	int slotToUse = mBuildingHashEnd;	// default slot is at the end
	for (int i = 0; i < mBuildingHashEnd; i++) { // first look for empty slots
		if (mBuildingHash[i] == NULL) {
			// found a free spot - make new building here
			slotToUse = i;
			break;
		}
	}
	mBuildingHash[slotToUse] = new Building; // allocate memory
	if (!mBuildingHash[slotToUse]) {
		printf("FATAL ERROR: not enough free memory to create building\n");
		exit(1);
	}
	if(!mBuildingHash[slotToUse]->init(&mBuildingDefHash[defIndex], ent, tempNode, pos, boundRad)) { // initialise building
		printf("ERROR: failed to initialise building\n");
		return -5;
	}
	mBuildingHash[slotToUse]->setYaw(yaw); // set rotation
	if (!balanceBuilding(slotToUse)) { // re-balance on terrain
		printf("ERROR: could not balance newly created building on terrain\n");
		return false;
	}
	if (slotToUse == mBuildingHashEnd) {
		mBuildingHashEnd++;
	}
	return slotToUse;
}

// create building with given def file name
int TerrainHandler::createBuilding(const char *defFile, Vector3& pos, double yaw) {
	int defIndex = -1;
	for (int i = 0; i < mBuildingDefHashEnd; i++) { // first check to see if name is loaded into memory
		if (strcmp(mBuildingDefHash[i].getFilename(), defFile) == 0) {
			defIndex = i;		// found it so grab the index numnber
			break;
		}
	}
	if (defIndex == -1) { // if not check if it exists and load file into memory
		if (!mBuildingDefHash[mBuildingDefHashEnd].loadFromFile(defFile)) { // load abuilding def into memory
			printf("ERROR: Could not load .def file %s\n",defFile);
			return false;
		}
		defIndex = mBuildingDefHashEnd;
		mBuildingDefHashEnd++;
	}
	return createBuilding(defIndex, pos, yaw); // create building using this index number
}

// create a building at pointer position
int TerrainHandler::createBuildingAtPointer(const char* defFile) {
	return createBuilding(defFile, mPointer, 0.0f);
}

// move the tree over terrain to follow the mouse pointer
bool TerrainHandler::moveShrubWithMouse(int shrubID) {
	if (!mShrubHash[shrubID]) {
		printf("ERROR: Shrub %i does not exist\n",shrubID);
		return false;
	}
	moveShrubTo(shrubID, mPointer.x , mPointer.y, mPointer.z);
	return true;
}

// move the tree over terrain to follow the mouse pointer
bool TerrainHandler::moveBuildingWithMouse(int buildingID) {
	if (!mBuildingHash[buildingID]) {
		printf("ERROR: Building %i does not exist\n",buildingID);
		return false;
	}
	mBuildingHash[buildingID]->setPosition(mPointer);
	if (!balanceBuilding(buildingID)) {
		printf("ERROR: balancing a building that is being moved with the mouse\n");
	}
	return true;
}

// cull shrubs from view if out of camera cull distance, or within uncull distance
void TerrainHandler::updateShrubCulling(Vector3 camPos) {
	for (int i = 0; i < mShrubHashEnd; i++) {
		if (!mShrubHash[i]) {
			continue;
		}
		if (mShrubHash[i]->mIsPagedGeometry) {
			continue; // skip paged geometry
		}
		Vector3 shrubPos = mShrubHash[i]->getPosition();
		float sqDistance = shrubPos.squaredDistance(camPos);
		if (sqDistance > mShrubCullDistance) { // if distance is greater than culling range
			shrubSetVisible(i, false);
		} else if (sqDistance < mShrubUnCullDistance) {
			shrubSetVisible(i, true); // else if distance is less than unculling rnage
		}
	}
}

// cull buildings from view if out of camera cull distance, or within uncull distance
void TerrainHandler::updateBuildingCulling(Vector3 camPos) {
	for (int i = 0; i < mBuildingHashEnd; i++) {
		if (!mBuildingHash[i]) {
			continue;
		}
		Vector3 buildingPos = mBuildingHash[i]->getPosition();
		float sqDistance = buildingPos.squaredDistance(camPos);
		if (sqDistance > mBuildingCullDistance) { // if distance is greater than culling range
			mBuildingHash[i]->setVisible(false);
		} else if (sqDistance < mBuildingUnCullDistance) {
			mBuildingHash[i]->setVisible(true); // else if distance is less than unculling rnage
		}
	}
}

// balance this shrub on terrain
bool TerrainHandler::balanceShrub(int shrubIndex) {
	if (!mShrubHash[shrubIndex]) { // validation
		printf("ERROR: can't balance shrub %i as it does not exist. in TerrainHandler::balanceShrub()\n", shrubIndex);
		return false;
	}
	Vector3 shrubPos = mShrubHash[shrubIndex]->getPosition(); // get existing position
	shrubPos.y = getTerrainHeightAt(shrubPos.x, shrubPos.z); // change height to height of terrain at centre of shrub
	moveShrubTo(shrubIndex, shrubPos.x, shrubPos.y, shrubPos.z); // place shrub
	return true;
}

// balance all shrubs within range of this position
void TerrainHandler::reBalanceShrubsNear(Vector3 pos, double range) {
	double squaredRange = range * range;
	for (int i = 0; i < mShrubHashEnd; i++) {
		if (!mShrubHash[i]) {
			continue;
		}
		Vector3 shrubPos = mShrubHash[i]->getPosition();
		double sqDistance = shrubPos.squaredDistance(pos);
		if (sqDistance < squaredRange) { // if distance is greater than culling range
			balanceShrub(i);
		}
	}
}

// balance this building on terrain
bool TerrainHandler::balanceBuilding(int buildingIndex) {
	if (!mBuildingHash[buildingIndex]) { // validation
		printf("ERROR: can't balance building %i as it does not exist\n", buildingIndex);
		return false;
	}
	Vector3 buildingPos = mBuildingHash[buildingIndex]->getPosition(); // get existing position
	buildingPos.y = getTerrainHeightAt(buildingPos.x, buildingPos.z); // change height to height of terrain at centre of building
	mBuildingHash[buildingIndex]->setPosition(buildingPos); // place building
	return true;
}

// balance all buildings within range of this position
bool TerrainHandler::reBalanceBuildingsNear(Vector3 pos, double range) {
	double squaredRange = range * range;
	for (int i = 0; i < mBuildingHashEnd; i++) {
		if (!mBuildingHash[i]) {
			continue;
		}
		Vector3 buildingPos = mBuildingHash[i]->getPosition();
		double sqDistance = buildingPos.squaredDistance(pos);
		if (sqDistance < squaredRange) { // if distance is greater than culling range
			if (!balanceBuilding(i)) {
				printf("ERROR: failed to balance building near pointer (bad record of buildings)\n");
				return false;
			}
		}
	}
	return true;
}

// new algorithm: get each corner; get grid cell at each corner, occupy all with a for loop
bool TerrainHandler::addAllToObstacleMap() {
#ifdef _GO_GRIDS_
	for (int i = 0; i < mBuildingHashEnd; i++) { // add all buildings Note: the following long process is because _getWorldAABB is highly unreliable
		Vector3 pos =  mBuildingHash[i]->getPosition();
		AxisAlignedBox boundingBox = mBuildingHash[i]->mMesh->getBoundingBox(); // get the bounding box of the building in scaled/world coords
		Vector3 min = boundingBox.getMinimum(); // get minimum corner of box
		Vector3 max = boundingBox.getMaximum(); // get minimum corner of box
		double scaleFactor = mBuildingHash[i]->mScaleFactor;
		min = min * scaleFactor;
		max = max * scaleFactor;
		min = min + pos;
		max = max + pos;
		if (!g.mBrainGrid.addBuilding(min.x, min.z, max.x, max.z, i)) {
			printf("ERROR: failed to add building to large grid cells in TerrainHandler::addAllToObstacleMap()\n");
			return false;
		}
		if (!g.mSmallGrid.addBuilding(min.x, min.z, max.x, max.z, i)) {
			printf("ERROR: failed to add building to small grid cells in TerrainHandler::addAllToObstacleMap()\n");
			return false;
		}
	}
	for (int i = 0; i < mShrubHashEnd; i++) { // Note: the following long process is because _getWorldAABB is highly unreliable
		Vector3 pos =  mShrubHash[i]->getPosition();
		AxisAlignedBox boundingBox = mShrubHash[i]->mMesh->getBoundingBox(); // get the bounding box of the building in scaled/world coords
		Vector3 min = boundingBox.getMinimum(); // get minimum corner of box
		Vector3 max = boundingBox.getMaximum(); // get minimum corner of box
		double scaleFactor = mShrubHash[i]->mScaleFactor;
		min = min * scaleFactor;
		max = max * scaleFactor;
		min = min + pos;
		max = max + pos;
		if (!g.mBrainGrid.addShrub(min.x, min.z, max.x, max.z, i)) {
			printf("ERROR: failed to add shrub to large grid cells in TerrainHandler::addAllToObstacleMap()\n");
			return false;
		}
		if (!g.mSmallGrid.addShrub(min.x, min.z, max.x, max.z, i)) {
			printf("ERROR: failed to add shrub to small grid cells in TerrainHandler::addAllToObstacleMap()\n");
			return false;
		}
	}
	#endif
	return true;
}

// get radians would need to turn to face
double TerrainHandler::getChangeInHeadingTo(const Vector3& origin, const double& currentHeading, const Vector3& target) {
	double opposite = target.x - origin.x;
	double adjacent = target.z - origin.z;
	double theta = atan2f(opposite, adjacent); // Calculate Angle to target
	double range = currentHeading - theta; // get change in heading
	if (range > M_PI) { // if > 180 degree turn then...
		range = range - 2.0f * M_PI; // ...adjust so left and right hand turns are equiv
	}
	return fabs(range); // return the absolute value of range (no negative values considered)
}

bool TerrainHandler::isTerrainBetween(const Vector3& start, const Vector3& end, Vector3& terrainPos) {
	Vector3 direction;
	direction.x = end.x - start.x;
	direction.y = end.y - start.y;
	direction.z = end.z - start.z;
	direction.normalise();
	Ray ourRay(start, direction);
	std::pair<bool, Vector3> result = mTerrainManager->getTerrainInfo().rayIntersects(ourRay); // query the terrain info directly
	if (result.first) {
		if (start.distance(end) > start.distance(result.second)) {
			terrainPos = result.second;
			return true;
		}
	}
	return false;
}

