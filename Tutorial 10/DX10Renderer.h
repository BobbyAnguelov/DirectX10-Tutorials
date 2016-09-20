#ifndef _NMD_RENDERER_DX10
#define _NMD_RENDERER_DX10

#include <D3D10.h>
#include <D3DX10.h>
#include <xnamath.h>
#include <vector>
#include "OBJLoader.h"
#include "DX10Resources.h"

struct RenderMessage
{
	int modelID;
	XMFLOAT4X4 world;
};

//DirectX 10 Renderer
class DX10Renderer
{
	//Members
	//********************************************************************

	//device and swapchain
	ID3D10Device*							pD3DDevice;
	IDXGISwapChain*							pSwapChain;
	ID3D10RenderTargetView*					pRenderTargetView;		

	//viewport
	D3D10_VIEWPORT							viewport;	

	//depth stencil
	ID3D10Texture2D*						pDepthStencil;
	ID3D10DepthStencilView*					pDepthStencilView;
		
	//effect
	ID3D10Effect*							pEffect;
	ID3D10EffectTechnique*					pRenderShadowMapTechnique;
	ID3D10EffectTechnique*					pRenderTechnique;
	ID3D10EffectTechnique*					pBillboardTechnique;

	//shadow map resources
	ID3D10Texture2D*						pShadowMap;
	ID3D10DepthStencilView*					pShadowMapDepthView;
	ID3D10ShaderResourceView*				pShadowMapSRView;
	D3D10_VIEWPORT							shadowMapViewport;
	float									shadowMapBias;
	
	//SCENE ELEMENTS
	
	//light
	XMFLOAT3								lightPos;
	XMFLOAT4X4								lightViewProjMatrix, lightProjMatrix;

	//vertex buffers
	std::vector<DX10Mesh>					meshes;
	ID3D10InputLayout*						pInputLayout;
	
	//scene storage
	std::vector<RenderMessage>				scene;

	//Methods
	//********************************************************************
public:
	DX10Renderer();
	~DX10Renderer();

	//initialize renderer
	bool Initialize(const HWND* pHWND);
	bool Shutdown();

	//create test scene
	bool InitializeScene();

	//toggle shadow mapping technique
	void SetTechnique( unsigned int i );
	void SetShadowMapTechnique( unsigned int i);
	void ToggleShadowMapSize();
	void SetShadowMapBias( float b );

	//render scene frame
	void RenderFrame();
};

#endif