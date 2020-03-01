#ifndef CLIENT_H_
#define CLIENT_H_

/* This is the Network Client for markIV Engine.
 * It uses the Raknet library. I have included the Raknet headers and libs
 */

#include "Raknet/MessageIdentifiers.h"
#include "Raknet/RakNetworkFactory.h"
#include "Raknet/RakPeerInterface.h"
#include "Raknet/RakNetStatistics.h"
#include "Raknet/RakNetTypes.h"
#include "Raknet/BitStream.h"

// Class representing a network client
class Client {
public:
	// memory and variables
	bool init();
	bool free();
	// connectivity
	bool connect();
	bool disconnect();
	// details
	void setIP(char* ip);
	char* getIP();
	bool getReadyStatus();
	void setReadyStatus(bool ready);
	int getReadyCount();
	int getNotReadyCount();
	void toggleReadyStatus();
	// arrivals
	bool handleMail();
	// departures
	bool login(char* name);	// login to server with our user name
	bool sendMsg(char *msg);
	bool sendRequestScenario();
	bool sendOrderMove(int vehicleID, float xd, float yd, float zd);
	bool sendOrderAttackVehicle(int vehicleID, int targetVehicle);
	bool sendRequestForce(int force);
	bool sendReadyStatus();
	bool ping();
	bool stat();
	bool gotOrderString();
	char* getOrderString();
	bool gotStartMessage();
	bool isConnected();
private:
	unsigned char GetPacketIdentifier(Packet *p);	// some code chopped from Raknet
	RakPeerInterface* mClient;	// The library's client interface
	int mClientPort;						// Our Port
	int mServerPort;						// Port of server
	char mIP[20];								// IP addr of server
	bool mIsConnected;					// if connected to server yet or not
	char mOrderString[100];
	bool mGotOrderString;
	bool mReady;								// if we are 'ready' to start or not
	int mServersReadyCount;
	int mServersNotReadyCount;
	bool mStart;								// if the server has told us to start or not
};

#endif /*CLIENT_H_*/
