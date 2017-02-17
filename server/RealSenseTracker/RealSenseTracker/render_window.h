#ifndef RENDER_WINDOW_H
#define RENDER_WINDOW_H

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace std;

GLuint vao;
GLuint shader_programme;

GLFWwindow* setup_window(){
	GLFWwindow* window;
	const GLubyte* renderer;
	const GLubyte* version;
	GLuint vs, fs;
	GLfloat points[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f
	};
	GLfloat texcoords[] = {
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f
	};

	const char* vertex_shader =
		"#version 410\n"
		"layout(location = 0) in vec3 vertex_position;\n"
		"layout(location = 1) in vec2 vt;\n"
		"out vec2 texture_coordinates;\n"
		"void main() {\n"
		"texture_coordinates = vt;\n"
		"gl_Position = vec4(vertex_position, 1.0);\n"
		"}\n";

	const char* fragment_shader =
		"#version 410\n"
		"in vec2 texture_coordinates;\n"
		"uniform sampler2D basic_texture;\n"
		"out vec4 frag_colour;\n"
		"void main() {\n"
		"vec4 texel = texture(basic_texture, texture_coordinates);\n"
		"frag_colour = texel;\n"
		"}\n";

	glfwInit();
	window = glfwCreateWindow(640, 480, "RealSense Image Viewer", NULL, NULL);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();
	renderer = glGetString(GL_RENDERER);
	version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), points, GL_STATIC_DRAW);
	GLuint texcoords_vbo;
	glGenBuffers(1, &texcoords_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), texcoords, GL_STATIC_DRAW);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	//Input uniforms positions
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	//Create Shader
	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);
	fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);
	shader_programme = glCreateProgram();
	glAttachShader(shader_programme, fs);
	glAttachShader(shader_programme, vs);
	glLinkProgram(shader_programme);
	return window;
}

void render_frame(const GLvoid * image, int width, int height, GLFWwindow* window)
{
	glfwMakeContextCurrent(window);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	if (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader_programme);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
}

void terminate_window(){
	glfwTerminate();
}

#endif