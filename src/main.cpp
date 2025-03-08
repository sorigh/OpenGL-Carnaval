#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <windows.h>
#include <MMSystem.h>

#include <glm.hpp> 
#include <gtc/matrix_transform.hpp> 
#include <gtc/matrix_inverse.hpp> 
#include <gtc/type_ptr.hpp> 

#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

// constants
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 700;
const unsigned int SHADOW_WIDTH = 8048;
const unsigned int SHADOW_HEIGHT = 8048;
const float CAMERA_SENSITIVITY = 0.7f;
const float CAMERA_SPEED = 0.7f;


int retina_width, retina_height;
GLFWwindow* glWindow = NULL;


// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::mat4 lightRotation;
GLfloat angle;
glm::vec3 pointLightPositions[4] = {
    glm::vec3 (- 20.0f, 6.0f, 0.0f), // Point light 1 position
    glm::vec3(-20.0f, 5.0f, 0.0f), // Point light 2 position
    glm::vec3(3.0f, 6.0f, -1.0f), // Point light 3 position
    glm::vec3(-2.0f, 1.0f, 2.0f)  // Point light 4 position
};
int directionalLightEnabled = 1;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

// fog
int fog = 1;
GLfloat fogLoc, fogSkyBoxLoc;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 2.0f, -20.0f),
    glm::vec3(-25.0f, 0.0f, -25.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

// mouse and keyboard
float lastX, lastY;
float pitch, yaw;
bool firstMouse = true;
float sensitivity = 0.7f, cameraSpeed = 0.7f;
GLboolean pressedKeys[1024];

// models
gps::Model3D caravan;
gps::Model3D caravan2;
gps::Model3D merchant;
gps::Model3D quad;
gps::Model3D staticScene;
gps::Model3D rustyFerrisWheel;
gps::Model3D lantern;
gps::Model3D ghost;

// shaders
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;

// skybox
gps::SkyBox mySkyBoxDay;
gps::SkyBox mySkyBox;
gps::Shader skyBoxShader;


GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

// animation logic
bool present = false;
int value = 0;

float caravan_x = 0.0f;
float caravan_y = 0.0f;
bool caravan_inc = true;

float merchant_x = 0.0f;
float merchant_y = 0.0f;
bool merchant_inc = true;

bool night = 1;

struct Spotlight {
    glm::vec3 position;
    glm::vec3 direction;
    float cutOff;
    float outerCutOff;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant;
    float linear;
    float quadratic;
};





GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

// GLFW Callbacks
void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    // Wireframe
    if (pressedKeys[GLFW_KEY_Z]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    // Point
    if (pressedKeys[GLFW_KEY_X]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    // Normal
    if (pressedKeys[GLFW_KEY_C]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    // Depth Map
    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        showDepthMap = !showDepthMap;
    }
    // Present scene
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        present = !present;
    }
    // Night mode and Day mode
    if (key == GLFW_KEY_N && action == GLFW_RELEASE) {
        fog = 1 - fog;
        night = !night;

        view = myCamera.getViewMatrix();
        myCustomShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

        glUniform1i(fogLoc, fog);
        skyBoxShader.useShaderProgram();
        glUniform1i(fogSkyBoxLoc, fog);
    }
    // Toggle Directional Light
    if (key == GLFW_KEY_M && action == GLFW_RELEASE) {
        directionalLightEnabled = 1 - directionalLightEnabled;
        myCustomShader.useShaderProgram();

        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "directionalLightEnabled"), directionalLightEnabled);
   
    }
    

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (present) {
        return;
    }
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xDiff = (float)xpos - lastX;
    float yDiff = (float)ypos - lastY;
    lastX = (float)xpos;
    lastY = (float)ypos;

    xDiff *= sensitivity;
    yDiff *= sensitivity;

    yaw += xDiff;
    pitch = glm::clamp(pitch - yDiff, -89.0f, 89.0f);

    myCamera.rotate(pitch, yaw);

    view = myCamera.getViewMatrix();
    myCustomShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}

void presentScene() {
	if (value < 600) {
		if (value < 100) {
			myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		}
		if (value >= 200 && value < 300) {
				yaw -= 1.0f;
				myCamera.rotate(pitch, yaw);
		}
		if (value > 300 && value < 330) {
			myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		}
		if (value > 330 && value < 600) {
				yaw -= 0.1f;
				myCamera.rotate(pitch, yaw);
		}
	}
	if (value >= 600) {
		present = false;
		value = 0;
	}
	else {
		value++;
	}
}

void processMovement() {
    if (present) {
        presentScene();
        //dont get other input if we are in present mode
        return;
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


    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    //for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

    glWindow = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL Project", NULL, NULL);
    if (!glWindow) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return false;
    }

    glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
    glfwSetKeyCallback(glWindow, keyboardCallback);
    glfwSetCursorPosCallback(glWindow, mouseCallback);

    glfwMakeContextCurrent(glWindow);
    glfwSwapInterval(1);

#if not defined (__APPLE__)
    glewExperimental = GL_TRUE;
    glewInit();
#endif
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);
    glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

    return true;
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW); 
}

void initSkyBox()
{
    std::vector<const GLchar*> faces;

    faces.push_back("skybox/skyboxDay/posx.jpg");
    faces.push_back("skybox/skyboxDay/negx.jpg");
    faces.push_back("skybox/skyboxDay/posy.jpg");
    faces.push_back("skybox/skyboxDay/negy.jpg");
    faces.push_back("skybox/skyboxDay/posz.jpg");
    faces.push_back("skybox/skyboxDay/negz.jpg");

    mySkyBox.Load(faces);
    skyBoxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
}

void renderSkyBox() {
    glDepthMask(GL_FALSE);
    skyBoxShader.useShaderProgram();

    glm::mat4 viewMatrix = glm::mat4(glm::mat3(myCamera.getViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    mySkyBox.Draw(skyBoxShader, view, projection);

    glDepthMask(GL_TRUE);
}

void initModels() {
    initSkyBox();
 
    caravan.LoadModel("models/caravan/caravan.obj");
    caravan2.LoadModel("models/caravan/caravan.obj");
    merchant.LoadModel("models/merchant/merchant.obj");
    lantern.LoadModel("models/lantern/lantern.obj");
    quad.LoadModel("models/quad/quad.obj");
    staticScene.LoadModel("models/scene/staticScene.obj");
    ghost.LoadModel("models/ghost/ghost.obj");
   
}

void initShaders() {
	myCustomShader.loadShader("shaders/myShader.vert", "shaders/myShader.frag");
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    depthMapShader.loadShader("shaders/depthMap.vert", "shaders/depthMap.frag");

}

void initUniforms() {
    myCustomShader.useShaderProgram();

    // === Model Matrix ===
    model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

    // === View Matrix ===
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // === Normal Matrix ===
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");

    // === Projection Matrix ===
    projection = glm::perspective(glm::radians(90.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f); //!
    projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // === Light Direction ===
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    // === Light Color ===
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    // === Light Shader Projection Matrix ===
    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // === Fog ===
    myCustomShader.useShaderProgram();
    fogLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fog");
    glUniform1i(fogLoc, fog);


    // === Point Lights ===
    for (int i = 0; i < 4; i++) {
        std::string index = std::to_string(i);

        GLuint positionLoc = glGetUniformLocation(myCustomShader.shaderProgram, ("pointLights[" + index + "].position").c_str());
        glUniform3fv(positionLoc, 1, glm::value_ptr(pointLightPositions[i]));

        GLuint constantLoc = glGetUniformLocation(myCustomShader.shaderProgram, ("pointLights[" + index + "].constant").c_str());
        glUniform1f(constantLoc, 1.0f);

        GLuint linearLoc = glGetUniformLocation(myCustomShader.shaderProgram, ("pointLights[" + index + "].linear").c_str());
        glUniform1f(linearLoc, 0.09f);

        GLuint quadraticLoc = glGetUniformLocation(myCustomShader.shaderProgram, ("pointLights[" + index + "].quadratic").c_str());
        glUniform1f(quadraticLoc, 0.032f);

        GLuint ambientLoc = glGetUniformLocation(myCustomShader.shaderProgram, ("pointLights[" + index + "].ambient").c_str());
        glUniform3fv(ambientLoc, 1, glm::value_ptr(glm::vec3(0.3f, 0.3f, 0.1f)));

        GLuint diffuseLoc = glGetUniformLocation(myCustomShader.shaderProgram, ("pointLights[" + index + "].diffuse").c_str());
        glUniform3fv(diffuseLoc, 1, glm::value_ptr(glm::vec3(0.8f, 0.8f, 0.8f)));

        GLuint specularLoc = glGetUniformLocation(myCustomShader.shaderProgram, ("pointLights[" + index + "].specular").c_str());
        glUniform3fv(specularLoc, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));
    }

    // === SkyBox ===
    skyBoxShader.useShaderProgram();
    fogSkyBoxLoc = glGetUniformLocation(skyBoxShader.shaderProgram, "fog");
    glUniform1i(fogSkyBoxLoc, fog);

}

void initFBO() {
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
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
    glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * 50.0f * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 0.1f, far_plane = 600.0f;
    glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void renderModels(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();
    //send normal to shader
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    // fog / night color and shader
    if (fog == 1) {
        myCustomShader.useShaderProgram(); 
        lightColor = glm::vec3(0.05f, 0.05f, 0.1f); 
    } else {
        myCustomShader.useShaderProgram(); 
        lightColor = glm::vec3(1.0f, 1.0f, 1.0f); 
    }
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    // === Render Static Scene ===
    shader.useShaderProgram();
    staticScene.Draw(shader); 
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }

    // === Render Caravan 1 ===
    //movement logic for caravans
    caravan_x += (caravan_inc ? 0.01f : -0.01f);
    caravan_y += (caravan_inc ? 0.01f : -0.01f);

    //reverse direction
    if (caravan_x >= 10.0f || caravan_x <= 0.0f) {
        	caravan_inc = !caravan_inc;
    }

    model = glm::mat4(1.0f);
    //the caravans float if it's night
    model = glm::translate(model, glm::vec3(-40.5f, night ? 1.5f : -8.5f, -19.0f));
    model = glm::translate(model, glm::vec3(caravan_x, 0.0f, -caravan_y));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    caravan.Draw(shader);

    // === Render Caravan 2 ===
    model = glm::mat4(1.0f);  // Reset the model matrix
    model = glm::translate(model, glm::vec3(4.0f, night ? 1.5f : -8.5f, -13.0f));
    model = glm::translate(model, glm::vec3(-caravan_x, 0.0f, caravan_y));
    //model = glm::rotate(model, glm::radians(-1.5f * angle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    caravan2.Draw(shader);

    // === Render Merchant ===
    merchant_x += (merchant_inc ? 0.01f : -0.01f);
    merchant_y += (merchant_inc ? 0.01f : -0.01f);

    //reverse direction
    if (merchant_x >= 10.0f || merchant_x <= 0.0f) {
        merchant_inc = !merchant_inc;
    }
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(74.0f, night ? 5.0f : -1.0f, -8.0f));
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, merchant_y));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    merchant.Draw(shader);

    // === Render Lantern ===
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-20.0f, -8.0f, 0.0f));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));  // Rotate model
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    lantern.Draw(shader);

    // === Render Ghost ===
    if (night){
        model = glm::mat4(1.0f);  // Reset the model matrix
        model = glm::translate(model, glm::vec3(10.0f, 4.5f, 0.0f));
        model = glm::translate(model, glm::vec3(caravan_x, 0.0f, caravan_y));
        model = glm::rotate(model, glm::radians(3.0f * angle), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        if (!depthPass) {
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        }
        ghost.Draw(shader);
    }
   

    //reset
    model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));


}

void renderScene() {
    depthMapShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    renderModels(depthMapShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (showDepthMap) {
        glViewport(0, 0, retina_width, retina_height);
        glClear(GL_COLOR_BUFFER_BIT);
        screenQuadShader.useShaderProgram();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);
        glDisable(GL_DEPTH_TEST);
        quad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glViewport(0, 0, retina_width, retina_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myCustomShader.useShaderProgram();
        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        //light logic
        lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
        
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
        glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

        //models
        renderModels(myCustomShader, false);

        //light source
        lightShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        model = lightRotation;
        model = glm::translate(model,100.0f * lightDir);
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        //skybox
        mySkyBox.Draw(skyBoxShader, view, projection);
        
        if (present) {
            presentScene();
        }
    }
    angle += 0.1f;
}

void cleanup() {
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    glfwDestroyWindow(glWindow);
    //cleanup code for your own data
    glfwTerminate();
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
    initFBO();
	initUniforms();
    glCheckError();

	// application loop
    while (!glfwWindowShouldClose(glWindow)) {
        processMovement();
        renderScene();
        glfwPollEvents();
        glfwSwapBuffers(glWindow);
    }
	cleanup();
    return EXIT_SUCCESS;
}
