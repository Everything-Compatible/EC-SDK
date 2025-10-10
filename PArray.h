#pragma once
#include <vector>

#ifndef PARRAY_DEFINITION
#define PARRAY_DEFINITION
template<typename T>
struct PArray
{
	size_t N;
	const T* Data;

	PArray() :N(0), Data(nullptr) {}
	PArray(const std::vector<T>& p) :N(p.size()), Data(p.data()) {}
	template<size_t _N>
	PArray(const T p[_N]) : N(_N), Data(p) {}

    void Delete()
    {
        if (Data != nullptr)
        {
            delete[] Data;
            Data = nullptr;
            N = 0;
        }
    }
    void Alloc(size_t Size)
    {
        Delete();
        if (Size > 0)
        {
            Data = new T[Size];
            N = Size;
        }
    }
};

template<typename T>
PArrat<T> GetNullPArray()
{
    return PArray<T>();
}

#endif