
/************************************************************************************************************
 * Added by gaoguanglei 2012-3-28
 ************************************************************************************************************/
#pragma once

#include <platform.h>//only need gl2.h
#include <vector>
#include <string.h>


#ifndef GL_COMPRESSED_RGBA_S3TC_DXT1_EXT 
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#endif

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT3_EXT 
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#endif

#ifndef GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#endif


using namespace std;
struct image_data
{
	int _width;
	int _height;
	int _depth;
	int _levelCount;
	int _faces;
	GLenum _format;
	GLenum _internalFormat;
	GLenum _type;
	int _elementSize;

	//pointers to the levels
	std::vector<GLubyte*> _data;

	~image_data()
	{
		for (vector<GLubyte*>::iterator it = _data.begin(); it != _data.end(); it++) 
		{
			delete []*it;
		}
		_data.clear();
	}
	
};



class image_tool
{
/*	static void flipSurface(GLubyte *surf, int element_size, int width, int height, int depth)
	{
		unsigned int lineSize;

		depth = (depth) ? depth : 1;

		lineSize = element_size * width;
		unsigned int sliceSize = lineSize * height;

		GLubyte *tempBuf = new GLubyte[lineSize];

		for ( int ii = 0; ii < depth; ii++) 
		{
			GLubyte *top = surf + ii*sliceSize;
			GLubyte *bottom = top + (sliceSize - lineSize);

			for ( int jj = 0; jj < (height >> 1); jj++) {
				memcpy( tempBuf, top, lineSize);
				memcpy( top, bottom, lineSize);
				memcpy( bottom, tempBuf, lineSize);

				top += lineSize;
				bottom -= lineSize;
			}
		}
		delete []tempBuf;
	}*/

	

public:

	static bool read_dds( const char *file_name, image_data& i);
	static bool read_tga( const char *file_name, image_data& i);
	static bool read_image(const char *file_name, image_data& i)
	{
		int len = strlen(file_name);

		if(stricmp(file_name+len-3, "DDS")==0)
		{
			return read_dds(file_name, i);
		
		}
		if(stricmp(file_name+len-3, "tga")==0)
		{
			return read_tga(file_name,i);
		}
		return false;
	}
};
