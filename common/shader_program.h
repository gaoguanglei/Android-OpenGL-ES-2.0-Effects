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

/**
 * Class for compiling and linking a shader.
 */
class shader_program
{

private:

	/**
	 * Vertex shader.
	 */
	GLuint m_vertex;

	/**
	 * Fragment shader.
	 */
	GLuint m_fragment;

	/**
	 * Shader program.
	 */
	GLuint m_program;

public:

	/**
	 * Constructor
	 */
	shader_program(void);

	/**
	 * Destructor
	 */
	~shader_program(void);

	/**
	 * Builds the given shader.
	 *
	 * @param vertexSource the source of a vertex shader
	 * @param fragmentSource the source of a frgament shader
	 */
	GLboolean build(const char* vertexSource, const char* fragmentSource);

	/**
	 * Deletes all shaders and the program.
	 */
	void delete_all(void);

	/**
	 * Returns the created program.
	 *
	 * @return the program
	 */
	GLuint get_program(void);

};
