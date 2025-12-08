#pragma once
#include <CCFileClass.h>
#include "SyringeEx.h"
#include "IH.Initial.h"

//所有的派生类应派生自 IHFileClass 或 IHReadOnlyFileClass 
//并具有接受noinit_t的构造函数
//实现所有虚函数并挂载即可

class IHFileClass :public FileClass
{
public:
	//Destructor
	virtual	~IHFileClass() {};//OK
	//FileClass
	virtual const char* GetFileName() const = 0;//OK
	virtual const char* SetFileName(const char* pFileName) = 0;//OK
	virtual BOOL CreateFile() = 0;//OK
	virtual BOOL DeleteFile() = 0;//OK
	virtual bool Exists(bool writeShared = false) = 0;
	virtual bool HasHandle() = 0;
	virtual bool Open(FileAccessMode access) = 0;
	virtual bool OpenEx(const char* pFileName, FileAccessMode access);
	virtual int ReadBytes(void* pBuffer, int nNumBytes) = 0; //Returns number of bytes read.
	virtual int Seek(int offset, FileSeekMode seek) = 0;
	virtual int GetFileSize() = 0;
	virtual int WriteBytes(void* pBuffer, int nNumBytes) = 0; //Returns number of bytes written.
	virtual void Close() = 0;//OK

	//默认空实现，可根据需要覆写，在未覆写时无行为
	virtual void CDCheck(DWORD errorCode, bool bUnk, const char* pMethodName);
	//受构造方式影响，此类型的构造函数不会真正执行，在Initialize前所在内存除了虚表是全0的
	//所有的派生类必须实现这个函数来完成初始化工作
	virtual void Initialize() = 0;

protected:
	explicit __forceinline IHFileClass(noinit_t _):FileClass(_)
	{ }

	//Properties
public:
};

class IHReadOnlyFileClass :public IHFileClass
{
public:
	//Destructor
	virtual	~IHReadOnlyFileClass() {};
	//FileClass
	virtual BOOL CreateFile() final;
	virtual BOOL DeleteFile() final;
	virtual int WriteBytes(void* pBuffer, int nNumBytes) final; //Returns number of bytes written.
protected:
	explicit __forceinline IHReadOnlyFileClass(noinit_t _) :IHFileClass(_)
	{ }

	//Properties
public:
};

class IHWriteOnlyFileClass :public IHFileClass
{
public:
	//Destructor
	virtual	~IHWriteOnlyFileClass() {};
	//FileClass
	virtual int ReadBytes(void* pBuffer, int nNumBytes) final; //Returns number of bytes read.
protected:
	explicit __forceinline IHWriteOnlyFileClass(noinit_t _) :IHFileClass(_)
	{ }

	//Properties
public:
};

class IHInputStreamClass : public IHReadOnlyFileClass
{
public:
	//Destructor
	virtual	~IHInputStreamClass() {};
	//FileClass
	virtual int Seek(int offset, FileSeekMode seek) final;
	virtual int GetFileSize() final;
protected:
	explicit __forceinline IHInputStreamClass(noinit_t _) :IHReadOnlyFileClass(_)
	{ }

	//Properties
public:
};

class IHOutputStreamClass : public IHWriteOnlyFileClass
{
public:
	//Destructor
	virtual	~IHOutputStreamClass() {};
	//FileClass
	virtual int Seek(int offset, FileSeekMode seek) final;
	virtual int GetFileSize() final;

protected:
	explicit __forceinline IHOutputStreamClass(noinit_t _) :IHWriteOnlyFileClass(_)
	{ }

	//Properties
public:
};

template<typename ParamType>
vptr_t GetIHFileRegisterKey()
{
	static_assert(std::is_base_of<IHFileClass, ParamType>::value, "ParamType 必须派生自 IHFileClass ！");
	ParamType p{ noinit_t() };
	return *reinterpret_cast<vptr_t*>(&p);
}