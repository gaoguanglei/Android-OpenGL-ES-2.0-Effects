

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
	GLuint loc_view_matrix;

	static_mesh()
	{
		program = NULL;
		attribute_locations[0] = -1;
		attribute_locations[1] = -1;
		attribute_locations[2] = -1;
	}

	void draw();
};

static_mesh blur_mesh;
static_mesh model_mesh;
static_mesh combine_mesh;

mat4	world_view_matrix;
mat4	world_matrix;
mat4	torus_proj_matrix;
mat4	quad_proj_matrix;
mat4	view_proj_matrix;
float	distance_z = 182;

int		width;
int		height;

GLuint render_fbo, render_rt;
GLuint blur_fbo, blur_rt;

void check_gl_error(const char* op) 
{
	for (GLint error = glGetError(); error; error= glGetError()) 
	{		
		LOGI("after %s() glError (0x%x)\n", op, error);
	}
}



void init_fbo(GLuint& fbo, GLuint& rt, bool enable_depth_render)
{
	GLuint rboId;

	glGenTextures(1, &rt);
	glBindTexture(GL_TEXTURE_2D, rt);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if(enable_depth_render)
	{
		glGenRenderbuffers(1, &rboId);
		glBindRenderbuffer(GL_RENDERBUFFER, rboId);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,512, 512);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
		
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// attach the render_rt to FBO color attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						   GL_TEXTURE_2D, rt, 0);

	// attach the renderbuffer to depth attachment point
	if(enable_depth_render)
	{
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
							  GL_RENDERBUFFER, rboId);
	}

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(status != GL_FRAMEBUFFER_COMPLETE)
	{
		return;
	}

	// switch back to window-system-provided framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

int create_texture(const char* file_name)
{
	image_data image;
	GLuint texId;
	image_tool::read_image(file_name, image);

	glGenTextures(1, &texId);
	glBindTexture( GL_TEXTURE_2D, texId);

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

	init_fbo(render_fbo, render_rt, true);
	init_fbo(blur_fbo, blur_rt, true);

	if(model_mesh.program == NULL)
	{
		LOGI("begin load shader");

		model_mesh.program = new shader_program;
		if(!model_mesh.program->build(vs.load_shader_source("media/model.vs"),ps.load_shader_source("media/model.ps")))
		{
			LOGI("build shader failed");
			return;
		}
	
		model_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(model_mesh.program->get_program(),"rm_Vertex");
		model_mesh.attribute_locations[static_mesh::NORMAL] = glGetAttribLocation(model_mesh.program->get_program(),"rm_Normal");
		model_mesh.attribute_locations[static_mesh::TEXCOORD] = glGetAttribLocation(model_mesh.program->get_program(),"rm_TexCoord0");

		LOGI("begin to load 3ds file");
		g_Load3ds.Import3DS(&g_3DModel, "media/Radeon.3ds");
		LOGI("load 3ds end");

		t3DObject* p = &g_3DModel.pObject[0];
LOGI("---------- 0");
		model_mesh.loc_view_matrix = glGetUniformLocation(model_mesh.program->get_program(), "view_matrix");
		model_mesh.loc_view_proj_matrix = glGetUniformLocation(model_mesh.program->get_program(), "view_proj_matrix");
		model_mesh.texs_locs[0] = glGetUniformLocation(model_mesh.program->get_program(), "base");
LOGI("---------- 1");
		model_mesh.texs_idxs[0] = create_texture("media/Wood.tga");

LOGI("---------- 2");
		glGenBuffers(4, model_mesh.attribute_vbos);
LOGI("---------- 3");
		glBindBuffer(GL_ARRAY_BUFFER, model_mesh.attribute_vbos[static_mesh::POSITION]);
		glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(p->pVerts[0].x), GL_STATIC_DRAW);
LOGI("---------- 4");
		glBindBuffer(GL_ARRAY_BUFFER, model_mesh.attribute_vbos[static_mesh::NORMAL]);
		glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(p->pNormals[0].x), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, model_mesh.attribute_vbos[static_mesh::TEXCOORD]);
		glBufferData(GL_ARRAY_BUFFER, 4*2*p->numOfVerts, &(p->pTexVerts[0].x), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model_mesh.attribute_vbos[static_mesh::INDEX]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*3*p->numOfFaces, &(p->pFaces[0].vertIndex[0]), GL_STATIC_DRAW);
LOGI("---------- 5");
		model_mesh.num_faces = p->numOfFaces;
	}

	/* init background *********************************************************************************/
	
	if(blur_mesh.program == NULL)
	{
		float *pos_data;
		GLushort *inds;
		GLuint num_ves;

		blur_mesh.program = new shader_program;
		if(!blur_mesh.program->build(vs.load_shader_source("media/quad.vs"),ps.load_shader_source("media/quad.ps")))
		{
			LOGI("build shader failed");
			return;
		}

		blur_mesh.texs_idxs[0] = render_rt;
		blur_mesh.texs_locs[0] = glGetUniformLocation(blur_mesh.program->get_program(), "RT");

		shape::create_plane(1, &pos_data, NULL, NULL, &inds, num_ves);

//		blur_mesh.loc_view_proj_matrix = glGetUniformLocation(blur_mesh.program->get_program(), "view_proj_matrix");
		
		blur_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(blur_mesh.program->get_program(),"rm_Vertex");

		glGenBuffers(1, &blur_mesh.attribute_vbos[static_mesh::POSITION]);
		glGenBuffers(1, &blur_mesh.attribute_vbos[static_mesh::INDEX]);

		glBindBuffer(GL_ARRAY_BUFFER, blur_mesh.attribute_vbos[static_mesh::POSITION]);
		glBufferData(GL_ARRAY_BUFFER, num_ves*4*3, pos_data, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blur_mesh.attribute_vbos[static_mesh::INDEX]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*2*3, inds, GL_STATIC_DRAW);

		blur_mesh.num_faces = 6;

		delete []pos_data;
		delete []inds;
	}

	if(combine_mesh.program == NULL)
	{
		float *pos_data;
		GLushort *inds;
		GLuint num_ves;

		combine_mesh = blur_mesh;

		combine_mesh.program = new shader_program;
		if(!combine_mesh.program->build(vs.load_shader_source("media/combine.vs"),ps.load_shader_source("media/combine.ps")))
		{
			LOGI("build shader failed");
			return;
		}

		combine_mesh.texs_idxs[0] = render_rt;
		combine_mesh.texs_locs[0] = glGetUniformLocation(combine_mesh.program->get_program(), "RT");

		combine_mesh.texs_idxs[1] = blur_rt;
		combine_mesh.texs_locs[1] = glGetUniformLocation(combine_mesh.program->get_program(), "Blur1");

		

	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	width = w;
	height = h;
	
	quad_proj_matrix.perspective(60, (float)w/h, 1.0f,3000.0f);
}



void opengl_display()
{
	
	glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);

	if(model_mesh.program != NULL)
	{
		glViewport(0,0,512,512);
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

		glEnable(GL_DEPTH_TEST);


		world_view_matrix.look_at(vec3(0,0,distance_z), vec3(0,0,0), vec3(0,1,0));
		world_view_matrix *= world_matrix;
		view_proj_matrix = quad_proj_matrix * world_view_matrix;

		glUseProgram(model_mesh.program->get_program());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, model_mesh.texs_idxs[0]);
		glUniform1i(model_mesh.texs_locs[0], 0);

		glUniformMatrix4fv(model_mesh.loc_view_proj_matrix, 1, GL_FALSE, &view_proj_matrix[0]);
		glUniformMatrix4fv(model_mesh.loc_view_matrix, 1, GL_FALSE, &world_view_matrix[0]);

		model_mesh.draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo);

	if(blur_mesh.program != NULL)
	{
		glViewport(0,0,512, 512);
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

		glDisable(GL_DEPTH_TEST);

		glUseProgram(blur_mesh.program->get_program());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, blur_mesh.texs_idxs[0]);
		glUniform1i(blur_mesh.texs_locs[0], 0);

		blur_mesh.draw();

	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if(combine_mesh.program != NULL)
	{
		glViewport(0,0,width, height);
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

		glDisable(GL_DEPTH_TEST);

		glUseProgram(combine_mesh.program->get_program());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, combine_mesh.texs_idxs[0]);
		glUniform1i(combine_mesh.texs_locs[0], 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, combine_mesh.texs_idxs[1]);
		glUniform1i(combine_mesh.texs_locs[1], 1);

		combine_mesh.draw();
	}
	
}

void opengl_end()
{
	LOGI("opengl_end()");
	if(blur_mesh.program != NULL)
	{
		delete blur_mesh.program;
		blur_mesh.program = NULL;
	}

	if(model_mesh.program != NULL)
	{
		delete model_mesh.program;
		model_mesh.program = NULL;
	}

	if(combine_mesh.program != NULL)
	{
		delete combine_mesh.program;
		combine_mesh.program = NULL;
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
