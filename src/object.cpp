#include "object.hpp"
#include <string>

int Object::Num = 0;
Object::Object(Shader& s, glm::vec3 amb, glm::vec3 diff, glm::vec3 spec, float shine)
{
    num = Num;
    Num++;
    s.setVec3("objects[" + std::to_string(num) + "].mat.ambient", amb);
    s.setVec3("objects[" + std::to_string(num) + "].mat.diffuse", diff);
    s.setVec3("objects[" + std::to_string(num) + "].mat.specular", spec);
    s.setFloat("objects[" + std::to_string(num) + "].mat.shininess", shine);
}

void Object::setPosition(Shader& s, glm::vec3 pos)
{
    s.setVec3("objects[" + std::to_string(num) + "].position", pos);
}

void Object::setOrientation(Shader& s, glm::mat3 m)
{
    s.setMat3("objects[" + std::to_string(num) + "].orient", m);
}


void Object::setDimensions(Shader& s, glm::vec3 dim)
{
    s.setVec3("objects[" + std::to_string(num) + "].dimensions", dim);
}
