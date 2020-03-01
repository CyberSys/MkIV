#ifndef SERVER_H_
#define SERVER_H_

/* This is the Network Server for markIV Engine.
 * It uses the Raknet library. I have included the Raknet headers and libs
 */

#include "Raknet/MessageIdentifiers.h"
#include "Raknet/RakNetworkFactory.h"
#include "Raknet/RakPeerInterface.h"
#include "Raknet/RakNetStatistics.h"
#include "Raknet/RakNetTypes.h"
#include "Raknet/BitStream.h"

// class representing the embedded network server (not a stand-alone server)
class Server {
public:
	// memory alloc/dealloc
	bool init();
	bool free();
	// server availability
	bool start();
	bool stop();
	// variables
	void setServerPort(int port);
	int  getServerPort();
	int getReadyCount();
	int getNotReadyCount();
	bool getReadyStatus();
	void setReadyStatus(bool ready);
	void toggleReadyStatus();
	// arrivals
	bool handleMail();
	// departures
	bool queryClientsReady();	// ask clients if they are ready to start
	bool sendScenarioDetailsToAll(char* name);
	bool sendScenarioDetailsToClient(int clientIndex, char* name);
	bool sendChat(char* msg);
	bool sendOrderMove(int vehicleID, float xp, float yp, float zp, float yr, float xd, float yd, float zd);
	bool sendOrderAttackVehicle(int vehicleID, float xp, float yp, float zp, float yr, int targetVehicle);
	bool sendOrderDestroyVehicle(int defender, int attacker, float xp, float yp, float zp, float yr);
	bool sendOrderCreateProjectile(float xp, float yp, float zp);
	bool sendOrderRicochet(int vehicleID, float xp, float yp, float zp, float yr);
	bool sendStart();
	bool sendCounter();
	bool ping(int userIndex);
	bool stat(int userIndex);
	bool gotOrderString();
	char* getOrderString();
	bool updateForceLists();
private:
	unsigned char GetPacketIdentifier(Packet *p);	// some code chopped from Raknet
	RakPeerInterface* mServer;	// library server interface
	int mServerPort;						// our port for incoming traffic
	int mClientReadyCount;			// a rough and temporary count of clients that are ready
	int mClientNotReadyCount;
	char mClientNames[36][15];	// names of clients connected to server
	int mClientForces[36];			// force each client controls
	char mOrderString[100];
	bool mGotOrderString;
	bool mReady;								// if we are 'ready' to start or not
};

#endif /*SERVER_H_*/
