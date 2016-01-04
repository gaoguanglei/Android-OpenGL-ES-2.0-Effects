/**
 * OpenGL 3.2 with GLEW and GLUT
 *
 * @author	Norbert Nopper norbert@nopper.tv
 * @version	1.0
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#pragma once

#include <platform.h>//gl2.h

/**
 * Class for loading a shader.
 */
class shader_loader
{

private:

	/**
	 * The content of the shader source file.
	 */
	char* m_content;

public:

	/**
	 * Constructor
	 */
	shader_loader(void);

	shader_loader(const char* filename):m_content(0)
	{
		load_shader_source(filename);
	}

	/**
	 * Destructor
	 */
	~shader_loader(void);


	/**
	 * Loads a given shader.
	 *
	 * @param filename the filename of the shader
	 *
	 * @return the content of the shader
	 */
	char* load_shader_source(const char* filename);


	const char* get_shader_source()const
	{
		return m_content;
	}

};
