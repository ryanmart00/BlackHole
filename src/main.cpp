#include "glm/common.hpp"
#include <ft2build.h>
#include <string>
#include FT_FREETYPE_H
#include <freetype2/freetype/config/ftheader.h>
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
void pollInput(GLFWwindow*, float);
void mouse_button_callback(GLFWwindow* window, int button, int action, int );
void window_size_callback(GLFWwindow* window, int width, int height);

Camera cam{glm::vec3{5,0,0}, glm::vec3{-1,0,0}, glm::vec3{0,1,0}};
int SCR_WIDTH = 800;
int SCR_HEIGHT = 800;


enum SimState
{
    TITLE,
    RUNNING,
    PAUSE
} state = TITLE;

const int NUM_FIELDS = 7;
const int SLIDER_START = 3;

enum Sliders
{
    MASS = 0,
    STEP = 1,
    LOOP = 2,
    DIST = 3,
};

std::string fieldNames[NUM_FIELDS] = {"Exit Simulation", "Reset Simulation", 
    "Resume Simulation", 
    "Black Hole Mass (more mass => larger): ",
    "Simulation Step  (smaller => more accurate): ",
    "Max Ray Loops (larger => more accurate): ",
    "Clipping Distance (larger => see further): "};
float sliders[NUM_FIELDS - SLIDER_START] = {M, MAX_STEP_SIZE, LOOP_NUM,
    MAX_DISTANCE};

GLuint tex;
GLuint textVAO = 0;
GLuint textVBO = 0;
void renderText(FT_Face& face, Shader& shader, const char* text, float x, float y, 
        float sx, float sy)
{
    if (textVAO == 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        shader.use();
        shader.setInt("tex", 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glGenVertexArrays(1, &textVAO);

        glBindVertexArray(textVAO);
        glGenBuffers(1, &textVBO);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);
    }

    const char* p;
    for(p = text; *p; p++)
    {
        if(FT_Load_Char(face, *p, FT_LOAD_RENDER))
        {
            continue;
        }
        
        FT_GlyphSlot g = face->glyph;

        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                g->bitmap.width,
                g->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                g->bitmap.buffer);

        float x2 = x + g->bitmap_left * sx;
        float y2 = -y - g->bitmap_top * sy;
        float w = g->bitmap.width * sx;
        float h = g->bitmap.rows * sy;
     
        GLfloat box[4][4] = {
            {x2,     -y2    , 0, 0},
            {x2 + w, -y2    , 1, 0},
            {x2,     -y2 - h, 0, 1},
            {x2 + w, -y2 - h, 1, 1},
        };
     
        glBindVertexArray(textVAO);
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
     
        x += (g->advance.x/64.0f) * sx;
        y += (g->advance.y/64.0f) * sy;
                
    }
}
glm::vec3 VERTEX_NORMS[4]; 

GLuint marchVAO = 0;
GLuint marchVBO;
void renderMarch()
{
    if (marchVAO == 0)
    {
        // setup plane VAO
        glGenVertexArrays(1, &marchVAO);
        glGenBuffers(1, &marchVBO);
        glBindVertexArray(marchVAO);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, marchVBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 
                (void*)(3 * sizeof(float)));
    }
    float marchVertices[] = {
        // positions        // Directions
        -1.0f,  1.0f, 0.0f, VERTEX_NORMS[0].x, VERTEX_NORMS[0].y, VERTEX_NORMS[0].z,
        -1.0f, -1.0f, 0.0f, VERTEX_NORMS[1].x, VERTEX_NORMS[1].y, VERTEX_NORMS[1].z,
         1.0f,  1.0f, 0.0f, VERTEX_NORMS[2].x, VERTEX_NORMS[2].y, VERTEX_NORMS[2].z,
         1.0f, -1.0f, 0.0f, VERTEX_NORMS[3].x, VERTEX_NORMS[3].y, VERTEX_NORMS[3].z
    };
    glBindVertexArray(marchVAO);
    glBindBuffer(GL_ARRAY_BUFFER, marchVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(marchVertices), &marchVertices, GL_STATIC_DRAW);
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

    window_size_callback(window, SCR_WIDTH, SCR_HEIGHT);

	#ifdef DEBUG 
	int numAttr;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttr);
        std::cout << "Max Vertex Attributes supported: " << numAttr << std::endl;
    #endif
    return window;
}

void initFreeType(FT_Library* ft, FT_Face* face)
{
    if(FT_Init_FreeType(ft))
    {
        std::cerr << "Couldn't init freetype!" << std::endl;
        std::exit(1);
    }

    if(FT_New_Face(*ft, "assets/FreeSans.ttf", 0, face))
    {
        std::cerr << "Couldn't open font!" << std::endl;
        std::exit(1);
    }

    FT_Set_Pixel_Sizes(*face, 0, 48);

}

double mouseX = 0;
double mouseY = 0;
float localTime = 0;
bool slider = false;
int sliderField = 0;
double delta = 0;

int main()
{
    //GLFW    
    GLFWwindow* window = initWindow();
    
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    FT_Library ft;
    FT_Face face;
    initFreeType(&ft, &face);

	//Generation of the Shader Program
	//--------------------------------
    //
    Shader text{"assets/text.vs", "assets/text.fs"};

    Shader march{"assets/march.vs", "assets/march.fs"};
    march.use();
    march.setFloat("M", glm::pow(FACTOR, sliders[MASS]));
    march.setVec4("BlackHoleColor", BLACKHOLE_COLOR);
    march.setVec4("BackgroundColor", BACKGROUND_COLOR);
    march.setFloat("Threshold", THRESHOLD);
    march.setFloat("EPSILON", EPSILON);
    march.setFloat("MaxStep", glm::pow(FACTOR, sliders[STEP]));
    march.setFloat("LOOP", glm::pow(FACTOR, sliders[LOOP]));
    march.setFloat("MaxDist", glm::pow(FACTOR, sliders[DIST]));

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
    orbit.setPosition(march, glm::vec3{4,0,0});
    orbit.setDimensions(march, glm::vec3{0.5f, 1, 0.5f});
    orbit.setOrientation(march, glm::mat3{1.0f});

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

        //Clear
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT); 

        if(state != PAUSE)
        {
            // Update objects
            march.use();
            march.setFloat("M", glm::pow(FACTOR, sliders[MASS]));
            march.setFloat("MaxStep", glm::pow(FACTOR, sliders[STEP]));
            march.setFloat("LOOP", glm::pow(FACTOR, sliders[LOOP]));
            march.setFloat("MaxDist", glm::pow(FACTOR, sliders[DIST]));
            pole.setPosition(march, glm::vec3{0, 0, 0});
                    
            pole.setOrientation(march, 
                    glm::mat3(glm::angleAxis((float)(ORBIT_RATE * localTime), 
                    glm::vec3{0.0f, 0.0f, 1.0f})));
            orbit.setPosition(march, glm::vec3{4.0f*cos(ORBIT_RATE * localTime), 0, 
                    4.0f*sin(ORBIT_RATE * localTime)});
            orbit.setOrientation(march, 
                   glm::mat3(glm::angleAxis((float)(ORBIT_RATE * localTime), 
                   glm::vec3{1.0f, 0.0f, 0.0f})));
            march.setVec3("Cam", cam.position_);
            march.setMat3("View", glm::mat3{cam.orientation_});
            

            //Draw
            march.use();
            renderMarch();

            //Show FPS
            text.use();
            text.setVec4("color", glm::vec4{1,1,1,.5f});
            float sx = 2.0 / SCR_WIDTH;
            float sy = 2.0 / SCR_HEIGHT;

            FT_Set_Pixel_Sizes(face, 0, 48);
            renderText(face, text, (std::string("FPS: ") 
                        + std::to_string((int)(1/dt))).c_str(), 
                        -1 + 8 * sx, 1 - 50 * sy, sx, sy);
        }

        float SX = 2.0 / SCR_WIDTH;
        float SY = 2.0 / SCR_HEIGHT;
        switch(state)
        {
        case TITLE:
        {
            FT_Set_Pixel_Sizes(face, 0, 48);
            text.use();
            text.setVec4("color", glm::vec4{.2,.2,.2,.75});

            float sx = SX/2;
            float sy = SY/2;
            renderText(face, text, "Click to Start Simulation",
                        - 245 * sx, 
                        -0.5, sx, sy);
            renderText(face, text, "Press Escape for Settings",
                        - 247 * sx, 
                        -0.75, sx, sy);
        }
            break;
        case RUNNING:
            localTime += dt;
            break;
        case PAUSE:
        {
            FT_Set_Pixel_Sizes(face, 0, 48);
            text.use();
            text.setVec4("color", glm::vec4{0,0,0,1});

            renderText(face, text, "Settings", 
                        -1 + 8 * SX, 1 - 50 * SY, SX, SY);
            FT_Set_Pixel_Sizes(face, 0, FIELD_PIXEL-8);

            for(int i = 0; i < SLIDER_START; i++)
            {
                text.setVec4("color", glm::vec4{((i+1)*FIELD_PIXEL < mouseY 
                            && mouseY < (i+2)*FIELD_PIXEL),0,0,1});
                renderText(face, text, fieldNames[i].c_str(), 
                        -1 + 8 * SX,  -1 + (i+1)*FIELD_PIXEL * SY, SX, SY);
            }
            for(int i = 0; i < NUM_FIELDS - SLIDER_START; i++)
            {
                int j = i + SLIDER_START;
                if (slider && sliderField == i)
                {
                    text.setVec4("color", glm::vec4(0,1,0,1));
                    renderText(face, text, (fieldNames[j] + std::to_string(
                                glm::pow(FACTOR, sliders[i] + delta))).c_str(), 
                        -1 + 8 * SX,  -1 + (j+1)*FIELD_PIXEL * SY, SX, SY);
                }
                else
                {
                    text.setVec4("color", glm::vec4{((j+1)*FIELD_PIXEL < mouseY 
                            && mouseY < (j+2)*FIELD_PIXEL),0,0,1});
                    renderText(face, text, (fieldNames[j] + std::to_string(
                                    glm::pow(FACTOR, sliders[i]))).c_str(), 
                        -1 + 8 * SX,  -1 + (j+1)*FIELD_PIXEL * SY, SX, SY);
                }
            }
        }
            break;
        }

		glfwSwapBuffers(window);
		glfwPollEvents();    
        pollInput(window, dt);
    }

    glfwDestroyWindow(window);
	glfwTerminate();
    #ifdef DEBUG
        std::cerr << "Successfully destroyed window" << std::endl;
    #endif
}


bool firstMouse = true;
double lastX = 0.0;
double lastY = 0.0;
void cursor_position_callback(GLFWwindow*, double xpos, double ypos)
{
    mouseX = xpos;
    mouseY = SCR_HEIGHT - ypos;
    switch(state)
    {
    case RUNNING:
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
        break;
    case PAUSE:
        if(slider)
        {
           delta = xpos - lastX;
        }
    case TITLE:
        break;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int )
{
    switch(state)
    {
    case TITLE:
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            state = RUNNING;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        break;
    case RUNNING:
        break;
    case PAUSE:
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            slider = false;
            sliders[sliderField] += delta;
        }
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && mouseY >= FIELD_PIXEL)
        {
            int field = (mouseY - FIELD_PIXEL)/FIELD_PIXEL;
            switch(field)
            {
            case 0: //exit
                glfwSetWindowShouldClose(window, true);
                break;
            case 1: //restart
                state = TITLE;
                localTime = 0;
                firstMouse = true;
                cam = Camera{glm::vec3{5,0,0}, glm::vec3{-1,0,0}, glm::vec3{0,1,0}};
                sliders[MASS] = M;
                sliders[STEP] = MAX_STEP_SIZE;
                sliders[LOOP] = LOOP_NUM;
                sliders[DIST] = MAX_DISTANCE;
                break;
            case 2: //resume
                state = RUNNING;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = true;
                break;
            default: //sliders
                slider = true;
                sliderField = field - SLIDER_START;
                lastX = mouseX;
                delta = 0;
            }
        }
        break;

    }
}

void pollInput(GLFWwindow *window, float dt)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_CAPS_LOCK) == GLFW_PRESS)
    {
        if(state == PAUSE)
        {
            state = RUNNING;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
            
        }
        else
        {
            state = PAUSE;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    if(state == RUNNING)
    {
        float speed = glm::clamp(PLAYER_SPEED * glm::length(cam.position_) 
                - 2*glm::pow(FACTOR, sliders[MASS]), MIN_PLAYER_SPEED, MAX_PLAYER_SPEED);
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            cam.position_ += glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cam.getRight()) 
                * dt * speed; 
        }
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            cam.position_ -= glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cam.getRight()) 
                * dt * speed;
        }
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            cam.position_ -= cam.getRight() * dt * speed;
        }
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            cam.position_ += cam.getRight() * dt * speed;
        }
        if(glfwGetKey(window,GLFW_KEY_SPACE) == GLFW_PRESS) 
        {
            cam.position_ += glm::vec3(0.0f,1.0f,0.0f) * dt * speed;

        }
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) 
        {
            cam.position_ -= glm::vec3(0.0f,1.0f,0.0f) * dt * speed;
        }
    }
}

void window_size_callback(GLFWwindow* , int width, int height)
{
    glViewport(0,0,width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    float upAngle = atan((float)height/FOV);
    float rightAngle = atan(width/sqrt(height*height + FOV2));
    VERTEX_NORMS[0] =  glm::angleAxis(rightAngle,
                glm::angleAxis(upAngle,RIGHT)*UP)
            * glm::angleAxis(upAngle, RIGHT) * FORWARD;
    VERTEX_NORMS[1] = glm::angleAxis(rightAngle,
                glm::angleAxis(-upAngle,RIGHT)*UP)
            * glm::angleAxis(-upAngle, RIGHT) * FORWARD;
    VERTEX_NORMS[2] = glm::angleAxis((float)-rightAngle,
                glm::angleAxis(upAngle,RIGHT)*UP)
            * glm::angleAxis(upAngle, RIGHT) * FORWARD;
    VERTEX_NORMS[3] = glm::angleAxis((float)-rightAngle,
                glm::angleAxis(-upAngle,RIGHT)*UP)
            * glm::angleAxis(-upAngle, RIGHT) * FORWARD;

}
