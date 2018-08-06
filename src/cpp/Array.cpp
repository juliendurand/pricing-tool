#include "Array.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>


template<typename T>
Array<T>::Array(std::string filename, int p, int n, bool readonly):
    filename(filename),
    p(p),
    n(n),
    size_type(sizeof(T)),
    readonly(readonly),
    size(n * p * size_type)
{
    std::cout << "Loading data file : " << filename << std::endl;

    //Open file
    int open_flags = readonly ? O_RDONLY : O_RDWR | O_CREAT;
    fd = open(filename.c_str(), open_flags, static_cast<mode_t>(0777));
    assert(fd != -1);

    size_t filesize = getFilesize();
    if(readonly){
        assert(filesize == size);
    }
    else if(size != filesize){
        if (lseek(fd, size - 1, SEEK_SET) == -1)
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
    int mmap_flags = readonly ? PROT_READ : PROT_READ | PROT_WRITE;
    mmappt = mmap(NULL, size, mmap_flags, MAP_SHARED, fd, 0);
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

template<typename T>
int Array<T>::getSize(){
    return size;
}

template<typename T>
const T& Array<T>::operator[](size_t i) const{
    return data[i];
}

template<typename T>
size_t Array<T>::getFilesize() {
    struct stat st;
    stat(filename.c_str(), &st);
    return st.st_size;
}

template class Array<uint8_t>;
template class Array<int32_t>;
template class Array<float>;
template class Array<double>;