#ifndef _NMD_HRTIMER
#define _NMD_HRTIMER

#include <windows.h>

class HRTimer
{
private:	

	LARGE_INTEGER startCount, stopCount, countsPerSecond, timeElapsed;

public:

	//constructor
	HRTimer()
	{
		QueryPerformanceFrequency( &countsPerSecond );
		Reset();
	}

	//reset timer
	void Reset() { QueryPerformanceCounter( &startCount ); }
	void Start() { Reset(); }

	//return elapsed times
	inline double GetElapsedTimeSeconds()
	{					
		QueryPerformanceCounter( &stopCount );	
		timeElapsed.QuadPart = stopCount.QuadPart - startCount.QuadPart;			
		return (double) timeElapsed.QuadPart / countsPerSecond.QuadPart ;		
	}
	inline double GetElapsedTimeMilliseconds()
	{		
		QueryPerformanceCounter( &stopCount );	
		timeElapsed.QuadPart = stopCount.QuadPart - startCount.QuadPart;			
		return (double) timeElapsed.QuadPart / countsPerSecond.QuadPart * 1000 ;		
	}
};	

#endif