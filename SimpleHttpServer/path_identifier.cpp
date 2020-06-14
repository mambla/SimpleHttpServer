#pragma once
#include "path_identifier.h"
#include <strsafe.h>
#include <iostream>
#include <Windows.h>
#include "Utils.h"

PathIdentifier::PathIdentifier(wstring absPath, DWORD cbMaxSizeForData)
	:m_absPath(absPath),
	m_cbMaxSizeForData(cbMaxSizeForData)
{
}

PWSTR PathIdentifier::readNow()
{
	PWCHAR szDataRead = NULL;
	DWORD attribute = GetFileAttributesW(m_absPath.c_str());

	if (INVALID_FILE_ATTRIBUTES == attribute)
	{
		szDataRead = NULL;
	}
	
	else if (FILE_ATTRIBUTE_DIRECTORY & attribute)
	{
		szDataRead = listDir();
	}
	else // is a file
	{
		szDataRead = readFile();
	}
	return szDataRead;
}



PWSTR PathIdentifier::readFile()
{
	const DWORD cbChunkSize=100;
	DWORD cbBytesRead=0;
	DWORD cbTotalBytesRead=0;
	PCHAR offsetInBuffer;
	HANDLE hfile = CreateFileW(m_absPath.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	if (INVALID_HANDLE_VALUE == hfile)
	{
		CloseHandle(hfile);
		return NULL;
	}

	else
	{
		//TODO: check allowed extantions
		PCHAR aBuffer = (PCHAR)fnAllocate(cbChunkSize);
		if (NULL == aBuffer)
		{
			return NULL;
		}

		while (
			(cbTotalBytesRead < m_cbMaxSizeForData)
			&&
			(ReadFile(hfile,
			aBuffer + cbTotalBytesRead,
			cbChunkSize,
			&cbBytesRead,
			NULL))
			)
		{
			cbTotalBytesRead += cbBytesRead;

			if (!cbBytesRead || NULL == aBuffer)
			{
				break;
			}

			aBuffer = (PCHAR)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY , aBuffer, cbTotalBytesRead + cbChunkSize);
			
			if(NULL == aBuffer)
			{
				return NULL;
			}

		}


			CloseHandle(hfile);
			return (PWSTR)convertCSTR((PCSTR)aBuffer, cbTotalBytesRead+1);
		

		CloseHandle(hfile);
		return NULL;

	}


}

PWSTR PathIdentifier::listDir()
{
	HANDLE hFirstFileInPath;
	WCHAR sLineSeperator[] = L"\n";
	WIN32_FIND_DATAW fileData;
	DWORD cbTotalWCharsCopied = 0;
	wstring sFinalMessage;
	DWORD cbChunkSize = 1024;
	WCHAR aPathToSearchAsFolder[MAX_PATH];
	
	StringCbCopyW(aPathToSearchAsFolder, MAX_PATH * sizeof(WCHAR), m_absPath.c_str());
	StringCchCatW(aPathToSearchAsFolder,
		fnGetWStringLength(aPathToSearchAsFolder, MAX_PATH) + fnGetWStringLength((PWSTR)L"\\*", MAX_PATH) + 1,
		L"\\*"
	);

	PWCHAR aDataBuffer = (PWCHAR)fnAllocate(m_cbMaxSizeForData * 2);

	if (NULL == aDataBuffer)
	{
		return NULL;
	}

	hFirstFileInPath = FindFirstFileW(aPathToSearchAsFolder, &fileData);
	if (INVALID_HANDLE_VALUE == hFirstFileInPath)
	{
		return NULL;
	}
	do {
		DWORD cbLastFileNameLength = fnGetWStringLength(fileData.cFileName, m_cbMaxSizeForData);
		cbTotalWCharsCopied += cbLastFileNameLength;
		StringCchCatW(aDataBuffer, cbTotalWCharsCopied + 1, fileData.cFileName);
		
		cbTotalWCharsCopied += fnGetWStringLength(sLineSeperator, m_cbMaxSizeForData);
		StringCchCatW(aDataBuffer, cbTotalWCharsCopied + 1, sLineSeperator);

	
	} while (FindNextFileW(hFirstFileInPath, &fileData) != 0
		&& cbTotalWCharsCopied < m_cbMaxSizeForData);
	return (PWSTR)aDataBuffer;
}


