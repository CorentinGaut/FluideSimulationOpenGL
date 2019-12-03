#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <string>

#define TINYPLY_IMPLEMENTATION

#include <tinyply.h>
#include "stl.h"
#include "texture.h"
#include "OBJLoader.h"

float distancez = 10.0f;
int width, height;


static void error_callback(int /*error*/, const char* description)
{
	std::cerr << "Error: " << description << std::endl;
}

static void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

}

glm::vec4 resetPos(glm::vec4 pos) {
	//cout << "jfgj" << endl;
	if (pos.x > 8.f) {
		pos = { -8.f,pos.y,pos.z, 1.f };
	}
	if (pos.y >= 8.f) {
		//std::cout << "yo" << std::endl;
		pos = { pos.x,-8.f,pos.z, 1.f };
	}
	if (pos.z > 8.f) {
		pos = { pos.x,pos.y,-8.0f, 1.f };
	}

	return pos;
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	std::cout << yoffset << std::endl;
	if (yoffset < 0) {
		distancez += 0.1f;
	}
	if (yoffset > 0) {
		distancez -= 0.1f;
	}
	
}

/* PARTICULES */
struct Particule {
	glm::vec4 position;
	glm::vec4 color;
	glm::vec4 speed;
};

struct Vertex {
	glm::vec3 position;
	glm::vec2 uv;
};

std::vector<Particule> MakeParticules(const int n)
{
	std::default_random_engine generator;
	std::uniform_real_distribution<float> distribution01(0.f, 1.f);
	std::uniform_real_distribution<float> distribution03(0.0f, 0.05f);
	std::uniform_real_distribution<float> distribution02(0.1f, 1.5f);
	std::uniform_real_distribution<float> distributionWorld(-5.f, 5.f);
	std::uniform_real_distribution<float> distributionForce(5.0f, 100.0f);

	std::vector<Particule> p;
	p.reserve(n);

	for(int i = 0; i < n; i++)
	{
		p.push_back(Particule{
				{
				distributionWorld(generator),//pos
				distributionWorld(generator),
				distributionWorld(generator), distributionForce(generator)},
				{
				distribution01(generator),//couleur
				distribution01(generator),
				distribution01(generator), 1.f},
				{
				distribution03(generator),//speed
				distribution03(generator),
				distribution03(generator), 1.f}
				});
	}

	return p;
}

GLuint MakeShader(GLuint t, std::string path)
{
	std::cout << path << std::endl;
	std::ifstream file(path.c_str(), std::ios::in);
	std::ostringstream contents;
	contents << file.rdbuf();
	file.close();

	const auto content = contents.str();
	std::cout << content << std::endl;

	const auto s = glCreateShader(t);

	GLint sizes[] = {(GLint) content.size()};
	const auto data = content.data();

	glShaderSource(s, 1, &data, sizes);
	glCompileShader(s);

	GLint success;
	glGetShaderiv(s, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		GLchar infoLog[512];
		GLsizei l;
		glGetShaderInfoLog(s, 512, &l, infoLog);

		std::cout << infoLog << std::endl;
	}

	return s;
}

GLuint AttachAndLink(std::vector<GLuint> shaders)
{
	const auto prg = glCreateProgram();
	for(const auto s : shaders)
	{
		glAttachShader(prg, s);
	}

	glLinkProgram(prg);

	GLint success;
	glGetProgramiv(prg, GL_LINK_STATUS, &success);
	if(!success)
	{
		GLchar infoLog[512];
		GLsizei l;
		glGetProgramInfoLog(prg, 512, &l, infoLog);

		std::cout << infoLog << std::endl;
	}

	return prg;
}

void APIENTRY opengl_error_callback(GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar *message,
		const void *userParam)
{
	std::cout << message << std::endl;
}

int main(void)
{
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}


	// NOTE: OpenGL error checks have been omitted for brevity
	//apelle de touche 
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	//glfwSwapInterval(1);

	if(!gladLoadGL()) {
		std::cerr << "Something went wrong!" << std::endl;
		exit(-1);
	}

	// Callbacks
	glDebugMessageCallback(opengl_error_callback, nullptr);

	// Shader
	const auto vertex = MakeShader(GL_VERTEX_SHADER, "shader.vert");
	const auto fragment = MakeShader(GL_FRAGMENT_SHADER, "shader.frag");
	const auto comput = MakeShader(GL_COMPUTE_SHADER, "shader.comput");

	const auto program = AttachAndLink({vertex, fragment});

	const auto program_comput = AttachAndLink({ comput });

	glUseProgram(program);

	//Particules ------------------------------
	const size_t nParticules = 1000000;
	auto Particules = MakeParticules(nParticules);

	//Load l'image---------------------
	Image texture = LoadImage("wood.bmp");

	//load obj--------------------------
	std::vector<glm::vec3> verticesOBJ;
	std::vector<glm::vec2> uvsOBJ;
	std::vector<glm::vec3> normalsOBJ;
	const char* filename = "cube.obj";
	loadOBJ(filename, verticesOBJ, uvsOBJ, normalsOBJ);

	std::vector<Vertex> cubeVert;

	for (unsigned int i = 0; i < verticesOBJ.size(); i++)
	{
		cubeVert.push_back({
			verticesOBJ[i],
			uvsOBJ[i]
			});
	}
	
	// Buffers
	GLuint vbo, vao;
	glGenBuffers(1, &vbo);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, nParticules * sizeof(Particule), Particules.data(), GL_STATIC_DRAW);

	//position
	const auto index = glGetAttribLocation(program, "position");
	glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), nullptr);
	glEnableVertexAttribArray(index);

	//couleur
	const auto indexColor = glGetAttribLocation(program, "color");
	glVertexAttribPointer(indexColor, 3, GL_FLOAT, GL_FALSE, sizeof(Particule), (void*)(sizeof(glm::vec3)));
	glEnableVertexAttribArray(indexColor);

	//normal
	/*const auto indexNormal = glGetAttribLocation(program, "normal");

	glVertexAttribPointer(indexNormal, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) * 2, (void*)(sizeof(glm::vec3) + sizeof(glm::vec2)));
	glEnableVertexAttribArray(indexNormal);*/

	//----------------------------------------Texture de BOIS----------------------------------------
	/*GLuint textureId;
	glCreateTextures(GL_TEXTURE_2D, 1, &textureId);
	glTextureStorage2D(textureId, 1, GL_RGB8, texture.width, texture.height);
	glTextureSubImage2D(textureId,0,0,0,texture.width,texture.height,GL_RGB,GL_UNSIGNED_BYTE,texture.data.data());
	const auto indextexture = glGetAttribLocation(program, "uv_in");
	glVertexAttribPointer(indextexture, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3)));
	glEnableVertexAttribArray(indextexture);
	int uniformTexture = glGetUniformLocation(program, "Texture");
	glProgramUniform1i(program, uniformTexture, 0);

	//----------------------------------------texture frame buffer color--------------------------------------
	GLuint textureColorFrameBuffer;
	glCreateTextures(GL_TEXTURE_2D, 1, &textureColorFrameBuffer);
	glTextureStorage2D(textureColorFrameBuffer, 1, GL_RGB8, 500, 500);

	//----------------------------------------texture frame buffer Depth----------------------------------------
	GLuint textureDepthFrameBuffer;
	glCreateTextures(GL_TEXTURE_2D, 1, &textureDepthFrameBuffer);
	glTextureStorage2D(textureDepthFrameBuffer, 1, GL_DEPTH_COMPONENT16, 500, 500);

	//FRAME BUFFER
	GLuint frameBufferId = 1;
	glCreateFramebuffers(1, &frameBufferId);
	glNamedFramebufferTexture(frameBufferId, GL_COLOR_ATTACHMENT0, textureColorFrameBuffer, 0);
	glNamedFramebufferTexture(frameBufferId, GL_DEPTH_ATTACHMENT, textureDepthFrameBuffer, 0);*/

	float scale = 1.0f;
	glm::mat4 scaleMat = glm::scale(glm::vec3(scale, scale, scale));

	float rotate = 0.0f;
	float angle = 90.0f;
	float rad = angle * 180.0 / 3.1415;

	glEnable(GL_DEPTH_TEST);

	glPointSize(2.0f);

	double time = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		glfwSetScrollCallback(window, scroll_callback);
		glfwGetFramebufferSize(window, &width, &height);

		auto timeFrame = glfwGetTime();
		auto dt = timeFrame - time;

		//vitesse des particlues sur le processeur ------------------------------
			/*for(Particule &var : Particules)
			{
				glm::vec4 acc = var.mass * 9.81f * glm::vec4{ 0.0f, 1.0f,0.0f,1.f };
				var.speed = acc * float(dt) * 0.5f;
				var.position += (var.speed) + glm::vec4{ var.vie, 1.f, 0.f, 1.f} * float(dt);

				var.position = resetPos(var.position);
			}
			time = timeFrame;
			std::cout << dt <<" "<< 1/dt << std::endl;
			glBufferSubData(GL_ARRAY_BUFFER,0, nParticules * sizeof(Particule), Particules.data());*/
		//vitesse sur le GPU ---------------------------------------------------
			//computpart
			glUseProgram(program_comput);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vbo);

			const auto indexDelatTime = glGetUniformLocation(program_comput, "deltaTime");
			glUniform1f(indexDelatTime, dt);
			//glEnableVertexAttribArray(index);

			glDispatchCompute(nParticules / 32, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			time = timeFrame;
			std::cout << dt << " " << 1 / dt << std::endl;
		glBindVertexArray(vao);
		glUseProgram(program);
		//CUBE
			//couleur
			glm::vec3 green = glm::vec3(0.f, 1.f, 1.f);
			int uniformCouleur = glGetUniformLocation(program, "color");
			glProgramUniform3f(program, uniformCouleur, green.x, green.y, green.z);
		
			//-----------------------------TRANSFORM-----------------------------
			angle = 0.0f;
			glm::mat4 rotateMat = glm::rotate(angle, glm::vec3(1.0f, 1.0f, 1.0f));
			glm::mat4 mat = scaleMat * rotateMat;
			int uniformTransform = glGetUniformLocation(program, "Mat");
			glProgramUniformMatrix4fv(program, uniformTransform, 1, GL_FALSE, &mat[0][0]);

			//-----------------------------PROJECTION-----------------------------
			glm::mat4 projection = glm::perspective(rad, float(width / height), 0.1f, 1000.0f);
			int uniformProjection = glGetUniformLocation(program, "projection");
			glProgramUniformMatrix4fv(program, uniformProjection, 1, GL_FALSE, &projection[0][0]);

			//-----------------------------VIEW-----------------------------
			glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, distancez), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			int uniformView = glGetUniformLocation(program, "view");
			glProgramUniformMatrix4fv(program, uniformView, 1, GL_FALSE, &view[0][0]);

/*
			//je dessine sur le frame buffer
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBufferId);
			glViewport(0, 0, 500, 500);
			glBindTextureUnit(0, textureId);
			glClearColor(0.6f, 0.6f, 0.6f, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLES, 0, verticesOBJ.size() * 3);

			//je dessine sur le frame buffer de la fenetre
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glViewport(0, 0, width, height);
			glBindTextureUnit(0, textureColorFrameBuffer);*/
			glViewport(0, 0, width, height);
			glClearColor(0.1f, 0.1f, 0.1f, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDrawArrays(GL_POINTS, 0, nParticules);
			
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


