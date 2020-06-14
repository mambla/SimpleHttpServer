#pragma once
#include <Windows.h>
#include <strsafe.h>


size_t fnGetWStringLength(PCWSTR szString, size_t maxSize);

size_t fnGetWStringSize(PCWSTR szString, size_t maxSize);

LPVOID fnAllocate(SIZE_T cbBytes);

PWSTR convertCSTR(PCSTR sString, DWORD cbBytesInString);