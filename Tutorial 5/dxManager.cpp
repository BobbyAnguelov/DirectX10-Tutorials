#include "dxManager.h"
#include <string>
using namespace std;

/*******************************************************************
* Constructor
*******************************************************************/
dxManager::dxManager() :	pD3DDevice(NULL),
							pSwapChain(NULL),
							pRenderTargetView(NULL),							
							pVertexLayout(0),
							pBasicEffect(0),
							pRS(0),							
							pDepthStencil(0),
							pMesh(0)
{
	D3DXMatrixIdentity(&worldMatrix);
	D3DXMatrixIdentity(&projectionMatrix);
	D3DXMatrixIdentity(&viewMatrix);
}
/*******************************************************************
* Destructor
*******************************************************************/
dxManager::~dxManager()
{
	if ( pRenderTargetView ) pRenderTargetView->Release();
	if ( pSwapChain ) pSwapChain->Release();
	if ( pD3DDevice ) pD3DDevice->Release();	
	if ( pVertexLayout ) pVertexLayout->Release();
	if ( pRS ) pRS->Release();
	if ( pBasicEffect ) pBasicEffect->Release();	
	if ( pDepthStencil ) pDepthStencil->Release();

	//free mesh
	if (pMesh) pMesh->Release();
}
/*******************************************************************
* Initializes Direct3D Device
*******************************************************************/
bool dxManager::initialize( HWND* hW )
{
	//window handle
	hWnd = hW;
	
	//get window dimensions
	RECT rc;
    GetClientRect( *hWnd, &rc );
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

	// CREATE DEVICE
	//*****************************************************************************
	if ( !createSwapChainAndDevice(width, height) ) return false;
	
	// INPUT ASSEMBLY STAGE
	//*****************************************************************************
	if ( !loadShadersAndCreateInputLayouts() ) return false;			

	// RASTERIZER STAGE SETUP
	//*****************************************************************************
	createViewports(width, height);
	initRasterizerState();	

	// OUTPUT-MERGER STAGE
	//*****************************************************************************
	if ( !createRenderTargetsAndDepthBuffer(width, height) ) return false;
	
	// Set up the view and projection matrices
	//*****************************************************************************
	const D3DXVECTOR3 camera[] = {	D3DXVECTOR3(0.0f, 5.0f, -10.0f),
									D3DXVECTOR3(0.0f, 0.0f, 1.0f),
									D3DXVECTOR3(0.0f, 1.0f, 0.0f)	};
	
	D3DXMatrixLookAtLH(&viewMatrix, &camera[0], &camera[1], &camera[2] );		
    D3DXMatrixPerspectiveFovLH(&projectionMatrix, (float)D3DX_PI * 0.5f, (float)width/(float)height, 0.1f, 100.0f);
	
	//set shader matrices
	pViewMatrixEffectVariable->SetMatrix(viewMatrix);
	pProjectionMatrixEffectVariable->SetMatrix(projectionMatrix);

	// Initialize Scene Objects
	//*****************************************************************************

	if ( !initializeObjects() ) return false;

	//everything completed successfully
	return true;
}
/*******************************************************************
* Create Swap Chain and D3D device
*******************************************************************/
bool dxManager::createSwapChainAndDevice( UINT width, UINT height )
{
	//Set up DX swap chain
	//--------------------------------------------------------------
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	
	//set buffer dimensions and format
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;;
	
	//set refresh rate
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	
	//sampling settings
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SampleDesc.Count = 1;

	//output window handle
	swapChainDesc.OutputWindow = *hWnd;
	swapChainDesc.Windowed = true;    

	//Create the D3D device
	//--------------------------------------------------------------
	if ( FAILED( D3D10CreateDeviceAndSwapChain(		NULL, 
													D3D10_DRIVER_TYPE_HARDWARE, 
													NULL, 
													0, 
													D3D10_SDK_VERSION, 
													&swapChainDesc, 
													&pSwapChain, 
													&pD3DDevice ) ) ) return fatalError("D3D device creation failed");
	return true;
}
/*******************************************************************
* Set up Viewports
*******************************************************************/
void dxManager::createViewports( UINT width, UINT height )
{	
	//create viewport structure	
	viewPort.Width = width;
	viewPort.Height = height;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;

	pD3DDevice->RSSetViewports(1, &viewPort);
}
/*******************************************************************
* Initialize Rasterizer State
*******************************************************************/
void dxManager::initRasterizerState()
{
	//set rasterizer	
	D3D10_RASTERIZER_DESC rasterizerState;
	rasterizerState.CullMode = D3D10_CULL_NONE;
	rasterizerState.FillMode = D3D10_FILL_SOLID;
	rasterizerState.FrontCounterClockwise = true;
    rasterizerState.DepthBias = false;
    rasterizerState.DepthBiasClamp = 0;
    rasterizerState.SlopeScaledDepthBias = 0;
    rasterizerState.DepthClipEnable = true;
    rasterizerState.ScissorEnable = false;
    rasterizerState.MultisampleEnable = false;
    rasterizerState.AntialiasedLineEnable = true;

	ID3D10RasterizerState* pRS;
	pD3DDevice->CreateRasterizerState( &rasterizerState, &pRS);
	
	pD3DDevice->RSSetState(pRS);
}
/*******************************************************************
* Create Rendering Targets
*******************************************************************/
bool dxManager::createRenderTargetsAndDepthBuffer( UINT width, UINT height )
{
	//try to get the back buffer
	ID3D10Texture2D* pBackBuffer;	
	if ( FAILED( pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*) &pBackBuffer) ) ) return fatalError("Could not get back buffer");

	//try to create render target view
	if ( FAILED( pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView) ) ) return fatalError("Could not create render target view");
	
	pBackBuffer->Release();
	
	//create depth stencil texture
    D3D10_TEXTURE2D_DESC descDepth;
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D32_FLOAT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D10_USAGE_DEFAULT;
    descDepth.BindFlags = D3D10_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;   
    
	if( FAILED( pD3DDevice->CreateTexture2D( &descDepth, NULL, &pDepthStencil ) ) )  return fatalError("Could not create depth stencil texture");

    // Create the depth stencil view
    D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    
    if( FAILED( pD3DDevice->CreateDepthStencilView( pDepthStencil, &descDSV, &pDepthStencilView ) ) ) return fatalError("Could not create depth stencil view");

	//set render targets
	pD3DDevice->OMSetRenderTargets( 1, &pRenderTargetView, pDepthStencilView );

	return true;
}
/*******************************************************************
* Shader Loader
*******************************************************************/
bool dxManager::loadShadersAndCreateInputLayouts()
{
	if ( FAILED( D3DX10CreateEffectFromFile(	"basicEffect.fx", 
												NULL, NULL, 
												"fx_4_0", 
												D3D10_SHADER_ENABLE_STRICTNESS, 
												0, 
												pD3DDevice, 
												NULL, 
												NULL, 
												&pBasicEffect, 
												NULL, 
												NULL	) ) ) return fatalError("Could not load effect file!");	

	pBasicTechnique = pBasicEffect->GetTechniqueByName("render");
	if ( pBasicTechnique == NULL ) return fatalError("Could not find specified technique!");	
	

	//create matrix effect pointers
	pViewMatrixEffectVariable = pBasicEffect->GetVariableByName( "View" )->AsMatrix();
	pProjectionMatrixEffectVariable = pBasicEffect->GetVariableByName( "Projection" )->AsMatrix();
	pWorldMatrixEffectVariable = pBasicEffect->GetVariableByName( "World" )->AsMatrix();	

	//create input layout
	D3D10_PASS_DESC PassDesc;
	pBasicTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	if ( FAILED( pD3DDevice->CreateInputLayout( vertexInputLayout, 
												2, 
												PassDesc.pIAInputSignature,
												PassDesc.IAInputSignatureSize, 
												&pVertexLayout ) ) ) return fatalError("Could not create Input Layout!");

	// Set the input layout
	pD3DDevice->IASetInputLayout( pVertexLayout );
	
	//get technique description
	pBasicTechnique->GetDesc( &techDesc );

	return true;
}
/*******************************************************************
* Initialize Scene Objects
*******************************************************************/
bool dxManager::initializeObjects()
{
	//vertices for a cube
	vertex v[8];
	
	v[0] = vertex( D3DXVECTOR3(-1,1,-1), D3DXVECTOR4(1,0,0,1) );	//front top left
	v[1] = vertex( D3DXVECTOR3(1,1,-1), D3DXVECTOR4(0,1,0,1) );		//front top right
	v[2] = vertex( D3DXVECTOR3(-1,-1,-1), D3DXVECTOR4(0,0,1,1) );	//front bottom left
	v[3] = vertex( D3DXVECTOR3(1,-1,-1), D3DXVECTOR4(1,1,0,1) );	//front bottom right

	v[4] = vertex( D3DXVECTOR3(-1,1,1), D3DXVECTOR4(1,0,0,1) );		//back top left
	v[5] = vertex( D3DXVECTOR3(1,1,1), D3DXVECTOR4(0,1,0,1) );		//back top right
	v[6] = vertex( D3DXVECTOR3(-1,-1,1), D3DXVECTOR4(0,0,1,1) );	//back bottom left
	v[7] = vertex( D3DXVECTOR3(1,-1,1), D3DXVECTOR4(1,1,0,1) );		//back bottom right

	//create indexes for a cube 
	unsigned int i[36] = {	2,0,3,3,1,0,							
							3,1,7,7,5,1,							
							6,4,2,2,0,4,							
							7,5,6,6,4,5,							
							0,4,1,1,5,4,							
							6,2,7,7,3,2 };	
	//create mesh
	if ( FAILED( D3DX10CreateMesh( pD3DDevice, vertexInputLayout, 2, "POSITION", 8, 6, D3DX10_MESH_32_BIT, &pMesh) ) ) return fatalError("Could not create mesh!");

	//insert data into mesh and commit changes
	pMesh->SetVertexData(0, v);
	pMesh->SetIndexData(i, 36);
	pMesh->CommitToDevice();

	return true;
}
/*******************************************************************
* Scene Renderer
*******************************************************************/
void dxManager::renderScene()
{
	//clear scene
	pD3DDevice->ClearRenderTargetView( pRenderTargetView, D3DXCOLOR(0,0,0,0) );
	pD3DDevice->ClearDepthStencilView( pDepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0 );
	
	//rotate object - rotation should be timer based but i'm lazy
	D3DXMATRIX temp;
	static float r = 0;	
	r += 0.0001f;

	D3DXVECTOR3 rcOrigin(-14, 0, 0);

	//draw lots of cubes
	for ( int cols = 0; cols < 10; cols++ )
	{
		for ( int rows = 0; rows < 15; rows ++ )
		{
			//position cube
			D3DXMatrixRotationY( &worldMatrix, r);
			D3DXMatrixTranslation( &temp, rcOrigin.x + 4 * cols, 0, rcOrigin.z + 4 * rows);
			worldMatrix *= temp;
			pWorldMatrixEffectVariable->SetMatrix(worldMatrix);

			//draw cube
			for( UINT p = 0; p < techDesc.Passes; p++ )
			{
				//apply technique
				pBasicTechnique->GetPassByIndex( p )->Apply( 0 );
				pMesh->DrawSubset(0);
			}
		}
	}
	
	//flip buffers
	pSwapChain->Present(0,0);
}

/*******************************************************************
* Fatal Error Handler
*******************************************************************/
bool dxManager::fatalError(const LPCSTR msg)
{
	MessageBox(*hWnd, msg, "Fatal Error!", MB_ICONERROR);
	return false;
}