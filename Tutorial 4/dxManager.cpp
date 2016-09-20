#include "dxManager.h"
#include <string>
using namespace std;

/*******************************************************************
* Constructor
*******************************************************************/
dxManager::dxManager() :	pD3DDevice(NULL),
							pSwapChain(NULL),
							pRenderTargetView(NULL),
							pVertexBuffer(0),
							pVertexLayout(0),
							pBasicEffect(0),
							pRS(0),
							pIndexBuffer(0),
							pDepthStencil(0)
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
	if ( pVertexBuffer ) pVertexBuffer->Release();
	if ( pVertexLayout ) pVertexLayout->Release();
	if ( pRS ) pRS->Release();
	if ( pBasicEffect ) pBasicEffect->Release();
	if ( pIndexBuffer ) pIndexBuffer->Release();
	if ( pDepthStencil ) pDepthStencil->Release();
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
	
	// SET UP SCENE VARIABLES
	//*****************************************************************************

	//create vertex and index buffers (space for 100 entries)
	//---------------------------------------------------------------------------------
	UINT numVertices = 100;

	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof( vertex ) * numVertices;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;

	if ( FAILED( pD3DDevice->CreateBuffer( &bd, NULL, &pVertexBuffer ) ) ) return fatalError("Could not create vertex buffer!");
	
	//change buffer desc bytewidth to index type	
	bd.ByteWidth = sizeof( unsigned int ) * numVertices;	
	
	if ( FAILED( pD3DDevice->CreateBuffer( &bd, NULL, &pIndexBuffer ) ) ) return fatalError("Could not create index buffer!");

	// Set vertex and index buffers
	UINT stride = sizeof( vertex );
	UINT offset = 0;
	pD3DDevice->IASetVertexBuffers( 0, 1, &pVertexBuffer, &stride, &offset );
	pD3DDevice->IASetIndexBuffer( pIndexBuffer, DXGI_FORMAT_R32_UINT, offset );

	// Set up the view and projection matrices
	//*****************************************************************************
	const D3DXVECTOR3 camera[] = {	D3DXVECTOR3(0.0f, 0.0f, -5.0f),
									D3DXVECTOR3(0.0f, 0.0f, 1.0f),
									D3DXVECTOR3(0.0f, 1.0f, 0.0f)	};
	
	D3DXMatrixLookAtLH(&viewMatrix, &camera[0], &camera[1], &camera[2] );		
    D3DXMatrixPerspectiveFovLH(&projectionMatrix, (float)D3DX_PI * 0.5f, (float)width/(float)height, 0.1f, 100.0f);
	
	//set shader matrices
	pViewMatrixEffectVariable->SetMatrix(viewMatrix);
	pProjectionMatrixEffectVariable->SetMatrix(projectionMatrix);

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

	//define input layout
	D3D10_INPUT_ELEMENT_DESC layout[] = 
	{	
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 }		
	};

	//create input layout
	D3D10_PASS_DESC PassDesc;
	pBasicTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	if ( FAILED( pD3DDevice->CreateInputLayout( layout, 
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
* Scene Renderer
*******************************************************************/
void dxManager::renderScene()
{
	//clear scene
	pD3DDevice->ClearRenderTargetView( pRenderTargetView, D3DXCOLOR(0,0,0,0) );
	pD3DDevice->ClearDepthStencilView( pDepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0 );
	
	// Set primitive topology 
	pD3DDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );
	
	//fill vertex buffer with vertices
	UINT numVertices = 24;	
	vertex* v = NULL;	

	//rotate object - rotation should be timer based but i'm lazy
	static float r = 0;
	D3DXMatrixRotationY( &worldMatrix, r);
	pWorldMatrixEffectVariable->SetMatrix(worldMatrix);
	r += 0.0001f;
	
	//CUBE DRAW METHOD 1
	//VERTEX VALUES NOT 100% CORRECT - TOO LAZY TO CORRECT
	//------------------------------------------------------------
/*
	//lock vertex buffer for CPU use
	pVertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**) &v );
	
	//vertices for a cube
	v[0] = vertex( D3DXVECTOR3(-1,-1,-1), D3DXVECTOR4(1,0,0,1) );
	v[1] = vertex( D3DXVECTOR3(-1,1,-1), D3DXVECTOR4(0,1,0,1) );
	v[2] = vertex( D3DXVECTOR3(1,-1,-1), D3DXVECTOR4(0,0,1,1) );
	v[3] = vertex( D3DXVECTOR3(1,1,-1), D3DXVECTOR4(1,1,0,1) );	

	v[4] = vertex( D3DXVECTOR3(1,-1,-1), D3DXVECTOR4(1,0,0,1) );
	v[5] = vertex( D3DXVECTOR3(1,1,-1), D3DXVECTOR4(0,1,0,1) );
	v[6] = vertex( D3DXVECTOR3(1,-1,1), D3DXVECTOR4(0,0,1,1) );
	v[7] = vertex( D3DXVECTOR3(1,1,1), D3DXVECTOR4(1,1,0,1) );

	v[8] = vertex( D3DXVECTOR3(-1,-1,1), D3DXVECTOR4(1,0,0,1) );
	v[9] = vertex( D3DXVECTOR3(-1,1,1), D3DXVECTOR4(0,1,0,1) );
	v[10] = vertex( D3DXVECTOR3(-1,-1,-1), D3DXVECTOR4(0,0,1,1) );
	v[11] = vertex( D3DXVECTOR3(-1,1,-1), D3DXVECTOR4(1,1,0,1) );

	v[12] = vertex( D3DXVECTOR3(-1,-1,1), D3DXVECTOR4(1,0,0,1) );
	v[13] = vertex( D3DXVECTOR3(-1,1,1), D3DXVECTOR4(0,1,0,1) );
	v[14] = vertex( D3DXVECTOR3(1,-1,1), D3DXVECTOR4(0,0,1,1) );
	v[15] = vertex( D3DXVECTOR3(1,1,1), D3DXVECTOR4(1,1,0,1) );

	v[16] = vertex( D3DXVECTOR3(-1,-1,1), D3DXVECTOR4(1,0,0,1) );
	v[17] = vertex( D3DXVECTOR3(-1,-1,-1), D3DXVECTOR4(0,1,0,1) );
	v[18] = vertex( D3DXVECTOR3(1,-1,1), D3DXVECTOR4(0,0,1,1) );
	v[19] = vertex( D3DXVECTOR3(1,-1,-1), D3DXVECTOR4(1,1,0,1) );

	v[20] = vertex( D3DXVECTOR3(-1,1,-1), D3DXVECTOR4(1,0,0,1) );
	v[21] = vertex( D3DXVECTOR3(-1,1,1), D3DXVECTOR4(0,1,0,1) );
	v[22] = vertex( D3DXVECTOR3(1,1,-1), D3DXVECTOR4(0,0,1,1) );
	v[23] = vertex( D3DXVECTOR3(1,1,1), D3DXVECTOR4(1,1,0,1) );

	pVertexBuffer->Unmap();	
	
	//send vertices down pipeline
	for( UINT p = 0; p < techDesc.Passes; p++ )
	{
		//apply technique
		pBasicTechnique->GetPassByIndex( p )->Apply( 0 );
				
		//draw 5 quads
		pD3DDevice->Draw( 4, 0 );
		pD3DDevice->Draw( 4, 4 );
		pD3DDevice->Draw( 4, 8 );
		pD3DDevice->Draw( 4, 12 );
		pD3DDevice->Draw( 4, 16 );
		pD3DDevice->Draw( 4, 20 );
	}*/

	//CUBE DRAW METHOD 2
	//------------------------------------------------------------

	//lock vertex buffer for CPU use
	pVertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**) &v );

	//vertices for a cube
	v[0] = vertex( D3DXVECTOR3(-1,1,-1), D3DXVECTOR4(1,0,0,1) );	//front top left
	v[1] = vertex( D3DXVECTOR3(1,1,-1), D3DXVECTOR4(0,1,0,1) );		//front top right
	v[2] = vertex( D3DXVECTOR3(-1,-1,-1), D3DXVECTOR4(0,0,1,1) );	//front bottom left
	v[3] = vertex( D3DXVECTOR3(1,-1,-1), D3DXVECTOR4(1,1,0,1) );	//front bottom right

	v[4] = vertex( D3DXVECTOR3(-1,1,1), D3DXVECTOR4(1,0,0,1) );		//back top left
	v[5] = vertex( D3DXVECTOR3(1,1,1), D3DXVECTOR4(0,1,0,1) );		//back top right
	v[6] = vertex( D3DXVECTOR3(-1,-1,1), D3DXVECTOR4(0,0,1,1) );	//back bottom left
	v[7] = vertex( D3DXVECTOR3(1,-1,1), D3DXVECTOR4(1,1,0,1) );		//back bottom right

	pVertexBuffer->Unmap();	

	//create indexes for a cube 
	unsigned int* i = NULL;	

	pIndexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**) &i );

	//front face
	i[0] = 2;
	i[1] = 0;
	i[2] = 3;
	i[3] = 1;

	i[4] = 0xffffffff;	//start new strip

	//right face
	i[5] = 3;
	i[6] = 1;
	i[7] = 7;
	i[8] = 5;

	i[9] = 0xffffffff;

	//left face
	i[10] = 6;
	i[11] = 4;
	i[12] = 2;
	i[13] = 0;

	i[14] = 0xffffffff;

	//back face
	i[15] = 7;
	i[16] = 5;
	i[17] = 6;
	i[18] = 4;

	i[19] = 0xffffffff;

	//top face
	i[20] = 0;
	i[21] = 4;
	i[22] = 1;
	i[23] = 5;

	i[24] = 0xffffffff;

	//bottom face
	i[25] = 6;
	i[26] = 2;
	i[27] = 7;
	i[28] = 3;

	pIndexBuffer->Unmap();	

	//send vertices down pipeline
	for( UINT p = 0; p < techDesc.Passes; p++ )
	{
		//apply technique
		pBasicTechnique->GetPassByIndex( p )->Apply( 0 );
				
		//draw 5 quads - 29 indexes = 4 indexes x 6 faces + 5 breaks
		pD3DDevice->DrawIndexed( 29, 0, 0 );		
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