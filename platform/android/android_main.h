
/******************************************************************
 * Name		android_main.h 
 * Author	gaoguagnlei
 * Date		2012-3-22 
 * ****************************************************************
 *                          Description
 *
 ******************************************************************/
#ifndef ANDROID_MAIN_H
#define ANDROID_MAIN_H

#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#include <sys/time.h>

/*******************************************************************************************
 * Log
 ******************************************************************************************/
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "info", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "warn", __VA_ARGS__))

/*******************************************************************************************
 * string compare function
 ******************************************************************************************/

#define stricmp strcasecmp

/*******************************************************************************************
 * asset operations
 ******************************************************************************************/
#define ASSET_ITEM AAsset

#define ASSET_OPEN(path, mode) 	\
	open_asset(path, mode)

#define ASSET_READ(buffer, size, count, asset) \
	AAsset_read(asset, buffer, count*size)

#define ASSET_SEEK(asset, offset, where) \
	AAsset_seek(asset, offset, where)

#define ASSET_CLOSE(asset)\
	AAsset_close(asset);

#define ASSET_REWIND(asset)\
	AAsset_seek(asset, 0, SEEK_SET)

#define ASSET_LENGTH(asset)\
	AAsset_getLength(asset)

#define ASSET_TELL(asset)\
	AAsset_seek(asset, 0, SEEK_CUR)
//	AAsset_getRemainingLength(asset)

/*******************************************************************************************
 * open asset with asset manager
 ******************************************************************************************/
AAsset* open_asset(const char* path, const char* mode);

/*******************************************************************************************
 * render function
 ******************************************************************************************/
void opengl_init(int w, int h);
void opengl_display();
void opengl_end();
void opengl_touch(int x,int y, int state);

/***************/
double get_time();
/**************/
#endif
