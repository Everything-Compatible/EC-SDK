#pragma once
#include <vector>
#include <string_view>
#include <Windows.h>

#ifndef PARRAY_DEFINITION
#define PARRAY_DEFINITION

inline void* GlobalHeapAllocateMemory(size_t Size)
{
	return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Size);
}

inline void GlobalHeapFreeMemory(const void* Ptr)
{
	HeapFree(GetProcessHeap(), 0, const_cast<void*>(Ptr));
}

template<typename T>
struct PArray
{
	size_t N;
	const T* Data;

	PArray() :N(0), Data(nullptr) {}
	PArray(const std::vector<T>& p) :N(p.size()), Data(p.data()) {}
	template<size_t _N>
	PArray(const T p[_N]) : N(_N), Data(p) {}
    PArray(size_t _N, const T* p) : N(_N), Data(p) {}

    void Delete()
    {
        if (Data != nullptr)
        {
            GlobalHeapFreeMemory(Data);
            Data = nullptr;
            N = 0;
        }
    }
    void Alloc(size_t Size)
    {
        Delete();
        if (Size > 0)
        {
            Data = GlobalHeapAllocateMemory(sizeof(T) * Size);
            for (size_t i = 0; i < Size; i++)
            {
                new ((T*)Data + i) T();
            }
            N = Size;
        }
    }
	std::string_view AsStringView() const
	{
		return std::string_view((const char*)Data, N * sizeof(T));
	}
};

template<typename T>
PArray<T> GetNullPArray()
{
    return PArray<T>();
}

#endif