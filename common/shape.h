/**
 * OpenGL 3.2 with GLEW and GLUT
 *
 * @author	Norbert Nopper norbert@nopper.tv
 * @version	1.0
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 *
 * Modified by gaoguanglei 2012-3-28
 */

#pragma once

#include <platform.h>

class shape
{

public:

	static GLuint create_plane(GLfloat halfExtend, GLfloat **vertices, GLfloat **normals, GLfloat **texCoords, GLushort **indices, GLuint& numberVertices);
		
	static GLuint create_cube(GLfloat halfExtend, GLfloat **vertices, GLfloat **normals, GLfloat **texCoords, GLushort **indices, GLuint& numberVertices);

	static GLuint create_sphere(GLuint numSlices, GLfloat radius, GLfloat **vertices, GLfloat **normals, GLfloat **texCoords, GLushort **indices, GLuint& numberVertices);

};
