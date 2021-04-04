#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/scalar_constants.hpp" 
#include "glm/geometric.hpp" 
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/matrix.hpp"
#include <iostream> 
#include <tgmath.h>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"
#include "camera.hpp"
#include "game_object.hpp"
#include "rgb.hpp" 
#include "constants.hpp"

std::ostream& operator<<(std::ostream& os, const Vector& q)
{
    os << "(" << q.u << "," << q.Du << ")";
    return os;
} 

void mouse_button_callback(GLFWwindow*, int, int, int);
void cursor_position_callback(GLFWwindow*, double, double);
GLuint colorTexture();
void pollInput(GLFWwindow*, Camera&, float);


void stepDE(Vector& y, float h);
Vector f(Vector y);
bool stepRayMarch(Vector& y, float& theta, RGB& out);
RGB getPixelColor(glm::vec3 camPos, glm::vec3 dir);
void transform(const glm::mat4& t);
void restore();

std::vector<GameObject*> objects;
Camera cam{glm::vec3{5,0,0}, glm::vec3{-1,0,1}, glm::vec3{0,1,0}};

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 
                (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
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
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

	//Generation of the Shader Program
	//--------------------------------
    //
	
	Shader shader("assets/gl/quad.vs", "assets/gl/quad.fs");
    shader.setInt("image", 0);

    // Enable depth testing
    // glEnable(GL_DEPTH_TEST);


    /**
    btCollisionConfiguration* collisionConfig = new btDefaultCollisionConfiguration();
    btDispatcher* dispatcher = new btCollisionDispatcher(collisionConfig);
    btBroadphaseInterface* broadphase = new btDbvtBroadphase();
    world = new btCollisionWorld(dispatcher, broadphase, collisionConfig);

    btCollisionShape* shape = new btSphereShape{0.5f};

    world->addCollisionObject(new GameObject{btVector3{0,0,3},RGB{255,0,0},shape});

    world->addCollisionObject(new GameObject{btVector3{0,3,0},RGB{0,255,0},shape});

    world->addCollisionObject(new GameObject{btVector3{0,0,-4},RGB{0,0,255},shape});
    */
    
    GameObject* g1 = new SphereObject{glm::vec3{0,0,3}, RGB{255,0,0}, 0.5f};
    GameObject* g2 = new SphereObject{glm::vec3{0,4,0}, RGB{0,255,0}, 0.5f};
    GameObject* g3 = new SphereObject{glm::vec3{0,0,-4}, RGB{0,0,255}, 0.5f};
    
    objects.push_back(g1);
    objects.push_back(g2);
    objects.push_back(g3);


    GLuint image = colorTexture();

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

        g2->worldTrans = glm::translate(glm::identity<glm::mat4>(), 
                glm::vec3{0, 4.0f*cos(ORBIT_RATE * glfwGetTime()), 0});
        g3->transformGlobal(glm::mat4{glm::angleAxis(ORBIT_RATE * dt, UP)});
        
		//Clear
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Draw
        shader.use();
        image = colorTexture();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, image);
        renderQuad();

        /*
        debugDepthQuad.use();
        debugDepthQuad.setFloat("near_plane", NEAR_SHADOW_CLIPPING_PLANE);
        debugDepthQuad.setFloat("far_plane", FAR_SHADOW_CLIPPING_PLANE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadow_casters.front()->depthMap_);
//        renderQuad();
//      */

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
    cam.orientation_ = t * cam.orientation_;

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

GLuint colorTexture()
{
    GLubyte image [HEIGHT_SAMPLES*WIDTH_SAMPLES*3];

    glm::vec3 vertex_norms[4];
    for(int i = 0; i < 4; i++)
    {
        vertex_norms[i] = cam.orientation_ * VERTEX_NORMS[i];
    }

    glm::vec3 camPos = cam.position_;
    for (int i = 0; i < HEIGHT_SAMPLES; i++)
    {
        glm::vec3 left = (((float)i)/(HEIGHT_SAMPLES - 1) * vertex_norms[0]
            +(float)(HEIGHT_SAMPLES-1 - i)/(HEIGHT_SAMPLES-1) * vertex_norms[1]);
        glm::vec3 right = ((float)i/(HEIGHT_SAMPLES - 1) * vertex_norms[2]
            +(float)(HEIGHT_SAMPLES-1 - i)/(HEIGHT_SAMPLES - 1) * vertex_norms[3]);
        for (int j = 0; j < WIDTH_SAMPLES; j++)
        {
            glm::vec3 dir = ((float) j)/(WIDTH_SAMPLES - 1) * right 
                + (float)(WIDTH_SAMPLES-1 - j)/ (WIDTH_SAMPLES - 1) * left;
            RGB color = getPixelColor(cam.position_, dir);

            GLubyte* ptr = image + i*WIDTH_SAMPLES*3 + j*3;
            ptr[0] = color.r;
            ptr[1] = color.g;
            ptr[2] = color.b;
        }
    }

    GLuint texName;
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH_SAMPLES, HEIGHT_SAMPLES, 0, GL_RGB, 
            GL_UNSIGNED_BYTE, image);
    

    return texName;
}

RGB getPixelColor(glm::vec3 camPos, glm::vec3 dir)
{
    /**
    btVector3 x = convert(camPos);
    btVector3 z = convert(dir);
    z = z - x.dot(z)/x.length2() * x;
    btVector3 y = z.cross(x);
    x.normalize();
    y.normalize();
    z.normalize();
    btMatrix3x3 m{x,y,z};
    
    btTransform t;
    t.setIdentity();
    t.setBasis(m);
    
    // Now this stores the direction vector in the new coordinate system
    z = t*convert(dir);
    */

    glm::vec3 x = camPos;
    glm::vec3 z = dir;
    z = z - glm::dot(x, z)/glm::length2(x) * x;
    glm::vec3 y = glm::cross(z,x);
    x = glm::normalize(x);
    y = glm::normalize(y);
    z = glm::normalize(z);

    glm::mat4 t = fromOrthoNormalBasis(x,y,z, ZERO);

    z = t*glm::vec4{dir,1.0f};


    Vector vec{1/glm::length(camPos), -z.x/glm::length(camPos)/z.z};
    float theta = 0;
    RGB out{0,0,0}; 

    transform(t);
    
    for (int i = 0; i < MAX_ITERATIONS; i++)
    {
        if (1 < vec.u * 2 *GM_over_c)
        {
            //We are within the Schwartzschild radius
            restore();
            return BLACKHOLE_COLOR;
        }
        if (1 > vec.u * MAX_DISTANCE)
        {
            //We have escaped
            restore();
            return BACKGROUND_COLOR;
        }
        if (glm::radians(360.0f) < glm::abs(theta))
        {
            //We have gone around the circle
            restore();
            return BACKGROUND_COLOR;
        }
        if(stepRayMarch(vec, theta, out))
        {
            restore();
            return out;
        }

    }
    restore();
    std::cout << "Max iterations" << std::endl;

    return BACKGROUND_COLOR;
}

void transform(const glm::mat4& t)
{
    for(auto i = objects.begin(); i != objects.end(); i++)
    {
        GameObject* obj = *i;
        obj->setLocalTransform(t);
    }

    /*
    for(int i = 0; i < world->getNumCollisionObjects(); i++)
    {
        // transform each object to this plane
        GameObject* obj = (GameObject*)world->getCollisionObjectArray().at(i);
        obj->setWorldTransform(t * obj->save());
    }
    */
}

void restore()
{
    for(auto i = objects.begin(); i != objects.end(); i++)
    {
        GameObject* obj = *i;
        obj->restore();
    }
    /*
    for(int i = 0; i < world->getNumCollisionObjects(); i++)
    {
        // transform each object to this plane
        GameObject* obj = (GameObject*)world->getCollisionObjectArray().at(i);
        obj->restore();
    }
    */

}
/** Required for btCollisionWorld
struct Callback : btCollisionWorld::ContactResultCallback
{

    int color[3] = {0,0,0}; 
    int num = 0;

    btCollisionObject* test;

    Callback(btCollisionObject* t)
    {
        test = t;
    }

    void clear()
    {
        color[0] = 0;
        color[1] = 0;
        color[2] = 0;
        num = 0;
    }

    virtual btScalar addSingleResult(btManifoldPoint&,
            const btCollisionObjectWrapper* c0, int , int ,
            const btCollisionObjectWrapper* c1, int , int )
    {
        const btCollisionObject* c = c0->getCollisionObject();
        if(c == test)
        {
            c = c1->getCollisionObject();    
        }
        color[0] += ((GameObject*)c)->color.r;
        color[1] += ((GameObject*)c)->color.g;
        color[2] += ((GameObject*)c)->color.b;
        num++;
        return 0;
    }
    
};
*/

bool intersectsAny(GameObject* test)
{
    for(auto i = objects.cbegin(); i != objects.cend(); i++)
    {
        GameObject* obj = *i;
        if(obj != test && test->intersects(obj))
        {
            return true;
        }
    }
    return false;
}

bool stepRayMarch(Vector& y, float& theta, RGB& colorOut)
{
    float step = MAX_STEP_SIZE;
    
    SphereObject* test = new SphereObject{glm::vec3{cos(theta)/y.u, 0, sin(theta)/y.u},
        RGB{0,0,0}, 0};

    while(step > MIN_STEP_SIZE)
    {
        test->setRadius(step/y.u);

        if(intersectsAny(test))
        {
            step /= 2.0f;
        }
        else
        {
            delete test;
            for(int i = 0; i < NUM_CUTS; i++)
            {
                stepDE(y,step/NUM_CUTS);
            }
            theta += step;
            return false;
        }

    }

    int r = 0;
    int g = 0;
    int b = 0;
    int num = 0;

    for(auto i = objects.cbegin(); i != objects.cend(); i++)
    {
        GameObject* obj = (*i);
        if(obj != test && test->intersects(obj))
        {
            r += obj->color.r;
            b += obj->color.b;
            g += obj->color.g;
            num++;
        }
    }
    delete test;

    colorOut.r = r/num;
    colorOut.g = g/num;
    colorOut.b = b/num;

    return true;
}

void stepDE(Vector& y, float h)
{
    Vector k1 = f(y);
    Vector k2 = f(y + k1 * (h/2));
    Vector k3 = f(y + k2 * (h/2));
    Vector k4 = f(y + k3 * h);
    y = y + (k1 + k2*2 + k3*2 + k4) *(h/6);
}

Vector f(Vector y)
{
    return Vector{y.Du, 3*GM_over_c * y.u*y.u - y.u};
}
