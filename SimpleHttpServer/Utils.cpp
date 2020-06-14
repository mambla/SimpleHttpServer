#include "Utils.h"

size_t fnGetWStringLength(PWSTR szString, size_t maxSize)
{
	size_t result;
	HRESULT status = StringCbLengthW(szString, maxSize, &result);
	if (S_OK == status)
		return result;
	else
		return 0;
}

size_t fnGetWStringSize(PWSTR szString, size_t maxSize)
{
	return fnGetWStringLength(szString, maxSize) * sizeof(WCHAR);
}

LPVOID fnAllocate(SIZE_T cbBytes)
{
	return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbBytes);
}

PWSTR convertCSTR(PCSTR sString, DWORD cbBytesInString)
{
	PWSTR sNewString = (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbBytesInString * 2);
	if (NULL == sNewString)
	{
		return NULL;
	}

	for (size_t i = 0; i < cbBytesInString; i++)
	{
		sNewString[i] = (WCHAR)sString[i];
	}
	//HeapFree(GetProcessHeap(), 0, (void*)sString);
	return sNewString;
}
