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

typedef void (* const fpLogger)(PCWSTR szMassage);

class SimpleHttpServer {
public:
	explicit SimpleHttpServer(PCWSTR szDomainName, DWORD dwPort, PCWSTR szServerRootPath, fpLogger lpfnLogger);
	SimpleHttpServer(const SimpleHttpServer& other) = delete;
	SimpleHttpServer& operator=(const SimpleHttpServer& other) = delete;

	explicit SimpleHttpServer(SimpleHttpServer&& other);
	SimpleHttpServer& operator=(SimpleHttpServer&& other)=delete;
	~SimpleHttpServer();
	void fnStart();
	void setIsRuningSwitch(BOOL isNeedToRun);

private:
	BOOL fnSetupHttpServer();
	BOOL fnRegisterUrl(PCWSTR szUrl)const;
	PCWSTR fnHandleRequest(LPVOID pRequest);
	PCWSTR fnHandleRequestGet(LPVOID pRequest);
	void fnSendResponse(PCWSTR sTextToSend, LPVOID referenceRequest);
	void logInitializtionMessage()const;
	void shutDown();

	BOOL isInitializedSuccessfully;
	HANDLE m_RequestQueueHandle;
	const DWORD m_dwPort;
	const PCWSTR m_szDomainName;
	const PCWSTR m_szDefaultMessage;
	const PCWSTR m_szServerRootPath;
	const fpLogger m_lpLoggerFunction;
	const DWORD m_cbRequestMaxSize;
	BOOL m_isRuning;
};

