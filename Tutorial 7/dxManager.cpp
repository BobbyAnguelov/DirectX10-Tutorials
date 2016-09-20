#include "dxManager.h"

/*******************************************************************
* Constructor
*******************************************************************/
dxManager::dxManager() :	pD3DDevice(0),
							pSwapChain(0),
							pRenderTargetView(0),
							pVertexBuffer(0),
							pBasicEffect(0),
							pVertexLayout(0),
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
	
	//free used resources
	if (pVertexBuffer) pVertexBuffer->Release();
	if (pBasicEffect) pBasicEffect->Release();
	if (pVertexLayout) pVertexLayout->Release();
	if (pRS) pRS->Release();
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

	//SHADER STAGE
	//*****************************************************************************
		
	//Load Basic Effect and Technique 
	//--------------------------------------------------------------
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

	pBasicTechnique1 = pBasicEffect->GetTechniqueByName("RenderMethod1");
	pBasicTechnique2 = pBasicEffect->GetTechniqueByName("RenderMethod2");
	

	//create matrix effect pointers
	pViewMatrixEffectVariable = pBasicEffect->GetVariableByName( "View" )->AsMatrix();
	pProjectionMatrixEffectVariable = pBasicEffect->GetVariableByName( "Projection" )->AsMatrix();
	pWorldMatrixEffectVariable = pBasicEffect->GetVariableByName( "World" )->AsMatrix();
	pActiveViewport = pBasicEffect->GetVariableByName("activeViewport")->AsScalar();

	//INPUT ASSEMBLY STAGE
	//*****************************************************************************
	
	//Create Input Layout
	//--------------------------------------------------------------	
	
	D3D10_INPUT_ELEMENT_DESC layout[] = 
	{	
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = 2;

	D3D10_PASS_DESC PassDesc;
	pBasicTechnique1->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	if ( FAILED( pD3DDevice->CreateInputLayout( layout, 
												numElements, 
												PassDesc.pIAInputSignature,
												PassDesc.IAInputSignatureSize, 
												&pVertexLayout ) ) ) return fatalError("Could not create Input Layout!");

	// Set the input layout
	pD3DDevice->IASetInputLayout( pVertexLayout );
	
	//RASTERIZER STAGE SETUP
	//*****************************************************************************
	
	//Create viewport
	//--------------------------------------------------------------
	
	//define viewports	
	viewports[0].Width = width/2;
	viewports[0].Height = height;
	viewports[0].MinDepth = 0.0f;
	viewports[0].MaxDepth = 1.0f;
	viewports[0].TopLeftX = 0;
	viewports[0].TopLeftY = 0;
	
	viewports[1].Width = width/2;
	viewports[1].Height = height;
	viewports[1].MinDepth = 0.0f;
	viewports[1].MaxDepth = 1.0f;
	viewports[1].TopLeftX = width/2 - 5;
	viewports[1].TopLeftY = 0;	

	//set rasterizer state
	//--------------------------------------------------------------
	
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

	// OUTPUT-MERGER STAGE
	//*****************************************************************************

	//Create render target view
	//--------------------------------------------------------------
	
	//try to get the back buffer
	ID3D10Texture2D* pBackBuffer;	
	if ( FAILED( pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*) &pBackBuffer) ) ) return fatalError("Could not get back buffer");

	//try to create render target view
	if ( FAILED( pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView) ) ) return fatalError("Could not create render target view");
	
	pBackBuffer->Release();
	pD3DDevice->OMSetRenderTargets(1, &pRenderTargetView, NULL);

	// Set up the view and projection matrices
	//*****************************************************************************
	D3DXVECTOR3 camera[3] = { D3DXVECTOR3(0.0f, 0.0f, -5.0f), D3DXVECTOR3(0.0f, 0.0f, 1.0f), D3DXVECTOR3(0.0f, 1.0f, 0.0f) };
	D3DXMatrixLookAtLH(&viewMatrix, &camera[0], &camera[1], &camera[2]);	
    D3DXMatrixPerspectiveFovLH(&projectionMatrix, (float)D3DX_PI * 0.5f, (float)width/(float)height, 0.1f, 100.0f);

	// VERTEX BUFFER INITIALIZATION
	//*****************************************************************************

	//create vertex buffer (space for 3 vertices)
	//---------------------------------------------------------------------------------
	
	vertex v[3] = { vertex( D3DXVECTOR3(-1,-1,0), D3DXVECTOR4(1,0,0,1) ),
					vertex( D3DXVECTOR3(0,1,0), D3DXVECTOR4(0,1,0,1) ),
					vertex( D3DXVECTOR3(1,-1,0), D3DXVECTOR4(0,0,1,1) ) };
	
	D3D10_BUFFER_DESC bd;
	bd.Usage = D3D10_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof( vertex ) * 3;
	bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
		
	D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = v;

	if ( FAILED( pD3DDevice->CreateBuffer( &bd, &InitData, &pVertexBuffer ) ) ) return fatalError("Could not create vertex buffer!");;

	// Set vertex buffer
	UINT stride = sizeof( vertex );
	UINT offset = 0;
	pD3DDevice->IASetVertexBuffers( 0, 1, &pVertexBuffer, &stride, &offset );	
		
	//everything completed successfully
	return true;
}

/*******************************************************************
* Scene Renderer
*******************************************************************/
void dxManager::renderScene()
{
	//clear scene
	pD3DDevice->ClearRenderTargetView( pRenderTargetView, D3DXCOLOR(0,0,0,0) );

	//create world matrix
	static float r;
	D3DXMATRIX w;
	D3DXMatrixIdentity(&w);

	//set effect matrices	
	pViewMatrixEffectVariable->SetMatrix(viewMatrix);
	pProjectionMatrixEffectVariable->SetMatrix(projectionMatrix);
	pWorldMatrixEffectVariable->SetMatrix(w);
	
	// Set primitive topology 
	pD3DDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

	//get technique desc
	D3D10_TECHNIQUE_DESC techDesc;		
		
	//SET VIEWPORT - METHOD 1 - Simple State Change
	//=============================================================================
	/*pBasicTechnique1->GetDesc( &techDesc );
	pD3DDevice->RSSetViewports(1, &viewports[0]);

	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		//apply technique
		pBasicTechnique1->GetPassByIndex( p )->Apply( 0 );
					
		//draw
		pD3DDevice->Draw( 3, 0 );
	}

	pD3DDevice->RSSetViewports(1, &viewports[1]);

	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		//apply technique
		pBasicTechnique1->GetPassByIndex( p )->Apply( 0 );
					
		//draw
		pD3DDevice->Draw( 3, 0 );
	}*/

	//SET VIEWPORT - METHOD 2 - Using a GS
	//==============================================================================
	pD3DDevice->RSSetViewports(2, viewports);
	pBasicTechnique2->GetDesc( &techDesc );
	pActiveViewport->SetInt(0);

	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		//apply technique
		pBasicTechnique2->GetPassByIndex( p )->Apply( 0 );
					
		//draw
		pD3DDevice->Draw( 3, 0 );
	}
	
	pActiveViewport->SetInt(1);

	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		//apply technique
		pBasicTechnique2->GetPassByIndex( p )->Apply( 0 );
					
		//draw
		pD3DDevice->Draw( 3, 0 );
	}

	//flip buffers
	pSwapChain->Present(0,0);
}

/*******************************************************************
* Fatal Error Handler
*******************************************************************/
bool dxManager::fatalError(LPCSTR msg)
{
	MessageBox(*hWnd, msg, "Fatal Error!", MB_ICONERROR);
	return false;
}