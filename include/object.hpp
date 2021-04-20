#ifndef OBJECT_HPP_INCLUDED
#define OBJECT_HPP_INCLUDED
#include "glm/glm.hpp"
#include "shader.hpp"

class Object
{
public:
    Object(Shader& s, glm::vec3 amb, glm::vec3 diff, 
            glm::vec3 spec, float shine);

    void setPosition(Shader& s, glm::vec3 pos);

    void setOrientation(Shader& s, glm::mat3 ori);

    void setDimensions(Shader& s, glm::vec3 dim);

    static int Num;

    int num;

    
};

#endif 
