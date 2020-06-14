// SimpleHttpServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <http.h>
#include <iostream>
#include <Windows.h>
#include <strsafe.h>
#include <string.h>
#include "simple_http_server.h"


void ConsoleLogger(PCWSTR szMessage)
{
    std::wcout << L"[LOG_MESSAGE] " << szMessage << std::endl;
}

int __cdecl wmain(
    int argc,
    wchar_t* argv[]
)
{
    const PWSTR serverRoot = (const PWSTR)L"C:\\Users\\";
	SimpleHttpServer myserver(L"http://127.0.0.1", 80, serverRoot, ConsoleLogger);
    myserver.fnStart();

    
}
