#include "Utils.h"
#include <signal.h>
#include <iostream>
size_t fnGetWStringLength(PCWSTR szString, size_t maxSize)
{
	size_t result;
	HRESULT status = StringCbLengthW(szString, maxSize, &result);
	if (S_OK == status)
		return result;
	else
		return 0;
}

size_t fnGetWStringSize(PCWSTR szString, size_t maxSize)
{
	return fnGetWStringLength(szString, maxSize) * sizeof(WCHAR);
}

LPVOID fnAllocate(SIZE_T cbBytes)
{
	return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbBytes);
}

void fnFree(LPVOID lpPtr)
{
	HeapFree(GetProcessHeap(), 0, lpPtr);
}

PWSTR convertCSTR(PCSTR sString, DWORD cbBytesInString)
{
	PWSTR sNewString = static_cast<PWSTR>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbBytesInString * 2));
	if (NULL == sNewString)
	{
		return NULL;
	}

	for (size_t i = 0; i < cbBytesInString; i++)
	{
		sNewString[i] = (WCHAR)sString[i];
	}
	return sNewString;
}

DestroyObjectOnCall::DestroyObjectOnCall()
	:m_lpObject(NULL)
{
}

DestroyObjectOnCall::DestroyObjectOnCall(LPVOID lpObject)
	:m_lpObject(lpObject)
{

}

DestroyObjectOnCall::DestroyObjectOnCall(DestroyObjectOnCall&& other)
	: m_lpObject(other.m_lpObject)
{
	other.m_lpObject = NULL;
}

DestroyObjectOnCall& DestroyObjectOnCall::operator=(DestroyObjectOnCall&& other)
{
	m_lpObject = other.m_lpObject;
	other.m_lpObject = NULL;
	return *this;
}


void DestroyObjectOnCall::OnCall(int sigValue)const {
	if (SIGINT == sigValue &&
		(NULL != m_lpObject))
	{
		delete m_lpObject;
	}
	
}
