#include "util.hh"

#include <cstdio>

char * load_entire_file_in_system_mem(MemoryArena * memory, const char *filepath){
    // should be later replaced with open call (instead of fopen)
    FILE * fptr = fopen(filepath, "rb");
    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    char * string = (char *) malloc(sizeof(char) * fsize + 1);
    string[fsize] = 0;
    fread(string, fsize, 1, fptr);
    return string;
}



