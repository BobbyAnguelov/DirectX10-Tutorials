#include "DX10Renderer.h"
#include "Camera.h"

extern Camera camera;

//render frame
void DX10Renderer::RenderFrame()
{
		
	//rotate light - shouldnt do this each frame but i'm lazy
	//***************************************************************************

	//rotate light position
	/*XMFLOAT3 yaxis(0,1,0), o(0,0,0), u(0,1,0);
	XMMATRIX rotMatrix = XMMatrixRotationAxis( XMLoadFloat3(&yaxis), 0.01f);
	
	//new rotated light pos
	XMVECTOR olpos = XMLoadFloat3(&lightPos);
	XMVECTOR lpos = XMVector3TransformCoord( olpos , rotMatrix);
	XMStoreFloat3(&lightPos, lpos);
	
	//get new view matrix
	XMVECTOR fp = XMLoadFloat3( &o  ), up = XMLoadFloat3( &u );	 
	XMMATRIX vmat = XMMatrixLookAtLH( lpos, fp, up );
	
	//create new light view proj matrix
	XMMATRIX lpmat = XMLoadFloat4x4(&lightProjMatrix);
	XMMATRIX lvpmat = XMMatrixMultiply(vmat, lpmat);
	XMStoreFloat4x4(&lightViewProjMatrix, lvpmat);*/
		
	//set per frame variables
	//***************************************************************************
	
	//effect vars
	ID3D10EffectMatrixVariable* pWorldMatrix = pEffect->GetVariableByName("world")->AsMatrix();
	ID3D10EffectMatrixVariable* pViewProjMatrix = pEffect->GetVariableByName("viewProj")->AsMatrix();
	ID3D10EffectMatrixVariable* pLightViewProjMatrix = pEffect->GetVariableByName("lightViewProj")->AsMatrix();
	ID3D10EffectVectorVariable* pLightPos = pEffect->GetVariableByName("lightPos")->AsVector();
	
	pLightPos->SetFloatVectorArray( (float*) &lightPos, 0, 3);
	pViewProjMatrix->SetMatrix( (float*) &camera.GetViewProjectionMatrix() );	
	pLightViewProjMatrix->SetMatrix( (float*) &lightViewProjMatrix );
	
	//set topology to triangle list
	pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	//create shadow map
	//***************************************************************************
		
	//set render targets and viewport
	pD3DDevice->OMSetRenderTargets(0, 0, pShadowMapDepthView);
	pD3DDevice->RSSetViewports(1, &shadowMapViewport);
	pD3DDevice->ClearDepthStencilView( pShadowMapDepthView, D3D10_CLEAR_DEPTH, 1.0f, 0 );

	//render scene
	int currMeshID = -1;	
	for (unsigned int i=0; i < scene.size(); i++)
	{
		//set appropriate vertex buffers
		if ( currMeshID != scene[i].modelID )
		{
			currMeshID = scene[i].modelID;
			pD3DDevice->IASetVertexBuffers( 0,1, &meshes[currMeshID].pVertexBuffer, &meshes[currMeshID].stride, &meshes[currMeshID].offset );
			pD3DDevice->IASetIndexBuffer( meshes[currMeshID].pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		}
		
		//set instance data and draw
		pWorldMatrix->SetMatrix((float*) &scene[i].world);
		pRenderShadowMapTechnique->GetPassByIndex(0)->Apply( 0 );	
		pD3DDevice->DrawIndexed(meshes[scene[i].modelID].numIndices, 0, 0);
	}	

	//render final scene	
	//***************************************************************************
		
	//set render targets and viewports
	pD3DDevice->OMSetRenderTargets(1, &pRenderTargetView, pDepthStencilView);	
	pD3DDevice->RSSetViewports(1, &viewport);
	pD3DDevice->ClearRenderTargetView( pRenderTargetView, D3DXCOLOR(0.6f,0.6f,0.6f,0) );
	pD3DDevice->ClearDepthStencilView( pDepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0 );
		
	//bind shadow map texture and set shadow map bias
	pEffect->GetVariableByName("shadowMap")->AsShaderResource()->SetResource( pShadowMapSRView );
	pEffect->GetVariableByName("shadowMapBias")->AsScalar()->SetFloat(shadowMapBias);
	int smSize[2] = { shadowMapViewport.Width, shadowMapViewport.Height };
	pEffect->GetVariableByName("shadowMapSize")->AsVector()->SetIntVector(smSize);

	//render scene
	currMeshID = -1;	
	for (unsigned int i=0; i < scene.size(); i++)
	{
		//set appropriate vertex buffers
		if ( currMeshID != scene[i].modelID )
		{
			currMeshID = scene[i].modelID;
			pD3DDevice->IASetVertexBuffers( 0,1, &meshes[currMeshID].pVertexBuffer, &meshes[currMeshID].stride, &meshes[currMeshID].offset );
			pD3DDevice->IASetIndexBuffer( meshes[currMeshID].pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
		}
		
		//set instance data and draw
		pWorldMatrix->SetMatrix((float*) &scene[i].world);
		pRenderTechnique->GetPassByIndex(0)->Apply( 0 );	
		pD3DDevice->DrawIndexed(meshes[scene[i].modelID].numIndices, 0, 0);
	}	
	
	//render shadow map billboard
	//***************************************************************************
	pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);	

	pBillboardTechnique->GetPassByIndex(0)->Apply( 0 );
	pD3DDevice->DrawIndexed(1, 0, 0);
	
	//unbind shadow map as SRV
	pEffect->GetVariableByName("shadowMap")->AsShaderResource()->SetResource( 0 );
	pBillboardTechnique->GetPassByIndex(0)->Apply( 0 );

	//swap buffers
	pSwapChain->Present(0,0);
}