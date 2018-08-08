#ifndef ARRAY_H_
#define ARRAY_H_

#include <string>

// Provides read access to a 2-dimensional numpy array persisted on disk. Uses
// mmap to ensure optimal performance.
//
// Template :
//      - the template type T is used to determine the type of data in the
//        numpy array.
//
// Params :
//      - filename : name of the file containing the numpy array ;
//      - p : number of features (horizontal dimension) ;
//      - n : number of observations (vertical dimension) ;
//
// Usage : a pointer to the file content is available with getData(). To access
//         the value in column i and row j : getData()[i + j * p].

template <typename T>
class Array
{
public:
    Array(const std::string filename, int p, int n);
    ~Array();
    T* getData();
    const T& operator[](size_t i) const;

private:
    const std::string filename;
    const int p;
    const int n;
    const std::size_t size_type;
    const int size;
    int fd;
    void* mmappt;
    T* data;

    size_t getFilesize();
};

#endif  // ARRAY_H_
