#ifndef PHYSICS_HH
#define PHYSICS_HH

#include <glm/glm.hpp>
 
#define NONE    0
#define GRAVITY 1
#define STATIC  2


struct BoxCollider{
    glm::vec2 pos;
    glm::vec2 dim;
    glm::vec2 center;
    glm::vec2 velocity;
    float rot;

    unsigned int properties;
};


bool check_collision_via_sat(BoxCollider * b1, BoxCollider * b2);


#endif
