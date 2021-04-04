#ifndef GAME_OBJECT_HPP_INCLUDED
#define GAME_OBJECT_HPP_INCLUDED

#include <glm/glm.hpp>
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/norm.hpp"
#include "rgb.hpp"

enum ObjectType
{
    SPHERE,
    RECTANGULAR_PRISM
};

class GameObject
{
public: 
    RGB color;

    GameObject(const glm::vec3& pos, const RGB& color);
    GameObject(const glm::mat4& trans, const RGB& color);

    void restore();
    void setLocalTransform(const glm::mat4&);

    virtual bool intersects(const GameObject* ) = 0;

    virtual ObjectType getType() const = 0;


protected:
    glm::mat4 worldTrans;
    glm::mat4 localTrans;

};

class SphereObject : public GameObject
{
public:
    SphereObject(const glm::vec3& pos, const RGB& color, const float radius);
    virtual bool intersects(const GameObject* );

    virtual ObjectType getType() const;

    void setRadius(float r);
private:
    float radius;

};


#endif

