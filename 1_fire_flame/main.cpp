

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
	GLuint attribute_locations[4];
	GLuint texs_idxs[3];
	GLuint texs_locs[3];
	GLuint num_faces;

	GLuint loc_time;

	static_mesh()
	{
		attribute_locations[0] = -1;
		attribute_locations[1] = -1;
		attribute_locations[2] = -1;
	}

	void draw();
};




static_mesh quad_mesh;
float current_time;


void check_gl_error(const char* op) 
{
	for (GLint error = glGetError(); error; error= glGetError()) 
	{		
		LOGI("after %s() glError (0x%x)\n", op, error);
	}
}

int create_texture(const char* file_name, GLuint wrap_type)
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_type);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_type);

		check_gl_error("glTexParameteri");
	}
	return texId;
}



void opengl_init(int w, int h)
{
	/* init statue *********************************************************************************/
	LOGI("begin load shader");
	shader_loader vs, ps;
	quad_mesh.program = new shader_program;
	if(!quad_mesh.program->build(vs.load_shader_source("media/flame.vs"),ps.load_shader_source("media/flame.ps")))
	{
		LOGI("build shader failed");
		return;
	}

	quad_mesh.texs_idxs[0] = create_texture("media/FireBase.tga", GL_CLAMP_TO_EDGE);
	quad_mesh.texs_idxs[1] = create_texture("media/FireDistortion.tga",GL_REPEAT);
	quad_mesh.texs_idxs[2] = create_texture("media/FireOpacity.tga",GL_CLAMP_TO_EDGE);

	quad_mesh.attribute_locations[static_mesh::POSITION] = glGetAttribLocation(quad_mesh.program->get_program(),"rm_Vertex");
	quad_mesh.attribute_locations[static_mesh::TEXCOORD] = glGetAttribLocation(quad_mesh.program->get_program(),"rm_TexCoord0");

	quad_mesh.loc_time = glGetUniformLocation(quad_mesh.program->get_program(),"time_0_X");
	quad_mesh.texs_locs[0] = glGetUniformLocation(quad_mesh.program->get_program(),"fire_base");
	quad_mesh.texs_locs[1] = glGetUniformLocation(quad_mesh.program->get_program(),"fire_distortion");
	quad_mesh.texs_locs[2] = glGetUniformLocation(quad_mesh.program->get_program(),"fire_opacity");

	float*pos, *texcoord;
	unsigned short *indx;
	GLuint num_vertexs;
	GLuint num_index;

	num_index = shape::create_plane(1, &pos, NULL, &texcoord, &indx, num_vertexs);

	glGenBuffers(4, quad_mesh.attribute_vbos);

	glBindBuffer(GL_ARRAY_BUFFER, quad_mesh.attribute_vbos[static_mesh::POSITION]);
	glBufferData(GL_ARRAY_BUFFER, 4*3*num_vertexs, pos, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, quad_mesh.attribute_vbos[static_mesh::TEXCOORD]);
	glBufferData(GL_ARRAY_BUFFER, 4*2*num_vertexs, texcoord, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad_mesh.attribute_vbos[static_mesh::INDEX]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2*num_index, indx, GL_STATIC_DRAW);

	quad_mesh.num_faces = num_index/3;

	delete []pos;
	delete []texcoord;
	delete []indx;

	glViewport(0,0,w,h);
}



void opengl_display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

	static double start = get_time();
	current_time = get_time()-start;


	if(quad_mesh.program != NULL)
	{
		glUseProgram(quad_mesh.program->get_program());

		glUniform1f(quad_mesh.loc_time, current_time);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, quad_mesh.texs_idxs[0]);
		glUniform1i(quad_mesh.texs_locs[0], 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, quad_mesh.texs_idxs[1]);
		glUniform1i(quad_mesh.texs_locs[1], 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, quad_mesh.texs_idxs[2]);
		glUniform1i(quad_mesh.texs_locs[2], 2);

		quad_mesh.draw();
	}
	
}

void opengl_end()
{
	LOGI("opengl_end()");

	if(quad_mesh.program != NULL)
	{
		delete quad_mesh.program;
		quad_mesh.program = NULL;
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
