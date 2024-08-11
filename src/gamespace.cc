#include "gamespace.hh"

#include <cstdio>

void enter_game_space(void *){
    printf("enter game called\n");
}


void * before_reset(void){
    printf("before reset called\n");
    return NULL;
}

void after_reset(void *){
    printf("after reset called\n");
}
