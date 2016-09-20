#ifndef _XCAMERA
#define _XCAMERA

#include <d3dx10.h>
#include "hrTimer.h"

#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.01745329251994329576923690768489

class Camera
{	
	/*******************************************************************
	* Members
	********************************************************************/	
private:
		
	//view parameters
	float heading, pitch;					//in radians

	//matrices
	D3DXMATRIX viewMatrix;
	D3DXMATRIX projectionMatrix;	
	D3DXMATRIX rotationMatrix;
		
	//view vectors
	const D3DXVECTOR3 dV, dU;				//default view and up vectors
	D3DXVECTOR3 eye, view, up;

	//movement vectors and movement toggles
	D3DXVECTOR3 forward, strafeRight;		
	int movementToggles[4];					//fwrd, bck, strfLft, strfRght
	float movementSpeed;

	//camera timer for movement
	timer camTimer;	
		
	/*******************************************************************
	* Methods
	********************************************************************/	
public:

	//constructor and destructor
	Camera();
	virtual ~Camera();

	//set projection methods
	void setPerspectiveProjectionLH( float FOV, float aspectRatio, float zNear, float zFar );
			
	//camera positioning methods
	void setPositionAndView( float x, float y, float z, float hDeg, float pDeg );
	void adjustHeadingPitch( float hRad, float pRad );
	void setMovementToggle( int i, int v );	
	void setMovementSpeed( float s );	

	//update camera view/position
	void update();	

	//get methods	
	D3DXMATRIX&		getViewMatrix(){ return viewMatrix; }
	D3DXMATRIX&		getProjectionMatrix(){ return projectionMatrix; }
	D3DXVECTOR3&	getCameraPosition(){ return eye; }

private:

	//create view, forward, strafe vectors from heading/pitch
	void updateView();
};

#endif