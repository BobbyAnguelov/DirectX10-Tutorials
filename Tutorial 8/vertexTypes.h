//create a basic vertex type
struct vertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR2 texCoords;
	D3DXVECTOR3 normal;

	vertex () : normal( D3DXVECTOR3(0,0,0) )
	{			
	}
	
	vertex( D3DXVECTOR3 p, D3DXVECTOR3 n ) : pos(p), normal(n)
	{		
	}	

	vertex( D3DXVECTOR3 p, D3DXVECTOR2 t, D3DXVECTOR3 n ) : pos(p), normal(n), texCoords(t)
	{
	}	
};

const D3D10_INPUT_ELEMENT_DESC vertexInputLayout[] = 
{	
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D10_INPUT_PER_VERTEX_DATA, 0 }	
};

const int vertexInputLayoutNumElements = sizeof(vertexInputLayout)/sizeof(vertexInputLayout[0]);