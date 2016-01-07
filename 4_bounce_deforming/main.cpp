

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
	GLuint texs_idxs[1];
	GLuint texs_locs[1];
	GLuint num_indx;

	// for vertex shader
	GLuint loc_view_matrix;
	GLuint loc_view_proj_matrix;
	GLuint loc_matViewInverseTranspose;
	GLuint loc_localTime;

	//for fragment shader--------------------------------

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

	glDrawElements(GL_TRIANGLES, num_indx, GL_UNSIGNED_SHORT, 0);

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

//----------------------------------------------
static_mesh sphere_mesh;
static_mesh plane_mesh;

//for vertex shader------------------------------
mat4 view_matrix;
mat4 view_rot_matrix;
mat4 proj_matrix;
mat4 view_proj_matrix;
float localTime = 0;
float distance_y = 220.0f;


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
	/* init  *********************************************************************************/

	LOGI("begin load shader");
	shader_loader vs, ps;
	sphere_mesh.program = new shader_program;
	if(!sphere_mesh.program->build(vs.load_shader_source("media/sphere.vs"),ps.load_shader_source("media/sphere.ps")))
	{
		LOGI("build shader failed");
		return;
	}

	sphere_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(sphere_mesh.program->get_program(),"rm_Vertex");
	sphere_mesh.attribute_locations[static_mesh::NORMAL] = glGetAttribLocation(sphere_mesh.program->get_program(),"rm_Normal");


	sphere_mesh.loc_localTime = glGetUniformLocation(sphere_mesh.program->get_program(), "localTime");
	sphere_mesh.loc_view_matrix = glGetUniformLocation(sphere_mesh.program->get_program(), "view_matrix");
	sphere_mesh.loc_view_proj_matrix = glGetUniformLocation(sphere_mesh.program->get_program(), "view_proj_matrix");

	float*pos, *normal;
	unsigned short *indx;
	GLuint num_vertexs;
	GLuint num_index;

	num_index = shape::create_sphere(60,20, &pos, &normal, NULL, &indx, num_vertexs);

	glGenBuffers(1, &sphere_mesh.attribute_vbos[static_mesh::POSITION]);
	glGenBuffers(1, &sphere_mesh.attribute_vbos[static_mesh::NORMAL]);
	glGenBuffers(1, &sphere_mesh.attribute_vbos[static_mesh::INDEX]);

	glBindBuffer(GL_ARRAY_BUFFER, sphere_mesh.attribute_vbos[static_mesh::POSITION]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*num_vertexs, pos, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, sphere_mesh.attribute_vbos[static_mesh::NORMAL]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*num_vertexs, normal, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_mesh.attribute_vbos[static_mesh::INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*num_index, indx, GL_STATIC_DRAW);

	sphere_mesh.num_indx = num_index;

	/**********************************************************************************/
	plane_mesh.texs_idxs[0] = create_texture("media/Tiles.tga");

	plane_mesh.program = new shader_program;
	if(!plane_mesh.program->build(vs.load_shader_source("media/plane.vs"),ps.load_shader_source("media/plane.ps")))
	{
		LOGI("build shader failed");
		return;
	}

	plane_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(plane_mesh.program->get_program(),"rm_Vertex");
	plane_mesh.attribute_locations[static_mesh::NORMAL] = glGetAttribLocation(plane_mesh.program->get_program(),"rm_Normal");
	plane_mesh.attribute_locations[static_mesh::TEXCOORD] = glGetAttribLocation(plane_mesh.program->get_program(),"rm_TexCoord0");

	plane_mesh.loc_localTime = glGetUniformLocation(plane_mesh.program->get_program(), "localTime");
	plane_mesh.loc_view_matrix = glGetUniformLocation(plane_mesh.program->get_program(), "view_matrix");
	plane_mesh.loc_view_proj_matrix = glGetUniformLocation(plane_mesh.program->get_program(), "view_proj_matrix");
	plane_mesh.texs_locs[0] = glGetUniformLocation(plane_mesh.program->get_program(), "baseMap");

	float* texcoord;

	CLoad3DS g_Load3ds;	
	t3DModel g_3DModel;

	LOGI("begin to load 3ds file");
	g_Load3ds.Import3DS(&g_3DModel, "media/Terrain.3ds");
	LOGI("load 3ds end");

	t3DObject* p = &g_3DModel.pObject[0];

	glGenBuffers(4, plane_mesh.attribute_vbos);

	glBindBuffer(GL_ARRAY_BUFFER, plane_mesh.attribute_vbos[static_mesh::POSITION]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, p->pVerts, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, plane_mesh.attribute_vbos[static_mesh::NORMAL]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, p->pNormals, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, plane_mesh.attribute_vbos[static_mesh::TEXCOORD]);
	glBufferData(GL_ARRAY_BUFFER, 4*2*p->numOfVerts, p->pTexVerts, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, plane_mesh.attribute_vbos[static_mesh::INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*3*p->numOfFaces, p->pFaces->vertIndex, GL_STATIC_DRAW);

	plane_mesh.num_indx = p->numOfFaces*3;

	/************************************************************************************/
LOGI("----- 8");
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
	glViewport(0,0,w,h);

	proj_matrix.perspective(60, (float)w/h, 1.0f,8000.0f);

}


void opengl_display()
{
	static double start = get_time();
	localTime = get_time()-start;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	view_matrix.look_at(vec3(0,0,distance_y), vec3(0,0,0), vec3(0,1,0));
	view_matrix *= view_rot_matrix;
	view_proj_matrix = proj_matrix * view_matrix;
	

	if(sphere_mesh.program != NULL)
	{
		glUseProgram(sphere_mesh.program->get_program());
		glUniformMatrix4fv(sphere_mesh.loc_view_matrix, 1, GL_FALSE, &view_matrix[0]);
		glUniformMatrix4fv(sphere_mesh.loc_view_proj_matrix, 1, GL_FALSE, &view_proj_matrix[0]);
		glUniform1f(sphere_mesh.loc_localTime, localTime);

		sphere_mesh.draw();
	}

	if(plane_mesh.program != NULL)
	{
		glUseProgram(plane_mesh.program->get_program());
		glUniformMatrix4fv(plane_mesh.loc_view_matrix, 1, GL_FALSE, &view_matrix[0]);
		glUniformMatrix4fv(plane_mesh.loc_view_proj_matrix, 1, GL_FALSE, &view_proj_matrix[0]);
		glUniform1f(plane_mesh.loc_localTime, localTime);

		glBindTexture(GL_TEXTURE_2D, plane_mesh.texs_idxs[0]);
		glUniform1i(plane_mesh.texs_locs[0], 0);

		plane_mesh.draw();
	}

}

void opengl_end()
{
	LOGI("opengl_end()");

	if(sphere_mesh.program != NULL)
	{
		delete sphere_mesh.program;
		sphere_mesh.program = NULL;
	}

	if(plane_mesh.program != NULL)
	{
		delete plane_mesh.program;
		plane_mesh.program = NULL;
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

			view_rot_matrix = rotx_mat * roty_mat;

			last_x = x;
			last_y = y;
		}
		else if(down_type == 1)
		{
			float cur_dis = sqrtf((x2-x)*(x2-x)+(y2-y)+(y2-y));
			float diff = (last_dis - cur_dis);
			if(diff > 0.1f)
			{		
				distance_y *= 1.03f;
			}
			if(diff < -0.1f)
			{
				distance_y *= 0.95f;
			}
			
			if(distance_y < 1.0f)
			{
				distance_y = 1.0f;		
			}
			if(distance_y > 300.0f)
			{
				distance_y = 300.0f;
			}
			LOGI("distance=%f,cur_dis=%f,last_dis=%f", distance_y, cur_dis, last_dis);
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
		distance_y *= 1.03f;
		if(distance_y > 300.f)
		{
			distance_y = 300.f;
		}
		break;

	case 6://WM_MOUSEWHEEL
		distance_y *= 0.98f;
		if(distance_y < 1.f)
		{
			distance_y = 1.f;
		}
		break;
	}
}


