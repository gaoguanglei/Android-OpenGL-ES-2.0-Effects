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

#include "shader_program.h"

#include <stdio.h>

shader_program::shader_program(void) :
m_vertex(0),
m_fragment(0),
m_program(0)
{
}

shader_program::~shader_program(void)
{
	// Delete everything if not already done.
	delete_all();
}

GLboolean shader_program::build(const char* vertexSource, const char* fragmentSource)
{
	GLint compiled;
    GLint linked;

	if (!vertexSource || !fragmentSource)
	{
		return GL_FALSE;
	}

	// http://www.opengl.org/sdk/docs/man/xhtml/glCreateShader.xml
	m_vertex = glCreateShader(GL_VERTEX_SHADER);

	// http://www.opengl.org/sdk/docs/man/xhtml/glShaderSource.xml
	glShaderSource(m_vertex, 1, &vertexSource, 0);

	// http://www.opengl.org/sdk/docs/man/xhtml/glCompileShader.xml
	glCompileShader(m_vertex);

	// http://www.opengl.org/sdk/docs/man/xhtml/glGetShader.xml
    glGetShaderiv(m_vertex, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		GLint logLength, charsWritten;

		// http://www.opengl.org/sdk/docs/man/xhtml/glGetShader.xml
		glGetShaderiv(m_vertex, GL_INFO_LOG_LENGTH, &logLength);

		char* log = new char[logLength];

		// http://www.opengl.org/sdk/docs/man/xhtml/glGetShaderInfoLog.xml
        glGetShaderInfoLog(m_vertex, logLength, &charsWritten, log);

		LOGI("Vertex shader compile error:\n");
		LOGI("%s\n", log);

		delete[] log;

		return GL_FALSE;
	}

	// http://www.opengl.org/sdk/docs/man/xhtml/glCreateShader.xml
	m_fragment = glCreateShader(GL_FRAGMENT_SHADER);

	// http://www.opengl.org/sdk/docs/man/xhtml/glShaderSource.xml
	glShaderSource(m_fragment, 1, &fragmentSource, 0);

	// http://www.opengl.org/sdk/docs/man/xhtml/glCompileShader.xml
	glCompileShader(m_fragment);

	// http://www.opengl.org/sdk/docs/man/xhtml/glGetShader.xml
    glGetShaderiv(m_fragment, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		GLint logLength, charsWritten;

		// http://www.opengl.org/sdk/docs/man/xhtml/glGetShader.xml
		glGetShaderiv(m_fragment, GL_INFO_LOG_LENGTH, &logLength);

		char* log = new char[logLength];

		// http://www.opengl.org/sdk/docs/man/xhtml/glGetShaderInfoLog.xml
        glGetShaderInfoLog(m_fragment, logLength, &charsWritten, log);

		LOGI("Fragment shader compile error:\n");
		LOGI("%s\n", log);

		delete[] log;

		return GL_FALSE;
	}

	// http://www.opengl.org/sdk/docs/man/xhtml/glCreateProgram.xml
	m_program = glCreateProgram();

	// http://www.opengl.org/sdk/docs/man/xhtml/glAttachShader.xml
    glAttachShader(m_program, m_vertex);

	// http://www.opengl.org/sdk/docs/man/xhtml/glAttachShader.xml
    glAttachShader(m_program, m_fragment);

	// http://www.opengl.org/sdk/docs/man/xhtml/glLinkProgram.xml
    glLinkProgram(m_program);

	// http://www.opengl.org/sdk/docs/man/xhtml/glGetProgram.xml
	glGetProgramiv(m_program, GL_LINK_STATUS, &linked);

	if (!linked)
	{
		GLint logLength, charsWritten;

		// http://www.opengl.org/sdk/docs/man/xhtml/glGetProgram.xml
		glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLength);

		char* log = new char[logLength];

		// http://www.opengl.org/sdk/docs/man/xhtml/glGetProgramInfoLog.xml
		glGetProgramInfoLog(m_program, logLength, &charsWritten, log);

		LOGI("Shader program link error:\n");
		LOGI("%s\n", log);
		delete[] log;

		return GL_FALSE;
	}

	return GL_TRUE;
}

void shader_program::delete_all(void)
{
	if (m_program)
	{
		// http://www.opengl.org/sdk/docs/man/xhtml/glDeleteProgram.xml
		glDeleteProgram(m_program);

		m_program = 0;
	}

	if (m_fragment)
	{
		// http://www.opengl.org/sdk/docs/man/xhtml/glDeleteShader.xml
		glDeleteShader(m_fragment);

		m_fragment = 0;
	}

	if (m_vertex)
	{
		// http://www.opengl.org/sdk/docs/man/xhtml/glDeleteShader.xml
		glDeleteShader(m_vertex);

		m_vertex = 0;
	}
}

GLuint shader_program::get_program(void)
{
	return m_program;
}
