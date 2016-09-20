#include "DX10Renderer.h"

struct vert
{
	float pos[3];
	float normal[3];

	vert( float p1, float p2, float p3, float n1, float n2, float n3 )
	{
		pos[0] = p1;
		pos[1] = p2;
		pos[2] = p3;

		normal[0] = n1;
		normal[1] = n2;
		normal[2] = n3;
	}
};

bool DX10Renderer::InitializeScene()
{
	//set light params
	//==================================================================

	lightPos = XMFLOAT3(0, 20, -30);

	//light volume proj
	//XMMATRIX pm = XMMatrixOrthographicLH(75, 75, 20.0f, 80.f);
	XMMATRIX pm = XMMatrixPerspectiveFovLH( XM_PIDIV4, 4.0f/3, 20.0f, 80.f );
	XMStoreFloat4x4(&lightProjMatrix, pm);

	//view
	XMFLOAT3 o = XMFLOAT3(0,0,0), up = XMFLOAT3(0,1,0);
	XMVECTOR e = XMLoadFloat3( &lightPos ), fp = XMLoadFloat3( &o ), u = XMLoadFloat3( &up );
	XMMATRIX vm = XMMatrixLookAtLH( e, fp, u );
	pm = XMMatrixMultiply(vm,pm);
	XMStoreFloat4x4(&lightViewProjMatrix, pm);
			
	//Load rendering techniques
	//==================================================================

	if ( FAILED( D3DX10CreateEffectFromFile( L"ShadowMap.fx", 0, 0, (LPCSTR) "fx_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, pD3DDevice, 0, 0, &pEffect, 0, 0) ) ) return false; 
	
	pRenderTechnique = pEffect->GetTechniqueByName("RenderPassStandard");
	pRenderShadowMapTechnique = pEffect->GetTechniqueByName("ShadowMapRenderPassFrontFaces");
	pBillboardTechnique = pEffect->GetTechniqueByName("RenderBillboard");

	//create input layout
	D3D10_PASS_DESC passDesc;
	pRenderTechnique->GetPassByIndex( 0 )->GetDesc( &passDesc );
		
	D3D10_INPUT_ELEMENT_DESC inLayoutDesc[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D10_INPUT_PER_VERTEX_DATA, 0 }
	};

	if ( FAILED( pD3DDevice->CreateInputLayout( inLayoutDesc, 4, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &pInputLayout ) ) ) return false;

	pD3DDevice->IASetInputLayout(pInputLayout);
	
	//load scene elements
	//==================================================================
	MeshData md;
	
	loadMeshDataFromOBJ("models\\floor.obj", &md);
	meshes.push_back( DX10Mesh() );
	meshes.back().CreateMesh(pD3DDevice, &md);
	
	loadMeshDataFromOBJ("models\\girl.obj", &md);
	meshes.push_back( DX10Mesh() );
	meshes.back().CreateMesh(pD3DDevice, &md);
	
	RenderMessage rm;
	XMMATRIX mat = XMMatrixIdentity();
	rm.modelID = 0;
	XMStoreFloat4x4(&rm.world, mat);

	//add floor to scene
	scene.push_back(rm);
	
	//add 9 meshes to scene
	float y = 0.0f;
	XMFLOAT3 a(1,0,0);
	XMVECTOR axis = XMLoadFloat3( &a);

	for (float z = -10; z <= 10; z+=5)
	{		
		for (float x = -10; x <= 10; x+=5 )
		{
			mat = XMMatrixRotationAxis( axis, -XM_PIDIV2) * XMMatrixTranslation(x,y,z);
			
			scene.push_back( rm );			
			scene.back().modelID = 1;
			XMStoreFloat4x4(&scene.back().world, mat);
		}		
	}
			
	return true;
}
