#include <iostream>
#include <fstream>

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "model.h"
#include "mesh.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//Forward Declaration
void processInput(GLFWwindow* window);
int init(GLFWwindow*& window);
void createGeometry(GLuint& vao, GLuint &ebo, int& size, int& numIndices);
void createShaders();
void createProgram(GLuint& programID, const char* vertex, const char* fragment);
GLuint loadTexture(const char* path, int comp = 0);
void renderSkyBox();
void renderTerrain();
void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);

unsigned int GeneratePlane(const char* heightmap, unsigned char* &data, GLenum format, int comp, float hScale, float xzScale, unsigned int& indexCount, unsigned int& heightmapID);


//Window Callbacks
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

bool keys[1024];

//Utilities
void loadFile(const char* filename, char*& output);

//Shader Programs
GLuint simpleProgram, skyProgram, terrainProgram, modelProgram;

const int WIDTH = 1280, HEIGHT = 720;

//World Data
glm::vec3 lightDirection = glm::normalize(glm::vec3(-0.5f, -0.5f, -0.5f));
glm::vec3 cameraPosition = glm::vec3(100.0f, 125.5f, 100.0f);


GLuint boxVAO, boxEBO;
int boxSize, boxIndexCount;
glm::mat4 view, projection;

float lastX, lastY;
bool firstMouse = true;
float camYaw, camPitch;
glm::quat camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));

//Terrain Data
GLuint terrainVAO, terrainIndexCount, heightmapID, heightNormalID;
unsigned char* heightmapTexture;

GLuint dirt, sand, grass, snow, rock;

Model* backpack;
Model* farmhouse;
Model* palmtree;

int main() {

	//Init
	GLFWwindow* window;
	int result = init(window);
	if (result != 0)
	{
		return result;
	}

	stbi_set_flip_vertically_on_load(true);

	createShaders();
	createGeometry(boxVAO, boxEBO, boxSize, boxIndexCount);

	terrainVAO = GeneratePlane("textures/heightmap2.png", heightmapTexture, GL_RGBA, 4, 150, 5.0f, terrainIndexCount, heightmapID);
	heightNormalID = loadTexture("textures/heightnormal2.png");
	 
	GLuint boxTex = loadTexture("textures/container.png");
	GLuint boxNormal = loadTexture("textures/container_normalMap.png");

	dirt = loadTexture("textures/dirt.jpg");
	snow = loadTexture("textures/snow.jpg");
	sand = loadTexture("textures/sand.jpg");
	rock = loadTexture("textures/rock.jpg");
	grass = loadTexture("textures/grass.png", 4);

	backpack = new Model("models/backpack/backpack.obj");
	farmhouse = new Model("models/oldhouse/Farmhouse.obj");
	palmtree = new Model("models/palmtree/palmtree.obj");

	//Tell opengl to create viewport
	glViewport(0, 0, WIDTH, HEIGHT);

	//Matrices!
	view = glm::lookAt(cameraPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	projection = glm::perspective(glm::radians(45.0f), WIDTH / (float)HEIGHT, 0.1f, 5000.0f);

	//Rendering loop
	while (!glfwWindowShouldClose(window))
	{
		//Input
		processInput(window);

		//Rendering
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderSkyBox();
		renderTerrain();

		float t = glfwGetTime();

		renderModel(backpack, glm::vec3(1000,100,1000), glm::vec3(0,t,0), glm::vec3(100,100,100));
		renderModel(farmhouse, glm::vec3(500,1,200), glm::vec3(0,0,0), glm::vec3(2,2,2));
		renderModel(farmhouse, glm::vec3(400, 1, 200), glm::vec3(0, 80, 0), glm::vec3(2, 2, 2));

		renderModel(palmtree, glm::vec3(600, 1, 1300), glm::vec3(0, 0, 0), glm::vec3(2, 2, 2));
		renderModel(palmtree, glm::vec3(550, 1, 1300), glm::vec3(40, 0, 0), glm::vec3(2, 2, 2));
		renderModel(palmtree, glm::vec3(505, 1, 1300), glm::vec3(0, 0, 0), glm::vec3(2, 2, 2));

		//Swap & Poll	
		glfwSwapBuffers(window);
		glfwPollEvents();

		//Rotates cube
		//world = glm::rotate(world, (float)glfwGetTime()/1000, glm::vec3(01.0f, 1.0f, 1.0f));
	}

	glfwTerminate();
	return 0;
}

void renderSkyBox()
{
	//OpenGL Setup
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH);

	glUseProgram(skyProgram);
	
	glm::mat4 world = glm::mat4(1.0f);
	world = glm::translate(world, cameraPosition);
	world = glm::scale(world, glm::vec3(100, 100, 100));

	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(skyProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3fv(glGetUniformLocation(skyProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(skyProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	//Rendering
	glBindVertexArray(boxVAO);
	glDrawElements(GL_TRIANGLES, boxIndexCount, GL_UNSIGNED_INT, 0);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH);

}

void renderTerrain()
{
	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(terrainProgram);

	glm::mat4 world = glm::mat4(1.0f);

	glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(terrainProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//float t = glfwGetTime();
	//lightDirection = glm::normalize(glm::vec3(glm::sin(t), -0.5f, glm::cos(t)));
	glUniform3fv(glGetUniformLocation(terrainProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(terrainProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmapID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, heightNormalID);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, dirt);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, sand);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, rock);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, grass);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, snow);

	//Rendering
	glBindVertexArray(terrainVAO);
	glDrawElements(GL_TRIANGLES, terrainIndexCount, GL_UNSIGNED_INT, 0);

}

unsigned int GeneratePlane(const char* heightmap, unsigned char* &data, GLenum format, int comp, float hScale, float xzScale, unsigned int& indexCount, unsigned int& heightmapID) {
	int width, height, channels;
	data = nullptr;
	if (heightmap != nullptr) {
		data = stbi_load(heightmap, &width, &height, &channels, comp);
		if (data) {
			glGenTextures(1, &heightmapID);
			glBindTexture(GL_TEXTURE_2D, heightmapID);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	int stride = 8;
	float* vertices = new float[(width * height) * stride];
	unsigned int* indices = new unsigned int[(width - 1) * (height - 1) * 6];

	int index = 0;
	for (int i = 0; i < (width * height); i++) {
		// TODO: calculate x/z values
		int x = i % width;
		int z = i / width;

		float texHeight = (float)data[i * comp];

		// TODO: set position
		vertices[index++] = x * xzScale;
		vertices[index++] = (texHeight / 255.0f) * hScale;
		vertices[index++] = z * xzScale;

		// TODO: set normal
		vertices[index++] = 0;
		vertices[index++] = 1;
		vertices[index++] = 0;

		// TODO: set uv
		vertices[index++] = x / (float)width;
		vertices[index++] = z / (float)height;
	}

	// OPTIONAL TODO: Calculate normal
	// TODO: Set normal

	index = 0;
	for (int i = 0; i < (width - 1) * (height - 1); i++) {
		// TODO: calculate x/z values
		int x = i % (width - 1);
		int z = i / (width - 1);
	
		int vertex = z * width + x;

		indices[index++] = vertex;
		indices[index++] = vertex + width;
		indices[index++] = vertex + width + 1;

		indices[index++] = vertex;
		indices[index++] = vertex + width + 1;
		indices[index++] = vertex + 1;
	}

	unsigned int vertSize = (width * height) * stride * sizeof(float);
	indexCount = ((width - 1) * (height - 1) * 6);

	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	// vertex information!
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, 0);
	glEnableVertexAttribArray(0);
	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);
	// uv
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * stride, (void*)(sizeof(float) * 6));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	delete[] vertices;
	delete[] indices;

	//stbi_image_free(data);

	return VAO;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	bool camChanged = false;
	if (keys[GLFW_KEY_W])
	{
		cameraPosition += camQuat * glm::vec3(0, 0, 1);
		camChanged = true;
	}
	if (keys[GLFW_KEY_A])
	{
		cameraPosition += camQuat * glm::vec3(1, 0, 0);
		camChanged = true;
	}
	if (keys[GLFW_KEY_S])
	{
		cameraPosition += camQuat * glm::vec3(0, 0, -1);
		camChanged = true;
	}
	if (keys[GLFW_KEY_D])
	{
		cameraPosition += camQuat * glm::vec3(-1, 0, 0);
		camChanged = true;
	}

	if (camChanged)
	{
		glm::vec3 camForward = camQuat * glm::vec3(0, 0, 1);
		glm::vec3 camUp = camQuat * glm::vec3(0, 1, 0);
		view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);
	}
}

int init(GLFWwindow*& window)
{
	//GLFW Init
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


	//Create window & make active context	
	window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL_2233", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	//Register Callbacks
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);


	//Load GLAD 
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) 
{
	float x = (float)xpos;
	float y = (float)ypos;

	if (firstMouse)
	{
		lastX = x;
		lastY = y;
		firstMouse = false;
	}

	float dx = x - lastX;
	float dy = y - lastY;
	lastX = x;
	lastY = y;

	camYaw -= dx;
	camPitch = glm::clamp(camPitch + dy, -90.0f, 90.0f);
	if (camYaw > 180)
	{
		camYaw -= 360.0f;
	}
	if (camYaw < -180)
	{
		camYaw += 360.0f;
	}

	camQuat = glm::quat(glm::vec3(glm::radians(camPitch), glm::radians(camYaw), 0));
	 
	glm::vec3 camForward = camQuat * glm::vec3(0, 0, 1);
	glm::vec3 camUp = camQuat * glm::vec3(0, 1, 0);
	view = glm::lookAt(cameraPosition, cameraPosition + camForward, camUp);

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		//store key is pressed
		keys[key] = true;
	}
	else if (action == GLFW_RELEASE)
	{
		//store key is not pressed
		keys[key] = false;
	}
}

void createGeometry(GLuint& vao, GLuint &ebo, int& size, int &numIndices)
{
	/* Makes Square or Triangle based on vertices & indices with color generated in the shader
	//position				//color
	float vertices[] = {
	-0.5f, -0.5f, 0.5f,		1.0f, 0.0f, 0.0f, 1.0f,
	 0.5f, -0.5f, 0.5f,		0.0f, 1.0f, 0.0f, 1.0f,
	-0.5f,  0.5f, 0.5f,		0.0f, 0.0f, 1.0f, 1.0f,
	 0.5f,  0.5f, 0.5f,		1.0f, 1.0f, 1.0f, 1.0f
	};

	int indices[] = {
		0,2,1,
		1,3,2
	};
	*/

	// need 24 vertices for normal/uv-mapped Cube
	float vertices[] = {
		// positions            //colors            // tex coords   // normals          //tangents      //bitangents
		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, -1.f, 0.f,     -1.f, 0.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 0.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,

		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,      -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,      -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,

		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,      0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,
		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,      0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,

		-0.5f, 0.5f, -.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, 0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   1.f, 1.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 0.f, 1.f,     1.f, 0.f, 0.f,  0.f, -1.f, 0.f,

		-0.5f, -0.5f, 0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,
		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   1.f, 1.f,       -1.f, 0.f, 0.f,     0.f, 1.f, 0.f,  0.f, 0.f, 1.f,

		-0.5f, -0.5f, -.5f,     1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,
		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   1.f, 0.f,       0.f, 0.f, -1.f,     1.f, 0.f, 0.f,  0.f, 1.f, 0.f,

		0.5f, -0.5f, -0.5f,     1.0f, 1.0f, 1.0f,   0.f, 1.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, -0.5f, 0.5f,      1.0f, 1.0f, 1.0f,   0.f, 0.f,       1.f, 0.f, 0.f,     0.f, -1.f, 0.f,  0.f, 0.f, 1.f,

		0.5f, 0.5f, -0.5f,      1.0f, 1.0f, 1.0f,   0.f, 1.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f,
		0.5f, 0.5f, 0.5f,       1.0f, 1.0f, 1.0f,   0.f, 0.f,       0.f, 1.f, 0.f,     1.f, 0.f, 0.f,  0.f, 0.f, 1.f
	};

	unsigned int indices[] = {  // note that we start from 0!
		// DOWN
		0, 1, 2,   // first triangle
		0, 2, 3,    // second triangle
		// BACK
		14, 6, 7,   // first triangle
		14, 7, 15,    // second triangle
		// RIGHT
		20, 4, 5,   // first triangle
		20, 5, 21,    // second triangle
		// LEFT
		16, 8, 9,   // first triangle
		16, 9, 17,    // second triangle
		// FRONT
		18, 10, 11,   // first triangle
		18, 11, 19,    // second triangle
		// UP
		22, 12, 13,   // first triangle
		22, 13, 23,    // second triangle
	};

	//Stride :: Vertices in Amount of columns 
	int stride = (3 + 3 + 2 + 3 + 3 + 3) * sizeof(float);

	size = sizeof(vertices);
	numIndices = sizeof(indices) / sizeof(int);	
	
	//Creates VertexArrayObject index
	glGenVertexArrays(1, &vao);
	//Bind it to create its configuration
	glBindVertexArray(vao);

	//VertexBufferObject, bind it and assign vertices to it
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//set layout of vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 *  sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_TRUE, stride, (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_TRUE, stride, (void*)(11 * sizeof(float)));
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(5, 3, GL_FLOAT, GL_TRUE, stride, (void*)(14 * sizeof(float)));
	glEnableVertexAttribArray(5);
}

void createShaders()
{
	createProgram(simpleProgram, "shaders/simpleVertex.shader", "shaders/simpleFragment.shader");
	
	//Set texture channels
	glUseProgram(simpleProgram);
	glUniform1i(glGetUniformLocation(simpleProgram, "mainTex"), 0);
	glUniform1i(glGetUniformLocation(simpleProgram, "normalTex"), 1);
	
	createProgram(skyProgram, "shaders/skyVertex.shader", "shaders/skyFragment.shader");
	createProgram(terrainProgram, "shaders/terrainVertex.shader", "shaders/terrainFragment.shader");

	glUseProgram(terrainProgram);
	glUniform1i(glGetUniformLocation(terrainProgram, "mainTex"), 0);
	glUniform1i(glGetUniformLocation(terrainProgram, "normalTex"), 1);

	glUniform1i(glGetUniformLocation(terrainProgram, "dirt"), 2);
	glUniform1i(glGetUniformLocation(terrainProgram, "sand"), 3);
	glUniform1i(glGetUniformLocation(terrainProgram, "rock"), 4);
	glUniform1i(glGetUniformLocation(terrainProgram, "grass"), 5);
	glUniform1i(glGetUniformLocation(terrainProgram, "snow"), 6);

	createProgram(modelProgram, "shaders/model.vs", "shaders/model.fs");
	
	glUseProgram(modelProgram);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_diffuse1"), 0);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_specular1"), 1);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_normal1"), 2);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_roughness1"), 3);
	glUniform1i(glGetUniformLocation(modelProgram, "texture_ao1"), 4);
}

void createProgram(GLuint& programID, const char* vertex, const char* fragment)
{
	//Create a GL Program with a vertex & fragment shader
	char* vertexSrc;
	char* fragmentSrc;
	loadFile(vertex, vertexSrc);
	loadFile(fragment, fragmentSrc);

	GLuint vertexShaderID;
	GLuint fragmentShaderID;

	vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, &vertexSrc, nullptr);
	glCompileShader(vertexShaderID);

	int success;
	char infoLog[512];
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderID, 512, nullptr, infoLog);
		std::cout << "ERROR COMPILING VERTEX SHADER\n" << infoLog << std::endl;
	}

	fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, &fragmentSrc, nullptr);
	glCompileShader(fragmentShaderID);

	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderID, 512, nullptr, infoLog);
		std::cout << "ERROR COMPILING FRAGMENT SHADER\n" << infoLog << std::endl;
	}

	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programID, 512, nullptr, infoLog);
		std::cout << "ERROR LINKING PROGRAM SHADER\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	delete vertexSrc;
	delete fragmentSrc;
}

void loadFile(const char* filename, char*& output)
{
	//open the file
	std::fstream file(filename, std::ios::in | std::ios::binary); //std::ios::in needed to open the file

	//if the file was succesfully opened
	if (file.is_open())
	{
		//get length of file
		file.seekg(0, file.end);
		int length = file.tellg();
		file.seekg(0, file.beg);

		//allocate memory for the char pointer
		output = new char[length + 1];

		//read data as a block
		file.read(output, length);

		//add null terminator to the end of the char pointer
		output[length] = '\0';

		//close the file
		file.close();
	}
	else
	{
		//if the file failed to open, set the char pointer to NULL
		output = NULL;
	}
}

GLuint loadTexture(const char* path, int comp)
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, numChannels;
	unsigned char* data = stbi_load(path, &width, &height, &numChannels, comp);
	if (data)
	{
		if (comp != 0)
		{
			numChannels = comp;
		}

		if (numChannels == 3)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else if (numChannels == 4)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "ERROR LOADING TEXTURE" << path << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return textureID;
}

void renderModel(Model* model, glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
{
	//glEnable(GL_BLEND);
	//Alpha blend
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//additive blend
	//glBlendFunc(GL_ONE, GL_ONE);
	//soft additive blend
	//glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE);

	//multiply blend
	//glBlendFunc(GL_DST_COLOR, GL_ZERO);
	//double multiply blend
	//glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);

	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glUseProgram(modelProgram);

	glm::mat4 world = glm::mat4(1.0f);
	world = glm::translate(world, pos);
	world = world * glm::toMat4(glm::quat(rot));
	world = glm::scale(world, scale);

	glUniformMatrix4fv(glGetUniformLocation(modelProgram, "world"), 1, GL_FALSE, glm::value_ptr(world));
	glUniformMatrix4fv(glGetUniformLocation(modelProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(modelProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3fv(glGetUniformLocation(modelProgram, "lightDirection"), 1, glm::value_ptr(lightDirection));
	glUniform3fv(glGetUniformLocation(modelProgram, "cameraPosition"), 1, glm::value_ptr(cameraPosition));

	model->Draw(modelProgram);

	glDisable(GL_BLEND);

}