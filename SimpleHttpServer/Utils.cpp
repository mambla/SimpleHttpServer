#include "Utils.h"
#include <signal.h>
#include <iostream>
#include <exception>
#include <vector>
#include <memory>

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

TranslateCstrAndUnicode::TranslateCstrAndUnicode(Direcrion direction)
	:_direction(direction)
{
}

LPVOID TranslateCstrAndUnicode::translate_data(LPVOID data)const
{
	switch (_direction)
	{
	case Direcrion::C_TO_UNICODE:
		return (LPVOID)(c_str_to_unicode(static_cast<PCSTR>(data)));
	case Direcrion::UNICODE_TO_C:
		return (LPVOID)(unicode_to_c(static_cast<PCWSTR>(data)));
	default:
		return NULL;
	}
}


PCWSTR TranslateCstrAndUnicode::c_str_to_unicode(PCSTR data)const
{
	static INT STRING_ENDS_WITH_ZERO = -1;
	static INT MAKE_API_RETURN_SIZE_OF_TARGET_BUFFER = 0;
	LPWSTR target_buffer=NULL;
	DWORD appropraite_size_target_buffer = sizeof(WCHAR) * \
		MultiByteToWideChar(CP_ACP,
			MB_PRECOMPOSED,
			data,
			STRING_ENDS_WITH_ZERO,
			target_buffer,
			MAKE_API_RETURN_SIZE_OF_TARGET_BUFFER);

	if (!MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED,
		data,
		STRING_ENDS_WITH_ZERO,
		target_buffer,
		appropraite_size_target_buffer))
	{
		return NULL;
	}
	else
	{
		return target_buffer;
	}
}


PCSTR TranslateCstrAndUnicode::unicode_to_c(PCWSTR data)const
{
	static DWORD NO_CONVERSION_FLAG = 0;
	static INT STRING_ENDS_WITH_ZERO = -1;
	static INT MAKE_API_RETURN_SIZE_OF_TARGET_BUFFER = 0;
	static LPCCH DEFAULT_CHAR_IF_CANNOT_CONVERT = NULL;
	BOOL is_default_char_used;
	LPSTR target_buffer=NULL;

	DWORD size_for_target_buffer = WideCharToMultiByte(CP_ACP,
		NO_CONVERSION_FLAG,
		data,
		STRING_ENDS_WITH_ZERO,
		target_buffer,
		MAKE_API_RETURN_SIZE_OF_TARGET_BUFFER,
		DEFAULT_CHAR_IF_CANNOT_CONVERT,
		&is_default_char_used);

	if (size_for_target_buffer)
	{
		if (!WideCharToMultiByte(CP_ACP,
			NO_CONVERSION_FLAG,
			data,
			STRING_ENDS_WITH_ZERO,
			target_buffer,
			size_for_target_buffer,
			DEFAULT_CHAR_IF_CANNOT_CONVERT,
			&is_default_char_used))
		{
			target_buffer = NULL;
		}
	}
	else
	{
		target_buffer = NULL;
	}
	return target_buffer;

}

inline Win32FreeHeap::Win32FreeHeap()
	:_heap_handle(GetProcessHeap()),
	_free_flags(0)
{
}

void Win32FreeHeap::operator()(LPVOID ptr)
{
	HeapFree(_heap_handle, _free_flags, ptr);
}

void add_string_to_vector(const std::wstring& str, std::vector<char>& buffer, unsigned int offset)
{
	CopyMemory(
		buffer.data() + offset,
		str.data(),
		str.size() * sizeof(WCHAR));
}