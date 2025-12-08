#include "IH.File.h"
#include "IH.Loader.h"



bool IHFileClass::OpenEx(const char* pFileName, FileAccessMode access)
{
	this->SetFileName(pFileName);
	return this->Open(access);
}

void IHFileClass::CDCheck(DWORD errorCode, bool bUnk, const char* pMethodName)
{
	//ERROR if(errorCode != 0)
}

BOOL IHReadOnlyFileClass::CreateFile()
{
	CDCheck(ERROR_NOT_SUPPORTED, false, "IHReadOnlyFileClass::CreateFile");
	//ERROR
	return FALSE;
}
BOOL IHReadOnlyFileClass::DeleteFile()
{
	CDCheck(ERROR_NOT_SUPPORTED, false, "IHReadOnlyFileClass::DeleteFile");
	//ERROR
	return FALSE;
}
int IHReadOnlyFileClass::WriteBytes(void* pBuffer, int nNumBytes)
{
	CDCheck(ERROR_NOT_SUPPORTED, false, "IHReadOnlyFileClass::WriteBytes");
	//ERROR
	return 0;
}

int IHInputStreamClass::Seek(int offset, FileSeekMode seek)
{
	CDCheck(ERROR_NOT_SUPPORTED, false, "IHInputStreamClass::Seek");
	//ERROR
	return 0;
}
int IHInputStreamClass::GetFileSize()
{
	CDCheck(ERROR_NOT_SUPPORTED, false, "IHInputStreamClass");
	//ERROR
	return 0;
}

int IHWriteOnlyFileClass::ReadBytes(void* pBuffer, int nNumBytes) //Returns number of bytes read.
{
	CDCheck(ERROR_NOT_SUPPORTED, false, "IHWriteOnlyFileClass::ReadBytes");
	//ERROR
	return 0;
}
int IHOutputStreamClass::Seek(int offset, FileSeekMode seek)
{
	CDCheck(ERROR_NOT_SUPPORTED, false, "IHOutputStreamClass::Seek");
	//ERROR
	return 0;
}
int IHOutputStreamClass::GetFileSize()
{
	CDCheck(ERROR_NOT_SUPPORTED, false, "IHOutputStreamClass::GetFileSize");
	//ERROR
	return 0;
}