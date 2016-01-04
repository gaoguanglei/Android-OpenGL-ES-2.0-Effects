/************************************************************************************************************
 * Added by gaoguanglei 2012-3-28
 ************************************************************************************************************/
#ifndef WINDOWS_MAIN_H
#define WINDOWS_MAIN_H

#include <windows.h>
#include <windowsx.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdio.h>

#define LOGI printf
#define LOGW printf

double get_time();

void opengl_init(int w, int h);
void opengl_display();
void opengl_end();
void opengl_touch(int x,int y, int state);

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

#endif
