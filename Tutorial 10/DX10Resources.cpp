#include "DX10Resources.h"

//DX10 MESH
//========================================================================================
DX10Mesh::DX10Mesh():pVertexBuffer(0),pIndexBuffer(0),size(0),stride(0),offset(0){}

//create DX10 mesh from intermediary mesh data
bool DX10Mesh::CreateMesh(ID3D10Device* pD3DDevice, const MeshData* pData)
{
	//subresource data
	D3D10_SUBRESOURCE_DATA bufferData;
	ZeroMemory(&bufferData, sizeof(D3D10_SUBRESOURCE_DATA));
	
	//set VB stride and offset
	stride = sizeof( MeshVertex );
	offset = 0;
		
	//set vertex buffer description
	D3D10_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D10_USAGE_DEFAULT;
	bufferDesc.ByteWidth = stride * pData->numVertices;
	bufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	//set buffer data and create buffer
	bufferData.pSysMem = pData->pVertices;
	if ( FAILED( pD3DDevice->CreateBuffer( &bufferDesc, &bufferData, &pVertexBuffer ) ) ) 
	{
		size = stride = offset = 0;
		return false;
	}		
	
	//set index buffer description	
	bufferDesc.ByteWidth = sizeof( unsigned int ) * pData->numIndices;
	bufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	
	//set buffer data and create buffer
	bufferData.pSysMem = pData->pIndices;
	if ( FAILED( pD3DDevice->CreateBuffer( &bufferDesc, &bufferData, &pIndexBuffer ) ) ) 
	{
		if (pVertexBuffer) pVertexBuffer->Release();
		size = stride = offset = 0;
		return false;
	}

	//set buffer sizes and mesh memory usage
	numVertices = pData->numVertices;
	numIndices = pData->numIndices;
	size = numIndices * 4 + numVertices * sizeof(MeshVertex);

	return true;
}

//release buffers
void DX10Mesh::Release()
{
	if (pVertexBuffer) pVertexBuffer->Release();
	if (pIndexBuffer) pIndexBuffer->Release();
}
