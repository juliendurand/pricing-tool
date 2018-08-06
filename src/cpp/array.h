#ifndef ARRAY_H_
#define ARRAY_H_

#include <string>


template <typename T>
class Array
{
private:
    std::string filename;
    int p;
    int n;
    std::size_t size_type;
    bool readonly;
    int size;
    int fd;
    void* mmappt;
    T* data;

public:
    Array(std::string filename, int p, int n, bool readonly);
    ~Array();
    T* getData();
    int getSize();
};

#endif  // ARRAY_H_
