

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

	GLuint loc_view_pos;
	GLuint loc_light_pos;
	GLuint loc_light_dir;

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

static_mesh object_mesh;


program object_program;

mat4	view_matrix;
mat4	world_matrix;
mat4	world_view_matrix;
mat4	world_view_proj_matrix;
mat4	proj_matrix;

vec4	light_pos;
vec4	light_dir;
vec4	view_pos;

GLuint texture_obj,texture_bump;

float	distance_z = 182;

int		width;
int		height;

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

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	check_gl_error("glTexParameteri");

	return texId;
}
			
void opengl_init(int w, int h)
{
	/**********************************************************************************/
	CLoad3DS g_Load3ds;	
	t3DModel g_3DModel;
	
	shader_loader vs,ps;

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
	object_program.loc_light_pos = glGetUniformLocation(object_program.p_program->get_program(), "lightPos");
	object_program.loc_light_dir = glGetUniformLocation(object_program.p_program->get_program(), "lightDir");
	object_program.loc_view_pos = glGetUniformLocation(object_program.p_program->get_program(), "view_position");
	object_program.texs_locs[0] = glGetUniformLocation(object_program.p_program->get_program(), "BumpMap");
	object_program.texs_locs[1] = glGetUniformLocation(object_program.p_program->get_program(), "ObjectMap");

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

	glViewport(0,0,w,h);
	
	width = w;
	height = h;
	
	proj_matrix.perspective(60, (float)w/h, 1.0f,3000.0f);
}


void draw(program* p, static_mesh* mesh);

void opengl_display()
{
	static double start = get_time();
	float current_time = get_time()-start;

	light_dir.x = sin(current_time * 0.6f);
	light_dir.z = cos(current_time * 0.6f);

	light_pos.x = -light_dir.x * 100.f;
	light_pos.z = -light_dir.z * 100.f;
	light_pos.y = 10.f * sin(current_time * 0.5f);

	/* background and object ******************************************************************/

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	//-------------------------------------------------------------------
	view_matrix.look_at(vec3(0,0,distance_z), vec3(0,0,0), vec3(0,1,0));
	world_view_matrix = view_matrix * world_matrix;
	world_view_proj_matrix = proj_matrix * world_view_matrix;

	vec3 viewpos  = world_view_matrix.inverse() * vec3(0,0,0);
	view_pos = vec4(viewpos.x, viewpos.y, viewpos.z, 1.0);

	glUseProgram(object_program.p_program->get_program());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_bump);
	glUniform1i(object_program.texs_locs[0], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_obj);
	glUniform1i(object_program.texs_locs[1], 1);

	glUniformMatrix4fv(object_program.loc_world_view_proj, 1, GL_FALSE, &world_view_proj_matrix[0]);
	glUniform4fv(object_program.loc_light_pos, 1, &light_pos.x);
	glUniform4fv(object_program.loc_light_dir, 1, &light_dir.x);
	glUniform4fv(object_program.loc_view_pos, 1, &view_pos.x);

	draw(&object_program, &object_mesh);
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
	safely_free_pointer(object_program.p_program);

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
