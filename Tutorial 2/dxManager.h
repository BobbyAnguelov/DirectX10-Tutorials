/************************************************************************************
 *	DirectX 10 Manager Class
 *	----------------------------------------
 *	code by : bobby anguelov - banguelov@cs.up.ac.za
 *	downloaded from : takinginitiative.wordpress.org
 *
 *	code is free for use in whatever way you want, however if you work for a game
 *	development firm you are obliged to offer me a job :P (haha i wish)
 ************************************************************************************/

#ifndef DXMANAGER
#define DXMANAGER

#include <windows.h>
#include <d3d10.h>
#include <d3dx10.h>

//create a basic vertex container
struct vertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR4 color;

	vertex( D3DXVECTOR3 p, D3DXVECTOR4 c )
	{
		pos = p;
		color = c;
	}
};

class dxManager
{
	/*******************************************************************
	* Members
	********************************************************************/	
private:

	//window handle
	HWND*						hWnd;
	
	//device vars
	ID3D10Device*				pD3DDevice;
	IDXGISwapChain*				pSwapChain;
	ID3D10RenderTargetView*		pRenderTargetView;
	D3D10_VIEWPORT				viewPort;
	ID3D10RasterizerState*		pRS;

	//input layout and vertex buffer
	ID3D10Buffer*				pVertexBuffer;
	ID3D10InputLayout*			pVertexLayout;

	//effects and techniques
	ID3D10Effect*				pBasicEffect;
	ID3D10EffectTechnique*		pBasicTechnique;

	//effect variable pointers
	ID3D10EffectMatrixVariable* pViewMatrixEffectVariable;
	ID3D10EffectMatrixVariable* pProjectionMatrixEffectVariable;
	ID3D10EffectMatrixVariable* pWorldMatrixEffectVariable;
	
	//projection and view matrices
	D3DXMATRIX                  viewMatrix;
	D3DXMATRIX                  projectionMatrix;

	/*******************************************************************
	* Methods
	********************************************************************/	
public:

	//constructor and destructor
	dxManager();
	~dxManager();

	//initialize directx device
	bool initialize(HWND*);	

	//renderScene
	void renderScene();	

private:

	//fatal error handler
	bool fatalError(LPCSTR msg); 
};

#endif