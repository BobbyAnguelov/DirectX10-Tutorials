#include "Camera.h"

//constructor
Camera::Camera():	flags(0),
					position( XMFLOAT3(0,0,0) ),
					viewVector( XMFLOAT3(0,0,1) ),
					upVector( XMFLOAT3(0,1,0) ),
					heading(0), 
					pitch(0), 
					movementSpeed(0), 
					dV(XMFLOAT3(0,0,1)),
					dU(XMFLOAT3(0,1,0))
{
	//set all matrices to identity matrix
	XMMATRIX m = XMMatrixIdentity();
	XMStoreFloat4x4(&viewMatrix, m);
	XMStoreFloat4x4(&projectionMatrix, m);
	XMStoreFloat4x4(&viewProjectionMatrix, m);	

	//start camera timer
	timer.Start();
}

void Camera::PositionCamera(const float camPosition[3], const float focusPoint[3], const float upDirection[3])
{
	//create view matrix
	XMVECTOR ep = XMLoadFloat3( (XMFLOAT3*) &camPosition);
	XMVECTOR fp = XMLoadFloat3( (XMFLOAT3*) &focusPoint);
	XMVECTOR ud = XMLoadFloat3( (XMFLOAT3*) &upDirection);
	XMMATRIX vm = XMMatrixLookAtLH( ep, fp, ud);
	XMStoreFloat4x4( &viewMatrix, vm);

	//set cam vars
	fp = XMVector3Normalize(fp - ep);
	ud = XMVector3Normalize(ud);
	position = camPosition;
	XMStoreFloat3( &viewVector, fp);
	XMStoreFloat3( &upVector, ud);
}
//projection methods
void Camera::SetPerspectiveProjectionLH(float FOVY, float aspectRatio, float zNear, float zFar)
{
	//create projection matrix and store it in a XMFLOAT4X4
	XMMATRIX p2 = XMMatrixPerspectiveFovLH(FOVY, aspectRatio, zNear, zFar);
	XMStoreFloat4x4( &projectionMatrix, p2);	
}
//get methods
const XMFLOAT3& Camera::GetCameraPosition() 
{ 
	return position; 
}
const XMFLOAT3& Camera::GetCameraUpVector()
{	
	return upVector;
}
const XMFLOAT3& Camera::GetCameraViewVector()
{
	return viewVector;
}
const XMFLOAT4X4& Camera::GetViewMatrix() 
{ 
	return viewMatrix; 
}
const XMFLOAT4X4& Camera::GetProjectionMatrix() 
{ 
	return projectionMatrix; 
}
const XMFLOAT4X4& Camera::GetViewProjectionMatrix()
{
	return viewProjectionMatrix;
}

//handle input message
void Camera::HandleInputMessage( unsigned int msg )
{
	if ( msg == 0 ) SetBit(flags, 0);
	else if ( msg == 4 ) ClearBit(flags, 0);
	else if ( msg == 1 ) SetBit(flags, 1);
	else if ( msg == 5 ) ClearBit(flags, 1);
	else if ( msg == 2 ) SetBit(flags, 2);
	else if ( msg == 6 ) ClearBit(flags, 2);
	else if ( msg == 3 ) SetBit(flags, 3);
	else if ( msg == 7 ) ClearBit(flags, 3);
}

//update camera view
void Camera::Update()
{
	//get current elapsed time
	float t = (float) timer.GetElapsedTimeSeconds();
	
	//create rotation matrix
	XMMATRIX m = XMMatrixRotationRollPitchYaw( pitch, heading, 0 );

	//create new view, up and right vectors	
	XMVECTOR view = XMLoadFloat3(&dV);
	XMVECTOR up = XMLoadFloat3(&dU);
	view = XMVector3Normalize(XMVector3Transform(view, m));
	up = XMVector3Normalize(XMVector3Transform(up, m));	
	XMVECTOR r = XMVector3Normalize(XMVector3Cross(up, view));
		
	//update camera position
	XMVECTOR eye = XMLoadFloat3(&position);
	if( (flags & 0x80000000) == 0x80000000 ) eye += movementSpeed * t * view;	//forward
	if( (flags & 0x40000000) == 0x40000000 ) eye -= movementSpeed * t * view;	//back
	if( (flags & 0x20000000) == 0x20000000 ) eye -= movementSpeed * t * r;		//left
	if( (flags & 0x10000000) == 0x10000000 ) eye += movementSpeed * t * r;		//right
	
	//HACK
	/*XMFLOAT3	he(28.741705f,16.966558f,-4.5183172f) , 
				hv(-0.81913942f,-0.47062564f,0.32790580f), 
				hu(-0.43691891f,0.88233304f,0.17490092f);
	eye = XMLoadFloat3(&he);
	view = XMLoadFloat3(&hv);
	up = XMLoadFloat3(&hu);*/
	
	//create view matrix
	m = XMMatrixLookAtLH( eye, eye+view, up );
	XMStoreFloat4x4(&viewMatrix, m);
	
	//set cam vars
	XMStoreFloat3(&position, eye);
	XMStoreFloat3(&viewVector, view);
	XMStoreFloat3(&upVector, up);

	//update view projection matrix
	XMMATRIX p = XMLoadFloat4x4(&projectionMatrix);
	m = XMMatrixMultiply(m, p);
	XMStoreFloat4x4(&viewProjectionMatrix, m);
	
	//reset timer
	timer.Reset();
}
//adjust heading and pitch
void Camera::AdjustOrientation( const float hRad, const float pRad )
{
	heading += hRad;
	pitch += pRad;

	//value clamping - keep heading and pitch between 0 and 2 pi
	if ( heading > XM_2PI ) heading -= (float) XM_2PI;
	else if ( heading < 0 ) heading = (float) XM_2PI + heading;
	
	if ( pitch > XM_2PI ) pitch -= (float) XM_2PI;
	else if ( pitch < 0 ) pitch = (float) XM_2PI + pitch;
}
//set movement speed
void Camera::SetMovementSpeed( const float s)
{
	movementSpeed = s;
}