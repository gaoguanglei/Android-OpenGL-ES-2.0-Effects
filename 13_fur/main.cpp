

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

	GLuint loc_world_view_proj_matrix;
	GLuint loc_pass_index;

	static_mesh()
	{
		program = NULL;
		attribute_locations[0] = -1;
		attribute_locations[1] = -1;
		attribute_locations[2] = -1;
	}

	void draw();
};



static_mesh colored_fur_mesh;
static_mesh textured_fur_mesh;

mat4	view_matrix;
mat4	world_matrix;
mat4	world_view_proj_matrix;
mat4	proj_matrix;

float	distance_z = 182;

int		width;
int		height;


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

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return texId;
}
						
void opengl_init(int w, int h)
{
	/* *********************************************************************************/
	shader_loader vs,ps;

	{
		CLoad3DS g_Load3ds;	
		t3DModel g_3DModel;

		LOGI("begin load shader");

		colored_fur_mesh.program = new shader_program;
		if(!colored_fur_mesh.program->build(vs.load_shader_source("media/colored.vs"),ps.load_shader_source("media/colored.ps")))
		{
			LOGI("build shader failed");
			return;
		}
	
		colored_fur_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(colored_fur_mesh.program->get_program(),"rm_Vertex");
		colored_fur_mesh.attribute_locations[static_mesh::NORMAL] = glGetAttribLocation(colored_fur_mesh.program->get_program(),"rm_Normal");
		colored_fur_mesh.attribute_locations[static_mesh::TEXCOORD] = glGetAttribLocation(colored_fur_mesh.program->get_program(),"rm_TexCoord0");

		LOGI("begin to load 3ds file");
		g_Load3ds.Import3DS(&g_3DModel, "media/Torus.3ds");
		LOGI("load 3ds end");

		if(g_3DModel.numOfObjects == 0)
		{
			return;
		}

		t3DObject* p = &g_3DModel.pObject[0];

		colored_fur_mesh.texs_idxs[0] = create_texture("media/Fur.tga");
		colored_fur_mesh.texs_locs[0] = glGetUniformLocation(colored_fur_mesh.program->get_program(), "Fur");
		colored_fur_mesh.loc_world_view_proj_matrix = glGetUniformLocation(colored_fur_mesh.program->get_program(), "world_view_proj_matrix");
		colored_fur_mesh.loc_pass_index = glGetUniformLocation(colored_fur_mesh.program->get_program(), "pass_index");

		glGenBuffers(4, colored_fur_mesh.attribute_vbos);

		glBindBuffer(GL_ARRAY_BUFFER, colored_fur_mesh.attribute_vbos[static_mesh::POSITION]);
		glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(p->pVerts[0].x), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, colored_fur_mesh.attribute_vbos[static_mesh::NORMAL]);
		glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(p->pNormals[0].x), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, colored_fur_mesh.attribute_vbos[static_mesh::TEXCOORD]);
		glBufferData(GL_ARRAY_BUFFER, 4*2*p->numOfVerts, &(p->pTexVerts[0].x), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, colored_fur_mesh.attribute_vbos[static_mesh::INDEX]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*3*p->numOfFaces, &(p->pFaces[0].vertIndex[0]), GL_STATIC_DRAW);

		colored_fur_mesh.num_faces = p->numOfFaces;
	}

	{

		LOGI("begin load shader");

		textured_fur_mesh.program = new shader_program;
		if(!textured_fur_mesh.program->build(vs.load_shader_source("media/textured.vs"),ps.load_shader_source("media/textured.ps")))
		{
			LOGI("build shader failed");
			return;
		}
	
		textured_fur_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(textured_fur_mesh.program->get_program(),"rm_Vertex");
		textured_fur_mesh.attribute_locations[static_mesh::NORMAL] = glGetAttribLocation(textured_fur_mesh.program->get_program(),"rm_Normal");
		textured_fur_mesh.attribute_locations[static_mesh::TEXCOORD] = glGetAttribLocation(textured_fur_mesh.program->get_program(),"rm_TexCoord0");

		textured_fur_mesh.texs_idxs[0] =colored_fur_mesh.texs_idxs[0];
		textured_fur_mesh.texs_idxs[1] = create_texture("media/rainbowfilm_smooth.tga");

		textured_fur_mesh.texs_locs[0] = glGetUniformLocation(textured_fur_mesh.program->get_program(), "Fur");
		textured_fur_mesh.texs_locs[1] = glGetUniformLocation(textured_fur_mesh.program->get_program(), "FurColor");

		textured_fur_mesh.loc_world_view_proj_matrix = glGetUniformLocation(textured_fur_mesh.program->get_program(), "world_view_proj_matrix");
		textured_fur_mesh.loc_pass_index = glGetUniformLocation(textured_fur_mesh.program->get_program(), "pass_index");

		textured_fur_mesh.attribute_vbos[0] = colored_fur_mesh.attribute_vbos[0];
		textured_fur_mesh.attribute_vbos[1] = colored_fur_mesh.attribute_vbos[1];
		textured_fur_mesh.attribute_vbos[2] = colored_fur_mesh.attribute_vbos[2];
		textured_fur_mesh.attribute_vbos[3] = colored_fur_mesh.attribute_vbos[3];

		textured_fur_mesh.num_faces = colored_fur_mesh.num_faces;
	}
	

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	
	width = w;
	height = h;
	

	proj_matrix.perspective(60, (float)w/h*0.5f, 1.0f,500.0f);
}



void opengl_display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		

	view_matrix.look_at(vec3(0,0,distance_z), vec3(0,0,0), vec3(0,1,0));

	if(colored_fur_mesh.program != NULL)
	{
		world_view_proj_matrix = proj_matrix * view_matrix * world_matrix;
		glViewport(0,0,width>>1, height);

		glUseProgram(colored_fur_mesh.program->get_program());
		glUniformMatrix4fv(colored_fur_mesh.loc_world_view_proj_matrix, 1, GL_FALSE, &world_view_proj_matrix[0]);

		glBindTexture(GL_TEXTURE_2D, colored_fur_mesh.texs_idxs[0]);
		glUniform1i(colored_fur_mesh.texs_locs[0], 0);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glUniform1f(colored_fur_mesh.loc_pass_index, 0);
		colored_fur_mesh.draw();

		glEnable(GL_BLEND);
		for(int i=1; i<15; i++)
		{
			glUniform1f(colored_fur_mesh.loc_pass_index, i);
			colored_fur_mesh.draw();
		}
	}
	if(textured_fur_mesh.program != NULL)
	{

		glViewport(width>>1,0,width>>1, height);

		glUseProgram(textured_fur_mesh.program->get_program());
		glUniformMatrix4fv(textured_fur_mesh.loc_world_view_proj_matrix, 1, GL_FALSE, &world_view_proj_matrix[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textured_fur_mesh.texs_idxs[0]);
		glUniform1i(textured_fur_mesh.texs_locs[0], 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textured_fur_mesh.texs_idxs[1]);
		glUniform1i(textured_fur_mesh.texs_locs[1], 1);

		glDisable(GL_BLEND);
		glUniform1f(textured_fur_mesh.loc_pass_index, 0);
		textured_fur_mesh.draw();

		glEnable(GL_BLEND);
		for(int i=1; i<15; i++)
		{
			glUniform1f(textured_fur_mesh.loc_pass_index, i);
			textured_fur_mesh.draw();
		}
	}
}

void opengl_end()
{
	LOGI("opengl_end()");
	if(colored_fur_mesh.program != NULL)
	{
		delete colored_fur_mesh.program;
		colored_fur_mesh.program = NULL;
	}

	if(textured_fur_mesh.program != NULL)
	{
		delete textured_fur_mesh.program;
		textured_fur_mesh.program = NULL;
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