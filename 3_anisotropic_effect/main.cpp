

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
	GLuint attribute_locations[3];
	GLuint texs_idxs[2];
	GLuint texs_locs[2];
	GLuint num_faces;

	// for vertex shader
	GLuint view_proj_matrix_loc;
	GLuint inv_view_matrix_loc;

	GLuint  mWorld_loc;
	GLuint  ambient_loc;
	GLuint  texture_space_matrix_loc;
	GLuint  fAmbient_loc;
	GLuint  vRingCenter_loc;
	GLuint  vRingScale_loc;
	GLuint  light2Position_loc;
	GLuint  light1Position_loc;


	//for fragment shader--------------------------------
	GLuint      lightColor_loc;
	GLuint      ringColor_loc;
	GLuint      ringAmbientColor_loc;

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


static_mesh statue_mesh;

//for vertex shader------------------------------
mat4 view_proj_matrix;
mat4 inv_view_matrix;

mat4  mWorld;
vec4  ambient=vec4(0.13f,0.16f,0.13f,1);
mat4  texture_space_matrix = mat4(1,	0,	0,	0,
								  0,	-2,	-4,	0,
								  0,	-1,	-2,	0,
								  0,	0,	0,	1);
float fAmbient = 0.1f;
vec4  vRingCenter = vec4(-1.5f, -20.f, 26.f, 1.0f);
vec4  vRingScale = vec4(0.5f, 0.25f, 0.75f, 1.0f);
vec4  light2Position = vec4(-500.f, 500.f, 1.f, 1.f);
vec4  light1Position = vec4(300.f, 300.f, 1.f, 1.f);

//for fragment shader--------------------------------
vec4      lightColor = vec4(1.0f, 0.9f, 0.8f, 1.f);
vec4      ringColor = vec4(0.9f,0.9f, 1.0f, 1.0f);
vec4      ringAmbientColor = vec4(0.2f, 0.2f, 0.3f, 1.f);

//---------------------------------------------------
float	distance_z = 150;
mat4	projection_matrix;

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

GLubyte* get_mipmap_texture(GLubyte* data, GLuint width, GLuint element_size, int level)
{

	if(width <= 1)
	{
		return NULL;
	}

	GLuint nwidth = (width>>level);

	if(nwidth < 1 )
	{
		return NULL;
	}

	GLubyte* newdata = new GLubyte[ nwidth * element_size];

	if(element_size == 3)
	{

		for(int j=1; j<nwidth-1; j++)
		{
			newdata[j*3+0] = data[(j*2+0)*3+0]*0.3+data[(j*2+1)*3+0]*0.3+data[(j*2-1)*3+0]*0.2+data[(j*2+2)*3+0]*0.2;
			newdata[j*3+1] = data[(j*2+0)*3+1]*0.3+data[(j*2+1)*3+1]*0.3+data[(j*2-1)*3+0]*0.2+data[(j*2+2)*3+1]*0.2;
			newdata[j*3+2] = data[(j*2+0)*3+2]*0.3+data[(j*2+1)*3+2]*0.3+data[(j*2-1)*3+2]*0.2+data[(j*2+2)*3+2]*0.2;
		}

		return newdata;
	}

	return NULL;
}

int create_ring_gradient(const char* file_name)
{
	image_data image;
	GLuint texId;
	image_tool::read_image(file_name, image);

	glGenTextures(1, &texId);
	glBindTexture( GL_TEXTURE_2D, texId);


	glTexImage2D( GL_TEXTURE_2D, 0, image._internalFormat, image._width, image._height, 0, image._format, image._type, image._data[0]);

	int i = 1;
	GLubyte* newdata = get_mipmap_texture(image._data[0], image._width, image._elementSize, i);
	while(newdata != NULL)
	{
		glTexImage2D( GL_TEXTURE_2D, i, image._internalFormat, image._width>>i, image._height, 0, image._format, image._type, newdata);
		delete []newdata;
		
		i++;
		newdata = get_mipmap_texture(image._data[0], image._width, image._elementSize, i);
	}

	check_gl_error("glTexImage2D");
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	check_gl_error("glTexParameteri");

	return texId;
}

void opengl_init(int w, int h)
{
	/* init statue *********************************************************************************/
	CLoad3DS g_Load3ds;	
	t3DModel g_3DModel;

	LOGI("begin load shader");
	shader_loader vs("media/statue.vs"), ps("media/statue.ps");

	statue_mesh.program = new shader_program;
	if(!statue_mesh.program->build(vs.get_shader_source(),ps.get_shader_source()))
	{
		LOGI("build shader failed");
		return;
	}
	
	statue_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(statue_mesh.program->get_program(),"rm_Vertex");
	statue_mesh.attribute_locations[static_mesh::NORMAL] = glGetAttribLocation(statue_mesh.program->get_program(),"rm_Normal");

	LOGI("begin to load 3ds file");
	g_Load3ds.Import3DS(&g_3DModel, "media/Teapot.3ds");
	LOGI("load 3ds end");

	t3DObject* p = &g_3DModel.pObject[0];

	statue_mesh.texs_idxs[0] = create_ring_gradient("media/RingGradient.tga");
	statue_mesh.texs_idxs[1] = create_texture("media/AnisoStrand.tga");

	statue_mesh.texs_locs[0] = glGetUniformLocation(statue_mesh.program->get_program(),"ring_gradient");
	statue_mesh.texs_locs[1] = glGetUniformLocation(statue_mesh.program->get_program(),"strand");

	statue_mesh.ambient_loc = glGetUniformLocation(statue_mesh.program->get_program(), "ambient");
	statue_mesh.fAmbient_loc = glGetUniformLocation(statue_mesh.program->get_program(), "fAmbient");
	statue_mesh.light1Position_loc = glGetUniformLocation(statue_mesh.program->get_program(), "light1Position");
	statue_mesh.light2Position_loc = glGetUniformLocation(statue_mesh.program->get_program(), "light2Position");
	statue_mesh.lightColor_loc = glGetUniformLocation(statue_mesh.program->get_program(), "lightColor");
	statue_mesh.ringAmbientColor_loc = glGetUniformLocation(statue_mesh.program->get_program(), "ringAmbientColor");
	statue_mesh.ringColor_loc = glGetUniformLocation(statue_mesh.program->get_program(), "ringColor");
	statue_mesh.texture_space_matrix_loc = glGetUniformLocation(statue_mesh.program->get_program(), "texture_space_matrix");
	statue_mesh.vRingCenter_loc = glGetUniformLocation(statue_mesh.program->get_program(), "vRingCenter");
	statue_mesh.vRingScale_loc = glGetUniformLocation(statue_mesh.program->get_program(), "vRingScale");

	statue_mesh.view_proj_matrix_loc = glGetUniformLocation(statue_mesh.program->get_program(), "view_proj_matrix");
	statue_mesh.inv_view_matrix_loc = glGetUniformLocation(statue_mesh.program->get_program(), "inv_view_matrix");
	statue_mesh.mWorld_loc = glGetUniformLocation(statue_mesh.program->get_program(), "mWorld");


	glGenBuffers(4, statue_mesh.attribute_vbos);

	glBindBuffer(GL_ARRAY_BUFFER, statue_mesh.attribute_vbos[static_mesh::POSITION]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(p->pVerts[0].x), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, statue_mesh.attribute_vbos[static_mesh::NORMAL]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*p->numOfVerts, &(p->pNormals[0].x), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, statue_mesh.attribute_vbos[static_mesh::INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*3*p->numOfFaces, &(p->pFaces[0].vertIndex[0]), GL_STATIC_DRAW);

	statue_mesh.num_faces = p->numOfFaces;


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnable(GL_DEPTH_TEST);
//	glEnable(GL_CULL_FACE);
	glViewport(0,0,w,h);
	projection_matrix.perspective(60, (float)w/h, 1.0f,3000.0f);

	glUseProgram(statue_mesh.program->get_program());

	glUniform4fv(statue_mesh.ambient_loc,1, &ambient[0]);
	glUniform4fv(statue_mesh.light2Position_loc,1, &light2Position[0]);
	glUniform4fv(statue_mesh.light1Position_loc,1, &light1Position[0]);
	glUniform4fv(statue_mesh.lightColor_loc,1, &lightColor[0]);
	glUniform4fv(statue_mesh.ringAmbientColor_loc, 1, &ringAmbientColor[0]);
	glUniform4fv(statue_mesh.ringColor_loc, 1, &ringColor[0]);
	glUniform4fv(statue_mesh.vRingCenter_loc,1, &vRingCenter[0]);
	glUniform4fv(statue_mesh.vRingScale_loc,1, &vRingScale[0]);
	glUniform1f(statue_mesh.fAmbient_loc,fAmbient);
	glUniformMatrix4fv(statue_mesh.texture_space_matrix_loc,1, GL_FALSE, &texture_space_matrix[0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, statue_mesh.texs_idxs[0]);
	glUniform1i(statue_mesh.texs_locs[0], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, statue_mesh.texs_idxs[1]);
	glUniform1i(statue_mesh.texs_locs[1], 1);
}



void opengl_display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	mat4 view_matrix;
	mat4 world_matrix;

	view_matrix.look_at(vec3(0,0,distance_z), vec3(0,0,0), vec3(0,1,0));
	view_matrix *= mWorld;
	view_proj_matrix = projection_matrix * view_matrix;
	inv_view_matrix = view_matrix.inverse();


	if(statue_mesh.program != NULL)
	{
		glUniformMatrix4fv(statue_mesh.mWorld_loc, 1, GL_FALSE, &world_matrix[0]);
		glUniformMatrix4fv(statue_mesh.view_proj_matrix_loc, 1, GL_FALSE, &view_proj_matrix[0]);
		glUniformMatrix4fv(statue_mesh.inv_view_matrix_loc, 1,GL_FALSE, &inv_view_matrix[0]);

		statue_mesh.draw();
	}
	
}

void opengl_end()
{
	LOGI("opengl_end()");

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

			mWorld = rotx_mat * roty_mat;

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


