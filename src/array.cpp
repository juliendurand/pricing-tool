#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include "array.h"


size_t getFilesize(const std::string filename) {
    struct stat st;
    stat(filename.c_str(), &st);
    return st.st_size;
}

template<typename T>
Array<T>::Array(){
}

template<typename T>
Array<T>::Array(std::string filename, int p, int n):
    filename(filename), p(p), n(n)
{
    std::cout << "Lodading data file : " << filename << std::endl;
    size = n * p;

    //Open file
    fd = open(filename.c_str(), O_RDWR | O_CREAT /*O_RDONLY*/, (mode_t)0777);
    assert(fd != -1);

    size_t filesize = getFilesize(filename);
    //assert(filesize == size);
    if(size != filesize){
        if (lseek(fd, size, SEEK_SET) == -1)
        {
            close(fd);
            perror("Error calling lseek() to 'stretch' the file");
            exit(EXIT_FAILURE);
        }
        if (write(fd, "", 1) == -1)
        {
            close(fd);
            perror("Error writing last byte of the file");
            exit(EXIT_FAILURE);
        }
    }

    //Execute mmap
    mmappt = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    assert(mmappt != MAP_FAILED);
    data = reinterpret_cast<T*>(mmappt);
}

template<typename T>
Array<T>::~Array(){
    int rc = munmap(mmappt, size);
    assert(rc == 0);
    close(fd);
}

template<typename T>
T* Array<T>::getData(){
    return data;
}

template class Array<uint8_t>;
template class Array<float>;
template class Array<double>;
