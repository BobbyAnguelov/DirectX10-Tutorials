#ifndef _LIGHTS
#define _LIGHTS

struct DirectionalLight
{
	D3DXVECTOR4 color;
	D3DXVECTOR3 direction;
};

struct Material
{
	float ambient, diffuse, specular, shininess;
};

#endif