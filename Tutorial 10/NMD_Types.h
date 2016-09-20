#ifndef _NMD_TYPES
#define _NMD_TYPES

//basic tuple containers
struct int2 
{ 
	int x,y; 
	int2():x(0),y(0){}
	int2( const int a, const int b ):x(a),y(b){}
};
struct int3 
{ 
	int x,y,z; 
	int3():x(0),y(0),z(0){}
	int3( const int a, const int b, const int c ):x(a),y(b),z(c){}
};
struct int4 
{ 
	int x,y,z,w; 
	int4():x(0),y(0),z(0),w(0){}
	int4( const int a, const int b, const int c, const int d ):x(a),y(b),z(c),w(d){}
};

struct uint2 
{ 
	unsigned int x,y; 
	uint2():x(0),y(0){}
	uint2( const unsigned int a, const unsigned int b ):x(a),y(b){}
};
struct uint3 
{ 
	unsigned int x,y,z; 
	uint3():x(0),y(0),z(0){}
	uint3( const unsigned int a, const unsigned int b, const unsigned int c ):x(a),y(b),z(c){}
};
struct uint4 
{ 
	unsigned int x,y,z,w; 
	uint4():x(0),y(0),z(0),w(0){}
	uint4( const unsigned int a, const unsigned int b, const unsigned int c, const unsigned int d ):x(a),y(b),z(c),w(d){}
};


//float
struct float2 
{	
	float x,y;
	float2():x(0),y(0){}
	float2( const float a, const float b):x(a),y(b){}
};
struct float3 
{ 
	float x,y,z; 
	float3():x(0),y(0),z(0){}	
	float3( const float a, const float b, const float c ):x(a),y(b),z(c){}
	float3( const float2& a, const float b ):x(a.x), y(a.y), z(b){}
};
struct float4 
{ 
	float x,y,z,w; 
	float4():x(0),y(0),z(0),w(0){}
	float4( const float a, const float b, const float c, const float d ):x(a),y(b),z(c),w(d){}
};

//double
struct double2 
{ 
	double x,y; 
	double2():x(0),y(0){}
	double2( const double a, const double b ):x(a),y(b){}
};
struct double3 
{ 
	double x,y,z;
	double3():x(0),y(0),z(0){}
	double3( const double a, const double b, const double c ):x(a),y(b),z(c){}
};
struct double4 
{ 
	double x,y,z,w; 
	double4():x(0),y(0),z(0),w(0){}
	double4( const double a, const double b, const double c, const double d ):x(a),y(b),z(c),w(d){}
};

//matrix
struct float4x4
{
	float m[16];
	float4x4(){ for ( unsigned int i=0; i < 16; i++ ) m[i] = 0; }
	float4x4( const float* pData ){ for ( unsigned int i=0; i < 16; i++ ) m[i] = pData[i]; }
	float* GetMatrix() { return m; }
};

#endif;