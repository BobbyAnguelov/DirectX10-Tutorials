//--------------------------------------------------------------------------------------
// basicEffect.fx
//
//--------------------------------------------------------------------------------------

Texture2D tex2D;
SamplerState linearSampler
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

matrix World;
matrix View;
matrix Projection;

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD;
};

struct VS_INPUT
{
	float4 Pos : POSITION;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
	PS_INPUT output;
	
	output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );    
    output.Pos = mul( output.Pos, Projection );
	output.Color = input.Color;
	output.Tex = input.Tex;
	
    return output;  
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 textured( PS_INPUT input ) : SV_Target
{
    return tex2D.Sample( linearSampler, input.Tex ); 
}

float4 noTexture( PS_INPUT input ) : SV_Target
{
    return input.Color; 
}


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique10 full
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, textured() ) );
    }
}

technique10 texturingDisabled
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, noTexture() ) );
    }
}