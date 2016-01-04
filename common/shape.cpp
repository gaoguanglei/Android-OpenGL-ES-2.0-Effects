//
// Source partly taken from:
//    OpenGL(R) ES 2.0 Programming Guide
//	  http://www.opengles-book.com
//

#include <shape.h>

#include <string.h>
#include <math.h>
#define PIf		3.1415927410125732f

GLuint shape::create_plane(GLfloat halfExtend, GLfloat **vertices, GLfloat **normals, GLfloat **texCoords, GLushort **indices, GLuint& numberVertices)
{
	GLuint numVertices = 4;
	GLuint numIndices = 6;

	numberVertices = numVertices;

	GLfloat xy_vertices[] =
	{
		-1.0f, -1.0f, 0.0f,
		+1.0f, -1.0f, 0.0f, 
		-1.0f, +1.0f, 0.0f, 
		+1.0f, +1.0f, 0.0f
	};

	GLfloat xy_normals[] =
	{
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f
	};

	GLfloat xy_texCoords[] =
	{
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f
	};

	GLushort xy_indices[] =
	{
		0, 1, 2,
		1, 3, 2
	};

	if (vertices)
	{
		*vertices = new GLfloat[4*numVertices];

		memcpy( *vertices, xy_vertices, sizeof( xy_vertices ) );

		for (GLuint i = 0; i < numVertices; i++)
		{
			(*vertices)[i*4+0] *= halfExtend;
			(*vertices)[i*4+1] *= halfExtend;
		}
	}

	if (normals)
	{
		*normals = new GLfloat[3*numVertices];

		memcpy( *normals, xy_normals, sizeof( xy_normals ) );
	}

	if (texCoords)
	{
		*texCoords = new GLfloat[2*numVertices];

		memcpy( *texCoords, xy_texCoords, sizeof( xy_texCoords ) );
	}

	if (indices)
	{
		*indices = new GLushort[numIndices];

		memcpy( *indices, xy_indices, sizeof( xy_indices ) );
	}

	return numIndices;
}
	
GLuint shape::create_cube( GLfloat halfExtend, GLfloat **vertices, GLfloat **normals, GLfloat **texCoords, GLushort **indices, GLuint& numberVertices)
{
	
	GLuint numVertices = 24;
	GLuint numIndices = 36;

	numberVertices = numVertices;

	GLfloat cubeVertices[] =
	{
	  -1.0f, -1.0f, -1.0f,
	  -1.0f, -1.0f, +1.0f,
	  +1.0f, -1.0f, +1.0f,
	  +1.0f, -1.0f, -1.0f,
	  -1.0f, +1.0f, -1.0f,
	  -1.0f, +1.0f, +1.0f,
	  +1.0f, +1.0f, +1.0f,
	  +1.0f, +1.0f, -1.0f,
	  -1.0f, -1.0f, -1.0f,
	  -1.0f, +1.0f, -1.0f,
	  +1.0f, +1.0f, -1.0f,
	  +1.0f, -1.0f, -1.0f,
	  -1.0f, -1.0f, +1.0f,
	  -1.0f, +1.0f, +1.0f,
	  +1.0f, +1.0f, +1.0f,
	  +1.0f, -1.0f, +1.0f, 
	  -1.0f, -1.0f, -1.0f,
	  -1.0f, -1.0f, +1.0f,
	  -1.0f, +1.0f, +1.0f,
	  -1.0f, +1.0f, -1.0f, 
	  +1.0f, -1.0f, -1.0f,
	  +1.0f, -1.0f, +1.0f,
	  +1.0f, +1.0f, +1.0f,
	  +1.0f, +1.0f, -1.0f
	};

	GLfloat cubeNormals[] =
	{
	   0.0f, -1.0f,  0.0f,
	   0.0f, -1.0f,  0.0f,
	   0.0f, -1.0f,  0.0f,
	   0.0f, -1.0f,  0.0f,
	   0.0f, +1.0f,  0.0f,
	   0.0f, +1.0f,  0.0f,
	   0.0f, +1.0f,  0.0f,
	   0.0f, +1.0f,  0.0f,
	   0.0f,  0.0f, -1.0f,
	   0.0f,  0.0f, -1.0f,
	   0.0f,  0.0f, -1.0f,
	   0.0f,  0.0f, -1.0f,
	   0.0f,  0.0f, +1.0f,
	   0.0f,  0.0f, +1.0f,
	   0.0f,  0.0f, +1.0f,
	   0.0f,  0.0f, +1.0f,
	  -1.0f,  0.0f,  0.0f,
	  -1.0f,  0.0f,  0.0f,
	  -1.0f,  0.0f,  0.0f,
	  -1.0f,  0.0f,  0.0f,
	  +1.0f,  0.0f,  0.0f,
	  +1.0f,  0.0f,  0.0f,
	  +1.0f,  0.0f,  0.0f,
	  +1.0f,  0.0f,  0.0f
	};

	GLfloat cubeTexCoords[] =
	{
	  0.0f, 0.0f,
	  0.0f, 1.0f,
	  1.0f, 1.0f,
	  1.0f, 0.0f,
	  1.0f, 0.0f,
	  1.0f, 1.0f,
	  0.0f, 1.0f,
	  0.0f, 0.0f,
	  0.0f, 0.0f,
	  0.0f, 1.0f,
	  1.0f, 1.0f,
	  1.0f, 0.0f,
	  0.0f, 0.0f,
	  0.0f, 1.0f,
	  1.0f, 1.0f,
	  1.0f, 0.0f,
	  0.0f, 0.0f,
	  0.0f, 1.0f,
	  1.0f, 1.0f,
	  1.0f, 0.0f,
	  0.0f, 0.0f,
	  0.0f, 1.0f,
	  1.0f, 1.0f,
	  1.0f, 0.0f,
	};
   
	GLushort cubeIndices[] =
	{
	 0, 2, 1,
	 0, 3, 2, 
	 4, 5, 6,
	 4, 6, 7,
	 8, 9, 10,
	 8, 10, 11, 
	 12, 15, 14,
	 12, 14, 13, 
	 16, 17, 18,
	 16, 18, 19, 
	 20, 23, 22,
	 20, 22, 21
	};


	if (vertices)
	{
		*vertices = new GLfloat[3*numVertices];

		memcpy( *vertices, cubeVertices, sizeof( cubeVertices ) );

		for (GLuint i = 0; i < numVertices; i++)
		{
			(*vertices)[i*3+0] *= halfExtend;
			(*vertices)[i*3+1] *= halfExtend;
			(*vertices)[i*3+2] *= halfExtend;
		}
	}

	if (normals)
	{
		*normals = new GLfloat[3*numVertices];

		memcpy( *normals, cubeNormals, sizeof( cubeNormals ) );
	}

	if (texCoords)
	{
		*texCoords = new GLfloat[2*numVertices];

		memcpy( *texCoords, cubeTexCoords, sizeof( cubeTexCoords ) ) ;
	}


	if (indices)
	{
		*indices = new GLushort[numIndices];

		memcpy( *indices, cubeIndices, sizeof( cubeIndices ) );
	}

	return numIndices;
}

GLuint shape::create_sphere(GLuint numSlices, GLfloat radius, GLfloat **vertices, GLfloat **normals, GLfloat **texCoords, GLushort **indices, GLuint& numberVertices)
{
	GLuint numParallels = numSlices;
	GLuint numVertices = ( numParallels + 1 ) * ( numSlices + 1 );
	GLuint numIndices = numParallels * numSlices * 6;

//	GLfloat angleStep = (2.0f * PIf) / ((GLfloat) numSlices);
	GLfloat parallelStep = PIf / ((GLfloat) numSlices); 
	GLfloat sliceStep = parallelStep * 2.f;

	numberVertices = numVertices;

	if (vertices)
	{
		*vertices = new GLfloat[3*numVertices];
	}

	if (normals)
	{
		*normals = new GLfloat[3*numVertices];
	}

	if (texCoords)
	{
		*texCoords = new GLfloat[2*numVertices];
	}

	if (indices)
	{
		*indices = new GLushort[numIndices];
	}

	for (GLuint i = 0; i < numParallels + 1; i++ )
	{
		for (GLuint j = 0; j < numSlices + 1; j++ )
		{
			if ( vertices )
			{
				GLuint vertexIndex = ( i * (numSlices + 1) + j ) * 3; 

				(*vertices)[vertexIndex + 0] = radius * sinf ( parallelStep * (GLfloat)i ) * sinf ( sliceStep * (GLfloat)j );
				(*vertices)[vertexIndex + 1] = radius * cosf ( parallelStep * (GLfloat)i );
				(*vertices)[vertexIndex + 2] = radius * sinf ( parallelStep * (GLfloat)i ) * cosf ( sliceStep * (GLfloat)j );
			}

			if ( normals )
			{
				GLuint vertexIndex = ( i * (numSlices + 1) + j ) * 3; 
				GLuint normalIndex = ( i * (numSlices + 1) + j ) * 3; 

				(*normals)[normalIndex + 0] = (*vertices)[vertexIndex + 0] / radius;
				(*normals)[normalIndex + 1] = (*vertices)[vertexIndex + 1] / radius;
				(*normals)[normalIndex + 2] = (*vertices)[vertexIndex + 2] / radius;
			}

			if ( texCoords )
			{
				GLuint texCoordsIndex = ( i * (numSlices + 1) + j ) * 2;

				(*texCoords)[texCoordsIndex + 0] = (GLfloat) j / (GLfloat) numSlices;
				(*texCoords)[texCoordsIndex + 1] = ( 1.0f - (GLfloat) i ) / (GLfloat) (numParallels - 1 );
			}
		}
	}

	if (indices)
	{
		GLushort* indexBuf = (*indices);
		for (GLuint i = 0; i < numParallels ; i++ ) 
		{
			for (GLuint j = 0; j < numSlices; j++ )
			{
				*indexBuf++  = i * ( numSlices + 1 ) + j;
				*indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + j;
				*indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + ( j + 1 );

				*indexBuf++ = i * ( numSlices + 1 ) + j;
				*indexBuf++ = ( i + 1 ) * ( numSlices + 1 ) + ( j + 1 );
				*indexBuf++ = i * ( numSlices + 1 ) + ( j + 1 );
			}
		}
	}

	return numIndices;
}
