#pragma once
#include <Windows.h>
#include <strsafe.h>
#include <vector>
#include <string>

size_t fnGetWStringLength(PCWSTR szString, size_t maxSize);

size_t fnGetWStringSize(PCWSTR szString, size_t maxSize);

LPVOID fnAllocate(SIZE_T cbBytes);

void fnFree(LPVOID);

PWSTR convertCSTR(PCSTR sString, DWORD cbBytesInString);



template<class stringType>
void add_string_to_vector(const stringType& str, std::vector<char>& buffer, unsigned int offset)
{
	CopyMemory(
		buffer.data() + offset,
		str.data(),
		str.size() * sizeof(WCHAR));
}