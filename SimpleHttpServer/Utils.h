#pragma once
#include <Windows.h>
#include <strsafe.h>
#include <functional>
#include <vector>
#include <string>

static auto fn_realloc_win32 = std::bind(HeapReAlloc, GetProcessHeap(), HEAP_ZERO_MEMORY, std::placeholders::_1, std::placeholders::_2);

size_t fnGetWStringLength(PCWSTR szString, size_t maxSize);

size_t fnGetWStringSize(PCWSTR szString, size_t maxSize);

LPVOID fnAllocate(SIZE_T cbBytes);

void fnFree(LPVOID);




class TranslateCstrAndUnicode {
public:
	enum class Direcrion {C_TO_UNICODE, UNICODE_TO_C};
	TranslateCstrAndUnicode(Direcrion direction);
	LPVOID translate_data(LPVOID data)const;

private:

	PCWSTR c_str_to_unicode(PCSTR data)const;
	PCSTR unicode_to_c(PCWSTR data)const;


	Direcrion _direction;
};

class Win32FreeHeap
{
public:
	Win32FreeHeap();
	void operator()(LPVOID ptr);
private:
	HANDLE _heap_handle;
	DWORD _free_flags;
};


void add_string_to_vector(const std::wstring& str, std::vector<char>& buffer, unsigned int offset);
