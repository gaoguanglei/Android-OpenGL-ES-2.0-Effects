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

#include "shader_loader.h"

#include <stdio.h>
#include <string.h>

shader_loader::shader_loader(void) :
m_content(0)
{
}

shader_loader::~shader_loader(void)
{
	if (m_content)
	{
		delete[] m_content;
		m_content = 0;
	}
}

char* shader_loader::load_shader_source(const char* filename)
{
	ASSET_ITEM* f;
	GLint size;

	LOGI("load %s", filename);

	if (!filename)
	{	
		LOGI("filename is empty");
		return 0;
	}

	// http://www.cplusplus.com/reference/clibrary/cstdio/ASSET_OPEN.html
	f = ASSET_OPEN(filename, "r");

	if (!f)
	{
		LOGI("%s is not exist", filename);
		return 0;
	}

	if (m_content)
	{
		delete[] m_content;
		m_content = 0;
	}

	// http://www.cplusplus.com/reference/clibrary/cstdio/ASSET_SEEK.html

	unsigned ret =0;


	ASSET_SEEK(f, 0, SEEK_END);

	// http://www.cplusplus.com/reference/clibrary/cstdio/ASSET_LENGTH.html

	size = ASSET_TELL(f);

	m_content = new char[size+1];

	// http://www.cplusplus.com/reference/clibrary/cstring/memset.html
	memset(m_content, 0, size+1);

	// http://www.cplusplus.com/reference/clibrary/cstdio/ASSET_REWIND.html
	ASSET_REWIND(f);

	// http://www.cplusplus.com/reference/clibrary/cstdio/ASSET_READ.html
	ASSET_READ(m_content, 1, size, f);

	// http://www.cplusplus.com/reference/clibrary/cstdio/ASSET_CLOSE.html
	ASSET_CLOSE(f);

	//LOGI(m_content);

	return m_content;
}

