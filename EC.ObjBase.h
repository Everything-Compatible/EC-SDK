#pragma once
#include <cstdint>
#include <concepts>
#include <YRPPCore.h>
#include "IH.Loader.h"
#include <memory>
#include <CRC.h>

struct ECRTTIInfo;

void Internal_RegisterExportRTTI(const char* ClassName, const ECRTTIInfo* pInfo);
const ECRTTIInfo* Internal_GetRTTIInfo (const char* ClassName);
void Internal_RegisterClassFactory(const char* ClassName, void* DefaultConstruct, void* CopyConstruct, void* Destroy);
void Internal_GetClassFactory(const char* ClassName, LibFuncHandle& DefaultConstruct, LibFuncHandle& CopyConstruct, LibFuncHandle& Destroy);

struct ECRTTIInfo {
    // 基类信息节点
    struct ImplNode {
        const ECRTTIInfo* pBaseInfo; // 基类RTTI信息
        size_t Offset;            // 在派生类中的偏移量
    };

    size_t Count; // 基类数量

    // 通过VLA方式访问基类信息
    const ImplNode& GetBaseInfo(size_t Index) const;

#define FindTypeOffset_Failure 0xFFFFFFFF
    size_t FindTypeOffset(const ECRTTIInfo* TypeToFind)const;

    const char* GetName() const;

    std::vector<const char*> GetBaseClassName() const;

    std::vector<const ECRTTIInfo*> GetBaseClassInfo() const;

    const char* GetFirstBaseClassName() const;

    const ECRTTIInfo* GetFirstBaseClassInfo() const;
};



template<typename T>
concept CanUseECRTTI = requires
{
    { T::StaticGetRTTIInfo() } -> std::same_as<const ECRTTIInfo*>;
};


class ECRTTI
{
    void* DynamicCast_Impl(const void* Ptr, const ECRTTIInfo* TargetType)const
    {
        if (auto Offset = WhatAmI()->FindTypeOffset(TargetType); Offset != FindTypeOffset_Failure)
            return reinterpret_cast<void*>(reinterpret_cast<char*>(const_cast<void*>(Ptr)) + Offset);
        return nullptr;
    }
public:
    virtual const ECRTTIInfo* WhatAmI() const = 0;

    template<CanUseECRTTI T, CanUseECRTTI Derived>
    T* DynamicCast_Impl(Derived* Ptr)const
    {
        return reinterpret_cast<T*>(DynamicCast_Impl(Ptr, T::StaticGetRTTIInfo()));
    }
    template<CanUseECRTTI T, CanUseECRTTI Derived>
    const T* DynamicCast_Impl(const Derived* Ptr)const
    {
        return reinterpret_cast<const T*>(DynamicCast_Impl(const_cast<Derived*>(Ptr), T::StaticGetRTTIInfo()));
    }
    template<CanUseECRTTI T, CanUseECRTTI Derived>
    bool HasSameActualType(const Derived* p1, const T* p2)
    {
        return p1->WhatAmI() == p2->WhatAmI();
    }
};


// ================ 类型偏移计算 ================
template <typename Base, typename Derived>
constexpr size_t GetOffset() {
    // 通过指针偏移计算技术获取基类位置
    // 原理：空指针类型转换后的地址差即为偏移量
    return reinterpret_cast<size_t>(
        static_cast<Base*>(
            reinterpret_cast<Derived*>(0x1000) // 任意非零地址避免空指针问题
            )
        ) - 0x1000; // 减去基准地址得到纯偏移
}

template <typename... Args>
struct CountHelper {
    static constexpr size_t Value = sizeof...(Args);
};

template <CanUseECRTTI Base, CanUseECRTTI Derived>
struct DeriveClassData {
    ECRTTIInfo::ImplNode Node;

    // 构造函数：绑定RTTI信息和偏移量
    DeriveClassData()
        : Node{ Base::StaticGetRTTIInfo(), GetOffset<Base, Derived>() } // 假设Derived实现GetRTTIInfoImpl
    {}

    // 类型特征用于concept检查
    using BaseType = Base;
    using DerivedType = Derived;
};

template <typename T>
concept CountHelperType = requires {
    // 检查是否为DeriveClassData特化类型
        requires std::is_same_v<T, DeriveClassData<typename T::BaseType, typename T::DerivedType>>;
};

template <CountHelperType... Args> // 只接受合法派生类数据
struct ECRTTIInfoDef {
    // 基类数量（编译期计算）
    size_t Count{ CountHelper<Args...>::Value };
    const static constexpr size_t Static_Count{ CountHelper<Args...>::Value };

    // 存储所有基类数据节点
    std::tuple<Args...> BaseClasses;

    const ECRTTIInfo* GetImpl() const {
        return reinterpret_cast<const ECRTTIInfo*>(this);
    }
};

template <typename Derived, typename... Bases>
using ECRTTIInfoType = ECRTTIInfoDef<DeriveClassData<Bases, Derived>...>;

template <typename... Args>
struct ECRTTIExportHelper
{
    ECRTTIExportHelper(const char* ClassName)
    {
        if (!Internal_GetRTTIInfo(ClassName))
        {
            auto pNewInfo = new ECRTTIInfoType<Args...>{};
            Internal_RegisterExportRTTI(ClassName, pNewInfo->GetImpl());
        }
        else MessageBoxA(NULL, (std::string("错误：重名的RTTI类：") + ClassName).c_str(), "EC SDK", MB_OK);
    }
};


template<typename T>
class ECObjectDeleter ;
template<typename T>
using ECObjPtr = std::unique_ptr<T, ECObjectDeleter<T> >;

template<typename T, typename = void>
struct ECExportFactoryHelper
{
    using CopyConstructible = std::false_type;

    static T* CALLBACK Construct() { return GameCreate<T>(); }
    static void CALLBACK Destroy(const T* pObj) { GameDelete(pObj); }
    ECExportFactoryHelper(const char* ClassName) noexcept
    {
        Internal_RegisterClassFactory(ClassName, Construct, nullptr, Destroy);
    }
};

template <typename T>
struct ECExportFactoryHelper<T,std::enable_if_t<std::is_copy_constructible_v<T>>>
{
    using CopyConstructible = std::true_type;

    static T* CALLBACK Construct() { return GameCreate<T>(); }
    static T* CALLBACK CopyConstruct(const T& Obj) { return GameCreate<T>(Obj); }
    static void CALLBACK Destroy(const T* pObj) { GameDelete(pObj); }
    ECExportFactoryHelper(const char* ClassName) noexcept
    {
        Internal_RegisterClassFactory(ClassName, Construct, CopyConstruct, Destroy);
    }
};

void ECImportFactoryHelper_Error_Construct(const char* Name);
void ECImportFactoryHelper_Error_CopyConstruct(const char* Name);
void ECImportFactoryHelper_Error_Destroy(const char* Name);

template <typename T>
struct ECImportFactoryHelper
{
    LibFuncHandle DefCon, CopyCon, Dest;
    const char* _ClassName;
    T* CALLBACK Construct() 
    { if (!DefCon)ECImportFactoryHelper_Error_Construct(_ClassName); return AsType<T* CALLBACK()>(DefCon)(); }
    T* CALLBACK CopyConstruct(const T& Obj) 
    { if (!CopyCon)ECImportFactoryHelper_Error_CopyConstruct(_ClassName); return AsType<T* CALLBACK(const T&)>(CopyCon)(Obj); }
    void CALLBACK Destroy(const T* pObj) 
    { if (!Dest)ECImportFactoryHelper_Error_Destroy(_ClassName); return AsType<void CALLBACK(const T*)>(Dest)(pObj); }
    ECImportFactoryHelper(const char* ClassName) noexcept : _ClassName(ClassName)
    {
        Internal_GetClassFactory(ClassName, DefCon, CopyCon, Dest);
    }
};

template<typename T, typename Helper, typename = void>
struct ECClassFactoryImpl
{
    static ECObjPtr<T> Create() { return ECObjPtr<T>(Helper::Construct()); }
    static ECObjPtr<T> Create(const T& Obj) { return ECObjPtr<T>(Helper::Construct(Obj)); }
};

template<typename T, typename Helper>
struct ECClassFactoryImpl<T, Helper, std::enable_if_t<std::is_copy_constructible_v<T>>>
{
    static ECObjPtr<T> Create() { return ECObjPtr<T>(Helper::Construct()); }
};



template<typename T>
struct ECClassFactory_Impl2
{
    using Type = void;
};

template<typename T>
using ECClassFactory = typename ECClassFactory_Impl2<T>::Type;

class ECUniqueID
{
    int UniqueID{ 0 };
public:
    ECUniqueID() noexcept;
    ECUniqueID(const ECUniqueID&) noexcept;
    ECUniqueID(ECUniqueID&&) noexcept;
    int GetUniqueID() const{ return UniqueID; }
};

class ECStreamReader;
class ECStreamWriter;
class AbstractClass;
class ECGameClass;
class SIBuffClass;
class CCINIClass;


struct ECLoadSaveable
{
    virtual void LoadGame(ECStreamReader& Stream) = 0;
    virtual void SaveGame(ECStreamWriter& Stream) = 0;
    virtual size_t GetSizeMax() = 0;
    virtual void FinalSwizzle() = 0;
    virtual void PointerGotInvalid(AbstractClass* const pObject, bool bRemoved) = 0;
    virtual void PointerGotInvalid(ECGameClass* const pObject, bool bRemoved) = 0;
};


struct ECINILoadable
{
    //先于WW载入
    virtual void InitFromINI(CCINIClass* INI) = 0;
    //后于WW载入
    virtual void LoadFromINI(CCINIClass* INI) = 0;
};


struct ECStaticInit
{
    virtual void InitOnExeRun() = 0;
    virtual void InitAfterECInitialize() = 0;
    virtual void InitOnLoadScenario() = 0;

    ECStaticInit();
    ~ECStaticInit();
};





#define ECObjType class

#define EC_USERTTI_VirtualInherit virtual public ECRTTI
#define EC_USERTTI public ECRTTI
#define ECRTTI_DefineRTTIFunction(Class) \
private:\
mutable const ECRTTIInfo* MyType{ nullptr };\
public:\
static const ECRTTIInfo* StaticGetRTTIInfo() { return Internal_GetRTTIInfo(#Class); }\
virtual const ECRTTIInfo* WhatAmI() const { if(!MyType)MyType = StaticGetRTTIInfo(); return MyType; }\
template<typename T>\
T* DynamicCast(){return DynamicCast_Impl<T, Class>(this); }\
template<typename T>\
const T* DynamicCast()const{return DynamicCast_Impl<T, Class>(this); }\
template<CanUseECRTTI T>\
bool HasSameActualType(const T* p) { return this->WhatAmI() == p->WhatAmI(); }
//注意！！只有创建这个类的DLL当中才需要加上这个！在其他库调用的时候不要加这个！！
#define ECRTTI_ExportRTTI(Derived, ...) \
class Derived;\
namespace ECRTTIHelperNamespace{ ::ECRTTIExportHelper<Derived, ##__VA_ARGS__> Derived ## ExportHelperObject{ #Derived }; }

#define EC_USEUNIQUEID_VirtualInherit virtual public ECUniqueID
#define EC_USEUNIQUEID public ECUniqueID

//自动调用载入INI的函数
#define EC_LOADFROMRULES public ECINILoadable

//自动调用存读档等函数
#define EC_USELOADSAVE public ECLoadSaveable

//自动调用初始化函数
#define EC_STATICINIT public ECStaticInit

//前提：已经实现了Serialize函数
#define ECObj_DefineLoadSaveFunction \
virtual void LoadGame(ECStreamReader& Stream){ this->Serialize(Stream);}\
virtual void SaveGame(ECStreamWriter& Stream){ this->Serialize(Stream);}

#define ECObj_ExportFactory(Class) \
namespace ECObjHelperNamespace{ inline ECExportFactoryHelper<Class> Class ## ExportFactory( #Class );}\
template<> struct ECObjectDeleter<Class> \
{ void operator()(Class* ptr){ ECExportFactoryHelper<Class>::Destroy(ptr); } };\
template<>\
struct ECClassFactory_Impl2<Class>{using Type = ECClassFactoryImpl<Class, ECExportFactoryHelper<Class>>; };

#define ECObj_ImportFactory(Class)\
namespace ECObjHelperNamespace{ inline ECImportFactoryHelper<Class> Class ## ImportFactory( #Class );}\
template<> struct ECObjectDeleter<Class> \
{ void operator()(Class* ptr){ ECObjHelperNamespace::Class ## ImportFactory.Destroy(ptr); } };\
namespace ECObjHelperNamespace{ struct Class ## ClassFactoryHelper {\
static ECObjPtr<Class> Create() { return ECObjPtr<Class>(ECObjHelperNamespace::Class ## ImportFactory.Construct()); }\
static ECObjPtr<Class> Create(const Class& Obj) { return ECObjPtr<Class>(ECObjHelperNamespace::Class ## ImportFactory.CopyConstruct(Obj)); }\
};}\
template<>\
struct ECClassFactory_Impl2<Class>{ using Type = ECObjHelperNamespace::Class ## ClassFactoryHelper; };
    