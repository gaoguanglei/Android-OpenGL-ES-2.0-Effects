

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
	GLuint texs_idxs[1];
	GLuint texs_locs[1];
	GLuint num_faces;

	GLuint loc_world_view_proj_mat;
	GLuint loc_world_view_mat;

	static_mesh()
	{
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


static_mesh object_mesh;

//for vertex shader------------------------------
mat4 world_view_proj_mat;
mat4 world_view_mat;
mat4 world_mat;
mat4 proj_mat;
mat4 view_mat;
float distance_z = 150;

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
	CLoad3DS g_Load3ds;	
	t3DModel g_3DModel;

	LOGI("begin load shader");
	shader_loader vs("media/statue.vs"), ps("media/statue.ps");

	object_mesh.program = new shader_program;
	if(!object_mesh.program->build(vs.get_shader_source(),ps.get_shader_source()))
	{
		LOGI("build shader failed");
		return;
	}
	
	object_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(object_mesh.program->get_program(),"rm_Vertex");
	object_mesh.attribute_locations[static_mesh::NORMAL] = glGetAttribLocation(object_mesh.program->get_program(),"rm_Normal");

	LOGI("begin to load 3ds file");
	g_Load3ds.Import3DS(&g_3DModel, "media/Teapot.3ds");
	LOGI("load 3ds end");

	t3DObject* p = &g_3DModel.pObject[0];

	object_mesh.texs_idxs[0] = create_texture("media/N2d_000.tga");

	object_mesh.texs_locs[0] = glGetUniformLocation(object_mesh.program->get_program(),"Noise2d");

	object_mesh.loc_world_view_mat = glGetUniformLocation(object_mesh.program->get_program(), "world_view_mat");
	object_mesh.loc_world_view_proj_mat = glGetUniformLocation(object_mesh.program->get_program(), "world_view_proj_mat");

	glGenBuffers(4, object_mesh.attribute_vbos);

	glBindBuffer(GL_ARRAY_BUFFER, object_mesh.attribute_vbos[static_mesh::POSITION]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(p->pVerts[0].x), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, object_mesh.attribute_vbos[static_mesh::NORMAL]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(p->pNormals[0].x), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object_mesh.attribute_vbos[static_mesh::INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*3*p->numOfFaces, &(p->pFaces[0].vertIndex[0]), GL_STATIC_DRAW);

	object_mesh.num_faces = p->numOfFaces;


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glViewport(0,0,w,h);
	proj_mat.perspective(60, (float)w/h, 1.0f,3000.0f);

}



void opengl_display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	view_mat.look_at(vec3(0,0,distance_z), vec3(0,0,0), vec3(0,1,0));
	world_view_mat = view_mat * world_mat;
	world_view_proj_mat = proj_mat * world_view_mat;
	
	if(object_mesh.program != NULL)
	{
		glUseProgram(object_mesh.program->get_program());

		glUniformMatrix4fv(object_mesh.loc_world_view_mat, 1, GL_FALSE, &world_view_mat[0]);
		glUniformMatrix4fv(object_mesh.loc_world_view_proj_mat, 1, GL_FALSE, &world_view_proj_mat[0]);
		

		glBindTexture(GL_TEXTURE_2D, object_mesh.texs_idxs[0]);
		glUniform1i(object_mesh.texs_locs[0], 0);

		object_mesh.draw();
	}
	
}

void opengl_end()
{
	LOGI("opengl_end()");

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

			world_mat = rotx_mat * roty_mat;

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


