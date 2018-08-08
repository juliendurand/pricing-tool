#include "Array.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>


template<typename T>
Array<T>::Array(std::string filename, int p, int n):
    filename(filename),
    p(p),
    n(n),
    size_type(sizeof(T)),
    size(n * p * size_type)
{
    std::cout << "Loading data file : " << filename << std::endl;

    // Open file
    fd = open(filename.c_str(), O_RDONLY, static_cast<mode_t>(0777));
    assert(fd != -1);

    // Check file size
    size_t filesize = getFilesize();
    assert(filesize == size);

    // Execute mmap
    mmappt = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    assert(mmappt != MAP_FAILED);
    data = reinterpret_cast<T*>(mmappt);
}

template<typename T>
Array<T>::~Array()
{
    int rc = munmap(mmappt, size);
    assert(rc == 0);
    close(fd);
}

template<typename T>
T* Array<T>::getData()
{
    return data;
}

template<typename T>
const T& Array<T>::operator[](size_t i) const
{
    return data[i];
}

template<typename T>
size_t Array<T>::getFilesize()
{
    struct stat st;
    stat(filename.c_str(), &st);
    return st.st_size;
}

template class Array<uint8_t>;
template class Array<int32_t>;
template class Array<float>;
template class Array<double>;
