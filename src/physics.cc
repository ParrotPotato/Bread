#include "physics.hh"

#include <imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <unordered_set>

struct HashGlmVec2 {
    size_t operator () (const glm::vec2 & a) const {
        unsigned int xhashint = *((unsigned int *)(&a.x));
        unsigned int yhashint = *((unsigned int *)(&a.y));
        return  (xhashint) ^ (yhashint << 1);
    }
};

struct EqualGlmVec2 {
    bool operator() (const glm::vec2 & a, const glm::vec2 & b) const {
        return a == b;
    }
};

bool check_collision_separating_axis_theorem(const std::vector<glm::vec2> & a, const std::vector<glm::vec2> & b){
    std::unordered_set<glm::vec2, HashGlmVec2, EqualGlmVec2> axises;

    for(unsigned int i = 0; i < a.size(); i++){
        const glm::vec2 along = glm::normalize((a[i] - a[(i + 1) % a.size()]));
        axises.insert(glm::vec2(along.y, -along.x));
    }

    for(unsigned int i = 0; i < b.size(); i++){
        const glm::vec2 along = glm::normalize((b[i] - b[(i + 1) % b.size()]));
        axises.insert(glm::vec2(along.y, -along.x));
    }


    for(const glm::vec2 axis : axises){

        float amin = 10e10;
        float amax = -10e10;
        float bmin = 10e10;
        float bmax = -10e10;
        for(const glm::vec2 point: a){
            float dot = glm::dot(axis, point);
            amin = amin > dot ? dot : amin;
            amax = dot > amax ? dot : amax;
        }
        for(const glm::vec2 point: b){
            float dot = glm::dot(axis, point);
            bmin = bmin > dot ? dot : bmin;
            bmax = dot > bmax ? dot : bmax;
        }

        if ( 
                (amax >= bmax && bmax >= amin) == false 
                && (amax >= bmin && bmin >= amin) == false 
                && (bmax >= amax && amax >= bmin) == false
           )
            return false;
    }
    return true;
}

// @note: this is an expensive function which performs a sa

bool check_collision(BoxCollider * a, BoxCollider * b){

    // @notes: this is a heap allocation
    std::vector<glm::vec2> apoints(4);
    std::vector<glm::vec2> bpoints(4);

    {
        apoints[0] = glm::vec2(a->pos.x - a->dim.x * 0.5, a->pos.y - a->dim.y * 0.5);
        apoints[1] = glm::vec2(a->pos.x - a->dim.x * 0.5, a->pos.y + a->dim.y * 0.5);
        apoints[2] = glm::vec2(a->pos.x + a->dim.x * 0.5, a->pos.y + a->dim.y * 0.5);
        apoints[3] = glm::vec2(a->pos.x + a->dim.x * 0.5, a->pos.y - a->dim.y * 0.5);
        glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), a->rot, glm::vec3(0.0f, 0.0f, 1.0f));

        for(unsigned int i = 0; i < 4 ; i++){
            apoints[i] = a->center + glm::vec2(rotation_matrix *  glm::vec4((apoints[i] - a->center), 0.0, 1.0));
        }
    }

    {
        bpoints[0] = glm::vec2(b->pos.x - b->dim.x * 0.5, b->pos.y - b->dim.y * 0.5);
        bpoints[1] = glm::vec2(b->pos.x - b->dim.x * 0.5, b->pos.y + b->dim.y * 0.5);
        bpoints[2] = glm::vec2(b->pos.x + b->dim.x * 0.5, b->pos.y + b->dim.y * 0.5);
        bpoints[3] = glm::vec2(b->pos.x + b->dim.x * 0.5, b->pos.y - b->dim.y * 0.5);
        glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), b->rot, glm::vec3(0.0f, 0.0f, 1.0f));

        for(unsigned int i = 0; i < 4 ; i++){
            bpoints[i] = b->center + glm::vec2(rotation_matrix *  glm::vec4((bpoints[i] - b->center), 0.0, 1.0));
        }

    }

    bool result = check_collision_separating_axis_theorem(apoints, bpoints);

    return result;
}

