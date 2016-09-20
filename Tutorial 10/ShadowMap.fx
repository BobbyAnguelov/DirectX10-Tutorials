//--------------------------------------------------------------------------------------
// CBuffers
//--------------------------------------------------------------------------------------

cbuffer perFrame
{
	matrix	viewProj;
	matrix	lightViewProj;
	float3	lightPos;	
};

cbuffer perObject
{
	matrix world;
};

//--------------------------------------------------------------------------------------
// Shader Structs
//--------------------------------------------------------------------------------------

struct VS_INPUT
{
	float4 pos : POSITION;	
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 bitangent : BITANGENT;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float4 wpos : TEXCOORD0;
	float4 lpos : TEXCOORD1;		//vertex with regard to light view
	float3 normal : NORMAL;  
};

struct SHADOW_PS_INPUT
{
	float4 pos : SV_POSITION;	
};

struct PS_INPUT2
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

RasterizerState frontFaceCulling
{
	cullmode = front;
};

RasterizerState backFaceCulling
{
	cullmode = back;
};

//--------------------------------------------------------------------------------------
// Global Vars
//--------------------------------------------------------------------------------------
float4 ambient = float4(0.3f, 0.3f, 0.3f, 0.0f);
float4 diffuse = float4(0.7f, 0.7f, 0.7f, 0.0f);	
float shadowMapBias;
uint2 shadowMapSize;

//--------------------------------------------------------------------------------------
// Shadow Map Samplers
//--------------------------------------------------------------------------------------

Texture2D shadowMap;
SamplerState bilinearSampler
{
    Filter = min_mag_mip_linear;
    AddressU = MIRROR;
    AddressV = MIRROR;	
};

SamplerState pointSampler
{
    Filter = min_mag_mip_point;
    AddressU = MIRROR;
    AddressV = MIRROR;	
};

SamplerComparisonState cmpSampler
{
   // sampler state
   Filter = COMPARISON_MIN_MAG_MIP_LINEAR;
   AddressU = MIRROR;
   AddressV = MIRROR;

   // sampler comparison state
   ComparisonFunc = LESS_EQUAL;   
};


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
	PS_INPUT output;
    output.pos = mul( input.pos, mul( world, viewProj ) );	
	output.wpos = input.pos;
	output.lpos = mul( input.pos, mul( world, lightViewProj ) );
	output.normal = input.normal;
    return output;  
}

SHADOW_PS_INPUT ShadowMapVS( VS_INPUT input )
{
	SHADOW_PS_INPUT output;
    output.pos = mul( input.pos, mul( world, lightViewProj ) );		
    return output;  
}

float4 VS2( VS_INPUT input ) : SV_POSITION
{
	return float4(0,0,0,0);
}

//--------------------------------------------------------------------------------------
// Geometry Shader - Billboard
//--------------------------------------------------------------------------------------

[maxvertexcount(6)]
void GS( point float4 s[1] : SV_POSITION, inout TriangleStream<PS_INPUT2> triStream )
{
	PS_INPUT2 p;
	
	p.pos = float4(-1,0.50,0,1);
	p.uv = float2(0,1);
	triStream.Append(p);
	
	p.pos = float4(-1,1,0,1);
	p.uv = float2(0,0);
	triStream.Append(p);

	p.pos = float4(-0.50,0.50,0,1);
	p.uv = float2(1,1);
	triStream.Append(p);

	p.pos = float4(-0.50,1,0,1);
	p.uv = float2(1,0);
	triStream.Append(p);
}

//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------
float4 PS_STANDARD( PS_INPUT input ) : SV_Target
{
	//re-homogenize position after interpolation
	input.lpos.xyz /= input.lpos.w;

	//if position is not visible to the light - dont illuminate it
	//results in hard light frustum
	if( input.lpos.x < -1.0f || input.lpos.x > 1.0f || 
	    input.lpos.y < -1.0f || input.lpos.y > 1.0f ||
	    input.lpos.z < 0.0f  || input.lpos.z > 1.0f ) return ambient;

	//transform clip space coords to texture space coords (-1:1 to 0:1)	
	input.lpos.x = input.lpos.x/2 + 0.5;
	input.lpos.y = input.lpos.y/-2 + 0.5;
	
	//apply shadow map bias
	input.lpos.z -= shadowMapBias;

	//sample shadow map - point sampler
	float shadowMapDepth = shadowMap.Sample(pointSampler, input.lpos.xy).r;

	//if clip space z value greater than shadow map value then pixel is in shadow
	float shadowFactor = input.lpos.z <= shadowMapDepth;	
	
	//calculate ilumination at fragment
	float3 L = normalize(lightPos - input.wpos.xyz);	
	float ndotl = dot( normalize(input.normal), L);
	return ambient + shadowFactor*diffuse*ndotl;
}

float4 PS_PCF( PS_INPUT input ) : SV_Target
{
	//re-homogenize position after interpolation
	input.lpos.xyz /= input.lpos.w;

	//if position is not visible to the light - dont illuminate it
	//results in hard light frustum
	if( input.lpos.x < -1.0f || input.lpos.x > 1.0f || 
	    input.lpos.y < -1.0f || input.lpos.y > 1.0f ||
	    input.lpos.z < 0.0f  || input.lpos.z > 1.0f ) return ambient;

	//transform clip space coords to texture space coords (-1:1 to 0:1)	
	input.lpos.x = input.lpos.x/2 + 0.5;
	input.lpos.y = input.lpos.y/-2 + 0.5;
	
	//apply shadow map bias
	input.lpos.z -= shadowMapBias;
	
	//basic hardware PCF - single texel
	float shadowFactor = shadowMap.SampleCmpLevelZero( cmpSampler, input.lpos.xy, input.lpos.z );

	//calculate ilumination at fragment
	float3 L = normalize(lightPos - input.wpos.xyz);	
	float ndotl = dot( normalize(input.normal), L);
	return ambient + shadowFactor*diffuse*ndotl;
}

float2 texOffset( int u, int v )
{
	return float2( u * 1.0f/shadowMapSize.x, v * 1.0f/shadowMapSize.y );
}

float4 PS_PCF2( PS_INPUT input ) : SV_Target
{
	//re-homogenize position after interpolation
	input.lpos.xyz /= input.lpos.w;

	//if position is not visible to the light - dont illuminate it
	//results in hard light frustum
	if( input.lpos.x < -1.0f || input.lpos.x > 1.0f || 
	    input.lpos.y < -1.0f || input.lpos.y > 1.0f ||
	    input.lpos.z < 0.0f  || input.lpos.z > 1.0f ) return ambient;

	//transform clip space coords to texture space coords (-1:1 to 0:1)	
	input.lpos.x = input.lpos.x/2 + 0.5;
	input.lpos.y = input.lpos.y/-2 + 0.5;
	
	//apply shadow map bias
	input.lpos.z -= shadowMapBias;
	
	//PCF sampling for shadow map
	float sum = 0;
	float x, y;	

	//perform PCF filtering on a 4 x 4 texel neighborhood
	for (y = -1.5; y <= 1.5; y += 1.0)
	{
		for (x = -1.5; x <= 1.5; x += 1.0)
		{
			sum += shadowMap.SampleCmpLevelZero( cmpSampler, input.lpos.xy + texOffset(x,y), input.lpos.z );			
		}
	}

	float shadowFactor = sum / 16.0;	 

	//calculate ilumination at fragment
	float3 L = normalize(lightPos - input.wpos.xyz);	
	float ndotl = dot( normalize(input.normal), L);
	return ambient + shadowFactor*diffuse*ndotl;
}

float4 PS2( PS_INPUT2 input ) : SV_TARGET
{		
	float r = shadowMap.Sample(bilinearSampler, input.uv).r;
	return float4(r, r, r, 1);	
}

void ShadowMapPS( SHADOW_PS_INPUT input ) {}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique10 ShadowMapRenderPassFrontFaces
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, ShadowMapVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ShadowMapPS() ) );
		SetRasterizerState( backFaceCulling );		
    }
}

technique10 ShadowMapRenderPassBackFaces
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, ShadowMapVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, ShadowMapPS() ) );
		SetRasterizerState( frontFaceCulling );		
    }
}

technique10 RenderBillboard
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS2() ) );
        SetGeometryShader( CompileShader( gs_4_0, GS() ) );
        SetPixelShader( CompileShader( ps_4_0, PS2() ) );	
		SetRasterizerState( backFaceCulling );		
    }
}

technique10 RenderPassStandard
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_STANDARD() ) );				
		SetRasterizerState( backFaceCulling );		
    }
}

technique10 RenderPassPCF
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_PCF() ) );				
		SetRasterizerState( backFaceCulling );		
    }
}

technique10 RenderPassPCF2
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_PCF2() ) );				
		SetRasterizerState( backFaceCulling );		
    }
}