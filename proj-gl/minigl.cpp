/**
 * minigl.cpp
 * -------------------------------
 * Implement miniGL here.
 *
 * You may include minigl.h and any of the standard C++ libraries.
 * No other includes are permitted.  Other preprocessing directives
 * are also not permitted.  These requirements are strictly
 * enforced.  Be sure to run a test grading to make sure your file
 * passes the sanity tests.
 *
 * The behavior of the routines your are implenting is documented here:
 * https://www.opengl.org/sdk/docs/man2/
 * Note that you will only be implementing a subset of this.  In particular,
 * you only need to implement enough to pass the tests in the suite.
 */

#include "minigl.h"
#include "vec.h"
#include "mat.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>
#include <cstdio>

using namespace std;

/**
 * Useful data types
 */
typedef mat<MGLfloat,4,4> mat4; //data structure storing a 4x4 matrix, see mat.h
typedef mat<MGLfloat,3,3> mat3; //data structure storing a 3x3 matrix, see mat.h
typedef vec<MGLfloat,4> vec4;   //data structure storing a 4 dimensional vector, see vec.h
typedef vec<MGLfloat,3> vec3;   //data structure storing a 3 dimensional vector, see vec.h
typedef vec<MGLfloat,2> vec2;   //data structure storing a 2 dimensional vector, see vec.h

struct vertex
{
	vec3 color;
	vec4 position;
};
struct triangle
{
	vertex a, b, c;
};
MGLpoly_mode drawMode;
MGLmatrix_mode matrixMode;
vector<vertex> listOfVertices;
vec3 currentColor;
vector<triangle> listOfTriangles;
mat4 currentMatrix;
/**
 * Standard macro to report errors
 */
inline void MGL_ERROR(const char* description) {
    printf("%s\n", description);
    exit(1);
}


/**
 * Read pixel data starting with the pixel at coordinates
 * (0, 0), up to (width,  height), into the array
 * pointed to by data.  The boundaries are lower-inclusive,
 * that is, a call with width = height = 1 would just read
 * the pixel at (0, 0).
 *
 * Rasterization and z-buffering should be performed when
 * this function is called, so that the data array is filled
 * with the actual pixel values that should be displayed on
 * the two-dimensional screen.
 */ 

float area(vec2 a, vec2 b, vec2 c)
{
	//area of abc = 0.5(axby−aybx+bxcy−bycx+cxay−cyax)
	float area = 0.5 * (a[0] * b[1] - a[1] * b[0] + b[0]*c[1] - b[1] * c[0] + c[0]*a[1] - c[1]*a[0]);
	return area
}
void mglReadPixels(MGLsize width,
                   MGLsize height,
                   MGLpixel *data)
{
	for (uint i = 0; i < listOfTriangles.size(); i++)
	{
		triangle t = listOfTriangles[i];
		//Convert 4D to 2D v1 = (x1/w1, y1,w1)
		vec2 v1 = {t.a.position[0] / t.a.position[3], t.a.position[1] / t.a.position[3]};
		vec2 v2 = {t.b.position[0] / t.b.position[3], t.b.position[1] / t.b.position[3]};
		vec2 v3 = {t.c.position[0] / t.c.position[3], t.c.position[1] / t.c.position[3]};
		//Convert 2D to Pixel Coordinates ((x+1) * w/2), (y+1)* (w/2))
		vec2 pixel1 = {floor((v1[0] + 1) * width/2), floor((v1[1] + 1) *height/2)}; //A
		vec2 pixel2 = {floor((v2[0] + 1) * width/2), floor((v2[1] + 1) *height/2)}; //B
		vec2 pixel3 = {floor((v3[0] + 1) * width/2), floor((v3[1] + 1) *height/2)}; //C
		//Get min max of bounding box
		int xmin = min(pixel3[0], min(pixel1[0], pixel2[0]));
		int ymin = min(pixel3[1], min(pixel1[1], pixel2[1]));
		int xmax = max(pixel3[0], max(pixel1[0], pixel2[0]));
		int ymax = max(pixel3[1], max(pixel1[1], pixel2[1]));
		//Check Barycentric Coordinates in Bounding Box
		//Total area of the triangle A
		if (xmin  < 0 || ymin < 0)
		{
			xmin = ymin = 0;
		}
		if (xmax > width)
		{
			xmax = width -1;
		}
		if (ymax > height)
		{
			ymax = height -1;
		}
		float area = 0.5 * (pixel1[0] * pixel2[1] - pixel1[1] * pixel2[0] + pixel2[0]*pixel3[1] - pixel2[1] * pixel3[0] + pixel3[0]*pixel1[1] - pixel3[1]*pixel1[0]);
		float alpha, beta, gamma;
		for (int i = xmin; i < xmax; i++)
		{	
			for (int j = ymin; j < ymax; j++)
			{
				//area of abc = 0.5(axby−aybx+bxcy−bycx+cxay−cyax)
				//alpha = area(pbc)/A
				//beta = area(pac)/A
				//gamma = area(pab)/A
				vec2 p = vec2(i,j);
				float subArea = 0.5 * (i * pixel2[1] - j * pixel2[0] + pixel2[0] * pixel3[1] - pixel2[1] * pixel3[0] + pixel3[0] * j - pixel3[1] * i);
				alpha = subArea / area;
				subArea = 0.5 * (i * pixel1[1] - j * pixel1[0] + pixel1[0] * pixel3[1] - pixel1[1] * pixel3[0] + pixel3[0] * j - pixel3[1] * i);
				beta = subArea / area;
				subArea = 0.5 * (i * pixel1[1] - j * pixel1[0] + pixel1[0] * pixel2[1] - pixel1[1] * pixel2[0] + pixel2[0] * j - pixel2[1] * i);
				gamma = subArea / area;
				//i min > 0 imax < width jmax < height
				if (alpha > 0 && beta > 0 && gamma > 0)
				{
					//The pixel is in the triangle draw pixel here
					*(data + i + j * width) = Make_Pixel(255,255,255);
				}
					
					
				//Write Pixel to Screen and color
			}
		}
		
		
	}
	
	
	
}

/**
 * Start specifying the vertices for a group of primitives,
 * whose type is specified by the given mode.
 */
void mglBegin(MGLpoly_mode mode)
{
	drawMode = mode;
}


/**
 * Stop specifying the vertices for a group of primitives.
 */
void mglEnd()
{
	triangle t;
	if (drawMode == MGL_TRIANGLES)
	{
		for (uint i = 0; i < listOfVertices.size(); i+=3)
		{
			t.a = listOfVertices[i];
			t.b = listOfVertices[i+1];
			t.c = listOfVertices[i+2];
			listOfTriangles.push_back(t);
		}
	}
	else if (drawMode == MGL_QUADS)
	{
		for (uint i = 0; i < listOfVertices.size(); i+=4)
		{
			t.a = listOfVertices[i];
			t.b = listOfVertices[i+1];
			t.c = listOfVertices[i+2];
			listOfTriangles.push_back(t);
			t.b = listOfVertices[i+2];
			t.c = listOfVertices[i+3];
			listOfTriangles.push_back(t);
		}
	}
	listOfVertices.clear();
	listOfVertices.shrink_to_fit();
}

/**
 * Specify a two-dimensional vertex; the x- and y-coordinates
 * are explicitly specified, while the z-coordinate is assumed
 * to be zero.  Must appear between calls to mglBegin() and
 * mglEnd().
 */
void mglVertex2(MGLfloat x,
                MGLfloat y)
{
	mglVertex3(x,y,0);
	
}

/**
 * Specify a three-dimensional vertex.  Must appear between
 * calls to mglBegin() and mglEnd().
 */
void mglVertex3(MGLfloat x,
                MGLfloat y,
                MGLfloat z)
{
	vertex v;
	v.color = currentColor;
	v.position = vec4(x,y,z,1);
	listOfVertices.push_back(v);
}

/**
 * Set the current matrix mode (modelview or projection).
 */
void mglMatrixMode(MGLmatrix_mode mode)
{
	matrixMode = mode;
}

/**
 * Push a copy of the current matrix onto the stack for the
 * current matrix mode.
 */
void mglPushMatrix()
{
}

/**
 * Pop the top matrix from the stack for the current matrix
 * mode.
 */
void mglPopMatrix()
{
}

/**
 * Replace the current matrix with the identity.
 */
void mglLoadIdentity()
{
}

/**
 * Replace the current matrix with an arbitrary 4x4 matrix,
 * specified in column-major order.  That is, the matrix
 * is stored as:
 *
 *   ( a0  a4  a8  a12 )
 *   ( a1  a5  a9  a13 )
 *   ( a2  a6  a10 a14 )
 *   ( a3  a7  a11 a15 )
 *
 * where ai is the i'th entry of the array.
 */
void mglLoadMatrix(const MGLfloat *matrix)
{
	
}

/**
 * Multiply the current matrix by an arbitrary 4x4 matrix,
 * specified in column-major order.  That is, the matrix
 * is stored as:
 *
 *   ( a0  a4  a8  a12 )
 *   ( a1  a5  a9  a13 )
 *   ( a2  a6  a10 a14 )
 *   ( a3  a7  a11 a15 )
 *
 * where ai is the i'th entry of the array.
 */
void mglMultMatrix(const MGLfloat *matrix)
{
}

/**
 * Multiply the current matrix by the translation matrix
 * for the translation vector given by (x, y, z).
 */
void mglTranslate(MGLfloat x,
                  MGLfloat y,
                  MGLfloat z)
{
}

/**
 * Multiply the current matrix by the rotation matrix
 * for a rotation of (angle) degrees about the vector
 * from the origin to the point (x, y, z).
 */
void mglRotate(MGLfloat angle,
               MGLfloat x,
               MGLfloat y,
               MGLfloat z)
{
}

/**
 * Multiply the current matrix by the scale matrix
 * for the given scale factors.
 */
void mglScale(MGLfloat x,
              MGLfloat y,
              MGLfloat z)
{
}

/**
 * Multiply the current matrix by the perspective matrix
 * with the given clipping plane coordinates.
 */
void mglFrustum(MGLfloat left,
                MGLfloat right,
                MGLfloat bottom,
                MGLfloat top,
                MGLfloat near,
                MGLfloat far)
{
}

/**
 * Multiply the current matrix by the orthographic matrix
 * with the given clipping plane coordinates.
 */
void mglOrtho(MGLfloat left,
              MGLfloat right,
              MGLfloat bottom,
              MGLfloat top,
              MGLfloat near,
              MGLfloat far)
{
	
}

/**
 * Set the current color for drawn shapes.
 */
void mglColor(MGLfloat red,
              MGLfloat green,
              MGLfloat blue)
{
	currentColor = vec3(red, green, blue);
}
