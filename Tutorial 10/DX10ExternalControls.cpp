#include "DX10Renderer.h"

//external controls
void DX10Renderer::SetTechnique( unsigned int i )
{
	pRenderTechnique = pEffect->GetTechniqueByIndex(i);
}

void DX10Renderer::SetShadowMapTechnique( unsigned int i)
{
	pRenderShadowMapTechnique = pEffect->GetTechniqueByIndex(i);
}

void DX10Renderer::ToggleShadowMapSize()
{
	//set size according to current size
	if ( shadowMapViewport.Width == viewport.Width )
	{
		shadowMapViewport.Width = viewport.Width * 2;
		shadowMapViewport.Height = viewport.Height * 2;
	}
	else
	{
		shadowMapViewport.Width = viewport.Width;
		shadowMapViewport.Height = viewport.Height;
	}

	//release current shadow map resources
	pShadowMapDepthView->Release();
	pShadowMapSRView->Release();
	pShadowMap->Release();

	//recreate resources
	D3D10_TEXTURE2D_DESC texDesc;
	texDesc.Width = shadowMapViewport.Width;
	texDesc.Height = shadowMapViewport.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D10_USAGE_DEFAULT;
	texDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;   
    
	D3D10_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;		

	D3D10_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	pD3DDevice->CreateTexture2D( &texDesc, NULL, &pShadowMap );
	pD3DDevice->CreateDepthStencilView( pShadowMap, &descDSV, &pShadowMapDepthView );
	pD3DDevice->CreateShaderResourceView( pShadowMap, &srvDesc, &pShadowMapSRView );	
}

void DX10Renderer::SetShadowMapBias( float b )
{
	shadowMapBias = b;
}