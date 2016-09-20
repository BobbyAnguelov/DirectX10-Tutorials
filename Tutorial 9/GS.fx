//--------------------------------------------------------------------------------------
// Sprite Rendering Effect - 2011 - Bobby Anguelov
//--------------------------------------------------------------------------------------

//color map texture
Texture2D colorMap;

//texture sampler state
SamplerState linearSampler
{
    Filter = min_mag_mip_linear;
    AddressU = Clamp;
    AddressV = Clamp;
    MaxAnisotropy = 16;
};

//blend state
BlendState SrcAlphaBlendingAdd 
{ 
	BlendEnable[0] = TRUE; 
	SrcBlend = SRC_ALPHA; 
	DestBlend = INV_SRC_ALPHA; 
	BlendOp = ADD; 
	SrcBlendAlpha = ZERO; 
	DestBlendAlpha = ZERO; 
	BlendOpAlpha = ADD; 
	RenderTargetWriteMask[0] = 0x0F; 
}; 

//VERTEX AND PIXEL SHADER INPUTS
//--------------------------------------------------------------------------------------
struct SPRITE_INPUT
{
	float2 topLeft : ANCHOR;
	float2 dimensions : DIMENSIONS;
	float opacity: OPACITY;
};

//pixel shader inputs
struct PS_INPUT
{
	float4 p : SV_POSITION; 
	float2 t : TEXCOORD;
	float opacity : OPACITY;	
};

//--------------------------------------------------------------------------------------
// VERTEX SHADER
//--------------------------------------------------------------------------------------
SPRITE_INPUT VS( SPRITE_INPUT input )
{
	return input;  
}

//--------------------------------------------------------------------------------------
// GEOMETRY SHADER
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void GS( point SPRITE_INPUT sprite[1], inout TriangleStream<PS_INPUT> triStream )
{
	PS_INPUT v;
	v.opacity = sprite[0].opacity;			
	
	//create sprite quad
	//--------------------------------------------

	//bottom left
	v.p = float4(sprite[0].topLeft[0],sprite[0].topLeft[1]-sprite[0].dimensions[1],0,1);	
	v.t = float2(0,1);	
	triStream.Append(v);
	
	//top left
	v.p = float4(sprite[0].topLeft[0],sprite[0].topLeft[1],0,1);	
	v.t = float2(0,0);
	triStream.Append(v);

	//bottom right
	v.p = float4(sprite[0].topLeft[0]+sprite[0].dimensions[0],sprite[0].topLeft[1]-sprite[0].dimensions[1],0,1);	
	v.t = float2(1,1);
	triStream.Append(v);

	//top right
	v.p = float4(sprite[0].topLeft[0]+sprite[0].dimensions[0],sprite[0].topLeft[1],0,1);	
	v.t = float2(1,0);
	triStream.Append(v);
}
//--------------------------------------------------------------------------------------
// PIXEL SHADER
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input ) : SV_Target
{ 
	float4 color = colorMap.Sample(linearSampler, input.t);
	color[3] = min(color[3],input.opacity);
	return color;      
}
//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique10 RENDER
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( CompileShader( gs_4_0, GS() ) );		
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
        SetBlendState( SrcAlphaBlendingAdd, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
    }    
}