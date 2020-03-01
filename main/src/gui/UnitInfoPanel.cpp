#include "UnitInfoPanel.h"
#define PANEL_MODE_COUNT 4

// set default variable values
UnitInfoPanel::UnitInfoPanel() {
	mpUnitPanel = NULL; // panel displaying selected unit details
	mPanelMode = 0; // 'mode' of panel (which info set is being displayed)
}

// allocate memory
bool UnitInfoPanel::init() {
	printf("Initialising UnitInfoPanel\n");
	mpUnitPanel = OverlayManager::getSingleton().getByName("mkiv/UnitPanel");
	if (!mpUnitPanel) {
		printf("ERROR: failed to alloc memory for UnitInfoPanel\n");
		return false;
	}

	return true;
}

// free memory
bool UnitInfoPanel::free() {
	printf("Freeing UnitInfoPanel\n");
	if (!mpUnitPanel) {
		printf("ERROR: failed to free memory for UnitInfoPanel\n");
		return false;
	}
	delete mpUnitPanel;
	mpUnitPanel = NULL;
	return true;
}

// start drawing
bool UnitInfoPanel::show() {
	if (!mpUnitPanel) {
		//printf("ERROR: failed to access memory for UnitInfoPanel\n");
		return false;
	}
	mpUnitPanel->show();
	return true;
}

// stop drawing
bool UnitInfoPanel::hide() {
	if (!mpUnitPanel) {
		printf("ERROR: failed to access memory for UnitInfoPanel\n");
		return false;
	}
	mpUnitPanel->hide();
	return true;
}

// MODE SELECTORS
void UnitInfoPanel::setBasicMode() {
	mPanelMode = 0;
} // mode 0

void UnitInfoPanel::setFuzzyNavMode() {
	mPanelMode = 1;
} // mode 1

void UnitInfoPanel::setBallisticsMode() {
	mPanelMode = 2;
} // mode 2

void UnitInfoPanel::setCrewMode() {
	mPanelMode = 3;
} // mode 3

void UnitInfoPanel::cycleMode() {
	mPanelMode = ++mPanelMode % PANEL_MODE_COUNT;
}

// BASIC DETAILS (mode 0)
bool UnitInfoPanel::updateUnitDetails(const char *desig, const char *name, int force, bool target, int tid) {
	if (!mpUnitPanel) {
		//printf("ERROR: failed to access memory for UnitInfoPanel\n");
		return false;
	}
	OverlayElement* desigOvA = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat0");
	OverlayElement* desigOvB = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat0B");
	OverlayElement* nameA = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat1");
	OverlayElement* nameB = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat1B");
	OverlayElement* targetOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat4");
	// set colourised name (matching force)
	if (force == 0) {
		// set designation
		desigOvA->setCaption(desig);
		nameA->setCaption(name);
		nameB->hide();
		desigOvB->hide();
		desigOvA->show();
		nameA->show();
	} else {
		// set designation
		desigOvB->setCaption(desig);
		nameB->setCaption(name);
		nameB->show();
		desigOvB->show();
		desigOvA->hide();
		nameA->hide();
	}
	// set target info
	if (!target) {
		targetOv->setCaption("No Targets");
	} else {
		targetOv->setCaption("Targeting: " + StringConverter::toString(tid));
	}
	targetOv->show();

	OverlayElement* actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat5"); // get action overlay 1
	actionOv->hide();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat6"); // get action overlay 2
	actionOv->hide();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat7"); // get action overlay 3
	actionOv->hide();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat8"); // get action overlay 4
	actionOv->hide();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat9"); // get action overlay 5
	actionOv->hide();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat10"); // get action overlay 6
	actionOv->hide();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat11"); // get action overlay 7
	actionOv->hide();

	return true;
}

// update unit details panel
bool UnitInfoPanel::updateUnitOrder(int orders) {
	if (!mpUnitPanel) {
		//printf("ERROR: failed to access memory for UnitInfoPanel\n");
		return false;
	}
	OverlayElement* ordersOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat2");

/*
#define ORDERS_NONE 0
#define ORDERS_MOVE 1
#define ORDERS_ATTACK 2
*/

	if (orders == 0) {	// 'No Orders' order
		ordersOv->setCaption("Orders: No Orders");
	} else if (orders == 1) {	// 'Move!' order
		ordersOv->setCaption("Orders: Move!");
	} else if (orders == 2) {	// 'Move!' order
		ordersOv->setCaption("Orders: Attack!");
	}
	ordersOv->show();
	return true;
}

bool UnitInfoPanel::setNavInputs(float targSqD, float targRad, float obstSqD, float obstRad, bool obstOnLeft) {
	if (!mpUnitPanel) {
		//printf("ERROR: failed to access memory for UnitInfoPanel\n");
		return false;
	}

	if (mPanelMode != 1) {
		return true; // don't bother updating if not in this mode
	}
	char tmp[60];
	OverlayElement* actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat4"); // get action overlay
	sprintf(tmp, "Targ:%.2frad@s%.2fm", targRad, targSqD);
	actionOv->setCaption(tmp);
	actionOv->show();

	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat5"); // get action overlay
	sprintf(tmp, "Obst:%.2frad@s%.2fm", obstRad, obstSqD);
	actionOv->setCaption(tmp);
	actionOv->show();

	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat6"); // get action overlay
	if (obstOnLeft) {
		sprintf(tmp, "Obstacle on LEFT side");
	} else {
		sprintf(tmp, "Obstacle on RIGHT side");
	}
	actionOv->setCaption(tmp);
	actionOv->show();
	return true;
}

bool UnitInfoPanel::setNavModes(bool tsMode, bool oaMode) {
	if (!mpUnitPanel) {
		//printf("ERROR: failed to access memory for UnitInfoPanel\n");
		return false;
	}
	if (mPanelMode != 1) {
		return true; // don't bother updating if not in this mode
	}
	char tmp[60];
	OverlayElement* actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat7"); // get action overlay
	if (tsMode && oaMode) {
		sprintf(tmp, "Targ.Seek:ON Obst.Avoi:ON");
	} else if (tsMode && !oaMode) {
		sprintf(tmp, "Targ.Seek:ON Obst.Avoi:OFF");
	} else if (!tsMode && oaMode) {
		sprintf(tmp, "Targ.Seek:OFF Obst.Avoi:ON");
	} else {
		sprintf(tmp, "Targ.Seek:OFF Obst.Avoi:OFF");
	}
	actionOv->setCaption(tmp);
	actionOv->show();
	return true;
}

bool UnitInfoPanel::setFuzzyInputMemsAngle(float small, float medium, float large) {
	if (!mpUnitPanel) {
		//printf("ERROR: failed to access memory for UnitInfoPanel\n");
		return false;
	}
	if (mPanelMode != 1) {
		return true; // don't bother updating if not in this mode
	}
	char tmp[60];
	OverlayElement* actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat8"); // get action overlay
	sprintf(tmp, "AngleIn{%.1f,%.1f,%.1f}",small,medium,large);
	actionOv->setCaption(tmp);
	actionOv->show();
	return true;
}

bool UnitInfoPanel::setFuzzyInputMemsDist(float near, float far, float vfar) {
	if (!mpUnitPanel) {
		//printf("ERROR: failed to access memory for UnitInfoPanel\n");
		return false;
	}
	if (mPanelMode != 1) {
		return true; // don't bother updating if not in this mode
	}
	char tmp[60];
	OverlayElement* actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat9"); // get action overlay
	//sprintf(tmp, "DistIn{%.1f,%.1f,%.1f}",near,far,vfar);
	actionOv->setCaption(tmp);
	actionOv->show();
	return true;
}

bool UnitInfoPanel::setFuzzyOutputMemsSteer(float a, float b, float c, float d, float e) {
	if (!mpUnitPanel) {
		//printf("ERROR: failed to access memory for UnitInfoPanel\n");
		return false;
	}
	if (mPanelMode != 1) {
		return true; // don't bother updating if not in this mode
	}
	char tmp[60];
	OverlayElement* actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat10"); // get action overlay
	sprintf(tmp, "AOut[%.1f][%.1f][%.1f][%.1f][%.1f]",a,b,c,d,e);
	actionOv->setCaption(tmp);
	actionOv->show();
	return true;
}

bool UnitInfoPanel::setFuzzyOutputMemsSpeed(float a, float b, float c, float d, float e, float f) {
	if (!mpUnitPanel) {
		//printf("ERROR: failed to access memory for UnitInfoPanel\n");
		return false;
	}
	if (mPanelMode != 1) {
		return true; // don't bother updating if not in this mode
	}
	char tmp[60];
	OverlayElement* actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat11"); // get action overlay
	sprintf(tmp, "SOut[%.1f][%.1f][%.1f][%.1f][%.1f][%.1f]",a,b,c,d,e,f);
	actionOv->setCaption(tmp);
	actionOv->show();
	return true;
}

bool UnitInfoPanel::setDefuzzifiedValues(float speed, float steering) {
	if (!mpUnitPanel) {
		//printf("ERROR: failed to access memory for UnitInfoPanel\n");
		return false;
	}
	if (mPanelMode != 1) {
		return true; // don't bother updating if not in this mode
	}
	char tmp[60];
	OverlayElement* actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat2"); // get action overlay
	sprintf(tmp, "Speed [%.2f]m/s",speed * 1000000.0f);
	actionOv->setCaption(tmp);
	actionOv->show();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat3");
	sprintf(tmp, "Steering [%.4f]rad/s", steering * 1000000.0f);
	actionOv->setCaption(tmp);
	actionOv->show();
	return true;
}

// ------------------------------ BALLISTICS (mode 2) ------------------------------
bool UnitInfoPanel::updateUnitAimingDetails(float horizDist, float yDist, float pitchArc, float muzzleVelocity) {
	if (!mpUnitPanel) {
		//printf("ERROR: failed to access memory for UnitInfoPanel\n");
		return false;
	}
	if (mPanelMode != 2) {
		return true; // don't bother updating if not in this mode
	}

	OverlayElement* actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat5"); // get action overlay
	char tmp[60];
	sprintf(tmp, "Dist. to Target: %.2fm", horizDist);
	actionOv->setCaption(tmp);
	actionOv->show();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat6"); // get action overlay
	sprintf(tmp, "Height to Target: %.2fm", yDist);
	actionOv->setCaption(tmp);
	actionOv->show();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat7"); // get action overlay
	sprintf(tmp, "Pitch Arc Req.: %.4frad", pitchArc);
	actionOv->setCaption(tmp);
	actionOv->show();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat8"); // get action overlay
	sprintf(tmp, "Muzzle Vel.: %.2fm/s", muzzleVelocity * 1000000.0f);
	actionOv->setCaption(tmp);
	actionOv->show();

	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat9"); // get action overlay
	actionOv->hide();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat10"); // get action overlay
	actionOv->hide();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat11"); // get action overlay
	actionOv->hide();
	return true;
}

// CREW (mode 3)

// update an action for an individual crew member
bool UnitInfoPanel::updateCrewAction(int crewIndex, int action) {
	if (!mpUnitPanel) {
		//printf("ERROR: failed to access memory for UnitInfoPanel\n");
		return false;
	}
	if (mPanelMode != 3) {
		return true; // don't bother updating if not in this mode
	}

	OverlayElement* actionOv = NULL;
	if (crewIndex == 0) {
		actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat5"); // get action overlay 1
	} else if (crewIndex == 1) {
		actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat6"); // get action overlay 2
	} else if (crewIndex == 2) {
		actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat7"); // get action overlay 3
	} else if (crewIndex == 3) {
		actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat8"); // get action overlay 4
	} else if (crewIndex == 4) {
		actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat9"); // get action overlay 5
	} else if (crewIndex == 5) {
		actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat10"); // get action overlay 6
	} else if (crewIndex == 6) {
		actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat11"); // get action overlay 7
	} else {
		// more crew members than spaces
		//printf("WARNING: too many crew members (more than 7) on selected vehicle\ncan not draw on GUI panel\n");
		return false;
	}
	/*
	#define ACTION_IDLE 0
	#define ACTION_DRIVING 1
	#define ACTION_TRAVERSING 2
	#define ACTION_AIMING 3
	#define ACTION_FIRING 4
	#define ACTION_RELOADING 5
	#define ACTION_SMOKED 6
	*/
	actionOv->show();

	if (action == 0) {
		actionOv->setCaption("Crewman: Idle");
	} else if (action == 1) {
		actionOv->setCaption("Crewman: Driving");
	} else if (action == 2) {
		actionOv->setCaption("Crewman: Traversing");
	} else if (action == 3) {
		actionOv->setCaption("Crewman: Aiming");
	} else if (action == 4) {
		actionOv->setCaption("Crewman: Firing");
	} else if (action == 5) {
		actionOv->setCaption("Crewman: Reloading");
	} else if (action == 6) {
		actionOv->setCaption("Crewman: Cabin Smoked");
	}
	return true;
}

// reset crew actions
bool UnitInfoPanel::resetCrewActions() {
	if (!mpUnitPanel) {
		//printf("ERROR: failed to access memory for UnitInfoPanel\n");
		return false;
	}
	if (mPanelMode != 3) {
		return true; // don't bother updating if not in this mode
	}

	OverlayElement* actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat5"); // get action overlay 1
	actionOv->hide();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat6"); // get action overlay 2
	actionOv->hide();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat7"); // get action overlay 3
	actionOv->hide();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat8"); // get action overlay 4
	actionOv->hide();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat9"); // get action overlay 5
	actionOv->hide();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat10"); // get action overlay 6
	actionOv->hide();
	actionOv = OverlayManager::getSingleton().getOverlayElement("mkiv/UnitStat11"); // get action overlay 7
	actionOv->hide();
	return true;
}

