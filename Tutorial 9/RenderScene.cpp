#include "dxManager.h"
extern int alignment;

/*******************************************************************
* Scene Renderer
*******************************************************************/
void dxManager::renderScene()
{
	//clear scene
	pD3DDevice->ClearRenderTargetView( pRenderTargetView, D3DXCOLOR(0.82f,0.863f,0.953f,1) );
	pD3DDevice->ClearDepthStencilView( pDepthStencilView, D3D10_CLEAR_DEPTH, 1.0f, 0 );
			
	//set topology
	pD3DDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
		
	//draw sprites
	//------------------------------------------------------------------------
	
	//set texture for header
	pColorMap->SetResource(pTexture2);
		
	//draw header
	for( UINT p = 0; p < techDesc.Passes; p++ )
	{		
		//apply technique			
		pTechnique->GetPassByIndex( p )->Apply( 0 );	

		//draw billboard
		pD3DDevice->Draw(1,0);
	}		
	
	//set texture for sprites
	pColorMap->SetResource(pTexture1);
		
	//draw extra sprites
	for( UINT p = 0; p < techDesc.Passes; p++ )
	{		
		//apply technique			
		pTechnique->GetPassByIndex( p )->Apply( 0 );	

		//draw billboard
		pD3DDevice->Draw(numSprites-1,1);
	}		

	//flip buffers
	pSwapChain->Present(0,0);
}