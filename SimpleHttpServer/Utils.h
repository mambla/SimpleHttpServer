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



void add_string_to_vector(const std::string& str, std::vector<char>& buffer, unsigned int offset);

void add_string_to_vector(const std::wstring& str, std::vector<char>& buffer, unsigned int offset);

class AutoCloseHandle
{
public:
	AutoCloseHandle(HANDLE handle);
	~AutoCloseHandle();

public:
	HANDLE data();

public:
	AutoCloseHandle(const AutoCloseHandle& other) = delete;
	AutoCloseHandle(AutoCloseHandle&& other) = delete;
	AutoCloseHandle& operator=(AutoCloseHandle&& other) = delete;
	AutoCloseHandle& operator=(AutoCloseHandle& other) = delete;


private:
	HANDLE _handle;
};