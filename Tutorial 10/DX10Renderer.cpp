#include "DX10Renderer.h"

DX10Renderer::DX10Renderer() :	pD3DDevice(0), 
								pSwapChain(0), 
								pRenderTargetView(0), 
								pDepthStencil(0), 
								pDepthStencilView(0),
								pShadowMap(0),
								pShadowMapDepthView(0),
								pShadowMapSRView(0),
								pEffect(0),
								pRenderShadowMapTechnique(0),
								pRenderTechnique(0),								
								pBillboardTechnique(0),
								shadowMapBias(0)
{
}

DX10Renderer::~DX10Renderer()
{
	Shutdown();
}

bool DX10Renderer::Initialize(const HWND *pHWND)
{
	//get window dimensions
	RECT rc;
    GetClientRect( *pHWND, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;	
	
	/*******************************************************************
	* Create Swap Chain and D3D device
	********************************************************************/
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	
	//set buffer dimensions and format
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.OutputWindow = *pHWND;
	swapChainDesc.Windowed = true;    

	
	//Create the D3D device
	//--------------------------------------------------------------
	// put the d3d device into debug mode under debug profile
	//--------------------------------------------------------------
	UINT flags = 0;
	#if defined(_DEBUG) | defined(DEBUG) 
	flags |= D3D10_CREATE_DEVICE_DEBUG; 
	#endif

	if ( FAILED( D3D10CreateDeviceAndSwapChain(		NULL, 
													D3D10_DRIVER_TYPE_HARDWARE, 
													NULL, 
													flags, 
													D3D10_SDK_VERSION, 
													&swapChainDesc, 
													&pSwapChain, 
													&pD3DDevice ) ) ) return false;
	
	/*******************************************************************
	* Create Rendering Targets and Depth Stencil
	********************************************************************/
	//try to get the back buffer
	ID3D10Texture2D* pBackBuffer;	
	if ( FAILED( pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*) &pBackBuffer) ) ) return false;

	//try to create render target view
	if ( FAILED( pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView) ) ) return false;
	pBackBuffer->Release();
	
	//create depth stencil texture desc
	D3D10_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_D32_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D10_USAGE_DEFAULT;
	texDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;   
    
	// Create the depth stencil view desc
	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = texDesc.Format;
	descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;		
	
	if( FAILED( pD3DDevice->CreateTexture2D( &texDesc, NULL, &pDepthStencil ) ) )  return false;    
	if( FAILED( pD3DDevice->CreateDepthStencilView( pDepthStencil, &descDSV, &pDepthStencilView ) ) ) return false;
	
	/*******************************************************************
	* Create Shadow Map Texture
	********************************************************************/

	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;	
	texDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE;	
  
	D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	if( FAILED( pD3DDevice->CreateTexture2D( &texDesc, NULL, &pShadowMap ) ) )  return false;
	if( FAILED( pD3DDevice->CreateDepthStencilView( pShadowMap, &descDSV, &pShadowMapDepthView ) ) ) return false;
	if( FAILED( pD3DDevice->CreateShaderResourceView( pShadowMap, &srvDesc, &pShadowMapSRView) ) ) return false;
			
	/*******************************************************************
	* Set up Viewports
	********************************************************************/	
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
		
	shadowMapViewport.Width = width;
	shadowMapViewport.Height = height;
	shadowMapViewport.MinDepth = 0.0f;
	shadowMapViewport.MaxDepth = 1.0f;
	shadowMapViewport.TopLeftX = 0;
	shadowMapViewport.TopLeftY = 0;	

	return true;
}

bool DX10Renderer::Shutdown()
{
	//release resources
	if ( pDepthStencil ) pDepthStencil->Release();
	if ( pDepthStencilView ) pDepthStencilView->Release();
	
	if ( pSwapChain ) pSwapChain->Release();	
	if ( pRenderTargetView ) pRenderTargetView->Release();

	if ( pEffect ) pEffect->Release();
	if ( pShadowMap ) pShadowMap->Release();
	if ( pShadowMapDepthView ) pShadowMapDepthView->Release();
	if ( pShadowMapSRView ) pShadowMapSRView->Release();

	for (unsigned int i=0; i < meshes.size(); i++)
	{
		meshes[i].Release();
	}
	meshes.clear();

	if ( pD3DDevice ) pD3DDevice->Release();

	//reset pointers
	pD3DDevice = 0;
	pSwapChain = 0; 
	pRenderTargetView = 0;
	pDepthStencil = 0;
	pDepthStencilView = 0;
	pShadowMap = 0;
	pShadowMapDepthView = 0;
	pShadowMapSRView = 0;
	pEffect = 0;
	pRenderTechnique = 0;
	pRenderShadowMapTechnique = 0;

	return true;
}

