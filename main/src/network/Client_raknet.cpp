#include "Client.h"
#include <string.h>
#include <stdio.h>
#include "../globals.h"

// initialise Client
bool Client::init() {
	// set some defaults
	mClientPort = 60000;
	mServerPort = 10000;
	strcpy(mIP,"127.0.0.1");
	mIsConnected = false;
	mGotOrderString = false;
	strcpy(mOrderString,"");

	mClient = RakNetworkFactory::GetRakPeerInterface();
	mClient->InitializeSecurity(0,0,0,0);

	strcpy(g.mUserName,"UnknownSoldier");	// set default client name
	mReady = false;	// not ready to start
	mServersReadyCount = 0;
	mServersNotReadyCount = 0;
	mStart = false; // server not told us to start

	return true;
}

// free allocated memory
bool Client::free() {
	if (!mClient) {
		printf("ERROR: no client mem alloc, could not free client\n");
		return false;
	}
	RakNetworkFactory::DestroyRakPeerInterface(mClient);
	return true;
}

// attempt to connect to server
bool Client::connect() {
	if (!mClient) {
		printf("ERROR: no client mem alloc, could not connect\n");
		return false;
	}
	printf("Connecting to server at %s ...",mIP);
	
	// Connecting the client is very simple.  0 means we don't care about
	// a connectionValidationInteger, and false for low priority threads
	SocketDescriptor socketDescriptor(mClientPort,0);
	mClient->Startup(1,30,&socketDescriptor, 1);
	// try to connect to server. the last two params are password and password length
	// returning true does not necessarily mean connected (yet).
	if (!mClient->Connect(mIP,mServerPort, "",0)) {
		printf("[FAILED]\n");
		return false;
	}
	
	return true;
}

// disconnect from server
bool Client::disconnect() {
	if (!mClient) {
		printf("ERROR: no client mem alloc, could not disconnect\n");
		return false;
	}
	mClient->Shutdown(300);
	mIsConnected = false;

	return true;
}

// change our user handle
bool Client::login(char* name) {
	if (!mClient) {
		printf("ERROR: no client mem alloc, could not log in to server\n");
		return false;
	}
	strcpy(g.mUserName,name);
	// send user name to server
	char tmp[30];
	strcpy(tmp,"/login ");
	strcat(tmp,g.mUserName);
	mClient->Send(tmp, (int)strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	return true;
}

// set the ip to connect to
void Client::setIP(char* ip) {
	strcpy(mIP,ip);
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
	mReady = ready;
}

// invert our ready status
void Client::toggleReadyStatus() {
	mReady = !mReady;
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
	if (!mClient) {
		printf("ERROR: no client mem alloc, could not send msg\n");
		return false;
	}
	mClient->Send(msg, (int)strlen(msg)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	return true;
}

// ask server for the current scenario name
bool Client::sendRequestScenario() {
	if (!mClient) {
		printf("ERROR: no client mem alloc, could not send msg\n");
		return false;
	}
	char tmp[50];
	sprintf(tmp,"/reqCurrentScenarioName");
	mClient->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	return true;
}

// send a move order request for a unit to server
bool Client::sendOrderMove(int vehicleID, float xd, float yd, float zd) {
	if (!mClient) {
		printf("ERROR: no client mem alloc, could not send msg\n");
		return false;
	}
	char tmp[50];
	sprintf(tmp,"/order mov [%i] {%.3f,%.3f,%.3f}",vehicleID,xd,yd,zd);
	mClient->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	
	return true;
}

// send an attack order request for a unit to server
bool Client::sendOrderAttackVehicle(int vehicleID, int targetVehicle) {
	if (!mClient) {
		printf("ERROR: no client mem alloc, could not send msg\n");
		return false;
	}
	char tmp[50];
	sprintf(tmp,"/order atv [%i] [%i]",vehicleID,targetVehicle);
	mClient->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	
	return true;
}

// request to control a force
bool Client::sendRequestForce(int force) {
	if (!mClient) {
		printf("ERROR: no client mem alloc, could not send msg\n");
		return false;
	}
	char tmp[50];
	sprintf(tmp,"/reqForce %i", force);
	mClient->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);

	return true;
}

// tell server we are ready to start (or not)
bool Client::sendReadyStatus() {
	if (!mClient) {
		printf("ERROR: no client mem alloc, could not send msg\n");
		return false;
	}
	char tmp[50];
	if (mReady) {
		sprintf(tmp,"/ready");
		mClient->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	} else {
		sprintf(tmp,"/notReady");
		mClient->Send(tmp, (const int) strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
	}
	return true;
}

// ping the server
bool Client::ping() {
	if (!mClient) {
		printf("ERROR: no client mem alloc, could not send msg\n");
		return false;
	}
	if (mClient->GetSystemAddressFromIndex(0) != UNASSIGNED_SYSTEM_ADDRESS) {
			mClient->Ping(mClient->GetSystemAddressFromIndex(0));
			char pingMsg[20];
			sprintf(pingMsg,"Ping Server = %i",mClient->GetAveragePing(mClient->GetSystemAddressFromIndex(0)));
			printf("%s\n", pingMsg);
			// write ping info in chat console
			g.mGUI.addChat(pingMsg);
	} else {
		return false;	// not a valid user index
	}

	return true;
}

// print server connection stats
bool Client::stat() {
	char stats[2048];
	RakNetStatistics* rss;			// Network Statistics
	rss = mClient->GetStatistics(mClient->GetSystemAddressFromIndex(0));
	StatisticsToString(rss, stats, 2);
	printf("%s", stats);
	char pingMsg[20];
	sprintf(pingMsg,"Ping Server = %i",mClient->GetAveragePing(mClient->GetSystemAddressFromIndex(0)));
	printf("%s\n", pingMsg);
	// write ping info in chat console
	g.mGUI.addChat(pingMsg);

	return true;
}

// check incoming packets
bool Client::handleMail() {
	if (!mClient) {
		printf("ERROR: no client mem alloc, could not process mail\n");
		return false;
	}
	// Holds packets
	Packet* p;
	// GetPacketIdentifier returns this
	unsigned char packetIdentifier;
	mGotOrderString = false;	// for checking if received msg is an order for units

	// Get a packet from either the server or the client
	p = mClient->Receive();
		
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
			mIsConnected = false;
			g.mGUI.addChat("System: Server Disconnected");
			break;

		case ID_ALREADY_CONNECTED:
			// Connection lost normally
			printf("ID_ALREADY_CONNECTED\n");
			g.mGUI.addChat("System: Already Connected");
			break;

		case ID_REMOTE_DISCONNECTION_NOTIFICATION: // Server telling the clients of another client disconnecting gracefully.  You can manually broadcast this in a peer to peer enviroment if you want.
			printf("ID_REMOTE_DISCONNECTION_NOTIFICATION\n");
			g.mGUI.addChat("System: A Client Disconnected");
			break;

		case ID_REMOTE_CONNECTION_LOST: // Server telling the clients of another client disconnecting forcefully.  You can manually broadcast this in a peer to peer enviroment if you want.
			printf("ID_REMOTE_CONNECTION_LOST\n");
			g.mGUI.addChat("System: Lost a Client");
			break;

		case ID_REMOTE_NEW_INCOMING_CONNECTION: // Server telling the clients of another client connecting.  You can manually broadcast this in a peer to peer enviroment if you want.
			printf("ID_REMOTE_NEW_INCOMING_CONNECTION\n");
			g.mGUI.addChat("System: A Client Connected");
			break;

		case ID_CONNECTION_BANNED: // Banned from this server
			printf("We are banned from this server.\n");
			g.mGUI.addChat("System: We are banned from this server.");
			break;

		case ID_CONNECTION_ATTEMPT_FAILED:
			printf("Connection attempt failed\n");
			g.mGUI.addChat("System: Connection attempt failed.");
			break;

		case ID_NO_FREE_INCOMING_CONNECTIONS:
			 // Sorry, the server is full.  I don't do anything here but
			// A real app should tell the user
			printf("ID_NO_FREE_INCOMING_CONNECTIONS\n");
			g.mGUI.addChat("System: server full.");
			break;

		case ID_MODIFIED_PACKET:
			// Cheater!
			printf("ID_MODIFIED_PACKET\n");
			break;

		case ID_INVALID_PASSWORD:
			printf("ID_INVALID_PASSWORD\n");
			break;

		case ID_CONNECTION_LOST:
			// Couldn't deliver a reliable packet - i.e. the other system was abnormally
			// terminated
			printf("ID_CONNECTION_LOST\n");
			g.mGUI.addChat("System: Lost connection to server");
			mIsConnected = false;
			break;

		case ID_CONNECTION_REQUEST_ACCEPTED:
			// This tells the client they have connected
			printf("ID_CONNECTION_REQUEST_ACCEPTED\n");
			g.mGUI.addChat("System: Connected to server");
			printf("Connected to Server\n");
			mIsConnected = true;
			// login to server
			char tmp[30];
			strcpy(tmp,"/login ");
			strcat(tmp,g.mUserName);
			mClient->Send(tmp, (int)strlen(tmp)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);			
			break;

		default:
			// get the message from the packet as a string
			char rxString[256];
			sprintf(rxString,"%s",p->data);
			
			// check if was a system message
			if (strncmp(rxString,"/login",6) == 0) {
				// do nothing
			} else if (strncmp(rxString,"/setName",8) == 0) {
				// do nothing
			} else if (strncmp(rxString, "/order", 6) == 0) {
				// indicate that have found an order (will let UnitHandler retrieve it)
				mGotOrderString = true;
				strcpy(mOrderString,rxString);
			} else if (strncmp(rxString,"/queryReady",11) == 0) {
				// we are being asked if we are ready - reply
				sendReadyStatus();
			} else if (strncmp(rxString,"/start",6) == 0) {
				mStart = true;
			} else if (strncmp(rxString,"/readyCount",11) == 0) {
				sscanf(rxString,"/readyCount %i,%i",&mServersReadyCount,&mServersNotReadyCount);
			} else if (strncmp(rxString,"/clearAllLobbyLists",19) == 0) {
				g.mGUI.clearAllLobbyLists();
			} else if (strncmp(rxString,"/pushLobbySpecName",18) == 0) {
				char specName[15];
				sscanf(rxString,"/pushLobbySpecName %s",specName);
				g.mGUI.pushLobbySpecName(specName);
			} else if (strncmp(rxString,"/pushLobbyF0Name",16) == 0) {
				char f0Name[15];
				sscanf(rxString,"/pushLobbyF0Name %s",f0Name);
				g.mGUI.pushLobbyF0Name(f0Name);
			} else if (strncmp(rxString,"/pushLobbyF1Name",16) == 0) {
				char f1Name[15];
				sscanf(rxString,"/pushLobbyF1Name %s",f1Name);
				g.mGUI.pushLobbyF1Name(f1Name);
			} else if (strncmp(rxString,"/newForce",9) == 0) {
				sscanf(rxString,"/newForce %i",&g.mOurForce);
			} else if (strncmp(rxString, "/scenario", 9) == 0) {
				sscanf(rxString,"/scenario %s", g.mScenario);
				g.mNeedToUpdateScenario = true;
			} else {
				// It's a client, so just show the message
				printf("%s\n", rxString);
				g.mGUI.addChat(rxString);
			}
			break;
	}

	// We're done with the packet
	mClient->DeallocatePacket(p);

	return true;
}

// Copied from Multiplayer.cpp
// If the first byte is ID_TIMESTAMP, then we want the 5th byte
// Otherwise we want the 1st byte
unsigned char Client::GetPacketIdentifier(Packet *p) {
	if (p == 0) {
		return 255;
	}

	if ((unsigned char)p->data[0] == ID_TIMESTAMP) {
		assert(p->length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char) p->data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	return (unsigned char) p->data[0];
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
	if (mStart) {
		// reset again
		mStart = false;
		return true;
	}
	return false;
}

// are we connected yet??
bool Client::isConnected() {
	return mIsConnected;
}
