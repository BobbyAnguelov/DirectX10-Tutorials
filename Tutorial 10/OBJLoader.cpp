#include "OBJLoader.h"

#include <vector>
using namespace std;

//helper structs
struct int9 { int a,b,c,d,e,f,g,h,i; };

//helper functions
inline void parseFloat3Line( char* buf, vector<float3>* pStorage )
{
	float3 v;		
	sscanf_s(buf, "%f %f %f", &v.x,&v.y,&v.z);			
	pStorage->push_back(v);
}

inline void parseFloat2Line( char* buf, vector<float2>* pStorage )
{
	float2 v;		
	sscanf_s(buf, "%f %f", &v.x,&v.y);	
	pStorage->push_back(v);
}

inline void parseFaceLine( char* buf, vector<int9>* pStorage )
{
	int9 v;		
	sscanf_s(buf+2,  "%i/%i/%i %i/%i/%i %i/%i/%i",	&v.a, &v.b, &v.c,
													&v.d, &v.e, &v.f,
													&v.g, &v.h, &v.i );

	//correct indices to start at 0
	for ( int i=0; i < 9; i++ ) ((int*) &v)[i]--;	
	pStorage->push_back(v);
}

inline void copyFloat3toFloatArray(float* pDest, float3 &rhs)
{
	pDest[0] = rhs.x;
	pDest[1] = rhs.y;
	pDest[2] = rhs.z;
}
	
inline void copyFloat2toFloatArray(float* pDest, float2 &rhs)
{
	pDest[0] = rhs.x;
	pDest[1] = rhs.y;		
}

bool loadMeshDataFromOBJ(const char* filename, MeshData* pMeshData)
{
	//create file handle
	FILE* objFile;	
	char path[255], buf[255];

	if ( fopen_s( &objFile, filename, "r" ) != 0 ) return false;
				
	//get model path		
	for ( int i = strlen(filename)-2; i >=0; i-- )
	{
		if ( filename[i] == '/' ) 
		{	
			strncpy_s(path, filename, i+1);
			break;
		}
	}		
		
	//temporary data storage
	vector<float3> vertices;
	vector<float3> normals;
	vector<float2> texcoords;
	vector<int9> faces;
		
	//read OBJ data
	while ( !feof(objFile) )
	{
		fgets(buf, 255, objFile);

		switch ( buf[0] )
		{							
			//vertex, normal, texcoords
			case 'v'	:	if ( buf[1] == 'n' ) 
							{
								parseFloat3Line(buf+2, &normals);								
							}
							else if ( buf[1] == 't' ) 
							{
								parseFloat2Line(buf+2, &texcoords);
								texcoords.back().y = -texcoords.back().y;	//flip v param
							}
							else 
							{
								parseFloat3Line(buf+1, &vertices);									
							}
							break;				
			//face
			case 'f'	:	parseFaceLine(buf, &faces);
							break;				
			//comments
			default		:	break;	
		}		
	}		
		
	//fill mesh data from OBJ data
	pMeshData->numVertices = faces.size() * 3;
	pMeshData->numIndices = pMeshData->numVertices;

	pMeshData->pVertices = new MeshVertex[pMeshData->numVertices];
	pMeshData->pIndices = new unsigned int[pMeshData->numIndices];

	MeshVertex v;
	unsigned int vi = 0;

	for ( unsigned int i=0; i < faces.size(); i++ )
	{
		v.pos = vertices[faces[i].a];
		v.uv = texcoords[faces[i].b];
		v.normal = normals[faces[i].c];

		pMeshData->pVertices[vi++] = v;

		v.pos = vertices[faces[i].d];
		v.uv = texcoords[faces[i].e];
		v.normal = normals[faces[i].f];

		pMeshData->pVertices[vi++] = v;

		v.pos = vertices[faces[i].g];
		v.uv = texcoords[faces[i].h];
		v.normal = normals[faces[i].i];

		pMeshData->pVertices[vi++] = v;
	}

	//create indices
	for (unsigned int i=0; i< pMeshData->numIndices; i++ ) pMeshData->pIndices[i] = i;

	//close file
	fclose( objFile);	
	return true;		
}
