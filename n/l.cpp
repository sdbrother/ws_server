#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <errno.h>

int main(){


    struct rlimit limit;

//    for (;;){
    if (getrlimit(RLIMIT_NOFILE, &limit) != 0) {
        std::cout << "getrlimit() failed with errno" << std::endl;
        return 1;
    }
    std::cout << limit.rlim_cur << std::endl;
    std::cout << limit.rlim_max << std::endl;

//    }

    return 0;
}