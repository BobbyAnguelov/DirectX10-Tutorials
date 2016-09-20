#ifndef _OBJ_LOADERS
#define _OBJ_LOADERS

#include "NMD_Types.h"

//intermediary mesh data struct
struct MeshVertex
{
	float3 pos;
	float2 uv;
	float3 normal;
	float3 bitangent;
};

struct MeshData
{			
	//vertex data
	MeshVertex* pVertices;		
	unsigned int numVertices;

	//index data
	unsigned int* pIndices;
	unsigned int numIndices;				

	void Release()
	{
		if ( pVertices ) delete[] pVertices;
		if ( pIndices ) delete[] pIndices;
	}
};

//model loaders
bool loadMeshDataFromOBJ(const char* filename, MeshData* pModelData);

#endif