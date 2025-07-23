#pragma once
#include <Windows.h>
#include <YRPP.h>

namespace ECMemory
{
	inline LPVOID Allocate(size_t sz) {
		return HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, sz);
	}

	inline void Deallocate(LPVOID mem) {
		HeapFree(GetProcessHeap(), 0, mem);
	}
}

template <typename T>
struct ECAllocator {
	using value_type = T;

	constexpr ECAllocator() noexcept = default;

	template <typename U>
	constexpr ECAllocator(const ECAllocator<U>&) noexcept {}

	constexpr bool operator == (const ECAllocator&) const noexcept { return true; }
	constexpr bool operator != (const ECAllocator&) const noexcept { return false; }

	T* allocate(const size_t count) const noexcept {
		return static_cast<T*>(ECMemory::Allocate(count * sizeof(T)));
	}

	void deallocate(T* const ptr, size_t count) const noexcept {
		ECMemory::Deallocate(ptr);
	}
};

struct ECDeleter {
	template <typename T>
	void operator ()(T* ptr) noexcept {
		if (ptr) {
			ECMemory::Deallocate(ptr);
		}
	}
};

template <typename T, typename... TArgs>
static inline T* ECCreateObj(TArgs&&... args) {
	static_assert(std::is_constructible<T, TArgs...>::value, "Cannot construct T from TArgs.");
	ECAllocator<T> alloc;
	return Memory::Create<T>(alloc, std::forward<TArgs>(args)...);
}

template<typename T>
static inline void ECDeleteObj(T* ptr) {
	ECAllocator<T> alloc;
	Memory::Delete(alloc, ptr);
}

template <typename T, typename... TArgs>
static inline T* ECCreateArray(size_t capacity, TArgs&&... args) {
	static_assert(std::is_constructible<T, TArgs...>::value, "Cannot construct T from TArgs.");
	ECAllocator<T> alloc;
	return Memory::CreateArray<T>(alloc, capacity, std::forward<TArgs>(args)...);
}

template<typename T>
static inline void ECDeleteArray(T* ptr, size_t capacity) {
	ECAllocator<T> alloc;
	Memory::DeleteArray(alloc, ptr, capacity);
}
