#include "path_identifier.h"
#include <strsafe.h>
#include <iostream>


PathIdentifier::PathIdentifier(wstring absPath)
	:m_absPath(absPath),
	cbMaxSizeForData(2048)
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


PWSTR convertCSTR(PCSTR sString, DWORD cbBytesInString)
{
	PWSTR sNewString = (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbBytesInString * 2);
	for (size_t i = 0; i < cbBytesInString; i++)
	{
		sNewString[i] = (WCHAR)sString[i];
	}
	//HeapFree(GetProcessHeap(), 0, (void*)sString);
	return sNewString;
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
		//check extantions
		PCHAR buffer = (PCHAR)HeapAlloc(GetProcessHeap(), 0 , cbChunkSize);
		if (NULL == buffer)
		{
			return NULL;
		}

		while ((cbTotalBytesRead < cbMaxSizeForData)
			&&
			(ReadFile(hfile,
			buffer + cbTotalBytesRead,
			cbChunkSize,
			&cbBytesRead,
			NULL))
			)
		{
			cbTotalBytesRead += cbBytesRead;

			if (!cbBytesRead)
			{
				break;
			}
			buffer = (PCHAR)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY , buffer, cbTotalBytesRead + cbChunkSize);

		}


			CloseHandle(hfile);
			return (PWSTR)convertCSTR((PCSTR)buffer, cbTotalBytesRead+1);
		

		CloseHandle(hfile);
		return NULL;

	}


}

PWSTR PathIdentifier::listDir()
{
	HANDLE hFirstFileInPath;
	WCHAR lineResharper[] = L"\n";
	WIN32_FIND_DATAW fileData;
	DWORD cbTotalWCharsCopied = 0;
	wstring finalMessage;
	DWORD cbChunkSize = 1024;
	WCHAR pszPathToSearchAsFolder[MAX_PATH];
	
	lstrcpyW(pszPathToSearchAsFolder, m_absPath.c_str());
	StringCchCatW(pszPathToSearchAsFolder,
		lstrlenW(pszPathToSearchAsFolder ) + lstrlenW(L"\\*") + 1,
		L"\\*"
	);
	PWCHAR dataBuffer = (PWCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbMaxSizeForData * 2);

	if (NULL == dataBuffer)
	{
		return NULL;
	}

	hFirstFileInPath = FindFirstFileW(pszPathToSearchAsFolder, &fileData);
	if (INVALID_HANDLE_VALUE == hFirstFileInPath)
	{
		return NULL;
	}
	do {
		DWORD cbLastFileNameLength = lstrlenW(fileData.cFileName);
		cbTotalWCharsCopied += cbLastFileNameLength;
		StringCchCatW(dataBuffer, cbTotalWCharsCopied + 1, fileData.cFileName);
		
		cbTotalWCharsCopied += lstrlenW(lineResharper);
		StringCchCatW(dataBuffer, cbTotalWCharsCopied + 1, lineResharper);

	
	} while (FindNextFileW(hFirstFileInPath, &fileData) != 0
		&& cbTotalWCharsCopied < cbMaxSizeForData);
	return (PWSTR)dataBuffer;
}


