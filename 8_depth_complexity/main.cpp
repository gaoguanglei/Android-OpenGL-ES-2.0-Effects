

#include <platform.h>
#include <image.h>
#include <3ds.h>
#include <shader_loader.h>
#include <shader_program.h>
#include <math_lib.h>
#include <shape.h>
#include <time.h>


struct static_mesh
{
	enum {POSITION, NORMAL, TEXCOORD, INDEX};
	shader_program* program;
	GLuint attribute_vbos[4];
	GLuint attribute_locations[3];
	GLuint texs_idxs[2];
	GLuint texs_locs[2];
	GLuint num_faces;

	GLuint loc_view_proj_matrix;

	static_mesh()
	{
		program = NULL;
		attribute_locations[0] = -1;
		attribute_locations[1] = -1;
		attribute_locations[2] = -1;
	}

	void draw();
};

void static_mesh::draw()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, attribute_vbos[static_mesh::INDEX]);

	if(attribute_locations[static_mesh::POSITION] != -1)
	{
		glEnableVertexAttribArray(attribute_locations[static_mesh::POSITION]);
		glBindBuffer(GL_ARRAY_BUFFER, attribute_vbos[static_mesh::POSITION]);
		glVertexAttribPointer(attribute_locations[static_mesh::POSITION], 3, GL_FLOAT, GL_FALSE, 12, 0);
	}
	if(attribute_locations[static_mesh::TEXCOORD] != -1)
	{
		glEnableVertexAttribArray(attribute_locations[static_mesh::TEXCOORD]);
		glBindBuffer(GL_ARRAY_BUFFER, attribute_vbos[static_mesh::TEXCOORD]);
		glVertexAttribPointer(attribute_locations[static_mesh::TEXCOORD], 2, GL_FLOAT, GL_FALSE, 8, 0);
	}
	if(attribute_locations[static_mesh::NORMAL] != -1)
	{
		glEnableVertexAttribArray(attribute_locations[static_mesh::NORMAL]);
		glBindBuffer(GL_ARRAY_BUFFER, attribute_vbos[static_mesh::NORMAL]);
		glVertexAttribPointer(attribute_locations[static_mesh::NORMAL], 3, GL_FLOAT, GL_FALSE, 12, 0);
	}

	glDrawElements(GL_TRIANGLES, num_faces*3, GL_UNSIGNED_SHORT, 0);

	if(attribute_locations[static_mesh::NORMAL] != -1)
	{
		glDisableVertexAttribArray(attribute_locations[static_mesh::NORMAL]);
	}
	if(attribute_locations[static_mesh::POSITION] != -1)
	{
		glDisableVertexAttribArray(attribute_locations[static_mesh::POSITION]);
	}
	if(attribute_locations[static_mesh::TEXCOORD] != -1)
	{
		glDisableVertexAttribArray(attribute_locations[static_mesh::TEXCOORD]);
	}
}


static_mesh quad_mesh;
static_mesh torus_mesh;

mat4	view_matrix;
mat4	world_matrix;
mat4	torus_proj_matrix;
mat4	quad_proj_matrix;
mat4	view_proj_matrix;
float	distance_z = 182;

int		width;
int		height;

GLuint frame_buffer, depth_render_buffer, texture;

void check_gl_error(const char* op) 
{
	for (GLint error = glGetError(); error; error= glGetError()) 
	{		
		LOGI("after %s() glError (0x%x)\n", op, error);
	}
}

void init_FBO()
{
	GLint max_buffer_size;
	GLint buffer_size = 512;
	
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &max_buffer_size);
	
	if(max_buffer_size < 512)
	{
		buffer_size = max_buffer_size;
	}

	glGenFramebuffers(1, &frame_buffer);
//	glGenRenderbuffers(1, &depth_render_buffer);
	glGenTextures(1, &texture);

//	glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer);
//	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, buffer_size, buffer_size);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, buffer_size, buffer_size, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
//	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_render_buffer);



	check_gl_error("init fbo"); 
}

void initFramebuffer()
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);
check_gl_error("glTexImage2D");
	glBindTexture(GL_TEXTURE_2D, 0);
check_gl_error("glBindTexture");
	GLuint rboId;
	glGenRenderbuffers(1, &rboId);
check_gl_error("glGenRenderbuffers");
	glBindRenderbuffer(GL_RENDERBUFFER, rboId);
check_gl_error("glBindRenderbuffer");
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,1024, 1024);
check_gl_error("glRenderbufferStorage");
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
check_gl_error("glBindRenderbuffer");
		
	glGenFramebuffers(1, &frame_buffer);
check_gl_error("glGenFramebuffers");
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
check_gl_error("glBindFramebuffer");
	// attach the texture to FBO color attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						   GL_TEXTURE_2D, texture, 0);

	// attach the renderbuffer to depth attachment point
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							  GL_RENDERBUFFER, rboId);
check_gl_error("glFramebufferRenderbuffer");
	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		return;
	}

	// switch back to window-system-provided framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	check_gl_error("init fbo"); 
}

int create_texture(const char* file_name)
{
//LOGI(file_name);
	image_data image;
	GLuint texId;
	image_tool::read_image(file_name, image);

	glGenTextures(1, &texId);
check_gl_error("glGenTextures");
	glBindTexture( GL_TEXTURE_2D, texId);
check_gl_error("glBindTexture");

	glTexImage2D( GL_TEXTURE_2D, 0, image._internalFormat, image._width, image._height, 0, image._format, image._type, image._data[0]);


	check_gl_error("glTexImage2D");
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	check_gl_error("glTexParameteri");

	return texId;
}
						
void opengl_init(int w, int h)
{
	/* init statue *********************************************************************************/
	CLoad3DS g_Load3ds;	
	t3DModel g_3DModel;
	
	shader_loader vs,ps;

	initFramebuffer();

	if(torus_mesh.program == NULL)
	{
		LOGI("begin load shader");

		torus_mesh.program = new shader_program;
		if(!torus_mesh.program->build(vs.load_shader_source("media/torus.vs"),ps.load_shader_source("media/torus.ps")))
		{
			LOGI("build shader failed");
			return;
		}
	
		torus_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(torus_mesh.program->get_program(),"rm_Vertex");
;

		LOGI("begin to load 3ds file");
		g_Load3ds.Import3DS(&g_3DModel, "media/Torus.3ds");
		LOGI("load 3ds end");

		t3DObject* p = &g_3DModel.pObject[0];

		torus_mesh.loc_view_proj_matrix = glGetUniformLocation(torus_mesh.program->get_program(), "view_proj_matrix");

		glGenBuffers(1, &torus_mesh.attribute_vbos[static_mesh::POSITION]);
		glGenBuffers(1, &torus_mesh.attribute_vbos[static_mesh::INDEX]);

		glBindBuffer(GL_ARRAY_BUFFER, torus_mesh.attribute_vbos[static_mesh::POSITION]);
		glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(p->pVerts[0].x), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, torus_mesh.attribute_vbos[static_mesh::INDEX]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*3*p->numOfFaces, &(p->pFaces[0].vertIndex[0]), GL_STATIC_DRAW);

		torus_mesh.num_faces = p->numOfFaces;
	}

	/* init background *********************************************************************************/
	
	if(quad_mesh.program == NULL)
	{
		float *pos_data;
		GLushort *inds;
		GLuint num_ves;

		quad_mesh.program = new shader_program;
		if(!quad_mesh.program->build(vs.load_shader_source("media/quad.vs"),ps.load_shader_source("media/quad.ps")))
		{
			LOGI("build shader failed");
			return;
		}
		check_gl_error("build shader");

		quad_mesh.texs_idxs[0] = texture;
		quad_mesh.texs_idxs[1] = create_texture("media/Flame.tga");

		shape::create_plane(1, &pos_data, NULL, NULL, &inds, num_ves);

//		quad_mesh.loc_view_proj_matrix = glGetUniformLocation(quad_mesh.program->get_program(), "view_proj_matrix");
		quad_mesh.texs_locs[0] = glGetUniformLocation(quad_mesh.program->get_program(), "RT");
		quad_mesh.texs_locs[1] = glGetUniformLocation(quad_mesh.program->get_program(), "color");

		quad_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(quad_mesh.program->get_program(),"rm_Vertex");

		glGenBuffers(1, &quad_mesh.attribute_vbos[static_mesh::POSITION]);
		glGenBuffers(1, &quad_mesh.attribute_vbos[static_mesh::INDEX]);

		glBindBuffer(GL_ARRAY_BUFFER, quad_mesh.attribute_vbos[static_mesh::POSITION]);
		glBufferData(GL_ARRAY_BUFFER, num_ves*4*3, pos_data, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_mesh.attribute_vbos[static_mesh::INDEX]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*2*3, inds, GL_STATIC_DRAW);

		quad_mesh.num_faces = 6;

		delete []pos_data;
		delete []inds;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	width = w;
	height = h;
	
	quad_proj_matrix.perspective(60, (float)w/h, 1.0f,3000.0f);
}



void opengl_display()
{
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	if(torus_mesh.program != NULL)
	{
		glViewport(0,0,1024,1024);
		glClearColor(0.004,0.01,0.01,0.01);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		


		view_matrix.look_at(vec3(0,0,distance_z), vec3(0,0,0), vec3(0,1,0));
		view_proj_matrix = quad_proj_matrix * view_matrix * world_matrix;

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		glUseProgram(torus_mesh.program->get_program());
		glUniformMatrix4fv(torus_mesh.loc_view_proj_matrix, 1, GL_FALSE, &view_proj_matrix[0]);

		torus_mesh.draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if(quad_mesh.program != NULL)
	{
		glViewport(0,0,width, height);
		glClearColor(0.004,0.01,0.01,0.01);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

		glDisable(GL_BLEND);

//		view_proj_matrix = mat4();

		glUseProgram(quad_mesh.program->get_program());
//		glUniformMatrix4fv(quad_mesh.loc_view_proj_matrix, 1, GL_FALSE, &view_proj_matrix[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, quad_mesh.texs_idxs[0]);
		glUniform1i(quad_mesh.texs_locs[0], 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, quad_mesh.texs_idxs[1]);
		glUniform1i(quad_mesh.texs_locs[1], 1);

		quad_mesh.draw();

	}
}

void opengl_end()
{
	LOGI("opengl_end()");
	if(quad_mesh.program != NULL)
	{
		delete quad_mesh.program;
		quad_mesh.program = NULL;
	}

	if(torus_mesh.program != NULL)
	{
		delete torus_mesh.program;
		torus_mesh.program = NULL;
	}
}
/***************************************************************************
 * state = 0: AMOTION_EVENT_ACTION_DOWN for android
 * state = 1: AMOTION_EVENT_ACTION_MOVE for android 
 * state = 2: AMOTION_EVENT_ACTION_UP for android
 * state = 3: AMOTION_EVENT_ACTION_POINTER_DOWN for android
 * state = 4: AMOTION_EVENT_ACTION_POINTER_UP for android
 * state = 5: WM_MOUSEWHEEL zdelta > 0 for windows
 * state = 6: WM_MOUSEWHEEL zdelta < 0 for windows 
 ***************************************************************************/

void opengl_touch(int tx,int ty,int state)
{
	static int down_type = -1;
	static bool multi_down = false;
	static int last_x, last_y;
	static float rot_x = 0;
	static float rot_y = 0;
	static float last_dis = 0;

	int x = tx & 0xffff;
	int y = ty & 0xffff;
	int x2 = (tx & 0xffff0000)>>16;
	int y2 = (ty & 0xffff0000)>>16;

	mat4 rotx_mat,roty_mat;

//	roty_mat.rotate_y(rot_y);

	switch(state&0xff)
	{
	case 0: //AMOTION_EVENT_ACTION_DOWN
		last_x = x;
		last_y = y;
		down_type = 0;
		break;

	case 1://AMOTION_EVENT_ACTION_MOVE
		if(down_type == 0)
		{
			if(y - last_y > 0.1f)
			{
				rot_x += 0.7f;
			}
			else if(y - last_y < -0.1f)
			{
				rot_x -= 0.7f;
			}
			if(x - last_x > 0.1f)
			{
				rot_y += 0.7f;
			}
			else if(x - last_x < -0.1f)
			{
				rot_y -= 0.7f;
			}
			
			rotx_mat.rotate_x(rot_x);
			roty_mat.rotate_y(rot_y);

			world_matrix = rotx_mat * roty_mat;

			last_x = x;
			last_y = y;
		}
		else if(down_type == 1)
		{
			float cur_dis = (x2-x)*(x2-x)+(y2-y)*(y2-y);
			float diff = (last_dis - cur_dis);
			if(diff > 0.1f)
			{		
				distance_z *= 1.03f;
			}
			if(diff < -0.1f)
			{
				distance_z *= 0.95f;
			}
			
			if(distance_z < 1.0f)
			{
				distance_z = 1.0f;		
			}
			if(distance_z > 300.0f)
			{
				distance_z = 300.0f;
			}
			LOGI("distance=%f,cur_dis=%f,last_dis=%f", distance_z, cur_dis, last_dis);
			last_dis = cur_dis;
		}
		
		break;;
	case 2://AMOTION_EVENT_ACTION_UP
		down_type = -1;
		break;
	
	case 3://AMOTION_EVENT_ACTION_POINTER_DOWN
		down_type = 1;
		last_dis = (x2-x)*(x2-x)+(y2-y)*(y2-y);
		break;

	case 4://AMOTION_EVENT_ACTION_POINTER_UP
		down_type = 0;
		last_x = x;
		last_y = y;
		break;

	case 5://WM_MOUSEWHEEL
		distance_z *= 1.03f;
		if(distance_z > 300.f)
		{
			distance_z = 300.f;
		}
		break;

	case 6://WM_MOUSEWHEEL
		distance_z *= 0.98f;
		if(distance_z < 1.f)
		{
			distance_z = 1.f;
		}
		break;
	}
}


