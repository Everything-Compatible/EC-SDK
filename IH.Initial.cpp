#include "IH.Initial.h"
#include "SyringeEx.h"
#include "EC.Misc.h"

//以下的InitialLoadParam及其派生类都是全局创建一次永不销毁
//所以尽管瞎分配内存就行

InitialLoadParam::InitialLoadParam(size_t sz) 
	:size(sz) {};

InitialLoadParam_CustomPath::InitialLoadParam_CustomPath(const char* _Path)
	: Path(_Path), 
	InitialLoadParam(sizeof(InitialLoadParam_CustomPath)) {}
InitialLoadParam_CustomPath::InitialLoadParam_CustomPath(UseRelativePath_t, const char* _Path)
	: Path(_strdup((SyringeData::ExecutableDirectoryPath()+ _Path).c_str())), 
	InitialLoadParam(sizeof(InitialLoadParam_CustomPath)) {}

InitialLoadParam_CustomFile::InitialLoadParam_CustomFile(const char* _Name)
	: FileName(_Name), 
	InitialLoadParam(sizeof(InitialLoadParam_CustomFile)) {}

InitialLoadParam_StringTablePair::InitialLoadParam_StringTablePair(const char* _Key, const wchar_t* _Value, const char* _Extra)
	: Key(_Key), Value(_Value), Extra(_Extra),
	InitialLoadParam(sizeof(InitialLoadParam_StringTablePair)) {}

InitialLoadParam_RedirectFile::InitialLoadParam_RedirectFile(const char* _OldName, const char* _NewName)
	: OriginalName(_OldName), TargetName(_NewName),
	InitialLoadParam(sizeof(InitialLoadParam_RedirectFile)) {}

InitialLoadParam_RegisterVTable::InitialLoadParam_RegisterVTable(const char* _Name , vptr_t _vptr, size_t size)
	: Name(_Name), vptr(_vptr), Size(size),
	InitialLoadParam(sizeof(InitialLoadParam_RegisterVTable)) {}

InitialLoadParam_RegisterFunction::InitialLoadParam_RegisterFunction(const char* _Name , void* _Hd)
	: Name(_Name), Handle(_Hd),
	InitialLoadParam(sizeof(InitialLoadParam_RegisterFunction)) {}

InitialLoadParam_RegisterTag::InitialLoadParam_RegisterTag(const char* _Name, const char* _TagType, const char* _TagVar)
	: Name(_Name), TagType(_TagType), TagVar(_TagVar),
	InitialLoadParam(sizeof(InitialLoadParam_RegisterTag)) {}

void Internal_SetGlobalVarPtr(const char* Usage, const char* Key, LPCVOID Ptr);

InitialLoadParam_RegisterTag::InitialLoadParam_RegisterTag(const char* _Name, const char* _TagType, void* TagValuePtr)
	: Name(_Name), TagType(_TagType),
	InitialLoadParam(sizeof(InitialLoadParam_RegisterTag)) 
{
	auto _TagVar = RandStr(16);
	TagVar = _strdup(_TagVar.c_str());
	Internal_SetGlobalVarPtr(_TagType, _TagVar.c_str(), TagValuePtr);
}
InitialLoadParam_RegisterTag::InitialLoadParam_RegisterTag(const char* _Name, const char* _TagType, int TagValue)
	: InitialLoadParam_RegisterTag(_Name, _TagType, reinterpret_cast<void*>(TagValue)) {}

InitialLoadParam_RegisterTag::InitialLoadParam_RegisterTag(const char* _Name, const char* _TagType, bool TagValue)
	: InitialLoadParam_RegisterTag(_Name, _TagType, TagValue ? 1 : 0) {
}

InitialLoadParam_RegisterTag::InitialLoadParam_RegisterTag(const char* _Name, const char* _TagType, float TagValue)
	: InitialLoadParam_RegisterTag(_Name, _TagType, *reinterpret_cast<int*>(&TagValue)) {}