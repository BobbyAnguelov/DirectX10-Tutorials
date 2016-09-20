#include "dxManager.h"

/*******************************************************************
* Initialize Scene Objects
*******************************************************************/
inline float convertPixelsToClipSpace( const int pixelDimension, const int pixels )
{
	return (float)pixels/pixelDimension*2 -1;
}

inline float convertPixelsToClipSpaceDistance( const int pixelDimension, const int pixels )
{
	return (float)pixels/pixelDimension*2;
}

bool dxManager::initializeSpriteObjects()
{
	//Create sprites
	//--------------------------------------------------------------------------------		
SpriteVertex verts[3];
numSprites = 3;

//header is positioned at 312, 0 with dimensions 400x42
verts[0].topLeft[0] = convertPixelsToClipSpace(800,200);
verts[0].topLeft[1] = -convertPixelsToClipSpace(600,0);
verts[0].dimensions[0] = convertPixelsToClipSpaceDistance(800,400);
verts[0].dimensions[1] = convertPixelsToClipSpaceDistance(600,42);
verts[0].opacity = 1;

verts[1].topLeft[0] = convertPixelsToClipSpace(800,0);
verts[1].topLeft[1] = -convertPixelsToClipSpace(600,500);
verts[1].dimensions[0] = convertPixelsToClipSpaceDistance(800,100);
verts[1].dimensions[1] = convertPixelsToClipSpaceDistance(600,100);
verts[1].opacity = 1;

verts[2].topLeft[0] = convertPixelsToClipSpace(800,700);
verts[2].topLeft[1] = -convertPixelsToClipSpace(600,500);
verts[2].dimensions[0] = convertPixelsToClipSpaceDistance(800,100);
verts[2].dimensions[1] = convertPixelsToClipSpaceDistance(600,100);
verts[2].opacity = 0.3f;	

//create vertex buffer
D3D10_SUBRESOURCE_DATA initData;
initData.pSysMem = &verts;

D3D10_BUFFER_DESC bd;
bd.Usage = D3D10_USAGE_DEFAULT;
bd.ByteWidth = sizeof( SpriteVertex ) * (numSprites);
bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
bd.CPUAccessFlags = 0;
bd.MiscFlags = 0;
		
if ( FAILED( pD3DDevice->CreateBuffer( &bd, &initData, &pVertexBuffer ) ) ) return fatalError("Could not create vertex buffer!");

	// Set vertex buffer
	UINT stride = sizeof( SpriteVertex );
	UINT offset = 0;
	pD3DDevice->IASetVertexBuffers( 0, 1, &pVertexBuffer, &stride, &offset );

	//Load Texture
	//--------------------------------------------------------------------------------
	if( FAILED(D3DX10CreateShaderResourceViewFromFile(pD3DDevice, "textures/sprite.png", NULL, NULL, &pTexture1, NULL)) )
	{
		return fatalError("Could not load sprite texture!");
	}
	if( FAILED(D3DX10CreateShaderResourceViewFromFile(pD3DDevice, "textures/header.png", NULL, NULL, &pTexture2, NULL)) )
	{
		return fatalError("Could not load billboard texture!");	
	}
	return true;
}
