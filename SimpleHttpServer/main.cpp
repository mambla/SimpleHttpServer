// SimpleHttpServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <http.h>
#include <iostream>
#include <Windows.h>
#include <strsafe.h>
#include <string.h>
#include <string>
#include <signal.h>
#include "simple_http_server.h"
#include "Utils.h"

SimpleHttpServer* myServerGlobal=NULL;

void ConsoleLogger(PCWSTR szMessage)
{
    std::wcout << L"[LOG_MESSAGE] " << szMessage << std::endl;
}

void SignalHandler(int signal)
{
    if (signal == SIGINT) {
        if (NULL != myServerGlobal)
        {
            myServerGlobal->setIsRuningSwitch(FALSE);
        }
    }
    else {
        std::cout << "Other Signal" << std::endl;

    }
}

int __cdecl wmain(
    int argc,
    wchar_t* argv[]
)
{
    (void)argc;
    (void)argv;

    typedef void (*SignalHandlerPointer)(int);
    SignalHandlerPointer previousHandler;
    {
        std::wstring serverRoot(L"C:\\Users\\amitb\\Downloads\\");
        SimpleHttpServer myserver(L"http://127.0.0.1", 80, serverRoot, ConsoleLogger);
        ::myServerGlobal = &myserver;
        previousHandler = signal(SIGINT, SignalHandler);
        myserver.fnStart();
    
    }
    
}
