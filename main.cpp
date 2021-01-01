#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <stddef.h> /*for function: offsetof */
#include <math.h>
#include <string.h>
#include "../GL/glew.h"
#include "../GL/glut.h"
#include "../shader_lib/shader.h"
#include "glm/glm.h"
#include "glm/glm.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
extern "C"
{
#include "glm_helper.h"
}

struct Vertex
{
	GLfloat position[3];
	GLfloat normal[3];
	GLfloat texcoord[2];
	GLfloat tangent[3];
	GLfloat bitangent[3];
};
typedef struct Vertex Vertex;

void init(void);
void shaderInit(char*, char*);
void display(void);
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void keyboardup(unsigned char key, int x, int y);
void motion(int x, int y);
void mouse(int button, int state, int x, int y);
void idle(void);
void draw_light_bulb(void);
void camera_light_ball_move();
char* readShader(char*);
unsigned int loadTexture(const char* path);

namespace
{
	char* obj_file_dir = "../Resources/Sphere.obj";
	char* main_tex_dir = "../Resources/rock.jpg";
	char* normal_dir = "../Resources/rock_normal.jpg";
	char* height_dir = "../Resources/rock_height.png";

	GLfloat light_rad = 0.05; //radius of the light bulb
	float eyet = 0.0; //theta in degree
	float eyep = 90.0; //phi in degree
	bool mleft = false;
	bool mright = false;
	bool mmiddle = false;
	bool forward = false;
	bool backward = false;
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
	bool lforward = false;
	bool lbackward = false;
	bool lleft = false;
	bool lright = false;
	bool lup = false;
	bool ldown = false;
	bool bforward = false;
	bool bbackward = false;
	bool bleft = false;
	bool bright = false;
	bool bup = false;
	bool bdown = false;
	bool bx = false;
	bool by = false;
	bool bz = false;
	bool brx = false;
	bool bry = false;
	bool brz = false;

	int mousex = 0;
	int mousey = 0;
}

// You can modify the moving/rotating speed if it's too fast/slow for you
const float speed = 0.03; // camera / light / ball moving speed
const float rotation_speed = 0.05; // ball rotating speed

//you may need to use some of the following variables in your program 

GLuint mainTextureID;
GLuint normalTextureID;
GLuint heightTextureID;

GLMmodel* model;
GLuint vboIds;
char* vertShaderPath = "../Shaders/Parallax.vert";
char* fragShaderPath = "../Shaders/Parallax.frag";

int shaderProgram;
float eyex = 0.0;
float eyey = 0.0;
float eyez = 5.6;

GLfloat light_pos[] = { 1.1, 1.0, 1.3 };
GLfloat ball_pos[] = { 0.0, 0.0, 0.0 };
GLfloat ball_rot[] = { 0.0, 0.0, 0.0 };

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	// remember to replace "YourStudentID" with your own student ID
	glutCreateWindow("RTR_HW2_0856631");
	glutReshapeWindow(512, 512);

	glewInit();

	init();
	shaderInit(vertShaderPath, fragShaderPath);

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardup);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	glutMainLoop();

	glmDelete(model);
	return 0;
}

void init(void)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glEnable(GL_CULL_FACE);				//清除看不到的面

	model = glmReadOBJ(obj_file_dir);

	mainTextureID = loadTexture(main_tex_dir);
	normalTextureID = loadTexture(normal_dir);
	heightTextureID = loadTexture(height_dir);

	glmUnitize(model);										//Normalize the model to the 3D space which is centered on the origin
	glmFacetNormals(model);
	glmVertexNormals(model, 90.0, GL_FALSE);
	glEnable(GL_DEPTH_TEST);
	print_model_info(model);

	const int vertex_count = model->numvertices;
	int triangles_count = model->numtriangles;

	Vertex* v = new Vertex[triangles_count * 3];
	GLMtriangle* triangles = model->triangles;

	for (int i = 0; i < triangles_count; i++) {
		int vindex[3] = { model->triangles[i].vindices[0], model->triangles[i].vindices[1], model->triangles[i].vindices[2] };
		int nindex[3] = { model->triangles[i].nindices[0], model->triangles[i].nindices[1], model->triangles[i].nindices[2] };
		int tindex[3] = { model->triangles[i].tindices[0], model->triangles[i].tindices[1], model->triangles[i].tindices[2] };

		for (int j = 0; j < 3; j++) {
			//Position
			v[i * 3 + j].position[0] = model->vertices[vindex[j] * 3 + 0];		//X軸座標
			v[i * 3 + j].position[1] = model->vertices[vindex[j] * 3 + 1];		//Y軸座標
			v[i * 3 + j].position[2] = model->vertices[vindex[j] * 3 + 2];		//Z軸座標

			//Normal
			v[i * 3 + j].normal[0] = model->normals[nindex[j] * 3 + 0];
			v[i * 3 + j].normal[1] = model->normals[nindex[j] * 3 + 1];
			v[i * 3 + j].normal[2] = model->normals[nindex[j] * 3 + 2];

			//TextureCoord
			v[i * 3 + j].texcoord[0] = model->texcoords[tindex[j] * 2 + 0];
			v[i * 3 + j].texcoord[1] = model->texcoords[tindex[j] * 2 + 1];
		}

		glm::vec3 edge1, edge2;
		glm::vec2 deltaUV1, deltaUV2;

		edge1.x = v[i * 3 + 1].position[0] - v[i * 3].position[0];
		edge1.y = v[i * 3 + 1].position[1] - v[i * 3].position[1];
		edge1.z = v[i * 3 + 1].position[2] - v[i * 3].position[2];

		edge2.x = v[i * 3 + 2].position[0] - v[i * 3].position[0];
		edge2.y = v[i * 3 + 2].position[1] - v[i * 3].position[1];
		edge2.z = v[i * 3 + 2].position[2] - v[i * 3].position[2];

		deltaUV1.x = v[i * 3 + 1].texcoord[0] - v[i * 3].texcoord[0];
		deltaUV1.y = v[i * 3 + 1].texcoord[1] - v[i * 3].texcoord[1];

		deltaUV2.x = v[i * 3 + 2].texcoord[0] - v[i * 3].texcoord[0];
		deltaUV2.y = v[i * 3 + 2].texcoord[1] - v[i * 3].texcoord[1];

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		glm::vec3 tmp;
		for (int j = 0; j < 3; j++) {
			v[i * 3 + j].tangent[0] = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			v[i * 3 + j].tangent[1] = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			v[i * 3 + j].tangent[2] = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

			v[i * 3 + j].bitangent[0] = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
			v[i * 3 + j].bitangent[1] = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
			v[i * 3 + j].bitangent[2] = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		}
	}

	glGenBuffers(1, &vboIds);
	glBindBuffer(GL_ARRAY_BUFFER, vboIds);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * triangles_count * 3, v, GL_STATIC_DRAW);

	// position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0); // stride 0 for tightly packed

	// normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal))); // stride 0 for tightly packed

	// texture coordinates
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texcoord))); // stride 0 for tightly packed

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, tangent)));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, bitangent)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);				//解bind

	delete[]v;
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);								//開始操作MV
	glLoadIdentity();
	gluLookAt(
		eyex,
		eyey,
		eyez,
		eyex + cos(eyet * M_PI / 180) * cos(eyep * M_PI / 180),
		eyey + sin(eyet * M_PI / 180),
		eyez - cos(eyet * M_PI / 180) * sin(eyep * M_PI / 180),
		0.0,
		1.0,
		0.0);

	float viewMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, viewMatrix);

	GLint v = glGetUniformLocation(shaderProgram, "viewMatrix");
	glUniformMatrix4fv(v, 1, GL_FALSE, viewMatrix);

	glUseProgram(NULL);
	draw_light_bulb();
	glPushMatrix();
	glTranslatef(ball_pos[0], ball_pos[1], ball_pos[2]);
	glRotatef(ball_rot[0], 1, 0, 0);
	glRotatef(ball_rot[1], 0, 1, 0);
	glRotatef(ball_rot[2], 0, 0, 1);

	glEnable(GL_TEXTURE_2D);

	float modelViewMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelViewMatrix);

	float projectMatrix[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projectMatrix);

	glUseProgram(shaderProgram);

	GLint mv = glGetUniformLocation(shaderProgram, "modelView");
	glUniformMatrix4fv(mv, 1, GL_FALSE, modelViewMatrix);

	GLint p = glGetUniformLocation(shaderProgram, "projection");
	glUniformMatrix4fv(p, 1, GL_FALSE, projectMatrix);

	GLint tex = glGetUniformLocation(shaderProgram, "diffuseMap");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mainTextureID);
	glUniform1i(tex, 0);

	GLint normalMap = glGetUniformLocation(shaderProgram, "normalMap");
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normalTextureID);
	glUniform1i(normalMap, 1);

	GLint heightMap = glGetUniformLocation(shaderProgram, "depthMap");
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, heightTextureID);
	glUniform1i(heightMap, 2);

	GLint light = glGetUniformLocation(shaderProgram, "lightPosition");
	glUniform3fv(light, 1, light_pos);

	GLint eyeX = glGetUniformLocation(shaderProgram, "eyeX");
	glUniform1f(eyeX, eyex);

	GLint eyeY = glGetUniformLocation(shaderProgram, "eyeY");
	glUniform1f(eyeY, eyey);

	GLint eyeZ = glGetUniformLocation(shaderProgram, "eyeZ");
	glUniform1f(eyeZ, eyez);


	glBindVertexArray(vboIds);
	glDrawArrays(GL_TRIANGLES, 0, model->numtriangles * 3);
	glBindTexture(GL_TEXTURE_2D, NULL);

	glPopMatrix();

	glutSwapBuffers();
	camera_light_ball_move();
}

void shaderInit(char* vShader, char* fShader) {
	int  Success;												//檢查是否Shader讀取成功
	char infoLog[512];											//接錯誤訊息
	unsigned int vertexShader;
	unsigned int fragmentShader;

	char* vertexShaderSource = readShader(vShader);
	char* fragmentShaderSource = readShader(fShader);

	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//Check the correctness of VertexShader Compilation
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &Success);
	if (!Success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//Check the correctness of FragmentShader Compilation
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &Success);
	if (!Success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	// check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

}

char* readShader(char* shaderPath) {
	FILE* fp;
	fp = fopen(shaderPath, "r");
	char* buffer = (char*)malloc(sizeof(char) * 4096);
	char* data = (char*)malloc(sizeof(char) * 4096);
	buffer[0] = '\0';
	data[0] = '\0';

	if (fp == NULL) {
		std::cout << "Error" << std::endl;
	}

	while (fgets(buffer, 4096, fp) != NULL) {
		strcat(data, buffer);
	}
	free(buffer);
	fclose(fp);

	return data;
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27:
	{	// ESC
		break;
	}
	case 'd':
	{
		right = true;
		break;
	}
	case 'a':
	{
		left = true;
		break;
	}
	case 'w':
	{
		forward = true;
		break;
	}
	case 's':
	{
		backward = true;
		break;
	}
	case 'q':
	{
		up = true;
		break;
	}
	case 'e':
	{
		down = true;
		break;
	}
	case 't':
	{
		lforward = true;
		break;
	}
	case 'g':
	{
		lbackward = true;
		break;
	}
	case 'h':
	{
		lright = true;
		break;
	}
	case 'f':
	{
		lleft = true;
		break;
	}
	case 'r':
	{
		lup = true;
		break;
	}
	case 'y':
	{
		ldown = true;
		break;
	}
	case 'i':
	{
		bforward = true;
		break;
	}
	case 'k':
	{
		bbackward = true;
		break;
	}
	case 'l':
	{
		bright = true;
		break;
	}
	case 'j':
	{
		bleft = true;
		break;
	}
	case 'u':
	{
		bup = true;
		break;
	}
	case 'o':
	{
		bdown = true;
		break;
	}
	case '7':
	{
		bx = true;
		break;
	}
	case '8':
	{
		by = true;
		break;
	}
	case '9':
	{
		bz = true;
		break;
	}
	case '4':
	{
		brx = true;
		break;
	}
	case '5':
	{
		bry = true;
		break;
	}
	case '6':
	{
		brz = true;
		break;
	}

	//special function key
	case 'z'://move light source to front of camera
	{
		light_pos[0] = eyex + cos(eyet * M_PI / 180) * cos(eyep * M_PI / 180);
		light_pos[1] = eyey + sin(eyet * M_PI / 180);
		light_pos[2] = eyez - cos(eyet * M_PI / 180) * sin(eyep * M_PI / 180);
		break;
	}
	case 'x'://move ball to front of camera
	{
		ball_pos[0] = eyex + cos(eyet * M_PI / 180) * cos(eyep * M_PI / 180) * 3;
		ball_pos[1] = eyey + sin(eyet * M_PI / 180) * 5;
		ball_pos[2] = eyez - cos(eyet * M_PI / 180) * sin(eyep * M_PI / 180) * 3;
		break;
	}
	case 'c'://reset all pose
	{
		light_pos[0] = 1.1;
		light_pos[1] = 1.0;
		light_pos[2] = 1.3;
		ball_pos[0] = 0;
		ball_pos[1] = 0;
		ball_pos[2] = 0;
		ball_rot[0] = 0;
		ball_rot[1] = 0;
		ball_rot[2] = 0;
		eyex = 0.0;
		eyey = 0.0;
		eyez = 5.6;
		eyet = 0;
		eyep = 90;
		break;
	}
	default:
	{
		break;
	}
	}
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.001f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

void motion(int x, int y)
{
	if (mleft)
	{
		eyep -= (x - mousex) * 0.1;
		eyet -= (y - mousey) * 0.12;
		if (eyet > 89.9)
			eyet = 89.9;
		else if (eyet < -89.9)
			eyet = -89.9;
		if (eyep > 360)
			eyep -= 360;
		else if (eyep < 0)
			eyep += 360;
	}
	mousex = x;
	mousey = y;
}

void mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN && !mright && !mmiddle)
		{
			mleft = true;
			mousex = x;
			mousey = y;
		}
		else
			mleft = false;
	}
	else if (button == GLUT_RIGHT_BUTTON)
	{
		if (state == GLUT_DOWN && !mleft && !mmiddle)
		{
			mright = true;
			mousex = x;
			mousey = y;
		}
		else
			mright = false;
	}
	else if (button == GLUT_MIDDLE_BUTTON)
	{
		if (state == GLUT_DOWN && !mleft && !mright)
		{
			mmiddle = true;
			mousex = x;
			mousey = y;
		}
		else
			mmiddle = false;
	}
}

void camera_light_ball_move()
{
	GLfloat dx = 0, dy = 0, dz = 0;
	if (left || right || forward || backward || up || down)
	{
		if (left)
			dx = -speed;
		else if (right)
			dx = speed;
		if (forward)
			dy = speed;
		else if (backward)
			dy = -speed;
		eyex += dy * cos(eyet * M_PI / 180) * cos(eyep * M_PI / 180) + dx * sin(eyep * M_PI / 180);
		eyey += dy * sin(eyet * M_PI / 180);
		eyez += dy * (-cos(eyet * M_PI / 180) * sin(eyep * M_PI / 180)) + dx * cos(eyep * M_PI / 180);
		if (up)
			eyey += speed;
		else if (down)
			eyey -= speed;
	}
	if (lleft || lright || lforward || lbackward || lup || ldown)
	{
		dx = 0;
		dy = 0;
		if (lleft)
			dx = -speed;
		else if (lright)
			dx = speed;
		if (lforward)
			dy = speed;
		else if (lbackward)
			dy = -speed;
		light_pos[0] += dy * cos(eyet * M_PI / 180) * cos(eyep * M_PI / 180) + dx * sin(eyep * M_PI / 180);
		light_pos[1] += dy * sin(eyet * M_PI / 180);
		light_pos[2] += dy * (-cos(eyet * M_PI / 180) * sin(eyep * M_PI / 180)) + dx * cos(eyep * M_PI / 180);
		if (lup)
			light_pos[1] += speed;
		else if (ldown)
			light_pos[1] -= speed;
	}
	if (bleft || bright || bforward || bbackward || bup || bdown)
	{
		dx = 0;
		dy = 0;
		if (bleft)
			dx = -speed;
		else if (bright)
			dx = speed;
		if (bforward)
			dy = speed;
		else if (bbackward)
			dy = -speed;
		ball_pos[0] += dy * cos(eyet * M_PI / 180) * cos(eyep * M_PI / 180) + dx * sin(eyep * M_PI / 180);
		ball_pos[1] += dy * sin(eyet * M_PI / 180);
		ball_pos[2] += dy * (-cos(eyet * M_PI / 180) * sin(eyep * M_PI / 180)) + dx * cos(eyep * M_PI / 180);
		if (bup)
			ball_pos[1] += speed;
		else if (bdown)
			ball_pos[1] -= speed;
	}
	if (bx || by || bz || brx || bry || brz)
	{
		dx = 0;
		dy = 0;
		dz = 0;
		if (bx)
			dx = -rotation_speed;
		else if (brx)
			dx = rotation_speed;
		if (by)
			dy = rotation_speed;
		else if (bry)
			dy = -rotation_speed;
		if (bz)
			dz = rotation_speed;
		else if (brz)
			dz = -rotation_speed;
		ball_rot[0] += dx;
		ball_rot[1] += dy;
		ball_rot[2] += dz;
	}
}

void draw_light_bulb()
{
	GLUquadric* quad;
	quad = gluNewQuadric();
	glPushMatrix();
	glColor3f(0.4, 0.5, 0);
	glTranslatef(light_pos[0], light_pos[1], light_pos[2]);
	gluSphere(quad, light_rad, 40, 20);
	glPopMatrix();
}

void keyboardup(unsigned char key, int x, int y)
{
	switch (key) {
	case 'd':
	{
		right = false;
		break;
	}
	case 'a':
	{
		left = false;
		break;
	}
	case 'w':
	{
		forward = false;
		break;
	}
	case 's':
	{
		backward = false;
		break;
	}
	case 'q':
	{
		up = false;
		break;
	}
	case 'e':
	{
		down = false;
		break;
	}
	case 't':
	{
		lforward = false;
		break;
	}
	case 'g':
	{
		lbackward = false;
		break;
	}
	case 'h':
	{
		lright = false;
		break;
	}
	case 'f':
	{
		lleft = false;
		break;
	}
	case 'r':
	{
		lup = false;
		break;
	}
	case 'y':
	{
		ldown = false;
		break;
	}
	case 'i':
	{
		bforward = false;
		break;
	}
	case 'k':
	{
		bbackward = false;
		break;
	}
	case 'l':
	{
		bright = false;
		break;
	}
	case 'j':
	{
		bleft = false;
		break;
	}
	case 'u':
	{
		bup = false;
		break;
	}
	case 'o':
	{
		bdown = false;
		break;
	}
	case '7':
	{
		bx = false;
		break;
	}
	case '8':
	{
		by = false;
		break;
	}
	case '9':
	{
		bz = false;
		break;
	}
	case '4':
	{
		brx = false;
		break;
	}
	case '5':
	{
		bry = false;
		break;
	}
	case '6':
	{
		brz = false;
		break;
	}

	default:
	{
		break;
	}
	}
}

void idle(void)
{
	glutPostRedisplay();
}

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}