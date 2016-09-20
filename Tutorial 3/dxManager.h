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

#include <vector>

//create a basic vertex type
struct vertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR4 color;
	D3DXVECTOR2 texCoord;

	vertex( D3DXVECTOR3 p, D3DXVECTOR4 c )
	{
		pos = p;
		color = c;
	}
	
	vertex( D3DXVECTOR3 p, D3DXVECTOR4 c, D3DXVECTOR2 tex )
	{
		pos = p;
		color = c;
		texCoord = tex;
	}
};

class dxManager
{
	/*******************************************************************
	* Members
	********************************************************************/	
private:

	//window handle shortcut
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

	//texture storage
	std::vector<ID3D10ShaderResourceView*> textureSRV;
	ID3D10EffectShaderResourceVariable* pTextureSR;
	int textureIndex;
	
	/*******************************************************************
	* Methods
	********************************************************************/	
public:

	//constructor and destructor
	dxManager();
	~dxManager();

	//initialize directx device
	bool initialize(HWND*);	

	//settings
	void enableTextures();
	void disableTextures();
	void swapTexture();

	//scene function
	void renderScene();	

private:

	//initialization methods
	bool createSwapChainAndDevice( UINT width, UINT height );
	bool loadShadersAndCreateInputLayouts();
	void createViewports( UINT width, UINT height );
	void initRasterizerState();
	bool createRenderTargets();

	//texture loader	
	bool loadTextures();

	//fatal error handler
	bool fatalError(const LPCSTR msg); 
};

#endif