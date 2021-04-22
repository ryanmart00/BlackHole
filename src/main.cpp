#include "glm/exponential.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/scalar_constants.hpp" 
#include "glm/geometric.hpp" 
#include "glm/gtc/constants.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/matrix.hpp"
#include <iostream> 
#include <math.h>
#include <tgmath.h>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include "object.hpp"

void cursor_position_callback(GLFWwindow*, double, double);
GLuint colorTexture();
void pollInput(GLFWwindow*, Camera&, float);

Camera cam{glm::vec3{5,0,0}, glm::vec3{-1,0,0}, glm::vec3{0,1,0}};

unsigned int marchVAO = 0;
unsigned int marchVBO;
void renderMarch()
{
    if (marchVAO == 0)
    {
        float marchVertices[] = {
            // positions        // Directions
            -1.0f,  1.0f, 0.0f, VERTEX_NORMS[0].x, VERTEX_NORMS[0].y, VERTEX_NORMS[0].z,
            -1.0f, -1.0f, 0.0f, VERTEX_NORMS[1].x, VERTEX_NORMS[1].y, VERTEX_NORMS[1].z,
             1.0f,  1.0f, 0.0f, VERTEX_NORMS[2].x, VERTEX_NORMS[2].y, VERTEX_NORMS[2].z,
             1.0f, -1.0f, 0.0f, VERTEX_NORMS[3].x, VERTEX_NORMS[3].y, VERTEX_NORMS[3].z
        };
        // setup plane VAO
        glGenVertexArrays(1, &marchVAO);
        glGenBuffers(1, &marchVBO);
        glBindVertexArray(marchVAO);
        glBindBuffer(GL_ARRAY_BUFFER, marchVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(marchVertices), &marchVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 
                (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(marchVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

/**
 * Initializes the GLFW Window and starts up GLAD
 */
GLFWwindow* initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	//Initialization of Window
	//------------------------
	#ifdef DEBUG
        std::cout << "Initializing Window..." << std::endl;
    #endif
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Black Hole", NULL, NULL);
	if (window == NULL)
	{
        std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
        std::exit(1);
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
        std::cerr << "Failed to initialize GLAD" << std::endl;
		std::exit(1);
	}   

	//glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 



	#ifdef DEBUG 
	int numAttr;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttr);
        std::cout << "Max Vertex Attributes supported: " << numAttr << std::endl;
    #endif
    return window;
}

int main()
{
    //GLFW    
    GLFWwindow* window = initWindow();
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, cursor_position_callback);

	//Generation of the Shader Program
	//--------------------------------
    //

    Shader march{"assets/march.vs", "assets/march.fs"};
    march.use();
    march.setFloat("M", GM_over_c);
    march.setVec4("BlackHoleColor", BLACKHOLE_COLOR);
    march.setVec4("BackgroundColor", BACKGROUND_COLOR);
    march.setFloat("MaxDist", MAX_DISTANCE);
    march.setFloat("Threshold", THRESHOLD);
    march.setFloat("MaxStep", MAX_STEP_SIZE);
    march.setFloat("EPSILON", EPSILON);
    march.setInt("MAX_ITER", MAX_ITERATIONS);

    // Set point lights
    march.setVec3("Lights[0].position", glm::vec3{6.0f, 0.0f, 6.0f});
    march.setVec3("Lights[0].ambient", glm::vec3{0.5f, 0.5f, 0.5f});
    march.setVec3("Lights[0].diffuse", glm::vec3{1.0f, 1.0f, 1.0f});
    march.setVec3("Lights[0].specular", glm::vec3{1.0f, 1.0f, 1.0f});

    march.setMat3("rot", glm::mat3(glm::angleAxis(5*glm::pi<float>()/8, glm::vec3{0,0,1})));

    march.setVec3("Lights[1].position", glm::vec3{-6.0f, 0.0f, -6.0f});
    march.setVec3("Lights[1].ambient", glm::vec3{0.5f, 0.5f, 0.5f});
    march.setVec3("Lights[1].diffuse", glm::vec3{1.0f, 1.0f, 1.0f});
    march.setVec3("Lights[1].specular", glm::vec3{1.0f, 1.0f, 1.0f});

    Object stat(march, glm::vec3{0.3, 0.0, 0.0}, glm::vec3{1.0, 0.0, 0.0},
            glm::vec3{0.7, 0.6, 0.6}, 32);
    stat.setPosition(march, glm::vec3{1.0,0.0,-1.0});
    stat.setOrientation(march, glm::mat3{1.0f});
    stat.setDimensions(march, glm::vec3{0.5f, 0.0f, 0.0f});

    Object pole(march, glm::vec3{0.0, 0.3, 0.0}, glm::vec3{0.0, 1.0, 0.0},
            glm::vec3{0.6, 0.7, 0.6}, 32);
    pole.setDimensions(march, glm::vec3{1, 0.2f, 0.5f});
    pole.setOrientation(march, glm::mat3{1.0f});

    Object orbit(march, glm::vec3{0.0, 0.0, 0.3}, glm::vec3{0.0, 0.0, 1.0},
            glm::vec3{0.6, 0.6, 0.7}, 32);
    orbit.setDimensions(march, glm::vec3{0.5f, 1, 0.5f});
    orbit.setOrientation(march, glm::mat3{1.0f});

    float dt = 0;
    float lastFrame = glfwGetTime();
	//Main Loop
	//---------
	while(!glfwWindowShouldClose(window))
	{
        // calculate dt
        float currentFrame = glfwGetTime();
        dt = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Update objects

        march.use();
        march.setFloat("M", 0.25f * (1-cos(0.1 * glfwGetTime())) );
        pole.setPosition(march, glm::vec3{0, 0, 0});
                
        pole.setOrientation(march, 
                glm::mat3(glm::angleAxis((float)(ORBIT_RATE * glfwGetTime()), 
                glm::vec3{0.0f, 0.0f, 1.0f})));
        orbit.setPosition(march, glm::vec3{4.0f*cos(ORBIT_RATE * glfwGetTime()), 0, 
                4.0f*sin(ORBIT_RATE * glfwGetTime())});
        orbit.setOrientation(march, 
                glm::mat3(glm::angleAxis((float)(ORBIT_RATE * glfwGetTime()), 
                glm::vec3{1.0f, 0.0f, 0.0f})));
        march.setVec3("Cam", cam.position_);
        march.setMat3("View", glm::mat3{cam.orientation_});
        
		//Clear
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT); 

		//Draw
        march.use();
        renderMarch();

		glfwSwapBuffers(window);
		glfwPollEvents();    
        pollInput(window, cam, dt);
    }

    glfwDestroyWindow(window);
	glfwTerminate();
    #ifdef DEBUG
        std::cerr << "Successfully destroyed window" << std::endl;
    #endif
}

void mouse_button_callback(GLFWwindow* , int button, int action, int )
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // zoom out
    }
}

bool firstMouse = true;
double lastX = 0.0;
double lastY = 0.0;
void cursor_position_callback(GLFWwindow*, double xpos, double ypos)
{
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    double dx = -xpos + lastX;
    double dy = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    dx *= PLAYER_MOUSE_SENSITIVITY;
    dy *= PLAYER_MOUSE_SENSITIVITY;

    glm::vec3 right = cam.getRight();
    glm::quat t = glm::identity<glm::quat>();
    if(glm::abs(glm::dot(glm::angleAxis((float)dy, right)
                    * cam.getForward(), UP)) < UP_CLAMP)
    {
        t = glm::angleAxis((float)dy, right) * t;
    }
    t = glm::angleAxis((float)dx, UP) * t;
    cam.orientation_ = glm::normalize(t * cam.orientation_);

}

void pollInput(GLFWwindow *window, Camera& cam, float dt)
{
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_CAPS_LOCK))
	{
		glfwSetWindowShouldClose(window, true);
	}
	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
        cam.position_ += glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cam.getRight()) 
            * dt * PLAYER_SPEED;
    }
	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
        cam.position_ -= glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cam.getRight()) 
            * dt * PLAYER_SPEED;
	}
	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
        cam.position_ -= cam.getRight() * dt * PLAYER_SPEED;
	}
	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
        cam.position_ += cam.getRight() * dt * PLAYER_SPEED;
	}
    if(glfwGetKey(window,GLFW_KEY_SPACE) == GLFW_PRESS) 
    {
        cam.position_ += glm::vec3(0.0f,1.0f,0.0f) * dt * PLAYER_SPEED;

    }
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) 
    {
        cam.position_ -= glm::vec3(0.0f,1.0f,0.0f) * dt * PLAYER_SPEED;
    }
    if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
            std::cout << 1/dt << std::endl;
    }
}
