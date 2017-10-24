#include "glToolkit.h"

#define GLFW_DLL
#include <GLFW/glfw3.h>

#include <math.h>

// Include GLM
#include <glm/glm.hpp>
using namespace glm;

#include "shader.hpp"

#include <iostream>
using namespace std;

GLFWwindow* window = NULL;
unsigned MatrixLoc = 0;

unsigned texIDs[1000];

unsigned winWidth = 512;
unsigned winHeight = 512;

GLFWwindow* gltGetWindow() {
	return window;
}

// initializes openGL an' all
bool gltInit(unsigned windowWidth, unsigned windowHeight, const char windowTitle[])
{
	// initialize GLFW and set-up window an' all:
	if (!glfwInit()) {
		cout << "FAILED glfwInit" << endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_DEPTH_BITS, 24);
	//glfwWindowHint(GLFW_STENCIL_BITS, 8);

	winWidth = windowWidth;
	winHeight = windowHeight;
	
	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
	if (!window) {
		cout << "FAILED creating window" << endl;
		return false;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		cout << "FAILED glewInit" << endl;
		return false;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	/*GLfloat vertices[] = {
		// x, y, z, u, v
		-0.5f, +0.5f, 0.0f, 0.01f, 0.01f,
		+0.5f, +0.5f, 0.0f, 0.99f, 0.01f,
		-0.5f, -0.5f, 0.0f, 0.01f, 0.99f,
		+0.5f, -0.5f, 0.0f, 0.99f, 0.99f,
	};
	unsigned vbuf;
	glGenBuffers(1, &vbuf);
	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenTextures(sizeof(texIDs)/sizeof(unsigned), texIDs);
	glActiveTexture(GL_TEXTURE0);

	unsigned prog = Shaders::createProgram("data/shaders/posuv.vert", nullptr, "data/shaders/uvmap.frag");
	if (prog == 0) {
		cout << "Could not create shader program!" << endl;
		return false;
	}

	glUseProgram(prog);

	// Get a handle for our "myTextureSampler" uniform
	unsigned TextureLocation  = glGetUniformLocation(prog, "myTextureSampler");
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(TextureLocation, 0);

	// Get a handle for our "MVP" uniform
	MatrixLoc = glGetUniformLocation(prog, "MVP");
	int posLoc = glGetAttribLocation(prog, "vertexPosition_modelspace");
	int uvLoc = glGetAttribLocation(prog, "vertexUV");

	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glEnableVertexAttribArray(posLoc);
	glVertexAttribPointer(
		posLoc,             // attribute vertexPosition_modelspace
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		5*sizeof(float),	// stride
		(void*)0            // array buffer offset
		);
	glEnableVertexAttribArray(uvLoc);
	glVertexAttribPointer(
		uvLoc,              // attribute vertexUV.
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		5*sizeof(float),	// stride
		(void*)(3*sizeof(float))            // array buffer offset
		);
	*/

	return true;
}

// begins a frame
void gltBegin()
{
	glClearColor(0,0,0,0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);	// ================================
}

// finishes a frame and displays the result
void gltEnd()
{
	glfwSwapBuffers(window);
}

// draws an image into the active cell and advances 1 cell.
void gltDrawImg(int x, int y, unsigned width, unsigned height, GLenum format, GLenum type, const GLvoid * texData)
{
	// bind correct texture
	/*glBindTexture(GL_TEXTURE_2D, texIDs[gridCell]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, type, texData);

	int cell_x = gridCell % gridW;
	int cell_y = gridCell / gridH;
	float border_factor = 0.95f; // allow some border between images
	float scaleX = 2.0f / gridW; 
	float scaleY = 2.0f / gridH;
	float transX = -1 + 2.0f / gridW * (cell_x+0.5f);
	float transY = +1 - 2.0f / gridH * (cell_y+0.5f);

	// glm::mat4 MVP =  glm::translate(glm::scale(mat4(1.0f), glm::vec3(scaleX*border_factor, scaleY*border_factor, 1.0f)), glm::vec3(transX, transY, 0));
	glm::mat4 MVP =  glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(transX, transY, 0)), glm::vec3(scaleX*border_factor, scaleY*border_factor, 1.0f));

	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	glUniformMatrix4fv(MatrixLoc, 1, GL_FALSE, &MVP[0][0]);

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Starting from vertex 0; 3 vertices total -> 1 triangle

	gridCell++;
	*/
}

