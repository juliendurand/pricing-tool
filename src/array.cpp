 #include "array.h"
#include <errno.h>

#include <iostream>

size_t getFilesize(const std::string filename) {
    struct stat st;
    stat(filename.c_str(), &st);
    return st.st_size;
}

template<typename T>
Array<T>::Array(std::string filename, int p, int n):
    filename(filename), p(p), n(n)
{
    size = n * p;
    size_t filesize = getFilesize(filename);
    assert(filesize == size);

    //Open file
    fd = open(filename.c_str(), O_RDONLY, 0);
    assert(fd != -1);

    //Execute mmap
    mmappt = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    assert(mmappt != MAP_FAILED);
    data = reinterpret_cast<T*>(mmappt);
    //std::cout << mmappt << " " << (void*) data << " " << "ok" << std::endl;
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
