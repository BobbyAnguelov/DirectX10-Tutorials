#ifndef _CAMERA
#define _CAMERA

//using XNA Math library
#include <Windows.h>
#include <Xnamath.h>

//include HighRes Timer
#include "HRTimer.h"

/*

WARNING - THIS CAMERA WAS HACKED TOGETHER REALLY QUICKLY AND THERE ARE A FAIR
NUMBER OF ISSUES WITH IT 

DONOT USE IT FOR YOUR OWN PROJECTS

*/

inline void SetBit(unsigned int& bitField, unsigned int bit)
{
	bitField |= 0x80000000 >> bit;
}

inline void ClearBit(unsigned int& bitField, unsigned int bit)
{
	bitField &= ~(0x80000000 >> bit);
}

inline void FlipBit(unsigned int& bitfield, unsigned int bit)
{
	if ( (bitfield & (0x80000000 >> bit)) == (0x80000000 >> bit) ) 
	{
		bitfield &= ~( 0x80000000 >> bit);
	}
	else bitfield |= 0x80000000 >> bit;;
}

inline bool IsBitSet(unsigned int& bitField, unsigned int bit)
{
	if ( ((bitField << bit) & 0x80000000) == 0x80000000 ) return true;
	else return false;
}

class Camera
{
	//Members
	//********************************************************************
protected:

	//flags bitfield
	unsigned int flags;
		
	//camera position and matrix data
	XMFLOAT3 position, viewVector, upVector;
	XMFLOAT4X4 viewMatrix, projectionMatrix, viewProjectionMatrix;
	
	//free look variables
	float heading, pitch, movementSpeed;
	XMFLOAT3 dV, dU;

	//camera timer
	HRTimer timer;

	//Methods
	//********************************************************************
public:
		
	Camera();

	//positioning and movement methods	
	void PositionCamera( const float camPosition[3], const float focusPoint[3], const float upDirection[3] );
	void HandleInputMessage( unsigned int msg );
	void AdjustOrientation(const float hRad, const float vRad);
	void SetMovementSpeed( const float s );
	
	//virtual update method - controls camera movement
	void Update();
				
	//projections
	void SetPerspectiveProjectionLH( float FOVY, float aspectRatio, float zNear, float zFar );
		
	//get 
	const XMFLOAT3& GetCameraPosition();
	const XMFLOAT3& GetCameraUpVector();
	const XMFLOAT3& GetCameraViewVector();
	const XMFLOAT4X4& GetViewMatrix();
	const XMFLOAT4X4& GetProjectionMatrix();
	const XMFLOAT4X4& GetViewProjectionMatrix();
};


#endif};