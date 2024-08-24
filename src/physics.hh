#ifndef PHYSICS_HH
#define PHYSICS_HH

#include <glm/glm.hpp>

struct BoxCollider{
    glm::vec2 pos;
    glm::vec2 dim;
    glm::vec2 center;
    float rot;
};


bool check_collision(BoxCollider * b1, BoxCollider * b2);

#endif
