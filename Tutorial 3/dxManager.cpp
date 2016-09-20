#include "dxManager.h"
#include <string>
using namespace std;

/*******************************************************************
* Constructor
*******************************************************************/
dxManager::dxManager() :	pD3DDevice(NULL),
							pSwapChain(NULL),
							pRenderTargetView(NULL), 
							textureIndex(0),
							pVertexBuffer(0),
							pVertexLayout(0),
							pBasicEffect(0),
							pRS(0)
{

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
	
	//free textures
	for ( unsigned int i=0; i < textureSRV.size(); i++ )
	{
		textureSRV[i]->Release();
	}	
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
	if ( !createRenderTargets() ) return false;
	
	// LOAD TEXTURES
	//*****************************************************************************
	if ( !loadTextures() ) return false;

	// SET UP SCENE VARIABLES
	//*****************************************************************************

	//create vertex buffer (space for 100 vertices)
	//---------------------------------------------------------------------------------
	UINT numVertices = 100;

	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof( vertex ) * numVertices;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;

	if ( FAILED( pD3DDevice->CreateBuffer( &bd, NULL, &pVertexBuffer ) ) ) return fatalError("Could not create vertex buffer!");;

	// Set vertex buffer
	UINT stride = sizeof( vertex );
	UINT offset = 0;
	pD3DDevice->IASetVertexBuffers( 0, 1, &pVertexBuffer, &stride, &offset );
	
	// Set up the view and projection matrices
	//*****************************************************************************
	D3DXVECTOR3 camera[3] = {	D3DXVECTOR3(0.0f, 0.0f, -5.0f),
								D3DXVECTOR3(0.0f, 0.0f, 1.0f),
								D3DXVECTOR3(0.0f, 1.0f, 0.0f) };
	D3DXMatrixLookAtLH(&viewMatrix, &camera[0], &camera[1], &camera[2]);		
	
    D3DXMatrixPerspectiveFovLH(&projectionMatrix, (float)D3DX_PI * 0.5f, (float)width/(float)height, 0.1f, 100.0f);

	D3DXMATRIX w;
	D3DXMatrixIdentity(&w);		
	pWorldMatrixEffectVariable->SetMatrix(w);
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
bool dxManager::createRenderTargets()
{
	//try to get the back buffer
	ID3D10Texture2D* pBackBuffer;	
	if ( FAILED( pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*) &pBackBuffer) ) ) return fatalError("Could not get back buffer");

	//try to create render target view
	if ( FAILED( pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView) ) ) return fatalError("Could not create render target view");
	
	pBackBuffer->Release();
	pD3DDevice->OMSetRenderTargets(1, &pRenderTargetView, NULL);

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

	pBasicTechnique = pBasicEffect->GetTechniqueByName("full");
	if ( pBasicTechnique == NULL ) return fatalError("Could not find specified technique!");	
	

	//create matrix effect pointers
	pViewMatrixEffectVariable = pBasicEffect->GetVariableByName( "View" )->AsMatrix();
	pProjectionMatrixEffectVariable = pBasicEffect->GetVariableByName( "Projection" )->AsMatrix();
	pWorldMatrixEffectVariable = pBasicEffect->GetVariableByName( "World" )->AsMatrix();
	pTextureSR = pBasicEffect->GetVariableByName( "tex2D" )->AsShaderResource();

	//define input layout
	D3D10_INPUT_ELEMENT_DESC layout[] = 
	{	
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28, D3D10_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = sizeof( layout ) / sizeof( layout[0] );

	//create input layout
	D3D10_PASS_DESC PassDesc;
	pBasicTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	if ( FAILED( pD3DDevice->CreateInputLayout( layout, 
												numElements, 
												PassDesc.pIAInputSignature,
												PassDesc.IAInputSignatureSize, 
												&pVertexLayout ) ) ) return fatalError("Could not create Input Layout!");

	// Set the input layout
	pD3DDevice->IASetInputLayout( pVertexLayout );

	return true;
}
/*******************************************************************
* Texture Loader
*******************************************************************/
bool dxManager::loadTextures()
{
	vector<string> filenames;
	filenames.push_back("textures/t1.bmp");
	filenames.push_back("textures/t2.bmp");
	filenames.push_back("textures/t3.bmp");
	
	//load textures
	for ( int i=0; i < (int) filenames.size(); i++ )
	{
		textureSRV.push_back(NULL);	
		if ( FAILED( D3DX10CreateShaderResourceViewFromFile( pD3DDevice, filenames[i].c_str(), NULL, NULL, &textureSRV[i], NULL ) ) ) 
		{
			char err[255];
			sprintf_s(err, "Could not load texture: %s!", filenames[i].c_str());
			return fatalError( err );
		}
	}

	return true;
}
/*******************************************************************
* Enable/Disable Texturing
*******************************************************************/
void dxManager::enableTextures()
{
	pBasicTechnique = pBasicEffect->GetTechniqueByName("full");
}

void dxManager::disableTextures()
{
	pBasicTechnique = pBasicEffect->GetTechniqueByName("texturingDisabled");
}
/*******************************************************************
* Enable/Disable Texturing
*******************************************************************/
void dxManager::swapTexture()
{
	textureIndex++;

	if ( textureIndex > (int) textureSRV.size() - 1 ) textureIndex = 0;
}
/*******************************************************************
* Scene Renderer
*******************************************************************/
void dxManager::renderScene()
{
	//clear scene
	pD3DDevice->ClearRenderTargetView( pRenderTargetView, D3DXCOLOR(0,0,0,0) );
	
	//fill vertex buffer with vertices
	UINT numVertices = 4;	
	vertex* v = NULL;	

	//lock vertex buffer for CPU use
	pVertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**) &v );
	
	v[0] = vertex( D3DXVECTOR3(-1,-1,0), D3DXVECTOR4(1,0,0,1), D3DXVECTOR2(0.0f, 2.0f) );
	v[1] = vertex( D3DXVECTOR3(-1,1,0), D3DXVECTOR4(0,1,0,1), D3DXVECTOR2(0.0f, 0.0f) );
	v[2] = vertex( D3DXVECTOR3(1,-1,0), D3DXVECTOR4(0,0,1,1), D3DXVECTOR2(2.0f, 2.0f) );
	v[3] = vertex( D3DXVECTOR3(1,1,0), D3DXVECTOR4(1,1,0,1), D3DXVECTOR2(2.0f, 0.0f) );	

	pVertexBuffer->Unmap();

	// Set primitive topology 
	pD3DDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	//set texture
	pTextureSR->SetResource( textureSRV[textureIndex] );

	//get technique desc
	D3D10_TECHNIQUE_DESC techDesc;
	pBasicTechnique->GetDesc( &techDesc );
	
	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		//apply technique
		pBasicTechnique->GetPassByIndex( p )->Apply( 0 );
				
		//draw
		pD3DDevice->Draw( numVertices, 0 );
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