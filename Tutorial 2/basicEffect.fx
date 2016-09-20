//--------------------------------------------------------------------------------------
// basicEffect.fx
//
//--------------------------------------------------------------------------------------

matrix World;
matrix View;
matrix Projection;

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( float4 Pos : POSITION, float4 Color : COLOR )
{
	PS_INPUT psInput;
	
	Pos = mul( Pos, World );
    Pos = mul( Pos, View );    
    psInput.Pos = mul( Pos, Projection );
	psInput.Color = Color;
	
    return psInput;  
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT psInput ) : SV_Target
{
    return psInput.Color; 
}

//--------------------------------------------------------------------------------------
technique10 Render
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}