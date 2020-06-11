#pragma once
#include <Windows.h>
#include <string>

#include "http_exceptions.h"

using namespace std;

class PathIdentifier
{
public:
	PathIdentifier(wstring absPath, DWORD cbMaxSizeForData=2048);
	PWSTR readNow();

private:
	PWSTR readFile();
	PWSTR listDir();

	const wstring m_absPath;
	const DWORD m_cbMaxSizeForData;
	const CHAR m_szDefaultDataToReturn[1] = "";
};
