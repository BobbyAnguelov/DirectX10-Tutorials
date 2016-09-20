//create a basic vertex type
struct vertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR4 color;
	
	vertex ()
	{		
	}
	
	vertex( D3DXVECTOR3 p, D3DXVECTOR4 c )
	{
		pos = p;
		color = c;
	}	
};

const D3D10_INPUT_ELEMENT_DESC vertexInputLayout[] = 
{	
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 }	
};
