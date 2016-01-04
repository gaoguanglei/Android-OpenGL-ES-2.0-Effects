/******************************************************************************
*(C) Copyright 2007 Marvell International Ltd.
* All Rights Reserved 
*
* Modified by gaoguanglei 2012-05-16
******************************************************************************/

#include <linux_main.h>


#define WINDOW_WIDTH        1280
#define WINDOW_HEIGHT       720

// Global Variables
NativeWindowType    nativeWin = NULL;
EGLDisplay          eglDisplay;
EGLConfig           eglConfig;
EGLContext          eglContext;
EGLSurface          eglWindowSurface;



//*****************************************************************************
// Name:            AppInit
// Description:     Application initialization 
//*****************************************************************************
bool AppInit()
{
    EGLint      numConfigs;
    EGLint      majorVersion;
    EGLint      minorVersion;
    EGLConfig   config;
    EGLint      attrib_list[] = {   EGL_RED_SIZE,           5,
                                    EGL_GREEN_SIZE,         6,
                                    EGL_BLUE_SIZE,          5,
                                    EGL_DEPTH_SIZE,         16,
                                    EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
                                    EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
                                    EGL_NONE,
                                };

    EGLint      ctxAttribList[] = { EGL_CONTEXT_CLIENT_VERSION, 2, 
                                    EGL_NONE,
                                  };
                                  
    GLint       bShaderCompiled = 0;
    GLint       bLinked = 0;                                  
                                
    // Init native window
    nativeWin = fbCreateWindow(EGL_DEFAULT_DISPLAY, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);    
    if(!nativeWin)
    {
        printf("fbCreateWindow failed ! \n");
        return false;
    } 

    // Get EGL display
    eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(eglDisplay == EGL_NO_DISPLAY || eglGetError() != EGL_SUCCESS)
    {
        return false;
    }

    // Initialize EGL
    eglInitialize(eglDisplay, &majorVersion, &minorVersion);
    if(eglGetError() != EGL_SUCCESS)
    {
        return false;
    }

    // Choose EGL config
    if(!eglChooseConfig(eglDisplay, attrib_list, &config, 1, &numConfigs))
    {
        return false;
    }

    // Create EGL rendering context
    eglContext = eglCreateContext(eglDisplay, config, NULL, ctxAttribList);
    if(eglContext == EGL_NO_CONTEXT || eglGetError() != EGL_SUCCESS)
    {
        return false;
    }

    // Create EGL window surface
    eglWindowSurface = eglCreateWindowSurface(eglDisplay, config, nativeWin, NULL);
    if(eglWindowSurface == EGL_NO_SURFACE || eglGetError() != EGL_SUCCESS)
    {
        return false;
    }

	// Attach the EGL rendering context to EGL surfaces
    eglMakeCurrent(eglDisplay, eglWindowSurface, eglWindowSurface, eglContext);
    if(eglGetError() != EGL_SUCCESS)
    {
        return false;
    }       

    // Check vendor, renderer and version
    printf("Vendor   : %s\n", glGetString(GL_VENDOR));
    printf("Renderer : %s\n", glGetString(GL_RENDERER));
    printf("Version  : %s\n", glGetString(GL_VERSION));

    opengl_init(WINDOW_WIDTH, WINDOW_HEIGHT);

    return true;
}



//*****************************************************************************
// Name:            AppDeInit
// Description:     Application release 
//*****************************************************************************
void AppDeInit( void )
{
	opengl_end();
    // Destroy all EGL resources    
    eglMakeCurrent(eglDisplay, NULL, NULL, NULL);
    eglDestroyContext(eglDisplay, eglContext);
    eglDestroySurface(eglDisplay, eglWindowSurface);
    eglTerminate(eglDisplay);
    
    if(nativeWin)
    {
        fbDestroyWindow(nativeWin);
    }
    
}


double get_time()
{
	timeval current;
	gettimeofday(&current, NULL);
	double secs = current.tv_sec + current.tv_usec*0.000001;
	return secs;
} 


//*****************************************************************************
// Name:            main
// Description:     Application entry 
//*****************************************************************************
int main(int argc, char*argv[])
{
	int count = 1000;

	if(argc ==2 )
	{
		sscanf(argv[1], "%d", &count);
		LOGI("count=%d\n", count);
	}

	double start =get_time();
    // Application initialization
	if( !AppInit() )
	{
	return -1;
	}

	int FPS = 0;

	// Render 1000 frames
	while(count--)
	{	
		FPS++;
	
		double current = get_time()-start;
		if(current > 1.0)
		{
			LOGI("FPS=%d\n", FPS);
			FPS = 0;
			start = get_time();
		}
		opengl_display();
		eglSwapBuffers( eglDisplay, eglWindowSurface );
	}    

	// Application release
	AppDeInit();
	LOGI("Application end.\n");

	return 0;
}

// EOF








