
/******************************************************************
 * Name		android_main.cpp 
 * Author	gaoguagnlei
 * Date		2012-3-22 
 * ****************************************************************
 *                          Description
 *
 ******************************************************************/

#include <android_main.h>

/******************************************************************
 * Our saved state data.
 *****************************************************************/
struct saved_state 
{
	int32_t x;
	int32_t y;
};



/********************************************************************
 * Shared state for our app.
 *******************************************************************/
struct engine 
{
	struct android_app* app;

	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;
	
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int32_t width;
	int32_t height;
	struct saved_state state;
};

/********************************************************************
 * Asset operation. 
 ********************************************************************/
static AAssetManager* mgr = NULL;

AAsset* open_asset(const char* path, const char* mode)
{
	if(mgr != NULL)
	{
		AAsset* asset = AAssetManager_open(mgr, path, AASSET_MODE_UNKNOWN);
		return asset;
	}
	return NULL;
}

/********************************************************************
 * Initialize an EGL context for the current display.
 *******************************************************************/
static int engine_init_display(struct engine* engine);


/********************************************************************
 * Just the current frame in the display.
 *******************************************************************/
static void engine_draw_frame(struct engine* engine);


/********************************************************************
 * Tear down the EGL context currently associated with 
 * the display.
 *******************************************************************/
static void engine_term_display(struct engine* engine);


/********************************************************************
 * Process the next input event.
 *******************************************************************/
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) 
{
	if(app==NULL)
		return 0;
	struct engine* engine = (struct engine*)app->userData;
	
	if(engine == NULL || event == NULL)
		return 0;

	if (AInputEvent_getType(event) != AINPUT_EVENT_TYPE_MOTION)
		return 0;

	switch(AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK)
	{
	
		case AMOTION_EVENT_ACTION_DOWN:
		{
		engine->state.x = (((int)AMotionEvent_getX(event, 1))<<16) | (int)AMotionEvent_getX(event, 0);
		engine->state.y = (((int)AMotionEvent_getY(event, 1))<<16) | (int)AMotionEvent_getY(event, 0);
		opengl_touch(engine->state.x, engine->state.y, 0);

		int32_t idx = (AMotionEvent_getAction(event)& AMOTION_EVENT_ACTION_POINTER_INDEX_MASK);
		LOGI("AMOTION_EVENT_ACTION_DOWN,idx=%d",idx>>8);
		}
		return 1;

		case AMOTION_EVENT_ACTION_UP:
		{
		engine->state.x = (((int)AMotionEvent_getX(event, 1))<<16) | (int)AMotionEvent_getX(event, 0);
		engine->state.y = (((int)AMotionEvent_getY(event, 1))<<16) | (int)AMotionEvent_getY(event, 0);
		opengl_touch(engine->state.x, engine->state.y, 2);
	
		int32_t idx = (AMotionEvent_getAction(event)& AMOTION_EVENT_ACTION_POINTER_INDEX_MASK);
		LOGI("AMOTION_EVENT_ACTION_UP,idx=%d",idx>>8);
		}
		return 1;

		case AMOTION_EVENT_ACTION_MOVE:
		engine->state.x = (((int)AMotionEvent_getX(event, 1))<<16) | (int)AMotionEvent_getX(event, 0);
		engine->state.y = (((int)AMotionEvent_getY(event, 1))<<16) | (int)AMotionEvent_getY(event, 0);
		opengl_touch(engine->state.x, engine->state.y, 1);
		return 1;
	
		case AMOTION_EVENT_ACTION_POINTER_DOWN:
		{
		engine->state.x = (((int)AMotionEvent_getX(event, 1))<<16) | (int)AMotionEvent_getX(event, 0);
		engine->state.y = (((int)AMotionEvent_getY(event, 1))<<16) | (int)AMotionEvent_getY(event, 0);
		int32_t idx = (AMotionEvent_getAction(event)& AMOTION_EVENT_ACTION_POINTER_INDEX_MASK);
		opengl_touch(engine->state.x, engine->state.y, 3);
		LOGI("AMOTION_EVENT_ACTION_POINTER_DOWN,idx=%d",idx>>8);
		}
		return 1;

		case AMOTION_EVENT_ACTION_POINTER_UP:
		{		
		engine->state.x = (((int)AMotionEvent_getX(event, 1))<<16) | (int)AMotionEvent_getX(event, 0);
		engine->state.y = (((int)AMotionEvent_getY(event, 1))<<16) | (int)AMotionEvent_getY(event, 0);
		int32_t idx = (AMotionEvent_getAction(event)& AMOTION_EVENT_ACTION_POINTER_INDEX_MASK);
		opengl_touch(engine->state.x, engine->state.y, 4);
		LOGI("AMOTION_EVENT_ACTION_POINTER_UP,idx=%d",idx>>8);
		}		
		return 1;
	
		default:
		break;
	}
/*
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) 
	{
		engine->state.x = AMotionEvent_getX(event, 0);
		engine->state.y = AMotionEvent_getY(event, 0);
		opengl_touch(engine->state.x, engine->state.y, 0);
		return 1;
	}*/
	return 0;
}


/***********************************************************************************
 * Process the next main command.
 **********************************************************************************/
static void engine_handle_cmd(struct android_app* app, int32_t cmd) 
{

	struct engine* engine = (struct engine*)app->userData;
	switch (cmd) 
	{
		case APP_CMD_SAVE_STATE:
			// The system has asked us to save our current state.  Do so.
			engine->app->savedState = malloc(sizeof(struct saved_state));
			*((struct saved_state*)engine->app->savedState) = engine->state;
			engine->app->savedStateSize = sizeof(struct saved_state);
			break;
		case APP_CMD_INIT_WINDOW:
			// The window is being shown, get it ready.
			
			
			LOGI("APP_CMD_INIT_WINDOW 11");
			if (engine->app->window != NULL) 
			{

					engine_init_display(engine);
	
				//engine_draw_frame(engine);
			}
			break;
		case APP_CMD_TERM_WINDOW:
			LOGI("APP_CMD_TERM_WINDOW 11");
			// The window is being hidden or closed, clean it up.
			engine_term_display(engine);
			break;
		case APP_CMD_GAINED_FOCUS:
			// When our app gains focus, we start monitoring the accelerometer.
			if (engine->accelerometerSensor != NULL) 
			{
				ASensorEventQueue_enableSensor(engine->sensorEventQueue,
						engine->accelerometerSensor);
				// We'd like to get 60 events per second (in us).
				ASensorEventQueue_setEventRate(engine->sensorEventQueue,
						engine->accelerometerSensor, (1000L/60)*1000);
			}
			break;
		case APP_CMD_LOST_FOCUS:
			// When our app loses focus, we stop monitoring the accelerometer.
			// This is to avoid consuming battery while not being used.
			if (engine->accelerometerSensor != NULL) 
			{
				ASensorEventQueue_disableSensor(engine->sensorEventQueue,
						engine->accelerometerSensor);
			}
			//engine_draw_frame(engine);
			break;
	}
}

/********************************************************************
 * Test for asset
 *******************************************************************/
void test_apk(struct android_app* state)
{
	AAssetManager* mgr = state->activity->assetManager;
	AAsset* asset = AAssetManager_open(mgr, 
					"fuck.txt", AASSET_MODE_UNKNOWN);
	
	if(asset == NULL)
	{
		LOGI("assets == NULL");
	}
	
	off_t size = AAsset_getLength(asset);
	LOGI("size=%d",size);
	
	char* buffer = new char[size+1];
	buffer[size] = 0;
	
	int num_read = AAsset_read(asset, buffer, size);
	//LOGI(buffer);
	
	AAsset_close(asset);
}

/********************************************************************
 * This is the main entry point of a native application 
 *that is using android_native_app_glue.  It runs in 
 *its own thread, with its own event loop for receiving 
 *input events and doing other things.
 *******************************************************************/
void android_main(struct android_app* state) 
{

//	mgr = state->activity->assetManager;

	struct engine engine;

	engine.display = NULL;

	// Make sure glue isn't stripped.
	app_dummy();

	memset(&engine, 0, sizeof(engine));
	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	engine.app = state;

	// Prepare to monitor accelerometer
	engine.sensorManager = ASensorManager_getInstance();
	engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
			ASENSOR_TYPE_ACCELEROMETER);
	engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
			state->looper, LOOPER_ID_USER, NULL, NULL);

	if (state->savedState != NULL) 
	{
		// We are starting with a previous saved state; restore from it.
		engine.state = *(struct saved_state*)state->savedState;
	}

	// loop waiting for stuff to do.

	while (1) 
	{
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;
	
		while ((ident=ALooper_pollAll(0, NULL, &events,
				(void**)&source)) >= 0) 
		{
			// Process this event.
			if (source != NULL) 
			{
				source->process(state, source);
			}

			// If a sensor has data, process it now.
			if (ident == LOOPER_ID_USER) 
			{
				if (engine.accelerometerSensor != NULL) 
				{
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
							&event, 1) > 0) 
					{
					//	LOGI("accelerometer: x=%f y=%f z=%f",
					//			event.acceleration.x, event.acceleration.y,
					//			event.acceleration.z);
					}
				}
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0) 
			{
				LOGI("engine_term_display(&engine); state->destroyRequested");
				engine_term_display(&engine);
				return;
			}
		}
//		if(mgr == NULL)break;
		engine_draw_frame(&engine);
	}
}

/**********************************************************************
 * Initialize OpenGL ES and EGL
***********************************************************************/
static int engine_init_display(struct engine* engine) 
{
	/*
	 * Here specify the attributes of the desired configuration.
	 * Below, we select an EGLConfig with at least 8 bits per color
	 * component compatible with on-screen windows
	 */
	const EGLint attribs[] = {
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_BLUE_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_RED_SIZE, 8,
			EGL_ALPHA_SIZE, 8,
			EGL_DEPTH_SIZE, 16,
			EGL_NONE
	};
	EGLint w, h, dummy, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* Here, the application chooses the configuration it desires. In this
	 * sample, we have a very simplified selection process, where we pick
	 * the first EGLConfig that matches our criteria */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	 * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	 * As soon as we picked a EGLConfig, we can safely reconfigure the
	 * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);

	EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs);


	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) 
	{
		LOGW("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	engine->display = display;
	engine->context = context;
	engine->surface = surface;
	engine->width = w;
	engine->height = h;

	if(engine->app != NULL && engine->app->activity != NULL)
	{
		mgr = engine->app->activity->assetManager;
	}
	opengl_init(w,h);
	LOGI("opengl init");
	
	return 0;
}

/**********************************************************************
 * Just the current frame in the display.
 *********************************************************************/

double get_time()
{
	timeval current;
	gettimeofday(&current, NULL);
	double secs = current.tv_sec + current.tv_usec*0.000001;
	return secs;
} 


const int fps = 35;
const float constant_t = 1.0f / fps;

static void engine_draw_frame(struct engine* engine) 
{
	static double start = get_time();
	if (engine->display == NULL) 
	{
		// No display.
		//LOGI("no display");
		return;
	}
	
	float t = get_time()-start;
	if(t > constant_t)
	{
		//LOGI("frame time = %f", t);
		start = get_time();
		opengl_display();
	}
	else
	{
		return;
	}

	eglSwapBuffers(engine->display, engine->surface);
}

/**********************************************************************
 * Tear down the EGL context currently associated with 
 * the display.
 *********************************************************************/
static void engine_term_display(struct engine* engine) 
{	
	if (engine->display != EGL_NO_DISPLAY) 
	{
		mgr = NULL;
		if(engine != NULL && engine->app != NULL && engine->app->savedState != NULL )
		{
			free(engine->app->savedState);
			engine->app->savedState = NULL;
		}
		opengl_end();

		eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (engine->context != EGL_NO_CONTEXT) 
		{
			eglDestroyContext(engine->display, engine->context);
		}
		if (engine->surface != EGL_NO_SURFACE) 
		{
			eglDestroySurface(engine->display, engine->surface);
		}
		eglTerminate(engine->display);
	}
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;

}

