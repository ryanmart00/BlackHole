#include "clock.hpp"
#include "constants.hpp"
#include <string>

int Clock::Num = 0;
Clock::Clock(Shader& s, glm::vec3 pos, glm::mat3 ori, glm::vec4 dim, bool geo,
            glm::vec3 rim_amb, glm::vec3 rim_diff, glm::vec3 rim_spec, float rim_shine,
            glm::vec3 hand_amb, glm::vec3 hand_diff, glm::vec3 hand_spec, float hand_shine) :
    shader_{s}, position_{pos}, seconds_{ori}, tenths_{glm::mat3{1.0f}}, dimensions_{dim}, 
    geodesic_{geo}
{
    num_ = Num;
    Num++;
    s.setFloat("ClockCount", Num);
    s.setVec3("clocks[" + std::to_string(num_) + "].rim.ambient", rim_amb);
    s.setVec3("clocks[" + std::to_string(num_) + "].rim.diffuse", rim_diff);
    s.setVec3("clocks[" + std::to_string(num_) + "].rim.specular", rim_spec);
    s.setFloat("clocks[" + std::to_string(num_) + "].rim.shininess", rim_shine);

    s.setVec3("clocks[" + std::to_string(num_) + "].hand.ambient", hand_amb);
    s.setVec3("clocks[" + std::to_string(num_) + "].hand.diffuse", hand_diff);
    s.setVec3("clocks[" + std::to_string(num_) + "].hand.specular", hand_spec);
    s.setFloat("clocks[" + std::to_string(num_) + "].hand.shininess", hand_shine);

    s.setVec3("clocks[" + std::to_string(num_) + "].position", position);
    s.setVec3("clocks[" + std::to_string(num_) + "].dimensions", dim);
    s.setVec3("clocks[" + std::to_string(num_) + "].seconds", seconds);
    s.setVec3("clocks[" + std::to_string(num_) + "].tenths", tenths);
}

Clock::update(float dt)
{
    if(geodesic_)
    {
        //update position and proper time
    }
    else
    {
        //update proper time = global time
        seconds_ = seconds_ * glm::angleAxis(PI/60 * dt, UP);
        tenths_ = tenths_ * glm::angleAxis(PI * dt, UP);
        s.setVec3("clocks[" + std::to_string(num_) + "].seconds", seconds_);
        s.setVec3("clocks[" + std::to_string(num_) + "].tenths", tenths_);
        
    }
}


