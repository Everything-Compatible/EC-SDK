#pragma once
#pragma once
#include "IH.Loader.h"
#include "EC.Stream.h"
#include "EC.LoadSave.h"
#include "EC.ObjBase.h"

/*

删掉所有自加载函数！！
写一套新的自加载接口
Load
Save
FinalSwizzle
2个PointerGotInvalid
InitFromINI
LoadFromINI
Update
ComputeCRC
OnExeRun
AfterECInitialize
OnReadingScenario

*/


class ECGameClass_
{
	void Register();
	void Unregister();
public:
	//virtual ECGameClass_* TypeCast(const ECGameType& Type) noexcept;

	virtual void LoadGame(ECStreamReader& Stream) = 0;

	virtual void SaveGame(ECStreamWriter& Stream) = 0;

	virtual size_t GetSizeMax() = 0;


	//bRemoved : true=真寄了 false=临时离场
	virtual void PointerGotInvalid(AbstractClass* const pObject, bool bRemoved) = 0;

	virtual void FinalSwizzle() = 0;

	virtual void Update() = 0;


	ECGameClass_();
	ECGameClass_(const ECGameClass_&);
	ECGameClass_(ECGameClass_&&);
	virtual ~ECGameClass_();
};

#ifndef IHCore

//其RTTI信息注册于IHCore
ECObjType ECGameClass : EC_USERTTI, EC_USEUNIQUEID, EC_USELOADSAVE
{
	ECRTTI_DefineRTTIFunction(ECGameClass)
	ECObj_DefineLoadSaveFunction
public:
	/*
	已经定义了以下接口：
	virtual const ECRTTIInfo* WhatAmI() const;
	int GetUniqueID() const;
	template<typename T>
	T* DynamicCast();
	template<typename T>
	const T* DynamicCast() const;

	以下接口请自行实现：

	virtual void LoadGame(ECStreamReader& Stream) = 0;
	virtual void SaveGame(ECStreamWriter& Stream) = 0;
	virtual size_t GetSizeMax() = 0;
	virtual void FinalSwizzle() = 0;
	virtual void PointerGotInvalid(AbstractClass* const pObject, bool bRemoved) = 0;
	virtual void PointerGotInvalid(ECGameClass* const pObject, bool bRemoved) = 0;
	*/

	virtual void ComputeCRC(CRCEngine& crc) = 0;
	virtual void Init() = 0;
	virtual void Update() = 0;

	virtual HouseClass* GetHouse() const = 0;
	virtual CoordStruct GetCoords() const = 0;
	virtual CoordStruct GetCenterCoords() const = 0;

	ECGameClass() { Init(); };

private:
	//在派生类当中自行实现Serialize函数
	template <typename T>
	void Serialize(T& Stream) {};
public:

	const ECRTTI* GetRTTI() const
	{
		return static_cast<const ECRTTI*>(this);
	}
};
ECObj_ImportFactory(ECGameClass)


//存读全局设置
ECObjType ECGlobalData : EC_USELOADSAVE, EC_LOADFROMRULES, EC_STATICINIT
{
public:
	/*
	已经定义了以下接口，以下接口请自行实现：
	virtual void LoadGame(ECStreamReader& Stream) = 0;
	virtual void SaveGame(ECStreamWriter& Stream) = 0;
	virtual size_t GetSizeMax() = 0;
	virtual void FinalSwizzle() = 0;
	virtual void LoadFromINI(CCINIClass* INI) = 0;
	virtual void PointerGotInvalid(AbstractClass* const pObject, bool bRemoved) = 0;
	virtual void PointerGotInvalid(ECGameClass* const pObject, bool bRemoved) = 0;

	virtual void InitFromINI(CCINIClass* INI) = 0;
	virtual void LoadFromINI(CCINIClass* INI) = 0;

	virtual void InitOnExeRun() = 0;
	virtual void InitAfterECInitialize() = 0;
	virtual void InitOnLoadScenario() = 0;
	*/

	ECGlobalData() {}
};
ECObj_ImportFactory(ECGlobalData)

//其RTTI信息注册于IHCore
ECObjType ECGameTypeClass : public ECGameClass, EC_LOADFROMRULES, EC_STATICINIT
{
	ECRTTI_DefineRTTIFunction(ECGameTypeClass)
	ECObj_DefineLoadSaveFunction
public:

	/*
	已经定义了以下接口：
	virtual const ECRTTIInfo* WhatAmI() const;
	int GetUniqueID() const;
	template<typename T>
	T* DynamicCast();
	template<typename T>
	const T* DynamicCast() const;

	以下接口请自行实现：

	virtual void ComputeCRC(CRCEngine& crc) = 0;

	virtual void LoadGame(ECStreamReader& Stream) = 0;
	virtual void SaveGame(ECStreamWriter& Stream) = 0;
	virtual size_t GetSizeMax() = 0;
	virtual void FinalSwizzle() = 0;
	virtual void PointerGotInvalid(AbstractClass* const pObject, bool bRemoved) = 0;
	virtual void PointerGotInvalid(ECGameClass* const pObject, bool bRemoved) = 0;

	virtual void Update() = 0;
	virtual HouseClass* GetHouse() const = 0;
	virtual CoordStruct GetCoords() const = 0;
	virtual CoordStruct GetCenterCoords() const = 0;

	virtual void InitFromINI(CCINIClass* INI) = 0;
	virtual void LoadFromINI(CCINIClass* INI) = 0;

	virtual void InitOnExeRun() = 0;
	virtual void InitAfterECInitialize() = 0;
	virtual void InitOnLoadScenario() = 0;
	*/
	virtual ECObjPtr<ECGameClass> CreateObjectOfType() = 0;

	ECGameTypeClass() : ECGameClass() { }
private:
	//在派生类当中自行实现Serialize函数
	template <typename T>
	void Serialize(T& Stream) {};
};
ECObj_ImportFactory(ECGameTypeClass)

#endif