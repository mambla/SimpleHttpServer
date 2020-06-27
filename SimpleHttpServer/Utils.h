#pragma once
#include <Windows.h>
#include <strsafe.h>


size_t fnGetWStringLength(PCWSTR szString, size_t maxSize);

size_t fnGetWStringSize(PCWSTR szString, size_t maxSize);

LPVOID fnAllocate(SIZE_T cbBytes);

void fnFree(LPVOID);

PWSTR convertCSTR(PCSTR sString, DWORD cbBytesInString);


class DestroyObjectOnCall
{
public:
	DestroyObjectOnCall();
	DestroyObjectOnCall(LPVOID lpObject);
	DestroyObjectOnCall(const DestroyObjectOnCall& other)=delete;
	DestroyObjectOnCall(DestroyObjectOnCall&& other);
	DestroyObjectOnCall& operator=(DestroyObjectOnCall&& other);
	DestroyObjectOnCall& operator=(const DestroyObjectOnCall& other)=delete;
	~DestroyObjectOnCall() = default;

	void OnCall(int sigValue)const;

private:
	LPVOID m_lpObject;

};