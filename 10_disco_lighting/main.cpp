

#include <platform.h>
#include <image.h>
#include <3ds.h>
#include <shader_loader.h>
#include <shader_program.h>
#include <math_lib.h>
#include <shape.h>



struct static_mesh
{
	enum {POSITION, NORMAL, TEXCOORD, INDEX,TANGENT,BINORMAL};
	shader_program* program;
	GLuint attribute_vbos[6];
	GLuint attribute_locations[6];
	GLuint texs_idxs[3];
	GLuint texs_locs[3];
	GLuint num_faces;

	GLuint loc_view_proj_matrix;
	GLuint loc_view_matrix;
	GLuint loc_inv_view_matrix;
	GLuint loc_light_matrix;
	GLuint loc_inv_world_matrix;

	static_mesh()
	{
		program = NULL;
		attribute_locations[0] = -1;
		attribute_locations[1] = -1;
		attribute_locations[2] = -1;
		attribute_locations[3] = -1;
		attribute_locations[4] = -1;
		attribute_locations[5] = -1;
	}

	void draw();
};

static_mesh wall_mesh;
static_mesh object_mesh;
static_mesh light_mesh;

mat4	light_matrix;
mat4	world_matrix;
mat4	view_matrix;
mat4	inv_view_matrix;
mat4	projection_matrix;
mat4	view_proj_matrix;

float	distance_z = 80.f;


int has_extension(const char* name) 
{
	const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
	const char* end = extensions + strlen(extensions);
	size_t name_length = strlen(name);
	while( extensions < end ) 
	{
		size_t position = strchr(extensions, ' ') - extensions;
		if( position == name_length &&
			strncmp(extensions, name, position) == 0 )
			return 1;
		extensions += (position + 1);
	}
	return 0;
}


void check_gl_error(const char* op) 
{
	for (GLint error = glGetError(); error; error= glGetError()) 
	{		
		LOGI("after %s() glError (0x%x)\n", op, error);
	}
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

int create_texture(const char* file_name)
{
	image_data image;
	GLuint texId = 0;

	bool compressed = false;

	LOGI("debug_bmp %s, create_texture", file_name);
	
	if(image_tool::read_image(file_name, image))
	{
		if(image._internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
			|| image._internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
			|| image._internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
		{
			compressed = true;
		}

		glGenTextures(1, &texId);
		glBindTexture( GL_TEXTURE_2D, texId);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if(!compressed)
		{
			glTexImage2D( GL_TEXTURE_2D, 0, image._internalFormat, image._width, image._height, 0, image._format, image._type, image._data[0]);
			check_gl_error("glTexImage2D");
		}
		else
		{

			glCompressedTexImage2D( 
				GL_TEXTURE_2D,
				0,
				image._internalFormat,
				image._width, 
				image._height, 
				0, 
				get_image_size(0,image), 
				image._data[0]);
		}
		check_gl_error("glCompressedTexImage2D");

	}
	return texId;
}


int create_cube_texture(const char* file_name)
{
	image_data image;
	GLuint texId;
	image_tool::read_image(file_name, image);

	LOGI("debug_bmp %s, create_cube_texture", file_name);

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
			check_gl_error("glTexImage2D");
		}
		else
		{
			LOGI("debug_bmp %s, glCompressedTexImage2D", file_name);
			glCompressedTexImage2D( 
				GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,
				0,
				image._internalFormat,
				image._width, 
				image._height, 
				0, 
				get_image_size(0,image), 
				image._data[i*image._levelCount]);

				check_gl_error("glCompressedTexImage2D");
		}
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	return texId;
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
	}
}
								
void opengl_init(int w, int h)
{
	/* init statue *********************************************************************************/
	
	const GLubyte *temp = glGetString(GL_EXTENSIONS);
	int compressed_format[32];
	int num;
	glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &num);
	glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, compressed_format);


	CLoad3DS g_Load3ds;	
	t3DModel g_3DModel;
	
	shader_loader vs,ps;


	/* init background *********************************************************************************/
	
	if(wall_mesh.program == NULL)
	{
		float *pos_data, *tex_data, *normal_data;
		GLushort *inds;
		GLuint num_ves;

		wall_mesh.program = new shader_program;
		if(!wall_mesh.program->build(vs.load_shader_source("media/wall.vs"),ps.load_shader_source("media/wall.ps")))
		{
			LOGI("build shader failed");
			return;
		}

		shape::create_cube(300, &pos_data, &normal_data, NULL, &inds, num_ves);

		wall_mesh.loc_view_proj_matrix = glGetUniformLocation(wall_mesh.program->get_program(), "view_proj_matrix");
		wall_mesh.loc_inv_view_matrix = glGetUniformLocation(wall_mesh.program->get_program(), "inv_view_matrix");
		wall_mesh.loc_light_matrix = glGetUniformLocation(wall_mesh.program->get_program(), "light_matrix");

		wall_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(wall_mesh.program->get_program(),"rm_Vertex");
		wall_mesh.attribute_locations[static_mesh::NORMAL] = glGetAttribLocation(wall_mesh.program->get_program(),"rm_Normal");

		glGenBuffers(4, wall_mesh.attribute_vbos);

		glBindBuffer(GL_ARRAY_BUFFER, wall_mesh.attribute_vbos[static_mesh::POSITION]);
		glBufferData(GL_ARRAY_BUFFER, num_ves*4*3, pos_data, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, wall_mesh.attribute_vbos[static_mesh::NORMAL]);
		glBufferData(GL_ARRAY_BUFFER, num_ves*4*3, normal_data, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wall_mesh.attribute_vbos[static_mesh::INDEX]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12*2*3, inds, GL_STATIC_DRAW);

		wall_mesh.texs_idxs[0] = create_texture("media/Tiles.dds");
		wall_mesh.texs_idxs[1] = create_texture("media/TilesDOT3.tga");
		wall_mesh.texs_idxs[2] = create_cube_texture("media/LightCube.dds");

		wall_mesh.texs_locs[0] = glGetUniformLocation(wall_mesh.program->get_program(), "Base2");
		wall_mesh.texs_locs[1] = glGetUniformLocation(wall_mesh.program->get_program(), "Bump2");
		wall_mesh.texs_locs[2] = glGetUniformLocation(wall_mesh.program->get_program(), "LightCube");

		wall_mesh.num_faces = 12;

		delete []pos_data;
		delete []inds;
		delete []normal_data;
	}

	/*****************************************************************************************************/
	if(object_mesh.program == NULL)
	{
		LOGI("begin load shader");

		object_mesh.program = new shader_program;
		if(!object_mesh.program->build(vs.load_shader_source("media/object.vs"),ps.load_shader_source("media/object.ps")))
		{
			LOGI("build shader failed");
			return;
		}

		object_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(object_mesh.program->get_program(),"rm_Vertex");
		object_mesh.attribute_locations[static_mesh::TEXCOORD] = glGetAttribLocation(object_mesh.program->get_program(),"rm_TexCoord0");
		object_mesh.attribute_locations[static_mesh::NORMAL] = glGetAttribLocation(object_mesh.program->get_program(),"rm_Normal");
		object_mesh.attribute_locations[static_mesh::TANGENT] = glGetAttribLocation(object_mesh.program->get_program(),"rm_Tangent");
		object_mesh.attribute_locations[static_mesh::BINORMAL] = glGetAttribLocation(object_mesh.program->get_program(),"rm_Binormal");

		LOGI("begin to load 3ds file");
		g_Load3ds.Import3DS(&g_3DModel, "media/CrackedQuad.3ds");
		LOGI("load 3ds end");

		t3DObject* p = &g_3DModel.pObject[0];

		object_mesh.texs_idxs[0] = create_texture("media/Fieldstone.tga");
		object_mesh.texs_idxs[1] = wall_mesh.texs_idxs[1];
		object_mesh.texs_idxs[2] = wall_mesh.texs_idxs[2];

		object_mesh.texs_locs[0] = glGetUniformLocation(object_mesh.program->get_program(),"Base2");
		object_mesh.texs_locs[1] = glGetUniformLocation(object_mesh.program->get_program(),"Bump2");
		object_mesh.texs_locs[2] = glGetUniformLocation(object_mesh.program->get_program(),"LightCube");

		object_mesh.loc_view_proj_matrix = glGetUniformLocation(object_mesh.program->get_program(), "view_proj_matrix");
		object_mesh.loc_inv_view_matrix = glGetUniformLocation(object_mesh.program->get_program(), "inv_view_matrix");
		object_mesh.loc_light_matrix = glGetUniformLocation(object_mesh.program->get_program(), "light_matrix");
		object_mesh.loc_inv_world_matrix = glGetUniformLocation(object_mesh.program->get_program(), "inv_world_matrix");

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

		glBindBuffer(GL_ARRAY_BUFFER, object_mesh.attribute_vbos[static_mesh::TEXCOORD]);
		glBufferData(GL_ARRAY_BUFFER, 4*2*p->numOfVerts, &(p->pTexVerts[0].x), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, object_mesh.attribute_vbos[static_mesh::POSITION]);
		glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(p->pVerts[0].x), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, object_mesh.attribute_vbos[static_mesh::NORMAL]);
		glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(p->pNormals[0].x), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, object_mesh.attribute_vbos[static_mesh::TANGENT]);
		glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(targents[0].x), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, object_mesh.attribute_vbos[static_mesh::BINORMAL]);
		glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(binormal[0].x), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object_mesh.attribute_vbos[static_mesh::INDEX]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*3*p->numOfFaces, &(p->pFaces[0].vertIndex[0]), GL_STATIC_DRAW);

		object_mesh.num_faces = p->numOfFaces;

		delete []targents;
		delete []binormal;

	}
	/****************************************************************************************/
	if(light_mesh.program == NULL)
	{
		float *pos_data;
		GLushort *inds;
		GLuint num_ves;

		light_mesh.program = new shader_program;
		if(!light_mesh.program->build(vs.load_shader_source("media/light.vs"),ps.load_shader_source("media/light.ps")))
		{
			LOGI("build shader failed");
			return;
		}

		int num_index = shape::create_sphere(30, 1, &pos_data, NULL, NULL, &inds, num_ves);

		light_mesh.loc_view_proj_matrix = glGetUniformLocation(light_mesh.program->get_program(), "view_proj_matrix");
		light_mesh.loc_light_matrix = glGetUniformLocation(light_mesh.program->get_program(), "light_matrix");

		light_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(light_mesh.program->get_program(),"rm_Vertex");

		glGenBuffers(1, &light_mesh.attribute_vbos[static_mesh::POSITION]);
		glGenBuffers(1, &light_mesh.attribute_vbos[static_mesh::INDEX]);


		glBindBuffer(GL_ARRAY_BUFFER, light_mesh.attribute_vbos[static_mesh::POSITION]);
		glBufferData(GL_ARRAY_BUFFER, num_ves*4*3, pos_data, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, light_mesh.attribute_vbos[static_mesh::INDEX]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_index * 2, inds, GL_STATIC_DRAW);

		light_mesh.texs_idxs[0] = wall_mesh.texs_idxs[2];
		light_mesh.texs_locs[0] = glGetUniformLocation(light_mesh.program->get_program(), "LightCube");
		light_mesh.num_faces = num_index/3;

		delete []pos_data;
		delete []inds;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0,1,1,0);
	glViewport(0,0,w,h);
	projection_matrix.perspective(60, (float)w/h, 1.0f,3000.0f);
}



void opengl_display()
{
	static double start = get_time();
	float t = get_time()-start;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	
	mat4 rot_mat;
	rot_mat.rotate_x(t*30.f);
	light_matrix= rot_mat;

	rot_mat.rotate_y(t*25.45f);
	light_matrix *= rot_mat;

	rot_mat.rotate_z(t*50.3f);
	light_matrix *= rot_mat;

	view_matrix.look_at(vec3(0,distance_z,80), vec3(0,0,0), vec3(0,0,1));
	view_matrix *= world_matrix;

	view_proj_matrix = projection_matrix * view_matrix;
	inv_view_matrix = view_matrix.inverse();


	//render wall
	if(wall_mesh.program != NULL)
	{
		glUseProgram(wall_mesh.program->get_program());
		glUniformMatrix4fv(wall_mesh.loc_light_matrix, 1, GL_FALSE, &light_matrix[0]);
		glUniformMatrix4fv(wall_mesh.loc_inv_view_matrix, 1, GL_FALSE, &inv_view_matrix[0]);
		glUniformMatrix4fv(wall_mesh.loc_view_proj_matrix, 1, GL_FALSE, &view_proj_matrix[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wall_mesh.texs_idxs[0]);
		glUniform1i(wall_mesh.texs_locs[0], 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, wall_mesh.texs_idxs[1]);
		glUniform1i(wall_mesh.texs_locs[1], 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, wall_mesh.texs_idxs[2]);
		glUniform1i(wall_mesh.texs_locs[2], 2);

		wall_mesh.draw();
	}

	//render ground
	if(object_mesh.program != NULL)
	{
		glUseProgram(object_mesh.program->get_program());
		glUniformMatrix4fv(object_mesh.loc_light_matrix, 1, GL_FALSE, &light_matrix[0]);
		glUniformMatrix4fv(object_mesh.loc_inv_view_matrix, 1, GL_FALSE, &inv_view_matrix[0]);
		glUniformMatrix4fv(object_mesh.loc_view_proj_matrix, 1, GL_FALSE, &view_proj_matrix[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, object_mesh.texs_idxs[0]);
		glUniform1i(object_mesh.texs_locs[0], 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, object_mesh.texs_idxs[1]);
		glUniform1i(object_mesh.texs_locs[1], 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, object_mesh.texs_idxs[2]);
		glUniform1i(object_mesh.texs_locs[2], 2);

		object_mesh.draw();
	}

	if(light_mesh.program != NULL)
	{
		glUseProgram(light_mesh.program->get_program());
		glUniformMatrix4fv(light_mesh.loc_light_matrix, 1, GL_FALSE, &light_matrix[0]);
		glUniformMatrix4fv(light_mesh.loc_view_proj_matrix, 1, GL_FALSE, &view_proj_matrix[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, light_mesh.texs_idxs[0]);
		glUniform1i(light_mesh.texs_locs[0], 0);

		light_mesh.draw();
	}


}

void opengl_end()
{
	LOGI("opengl_end()");
	if(wall_mesh.program != NULL)
	{
		delete wall_mesh.program;
		wall_mesh.program = NULL;
	}

	if(object_mesh.program != NULL)
	{
		delete object_mesh.program;
		object_mesh.program = NULL;
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
	if(attribute_locations[static_mesh::TANGENT] != -1)
	{
		glEnableVertexAttribArray(attribute_locations[static_mesh::TANGENT]);
		glBindBuffer(GL_ARRAY_BUFFER, attribute_vbos[static_mesh::TANGENT]);
		glVertexAttribPointer(attribute_locations[static_mesh::TANGENT], 3, GL_FLOAT, GL_FALSE, 12, 0);
	}
	if(attribute_locations[static_mesh::BINORMAL] != -1)
	{
		glEnableVertexAttribArray(attribute_locations[static_mesh::BINORMAL]);
		glBindBuffer(GL_ARRAY_BUFFER, attribute_vbos[static_mesh::BINORMAL]);
		glVertexAttribPointer(attribute_locations[static_mesh::BINORMAL], 3, GL_FLOAT, GL_FALSE, 12, 0);
	}

	glDrawElements(GL_TRIANGLES,num_faces*3, GL_UNSIGNED_SHORT, 0);

	if(attribute_locations[static_mesh::TANGENT] != -1)
	{
		glDisableVertexAttribArray(attribute_locations[static_mesh::TANGENT]);
	}
	if(attribute_locations[static_mesh::BINORMAL] != -1)
	{
		glDisableVertexAttribArray(attribute_locations[static_mesh::BINORMAL]);
	}

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
