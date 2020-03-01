#include "Server.h"
#include <string.h>
#include <stdio.h>
#include "../globals.h"

// initialise Server
bool Server::init() {
	printf("Initialising Server...");
	// allocate memory to interface
	mServer = RakNetworkFactory::GetRakPeerInterface();
	mServer->InitializeSecurity(0,0,0,0);
	mServerPort = 10000;
	strcpy(g.mUserName,"Admin");
	mGotOrderString = false;
	strcpy(mOrderString,"");
	for (int i=0;i<36;i++) {
		strcpy(mClientNames[i],"Player");
		mClientForces[i] = -2;	// set to observers by default
	}
	mClientReadyCount = 0;
	mClientNotReadyCount = 0;
	mReady = false;
	printf("[OK]\n");

	return true;
}

// free allocated memory
bool Server::free() {
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not free\n");
		return false;
	}
	RakNetworkFactory::DestroyRakPeerInterface(mServer);
	return true;
}

// start the server
bool Server::start() {
	printf("Starting Server...");
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not start server\n");
		return false;
	}
	
	// 0 means we don't care about a connectionValidationInteger
	SocketDescriptor socketDescriptor(mServerPort,0);

	// start the server (32 connections possible, sleep 30ms - gives CPU priority to game (if 1 CPU), 0 is reccomended for fastest network, port to listen on, size of socketDarray).
	if (!mServer->Startup(32, 30, &socketDescriptor, 1)) {
		printf("[FAILED]\nERROR: Server Failed to Start!\n");
		return false;
	}
	// 2 players allowed
	mServer->SetMaximumIncomingConnections(2);
	printf("[OK]\n");

	return true;
}

// stop the server
bool Server::stop() {
	printf("Stopping Server...");
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not stop server\n");
		return false;
	}
	mServer->Shutdown(300);
	printf("[OK]\n");
	return true;
}

// change our incoming port
void Server::setServerPort(int port) {
	mServerPort = port;
}

// return our incoming port
int Server::getServerPort() {
	return mServerPort;
}

// get the count of connected clients that are ready + the server if it is also ready
int Server::getReadyCount() {
	if (mReady) {
		return mClientReadyCount + 1;
	}
	return mClientReadyCount;
}

// get the count of connected clients that are not ready + the server if it is also not ready
int Server::getNotReadyCount() {
	if (!mReady) {
		return mClientNotReadyCount + 1;
	}
	return mClientNotReadyCount;
}

void Server::setReadyStatus(bool ready) {
	mReady = ready;
}

bool Server::getReadyStatus() {
	return mReady;
}

void Server::toggleReadyStatus() {
	mReady = !mReady;
}

// Copied from Multiplayer.cpp
// If the first byte is ID_TIMESTAMP, then we want the 5th byte
// Otherwise we want the 1st byte
unsigned char Server::GetPacketIdentifier(Packet *p) {
	if (p==0) {
		return 255;
	}
	if ((unsigned char)p->data[0] == ID_TIMESTAMP) {
		assert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	return (unsigned char) p->data[0];
}

// check incoming packets
bool Server::handleMail() {
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not send msg\n");
		return false;
	}

	// Holds packets
	Packet* p;
	// GetPacketIdentifier returns this
	unsigned char packetIdentifier;
	mGotOrderString = false;

	// Get a packet from either the server or the client
	p = mServer->Receive();
		
	if (p == 0) {
		return true; // Didn't get any packets
	}

	// We got a packet, get the identifier with our handy function
	packetIdentifier = GetPacketIdentifier(p);

	// Check if this is a network message packet
	switch (packetIdentifier) {
		case ID_DISCONNECTION_NOTIFICATION:
			// Connection lost normally		
			printf("ID_DISCONNECTION_NOTIFICATION\n");
			g.mGUI.addChat("System: Client Disconnected");
			break;
		
		case ID_NEW_INCOMING_CONNECTION:
			// Somebody connected.  We have their IP now
			printf("ID_NEW_INCOMING_CONNECTION\n");
			g.mGUI.addChat("System: Client Connected");
			break;

		case ID_MODIFIED_PACKET:
			// Cheater!
			printf("ID_MODIFIED_PACKET\n");
			break;

		case ID_CONNECTION_LOST:
			// Couldn't deliver a reliable packet - i.e. the other system was abnormally
			// terminated
			printf("ID_CONNECTION_LOST\n");
			g.mGUI.addChat("System: Lost a Client");
			break;

		default:
			// The server knows the static data of all clients, so we can prefix the message
			// With the name data
			// printf("%s\n", p->data);

			char rxString[256];
			sprintf(rxString, "%s", p->data);
			SystemAddress clientID = p->systemAddress;

			// print all received messages to server console

			// find username from records
			char extractedName[15];
			int userIndex = mServer->GetIndexFromSystemAddress(clientID);
			//printf("DEBUG: userindex = %i\n",userIndex);
			strcpy(extractedName,mClientNames[userIndex]);

			// check if was a system message
			if (strncmp(rxString,"/login",6) == 0) {
				char userName[15];
				// get rest of string until terminating char
				StripString(rxString,7,userName,15);
				printf("user %s logged in.\n", userName);
				// assign the user to a force (observers by default)
				mClientForces[userIndex] = -1;
				// match to our recorded user
				strcpy(mClientNames[userIndex],userName);
				updateForceLists();	// update lobby GUI (maybe not in lobby but what the hell)
			} else if (strncmp(rxString,"/order",6) == 0) {
				// inicate that have found an order (will let UnitHandler retrieve it)
				mGotOrderString = true;
				strcpy(mOrderString,rxString);
			} else if (strncmp(rxString,"/ready",6) == 0) {
				mClientReadyCount++;
			} else if (strncmp(rxString,"/notReady",9) == 0) {
				mClientNotReadyCount++;
			} else if (strncmp(rxString,"/reqForce",9) == 0) {
				// change player's force
				sscanf(rxString,"/reqForce %i",&mClientForces[userIndex]);
				// tell player that we have changed their force
				char tmp[256];
				sprintf(tmp,"/newForce %i",mClientForces[userIndex]);
				mServer->Send(tmp, (const int)strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED,userIndex, p->systemAddress,false);	// last two params mean 'only send to the guy who sent it to us'
				updateForceLists();
			} else {
				if (userIndex == -1) {
					char msg[256];
					sprintf(msg,"%s",p->data);
					//printf("%s\n", msg);	// could not find user - just print command
					g.mGUI.addChat(msg);
				} else {
					char msg[256];
					sprintf(msg,"%s: %s", extractedName, rxString);
					//printf("%s\n",msg);
					g.mGUI.addChat(msg);
				}
				// Relay the message.  We prefix the name for other clients.  This demonstrates
				// That messages can be changed on the server before being broadcast
				// Sending is the same as before
					char message[256];
				sprintf(message, "%s", p->data);
				mServer->Send(message, (const int)strlen(message)+1, HIGH_PRIORITY, RELIABLE_ORDERED,0, p->systemAddress,true);	// the last two parms mean 'dont send back to the guy who sent it to us'
				// TODO !!! IS this actually working??? what does systemAddress do? (exclude that one?)
			}
			
			break;
	}

	// We're done with the packet
	mServer->DeallocatePacket(p);

	return true;
}

// ask clients if they are ready to start
bool Server::queryClientsReady() {
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not send msg\n");
		return false;
	}
	// reset ready counts
	mClientReadyCount = 0;
	mClientNotReadyCount = 0;

	// send query
	char tmp[50];
	strcpy(tmp,"/queryReady");
	mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all
	return true;
}

// send a chat message (written by the user on the server) to everyone
bool Server::sendChat(char* msg) {
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not send msg\n");
		return false;
	}
	char tmp[300];
	strcpy(tmp,g.mUserName);
	strcat(tmp,": ");
	strcat(tmp,msg);
	mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all
	return true;
}

// send the selected scenario details to all clients
// TODO - send something for file/version integrity
bool Server::sendScenarioDetailsToAll(char* name) {
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not send msg\n");
		return false;
	}
	char tmp[300];
	sprintf(tmp,"/scenario %s", name);
	mServer->Send(tmp, (const int)strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all

	return true;
}

// send the selected scenario details to a given clients
// TODO - send something for file/version integrity
bool Server::sendScenarioDetailsToClient(int clientIndex, char* name) {
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not send msg\n");
		return false;
	}
	char tmp[300];
	mServer->Send(tmp, (const int)strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, clientIndex, UNASSIGNED_SYSTEM_ADDRESS, false);	// last two params mean 'only send to this guy
	// TODO: not sure about param 2nd to last - should be an address

	return true;
}

// send an order for a unit to all clients on network. Params are unitID,current pos,orient, destination
bool Server::sendOrderMove(int vehicleID, float xp, float yp, float zp, float yr, float xd, float yd, float zd) {
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not send msg\n");
		return false;
	}
	char tmp[300];
	// send order in form [unitID] {current pos} {current orient} {destination}
	// note: client just sends order without current pos and orient
	sprintf(tmp,"/order mov [%i] {%.3f,%.3f,%.3f} {%.3f} {%.3f,%.3f,%.3f}",vehicleID,xp,yp,zp,yr,xd,yd,zd);
	mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all
	
	return true;
}

// send an order for a unit to all clients on network. Params are unitID,current pos,orient, target index
bool Server::sendOrderAttackVehicle(int vehicleID, float xp, float yp, float zp, float yr, int targetVehicle) {
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not send msg\n");
		return false;
	}
	char tmp[300];
	sprintf(tmp,"/order atv [%i] {%.3f,%.3f,%.3f} {%.3f} [%i]", vehicleID, xp, yp, zp, yr, targetVehicle); // send order in form [unitID] {current pos} {current orient} [targetID]
	mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all
	return true;
}

bool Server::sendOrderDestroyVehicle(int defender, int attacker, float xp, float yp, float zp, float yr) {
	if (!mServer) {
		printf("ERROR: Server::sendDestroyVehicle - can not send msg\n");
		return false;
	}
	char tmp[256];
	sprintf(tmp,"/order kil [%i][%i] {%.3f,%.3f,%.3f} {%.3f}", defender, attacker, xp, yp, zp, yr); // send order in form [unitID] {current pos} {current orient} [targetID]
	mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all
	return true;
}

bool Server::sendOrderCreateProjectile(float xp, float yp, float zp) {
	if (!mServer) {
		printf("ERROR: Server::sendOrderCreateProjectile - can not send msg\n");
		return false;
	}
	char tmp[256];
	sprintf(tmp,"/order cpj {%.3f,%.3f,%.3f}", xp, yp, zp); // send order in form [unitID] {current pos} {current orient} [targetID]
	mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all
	return true;
}

bool Server::sendOrderRicochet(int vehicleID, float xp, float yp, float zp, float yr) {
	if (!mServer) {
		printf("ERROR: Server::sendOrderRicochet - can not send msg\n");
		return false;
	}
	char tmp[256];
	sprintf(tmp,"/order ric [%i] {%.3f,%.3f,%.3f} {%.3f}", vehicleID, xp, yp, zp, yr); // send order in form [unitID] {current pos} {current orient} [targetID]
	mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all
	return true;
}

// tell clients to start game
bool Server::sendStart() {
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not send msg\n");
		return false;
	}
	char tmp[50];
	sprintf(tmp,"/start");
	mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all

	return true;
}

// tell clients what the ready counter is
bool Server::sendCounter() {
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not send msg\n");
		return false;
	}
	char tmp[50];
	sprintf(tmp,"/readyCount %i,%i",getReadyCount(),getNotReadyCount());
	mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all

	return true;
}

// ping user
bool Server::ping(int userIndex) {
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not send msg\n");
		return false;
	}
	if (mServer->GetSystemAddressFromIndex(userIndex) != UNASSIGNED_SYSTEM_ADDRESS) {
			mServer->Ping(mServer->GetSystemAddressFromIndex(userIndex));
			char pingMsg[20];
			sprintf(pingMsg,"Ping Client[%i] = %i",userIndex,mServer->GetAveragePing(mServer->GetSystemAddressFromIndex(userIndex)));
			printf("%s\n", pingMsg);
			// write ping info in chat console
			g.mGUI.addChat(pingMsg);
	} else {
		return false;	// not a valid user index
	}

	return true;
}

// stat a given user
bool Server::stat(int userIndex) {
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not send msg\n");
		return false;
	}
	char stats[2048]; // printout
	RakNetStatistics* rss;			// Network Statistics
	rss = mServer->GetStatistics(mServer->GetSystemAddressFromIndex(userIndex));
	StatisticsToString(rss, stats, 2);
	printf("%s", stats);
	char pingMsg[20];
	sprintf(pingMsg,"Ping Client[%i] = %i",userIndex,mServer->GetAveragePing(mServer->GetSystemAddressFromIndex(userIndex)));
	// write ping info in chat console
	g.mGUI.addChat(pingMsg);

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
	if (!mServer) {
		printf("ERROR: peer interface uninit - can not send msg\n");
		return false;
	}
	char tmp[256];
	// clear all lists
	g.mGUI.clearAllLobbyLists();

	// tell client to clear all lists
	sprintf(tmp,"/clearAllLobbyLists");
	mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all

	// add us to lobby lists (and tell client to add us)
	if (g.mOurForce == -1) {
		g.mGUI.pushLobbySpecName(g.mUserName);
		sprintf(tmp,"/pushLobbySpecName %s", g.mUserName);
	mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all
	} else if (g.mOurForce == 0) {
		g.mGUI.pushLobbyF0Name(g.mUserName);
		sprintf(tmp,"/pushLobbyF0Name %s", g.mUserName);
	mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all
	} else if (g.mOurForce == 1) {
		g.mGUI.pushLobbyF1Name(g.mUserName);
		sprintf(tmp,"/pushLobbyF1Name %s", g.mUserName);
	mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all
	}
	
	// add all the clients
	for (int i = 0; i < 36; i++) {
		if (mClientForces[i] == -1) {
			g.mGUI.pushLobbySpecName(mClientNames[i]);
			sprintf(tmp,"/pushLobbySpecName %s", mClientNames[i]);
			mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all
		} else if (mClientForces[i] == 0) {
			g.mGUI.pushLobbyF0Name(mClientNames[i]);
			sprintf(tmp,"/pushLobbyF0Name %s", mClientNames[i]);
			mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all
		} else if (mClientForces[i] == 1) {
			g.mGUI.pushLobbyF1Name(mClientNames[i]);
			sprintf(tmp,"/pushLobbyF1Name %s", mClientNames[i]);
			mServer->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);	// send to all
		}
	}

	return true;
}
