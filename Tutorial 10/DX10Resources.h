#ifndef _DX10_RESOURCES
#define _DX10_RESOURCES

#include <D3D10.h>
#include "OBJLoader.h"

//DX10 Mesh Object - Avoid using D3DX
struct DX10Mesh
{	
	//size values
	unsigned int size, numVertices, numIndices, stride, offset;
		
	//vertex and index buffers
	ID3D10Buffer* pVertexBuffer;
	ID3D10Buffer* pIndexBuffer;
		
	///create and release methods
	DX10Mesh();
	bool CreateMesh(ID3D10Device* pD3DDevice, const MeshData* pData);
	void Release();
};

#endif