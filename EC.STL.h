#pragma once
#include "EC.Allocator.h"
#include <vector>
#include <list>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <string>
#include <memory>
#include <stdexcept>
#include <string_view>

#ifndef LIBFUNC_HANDLE
using LibFuncHandle = void*;
#endif // !LIBFUNC_HANDLE
using LibObjectHandle = LibFuncHandle;

namespace ECLocal
{
	//STL containers with ECAllocator
	template<typename T>
	using vector = std::vector<T, ECAllocator<T>>;
	template<typename T>
	using list = std::list<T, ECAllocator<T>>;
	template<typename T>
	using deque = std::deque<T, ECAllocator<T>>;
	template<typename K, typename V>
	using unordered_map = std::unordered_map<K, V, std::hash<K>, std::equal_to<K>, ECAllocator<std::pair<const K, V>>>;
	template<typename K, typename V>
	using map = std::map<K, V, std::less<K>, ECAllocator<std::pair<const K, V>>>;
	template<typename K>
	using unordered_set = std::unordered_set<K, std::hash<K>, std::equal_to<K>, ECAllocator<K>>;
	template<typename K>
	using set = std::set<K, std::less<K>, ECAllocator<K>>;
	template<typename K, typename V>
	using multimap = std::multimap<K, V, std::less<K>, ECAllocator<std::pair<const K, V>>>;
	template<typename K>
	using multiset = std::multiset<K, std::less<K>, ECAllocator<K>>;
	template<typename K>
	using unordered_multiset = std::unordered_multiset<K, std::hash<K>, std::equal_to<K>, ECAllocator<K>>;
	template<typename K, typename V>
	using unordered_multimap = std::unordered_multimap<K, V, std::hash<K>, std::equal_to<K>, ECAllocator<std::pair<const K, V>>>;
	template<typename T>
	using basic_string = std::basic_string<T, std::char_traits<T>, ECAllocator<T>>;

	template<typename T>
	struct ResPtr
	{
		//a unique ptr all same
		T* ptr;
		ResPtr() : ptr(nullptr) {}
		ResPtr(T* p) : ptr(p) {}
		ResPtr(const ResPtr&) = delete;
		ResPtr(ResPtr&& other) : ptr(other.ptr) { other.ptr = nullptr; }
		ResPtr& operator=(const ResPtr&) = delete;
		ResPtr& operator=(ResPtr&& other)
		{
			if (this != &other)
			{
				if (ptr) { ECMemory::Deallocate(ptr); }
				ptr = other.ptr;
				other.ptr = nullptr;
			}
			return *this;
		}
		ResPtr& operator=(nullptr_t) noexcept {
			if (ptr) { ECMemory::Deallocate(ptr); }
			ptr = nullptr;
			return *this;
		}
		ResPtr& operator=(T* p) noexcept {
			if (ptr) { ECMemory::Deallocate(ptr); }
			ptr = p;
			return *this;
		}
		~ResPtr() { if (ptr) { ECMemory::Deallocate(ptr); } }
		operator T* () { return ptr; }
		operator const T* () const { return ptr; }
		template<typename U>
		operator U* () { return static_cast<U*>(ptr); }
		operator bool() const { return ptr != nullptr; }
		bool operator!() const { return ptr == nullptr; }
		void swap(ResPtr& other) noexcept
		{
			std::swap(ptr, other.ptr);
		}
		T* operator->() { return ptr; }
		const T* operator->() const { return ptr; }
		T& operator*() { return *ptr; }
		const T& operator*() const { return *ptr; }
		T* Get() { return ptr; }
		const T* Get() const { return ptr; }
		void Reset() { if (ptr) { ECMemory::Deallocate(ptr); } ptr = nullptr; }
		void Reset(T* p) { if (ptr) { ECMemory::Deallocate(ptr); } ptr = p; }
		void Release() { if (ptr) { ECMemory::Deallocate(ptr); } ptr = nullptr; }
		void Release(T* p) { if (ptr) { ECMemory::Deallocate(ptr); } ptr = p; }
		void Release(ResPtr& other) { if (ptr) { ECMemory::Deallocate(ptr); } ptr = other.ptr; other.ptr = nullptr; }
		void Release(ResPtr&& other) { if (ptr) { ECMemory::Deallocate(ptr); } ptr = other.ptr; other.ptr = nullptr; }
	};
	struct ECContainerBase;
	struct ECIteratorBase;
	using ECContainerPtr = ResPtr<ECContainerBase>;
	using ECIteratorPtr = ResPtr<ECIteratorBase>;
	struct ECContainerBase
	{
		virtual ~ECContainerBase() = default;
		virtual ECContainerBase* Duplicate() const = 0;
		virtual ECIteratorPtr Begin() const = 0;
		virtual ECIteratorPtr End() const = 0;
	};
	struct ECIteratorBase
	{
		virtual ~ECIteratorBase() = default;
		virtual ECIteratorPtr Duplicate() const = 0;
		virtual void Increment() = 0;
		virtual bool Equal(const ECIteratorBase& other) const = 0;
		virtual const void* Get() const = 0;
	};
	template<typename T, typename Factory>
		requires std::is_base_of<ECContainerBase, T>::value
	class ECContainerWrapper;

	template<typename T, typename Cont>
	struct ECLinearIterator : public ECIteratorBase
	{
		size_t Index;
		Cont* Vec;

		//override ECIteratorBase
		virtual ~ECLinearIterator() = default;
		virtual ECIteratorPtr Duplicate() const override
		{
			auto p = ECMemory::Allocate(sizeof(ECLinearIterator));
			auto* newIt = new (p) ECLinearIterator(*this);
			return ECIteratorPtr(newIt);
		}
		virtual void Increment() override { ++Index; }
		virtual bool Equal(const ECIteratorBase& other) const override
		{
			auto* otherIt = static_cast<const ECLinearIterator*>(&other);
			return Index == otherIt->Index;
		}
		virtual const void* Get() const override
		{
			return reinterpret_cast<void*>(&Vec->At(Index));
		}

		ECLinearIterator(Cont* vec, size_t index) : Vec(vec), Index(index) {}
		static ECIteratorPtr Create(Cont* vec, size_t index)
		{
			auto p = ECMemory::Allocate(sizeof(ECLinearIterator));
			auto* newIt = new (p) ECLinearIterator(vec, index);
			return ECIteratorPtr(newIt);
		}
		static ECIteratorPtr Create(const Cont* vec, size_t index)
		{
			return Create(const_cast<Cont*>(vec), index);
		}
	};

	template<typename T>
	struct ECIString : public ECContainerBase
	{
		virtual ~ECIString() = default;
		virtual T& At(size_t index) = 0;
		virtual const T& At(size_t index) const = 0;
		virtual T& Back() = 0;
		virtual T& Front() = 0;
		virtual size_t Size() const = 0;
		virtual size_t Capacity() const = 0;
		virtual bool Empty() const = 0;
		virtual void Reserve(size_t size) = 0;
		virtual void Resize(size_t size) = 0;
		virtual void Clear() = 0;
		virtual void PushBack(const T& value) = 0;
		virtual void PushBack(T&& value) = 0;
		virtual void PopBack() = 0;
		virtual void Erase(size_t index) = 0;
		virtual void Insert(size_t index, const T& value) = 0;
		virtual void Insert(size_t index, T&& value) = 0;
		virtual void Append(const ECIString<T>& value) = 0;
		virtual void Append(ECIString<T>&& value) = 0;
		virtual const T* CStr() const = 0;
		virtual T* Data() = 0;

		using IndexType = T;
		using DataType = T;
		using AddResult = T;
		T& operator[](size_t index) { return At(index); }
		const T& operator[](size_t index) const { return At(index); }
		ECIString<T>& operator+=(const ECIString<T>& other) { Append(other); return *this; }
		operator std::basic_string_view<T>() const { return std::basic_string_view<T>(CStr(), Size()); }
		std::basic_string_view<T> ToView() const { return std::basic_string_view<T>(CStr(), Size()); }
	};
	template<typename T>
	using ECStringIterator = ECLinearIterator<T, ECIString<T>>;
	template<typename T, typename Alloc>
	struct StdBasicString : public ECIString<T>
	{
		std::basic_string<T, std::char_traits<T>, Alloc> Str;
		StdBasicString() = default;
		StdBasicString(const std::basic_string<T, std::char_traits<T>, Alloc>& str) : Str(str) {}
		StdBasicString(std::basic_string<T, std::char_traits<T>, Alloc>&& str) : Str(std::move(str)) {}


		//override ECIString
		virtual T& At(size_t index) override { return Str.at(index); }
		virtual const T& At(size_t index) const override { return Str.at(index); }
		virtual T& Back() override { return Str.back(); }
		virtual T& Front() override { return Str.front(); }
		virtual size_t Size() const override { return Str.size(); }
		virtual size_t Capacity() const override { return Str.capacity(); }
		virtual bool Empty() const override { return Str.empty(); }
		virtual void Reserve(size_t size) override { Str.reserve(size); }
		virtual void Resize(size_t size) override { Str.resize(size); }
		virtual void Clear() override { Str.clear(); }
		virtual void PushBack(const T& value) override { Str.push_back(value); }
		virtual void PushBack(T&& value) override { Str.push_back(std::move(value)); }
		virtual void PopBack() override { Str.pop_back(); }
		virtual void Erase(size_t index) override { Str.erase(Str.begin() + index); }
		virtual void Insert(size_t index, const T& value) override { Str.insert(Str.begin() + index, value); }
		virtual void Insert(size_t index, T&& value) override { Str.insert(Str.begin() + index, std::move(value)); }
		virtual void Append(const ECIString<T>& value) override { Str.append(value.CStr(), value.Size()); }
		virtual void Append(ECIString<T>&& value) override { Str.append(value.CStr(), value.Size()); }
		virtual const T* CStr() const override { return Str.c_str(); }
		virtual T* Data() override { return Str.data(); }
		//override ECContainerBase
		virtual ECContainerBase* Duplicate() const override
		{
			auto p = ECMemory::Allocate(sizeof(StdBasicString<T, Alloc>));
			auto* newStr = new (p) StdBasicString<T, Alloc>(Str);
			return newStr;
		}
		virtual ~StdBasicString() override = default;
		virtual ECIteratorPtr Begin() const override
		{
			return ECStringIterator<T>::Create(this, 0);
		}
		virtual ECIteratorPtr End() const override
		{
			return ECStringIterator<T>::Create(this, Str.size());
		}
	};
	template<typename T>
	using StdString1 = StdBasicString<T, ECAllocator<T>>;
	template<typename T>
	using StdString2 = StdBasicString<T, std::allocator<T>>;
	template<typename T>
	struct ECStringFactory
	{
		using PtrType = ResPtr<ECIString<T>>;

		static PtrType Create();
		static PtrType Create(basic_string<T>&& str);
		static PtrType Create(const basic_string<T>& str);
		static PtrType Create(const std::basic_string<T>& str);
		static PtrType Create(std::basic_string<T>&& str);
		static PtrType Create(const std::basic_string_view<T>& str);
		static PtrType Create(const T* Str);
		template<typename Factory>
		static PtrType Create(const ECContainerWrapper<ECIString<T>, Factory>& Container);

		static PtrType Duplicate(const PtrType& C);
	};

	template<typename T>
	struct ECIVector: public ECContainerBase
	{
		virtual ~ECIVector() = default;
		virtual T& At(size_t index) = 0;
		virtual const T& At(size_t index) const = 0;
		virtual T& Back() = 0;
		virtual T& Front() = 0;
		virtual size_t Size() const = 0;
		virtual size_t Capacity() const = 0;
		virtual bool Empty() const = 0;
		virtual void Reserve(size_t size) = 0;
		virtual void Resize(size_t size) = 0;
		virtual void Clear() = 0;
		virtual void PushBack(const T& value) = 0;
		virtual void PushBack(T&& value) = 0;
		virtual void PopBack() = 0;
		virtual void Erase(size_t index) = 0;
		virtual void Insert(size_t index, const T& value) = 0;
		virtual void Insert(size_t index, T&& value) = 0;
		virtual T* Data() = 0;
		virtual const T* Data() const = 0;

		using IndexType = T;
		using DataType = T;
		T& operator[](size_t index) { return At(index); }
		const T& operator[](size_t index) const { return At(index); }
	};
	template<typename T>
	using ECVectorIterator = ECLinearIterator<T, ECIVector<T>>;
	template<typename T>
	struct StdVector : public ECIVector<T>
	{
		vector<T> Vec;
		StdVector() = default;
		StdVector(const vector<T>& vec) : Vec(vec) {}
		StdVector(vector<T>&& vec) : Vec(std::move(vec)) {}
		StdVector(const std::vector<T>& vec) : Vec(vec) {}
		StdVector(std::vector<T>&& vec) : Vec(std::move(vec)) {}
		
		//override ECIVector
		virtual T& At(size_t index) override { return Vec.at(index); }
		virtual const T& At(size_t index) const override { return Vec.at(index); }
		virtual T& Back() override { return Vec.back(); }
		virtual T& Front() override { return Vec.front(); }
		virtual size_t Size() const override { return Vec.size(); }
		virtual size_t Capacity() const override { return Vec.capacity(); }
		virtual bool Empty() const override { return Vec.empty(); }
		virtual void Reserve(size_t size) override { Vec.reserve(size); }
		virtual void Resize(size_t size) override { Vec.resize(size); }
		virtual void Clear() override { Vec.clear(); }
		virtual void PushBack(const T& value) override { Vec.push_back(value); }
		virtual void PushBack(T&& value) override { Vec.push_back(std::move(value)); }
		virtual void PopBack() override { Vec.pop_back(); }
		virtual void Erase(size_t index) override { Vec.erase(Vec.begin() + index); }
		virtual void Insert(size_t index, const T& value) override { Vec.insert(Vec.begin() + index, value); }
		virtual void Insert(size_t index, T&& value) override { Vec.insert(Vec.begin() + index, std::move(value)); }
		virtual T* Data() override { return Vec.data(); }
		virtual const T* Data() const override { return Vec.data(); }
		//override ECContainerBase
		virtual ECContainerBase* Duplicate() const override
		{
			auto p = ECMemory::Allocate(sizeof(StdVector<T>));
			auto* newVec = new (p) StdVector<T>(Vec);
			return newVec;
		}
		virtual ~StdVector() override = default;
		virtual ECIteratorPtr Begin() const override
		{
			return ECVectorIterator<T>::Create(this, 0);
		}
		virtual ECIteratorPtr End() const override
		{
			return ECVectorIterator<T>::Create(this, Vec.size());
		}

	};
	template<typename T>
	struct DynamicVector :public ECIVector<T>
	{
		DynamicVectorClass<T> Vec;
		DynamicVector() = default;
		DynamicVector(const DynamicVectorClass<T>& vec) : Vec(vec) {}

		//override ECIVector
		virtual T& At(size_t index) override { return Vec[index]; }
		virtual const T& At(size_t index) const override { return Vec[index]; }
		virtual T& Back() override { return *Vec.back(); }
		virtual T& Front() override { return *Vec.front(); }
		virtual size_t Size() const override { return Vec.Count; }
		virtual size_t Capacity() const override { return Vec.Capacity; }
		virtual bool Empty() const override { return !!Vec.Count; }
		virtual void Reserve(size_t size) override { Vec.Reserve(size); }
		virtual void Resize(size_t size) override 
		{  
			if ((int)size > Vec.Count)
				for (size_t i = Vec.Count; i < size; ++i)
					Vec.AddItem(T());
			else if ((int)size < Vec.Count)
				for (size_t i = Vec.Count; i > size; --i)
					Vec.RemoveItem(i - 1);
		}
		virtual void Clear() override { Vec.Clear(); }
		virtual void PushBack(const T& value) override { Vec.AddItem(value); }
		virtual void PushBack(T&& value) override { Vec.AddItem(std::move(value)); }
		virtual void PopBack() override { Vec.RemoveItem(Vec.Count - 1); }
		virtual void Erase(size_t index) override { Vec.RemoveItem(index); }
		virtual void Insert(size_t index, const T& value) override
		{
			if ((int)index > Vec.Count) throw std::out_of_range("Index out of range");
			Vec.AddItem(Vec.Items[Vec.Count - 1]);
			for (size_t i = Vec.Count - 2; i > index; --i)
			{
				Vec.Items[i] = Vec.Items[i - 1];
			}
			Vec.Items[index] = value;
		}
		virtual void Insert(size_t index, T&& value) override { Insert(index, value); }
		virtual T* Data() override { return Vec.Items; }
		virtual const T* Data() const override { return Vec.Items; }
		//override ECContainerBase
		virtual ECContainerBase* Duplicate() const override
		{
			auto p = ECMemory::Allocate(sizeof(DynamicVector<T>));
			auto* newVec = new (p) DynamicVector<T>(Vec);
			return newVec;
		}
		virtual ~DynamicVector() override = default;
		virtual ECIteratorPtr Begin() const override
		{
			return ECVectorIterator<T>::Create(this, 0);
		}
		virtual ECIteratorPtr End() const override
		{
			return ECVectorIterator<T>::Create(this, Vec.Count);
		}
	};
	template<typename T>
	struct ECVectorFactory
	{
		using PtrType = ResPtr<ECIVector<T>>;
		static PtrType Create();
		static PtrType Create(const vector<T>& vec);
		static PtrType Create(vector<T>&& vec);
		static PtrType Create(const DynamicVectorClass<T>& vec);
		static PtrType Create(DynamicVectorClass<T>&& vec);
		template<typename Factory>
		static PtrType Create(const ECContainerWrapper< ECIVector<T>, Factory>& Container);

		static PtrType Duplicate(const PtrType& C);
	};

	template<typename MapType>
	struct ECIMap
	{

	};
	
	template<typename MapType>
	struct ECMapFactory
	{

	};

	template<typename T>
	struct ECIterableWrapper
	{
		ECIteratorPtr Iterator;
		ECIterableWrapper(ECIteratorPtr&& iterator) : Iterator(std::move(iterator)) {}
		ECIterableWrapper(const ECIterableWrapper& other) : Iterator(other.Iterator ? std::move(other.Iterator->Duplicate()) : ECIteratorPtr()) {}

		//operators
		ECIterableWrapper& operator=(const ECIterableWrapper& other)
		{
			if (this != &other)
			{
				Iterator = other.Iterator ? std::move(other.Iterator->Duplicate()) : ECIteratorPtr();
			}
			return *this;
		}
		ECIterableWrapper& operator=(ECIteratorPtr&& other)
		{
			if (this != &other)
			{
				Iterator = std::move(other);
			}
			return *this;
		}
		ECIterableWrapper& operator++()
		{
			if (Iterator)
			{
				Iterator->Increment();
			}
			return *this;
		}
		ECIterableWrapper operator++(int)
		{
			ECIterableWrapper tmp{ *this };
			if (Iterator)
			{
				Iterator->Increment();
			}
			return tmp;
		}
		bool operator==(const ECIterableWrapper& other) const
		{
			if (Iterator && other.Iterator)
			{
				return Iterator->Equal(*other.Iterator);
			}
			return false;
		}
		bool operator!=(const ECIterableWrapper& other) const
		{
			if (Iterator && other.Iterator)
			{
				return !Iterator->Equal(*other.Iterator);
			}
			return true;
		}

		T& operator*() const
		{
			if (Iterator)
			{
				return *((T*)Iterator->Get());
			}
			else throw std::runtime_error("Iterator is null");
		}
		T* operator->() const
		{
			if (Iterator)
			{
				return reinterpret_cast<T*>(Iterator->Get());
			}
		}
	};
	template<typename T>
	using ECIterator = ECIterableWrapper<T>;
	template<typename T>
	using ECConstIterator = ECIterableWrapper<const T>;
	template<typename T, typename Factory>
	concept IsECFactoryOf = requires(Factory factory)
	{
		{ factory.Create() } -> std::same_as<ResPtr<T>>;
		{ factory.Duplicate(std::declval<ResPtr<T>>()) } -> std::same_as<ResPtr<T>>;
	};
	template <typename T> concept HasIndexMethod = requires (T t) { typename T::IndexType; };
	template <typename T> concept HasIter = requires (T t) { typename T::DataType; };
	template <typename T> concept HasAdd = requires (T t) { typename T::AddResult; };
	template<typename T, typename Factory>
		requires std::is_base_of<ECContainerBase, T>::value //&& IsECFactoryOf<T, Factory>
	class ECContainerWrapper
	{
	public:
		using ContainerType = T;
		using FactoryType = Factory;

		ResPtr<T> Container;

		template<typename... Args>
		ECContainerWrapper(Args&&... args) noexcept;
		ECContainerWrapper() noexcept;
		ECContainerWrapper(ECContainerWrapper&& A) noexcept;
		ECContainerWrapper(const ECContainerWrapper& A) noexcept;
		ECContainerWrapper(ECContainerPtr&& container) noexcept;
		

		//operators
		template<std::integral U> 
		auto& operator[](U index) requires HasIndexMethod<T> { return (*Container)[static_cast<size_t>(index)]; }
		template<std::integral U>
		auto& operator[](U index) const requires HasIndexMethod<T>{ return (*Container)[static_cast<size_t>(index)]; }
		auto& operator+=(const ECContainerWrapper& A) requires HasAdd<T>
		{
			if (Container && A.Container)Container->Append(*A.Container); 
			return *this;
		}
		auto& operator+=(ECContainerWrapper&& A) requires HasAdd<T>
		{
			if (Container && A.Container)Container->Append(*A);
			return *this;
		}
		auto& operator+=(const T& A) requires HasAdd<T>
		{
			if (Container)Container->Append(A);
			return *this;
		}
		auto& operator+=(T&& A) requires HasAdd<T>
		{
			if (Container)Container->Append(std::move(A));
			return *this;
		}
		auto operator+(const ECContainerWrapper& A) requires HasAdd<T>
		{
			auto result = *this;
			result += A;
			return result;
		}
		auto operator+(ECContainerWrapper&& A) requires HasAdd<T>
		{
			auto result = *this;
			result += std::move(A);
			return result;
		}
		auto operator+(const T& A) requires HasAdd<T>
		{
			auto result = *this;
			result += A;
			return result;
		}
		auto operator+(T&& A) requires HasAdd<T>
		{
			auto result = *this;
			result += std::move(A);
			return result;
		}

		T* operator->() { return static_cast<T*>(Container); }
		const T* operator->() const { return static_cast<const T*>(Container); }
		T& operator*() { return *static_cast<T*>(Container); }
		const T& operator*() const { return *static_cast<const T*>(Container); }
		operator T* () { return static_cast<T*>(Container); }
		operator const T* () const { return static_cast<const T*>(Container); }
		operator bool() const { return Container != nullptr; }
		bool operator!() const { return Container == nullptr; }
		ECContainerWrapper<T, Factory>& operator=(const ECContainerWrapper<T, Factory>& other)
		{
			if (this != &other)Container = Factory::Duplicate(other.Container);
			return *this;
		}
		ECContainerWrapper<T, Factory>& operator=(ECContainerWrapper<T, Factory>&& other)
		{
			if (this != &other)Container = std::move(other.Container);
			return *this;
		}
		ECContainerWrapper<T, Factory>& operator=(ECContainerPtr&& other)
		{
			if (this != &other)Container = std::move(other);
			return *this;
		}
		template<typename... Args>
		ECContainerWrapper<T, Factory>& operator=(Args&&... args) { Container = std::move(Factory::Create(std::forward<Args>(args)...)); }

		//member functions
		void swap(ECContainerWrapper& other) noexcept { Container.swap(other.Container); }
		auto begin() requires HasIter<T> { return ECIterator<typename T::DataType>(Container->Begin()); }
		auto end() requires HasIter<T> { return ECIterator<typename T::DataType>(Container->End()); }
		auto begin() const requires HasIter<T> { return ECConstIterator<typename T::DataType>(Container->Begin()); }
		auto end() const requires HasIter<T> { return ECConstIterator<typename T::DataType>(Container->End()); }
		auto cbegin() const requires HasIter<T> { return ECConstIterator<typename T::DataType>(Container->Begin()); }
		auto cend() const requires HasIter<T> { return ECConstIterator<typename T::DataType>(Container->End()); }
	};
}
//and unique_ptr shared_ptr weak_ptr

template<typename T>
using ECVector = ECLocal::ECContainerWrapper<ECLocal::ECIVector<T>, ECLocal::ECVectorFactory<T>>;

template<typename T>
using ECBasicString = ECLocal::ECContainerWrapper<ECLocal::ECIString<T>, ECLocal::ECStringFactory<T>>;
using ECString = ECBasicString<char>;
using ECWString = ECBasicString<wchar_t>;

template<typename K, typename V>
using ECMap = ECLocal::ECContainerWrapper
	<ECLocal::ECIMap<ECLocal::map<K, V>>, ECLocal::ECMapFactory<ECLocal::map<K, V>>>;
template<typename K, typename V>
using ECUnorderedMap = ECLocal::ECContainerWrapper
	<ECLocal::ECIMap<ECLocal::unordered_map<K, V>>, ECLocal::ECMapFactory<ECLocal::unordered_map<K, V>>>;

namespace ECLocal
{
	template<typename T>
	ECVectorFactory<T>::PtrType ECVectorFactory<T>::Create()
	{
		((void)0);
		size_t sz = sizeof(StdVector<T>);
		((void)0);
		auto p = ECMemory::Allocate(sz);
		((void)0);
		auto* newVec = new (p) StdVector<T>();
		return PtrType(newVec);
	}
	template<typename T>
	ECVectorFactory<T>::PtrType ECVectorFactory<T>::Create(const vector<T>& vec)
	{
		auto p = ECMemory::Allocate(sizeof(StdVector<T>));
		auto* newVec = new (p) StdVector<T>(vec);
		return PtrType(newVec);
	}
	template<typename T>
	ECVectorFactory<T>::PtrType ECVectorFactory<T>::Create(vector<T>&& vec)
	{
		auto p = ECMemory::Allocate(sizeof(StdVector<T>));
		auto* newVec = new (p) StdVector<T>(std::move(vec));
		return PtrType(newVec);
	}
	template<typename T>
	ECVectorFactory<T>::PtrType ECVectorFactory<T>::Create(const DynamicVectorClass<T>& vec)
	{
		auto p = ECMemory::Allocate(sizeof(DynamicVector<T>));
		auto* newVec = new (p) DynamicVector<T>(vec);
		return PtrType(newVec);
	}
	template<typename T>
	ECVectorFactory<T>::PtrType ECVectorFactory<T>::Create(DynamicVectorClass<T>&& vec)
	{
		auto p = ECMemory::Allocate(sizeof(DynamicVector<T>));
		auto* newVec = new (p) DynamicVector<T>(std::move(vec));
		return PtrType(newVec);
	}
	template<typename T>template<typename Factory>
	ECVectorFactory<T>::PtrType ECVectorFactory<T>::Create(const ECContainerWrapper< ECIVector<T>, Factory>& Container)
	{
		return Duplicate(Container.Container);
	}

	template<typename T>
	ECVectorFactory<T>::PtrType ECVectorFactory<T>::Duplicate(const PtrType& C)
	{
		if (C)
		{
			return PtrType(dynamic_cast<ECIVector<T>*>(C->Duplicate()));
		}
		return nullptr;
	}

	template<typename T>
	ECStringFactory<T>::PtrType ECStringFactory<T>::Create()
	{
		auto p = ECMemory::Allocate(sizeof(StdString1<T>));
		auto* newVec = new (p) StdString1<T>();
		return PtrType(newVec);
	}
	template<typename T>
	ECStringFactory<T>::PtrType ECStringFactory<T>::Create(basic_string<T>&& str)
	{
		auto p = ECMemory::Allocate(sizeof(StdString1<T>));
		auto* newVec = new (p) StdString1<T>(std::move(str));
		return PtrType(newVec);
	}
	template<typename T>
	ECStringFactory<T>::PtrType ECStringFactory<T>::Create(const basic_string<T>& str)
	{
		auto p = ECMemory::Allocate(sizeof(StdString1<T>));
		auto* newVec = new (p) StdString1<T>(str);
		return PtrType(newVec);
	}
	template<typename T>
	ECStringFactory<T>::PtrType ECStringFactory<T>::Create(const std::basic_string<T>& str)
	{
		auto p = ECMemory::Allocate(sizeof(StdString2<T>));
		auto* newVec = new (p) StdString2<T>(str);
		return PtrType(newVec);
	}
	template<typename T>
	ECStringFactory<T>::PtrType ECStringFactory<T>::Create(std::basic_string<T>&& str)
	{
		auto p = ECMemory::Allocate(sizeof(StdString2<T>));
		auto* newVec = new (p) StdString2<T>(std::move(str));
		return PtrType(newVec);
	}
	template<typename T>
	ECStringFactory<T>::PtrType ECStringFactory<T>::Create(const std::basic_string_view<T>& str)
	{
		auto p = ECMemory::Allocate(sizeof(StdString1<T>));
		auto* newVec = new (p) StdString1<T>(str);
		return PtrType(newVec);
	}
	template<typename T>
	ECStringFactory<T>::PtrType ECStringFactory<T>::Create(const T* Str)
	{
		return Create(basic_string<T>(Str));
	}
	template<typename T>template<typename Factory>
	ECStringFactory<T>::PtrType ECStringFactory<T>::Create(const ECContainerWrapper<ECIString<T>, Factory>& c)
	{
		return Duplicate(c.Container);
	}

	template<typename T>
	ECStringFactory<T>::PtrType ECStringFactory<T>::Duplicate(const PtrType& C)
	{
		if (C)
		{
			return PtrType(dynamic_cast<ECIString<T>*>(C->Duplicate()));
		}
		return nullptr;
	}

	template<typename T, typename Factory>
		requires std::is_base_of<ECContainerBase, T>::value
	template<typename... Args>
	ECContainerWrapper<T, Factory>::ECContainerWrapper(Args&&... args) noexcept : Container(Factory::Create(std::forward<Args>(args)...)) {}
	template<typename T, typename Factory>
		requires std::is_base_of<ECContainerBase, T>::value
	ECContainerWrapper<T, Factory>::ECContainerWrapper() noexcept : Container(std::move(Factory::Create())) {}
	template<typename T, typename Factory>
		requires std::is_base_of<ECContainerBase, T>::value
	ECContainerWrapper<T, Factory>::ECContainerWrapper(ECContainerWrapper&& A) noexcept : Container(std::move(A.Container)) {}
	template<typename T, typename Factory>
		requires std::is_base_of<ECContainerBase, T>::value
	ECContainerWrapper<T, Factory>::ECContainerWrapper(const ECContainerWrapper& A) noexcept : Container(std::move(Factory::Duplicate(A.Container))) {}
	template<typename T, typename Factory>
		requires std::is_base_of<ECContainerBase, T>::value
	ECContainerWrapper<T, Factory>::ECContainerWrapper(ECContainerPtr&& container) noexcept : Container(std::move(container)) {}
}