#ifndef ARRAY_H_
#define ARRAY_H_

#include <string>


template <typename T>
class Array
{
public:
    Array(const std::string filename, int p, int n, bool readonly);
    ~Array();
    T* getData();
    int getSize();

private:
    const std::string filename;
    const int p;
    const int n;
    const std::size_t size_type;
    const bool readonly;
    const int size;
    int fd;
    void* mmappt;
    T* data;

    size_t getFilesize();
};

#endif  // ARRAY_H_
