#ifndef PHYSICS_HH
#define PHYSICS_HH

#include <glm/glm.hpp>


struct SimpleCollider{
    glm::vec2 pos;
    glm::vec2 dim;
    glm::vec2 center;
    float rotation;
};

bool check_collision(SimpleCollider * a, SimpleCollider * b);

#endif
