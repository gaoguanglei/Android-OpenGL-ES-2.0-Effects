

#include <platform.h>
#include <image.h>
#include <3ds.h>
#include <shader_loader.h>
#include <shader_program.h>
#include <math_lib.h>
#include <shape.h>
#include <time.h>


class vec4: public vec3
{
public:
	vec4(float a,float b, float c, float d):vec3(a,b,c), w(d){}
	float w;
};

struct static_mesh
{
	enum {POSITION, NORMAL, TEXCOORD, INDEX};
	shader_program* program;
	GLuint attribute_vbos[4];
	GLuint attribute_locations[4];
	GLuint texs_idxs[3];
	GLuint texs_locs[3];
	GLuint num_faces;

	// for vertex shader
	GLuint loc_world_view_proj_matrix;
	GLuint loc_world_view_matrix;

	GLuint loc_cos_time;
	GLuint loc_sin_time;

	static_mesh()
	{
		attribute_locations[0] = -1;
		attribute_locations[1] = -1;
		attribute_locations[2] = -1;
	}

	void draw();
};




static_mesh earth_mesh;

mat4 world_view_proj_matrix;
mat4 world_view_matrix;
mat4 projection_matrix;
mat4 view_matrix;
mat4 world_matrix;

float distance_z = 50;
float cos_time;
float sin_time;


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
	GLuint texId = 0;
	
	if(image_tool::read_image(file_name, image))
	{
		glGenTextures(1, &texId);
		glBindTexture( GL_TEXTURE_2D, texId);


		glTexImage2D( GL_TEXTURE_2D, 0, image._internalFormat, image._width, image._height, 0, image._format, image._type, image._data[0]);
		check_gl_error("glTexImage2D");
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		check_gl_error("glTexParameteri");
	}
	return texId;
}



void opengl_init(int w, int h)
{
	/* init statue *********************************************************************************/
	LOGI("begin load shader");
	shader_loader vs, ps;
	earth_mesh.program = new shader_program;
	if(!earth_mesh.program->build(vs.load_shader_source("media/earth.vs"),ps.load_shader_source("media/earth.ps")))
	{
		LOGI("build shader failed");
		return;
	}

	earth_mesh.texs_idxs[0] = create_texture("media/Earth.tga");
	earth_mesh.texs_idxs[1] = create_texture("media/EarthNight.tga");
	earth_mesh.texs_idxs[2] = create_texture("media/EarthClouds.tga");

	earth_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(earth_mesh.program->get_program(),"rm_Vertex");
	earth_mesh.attribute_locations[static_mesh::NORMAL] = glGetAttribLocation(earth_mesh.program->get_program(),"rm_Normal");
	earth_mesh.attribute_locations[static_mesh::TEXCOORD] = glGetAttribLocation(earth_mesh.program->get_program(),"rm_TexCoord0");


	earth_mesh.loc_world_view_matrix = glGetUniformLocation(earth_mesh.program->get_program(), "world_view_matrix");
	earth_mesh.loc_world_view_proj_matrix = glGetUniformLocation(earth_mesh.program->get_program(), "world_view_proj_matrix");
	earth_mesh.loc_cos_time = glGetUniformLocation(earth_mesh.program->get_program(),"cos_time_0_2PI");
	earth_mesh.loc_sin_time = glGetUniformLocation(earth_mesh.program->get_program(),"sin_time_0_2PI");

	earth_mesh.texs_locs[0] = glGetUniformLocation(earth_mesh.program->get_program(),"EarthDay");
	earth_mesh.texs_locs[1] = glGetUniformLocation(earth_mesh.program->get_program(),"EarthNight");
	earth_mesh.texs_locs[2] = glGetUniformLocation(earth_mesh.program->get_program(),"EarthCloudGloss");

	float*pos, *normal, *texcoord;
	unsigned short *indx;
	GLuint num_vertexs;
	GLuint num_index;

	num_index = shape::create_sphere(60,20, &pos, &normal, &texcoord, &indx, num_vertexs);

	glGenBuffers(4, earth_mesh.attribute_vbos);

	glBindBuffer(GL_ARRAY_BUFFER, earth_mesh.attribute_vbos[static_mesh::POSITION]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*num_vertexs, pos, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, earth_mesh.attribute_vbos[static_mesh::NORMAL]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*num_vertexs, normal, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, earth_mesh.attribute_vbos[static_mesh::TEXCOORD]);
	glBufferData(GL_ARRAY_BUFFER, 4*2*num_vertexs, texcoord, GL_STATIC_DRAW);


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, earth_mesh.attribute_vbos[static_mesh::INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*num_index, indx, GL_STATIC_DRAW);

	earth_mesh.num_faces = num_index/3;

	delete []pos;
	delete []normal;
	delete []texcoord;
	delete []indx;

	glEnable(GL_DEPTH_TEST);

	projection_matrix.perspective(90.f, (float)w/h, 1.f, 3000.f);
	
}



void opengl_display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	static double start = get_time();
	float t = get_time()-start;

	cos_time = cos(t)*0.2f;
	sin_time = sin(t)*0.2f;

	view_matrix.look_at(vec3(0,0,distance_z), vec3(0,0,0), vec3(0,1,0));
	world_view_matrix = view_matrix * world_matrix;
	world_view_proj_matrix = projection_matrix * world_view_matrix;


	if(earth_mesh.program != NULL)
	{
		glUseProgram(earth_mesh.program->get_program());
		glUniformMatrix4fv(earth_mesh.loc_world_view_matrix, 1, GL_FALSE, &world_view_matrix[0]);
		glUniformMatrix4fv(earth_mesh.loc_world_view_proj_matrix, 1, GL_FALSE, &world_view_proj_matrix[0]);
		glUniform1f(earth_mesh.loc_cos_time, cos_time);
		glUniform1f(earth_mesh.loc_sin_time, sin_time);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, earth_mesh.texs_idxs[0]);
		glUniform1i(earth_mesh.texs_locs[0], 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, earth_mesh.texs_idxs[1]);
		glUniform1i(earth_mesh.texs_locs[1], 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, earth_mesh.texs_idxs[2]);
		glUniform1i(earth_mesh.texs_locs[2], 2);

		earth_mesh.draw();
	}
	
}

void opengl_end()
{
	LOGI("opengl_end()");

	if(earth_mesh.program != NULL)
	{
		delete earth_mesh.program;
		earth_mesh.program = NULL;
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
			float cur_dis = sqrtf((x2-x)*(x2-x)+(y2-y)+(y2-y));
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
		last_dis = sqrtf((x2-x)*(x2-x)+(y2-y)+(y2-y));
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
