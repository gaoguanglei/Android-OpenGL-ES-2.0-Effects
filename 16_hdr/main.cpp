

#include <platform.h>
#include <image.h>
#include <3ds.h>
#include <shader_loader.h>
#include <shader_program.h>
#include <math_lib.h>
#include <shape.h>
#include <time.h>

enum {POSITION, NORMAL, TEXCOORD, TARGENT, BINORMAL, INDEX};

struct static_mesh
{
	GLuint attribute_vbos[6];
	GLuint num_faces;
};

struct program
{
	shader_program* p_program;
	GLuint attribute_locations[6];
	GLuint texs_locs[2];
	GLuint loc_world_view_proj;
	GLuint loc_kernel;

	program()
	{
		p_program = NULL;
		attribute_locations[0] = -1;
		attribute_locations[1] = -1;
		attribute_locations[2] = -1;
		attribute_locations[3] = -1;
		attribute_locations[4] = -1;
		attribute_locations[5] = -1;
	}
};

struct vec4 : public vec3
{
	float w;
	vec4(){}
	vec4(float a, float b, float c, float d) : vec3(a,b,c), w(d){}
};

static_mesh cube_mesh;
static_mesh object_mesh;
static_mesh quad_mesh;

program env_program;
program object_program;
program shrinking_program;
program filter_program;
program final_program;

mat4	view_matrix;
mat4	world_matrix;
mat4	world_view_proj_matrix;
mat4	proj_matrix;

float	distance_z = 182;

int		width;
int		height;

GLuint fbo_color, texture_color, renderbuffer;
GLuint fbo_smallsize, texture_smallsize;
GLuint fbo_blur0, texture_blur0;
GLuint fbo_blur1, texture_blur1;
GLuint texture_env, texture_obj, texture_bump;

vec4 gaussFilterH[7] = {
	vec4(-3.0, 0.0, 0.0,  1.0/64.0),
	vec4(-2.0, 0.0, 0.0,  6.0/64.0),
	vec4(-1.0, 0.0, 0.0, 15.0/64.0),
	vec4( 0.0, 0.0, 0.0, 20.0/64.0),
	vec4( 1.0, 0.0, 0.0, 15.0/64.0),
	vec4( 2.0, 0.0, 0.0,  6.0/64.0),
	vec4( 3.0, 0.0, 0.0,  1.0/64.0)
};
vec4 gaussFilterV[7] = {
	vec4(0.0, -3.0, 0.0,  1.0/64.0),
	vec4(0.0, -2.0, 0.0,  6.0/64.0),
	vec4(0.0, -1.0, 0.0, 15.0/64.0),
	vec4(0.0,  0.0, 0.0, 20.0/64.0),
	vec4(0.0,  1.0, 0.0, 15.0/64.0),
	vec4(0.0,  2.0, 0.0,  6.0/64.0),
	vec4(0.0,  3.0, 0.0,  1.0/64.0)
};


inline void swap_channel(float* p)
{
	float temp;
	temp = p[1];
	p[1] = -p[2];
	p[2] = -temp; 
	p[0] = -p[0];
}

void computer_targent_space(vec3* normal, vec3*pos, vec3*targent, 
							vec3*binormal,float* texcoord, 
							unsigned short* idx, int num_vertex,int num_faces)
{
	memset(targent, 0, num_vertex*sizeof(vec3));
	memset(binormal, 0, num_vertex*sizeof(vec3));

	for(int i=0; i<num_faces; i++)
	{
		vec3 p0 = pos[idx[3*i+0]];
		vec3 p1 = pos[idx[3*i+1]];
		vec3 p2 = pos[idx[3*i+2]];

		vec3 n0 = normal[idx[3*i+0]];
		vec3 n1 = normal[idx[3*i+0]];
		vec3 n2 = normal[idx[3*i+0]];

		float s0 = texcoord[idx[3*i+0]*2+0];
		float t0 = texcoord[idx[3*i+0]*2+1];

		float s1 = texcoord[idx[3*i+1]*2+0];
		float t1 = texcoord[idx[3*i+1]*2+1];

		float s2 = texcoord[idx[3*i+2]*2+0];
		float t2 = texcoord[idx[3*i+2]*2+1];

		vec3 T = ( (p1-p0)*(t2-t0) - (p2-p0)*(t1-t0) ) /((t2-t0)*(s1-s0) - (t1-t0)*(s2-s0));
		vec3 B = ( (p1-p0)*(s2-s0) - (p2-p0)*(s1-s0) ) /((s2-s0)*(t1-t0) - (s1-s0)*(t2-t0));

		targent[idx[3*i+0]] += T;
		targent[idx[3*i+1]] += T;
		targent[idx[3*i+2]] += T;

		binormal[idx[3*i+0]] += B;
		binormal[idx[3*i+1]] += B;
		binormal[idx[3*i+2]] += B;
	}

	for(int i=0; i<num_vertex; i++)
	{
		targent[i].normalize();
		binormal[i].normalize();

		swap_channel(&(pos[i].x));
		swap_channel(&(normal[i].x));
		swap_channel(&(targent[i].x));
		swap_channel(&(binormal[i].x));
	}
}


void check_gl_error(const char* op) 
{
	for (GLint error = glGetError(); error; error= glGetError()) 
	{		
		LOGI("after %s() glError (0x%x)\n", op, error);
	}
}


void init_fbo(GLuint& fbo, GLuint& rt,GLsizei w, GLsizei h, bool enable_depth_render)
{
	GLuint rboId;

	glGenTextures(1, &rt);
	glBindTexture(GL_TEXTURE_2D, rt);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if(enable_depth_render)
	{
		glGenRenderbuffers(1, &rboId);
		glBindRenderbuffer(GL_RENDERBUFFER, rboId);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,w, h);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		renderbuffer = rboId;
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
	
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	check_gl_error("glTexParameteri");

	return texId;
}

int get_image_size( int level,image_data& image) 
{
	bool compressed = false;
	int w = image._width >> level;
	int h = image._height >> level;
	int d = image._depth >> level;
	w = (w) ? w : 1;
	h = (h) ? h : 1;
	d = (d) ? d : 1;
	int bw = ( w + 3 ) / 4;
	int bh = ( h + 3 ) / 4;
	int elementSize = image._elementSize;

	return bw*bh*d*elementSize;
}


int create_cube_texture(const char* file_name)
{
	image_data image;
	GLuint texId;
	image_tool::read_image(file_name, image);

	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texId);

	bool compressed = false;

	for(int i=0; i<6; i++)
	{
		if(image._internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
			|| image._internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
			|| image._internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
		{
			compressed = true;
		}

		if(!compressed)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, image._internalFormat, image._width, image._height, 0, image._format, image._type, image._data[i*image._levelCount]);
		}
		else
		{
			glCompressedTexImage2D( 
				GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,
				0,
				image._internalFormat,
				image._width, 
				image._height, 
				0, 
				get_image_size(0,image), 
				image._data[i*image._levelCount]);
		}
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	return texId;
}


						
void opengl_init(int w, int h)
{
	/**********************************************************************************/
	CLoad3DS g_Load3ds;	
	t3DModel g_3DModel;
	
	shader_loader vs,ps;

	init_fbo(fbo_color,	texture_color,512,512,true);
	init_fbo(fbo_smallsize, texture_smallsize,128,128,false);
	init_fbo(fbo_blur0,	texture_blur0,	128,128,false);
	init_fbo(fbo_blur1,	texture_blur1,	128,128,false);

	/*enviroment-**********************************************************************************/

	env_program.p_program = new shader_program;
	if(!env_program.p_program->build(vs.load_shader_source("media/env.vs"), ps.load_shader_source("media/env.ps")))
	{
		LOGI("build shader failed\n");
		return;
	}
	env_program.attribute_locations[POSITION] = glGetAttribLocation(env_program.p_program->get_program(),"rm_Vertex");
	env_program.loc_world_view_proj = glGetUniformLocation(env_program.p_program->get_program(), "world_view_proj_mat");
	env_program.texs_locs[0] = glGetUniformLocation(env_program.p_program->get_program(), "EnvironmentMap");

	/*object**********************************************************************************/

	object_program.p_program = new shader_program;
	if(!object_program.p_program->build(vs.load_shader_source("media/obj.vs"), ps.load_shader_source("media/obj.ps")))
	{
		LOGI("build shader failed\n");
		return;
	}

	object_program.attribute_locations[POSITION] = glGetAttribLocation(object_program.p_program->get_program(),"rm_Vertex");
	object_program.attribute_locations[NORMAL] = glGetAttribLocation(object_program.p_program->get_program(),"rm_Normal");
	object_program.attribute_locations[TEXCOORD] = glGetAttribLocation(object_program.p_program->get_program(),"rm_TexCoord0");
	object_program.attribute_locations[TARGENT] = glGetAttribLocation(object_program.p_program->get_program(),"rm_Tangent");
	object_program.attribute_locations[BINORMAL] = glGetAttribLocation(object_program.p_program->get_program(),"rm_Binormal");
	
	object_program.loc_world_view_proj = glGetUniformLocation(object_program.p_program->get_program(), "world_view_proj_mat");
	object_program.texs_locs[0] = glGetUniformLocation(object_program.p_program->get_program(), "BumpMap");
	object_program.texs_locs[1] = glGetUniformLocation(object_program.p_program->get_program(), "ObjectMap");

	/*shrink**********************************************************************************/

	shrinking_program.p_program = new shader_program;
	if(!shrinking_program.p_program->build(vs.load_shader_source("media/shrink.vs"), ps.load_shader_source("media/shrink.ps")))
	{
		LOGI("build shader failed\n");
		return;
	}
	shrinking_program.attribute_locations[POSITION] = glGetAttribLocation(shrinking_program.p_program->get_program(),"rm_Vertex");
	shrinking_program.texs_locs[0] = glGetUniformLocation(shrinking_program.p_program->get_program(), "RT");

	//filter**********************************************************************************/
	
	filter_program.p_program = new shader_program;
	if(!filter_program.p_program->build(vs.load_shader_source("media/filter.vs"), ps.load_shader_source("media/filter.ps")))
	{
		LOGI("build shader failed\n");
		return;
	}
	filter_program.attribute_locations[POSITION] = glGetAttribLocation(filter_program.p_program->get_program(),"rm_Vertex");
	filter_program.texs_locs[0] = glGetUniformLocation(filter_program.p_program->get_program(), "Src");
	filter_program.loc_kernel = glGetUniformLocation(filter_program.p_program->get_program(), "gaussFilter");

	/*final**********************************************************************************/

	final_program.p_program = new shader_program;
	if(!final_program.p_program->build(vs.load_shader_source("media/final.vs"), ps.load_shader_source("media/final.ps")))
	{
		LOGI("build shader failed\n");
		return;
	}
	final_program.attribute_locations[POSITION] = glGetAttribLocation(final_program.p_program->get_program(),"rm_Vertex");
	final_program.texs_locs[0] = glGetUniformLocation(final_program.p_program->get_program(), "SrcColor");
	final_program.texs_locs[1] = glGetUniformLocation(final_program.p_program->get_program(), "SrcHDR");

	/*cube mesh**********************************************************************************/

	float *pos_data, *tex_data, *normal_data;
	GLushort *inds;
	GLuint num_ves;

	
	GLuint num_indx = shape::create_cube(1000.0f, &pos_data, NULL, NULL, &inds, num_ves);

	glGenBuffers(1, &cube_mesh.attribute_vbos[POSITION]);
	glGenBuffers(1, &cube_mesh.attribute_vbos[INDEX]);

	glBindBuffer(GL_ARRAY_BUFFER, cube_mesh.attribute_vbos[POSITION]);
	glBufferData(GL_ARRAY_BUFFER, num_ves*4*3, pos_data, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_mesh.attribute_vbos[INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12*2*3, inds, GL_STATIC_DRAW);

	cube_mesh.num_faces = num_indx/3;

	texture_env = create_cube_texture("media/Snow.dds");

	/*quad**********************************************************************************/


	num_indx = shape::create_plane(1.0f, &pos_data, NULL, NULL, &inds, num_ves);

	glGenBuffers(1, &quad_mesh.attribute_vbos[POSITION]);
	glGenBuffers(1, &quad_mesh.attribute_vbos[INDEX]);

	glBindBuffer(GL_ARRAY_BUFFER, quad_mesh.attribute_vbos[POSITION]);
	glBufferData(GL_ARRAY_BUFFER, num_ves*4*3, pos_data, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_mesh.attribute_vbos[INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_ves*2*3, inds, GL_STATIC_DRAW);

	quad_mesh.num_faces = num_indx/3;

	/* object mesh *****************************************************************************************/

	LOGI("begin to load 3ds file");
	g_Load3ds.Import3DS(&g_3DModel, "media/Teapot.3ds");
	LOGI("load 3ds end");

	if(g_3DModel.numOfObjects == 0)
	{
		return;
	}

	t3DObject* p = &g_3DModel.pObject[0];

	texture_obj = create_texture("media/Fieldstone.tga");
	texture_bump = create_texture("media/FieldstoneBumpDOT3.tga");

	vec3* targents = new vec3[p->numOfVerts];
	vec3* binormal = new vec3[p->numOfVerts];

	computer_targent_space((vec3*)p->pNormals, 
		(vec3*)p->pVerts, 
		targents, 
		binormal, 
		(float*)p->pTexVerts, 
		p->pFaces->vertIndex, 
		p->numOfVerts, 
		p->numOfFaces);


	glGenBuffers(6, object_mesh.attribute_vbos);

	glBindBuffer(GL_ARRAY_BUFFER, object_mesh.attribute_vbos[TEXCOORD]);
	glBufferData(GL_ARRAY_BUFFER, 4*2*p->numOfVerts, &(p->pTexVerts[0].x), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, object_mesh.attribute_vbos[POSITION]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(p->pVerts[0].x), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, object_mesh.attribute_vbos[NORMAL]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(p->pNormals[0].x), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, object_mesh.attribute_vbos[TARGENT]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(targents[0].x), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, object_mesh.attribute_vbos[BINORMAL]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(binormal[0].x), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object_mesh.attribute_vbos[INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*3*p->numOfFaces, &(p->pFaces[0].vertIndex[0]), GL_STATIC_DRAW);

	object_mesh.num_faces = p->numOfFaces;


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	
	width = w;
	height = h;
	
	proj_matrix.perspective(60, (float)w/h, 1.0f,3000.0f);
}


void draw(program* p, static_mesh* mesh);

void opengl_display()
{
	LOGI("---------- 0 ");
	/* background and object ******************************************************************/

//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_color);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	glViewport(0,0,512,512);
	glEnable(GL_DEPTH_TEST);

	//-------------------------------------------------------------------
	world_view_proj_matrix = proj_matrix * world_matrix;

	glUseProgram(env_program.p_program->get_program());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_env);
	glUniform1i(env_program.texs_locs[0], 0);

	glUniformMatrix4fv(env_program.loc_world_view_proj, 1, GL_FALSE, &world_view_proj_matrix[0]);

	draw(&env_program, &cube_mesh);


	//-----------------------------------------------------------------------

	view_matrix.look_at(vec3(0,0,distance_z), vec3(0,0,0), vec3(0,1,0));
	world_view_proj_matrix = proj_matrix * view_matrix * world_matrix;

	glUseProgram(object_program.p_program->get_program());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_bump);
	glUniform1i(object_program.texs_locs[0], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_obj);
	glUniform1i(object_program.texs_locs[1], 1);

	glUniformMatrix4fv(object_program.loc_world_view_proj, 1, GL_FALSE, &world_view_proj_matrix[0]);

	draw(&object_program, &object_mesh);

	/*shrinking************************************************************************/
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_smallsize);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0,0,128,128);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(shrinking_program.p_program->get_program());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_color);
	glUniform1i(shrinking_program.texs_locs[0], 0);

	draw(&shrinking_program, &quad_mesh);

glFinish();
#if 1
	/*filter_h0************************************************************************************/
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_blur0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(filter_program.p_program->get_program());
	glViewport(0,0,128,128);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_smallsize);
	glUniform1i(filter_program.texs_locs[0], 0);
	glUniform4fv(filter_program.loc_kernel, 7, gaussFilterH[0].v);

	draw(&filter_program, &quad_mesh);


glFinish();
	/*filter_v0************************************************************************************/
//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_blur1);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(filter_program.p_program->get_program());
	glViewport(0,0,128,128);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_blur0);
	glUniform1i(filter_program.texs_locs[0], 0);

	glUniform4fv(filter_program.loc_kernel, 7, gaussFilterV[0].v);
	draw(&filter_program, &quad_mesh);

glFinish();
	/*filter_h1************************************************************************************/
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_blur0);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(filter_program.p_program->get_program());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_blur1);
	glUniform1i(filter_program.texs_locs[0], 0);

	glUniform4fv(filter_program.loc_kernel, 7, gaussFilterH[0].v);

	draw(&filter_program, &quad_mesh);
glFinish();
	/*filter_v1************************************************************************************/
	
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_blur1);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(filter_program.p_program->get_program());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_blur0);
	glUniform1i(filter_program.texs_locs[0], 0);

	glUniform4fv(filter_program.loc_kernel, 7, gaussFilterV[0].v);
	draw(&filter_program, &quad_mesh);
glFinish();
	/*final************************************************************************************/

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0,0,width,height);
	glUseProgram(final_program.p_program->get_program());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_color);
	glUniform1i(final_program.texs_locs[0], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_blur1);
	glUniform1i(final_program.texs_locs[1], 1);

	draw(&final_program, &quad_mesh);
glFinish();
#endif
}

#define  safely_free_pointer(p)\
{\
	if(p != NULL)\
	{\
		delete p;\
		p = NULL;\
	}\
}

void opengl_end()
{
	glDeleteRenderbuffers(1, &renderbuffer);
	glDeleteTextures(1, &texture_color);
	glDeleteFramebuffers(1, &fbo_color);

	glDeleteTextures(1, &texture_smallsize);
	glDeleteFramebuffers(1, &fbo_smallsize);

	glDeleteTextures(1, &texture_blur0);
	glDeleteFramebuffers(1, &fbo_blur0);

	glDeleteTextures(1, &texture_blur1);
	glDeleteFramebuffers(1, &fbo_blur1);

	safely_free_pointer(env_program.p_program);
	safely_free_pointer(object_program.p_program);
	safely_free_pointer(shrinking_program.p_program);
	safely_free_pointer(final_program.p_program);
	safely_free_pointer(filter_program.p_program);
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


void draw(program* p, static_mesh* mesh)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->attribute_vbos[INDEX]);

	if(p->attribute_locations[POSITION] != -1)
	{
		glEnableVertexAttribArray(p->attribute_locations[POSITION]);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->attribute_vbos[POSITION]);
		glVertexAttribPointer(p->attribute_locations[POSITION], 3, GL_FLOAT, GL_FALSE, 12, 0);
	}
	if(p->attribute_locations[TEXCOORD] != -1)
	{
		glEnableVertexAttribArray(p->attribute_locations[TEXCOORD]);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->attribute_vbos[TEXCOORD]);
		glVertexAttribPointer(p->attribute_locations[TEXCOORD], 2, GL_FLOAT, GL_FALSE, 8, 0);
	}
	if(p->attribute_locations[NORMAL] != -1)
	{
		glEnableVertexAttribArray(p->attribute_locations[NORMAL]);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->attribute_vbos[NORMAL]);
		glVertexAttribPointer(p->attribute_locations[NORMAL], 3, GL_FLOAT, GL_FALSE, 12, 0);
	}

	if(p->attribute_locations[TARGENT] != -1)
	{
		glEnableVertexAttribArray(p->attribute_locations[TARGENT]);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->attribute_vbos[TARGENT]);
		glVertexAttribPointer(p->attribute_locations[TARGENT], 3, GL_FLOAT, GL_FALSE, 12, 0);
	}

	if(p->attribute_locations[BINORMAL] != -1)
	{
		glEnableVertexAttribArray(p->attribute_locations[BINORMAL]);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->attribute_vbos[BINORMAL]);
		glVertexAttribPointer(p->attribute_locations[BINORMAL], 3, GL_FLOAT, GL_FALSE, 12, 0);
	}

	glDrawElements(GL_TRIANGLES, mesh->num_faces*3, GL_UNSIGNED_SHORT, 0);

	if(p->attribute_locations[NORMAL] != -1)
	{
		glDisableVertexAttribArray(p->attribute_locations[NORMAL]);
	}
	if(p->attribute_locations[POSITION] != -1)
	{
		glDisableVertexAttribArray(p->attribute_locations[POSITION]);
	}
	if(p->attribute_locations[TEXCOORD] != -1)
	{
		glDisableVertexAttribArray(p->attribute_locations[TEXCOORD]);
	}

	if(p->attribute_locations[TARGENT] != -1)
	{
		glDisableVertexAttribArray(p->attribute_locations[TARGENT]);
	}

	if(p->attribute_locations[BINORMAL] != -1)
	{
		glDisableVertexAttribArray(p->attribute_locations[BINORMAL]);
	}
}
