#include <image.h>
//#include <stdio.h>


static void flipSurface(GLubyte *surf, int element_size, int width, int height, int depth)
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
}

/***************************************************************************************************
 * load tga
****************************************************************************************************/
GLvoid swap_color_channel(GLuint width, GLuint height, GLenum format, GLubyte* pixelData)
{
	GLubyte temp;
	GLubyte bytesPerPixel = 3;
	
	if (format == GL_RGBA)
	{
		bytesPerPixel = 4;
	}
	
	// swap the R and B values to get RGB since the bitmap color format is in BGR
	for (GLuint i = 0; i < width*height*bytesPerPixel; i+=bytesPerPixel)
	{
		temp = pixelData[i];
		pixelData[i] = pixelData[i+2];
		pixelData[i+2] = temp;
	}
}



bool image_tool::read_tga(const char *filename, image_data& i)
//GLboolean Image::load_tga(const char* filename, GLushort& width, GLushort& height, GLenum& format, GLubyte** pixelData)
{
	GLushort width;
	GLushort height;
	GLenum format;
	GLubyte* pixelData;

	ASSET_ITEM*				file;
	GLubyte				type;
	GLubyte				bitsPerPixel;				

	// check, if we have a valid pointer
	if (!filename)
	{
		LOGI(">>>>>>>1");
		return GL_FALSE;
	}

	// open filename in "read binary" mode
	file = ASSET_OPEN(filename, "rb");

 	// seek through the tga header, up to the type:
 	ASSET_SEEK(file, 2, SEEK_CUR);

	if(file == NULL)
	{
		return GL_FALSE;
	}
 	// read the type
 	ASSET_READ(&type, 1, 1, file);

	// check the type	
	if (type != 2 && type != 10 && type != 11 && type !=3)
	{
		LOGI(">>>>>>>2");
		ASSET_CLOSE(file);
		return GL_FALSE;
	}

 	// seek through the tga header, up to the width/height:
 	ASSET_SEEK(file, 9, SEEK_CUR);
	
 	// read the width
 	ASSET_READ(&width, 2, 1, file);
 
 	// read the height 
 	ASSET_READ(&height, 2, 1, file);

	// read the bits per pixel
	ASSET_READ(&bitsPerPixel, 1, 1, file);

	LOGI("bitsPerPixel=%d", bitsPerPixel);

	// check the pixel depth
	if (bitsPerPixel != 8 && bitsPerPixel != 24 && bitsPerPixel != 32)
	{
		LOGI(">>>>>>>3");
		ASSET_CLOSE(file);
		return GL_FALSE;
	}
	else
	{	
		format = GL_LUMINANCE;	
		if (bitsPerPixel == 24)
		{
			format = GL_RGB;
		}
		else if (bitsPerPixel == 32)
		{
			format = GL_RGBA;
		}
	}

	// move file pointer to beginning of targa  data
//	ASSET_SEEK(file, 1, SEEK_CUR);
	GLubyte flipFlag;
	ASSET_READ(&flipFlag, 1, 1, file);

	// allocate enough memory for the targa  data
	pixelData = new GLubyte[width*height*bitsPerPixel/8];

	// verify memory allocation
	if (!pixelData)
	{
		LOGI(">>>>>>>4");
		ASSET_CLOSE(file);
		return GL_FALSE;
	}

	int temp;
	if (type == 2 || type == 3)
	{
		// read in the raw data
		temp = ASSET_READ(pixelData, 1, width*height*bitsPerPixel/8, file);
	}
	else
	{
		// RLE encoded
		GLuint pixelsRead = 0;

		while (pixelsRead < (GLuint)width*height)
		{
			GLubyte amount;

			ASSET_READ(&amount, 1, 1, file);

			if (amount & 0x80)
			{
				amount&=0x7F;

				amount++;

				// read in the rle data
				ASSET_READ(&(pixelData)[pixelsRead*bitsPerPixel/8], 1, bitsPerPixel/8, file);

				for (GLuint i = 1; i < amount; i++)
				{
					for (GLuint k = 0; k < (GLuint)bitsPerPixel/8; k++)
					{
						(pixelData)[(pixelsRead+i)*bitsPerPixel/8+k] = (pixelData)[pixelsRead*bitsPerPixel/8+k];
					}
				}
			}
			else
			{
				amount&=0x7F;

				amount++;
				
				// read in the raw data
				ASSET_READ(&(pixelData)[pixelsRead*bitsPerPixel/8], 1, (GLuint)amount*bitsPerPixel/8, file);				
			}

			pixelsRead += amount;
		}
	}


	// swap the color if necessary
	if (bitsPerPixel == 24 || bitsPerPixel == 32)
	{
		swap_color_channel(width, height, format, pixelData);
	}

	if(flipFlag == 32)
	{
		flipSurface(pixelData, bitsPerPixel/8, width, height, 0);
	}
	// close the file
	ASSET_CLOSE(file);

	
	i._data.push_back(pixelData);

	i._width = width;
	i._height = height;
	i._format = format;
	i._internalFormat = format;
	i._type = GL_UNSIGNED_BYTE;
	i._faces = 1;
	i._levelCount = 1;
	i._depth = bitsPerPixel/8;
	i._elementSize = bitsPerPixel/8;
	
	LOGI("w=%d",i._width);
	LOGI("h=%d",i._height);

	return GL_TRUE;
}
