#pragma once
#include <Windows.h>
#include <strsafe.h>


size_t fnGetWStringLength(PWSTR szString, size_t maxSize);

size_t fnGetWStringSize(PWSTR szString, size_t maxSize);

LPVOID fnAllocate(SIZE_T cbBytes);

PWSTR convertCSTR(PCSTR sString, DWORD cbBytesInString);