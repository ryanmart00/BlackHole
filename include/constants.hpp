#ifndef CONSTANTS_HPP_INCLUDED
#define CONSTANTS_HPP_INCLUDED
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <iostream>
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#include <limits>



const glm::vec3 UP{0.0f, 1.0f, 0.0f};
const glm::vec3 FORWARD{0.0f, 0.0f, -1.0f};
const glm::vec3 RIGHT{1.0f, 0.0f, 0.0f};
const glm::vec3 ZERO{0.0f, 0.0f, 0.0f};

static std::ostream& operator<<(std::ostream& os, const glm::vec3& vec)
{
    os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return os;
} 

static std::ostream& operator<<(std::ostream& os, const glm::quat& q)
{
    os << "(" <<q.w <<","<< q.x << ", " << q.y << ", " << q.z << ")";
    return os;
} 

static std::ostream& operator<<(std::ostream& os, const glm::mat4& q)
{
    for(int i = 0; i < 16; i++)
    {
        if (i % 4 == 0)
        {
            os << "[";
        }
        os << q[i%4][i/4];
        if (i % 4 == 3)
        {
            os << "]" << std::endl;
        }
        else
        {
            os << ",";
        }
    }
    return os;
} 


static glm::vec3 quatForward(glm::quat q) 
{
    return q * FORWARD;
}

static glm::vec3 quatRight(glm::quat q) 
{
    return q * RIGHT;
}

static glm::vec3 quatUp(glm::quat q) 
{
    return q * UP;
}

static glm::mat4 fromOrthoNormalBasis(glm::vec3 x, glm::vec3 y, glm::vec3 z, glm::vec3 t)
{
    glm::mat4 m;
    m[0][0] = x.x;
    m[0][1] = y.x;
    m[0][2] = z.x;
    m[0][3] = 0.0f;

    m[1][0] = x.y;
    m[1][1] = y.y;
    m[1][2] = z.y;
    m[1][3] = 0.0f;

    m[2][0] = x.z;
    m[2][1] = y.z;
    m[2][2] = z.z;
    m[2][3] = 0.0f;

    m[3][0] = t.x;
    m[3][1] = t.y;
    m[3][2] = t.z;
    m[3][3] = 1.0f;

    return m;

}

const glm::vec4 BLACKHOLE_COLOR{0, 0, 0, 1};
const glm::vec4 BACKGROUND_COLOR{0.7f, 0.7f, 0.7f, 1.0f};

const int FIELD_PIXEL = 32;
const float FACTOR = 1.01;
const float LOOP_NUM = log(4)/log(FACTOR);

const float INIT_RS = log(0.1)/log(FACTOR);
const float INIT_STEP_SIZE = log(0.5f)/log(FACTOR);
const float THRESHOLD = 0.00001f;
const float INIT_MAX_DISTANCE = log(100.0f)/log(FACTOR);
const float INIT_LOOP = log(4.0f)/log(FACTOR);
const float EPSILON = 0.00001f;

const float FOV = 800/tan(glm::radians(45.0f)/2);
const float FOV2 = FOV*FOV;

const double PLAYER_MOUSE_SENSITIVITY = 0.005; 
const float PLAYER_SPEED = 1;
const float MAX_PLAYER_SPEED = 2;
const float MIN_PLAYER_SPEED = 0.1;
const float UP_CLAMP = 0.95f;

const float ORBIT_RATE = glm::radians(45.0f);

const float PI = glm::radians(360.0f);




#endif
