#include "game_object.hpp"


GameObject::GameObject(const glm::mat4& trans, const RGB& c) :
    color{c}, worldTrans{trans}, localTrans{trans}
{
}

GameObject::GameObject(const glm::vec3& pos, const RGB& c) :
    color{c}, worldTrans{glm::identity<glm::mat4>()}, localTrans{glm::identity<glm::mat4>()}
{
    worldTrans = glm::translate(worldTrans, pos);
    localTrans = worldTrans;
}

void GameObject::restore()
{
    localTrans = worldTrans;
}

void GameObject::setLocalTransform(const glm::mat4& t)
{
    localTrans = t * worldTrans;
}


SphereObject::SphereObject(const glm::vec3& pos, const RGB& color, const float r)
    : GameObject{pos,color}, radius{r}
{
}

bool SphereObject::intersects(const GameObject* other)
{
    switch(other->getType())
    {
        case SPHERE:
        {
            SphereObject* o = (SphereObject*)other;
            return glm::length2(glm::vec3{localTrans * glm::vec4{0,0,0,1}}
                    -glm::vec3{o->localTrans * glm::vec4{0,0,0,1}}) 
                <= (radius + o->radius)*(radius + o->radius);
        }
        case RECTANGULAR_PRISM:
        {
            return false;
        }
    }
    return false;
}


ObjectType SphereObject::getType() const
{
    return ObjectType::SPHERE;
}

void SphereObject::setRadius(float r)
{
    radius = r;
}
