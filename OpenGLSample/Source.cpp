/*
	Francis Naughton
	SNHU CS-330
	10/14/2022

	You can scroll up and down to increase and decrease the camera's speed.

	Light Object Controls
	Y- light up
	H- light down
	G- light neg on x axis
	J- light pos on x axis
	N- light neg on z axis
	M- light pos on z axis

	Camera Movement
	w forward
	s backward
	a left
	d right
	q up
	e down

	Modes
	//reset camera with 'F' before switching modes
	L- turn on orbit mode
	O- turn on ortho mode

	F- resets camera

*/


#include <glad/glad.h>
#include <GLFW/glfw3.h >
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Sphere.h"

#include "shader.h"
#include "camera.h"
#include <iostream>

#include "cylinder.h"

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);
static void resetCamera();
void TransformCamera(GLFWwindow* window);

float xlight = -1.2f, ylight = 1.0f, zlight = 2.0f;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
glm::vec3 orgin = glm::vec3(0.0f, 1.5f, 0.0f);
glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 4.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraDir = glm::normalize(cameraPos - orgin);
glm::vec3 cameraRight = glm::normalize(glm::cross(worldUp, cameraDir));
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDir, cameraRight));

glm::vec3 getOrgin();

const float MAX_CAMERA_SPEED = 25.0f;
const float MIN_CAMERA_SPEED = 1.0;

bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0;
float lastY = SCR_HEIGHT / 2.0;
float fov = 45.0f;
float xoffset, yoffset;

//obiting
float orbitRadius = 10.0f;
float camX = sin(glfwGetTime()) * orbitRadius;
float camZ = cos(glfwGetTime()) * orbitRadius;

//booleans for varius modes
bool ortho = false;
bool isOrbiting = false;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//camera speed
float cameraSpeed = 1.5;
float cameraOffset = cameraSpeed * deltaTime;

//lighting 
glm::vec3 lightPos(-1.2f, 2.0f, 2.0f);


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Francis Naughton Final Scene", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	Shader lightingShader("shaderfiles/5.4.light_casters.vs", "shaderfiles/5.4.light_casters.fs");
	Shader lightCubeShader("shaderfiles/5.4.light_cube.vs", "shaderfiles/5.4.light_cube.fs");
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float planeV[] = {

		//negative  z normals	      text cord
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,	//side
		 0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
		//positive z normals	text cord
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,	//side
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
		//negative x normals	text cord
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,	//side
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		//positive x normals	text cord
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		//Negative Y Normals	text cord
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,	//bottom
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		//Positive Y Normals	text cord
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,	//top
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	//pyramid vertices
	float pyramidV[] = {
		//verts					//normals			//texture
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,	 //bottom
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,	//back bottom left
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,	//front bottom left
		 0.0f,  1.5f,  0.0f,  0.0f,  0.0f,  1.0f,  0.5f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,	//front bottom left
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,	//front bottom right
		 0.0f,  1.5f,  0.0f,  0.0f,  0.0f,  1.0f,  0.5f, 1.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,	//front bottom right
		 0.5f,	0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,	//back bottom right
		 0.0f,  1.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.5f, 1.0f,
		 0.5f,	0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.0f,  1.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.5f, 1.0f
	};

	//milk carton vertices
	float milkV[] = {
		-0.5f,  -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,	 0.0f, 1.0f,
		 0.5f,  -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,     1.0f, 1.0f,
		 0.5f,  -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,	1.0f, 0.0f,	 //bottom
		 0.5f,  -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,	1.0f, 0.0f,
		-0.5f,  -0.5f,  0.5f,	 0.0f, -1.0f,  0.0f,	0.0f, 0.0f,
		-0.5f,  -0.5f, -0.5f,	 0.0f, -1.0f,  0.0f,	0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,	1.0f, 0.0f,	 //top
		 0.5f,  0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,	1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	 1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	 1.0f,  0.0f,  0.0f,	0.0f, 1.0f,

		-0.5f, -0.5f, -0.5f,	 0.0f, 0.0f, -1.0f,		0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,	 0.0f, 0.0f, -1.0f,		1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,	 0.0f, 0.0f, -1.0f,		1.0f, 1.0f,	//leftside
		 0.5f,  0.5f, -0.5f,	 0.0f, 0.0f, -1.0f,		1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	 0.0f, 0.0f, -1.0f,		0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	 0.0f, 0.0f, -1.0f,		0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,	0.0f,  0.0f,  1.0f,		0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,	0.0f,  0.0f,  1.0f,		1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,	0.0f,  0.0f,  1.0f,		1.0f, 1.0f,	//rightside
		 0.5f,  0.5f,  0.5f,	0.0f,  0.0f,  1.0f,		1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,	0.0f,  0.0f,  1.0f,		0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,	0.0f,  0.0f,  1.0f,		0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	0.0f, 0.0f,	//front side
		-0.5f, -0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	1.0f, 1.0f,

		 0.5f,  0.5f,  0.5f,	1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,	1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,	1.0f,  0.0f,  0.0f,		0.0f, 0.0f,	//back side
		 0.5f, -0.5f, -0.5f,	1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,	1.0f,  0.0f,  0.0f,		1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,	1.0f,  0.0f,  0.0f,		1.0f, 1.0f,

		 0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,	0.0f, 0.0f, -1.0f,		1.0f, 0.0f,	//left top triangle
		 0.0f,  1.0f, -0.5f,	0.0f, 0.0f, -1.0f,		0.5f, 1.0f,

		-0.5f,  0.5f, 0.5f,		0.0f,  0.0f,  1.0f,		0.0f, 0.0f,
		 0.5f,  0.5f, 0.5f,		0.0f,  1.0f,  0.5f,		1.0f, 0.0f,	//right top triangle
		 0.0f,  1.0f, 0.5f,		0.0f,  1.0f,  0.5f,		0.5f, 1.0f,

		-0.5f,  0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,	-1.0f,  0.0f,  0.0f,	1.0f, 0.0f,
		 0.0f,  1.0f,  0.5f,	-1.0f,  0.0f,  0.0f,	1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,	-1.0f,  0.0f,  0.0f,	0.0f, 0.0f,
		 0.0f,  1.0f, -0.5f,	-1.0f,  0.0f,  0.0f,	0.0f, 1.0f,
		 0.0f,  1.0f,  0.5f,	-1.0f,  0.0f,  0.0f,	1.0f, 1.0f,

		 0.5f,  0.5f,  0.5f,	1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,	1.0f,  0.0f,  0.0f,		1.0f, 0.0f,
		 0.0f,  1.0f, -0.5f,	1.0f,  0.0f,  0.0f,		1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,	1.0f,  0.0f,  0.0f,		0.0f, 0.0f,
		 0.0f,  1.0f,  0.5f,	1.0f,  0.0f,  0.0f,		0.0f, 1.0f,
		 0.0f,  1.0f, -0.5f,	1.0f,  0.0f,  0.0f,		1.0f, 1.0f
	};

	float lightCubeV[] = {
		//negative z normals	text cord
			-0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
			 0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f,  1.0f, 1.0f,	//side
			 0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f, 0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f, 0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
			//positive z normals	text cord
					-0.5f, -0.5f,  0.5f, 0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
					 0.5f, -0.5f,  0.5f, 0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
					 0.5f,  0.5f,  0.5f, 0.0f,  0.0f,  1.0f,  1.0f, 1.0f,	//side
					 0.5f,  0.5f,  0.5f, 0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
					-0.5f,  0.5f,  0.5f, 0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
					-0.5f, -0.5f,  0.5f, 0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
					//negative x normals	text cord
							-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
							-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
							-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,	//side
							-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
							-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
							-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
							//positive x normals	text cord
									0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
									0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
									0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
									0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
									0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
									0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
									//Negative Y Normals	text cord
											-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
											 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
											 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,	//bottom
											 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
											-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
											-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
											//Positive Y Normals	text cord
												-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
												 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
												 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
												 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,	//top
												-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
												-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};


	unsigned int planeVBO, planeVAO;
	unsigned int pyramidVAO, pyramidVBO;
	unsigned int milkVAO, milkVBO;
	unsigned int lightingVAO;
	unsigned int ballVAO, ballVBO;

	//plane
	//-----------------------------
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeV), planeV, GL_STATIC_DRAW);

	glBindVertexArray(planeVAO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normals attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture chord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//pyramid
	//-------------------------------
	glGenVertexArrays(1, &pyramidVAO);
	glGenBuffers(1, &pyramidVBO);
	glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidV), pyramidV, GL_STATIC_DRAW);

	glBindVertexArray(pyramidVAO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normals attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture chord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//milk carton
	//----------------------------
	glGenVertexArrays(1, &milkVAO);
	glGenBuffers(1, &milkVBO);
	glBindBuffer(GL_ARRAY_BUFFER, milkVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(milkV), milkV, GL_STATIC_DRAW);

	glBindVertexArray(milkVAO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normals attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture chord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//Sphere
	//----------
	glGenVertexArrays(1, &ballVAO);
	glBindVertexArray(ballVAO);
	glGenBuffers(1, &ballVBO);
	glBindBuffer(GL_ARRAY_BUFFER, ballVBO);


	//lighting cube VAOVBO
	glGenVertexArrays(1, &lightingVAO);
	glBindVertexArray(lightingVAO);

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(lightCubeV), lightCubeV, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//plane Textures
	unsigned int planeDiffuseMap = loadTexture("images/texWood.jpg");
	unsigned int planeSpecularMap = loadTexture("images/texWood_specular.jpg");

	//Pyramid Textures
	unsigned int pyramidDiffuseMap = loadTexture("images/texPyramid.jpg");
	unsigned int pyramidSpecularMap = loadTexture("images/texPyramid_specular.jpg");

	//Milk textures
	unsigned int milkDiffuseMap = loadTexture("images/texMilk.jpg");
	unsigned int milkSpecularMap = loadTexture("images/texMilk_specular.jpg");

	//CrystalBall Textures
	unsigned int ballDiffuseMap = loadTexture("images/texCrystal.jpg");
	unsigned int ballSpecularMap = loadTexture("images/texBall.jpg");



	//activate shader and set diffuse and specular maps
	lightingShader.use();
	lightingShader.setInt("material.diffuse", 0);
	lightingShader.setInt("material.specular", 1);

	//creates sphere object from Sphere.h 
	Sphere S(1, 60, 60);
	float angle = 0.0f;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		lightPos[0] = xlight;
		lightPos[1] = ylight;
		lightPos[2] = zlight;

		// render
		// ------
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// activate shader
		lightingShader.use();
		lightingShader.setVec3("light.position", lightPos);
		lightingShader.setVec3("light.direction", glm::vec3(0.5f, -1.0f, 0.5f));
		lightingShader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
		lightingShader.setVec3("viewPos", cameraPos);

		//light properties
		lightingShader.setVec3("light.ambient", 0.5f, 0.5f, 0.5f);
		lightingShader.setVec3("light.diffuse", 1.3f, 1.3f, 1.3f);
		lightingShader.setVec3("light.specular", 1.5f, 1.5f, 1.5f);
		//Light math set for a distance of 100
		lightingShader.setFloat("light.constant", 1.0f);
		lightingShader.setFloat("light.linear", 0.045f);
		lightingShader.setFloat("light.quadratic", 0.0075f);

		//material properties
		lightingShader.setFloat("material.shininess", 32.0f);
		int width, height;
		// pass projection matrix to shader (note that in this case it could change every frame)
		glfwGetFramebufferSize(window, &width, &height);
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, worldUp);
		glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		//change view and perspective depending on the selected mode
		if (ortho) {
			GLfloat oWidth = (GLfloat)width * 0.01f; // 10% the width
			GLfloat oHeight = (GLfloat)height * 0.01f; //10% the height
			view = glm::lookAt(cameraPos, getOrgin(), worldUp);
			projection = glm::ortho(-oWidth, oWidth, -oHeight, oHeight, 0.1f, 150.0f);
		}
		else if (isOrbiting) {

			camX = sinf(glfwGetTime()) * orbitRadius;
			camZ = cosf(glfwGetTime()) * orbitRadius;

			view = glm::lookAt(glm::vec3(camX, 2.5, camZ), glm::vec3(0.0, 0.0, 0.0), worldUp);
			projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		}
		else if (!ortho && !isOrbiting) {
			view = glm::lookAt(cameraPos, cameraPos + cameraFront, worldUp);
			projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		}

		// camera/view transformation
		lightingShader.setMat4("projection", projection);
		lightingShader.setMat4("view", view);
		glm::mat4 model = glm::mat4(1.0f);
		lightingShader.setMat4("model", model);


		//render PLANE
		// -------------------------
		//bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, planeDiffuseMap);
		//bind specular map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, planeSpecularMap);
		//set model to identity matrix
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.0f));
		model = glm::scale(model, glm::vec3(7.0f, 1.0f, 7.0f));
		//model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		lightingShader.setMat4("model", model);
		glBindVertexArray(planeVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		//render PYRAMID
		//---------------------
		//bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pyramidDiffuseMap);
		//bind specular map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pyramidSpecularMap);
		//set model to identity matrix
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-1.75f, -0.25f, -1.0f));
		model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
		angle = 45.00f;
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		lightingShader.setMat4("model", model);
		glBindVertexArray(pyramidVAO);
		glDrawArrays(GL_TRIANGLES, 0, 24);


		//render MILK CARTON
		//---------------------
		//bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, milkDiffuseMap);
		//bind specular map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, milkSpecularMap);
		//set model to identity matrix
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 1.75f, -1.5f));
		angle = 45.0f;
		model = glm::scale(model, glm::vec3(1.25f, 2.5f, 1.25f));
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
		lightingShader.setMat4("model", model);
		glBindVertexArray(milkVAO);
		glDrawArrays(GL_TRIANGLES, 0, 64);


		//render CRYSTAL BALL
		//--------------------
		//bind diffuse map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ballDiffuseMap);
		//bind specular map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, ballSpecularMap);
		glBindVertexArray(ballVAO);
		//set model to identity matrix
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(1.0f, 1.15f, 0.00f));
		model = glm::scale(model, glm::vec3(0.60f));
		lightingShader.setFloat("material.shininess", 128.0f);
		lightingShader.setMat4("model", model);
		glBindVertexArray(lightingVAO);
		S.Draw();


		//activate the cube light shader
		lightCubeShader.use();
		lightCubeShader.setMat4("projection", projection);
		lightCubeShader.setMat4("view", view);

		//render cube light
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.3f));
		lightCubeShader.setMat4("model", model);
		glBindVertexArray(lightingVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
		//checks for user mode switches
		TransformCamera(window);
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteVertexArrays(1, &pyramidVAO);
	glDeleteBuffers(1, &pyramidVBO);
	glDeleteVertexArrays(1, &milkVAO);
	glDeleteBuffers(1, &milkVBO);
	glDeleteVertexArrays(1, &lightingVAO);


	//delete textures
	glDeleteTextures(1, &planeDiffuseMap);
	glDeleteTextures(1, &planeSpecularMap);
	glDeleteTextures(1, &pyramidDiffuseMap);
	glDeleteTextures(1, &pyramidSpecularMap);
	glDeleteTextures(1, &milkDiffuseMap);
	glDeleteTextures(1, &milkSpecularMap);
	glDeleteTextures(1, &ballDiffuseMap);
	glDeleteTextures(1, &ballSpecularMap);
	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{

	float cameraOffset = cameraSpeed * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	//Light controls
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
		ylight += 0.01;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		ylight -= 0.01;
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		xlight -= 0.01;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		xlight += 0.01;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		zlight += 0.01;
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		zlight -= 0.01;




	//camera controls
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraOffset * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraOffset * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraOffset;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraOffset;


	//up and down movement
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		cameraPos += cameraOffset * cameraUp;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		cameraPos -= cameraOffset * cameraUp;

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void TransformCamera(GLFWwindow* window) {
	//ortho perspective controls
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		ortho = true;

	//orbiting controls
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		isOrbiting = true;

	//resets the camera and app booleans
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		resetCamera();
		ortho = false;
		isOrbiting = false;
	}

}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front = glm::vec3(1.0);
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	//restricts the camera speed between the min and max set global value
	if (cameraSpeed <= MIN_CAMERA_SPEED) {
		cameraSpeed = MIN_CAMERA_SPEED + 0.5f;
	}
	else if (cameraSpeed >= MAX_CAMERA_SPEED) {
		cameraSpeed = MAX_CAMERA_SPEED - 0.5f;
	}
	else if ((cameraSpeed <= MAX_CAMERA_SPEED) && (cameraSpeed >= MIN_CAMERA_SPEED)) {

		cameraSpeed += (float)yoffset;
		cameraSpeed += (float)xoffset;
	}

	cameraOffset = cameraSpeed * deltaTime;

	if (cameraSpeed > MAX_CAMERA_SPEED)
		cameraSpeed--;
	else if (cameraSpeed < MIN_CAMERA_SPEED)
		cameraSpeed++;

}

static void resetCamera() {
	//resets all basic lighting values
	orgin = glm::vec3(0.0f, 1.5f, 0.0f);
	cameraPos = glm::vec3(-5.5f, 4.0f, 5.0f);
	cameraFront = glm::normalize(glm::vec3(1.0f, -0.75f, -1.0f));
	worldUp = glm::vec3(0.0f, 3.0f, 0.0f);
	cameraDir = glm::normalize(cameraPos - orgin);
	cameraRight = glm::normalize(glm::cross(worldUp, cameraDir));
	cameraUp = glm::normalize(glm::cross(cameraDir, cameraRight));
	cameraSpeed = 1.5f;
	firstMouse = true;
	fov = 45.0f;
}
glm::vec3 getOrgin() {

	orgin = cameraPos + cameraFront;
	return orgin;
}


// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format{};
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;
		stbi_set_flip_vertically_on_load(true);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
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
