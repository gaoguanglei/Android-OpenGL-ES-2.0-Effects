
/************************************************************************************************************
 * Added by gaoguanglei 2012-3-28
 ************************************************************************************************************/

#include <image.h>
#include <stdio.h>
#include <vector>
#include <math.h>

using std::vector;


// surface description flags
const unsigned long DDSF_CAPS           = 0x00000001l;
const unsigned long DDSF_HEIGHT         = 0x00000002l;
const unsigned long DDSF_WIDTH          = 0x00000004l;
const unsigned long DDSF_PITCH          = 0x00000008l;
const unsigned long DDSF_PIXELFORMAT    = 0x00001000l;
const unsigned long DDSF_MIPMAPCOUNT    = 0x00020000l;
const unsigned long DDSF_LINEARSIZE     = 0x00080000l;
const unsigned long DDSF_DEPTH          = 0x00800000l;

// pixel format flags
const unsigned long DDSF_ALPHAPIXELS    = 0x00000001l;
const unsigned long DDSF_FOURCC         = 0x00000004l;
const unsigned long DDSF_RGB            = 0x00000040l;
const unsigned long DDSF_RGBA           = 0x00000041l;

// dwCaps1 flags
const unsigned long DDSF_COMPLEX         = 0x00000008l;
const unsigned long DDSF_TEXTURE         = 0x00001000l;
const unsigned long DDSF_MIPMAP          = 0x00400000l;

// dwCaps2 flags
const unsigned long DDSF_CUBEMAP         = 0x00000200l;
const unsigned long DDSF_CUBEMAP_POSITIVEX  = 0x00000400l;
const unsigned long DDSF_CUBEMAP_NEGATIVEX  = 0x00000800l;
const unsigned long DDSF_CUBEMAP_POSITIVEY  = 0x00001000l;
const unsigned long DDSF_CUBEMAP_NEGATIVEY  = 0x00002000l;
const unsigned long DDSF_CUBEMAP_POSITIVEZ  = 0x00004000l;
const unsigned long DDSF_CUBEMAP_NEGATIVEZ  = 0x00008000l;
const unsigned long DDSF_CUBEMAP_ALL_FACES  = 0x0000FC00l;
const unsigned long DDSF_VOLUME          = 0x00200000l;

// compressed texture types
const unsigned long FOURCC_UNKNOWN       = 0;

#ifndef MAKEFOURCC
#define MAKEFOURCC(c0,c1,c2,c3) \
	((unsigned long)(unsigned char)(c0)| \
	((unsigned long)(unsigned char)(c1) << 8)| \
	((unsigned long)(unsigned char)(c2) << 16)| \
	((unsigned long)(unsigned char)(c3) << 24))
#endif

const unsigned long FOURCC_R8G8B8        = 20;
const unsigned long FOURCC_A8R8G8B8      = 21;
const unsigned long FOURCC_X8R8G8B8      = 22;
const unsigned long FOURCC_R5G6B5        = 23;
const unsigned long FOURCC_X1R5G5B5      = 24;
const unsigned long FOURCC_A1R5G5B5      = 25;
const unsigned long FOURCC_A4R4G4B4      = 26;
const unsigned long FOURCC_R3G3B2        = 27;
const unsigned long FOURCC_A8            = 28;
const unsigned long FOURCC_A8R3G3B2      = 29;
const unsigned long FOURCC_X4R4G4B4      = 30;
const unsigned long FOURCC_A2B10G10R10   = 31;
const unsigned long FOURCC_A8B8G8R8      = 32;
const unsigned long FOURCC_X8B8G8R8      = 33;
const unsigned long FOURCC_G16R16        = 34;
const unsigned long FOURCC_A2R10G10B10   = 35;
const unsigned long FOURCC_A16B16G16R16  = 36;

const unsigned long FOURCC_L8            = 50;
const unsigned long FOURCC_A8L8          = 51;
const unsigned long FOURCC_A4L4          = 52;
const unsigned long FOURCC_DXT1          = 0x31545844l; //(MAKEFOURCC('D','X','T','1'))
const unsigned long FOURCC_DXT2          = 0x32545844l; //(MAKEFOURCC('D','X','T','1'))
const unsigned long FOURCC_DXT3          = 0x33545844l; //(MAKEFOURCC('D','X','T','3'))
const unsigned long FOURCC_DXT4          = 0x34545844l; //(MAKEFOURCC('D','X','T','3'))
const unsigned long FOURCC_DXT5          = 0x35545844l; //(MAKEFOURCC('D','X','T','5'))
const unsigned long FOURCC_ATI1          = MAKEFOURCC('A','T','I','1');
const unsigned long FOURCC_ATI2          = MAKEFOURCC('A','T','I','2');

const unsigned long FOURCC_D16_LOCKABLE  = 70;
const unsigned long FOURCC_D32           = 71;
const unsigned long FOURCC_D24X8         = 77;
const unsigned long FOURCC_D16           = 80;

const unsigned long FOURCC_D32F_LOCKABLE = 82;

const unsigned long FOURCC_L16           = 81;

// Floating point surface formats

// s10e5 formats (16-bits per channel)
const unsigned long FOURCC_R16F          = 111;
const unsigned long FOURCC_G16R16F       = 112;
const unsigned long FOURCC_A16B16G16R16F = 113;

// IEEE s23e8 formats (32-bits per channel)
const unsigned long FOURCC_R32F          = 114;
const unsigned long FOURCC_G32R32F       = 115;
const unsigned long FOURCC_A32B32G32R32F = 116;

struct DXTColBlock
{
    GLushort col0;
    GLushort col1;

    GLubyte row[4];
};

struct DXT3AlphaBlock
{
    GLushort row[4];
};

struct DXT5AlphaBlock
{
    GLubyte alpha0;
    GLubyte alpha1;
    
    GLubyte row[6];
};

struct DDS_PIXELFORMAT
{
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwFourCC;
    unsigned long dwRGBBitCount;
    unsigned long dwRBitMask;
    unsigned long dwGBitMask;
    unsigned long dwBBitMask;
    unsigned long dwABitMask;
};

struct DDS_HEADER
{
    unsigned long dwSize;
    unsigned long dwFlags;
    unsigned long dwHeight;
    unsigned long dwWidth;
    unsigned long dwPitchOrLinearSize;
    unsigned long dwDepth;
    unsigned long dwMipMapCount;
    unsigned long dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    unsigned long dwCaps1;
    unsigned long dwCaps2;
    unsigned long dwReserved2[3];
};


inline float ushort2float(GLushort s)
{
	GLushort *i = &s;
	GLuint sign = (*i >> 15) & 0x1;
	GLuint exponent = ((*i >> 10) & 0x1f) - 0xf;
	GLuint mantissa = (*i) & 0x3ff;

	exponent += 0x7f;
	GLuint ret = ((sign & 0x1) << 31);
	ret |= (exponent & 0xff) << 23;
	ret |= (mantissa << 13) & 0x7fffff;

	return *((float*)&ret);
}


inline GLubyte half2ubyte(GLushort f16)
{
		GLushort *i = &f16;
		GLuint sign = (*i >> 15) & 0x1;
		GLuint exponent = ((*i >> 10) & 0x1f) - 0xf;
		GLuint mantissa = (*i) & 0x3ff;

		exponent += 0x7f;
		GLuint ret = ((sign & 0x1) << 31);
		ret |= (exponent & 0xff) << 23;
		ret |= (mantissa << 13) & 0x7fffff;
		
		float f = *((float*)&ret);

		f = f>1? 1:f;

		return(GLubyte)(f * 0xff);
		
} 


inline int min_int(int v, int minv)
{
	return v<minv? v:minv;
}

inline void ushort2rgba(unsigned char rgbe[4], GLushort srgbe[4])
{
	int red = ushort2float(srgbe[0]) * 0xff;
	int green = ushort2float(srgbe[1]) * 0xff;
	int blue = ushort2float(srgbe[2]) * 0xff;
	int alpha = ushort2float(srgbe[3]) * 0xff;

	rgbe[0] = min_int(red, 0xff);
	rgbe[1] = min_int(green, 0xff);
	rgbe[2] = min_int(blue, 0xff);;
	rgbe[3] = min_int(alpha, 0xff);
}

//
// flip a DXT1 color block
////////////////////////////////////////////////////////////
void flip_blocks_dxtc1(GLubyte *ptr, unsigned int numBlocks)
{
	DXTColBlock *curblock = (DXTColBlock*)ptr;
	GLubyte temp;

	for (unsigned int i = 0; i < numBlocks; i++) {
		temp = curblock->row[0];
		curblock->row[0] = curblock->row[3];
		curblock->row[3] = temp;
		temp = curblock->row[1];
		curblock->row[1] = curblock->row[2];
		curblock->row[2] = temp;

		curblock++;
	}
}

//
// flip a DXT3 color block
////////////////////////////////////////////////////////////
void flip_blocks_dxtc3(GLubyte *ptr, unsigned int numBlocks)
{
	DXTColBlock *curblock = (DXTColBlock*)ptr;
	DXT3AlphaBlock *alphablock;
	GLushort tempS;
	GLubyte tempB;

	for (unsigned int i = 0; i < numBlocks; i++)
	{
		alphablock = (DXT3AlphaBlock*)curblock;

		tempS = alphablock->row[0];
		alphablock->row[0] = alphablock->row[3];
		alphablock->row[3] = tempS;
		tempS = alphablock->row[1];
		alphablock->row[1] = alphablock->row[2];
		alphablock->row[2] = tempS;

		curblock++;

		tempB = curblock->row[0];
		curblock->row[0] = curblock->row[3];
		curblock->row[3] = tempB;
		tempB = curblock->row[1];
		curblock->row[1] = curblock->row[2];
		curblock->row[2] = tempB;

		curblock++;
	}
}

//
// flip a DXT5 alpha block
////////////////////////////////////////////////////////////
void flip_dxt5_alpha(DXT5AlphaBlock *block)
{
	GLubyte gBits[4][4];

	const unsigned long mask = 0x00000007;          // bits = 00 00 01 11
	unsigned long bits = 0;
	memcpy(&bits, &block->row[0], sizeof(unsigned char) * 3);

	gBits[0][0] = (GLubyte)(bits & mask);
	bits >>= 3;
	gBits[0][1] = (GLubyte)(bits & mask);
	bits >>= 3;
	gBits[0][2] = (GLubyte)(bits & mask);
	bits >>= 3;
	gBits[0][3] = (GLubyte)(bits & mask);
	bits >>= 3;
	gBits[1][0] = (GLubyte)(bits & mask);
	bits >>= 3;
	gBits[1][1] = (GLubyte)(bits & mask);
	bits >>= 3;
	gBits[1][2] = (GLubyte)(bits & mask);
	bits >>= 3;
	gBits[1][3] = (GLubyte)(bits & mask);

	bits = 0;
	memcpy(&bits, &block->row[3], sizeof(GLubyte) * 3);

	gBits[2][0] = (GLubyte)(bits & mask);
	bits >>= 3;
	gBits[2][1] = (GLubyte)(bits & mask);
	bits >>= 3;
	gBits[2][2] = (GLubyte)(bits & mask);
	bits >>= 3;
	gBits[2][3] = (GLubyte)(bits & mask);
	bits >>= 3;
	gBits[3][0] = (GLubyte)(bits & mask);
	bits >>= 3;
	gBits[3][1] = (GLubyte)(bits & mask);
	bits >>= 3;
	gBits[3][2] = (GLubyte)(bits & mask);
	bits >>= 3;
	gBits[3][3] = (GLubyte)(bits & mask);

	// clear existing alpha bits
	memset(block->row, 0, sizeof(GLubyte) * 6);

	unsigned long *pBits = ((unsigned long*) &(block->row[0]));

	*pBits = *pBits | (gBits[3][0] << 0);
	*pBits = *pBits | (gBits[3][1] << 3);
	*pBits = *pBits | (gBits[3][2] << 6);
	*pBits = *pBits | (gBits[3][3] << 9);

	*pBits = *pBits | (gBits[2][0] << 12);
	*pBits = *pBits | (gBits[2][1] << 15);
	*pBits = *pBits | (gBits[2][2] << 18);
	*pBits = *pBits | (gBits[2][3] << 21);

	pBits = ((unsigned long*) &(block->row[3]));

	*pBits = *pBits | (gBits[1][0] << 0);
	*pBits = *pBits | (gBits[1][1] << 3);
	*pBits = *pBits | (gBits[1][2] << 6);
	*pBits = *pBits | (gBits[1][3] << 9);

	*pBits = *pBits | (gBits[0][0] << 12);
	*pBits = *pBits | (gBits[0][1] << 15);
	*pBits = *pBits | (gBits[0][2] << 18);
	*pBits = *pBits | (gBits[0][3] << 21);
}

//
// flip a DXT5 color block
////////////////////////////////////////////////////////////
void flip_blocks_dxtc5(GLubyte *ptr, unsigned int numBlocks)
{
	DXTColBlock *curblock = (DXTColBlock*)ptr;
	DXT5AlphaBlock *alphablock;
	GLubyte temp;

	for (unsigned int i = 0; i < numBlocks; i++)
	{
		alphablock = (DXT5AlphaBlock*)curblock;

		flip_dxt5_alpha(alphablock);

		curblock++;

		temp = curblock->row[0];
		curblock->row[0] = curblock->row[3];
		curblock->row[3] = temp;
		temp = curblock->row[1];
		curblock->row[1] = curblock->row[2];
		curblock->row[2] = temp;

		curblock++;
	}
}

static void flipSurface(GLubyte *surf, int _elementSize, int width, int height, int depth,GLuint _format, bool compressed=false)
{
	unsigned int lineSize;


	if (!compressed) {
		lineSize = _elementSize * width;
		unsigned int sliceSize = lineSize * height;

		GLubyte *tempBuf = new GLubyte[lineSize];

		for ( int ii = 0; ii < depth; ii++) {
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
	else
	{
		void (*flipblocks)(GLubyte*, unsigned int);
		width = (width + 3) / 4;
		height = (height + 3) / 4;
		unsigned int blockSize = 0;

		switch (_format)
		{
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: 
			blockSize = 8;
			flipblocks = &flip_blocks_dxtc1; 
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: 
			blockSize = 16;
			flipblocks = &flip_blocks_dxtc3; 
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: 
			blockSize = 16;
			flipblocks = &flip_blocks_dxtc5; 
			break;
		default:
			return;
		}

		lineSize = width * blockSize;
		GLubyte *tempBuf = new GLubyte[lineSize];

		GLubyte *top = surf;
		GLubyte *bottom = surf + (height-1) * lineSize;

		for (unsigned int j = 0; j < max( (unsigned int)height >> 1, (unsigned int)1); j++)
		{
			if (top == bottom)
			{
				flipblocks(top, width);
				break;
			}

			flipblocks(top, width);
			flipblocks(bottom, width);

			memcpy( tempBuf, top, lineSize);
			memcpy( top, bottom, lineSize);
			memcpy( bottom, tempBuf, lineSize);

			top += lineSize;
			bottom -= lineSize;
		}
		delete []tempBuf;
	}
}    


////////////////////////////////////////////////////////////
bool image_tool::read_dds( const char *file, image_data& i) 
{

	LOGI("read dds");
    // open file
    ASSET_ITEM *fp = ASSET_OPEN(file, "rb");
    if (fp == NULL)
    {
	LOGI("fp == NULL");
	    return false;
    }
    // read in file marker, make sure its a DDS file
    char filecode[4];
    ASSET_READ(filecode, 1, 4, fp);
    if (strncmp(filecode, "DDS ", 4) != 0)
    {
	LOGI("not dds file");
        ASSET_CLOSE(fp);
        return false;
    }

    // read in DDS header
    DDS_HEADER ddsh;
    ASSET_READ(&ddsh, sizeof(DDS_HEADER), 1, fp);

    // check if image is a volume texture
    if ((ddsh.dwCaps2 & DDSF_VOLUME) && (ddsh.dwDepth > 0))
        i._depth = ddsh.dwDepth;
    else
        i._depth = 0;

    // There are flags that are supposed to mark these fields as valid, but some dds files don't set them properly
    i._width = ddsh.dwWidth;
    i._height = ddsh.dwHeight;
    
    if (ddsh.dwFlags & DDSF_MIPMAPCOUNT) {
        i._levelCount = ddsh.dwMipMapCount;
    }
    else
        i._levelCount = 1;

    //check cube-map faces
    if ( ddsh.dwCaps2 & DDSF_CUBEMAP) 
	{
        //this is a cubemap, count the faces
        i._faces = 0;
        i._faces += (ddsh.dwCaps2 & DDSF_CUBEMAP_POSITIVEX) ? 1 : 0;
        i._faces += (ddsh.dwCaps2 & DDSF_CUBEMAP_NEGATIVEX) ? 1 : 0;
        i._faces += (ddsh.dwCaps2 & DDSF_CUBEMAP_POSITIVEY) ? 1 : 0;
        i._faces += (ddsh.dwCaps2 & DDSF_CUBEMAP_NEGATIVEY) ? 1 : 0;
        i._faces += (ddsh.dwCaps2 & DDSF_CUBEMAP_POSITIVEZ) ? 1 : 0;
        i._faces += (ddsh.dwCaps2 & DDSF_CUBEMAP_NEGATIVEZ) ? 1 : 0;

        //check for a complete cubemap
        if ( (i._faces != 6) || (i._width != i._height) ) {
            ASSET_CLOSE(fp);
            return false;
        }
    }
    else 
	{
        //not a cubemap
        i._faces = 0;
    }

    bool btcCompressed = false;
    int bytesPerElement = 0;

    // figure out what the image format is
    if (ddsh.ddspf.dwFlags & DDSF_FOURCC) 
    {
        switch(ddsh.ddspf.dwFourCC)
        {
			//-------------------------------------------------------
			case FOURCC_DXT1:
				bytesPerElement = 8;
				btcCompressed = true;
				break;

			case FOURCC_DXT2:
			case FOURCC_DXT3:
				bytesPerElement = 16;
				btcCompressed = true;
				break;

			case FOURCC_DXT4:
			case FOURCC_DXT5:
				bytesPerElement = 16;
				btcCompressed = true;
				break;
			//-------------------------------------------------------

            case FOURCC_R8G8B8:
                bytesPerElement = 3;
                break;

            case FOURCC_A8R8G8B8:
                bytesPerElement = 4;
                break;

            case FOURCC_X8R8G8B8:
                bytesPerElement = 4;
                break;

            case FOURCC_R5G6B5:
                bytesPerElement = 2;
                break;

            case FOURCC_A8:
                bytesPerElement = 1;
                break;

            case FOURCC_A2B10G10R10:
                bytesPerElement = 4;
                break;

            case FOURCC_A8B8G8R8:
                bytesPerElement = 4;
                break;

            case FOURCC_X8B8G8R8:
                bytesPerElement = 4;
                break;

            case FOURCC_A2R10G10B10:
                bytesPerElement = 4;
                break;

            case FOURCC_A16B16G16R16:
                bytesPerElement = 8;
                break;

            case FOURCC_L8:
                bytesPerElement = 1;
                break;

            case FOURCC_A8L8:
                bytesPerElement = 2;
                break;

            case FOURCC_L16:
                bytesPerElement = 2;
                break;

            case FOURCC_R16F:
                bytesPerElement = 2;
                break;

            case FOURCC_A16B16G16R16F:
                bytesPerElement = 8;
                break;

            case FOURCC_R32F:
                bytesPerElement = 4;
                break;

            case FOURCC_A32B32G32R32F:
                bytesPerElement = 16;
                break;

            case FOURCC_UNKNOWN:
            case FOURCC_X1R5G5B5:
            case FOURCC_A1R5G5B5:
            case FOURCC_A4R4G4B4:
            case FOURCC_R3G3B2:
            case FOURCC_A8R3G3B2:
            case FOURCC_X4R4G4B4:
            case FOURCC_A4L4:
            case FOURCC_D16_LOCKABLE:
            case FOURCC_D32:
            case FOURCC_D24X8:
            case FOURCC_D16:
            case FOURCC_D32F_LOCKABLE:
            case FOURCC_G16R16:
            case FOURCC_G16R16F:
            case FOURCC_G32R32F:
                //these are unsupported for now
            default:
                ASSET_CLOSE(fp);
                return false;
        }
    }
    else if (ddsh.ddspf.dwFlags == DDSF_RGBA && ddsh.ddspf.dwRGBBitCount == 32)
    {
        bytesPerElement = 4;
    }
    else if (ddsh.ddspf.dwFlags == DDSF_RGB  && ddsh.ddspf.dwRGBBitCount == 32)
    {
        bytesPerElement = 4;
    }
    else if (ddsh.ddspf.dwFlags == DDSF_RGB  && ddsh.ddspf.dwRGBBitCount == 24)
    {
        bytesPerElement = 3;
    }
	else if (ddsh.ddspf.dwRGBBitCount == 8)
	{
		bytesPerElement = 1;
	}
    else 
    {
        ASSET_CLOSE(fp);
        return false;
    }

    i._elementSize = bytesPerElement;

    for (int face = 0; face < ((i._faces) ? i._faces : 1); face++) 
	{
        int w = i._width, h = i._height, d = (i._depth) ? i._depth : 1;
        for (int level = 0; level < i._levelCount; level++) 
		{
			int bw = (btcCompressed) ? (w+3)/4 : w;
			int bh = (btcCompressed) ? (h+3)/4 : h;

//            int bw = w;
  //          int bh = h;
            int size = bw*bh*d*bytesPerElement;

            GLubyte *data = new GLubyte[size];

            ASSET_READ( data, size, 1, fp);

            i._data.push_back(data);

            if (i._faces != 6)
                flipSurface( data, bytesPerElement, w, h, d, i._format, btcCompressed);

            //reduce mip sizes
            w = ( w > 1) ? w >> 1 : 1;
            h = ( h > 1) ? h >> 1 : 1;
            d = ( d > 1) ? d >> 1 : 1;
        }
    }
/*
    //reverse cube map y faces
    if (i._faces == 6) {
        for (int level = 0; level < i._levelCount; level++) {
            GLubyte *temp = i._data[2*i._levelCount + level];
            i._data[2*i._levelCount + level] = i._data[3*i._levelCount + level];
            i._data[3*i._levelCount + level] = temp;
        }
    }
  */  


	// figure out what the image format is
	if (ddsh.ddspf.dwFlags & DDSF_FOURCC) 
	{
		switch(ddsh.ddspf.dwFourCC)
		{
		//-------------------------------------------------------
		case FOURCC_DXT1:
			i._format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			i._internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			i._type = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			bytesPerElement = 8;
			btcCompressed = true;
			break;

		case FOURCC_DXT2:
		case FOURCC_DXT3:
			i._format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			i._internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			i._type = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			bytesPerElement = 16;
			btcCompressed = true;
			break;

		case FOURCC_DXT4:
		case FOURCC_DXT5:
			i._format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			i._internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			i._type = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			bytesPerElement = 16;
			btcCompressed = true;
			break;
		//-------------------------------------------------------
		case FOURCC_R8G8B8:
			i._format = GL_RGB;
			i._internalFormat = GL_RGB;
			i._type = GL_UNSIGNED_BYTE;

			for(int face=0; face<i._faces; face++)
			{
				int tw = i._width;
				int th = i._height;

				for(int level=0; level<i._levelCount; level++)
				{
					GLubyte* p = i._data[face*i._levelCount+level];

					for(int h=0; h<th; h++)
					{
						for(int w=0; w<tw; w++)
						{
							int idx = (h*i._width+w)*3+0;
							GLubyte temp = p[idx];
							p[idx] = p[idx+2];
							p[idx+2] = temp;
						}
					}
					tw = (tw>>1);
					th = (th>>1);
				}
			}

			break;

		case FOURCC_A8R8G8B8:
			i._format = GL_RGBA;
			i._internalFormat = GL_RGBA;
			i._type = GL_UNSIGNED_BYTE;
	

			for(int face=0; face<i._faces; face++)
			{

				int tw = i._width;
				int th = i._height;

				for(int level=0; level<i._levelCount; level++)
				{
					GLubyte* p = i._data[face*i._levelCount+level];
					for(int h=0; h<th; h++)
					{
						for(int w=0; w<tw; w++)
						{
							int idx = (h*tw+w)*4;
							GLubyte temp = p[idx];
							p[idx] = p[idx+2];
							p[idx+2] = temp;
						}
					}

					tw = (tw>>1);
					th = (th>>1);
				}
			}
			break;

		case FOURCC_X8R8G8B8:
			i._format = GL_RGBA;
			i._internalFormat = GL_RGBA;
			i._type = GL_UNSIGNED_BYTE;
			bytesPerElement = 4;

			for(int face=0; face<i._faces; face++)
			{
				int tw = i._width;
				int th = i._height;

				for(int level=0; level<i._levelCount; level++)
				{
					GLubyte* p = i._data[face*i._levelCount+level];
					for(int h=0; h<th; h++)
					{
						for(int w=0; w<tw; w++)
						{
							int idx = (h*tw+w)*4;
							GLubyte temp = p[idx];
							p[idx] = p[idx+2];
							p[idx+2] = temp;
						}
					}

					tw = (tw>>1);
					th = (th>>1);
				}
			}

			break;

		case FOURCC_R5G6B5:
			i._format = GL_RGB;
			i._internalFormat = GL_RGB;
			i._type = GL_UNSIGNED_SHORT_5_6_5;
			bytesPerElement = 2;

			for(int face=0; face<i._faces; face++)
			{
				int tw = i._width;
				int th = i._height;

				for(int level=0; level<i._levelCount; level++)
				{
					GLubyte* p = i._data[face*i._levelCount+level];
					for(int h=0; h<th; h++)
					{
						for(int w=0; w<tw; w++)
						{
							int idx = (h*tw+w)*2;
							int temp = p[idx+0];
							p[idx+0] = (p[idx+1]>>3) + p[idx+0]&0xf8;
							p[idx+1] = (p[idx+0]<<3) + p[idx+1]&0x07;
						}
					}

					tw = (tw>>1);
					th = (th>>1);
				}
			}

			break;

		case FOURCC_A8:
			i._format = GL_ALPHA;
			i._internalFormat = GL_ALPHA;
			i._type = GL_UNSIGNED_BYTE;
			bytesPerElement = 1;
			break;

		case FOURCC_A2B10G10R10:
			i._format = GL_RGBA;
			i._internalFormat = GL_RGB;
			i._type = GL_UNSIGNED_BYTE;
			bytesPerElement = 4;

			for(int face=0; face<i._faces; face++)
			{
				for(int level=0; level<i._levelCount; level++)
				{
					int tw = i._width;
					int th = i._height;

					GLubyte* p = i._data[face*i._levelCount+level];
					int* p2 = (int*)p;
					for(int h=0; h<th; h++)
					{
						for(int w=0; w<tw; w++)
						{
							int idx = h*tw+w;
							p[idx*4+0] = p2[idx]>>24;
							p[idx*4+1] = (p2[idx]>>14)&0xff;
							p[idx*4+2] = (p2[idx]>>4)&0xff;
							p[idx*4+3] = (p2[idx]&0x3)<<6;
						}
					}

					tw = (tw>>1);
					th = (th>>1);
				}
			}

			break;

		case FOURCC_A8B8G8R8:
			i._format = GL_RGBA;
			i._internalFormat = GL_RGBA;
			i._type = GL_UNSIGNED_BYTE;
			bytesPerElement = 4;
			break;

		case FOURCC_X8B8G8R8:
			i._format = GL_RGBA;
			i._internalFormat = GL_RGBA;
			i._type = GL_UNSIGNED_BYTE;
			bytesPerElement = 4;
			break;

		case FOURCC_A2R10G10B10:
			i._format = GL_RGBA;
			i._internalFormat = GL_RGBA;
			i._type = GL_UNSIGNED_BYTE;
			bytesPerElement = 4;

			for(int face=0; face<i._faces; face++)
			{
				for(int level=0; level<i._levelCount; level++)
				{
					int tw = i._width;
					int th = i._height;

					GLubyte* p = i._data[face*i._levelCount+level];
					int* p2 = (int*)p;
					for(int h=0; h<th; h++)
					{
						for(int w=0; w<tw; w++)
						{
							int idx = h*tw+w;
							p[idx*4+0] = min_int(p2[idx]>>24, 255);
							p[idx*4+1] = min_int((p2[idx]>>14)&0xff,255);
							p[idx*4+2] = min_int((p2[idx]>>4)&0xff,255);
							p[idx*4+3] = min_int((p2[idx]&0x3)<<6,255);
						}
					}

					tw = (tw>>1);
					th = (th>>1);
				}
			}

			break;

		case FOURCC_A16B16G16R16:
			i._format = GL_RGBA;
			i._internalFormat = GL_RGBA;
			i._type = GL_UNSIGNED_BYTE;
			bytesPerElement = 8;

			for(int face=0; face<i._faces; face++)
			{
				int tw = i._width;
				int th = i._height;

				for(int level=0; level<i._levelCount; level++)
				{
					GLubyte *ndata = new GLubyte[tw * th * 4];
					GLushort* odata = (GLushort*)(i._data[face*i._levelCount+level]);
					for(int h=0; h<th; h++)
					{
						for(int w=0; w<tw; w++)
						{
							int idx = h * tw + w;
							ndata[idx*4+0] = odata[idx*4+0]>>8; 
							ndata[idx*4+1] = odata[idx*4+1]>>8;
							ndata[idx*4+2] = odata[idx*4+2]>>8;
							ndata[idx*4+3] = odata[idx*4+3]>>8;
						}
					}

					i._data[face*i._levelCount+level] = ndata;
					delete []odata;
					
					tw = (tw>>1);
					th = (th>>1);
				}

			}


			break;

		case FOURCC_L8:
			i._format = GL_LUMINANCE;
			i._internalFormat = GL_LUMINANCE;
			i._type = GL_UNSIGNED_BYTE;
			bytesPerElement = 1;
			break;

		case FOURCC_A8L8:
			i._format = GL_LUMINANCE_ALPHA;
			i._internalFormat = GL_LUMINANCE_ALPHA;
			i._type = GL_UNSIGNED_BYTE;
			bytesPerElement = 2;
			break;

		case FOURCC_L16:
			i._format = GL_LUMINANCE;
			i._internalFormat = GL_LUMINANCE;
			i._type = GL_UNSIGNED_BYTE;
			bytesPerElement = 2;

			for(int face=0; face<i._faces; face++)
			{
				int tw = i._width;
				int th = i._height;

				for(int level=0; level<i._levelCount; level++)
				{
					GLubyte *ndata = new GLubyte[tw * th];
					GLushort* odata = (GLushort*)(i._data[face*i._levelCount+level]);
					for(int h=0; h<th; h++)
					{
						for(int w=0; w<tw; w++)
						{
							int idx = h * tw + w;
							ndata[idx] = odata[idx]>>8; 
						}
					}

					i._data[face*i._levelCount+level] = ndata;
					delete []odata;

					tw = (tw>>1);
					th = (th>>1);
				}
			}


			break;

		case FOURCC_R16F:
			i._format = GL_LUMINANCE; //should use red, once it is available
			i._internalFormat = GL_LUMINANCE; 
			i._type = GL_UNSIGNED_BYTE;
			bytesPerElement = 2;

			for(int face=0; face<i._faces; face++)
			{
				int tw = i._width;
				int th = i._height;

				for(int level=0; level<i._levelCount; level++)
				{
					GLubyte *ndata = new GLubyte[tw * th];
					GLushort* odata = (GLushort*)(i._data[face*i._levelCount+level]);
					for(int h=0; h<th; h++)
					{
						for(int w=0; w<tw; w++)
						{
							int idx = h * tw + w;
							ndata[idx] = half2ubyte(odata[idx]); 
						}
					}

					i._data[face*i._levelCount+level] = ndata;
					delete []odata;

					tw = (tw>>1);
					th = (th>>1);
				}

			}


			break;

		case FOURCC_A16B16G16R16F:
			i._format = GL_RGBA;
			i._internalFormat = GL_RGBA;
			i._type = GL_UNSIGNED_BYTE;
			bytesPerElement = 4;

			for(int face=0; face<i._faces; face++)
			{
				int tw = i._width;
				int th = i._height;

				for(int level=0; level<i._levelCount; level++)
				{
					GLubyte *ndata = new GLubyte[tw * th * 4];
					GLushort* odata = (GLushort*)(i._data[face*i._levelCount+level]);
					for(int h=0; h<th; h++)
					{
						for(int w=0; w<tw; w++)
						{
							int idx = h * tw + w;
							ushort2rgba(ndata+idx*4, odata+idx*4);
						}
					}

					i._data[face*i._levelCount+level] = ndata;
					delete []odata;

					tw = (tw>>1);
					th = (th>>1);
				}

			}

			break;

		case FOURCC_R32F:
			i._format = GL_LUMINANCE; //should use red, once it is available
			i._internalFormat = GL_LUMINANCE; 
			i._type = GL_FLOAT;
			bytesPerElement = 4;

			for(int face=0; face<i._faces; face++)
			{
				int tw = i._width;
				int th = i._height;

				for(int level=0; level<i._levelCount; level++)
				{
					GLubyte *ndata = new GLubyte[tw * th];
					GLushort* odata = (GLushort*)(i._data[face*i._levelCount+level]);
					for(int h=0; h<th; h++)
					{
						for(int w=0; w<tw; w++)
						{
							int idx = h * tw + w;
							ndata[idx] = odata[idx] * 0xff;
						}
					}

					i._data[face*i._levelCount+level] = ndata;
					delete []odata;

					tw = (tw>>1);
					th = (th>>1);
				}

			}

			break;

		case FOURCC_A32B32G32R32F:
			i._format = GL_RGBA;
			i._internalFormat = GL_RGBA;
			i._type = GL_UNSIGNED_BYTE;
			bytesPerElement = 16;

			for(int face=0; face<i._faces; face++)
			{
				int tw = i._width;
				int th = i._height;

				for(int level=0; level<i._levelCount; level++)
				{
					GLubyte *ndata = new GLubyte[tw * th * 4];
					GLushort* odata = (GLushort*)(i._data[face*i._levelCount+level]);
					for(int h=0; h<th; h++)
					{
						for(int w=0; w<tw; w++)
						{
							int idx = h * tw + w;
							ushort2rgba(ndata+idx*4, odata+idx*4);
						}
					}

					i._data[face*i._levelCount+level] = ndata;
					delete []odata;

					tw = (tw>>1);
					th = (th>>1);
				}


			}
			break;

		case FOURCC_UNKNOWN:
		case FOURCC_X1R5G5B5:
		case FOURCC_A1R5G5B5:
		case FOURCC_A4R4G4B4:
		case FOURCC_R3G3B2:
		case FOURCC_A8R3G3B2:
		case FOURCC_X4R4G4B4:
		case FOURCC_A4L4:
		case FOURCC_D16_LOCKABLE:
		case FOURCC_D32:
		case FOURCC_D24X8:
		case FOURCC_D16:
		case FOURCC_D32F_LOCKABLE:
		case FOURCC_G16R16:
		case FOURCC_G16R16F:
		case FOURCC_G32R32F:
			//these are unsupported for now
		default:
			ASSET_CLOSE(fp);
			return false;
		}
	}
	else if (ddsh.ddspf.dwFlags == DDSF_RGBA && ddsh.ddspf.dwRGBBitCount == 32)
	{
		i._format = GL_RGBA;
		i._internalFormat = GL_RGBA;
		i._type = GL_UNSIGNED_BYTE;
		bytesPerElement = 4;

		for(int face=0; face<i._faces; face++)
		{
			int tw = i._width;
			int th = i._height;

			for(int level=0; level<i._levelCount; level++)
			{
				GLubyte* p = i._data[face*i._levelCount+level];
				for(int h=0; h<th; h++)
				{
					for(int w=0; w<tw; w++)
					{
						int idx = (h*tw+w)*4;
						GLubyte temp = p[idx];
						p[idx] = p[idx+2];
						p[idx+2] = temp;
					}
				}

				tw = (tw>>1);
				th = (th>>1);
			}
		}

	}
	else if (ddsh.ddspf.dwFlags == DDSF_RGB  && ddsh.ddspf.dwRGBBitCount == 32)
	{
		i._format = GL_RGBA;
		i._internalFormat = GL_RGBA;
		i._type = GL_UNSIGNED_BYTE;
		bytesPerElement = 4;

		for(int face=0; face<i._faces; face++)
		{
			int tw = i._width;
			int th = i._height;

			for(int level=0; level<i._levelCount; level++)
			{
				GLubyte* p = i._data[face*i._levelCount+level];
				for(int h=0; h<th; h++)
				{
					for(int w=0; w<tw; w++)
					{
						int idx = (h*tw+w)*4;
						GLubyte temp = p[idx];
						p[idx] = p[idx+2];
						p[idx+2] = temp;
					}
				}

				tw = (tw>>1);
				th = (th>>1);
			}
		}
	}
	else if (ddsh.ddspf.dwFlags == DDSF_RGB  && ddsh.ddspf.dwRGBBitCount == 24)
	{
		i._format = GL_RGB;
		i._internalFormat = GL_RGB;
		i._type = GL_UNSIGNED_BYTE;
		bytesPerElement = 3;
		for(int face=0; face<i._faces; face++)
		{
			int tw = i._width;
			int th = i._height;

			for(int level=0; level<i._levelCount; level++)
			{
				GLubyte* p = i._data[face*i._levelCount+level];
				for(int h=0; h<th; h++)
				{
					for(int w=0; w<tw; w++)
					{
						int idx = (h*tw+w)*3;
						GLubyte temp = p[idx];
						p[idx] = p[idx+2];
						p[idx+2] = temp;
					}
				}

				tw = (tw>>1);
				th = (th>>1);
			}
		}
	}
	else if (ddsh.ddspf.dwRGBBitCount == 8)
	{
		i._format = GL_LUMINANCE; 
		i._internalFormat = GL_LUMINANCE; 
		i._type = GL_UNSIGNED_BYTE;
		bytesPerElement = 1;
	}
	else 
	{
		return false;
	}

    return true;
}
