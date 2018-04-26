#ifndef ARRAY_H_
#define ARRAY_H_

#include <string>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>

template <typename T>
class Array
{
    std::string filename;
    int p;
    int n;
    int size;
    int fd;
    void* mmappt;
    T* data;

public:
    Array(std::string filename, int p, int n);
    ~Array();
    T* getData();
};

#endif  // ARRAY_H_
