#include "Client.h"
#include <string.h>
#include <stdio.h>
#include "../globals.h"

// initialise Client
bool Client::init() {
	return true;
}

// free allocated memory
bool Client::free() {
	return true;
}

// attempt to connect to server
bool Client::connect() {
	return true;
}

// disconnect from server
bool Client::disconnect() {
	return true;
}

// change our user handle
bool Client::login(char* name) {
	return true;
}

// set the ip to connect to
void Client::setIP(char* ip) {
}

// get the ip of the remote connection
char* Client::getIP() {
	return mIP;
}

// indicate if we are ready or not
bool Client::getReadyStatus() {
	return mReady;
}

// change our ready to start status
void Client::setReadyStatus(bool ready) {
}

// invert our ready status
void Client::toggleReadyStatus() {
}

// get the count of connected players that are ready
int Client::getReadyCount() {
	return mServersReadyCount;
}

// get the count of connected players that are not ready
int Client::getNotReadyCount() {
	return mServersNotReadyCount;
}

// send a message to the server
bool Client::sendMsg(char* msg) {
	return true;
}

// ask server for the current scenario name
bool Client::sendRequestScenario() {
	return true;
}

// send a move order request for a unit to server
bool Client::sendOrderMove(int vehicleID, float xd, float yd, float zd) {
	return true;
}

// send an attack order request for a unit to server
bool Client::sendOrderAttackVehicle(int vehicleID, int targetVehicle) {
	return true;
}

// request to control a force
bool Client::sendRequestForce(int force) {
	return true;
}

// tell server we are ready to start (or not)
bool Client::sendReadyStatus() {
	return true;
}

// ping the server
bool Client::ping() {
	return true;
}

// print server connection stats
bool Client::stat() {
	return true;
}

// check incoming packets
bool Client::handleMail() {
	return true;
}

// return true if have received an order
bool Client::gotOrderString() {
	return mGotOrderString;
}

// return latest order string
char* Client::getOrderString() {
	return mOrderString;
}

// return true if server told us to start
bool Client::gotStartMessage() {
	return false;
}

// are we connected yet??
bool Client::isConnected() {
	return mIsConnected;
}

