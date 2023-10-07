#ifndef CLOCK_HPP_INCLUDED
#define CLOCK_HPP_INCLUDED
#include "glm/glm.hpp"
#include "shader.hpp"

class Clock
{
public:
    Clock(Shader& shader, glm::vec3 pos, glm::mat3 ori, glm::vec4 dim, bool geodesic,
            glm::vec3 rim_amb, glm::vec3 rim_diff, glm::vec3 rim_spec, float rim_shine,
            glm::vec3 hand_amb, glm::vec3 hand_diff, glm::vec3 hand_spec, float hand_shine);

    void update(float dt);

    static int Num;

    int num_;
    Shader& shader_;
    glm::vec3 position_;
    float seconds_;
    float tenths_;
    glm::vec4 dimensions_;
    bool geodesic_;


    
};

#endif 
