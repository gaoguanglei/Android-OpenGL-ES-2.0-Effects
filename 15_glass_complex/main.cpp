

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
	GLuint wvp_loc;
	GLuint world_loc;
	GLuint env_inv_world_loc;
	GLuint eye_loc;

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


static_mesh background_mesh;
static_mesh statue_mesh;

mat4	background_world;
mat4	statue_world;
mat4	projection;
mat4	wvp;
mat4	view;
mat4	env_inv_world;

float	distance_z = 120;

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
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
	/* init statue *********************************************************************************/

	shader_loader vs,ps;
	if(statue_mesh.program == NULL)
	{
		LOGI("begin load shader");

		statue_mesh.program = new shader_program;
		if(!statue_mesh.program->build(vs.load_shader_source("media/statue.vs"),ps.load_shader_source("media/statue.ps")))
		{
			LOGI("build shader failed");
			return;
		}
	
		statue_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(statue_mesh.program->get_program(),"a_position");
		statue_mesh.attribute_locations[static_mesh::TEXCOORD] = glGetAttribLocation(statue_mesh.program->get_program(),"a_texcoord");
		statue_mesh.attribute_locations[static_mesh::NORMAL] = glGetAttribLocation(statue_mesh.program->get_program(),"a_normal");

		float* normal, *pos, *texcoord;
		unsigned short* indxs;
		GLuint num_vers, num_indx;
		num_indx = shape::create_sphere(20, 15.0, &pos, &normal, &texcoord, &indxs, num_vers);

		statue_mesh.texs_idxs[0] = create_texture("media/Rainbow.tga");
		statue_mesh.texs_idxs[1] = create_cube_texture("media/Snow.dds");

		statue_mesh.texs_locs[0] = glGetUniformLocation(statue_mesh.program->get_program(),"s_diffuse");
		statue_mesh.texs_locs[1] = glGetUniformLocation(statue_mesh.program->get_program(),"s_enviroment");

		statue_mesh.wvp_loc = glGetUniformLocation(statue_mesh.program->get_program(), "u_wvp");
		statue_mesh.world_loc = glGetUniformLocation(statue_mesh.program->get_program(), "u_world_mat");
		statue_mesh.env_inv_world_loc = glGetUniformLocation(statue_mesh.program->get_program(), "u_env_inv_world");
		statue_mesh.eye_loc = glGetUniformLocation(statue_mesh.program->get_program(), "eye");

		glGenBuffers(4, statue_mesh.attribute_vbos);

		glBindBuffer(GL_ARRAY_BUFFER, statue_mesh.attribute_vbos[static_mesh::TEXCOORD]);
		glBufferData(GL_ARRAY_BUFFER, 4*2*num_vers, texcoord, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, statue_mesh.attribute_vbos[static_mesh::POSITION]);
		glBufferData(GL_ARRAY_BUFFER, 4*3*num_vers, pos, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, statue_mesh.attribute_vbos[static_mesh::NORMAL]);
		glBufferData(GL_ARRAY_BUFFER, 4*3*num_vers, normal, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, statue_mesh.attribute_vbos[static_mesh::INDEX]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*num_indx, indxs, GL_STATIC_DRAW);

		statue_mesh.num_faces = num_indx/3;
	}

	/* init background *********************************************************************************/
	
	if(background_mesh.program == NULL)
	{
		float *pos_data, *tex_data, *normal_data;
		GLushort *inds;
		GLuint num_ves;

		background_mesh.program = new shader_program;
		if(!background_mesh.program->build(vs.load_shader_source("media/background.vs"),ps.load_shader_source("media/background.ps")))
		{
			LOGI("build shader failed");
			return;
		}


		shape::create_cube(1000.0f, &pos_data, NULL, NULL, &inds, num_ves);

		background_mesh.wvp_loc = glGetUniformLocation(background_mesh.program->get_program(), "u_matViewProjection");

		background_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(background_mesh.program->get_program(),"a_position");

		glGenBuffers(4, background_mesh.attribute_vbos);

		glBindBuffer(GL_ARRAY_BUFFER, background_mesh.attribute_vbos[static_mesh::POSITION]);
		glBufferData(GL_ARRAY_BUFFER, num_ves*4*3, pos_data, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, background_mesh.attribute_vbos[static_mesh::INDEX]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12*2*3, inds, GL_STATIC_DRAW);

		background_mesh.texs_locs[0] = glGetUniformLocation(background_mesh.program->get_program(),"s_enviroment");
		background_mesh.texs_idxs[0] = statue_mesh.texs_idxs[1];
		background_mesh.num_faces = 12;

		delete []pos_data;
		delete []inds;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);

	
	glViewport(0,0,w,h);
	projection.perspective(60, (float)w/h, 1.0f,3000.0f);

	
}



void opengl_display()
{
	static double start = get_time();
	float t = get_time()-start;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	
	t *= 25.f;


	
//	background_world.rotate_y(t);
	background_world = statue_world;

	vec3 eye = vec3(0,0,distance_z);
	view.look_at(eye, vec3(0,0,0), vec3(0,1,0));

	
	//render background
	wvp = projection * background_world;

	if(background_mesh.program != NULL)
	{
		glUseProgram(background_mesh.program->get_program());
		glUniformMatrix4fv(background_mesh.wvp_loc, 1, GL_FALSE, &wvp[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, background_mesh.texs_idxs[0]);
		glUniform1i(background_mesh.texs_locs[0], 0);

		glUniformMatrix4fv(background_mesh.wvp_loc, 1, GL_FALSE, &wvp[0]);

		background_mesh.draw();
	}

	//render statue

	env_inv_world = background_world.inverse();
	wvp = projection * view;
	wvp *= statue_world;
	eye = env_inv_world * eye;


	if(statue_mesh.program != NULL)
	{

		glUseProgram(statue_mesh.program->get_program());
		glUniformMatrix4fv(statue_mesh.wvp_loc, 1, GL_FALSE, &wvp[0]);
		glUniformMatrix4fv(statue_mesh.world_loc, 1, GL_FALSE, &statue_world[0]);
		glUniformMatrix4fv(statue_mesh.env_inv_world_loc, 1,GL_FALSE, &env_inv_world[0]);
		glUniform3fv(statue_mesh.eye_loc, 1, &eye.x);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, statue_mesh.texs_idxs[0]);
		glUniform1i(statue_mesh.texs_locs[0], 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, statue_mesh.texs_idxs[1]);
		glUniform1i(statue_mesh.texs_locs[1], 1);

		statue_mesh.draw();

	}
}

void opengl_end()
{
	LOGI("opengl_end()");
	if(background_mesh.program != NULL)
	{
		delete background_mesh.program;
		background_mesh.program = NULL;
	}

	if(statue_mesh.program != NULL)
	{
		delete statue_mesh.program;
		statue_mesh.program = NULL;
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

			statue_world = rotx_mat * roty_mat;

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


