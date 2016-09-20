//--------------------------------------------------------------------------------------
// basicEffect.fx
//
//--------------------------------------------------------------------------------------

matrix World;
matrix View;
matrix Projection;

uint activeViewport;

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
    float4 Color : COLOR0;		
};

struct GS_OUTPUT
{
	float4 Pos : SV_POSITION;
    float4 Color : COLOR0;	
	uint viewport : SV_ViewportArrayIndex;
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
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(3)]
void GS( triangle PS_INPUT In[3], inout TriangleStream<GS_OUTPUT> TriStream )
{
	GS_OUTPUT output;
   	output.viewport = activeViewport;

    for( int v = 0; v < 3; v++ )
    {
		output.Pos = In[v].Pos;
        output.Color = In[v].Color; 			
		TriStream.Append( output );
    }
}

//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input ) : SV_Target
{
    return input.Color; 
}

//--------------------------------------------------------------------------------------
technique10 RenderMethod1
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}

technique10 RenderMethod2
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( CompileShader( gs_4_0, GS() ) );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}