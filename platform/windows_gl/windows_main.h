
/******************************************************************
 * Name		windows_main.h 
 * Author	gaoguagnlei
 * Date		2012-3-26 
 * ****************************************************************
 *                          Description
 *
 ******************************************************************/


#pragma once

#include <windows.h>		// Header File For Windows
#include <gl/glew.h>


#define LOGI printf
#define LOGW printf

/**********************************************************************/
#define ASSET_ITEM FILE

#define ASSET_OPEN(path, mode) 	\
	fopen(path, mode)

#define ASSET_READ(buffer, size, count, asset) \
	fread(buffer, size, count, asset)

#define ASSET_SEEK(asset, offset, where) \
	fseek(asset, offset, where)

#define ASSET_CLOSE(asset)\
	fclose(asset);

#define ASSET_REWIND(asset)\
	rewind(asset)


#define ASSET_TELL(asset)\
	ftell(asset)

/**********************************************************************/
double get_time();
void opengl_init(int w, int h);
void opengl_display();
void opengl_end();
void opengl_touch(int x,int y, int state);
