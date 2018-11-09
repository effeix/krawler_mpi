#ifndef __MAIN_H__
#define __MAIN_H__

#include <chrono>
#include <string>

#define ROOT_NODE 0

typedef std::chrono::high_resolution_clock Time;

typedef struct envvars_t {
    std::string URL;
    int NODES;
} envvars;

#endif/*__MAIN_H__*/
