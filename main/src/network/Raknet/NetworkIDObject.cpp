/// \file
///
/// This file is part of RakNet Copyright 2003 Kevin Jenkins.
///
/// Usage of RakNet is subject to the appropriate license agreement.
/// Creative Commons Licensees are subject to the
/// license found at
/// http://creativecommons.org/licenses/by-nc/2.5/
/// Single application licensees are subject to the license found at
/// http://www.rakkarsoft.com/SingleApplicationLicense.html
/// Custom license users are subject to the terms therein.
/// GPL license users are subject to the GNU General Public
/// License as published by the Free
/// Software Foundation; either version 2 of the License, or (at your
/// option) any later version.

#include "NetworkIDObject.h"
#include "NetworkIDManager.h"
#include "RakAssert.h"

#ifdef _COMPATIBILITY_1
#elif defined(_WIN32)
#include <malloc.h> // alloca
#elif defined(_COMPATIBILITY_2)
#include "Compatibility2Includes.h"
#else
#include <stdlib.h> // alloca
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

NetworkIDObject::NetworkIDObject()
{
	callGenerationCode=true;
	networkID=UNASSIGNED_NETWORK_ID;
	parent=0;
	networkIDManager=0;
}

//-------------------------------------------------------------------------------------


NetworkIDObject::~NetworkIDObject()
{
	if (networkID!=UNASSIGNED_NETWORK_ID)
	{
		NetworkIDNode * object = networkIDManager->IDTree.GetPointerToNode( NetworkIDNode( ( networkID ), 0 ) );
		if ( object && object->object == this )
			networkIDManager->IDTree.Del( NetworkIDNode( object->networkID, 0 ) );
	}
}

//////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////

void NetworkIDObject::SetNetworkIDManager( NetworkIDManager *manager)
{
	networkIDManager = manager;
}

//-------------------------------------------------------------------------------------

NetworkIDManager * NetworkIDObject::GetNetworkIDManager( void )
{
	return networkIDManager;
}

//-------------------------------------------------------------------------------------

NetworkID NetworkIDObject::GetNetworkID( void )
{
	RakAssert(networkIDManager);
	if (callGenerationCode && networkIDManager->IsNetworkIDAuthority())
	{
		GenerateID();
		callGenerationCode=false;
	}

	RakAssert(networkID!=UNASSIGNED_NETWORK_ID);
	return networkID;
};
//-------------------------------------------------------------------------------------

bool NetworkIDObject::RequiresSetParent(void) const
{
	return false;
}

//-------------------------------------------------------------------------------------

void NetworkIDObject::SetNetworkID( NetworkID id )
{
	callGenerationCode=false;

	if ( id == UNASSIGNED_NETWORK_ID )
	{
		// puts("Warning: NetworkIDObject passed UNASSIGNED_NETWORK_ID.  SetID ignored");
		return ;
	}

	if ( networkID == id )
	{
		// printf("NetworkIDObject passed %i which already exists in the tree.  SetID ignored", id);
		return ;
	}

	RakAssert(networkIDManager);
	NetworkIDNode* collision = networkIDManager->IDTree.GetPointerToNode( NetworkIDNode( ( id ), 0 ) );

	if ( collision )   // Tree should have only unique values.  The new value is already in use.
	{
		//printf("Warning: NetworkIDObject::SetID passed %i, which has an existing node in the tree.  Old node removed, which will cause the item pointed to to be inaccessible to the network", id);
		networkIDManager->IDTree.Del( NetworkIDNode( collision->networkID, collision->object ) );
	}

	if ( networkID == UNASSIGNED_NETWORK_ID )   // Object has not had an ID assigned so does not already exist in the tree
	{
		networkID = id;
		networkIDManager->IDTree.Add( NetworkIDNode( networkID, this ) );
	}
	else // Object already exists in the tree and has an assigned ID
	{
		networkIDManager->IDTree.Del( NetworkIDNode( networkID, this ) ); // Delete the node with whatever ID the existing object is using
		networkID = id;
		networkIDManager->IDTree.Add( NetworkIDNode( networkID, this ) );
	}
}

//-------------------------------------------------------------------------------------
void NetworkIDObject::SetParent( void *_parent )
{
	parent=_parent;

#ifdef _DEBUG
	if (networkIDManager)
	{
		// Avoid duplicate parents in the tree
		unsigned size = networkIDManager->IDTree.Size();
		NetworkIDNode *nodeArray;

		bool usedAlloca=false;
#if !defined(_COMPATIBILITY_1)
		if (sizeof(NetworkIDNode) * size < MAX_ALLOCA_STACK_ALLOCATION)
		{
			nodeArray = (NetworkIDNode*) alloca(sizeof(NetworkIDNode) * size);
			usedAlloca=true;
		}
		else
#endif
			nodeArray = new NetworkIDNode[size];

		networkIDManager->IDTree.DisplayBreadthFirstSearch( nodeArray );
		unsigned i;
		for (i=0; i < size; i++)
		{
			// If this assert hits then this _parent is already in the tree.  Classes instance should never contain more than one NetworkIDObject
			RakAssert(nodeArray->object->GetParent()!=parent);
		}

		if (usedAlloca==false)
			delete [] nodeArray;

	}
	
#endif
}

//-------------------------------------------------------------------------------------
void* NetworkIDObject::GetParent( void ) const
{
	return parent;
}
//-------------------------------------------------------------------------------------
void NetworkIDObject::GenerateID(void)
{
	RakAssert(networkIDManager->IsNetworkIDAuthority());
	// If you want more than 65535 network objects, change the type of networkID
	RakAssert(networkIDManager->IDTree.Size() < 65535);

	NetworkIDNode* collision;
	do
	{
		networkID.localSystemAddress=networkIDManager->sharedNetworkID++;
		if (NetworkID::peerToPeerMode)
		{
			 // If this assert hits you forgot to call SetExternalSystemAddress
			RakAssert(networkIDManager->externalSystemAddress!=UNASSIGNED_SYSTEM_ADDRESS);
			networkID.systemAddress=networkIDManager->externalSystemAddress;
		}
		collision = networkIDManager->IDTree.GetPointerToNode( NetworkIDNode( ( networkID ), 0 ) );
	}
	while ( collision );

	networkIDManager->IDTree.Add( NetworkIDNode( networkID, this ) );
}

//////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////
