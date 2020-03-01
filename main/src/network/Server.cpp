#include "Server.h"
#include <string.h>
#include <stdio.h>
#include "../globals.h"

// initialise Server
bool Server::init() {
	return true;
}

// free allocated memory
bool Server::free() {
	return true;
}

// start the server
bool Server::start() {
	return true;
}

// stop the server
bool Server::stop() {
	return true;
}

// change our incoming port
void Server::setServerPort(int port) {
}

// return our incoming port
int Server::getServerPort() {
	return mServerPort;
}

// get the count of connected clients that are ready + the server if it is also ready
int Server::getReadyCount() {
	return mClientReadyCount;
}

// get the count of connected clients that are not ready + the server if it is also not ready
int Server::getNotReadyCount() {
	return mClientNotReadyCount;
}

void Server::setReadyStatus(bool ready) {
}

bool Server::getReadyStatus() {
	return mReady;
}

void Server::toggleReadyStatus() {
}

// check incoming packets
bool Server::handleMail() {
	return true;
}

// ask clients if they are ready to start
bool Server::queryClientsReady() {
	return true;
}

bool Server::sendChat(char* msg) {
	return true;
}

// send the selected scenario details to all clients
// TODO - send something for file/version integrity
bool Server::sendScenarioDetailsToAll(char* name) {
	return true;
}

// send an order for a unit to all clients on network. Params are unitID,current pos,orient, destination
bool Server::sendOrderMove(int vehicleID, float xp, float yp, float zp, float yr, float xd, float yd, float zd) {
	return true;
}

// send an order for a unit to all clients on network. Params are unitID,current pos,orient, target index
bool Server::sendOrderAttackVehicle(int vehicleID, float xp, float yp, float zp, float yr, int targetVehicle) {
	return true;
}

bool Server::sendOrderDestroyVehicle(int defender, int attacker, float xp, float yp, float zp, float yr) {
	return true;
}

bool Server::sendOrderCreateProjectile(float xp, float yp, float zp) {
	return true;
}

bool Server::sendOrderRicochet(int vehicleID, float xp, float yp, float zp, float yr) {
	return true;
}

bool Server::sendStart() {
	return true;
}

// tell clients what the ready counter is
bool Server::sendCounter() {
	return true;
}

// ping user
bool Server::ping(int userIndex) {
	return true;
}

// stat a given user
bool Server::stat(int userIndex) {
	return true;
}

// return true if have received an order
bool Server::gotOrderString() {
	return mGotOrderString;
}

// return latest order string (from a client)
char* Server::getOrderString() {
	return mOrderString;
}

// update force list gui and send to client
// NOTE: many of these functions would be better as RPCs
bool Server::updateForceLists() {
	return true;
}

