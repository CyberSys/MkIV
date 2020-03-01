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

#include "GetTime.h"
#ifdef _COMPATIBILITY_1
#include "Compatibility1Includes.h" // Developers of a certain platform will know what to do here.
#elif defined(_WIN32)
#include <windows.h>
#elif defined(_COMPATIBILITY_2)
#include "Compatibility2Includes.h"
#include <sys/time.h>
#include <unistd.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif

static bool initialized=false;
#ifdef _WIN32
static LARGE_INTEGER yo;
#else
static timeval tp, initialTime;
#endif

RakNetTime RakNet::GetTime( void )
{
	return (RakNetTime)(GetTimeNS()/1000);
}
RakNetTimeNS RakNet::GetTimeNS( void )
{
	if ( initialized == false )
	{
#ifdef _WIN32
		QueryPerformanceFrequency( &yo );
		// The original code shifted right 10 bits
		//counts = yo.QuadPart >> 10;
		// It gives the wrong value since 2^10 is not 1000
		//	counts = yo.QuadPart;// / 1000;
#else
		gettimeofday( &initialTime, 0 );
#endif

		initialized = true;
	}

	static RakNetTimeNS lastVal=(RakNetTimeNS)0;
	RakNetTimeNS curTime;

#ifdef _WIN32
	LARGE_INTEGER PerfVal;

	QueryPerformanceCounter( &PerfVal );

	__int64 quotient, remainder;
	quotient=((PerfVal.QuadPart) / yo.QuadPart);
	remainder=((PerfVal.QuadPart) % yo.QuadPart);
	curTime = (RakNetTimeNS) quotient*(RakNetTimeNS)1000000 + (remainder*(RakNetTimeNS)1000000 / yo.QuadPart);

#else
	gettimeofday( &tp, 0 );

	curTime = ( tp.tv_sec - initialTime.tv_sec ) * (RakNetTimeNS) 1000000 + ( tp.tv_usec - initialTime.tv_usec );
#endif

	// To workaround http://support.microsoft.com/kb/274323 where the timer can sometimes jump forward by hours or days
	// Call it twice and hope that fixes it
	if (lastVal!=(RakNetTimeNS)0 && (curTime < lastVal || curTime-lastVal>(RakNetTimeNS)1000*(RakNetTimeNS)1000*(RakNetTimeNS)60*(RakNetTimeNS)1))
	{
#ifdef _WIN32
		LARGE_INTEGER PerfVal;

		QueryPerformanceCounter( &PerfVal );

		__int64 quotient, remainder;
		quotient=((PerfVal.QuadPart) / yo.QuadPart);
		remainder=((PerfVal.QuadPart) % yo.QuadPart);
		curTime = (RakNetTimeNS) quotient*(RakNetTimeNS)1000000 + (remainder*(RakNetTimeNS)1000000 / yo.QuadPart);

#else
		gettimeofday( &tp, 0 );

		curTime = ( tp.tv_sec - initialTime.tv_sec ) * (RakNetTimeNS) 1000000 + ( tp.tv_usec - initialTime.tv_usec );
#endif

	}

	lastVal=curTime;
	return curTime;
}
