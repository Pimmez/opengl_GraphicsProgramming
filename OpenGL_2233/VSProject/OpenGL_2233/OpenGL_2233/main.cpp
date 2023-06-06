#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

//Forward Declaration
void processInput(GLFWwindow* window);

int main() {

	//GLFW Init
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	//Create window & make active context	
	GLFWwindow* window = glfwCreateWindow(1280, 720, "OpenGL_2233", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//Load GLAD 
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//Tell opengl to create viewport
	glViewport(0, 0, 1280, 720);
	
	//Rendering loop
	while (!glfwWindowShouldClose(window))
	{
		//Input
		processInput(window);

		//Rendering
		glClearColor(0.5f, 0.2f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Swap & Poll	
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}