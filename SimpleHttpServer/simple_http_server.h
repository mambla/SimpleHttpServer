#pragma once
#ifndef UNICODE
#define UNICODE
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <http.h>
#include <stdio.h>

#include "http_exceptions.h"

typedef void (*fpLogger)(PCWSTR szMassage);

class SimpleHttpServer {
public:
	SimpleHttpServer(const PCWSTR szUrl, DWORD dwPort, fpLogger);
	~SimpleHttpServer();
	void fnStart();

private:
	BOOL fnSetupHttpServer();
	BOOL fnRegisterUrl(PCWSTR szUrl)noexcept;
	PCWSTR fnHandleRequest(LPVOID pRequest);
	PCWSTR fnHandleRequestGet(LPVOID pRequest);
	void fnSendResponse(std::wstring sTextToSend, LPVOID referenceRequest);

	const DWORD m_dwPort;
	const PCWSTR m_szUrl;
	const PCWSTR m_DefaultMessage;
	const fpLogger m_lpLoggerFunction;
	const DWORD m_cbRequestMaxSize = 2048;
	HANDLE m_RequestQueueHandle;
};

