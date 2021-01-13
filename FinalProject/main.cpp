//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"





#include <iostream>

int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

//const unsigned int SHADOW_WIDTH = 2048;
//const unsigned int SHADOW_HEIGHT = 2048;

//const unsigned int SHADOW_WIDTH = 16384;
//const unsigned int SHADOW_HEIGHT = 16384;

const unsigned int SHADOW_WIDTH = 8192;
const unsigned int SHADOW_HEIGHT = 8192;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

//Point light
glm::vec3 pointLight;
GLuint pointLightPosition;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;


gps::Camera myCamera(glm::vec3(10.58f, 5.7f, 33.54f), glm::vec3(0.0f, 10.0f, 0.0f));
gps::Camera backupCamera(glm::vec3(10.58f, 5.7f, 33.54f), glm::vec3(0.0f, 10.0f, 0.0f));
gps::Camera restoreCamera(glm::vec3(10.58f, 5.7f, 33.54f), glm::vec3(0.0f, 10.0f, 0.0f));
float cameraSpeed = 0.1f;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D ground;
gps::Model3D screenQuad;
gps::Model3D tank;
gps::Model3D towers;
gps::Model3D crates;
gps::Model3D soldier;
gps::Model3D lamp;
gps::Model3D palm;
gps::Model3D waterDrop;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;


GLuint shadowMapFBO;
GLuint depthMapTexture;


bool showDepthMap;
//For soldier animation
bool animated = true;
//For camera animation
bool cameraAnimation = false;
//Original camera position
glm::vec3 originalCameraPosition = glm::vec3(15.77f, 7.02f, 24.67f);

float numberOfRaindDrops = 10000;
float rainDropCoord[10000][4];

//Boolean to enable or disable rain
bool rainEnabled = false;

void initializeRainCoord() {
	for (int i = 0; i < numberOfRaindDrops; i++) {
			float x = rand() % 60;
			float y = rand() % 60;
			float z = rand() % 30;
			float speed = ((float)rand() / (RAND_MAX));
			if (speed < 0.1) {
				speed = 0.1;
			}
			else if (speed > 0.7) {
				speed = 0.7;
			}
			float negative = rand() % 4;
			if (negative == 0) {
				x *= -1;
				y *= -1;
			}
			else if (negative == 1) {
				x *= -1;
			}
			else if (negative == 2) {
				y *= -1;
			}		
			rainDropCoord[i][0] = x;
			rainDropCoord[i][1] = z;
			rainDropCoord[i][2] = y;
			rainDropCoord[i][3] = speed;
	}
}

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);

}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}

	if (pressedKeys[GLFW_KEY_1] && action == GLFW_PRESS) {
		animated = not animated;
	}
	if (pressedKeys[GLFW_KEY_2] && action == GLFW_PRESS) {
		cameraAnimation = true;
		myCamera = backupCamera;
	}
	if (pressedKeys[GLFW_KEY_3] && action == GLFW_PRESS) {
		cameraAnimation = false;
		myCamera = restoreCamera;
	}
	if (pressedKeys[GLFW_KEY_4] && action == GLFW_PRESS) {
		std::cout << "Camera coord are: \n";
		std::cout << myCamera.getCameraPosition().x << " " << myCamera.getCameraPosition().y << " " << myCamera.getCameraPosition().z << "\n";
	}
	if (pressedKeys[GLFW_KEY_5] && action == GLFW_PRESS) {
		rainEnabled = not rainEnabled;
	}
}

/////////////camera///////////////
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400;
float lastY = 300;
/////////////camera///////////////

void mouseCallback(GLFWwindow* window, double xPosition, double yPosition) {
	/////////////camera///////////////
	if (firstMouse)
	{
		lastX = xPosition;
		lastY = yPosition;
		firstMouse = false;
	}

	float xoffset = xPosition - lastX;
	float yoffset = lastY - yPosition;
	lastX = xPosition;
	lastY = yPosition;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	myCamera.rotate(pitch, yaw);
	/////////////camera///////////////
}

/////////////camera///////////////
float zoom = 45.0f;
gps::MOVE_DIRECTION mouseScrollDirection;
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	zoom -= (float)yoffset; // amount scrolled vertically

	if (zoom < 1.0f)
		zoom = 1.0f;
	if (zoom > 45.0f)
		zoom = 45.0f;

	if (xoffset > yoffset)
		mouseScrollDirection = gps::MOVE_BACKWARD;
	else
		mouseScrollDirection = gps::MOVE_FORWARD;

	myCamera.move(mouseScrollDirection, 0.1f);
}
/////////////camera///////////////



void processMovement()
{
	if (pressedKeys[GLFW_KEY_Q]) {
		//Wireframe
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_E]) {
		//Solid
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_X]) {
		exit(0);
	}
	
	

}

bool initOpenGLWindow()
{

	

	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetScrollCallback(glWindow, scrollCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	

	

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}


void initObjects() {
	towers.LoadModel("D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/objects/tower/tower.obj");
	ground.LoadModel("D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/objects/ground/Ground.obj");
	screenQuad.LoadModel("D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/objects/quad/quad.obj");
	tank.LoadModel("D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/objects/tank/Tank.obj");
	crates.LoadModel("D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/objects/crates/ArmyCrates.obj");
	soldier.LoadModel("D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/objects/soldier/OneSoldier.obj");
	lamp.LoadModel("D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/objects/lamp/lamp.obj");
	palm.LoadModel("D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/objects/palm/palm.obj");
	waterDrop.LoadModel("D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/objects/waterDrop/waterDrop.obj");
}

void initShaders() {
	myCustomShader.loadShader("D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/shaders/shaderStart.vert", "D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	screenQuadShader.loadShader("D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/shaders/screenQuad.vert", "D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/shaders/depth.vert", "D:/Desktop/UTCN/An 3/Sem 1/Graphics Processing - GP/FinalProject/FinalProject/FinalProject/shaders/depth.frag");
	depthMapShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	//Point light
	pointLight = glm::vec3(-3.80f, 2.0f, 3.0f);
	pointLightPosition = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightPosition");
	glUniform3fv(pointLightPosition, 1, glm::value_ptr(pointLight));


	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 20.0f, 2.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	//bind nothing to attachment points
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	//unbind until ready to use
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}



glm::mat4 computeLightSpaceTrMatrix()
{
	const GLfloat near_plane = 0.1f, far_plane = 30.0f;
	glm::mat4 lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}


void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();

	model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	towers.Draw(shader);
	tank.Draw(shader);
	crates.Draw(shader);
	lamp.Draw(shader);
	palm.Draw(shader);
	ground.Draw(shader);

		

	model = glm::translate(model, glm::vec3(0.0f, 0.42966f, -5.61450f));
	model = glm::rotate(model, glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0f, -0.42966f, 5.61450f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	soldier.Draw(shader);
	if (!depthPass && rainEnabled) {
		
		for (int i = 0; i < numberOfRaindDrops; i++) {
			
		
			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(rainDropCoord[i][0], rainDropCoord[i][1], -rainDropCoord[i][2]));
			glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

			// do not send the normal matrix if we are rendering in the depth map

			/*normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
			glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));*/

			waterDrop.Draw(shader);

			rainDropCoord[i][1] -= rainDropCoord[i][3];
			if (rainDropCoord[i][1] < -1) {
				rainDropCoord[i][1] = 30;
			}
		}
	}


	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


	
	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	
}

//This method does the camera animation
void animateCamera() {


	for (int i = 0; i < 100; i++) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed / 20);
		if (i % 5 == 0) {
			yaw -= 0.03;
			myCamera.rotate(0.0, yaw);
		}
	}
	
}



void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map
	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	//render scene = draw objects
	drawObjects(depthMapShader, 1);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
	// render depth map on screen - toggled with the M key
	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

	

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		//For soldier animation
		if (animated) {
			angleY++;
		}

		//For camera animation
		if (cameraAnimation) {
			animateCamera();
		}
		

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		

		model = lightRotation;
		model = glm::translate(model, 1.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	


	}
}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}



int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initializeRainCoord();
	
	backupCamera = myCamera;
	restoreCamera = myCamera;

	glCheckError();
	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	
	

	return 0;
}
