/************************************************************************************************************
 * Added by gaoguanglei 2012-3-28
 ************************************************************************************************************/
#include <windows_main.h>


class esContext
{
public:
    esContext() :
        nativeDisplay(0), nativeWin(0),
        eglDisplay(0), eglSurface(0), eglContext(0), 
        nWindowWidth(0), nWindowHeight(0)
    {}

    ~esContext() {}

    EGLNativeDisplayType nativeDisplay;
    EGLNativeWindowType nativeWin;
    EGLDisplay eglDisplay;
    EGLSurface eglSurface;
    EGLContext eglContext;

    int         nWindowWidth;
    int         nWindowHeight;

};

esContext ctx;


/************************************************************************************************************
 * windows
************************************************************************************************************/
static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	unsigned int key = 0;
	// Handle relevant messages individually
	switch(uMsg)
	{
	case WM_ACTIVATE:
	case WM_SETFOCUS:
		return 0;
	case WM_SIZE:
//		OnNativeWinResize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			return 0;
		default:
			break;
		}
		break;
	case WM_MOUSEMOVE:
		opengl_touch(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 1);
		break;
	case WM_LBUTTONDOWN:
		opengl_touch(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
		break;
	case WM_LBUTTONUP:
		opengl_touch(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 2);
		break;
	case WM_MOUSEWHEEL:
		if(GET_Y_LPARAM(wParam)>0)
		{
			opengl_touch(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 5);
		}
		else if(GET_Y_LPARAM(wParam)<0)
		{
			opengl_touch(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 6);
		}
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

bool OpenNativeDisplay(EGLNativeDisplayType* nativedisp_out)
{
	*nativedisp_out = (EGLNativeDisplayType) NULL;
	return true;
}

void CloseNativeDisplay(EGLNativeDisplayType nativedisp)
{
}

bool CreateNativeWin(EGLNativeDisplayType nativedisp, int width, int height, int visid, EGLNativeWindowType* nativewin_out)
{
	bool result = true;
	HINSTANCE hInstance = GetModuleHandle(NULL);
	HWND hWnd = NULL;
	DWORD dwExtStyle;
	DWORD dwWindStyle;

	TCHAR szWindowName[50] =  TEXT("OpenGL ES Sample");
	TCHAR szClassName[50]  =  TEXT("OGL_CLASS");

	// setup window class
	WNDCLASS wndClass;
	wndClass.lpszClassName = szClassName;                // Set the name of the Class
	wndClass.lpfnWndProc   = (WNDPROC)WndProc;
	wndClass.hInstance     = hInstance;              // Use this module for the module handle
	wndClass.hCursor       = LoadCursor(NULL, IDC_ARROW);// Pick the default mouse cursor
	wndClass.hIcon         = LoadIcon(NULL, IDI_WINLOGO);// Pick the default windows icons
	wndClass.hbrBackground = NULL;                       // No Background
	wndClass.lpszMenuName  = NULL;                       // No menu for this window
	wndClass.style         = CS_HREDRAW | CS_OWNDC |     // set styles for this class, specifically to catch
		CS_VREDRAW;               // window redraws, unique DC, and resize
	wndClass.cbClsExtra    = 0;                          // Extra class memory
	wndClass.cbWndExtra    = 0;                          // Extra window memory

	// Register the newly defined class
	if(!RegisterClass( &wndClass ))
	{
		result = false;
	}

	if(result)
	{
		dwExtStyle  = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwWindStyle = WS_OVERLAPPEDWINDOW;
		ShowCursor(TRUE);

		RECT windowRect;
		windowRect.left   = 0;
		windowRect.right  = width;
		windowRect.top    = 0;
		windowRect.bottom = height;

		// Setup window width and height
		AdjustWindowRectEx(&windowRect, dwWindStyle, FALSE, dwExtStyle);

		//Adjust for adornments
		int nWindowWidth  = windowRect.right  - windowRect.left;
		int nWindowHeight = windowRect.bottom - windowRect.top;

		// Create window
		hWnd = CreateWindowEx(
			dwExtStyle,      // Extended style
			szClassName,     // class name
			szWindowName,    // window name
			dwWindStyle |
			WS_CLIPSIBLINGS |
			WS_CLIPCHILDREN, // window stlye
			0,               // window position, x
			0,               // window position, y
			nWindowWidth,    // height
			nWindowHeight,   // width
			NULL,            // Parent window
			NULL,            // menu
			hInstance,       // instance
			NULL);           // pass this to WM_CREATE

		ShowWindow(hWnd, SW_SHOWDEFAULT);
	}
	*nativewin_out = (EGLNativeWindowType) hWnd;
	return result;
}

void DestroyNativeWin(EGLNativeDisplayType nativedisp, EGLNativeWindowType nativewin)
{
	WINDOWINFO info;
	GetWindowInfo((HWND) nativewin, &info);
	DestroyWindow((HWND) nativewin);
	UnregisterClass((LPCTSTR) info.atomWindowType, GetModuleHandle(NULL));
}

bool UpdateNativeWin(EGLNativeDisplayType nativedisp, EGLNativeWindowType nativewin)
{
	bool result = true;
	// Peek or wait for messages
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message==WM_QUIT)
		{
			result = false;
		}
		else
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
	}
	return result;
}

/************************************************************************************************************
 * egl
/************************************************************************************************************/


EGLBoolean Setup(esContext &ctx)
{
    EGLBoolean bsuccess;

    // create native window
    EGLNativeDisplayType nativeDisplay;
    if(!OpenNativeDisplay(&nativeDisplay))
    {
        printf("Could not get open native display\n");
        return GL_FALSE;
    }

    // get egl display handle
    EGLDisplay eglDisplay;
    eglDisplay = eglGetDisplay(nativeDisplay);
    if(eglDisplay == EGL_NO_DISPLAY)
    {
        printf("Could not get EGL display\n");
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }
    ctx.eglDisplay = eglDisplay;

    // Initialize the display
    EGLint major = 0;
    EGLint minor = 0;
    bsuccess = eglInitialize(eglDisplay, &major, &minor);
    if (!bsuccess)
    {
        printf("Could not initialize EGL display\n");
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }
/*    if (major < 1 || minor < 3)
    {
        // Does not support EGL 1.4
        printf("System does not support at least EGL 1.4\n");
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }*/

    // Obtain the first configuration with a depth buffer
    EGLint attrs[] = { EGL_DEPTH_SIZE, 16, EGL_NONE };
    EGLint numConfig =0;
    EGLConfig eglConfig = 0;
    bsuccess = eglChooseConfig(eglDisplay, attrs, &eglConfig, 1, &numConfig);
    if (!bsuccess)
    {
        printf("Could not find valid EGL config\n");
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    // Get the native visual id
    int nativeVid;
    if (!eglGetConfigAttrib(eglDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, &nativeVid))
    {
        printf("Could not get native visual id\n");
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    EGLNativeWindowType nativeWin;
    if(!CreateNativeWin(nativeDisplay, 640, 480, nativeVid, &nativeWin))
    {
        printf("Could not create window\n");
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    // Create a surface for the main window
    EGLSurface eglSurface;
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWin, NULL);
    if (eglSurface == EGL_NO_SURFACE)
    {
        printf("Could not create EGL surface\n");
        DestroyNativeWin(nativeDisplay, nativeWin);
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }
    ctx.eglSurface = eglSurface;

    // Create an OpenGL ES context
    EGLContext eglContext;
    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, NULL);
    if (eglContext == EGL_NO_CONTEXT)
    {
        printf("Could not create EGL context\n");
        DestroyNativeWin(nativeDisplay, nativeWin);
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    // Make the context and surface current
    bsuccess = eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
    if(!bsuccess)
    {
        printf("Could not activate EGL context\n");
        DestroyNativeWin(nativeDisplay, nativeWin);
        CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    ctx.nativeDisplay = nativeDisplay;
    ctx.nativeWin = nativeWin;
	ctx.eglContext = eglContext;
    return GL_TRUE;
}

static double         f;

void ResetTimer() 
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	f = (double) freq.QuadPart;
}

double get_time()
{
	LARGE_INTEGER s;
	double d;
	QueryPerformanceCounter(&s);
	d = (double)(s.QuadPart) / f;

	return d;
}
int main(int argc, char** argv)
{
	ResetTimer();
    ctx.nWindowWidth  = 640;
    ctx.nWindowHeight = 480;
    int lRet = 0;

    // create window and setup egl
    if(Setup(ctx) == GL_FALSE)
    {
        return lRet;
    }

	opengl_init(ctx.nWindowWidth, ctx.nWindowHeight);
    // main loop
    while (UpdateNativeWin(ctx.nativeDisplay, ctx.nativeWin))
    {
        // render the model
		opengl_display();
		eglSwapBuffers(ctx.eglDisplay, ctx.eglSurface);
    }

	opengl_end();

    eglMakeCurrent(EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(ctx.eglDisplay, ctx.eglContext);
    eglDestroySurface(ctx.eglDisplay, ctx.eglSurface);
    eglTerminate(ctx.eglDisplay);
    DestroyNativeWin(ctx.nativeDisplay, ctx.nativeWin);
    CloseNativeDisplay(ctx.nativeDisplay);

    return lRet;
}
