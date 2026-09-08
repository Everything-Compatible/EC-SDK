#include "IH.Loader.h"
#include "IH.Config.h"
#include "IH.Initial.h"
#include "IH.InitialService.h"
#include "EC.Misc.h"
#include "SyringeEx.h"
#include <Windows.h>
#include <string_view>
#include <cwchar>




#ifndef IHCore
void MyInit(InitResult& Result);

#include <iostream>
#include <cstdio>
#endif

namespace SyringeData
{
	void InitRemoteData();
}

namespace Init
{
	bool Initialize();
	bool __cdecl RegisterEntry(LibFuncHandle Entry);
}

LibFuncHandle __cdecl ListenerAccess(const char* Name);
void RegisterLoadSaveInterface();
struct ECRTTIInfo;

namespace Init
{
	bool InitAvailable{ false };
	bool VeryFirst{ true };
	HMODULE LibListDLL;
	InitInput* LibInput;
	InitResult Result;
	bool Internal_EnableSaveLoad{ true };

	bool(__cdecl* _RegisterEntry)(LibFuncHandle);
	const LibFuncHandle* (__cdecl* _GetEntries)(void);
	void(__cdecl* _ServiceRequest)(IHInitialLoadService);
	void(__cdecl* _QueryServiceRequest)(const char*, PArray<IHInitialLoadService>&);
	ConfData* (__cdecl* _GetConfData)(void);
	unsigned int(__cdecl* _RandomUINT)(void);
	int (__cdecl* _GenerateID)(void);
	LPCVOID(__cdecl* _GetNamedPointer)(const char* Usage, const char* Name);
	void  (__cdecl* _SetNamedPointer)(const char* Usage, const char* Name, LPCVOID Pointer);
	const char* (__cdecl* _NamedPointer_GetName)(const char* Usage, LPCVOID Pointer);

	bool IsSyringeReadingHooks()
	{
		char Buf[1000];
		return GetEnvironmentVariableA("HERE_IS_SYRINGE", Buf, 1000) ? true : false;
	}

	bool LoaderLoaded{ false };
	struct IHLibListLoader
	{
		// 定位 IHLibList.dll。
		// 依次尝试：进程内已加载 → 注射器通过环境变量给出的扫描目录（ExtensionPacks）
		//          → 与当前模块同目录 → 游戏根目录\Patches → 游戏根目录。
		static HMODULE FindIHLibList()
		{
			if (HMODULE h = GetModuleHandleW(L"IHLibList.dll"))
				return h;

			// 认注射器配置：Syringe 把实际扫描过的扩展目录以分号分隔写入环境变量
			{
				WCHAR buf[8192] = { 0 };
				if (DWORD const n = GetEnvironmentVariableW(L"SYRINGE_EXTENSION_DIRS", buf, 8192);
					n > 0)
				{
					DWORD const Need = 13; // len(L"\\IHLibList.dll") + 1
					WCHAR* ctx = nullptr;
					for (WCHAR* d = wcstok_s(buf, L";", &ctx); d; d = wcstok_s(nullptr, L";", &ctx))
					{
						if (wcslen(d) + Need >= MAX_PATH) continue;
						WCHAR path[MAX_PATH] = { 0 };
						wcscpy_s(path, d);
						wcscat_s(path, L"\\IHLibList.dll");
						if (HMODULE h = LoadLibraryW(path))
							return h;
					}
				}
			}

			// 与当前模块同目录（支持部署到根目录之外的扩展包子目录）
			HMODULE self = nullptr;
			if (GetModuleHandleExW(
					GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
					| GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
					reinterpret_cast<LPCWSTR>(&FindIHLibList), &self)
				&& self)
			{
				WCHAR path[MAX_PATH] = { 0 };
				if (DWORD const len = GetModuleFileNameW(self, path, MAX_PATH);
					len > 0 && len < MAX_PATH)
				{
					if (WCHAR* const sep = wcsrchr(path, L'\\'))
						*(sep + 1) = 0;
					wcscat_s(path, L"IHLibList.dll");
					if (HMODULE h = LoadLibraryW(path))
						return h;
				}
			}

			if (HMODULE h = LoadLibraryW(L"Patches\\IHLibList.dll"))
				return h;
			return LoadLibraryW(L"IHLibList.dll");
		}

		void Load()
		{
			if (LibListDLL || LoaderLoaded || IsSyringeReadingHooks())return;
			LibListDLL = FindIHLibList();
			if (LibListDLL == NULL)
			{
				MessageBoxW(NULL, L"找不到IHLibList.dll，请重新安装。", L"万物互通", MB_OK | MB_ICONERROR);
				return;
			}

#define ____GetFunc(x) {_ ## x=(decltype(_ ## x))GetProcAddress(LibListDLL, #x);\
			if(_ ## x==nullptr)return;}
			____GetFunc(RegisterEntry);
			____GetFunc(GetEntries);
			____GetFunc(ServiceRequest);
			____GetFunc(QueryServiceRequest);
			____GetFunc(GetConfData);
			____GetFunc(RandomUINT);
			____GetFunc(GenerateID);
			____GetFunc(GetNamedPointer);
			____GetFunc(SetNamedPointer);
			____GetFunc(NamedPointer_GetName);
#undef ____GetFunc
			LoaderLoaded = true;
		}
		IHLibListLoader() { Load(); }
		void Rely() { Load(); };
}Loader;

#ifndef IHCore
	void BindConsole();

	InitResult* __cdecl InitFn(InitInput* Input)
	{
		LibInput = Input;
		BindConsole();
		MyInit(Result);
#ifndef EC_NoObjBase
#ifndef SIWIC
		if(Init::Internal_EnableSaveLoad)RegisterLoadSaveInterface();
#endif
#endif
		return &Result;
	}

	void RegisterListenerImpl()
	{
		InitialLoad::CreateRequestAndSubmit<InitialLoadParam_RegisterFunction>("EC::Internal::ListenerAccess", _strdup(RandStr(12).c_str()), ListenerAccess);
	}

	void BindConsole()
	{
		if (GetConsoleWindow() != NULL) {
			FILE* fp;
			freopen_s(&fp, "CONOUT$", "w", stdout);
			freopen_s(&fp, "CONOUT$", "w", stderr);
			freopen_s(&fp, "CONIN$", "r", stdin);

			std::ios::sync_with_stdio(true);
			std::cout.clear();
			std::cin.clear();
			std::cerr.clear();
		}
	}

	bool Initialize()
	{
		if (IsSyringeReadingHooks())return false;
		
		SyringeData::InitRemoteData();
		
		if (InitAvailable)return true;

		InitAvailable = true;

		RegisterListenerImpl();

		RegisterEntry(InitFn);
		return true;
	}
#else
	void RegisterListenerImpl()
	{
		InitialLoad::CreateRequestAndSubmit<InitialLoadParam_RegisterFunction>("EC::Internal::ListenerAccess", "EC::Internal::IHCore", ListenerAccess);
		RegisterLoadSaveInterface();
	}

	bool Initialize()
	{
		if (IsSyringeReadingHooks())return false;

		SyringeData::InitRemoteData();

		if (InitAvailable)return true;

		InitAvailable = true;
		RegisterListenerImpl();
		return true;
	}
#endif
	bool __cdecl RegisterEntry(LibFuncHandle Entry)
	{
		if (InitAvailable)return _RegisterEntry(Entry);
		else return false;
	}

	ConfData* GetConfData()
	{
		if (_GetConfData)return _GetConfData();
		else return nullptr;
	}

	PArray<IHInitialLoadService> __cdecl QueryServiceRequest(const char* Name)
	{
		if (InitAvailable)
		{
			PArray<IHInitialLoadService> pa;
			_QueryServiceRequest(Name, pa);
			return pa;
		}
		else return PArray<IHInitialLoadService>();
	}
}

bool CanInvokeAsCommand(FuncType Type)
{
	switch (Type)
	{
	case FuncType::Condition:
	case FuncType::Action:
	case FuncType::Callback:
	case FuncType::Procedure:
	case FuncType::ConditionAlt:
	case FuncType::ActionAlt:
	case FuncType::Remote:
		return true;
	default:
		return false;
	}
}

ConfData& GetConfigData()
{
	auto ptr = Init::GetConfData();
	if (!ptr)MessageBoxA(Game::hWnd, "Init::GetConfData() == nullptr", "EC SDK", MB_OK);
	return *ptr;
}
void UseOriginalFileClass(bool Option)
{
	GetConfigData().UseOriginalCCFile = Option;
}
bool UseOriginalFileClass()
{
	return GetConfigData().UseOriginalCCFile;
}
void DisableLoadSave()
{
	Init::Internal_EnableSaveLoad = false;
}

const char* Internal_GetLibRegName()
{
#ifdef IHCore
	return "IHCore";
#else
	return Init::Result.Info->LibName;
#endif
}

int Internal_GetLibVersion()
{
#ifdef IHCore
#include "..\IHCore\Version.h"
	return PRODUCT_VERSION;
#else
	return Init::Result.Info->Version;
#endif
}

void Internal_DebugLog(const char* pFormat, ...)
{
	JMP_STD(0x4068E0);
}

int Internal_GenerateID()
{
	return Init::_GenerateID();
}


void Internal_RegisterExportRTTI(const char* ClassName, const ECRTTIInfo* pInfo)
{
	Init::Loader.Rely();
	//if (!Init::_SetNamedPointer)MessageBoxA(Game::hWnd, "Init::_SetNamedPointer == nullptr", "EC SDK", MB_OK);

	if(Init::_SetNamedPointer)Init::_SetNamedPointer("EC::RTTI", ClassName, pInfo);
}
const ECRTTIInfo* Internal_GetRTTIInfo(const char* ClassName)
{
	Init::Loader.Rely();
	//if (!Init::_GetNamedPointer)MessageBoxA(Game::hWnd, "Init::_GetNamedPointer == nullptr", "EC SDK", MB_OK);

	return Init::_GetNamedPointer ? reinterpret_cast<const ECRTTIInfo*>(Init::_GetNamedPointer("EC::RTTI", ClassName)) : nullptr;
}
const char* Internal_GetRTTIClassName(const ECRTTIInfo* pInfo)
{
	Init::Loader.Rely();
	//if (!Init::_NamedPointer_GetName)MessageBoxA(Game::hWnd, "Init::_NamedPointer_GetName == nullptr", "EC SDK", MB_OK);
	return Init::_NamedPointer_GetName ? Init::_NamedPointer_GetName("EC::RTTI", pInfo) : nullptr;
}

void Internal_SetGlobalVarString(const char* Usage, const char* Key, const char* Value)
{
	Init::Loader.Rely();
	Init::_SetNamedPointer(Usage, Key, Value);
}

const char* Internal_GetGlobalVarString(const char* Usage, const char* Key)
{
	Init::Loader.Rely();
	return (const char*)Init::_GetNamedPointer(Usage, Key);
}

void Internal_SetGlobalVarPtr(const char* Usage, const char* Key, LPCVOID Ptr)
{
	Init::Loader.Rely();
	Init::_SetNamedPointer(Usage, Key, Ptr);
}

LPCVOID Internal_GetGlobalVarPtr(const char* Usage, const char* Key)
{
	Init::Loader.Rely();
	return Init::_GetNamedPointer(Usage, Key);
}

namespace InitialLoad
{
	void ServiceRequest(IHInitialLoadService IService)
	{
		if (Init::InitAvailable)Init::_ServiceRequest(IService);
	}
	unsigned int RandomUINT()
	{
		if (Init::_RandomUINT)return Init::_RandomUINT();
		else return (unsigned)rand();//regardless of quality when IHLibList version is low
	}
}



bool IHAvailable()
{
	return Init::InitAvailable;
}

