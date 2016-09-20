/************************************************************************************
 *	High Resolution Windows Timer
 *	------------------------------
 *	code by : bobby anguelov - banguelov@cs.up.ac.za
 *	downloaded from : takinginitiative.wordpress.org
 *
 *	code is free for use in whatever way you want, however if you work for a game
 *	development firm you are obliged to offer me a job :P (haha i wish)
 ************************************************************************************/
#ifndef _HRTIMER
#define _HRTIMER

#include <windows.h>

class timer
{
private:	

	LARGE_INTEGER strt, frequency;

public:

	//constructor
	timer()
	{
		QueryPerformanceFrequency( &frequency );
		reset();
	}

	//reset timer
	void reset() { QueryPerformanceCounter( &strt ); }
	void start() { reset(); }

	//return elapsed times
	double getElapsedTimeSeconds()
	{	
		LARGE_INTEGER stp, timeElapsed;
		
		QueryPerformanceCounter( &stp );	
		timeElapsed.QuadPart = stp.QuadPart - strt.QuadPart;			
		return (double) timeElapsed.QuadPart / frequency.QuadPart ;		
	}

	double getElapsedTimeMilliseconds()
	{	
		LARGE_INTEGER stp, timeElapsed;
		
		QueryPerformanceCounter( &stp );	
		timeElapsed.QuadPart = stp.QuadPart - strt.QuadPart;			
		return (double) timeElapsed.QuadPart / frequency.QuadPart * 1000 ;		
	}
};	

#endif
