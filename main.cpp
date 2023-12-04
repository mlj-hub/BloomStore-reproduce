#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <ctime>

#include "inc/BloomFilter.h"

int main(int argc,char ** argv){
    int fd;
    fd = open(argv[1],O_RDWR|O_DIRECT);
    if(fd == -1){
        printf("unable to open device\n");
        exit(-1);
    }
    printf("fd: %d\n",fd);

    close(fd);
    return 0;
}