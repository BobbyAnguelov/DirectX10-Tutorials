//create a basic vertex type
struct SpriteVertex
{
	//clip space coordinates
	float topLeft[2];
	float dimensions[2];
	float opacity;	
};

const D3D10_INPUT_ELEMENT_DESC vertexInputLayout[] = 
{	
	{ "ANCHOR", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	{ "DIMENSIONS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	{ "OPACITY", 0, DXGI_FORMAT_R32_FLOAT, 0, 16, D3D10_INPUT_PER_VERTEX_DATA, 0 }
};

const int vertexInputLayoutNumElements = sizeof(vertexInputLayout)/sizeof(vertexInputLayout[0]);