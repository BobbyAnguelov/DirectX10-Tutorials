#include "dxManager.h"
#include <math.h>
using namespace std;

#include "Camera.h"
extern Camera* pCamera;
extern unsigned char lightingTechnique;

/*******************************************************************
* Constructor
*******************************************************************/
dxManager::dxManager() :	pD3DDevice(NULL),
							pSwapChain(NULL),
							pRenderTargetView(NULL),
							pBasicEffect(NULL),
							pTerrainMesh(NULL)
{
	D3DXMatrixIdentity(&worldMatrix);
}
/*******************************************************************
* Destructor
*******************************************************************/
dxManager::~dxManager()
{
	if ( pRenderTargetView ) pRenderTargetView->Release();
	if ( pSwapChain ) pSwapChain->Release();
	if ( pD3DDevice ) pD3DDevice->Release();
	if ( pBasicEffect ) pBasicEffect->Release();
	if ( pDepthStencil ) pDepthStencil->Release();
	if ( pTerrainMesh ) pTerrainMesh->Release();
	if ( pTerrainTexture ) pTerrainTexture->Release();
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

	// OUTPUT-MERGER STAGE
	//*****************************************************************************
	if ( !createRenderTargetsAndDepthBuffer(width, height) ) return false;

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
	if ( FAILED( D3DX10CreateEffectFromFile(	"lighting.fx", 
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
	
	pTechnique = pBasicEffect->GetTechniqueByName("RENDER");	
	if ( pTechnique == NULL ) return fatalError("Could not find specified technique!");	
	
	//create matrix effect pointers
	pViewMatrixEffectVariable = pBasicEffect->GetVariableByName( "View" )->AsMatrix();
	pProjectionMatrixEffectVariable = pBasicEffect->GetVariableByName( "Projection" )->AsMatrix();
	pWorldMatrixEffectVariable = pBasicEffect->GetVariableByName( "World" )->AsMatrix();	

	//create texture effect variable
	pColorMap = pBasicEffect->GetVariableByName( "colorMap" )->AsShaderResource();

	//create input layout
	D3D10_PASS_DESC PassDesc;
	pTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	if ( FAILED( pD3DDevice->CreateInputLayout( vertexInputLayout, 
												vertexInputLayoutNumElements, 
												PassDesc.pIAInputSignature,
												PassDesc.IAInputSignatureSize, 
												&pVertexLayout ) ) ) return fatalError("Could not create Input Layout!");

	// Set the input layout
	pD3DDevice->IASetInputLayout( pVertexLayout );
	
	return true;
}
/*******************************************************************
* Initialize Scene Objects
*******************************************************************/
bool dxManager::initializeObjects()
{
	//read heightmap values
	FIBITMAP *pHeightMap = FreeImage_Load(FIF_BMP, "terrain_heightmap.bmp", BMP_DEFAULT);
	//FIBITMAP *pHeightMap = FreeImage_Load(FIF_BMP, "heightmap.bmp", BMP_DEFAULT);
	int hmHeight = 0, hmWidth = 0;
	float* pHeightData = 0;
		
	if ( pHeightMap )
	{
		//get height and width & allocate memory
		hmHeight = FreeImage_GetHeight(pHeightMap);
		hmWidth = FreeImage_GetWidth(pHeightMap);
		
		pHeightData = new float[hmHeight * hmWidth];

		RGBQUAD color;	
		for (int r=0; r < hmHeight; r++)
		{
			for(int c=0; c < hmWidth; c++)
			{
				FreeImage_GetPixelColor(pHeightMap,c,r,&color);
				
				//scale height data 
				float divisor = 255 / 6;
				pHeightData[r*hmWidth + c] = color.rgbRed / divisor - 3;
			}
		}

		FreeImage_Unload(pHeightMap);
	}

	//CREATE VERTICES
	//--------------------------------------------------------------------------------

	//create vertices for mesh
	unsigned int numVertices = hmHeight * hmWidth;
	vertex *pMeshVertices = new vertex[ numVertices ];
	float tC[2] = {0,1}, tCStepSize[2] = { 1.0f/hmWidth, 1.0f/hmHeight};

	//set positions
	float z = -32, x = -32;
	int cnt=0, wCnt = 0, hCnt = 0;	
	
	while (hCnt < hmHeight)
	{
		while (wCnt < hmWidth)
		{
			pMeshVertices[cnt].texCoords = D3DXVECTOR2( tC[0], tC[1] );			
			pMeshVertices[cnt++].pos = D3DXVECTOR3( x, pHeightData[cnt], z );
			tC[0] += tCStepSize[0];
			wCnt++;
			x += 64.0f/(hmWidth-1);
		}

		//set vars
		x = -32;
		z += 64.0f/(hmWidth-1);
		tC[0] = 0;
		tC[1] -= tCStepSize[1];
		wCnt = 0;
		hCnt++;
	}
	
	//CALCULATE NORMALS
	//--------------------------------------------------------------------------------

	//calculate normals	
	for ( unsigned int i = 0; i < numVertices - hmWidth; i++ )
	{
		D3DXVECTOR3 N, v1, v2;
		
		//calculate per primitive normal - face 1		
		v1 = pMeshVertices[i+hmWidth].pos - pMeshVertices[i].pos;
		v2 = pMeshVertices[i+1].pos - pMeshVertices[i].pos;

		D3DXVec3Cross( &N, &v1, &v2);

		//add primitive normal to all primitive's vertices
		pMeshVertices[i].normal += N;
		pMeshVertices[i+1].normal += N;
		pMeshVertices[i+hmWidth].normal += N;

		//calculate per primitive normal - face 2	
		v1 = pMeshVertices[i+hmWidth].pos - pMeshVertices[i+1].pos;
		v2 = pMeshVertices[i+hmWidth+1].pos - pMeshVertices[i+1].pos;
		
		D3DXVec3Cross( &N, &v1, &v2);

		//add primitive normal to all primitive's vertices
		pMeshVertices[i+1].normal += N;
		pMeshVertices[i+hmWidth].normal += N;
		pMeshVertices[i+hmWidth+1].normal += N;

		//skip last vert
		if ( i % hmWidth == hmWidth - 2 ) i++;
	}
	
	for ( unsigned int i = 0; i < numVertices - hmWidth; i++ )
	{
		D3DXVec3Normalize(&pMeshVertices[i].normal,&pMeshVertices[i].normal); 
	}

	//CREATE INDICES
	//--------------------------------------------------------------------------------

	//set indices
	unsigned int numIndices = (hmHeight-1) * (hmWidth-1) * 6;
	unsigned int *pMeshIndices = new unsigned int[numIndices];
	cnt = 0;

	for ( unsigned int i = 0; i < numVertices - hmWidth; i++ )
	{
		//add indices for 2 faces;
		pMeshIndices[cnt++] = i;
		pMeshIndices[cnt++] = i + hmWidth;
		pMeshIndices[cnt++] = i + 1;
		pMeshIndices[cnt++] = i + 1;
		pMeshIndices[cnt++] = i + hmWidth;
		pMeshIndices[cnt++] = i + hmWidth + 1;
		
		//skip last vert
		if ( i % hmWidth == hmWidth - 2 ) i++;
	}

	//CREATE MESH
	//--------------------------------------------------------------------------------
	if ( FAILED( D3DX10CreateMesh(	pD3DDevice, 
									vertexInputLayout, 
									vertexInputLayoutNumElements, 
									"POSITION", 
									numVertices, 
									numIndices/3, 
									D3DX10_MESH_32_BIT, 
									&pTerrainMesh) ) ) 
	{
		return fatalError("Could not create mesh!");
	}
	
	//insert data into mesh and commit changes
	pTerrainMesh->SetVertexData(0, pMeshVertices);
	pTerrainMesh->SetIndexData(pMeshIndices, numIndices);
	pTerrainMesh->CommitToDevice();

	//free memory
	delete[] pMeshVertices;
	delete[] pMeshIndices;
			
	//CREATE LIGHTS AND MATERIAL
	//--------------------------------------------------------------------------------
	ambientLight = D3DXVECTOR4(1.0f,1.0f,1.0f,1.0f);
	
	//set directional light - MAKE sure light direction is a unit vector
	directionalLight.color = D3DXVECTOR4(1.0f,1.0f,1.0f,1.0f);
	directionalLight.direction = D3DXVECTOR3(1,-1,0);
	D3DXVec3Normalize(&directionalLight.direction, &directionalLight.direction);

	material.ambient = 0.1f;
	material.diffuse = 0.5f;
	material.specular = 0.5f;
	material.shininess = 30;

	//set variables
	ID3D10EffectVariable* pVar = pBasicEffect->GetVariableByName( "light" );
	pVar->SetRawValue(&directionalLight, 0, sizeof(DirectionalLight));

	pVar = pBasicEffect->GetVariableByName( "material" );
	pVar->SetRawValue(&material, 0, sizeof(Material));

	pVar = pBasicEffect->GetVariableByName( "ambientLight" );
	pVar->SetRawValue( &ambientLight, 0, 16 );


	//Load Terrain Texture
	//--------------------------------------------------------------------------------
	if( D3DX10CreateShaderResourceViewFromFile(pD3DDevice, "Terrain_texture.jpg", NULL, NULL, &pTerrainTexture, NULL) )
	{
		return fatalError("Could not load terrain texture!");
	}

	return true;
}
/*******************************************************************
* Scene Renderer
*******************************************************************/
void dxManager::renderScene()
{
	//clear scene
	pD3DDevice->ClearRenderTargetView( pRenderTargetView, D3DXCOLOR(0.82f,0.863f,0.953f,1) );
	pD3DDevice->ClearDepthStencilView( pDepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0 );
	
	//set effect variables
	//------------------------------------------------------------------------

	//set view & projection matrices
	pViewMatrixEffectVariable->SetMatrix(pCamera->getViewMatrix());
	pProjectionMatrixEffectVariable->SetMatrix(pCamera->getProjectionMatrix());

	//set view position
	ID3D10EffectVectorVariable* var = pBasicEffect->GetVariableByName( "eye" )->AsVector();
	var->SetFloatVector( (float*) pCamera->getCameraPosition() );

	//set world matrix
	D3DXMatrixIdentity(&worldMatrix);
	pWorldMatrixEffectVariable->SetMatrix(worldMatrix);
	
	//set texture
	pColorMap->SetResource(pTerrainTexture);

	//draw terrain
	//------------------------------------------------------------------------
	
	//get appropriate technique
	switch (lightingTechnique)
	{
		case 0 : pTechnique = pBasicEffect->GetTechniqueByName("RENDER_VL_PHONG");	break;
		case 1 : pTechnique = pBasicEffect->GetTechniqueByName("RENDER_PL_PHONG");	break;
		case 2 : pTechnique = pBasicEffect->GetTechniqueByName("RENDER_VL_BLINNPHONG");	break;
		case 3 : pTechnique = pBasicEffect->GetTechniqueByName("RENDER_PL_BLINNPHONG");	break;
	};

	//get technique description
	pTechnique->GetDesc( &techDesc );

	//draw
	for( UINT p = 0; p < techDesc.Passes; p++ )
	{		
		//apply technique			
		pTechnique->GetPassByIndex( p )->Apply( 0 );
		pTerrainMesh->DrawSubset(0);			
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