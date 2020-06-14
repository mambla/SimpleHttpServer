#include <time.h>
#include<stdlib.h>
#include <string>
#include <strsafe.h>
#include "simple_http_server.h"
#include "path_identifier.h"
#include "Utils.h"

PCSTR renderUnicodeToByteStrHtml(PCWSTR szOriginalUnicodeMessage, size_t resultMaxSize);


#define INITIALIZE_HTTP_RESPONSE( resp, status, reason )    \
    do                                                      \
    {                                                       \
        RtlZeroMemory( (resp), sizeof(*(resp)) );           \
        (resp)->StatusCode = (status);                      \
        (resp)->pReason = (reason);                         \
        (resp)->ReasonLength = (USHORT) strlen(reason);     \
    } while (FALSE)

#define ADD_KNOWN_HEADER(Response, HeaderId, RawValue)               \
    do                                                               \
    {                                                                \
        (Response).Headers.KnownHeaders[(HeaderId)].pRawValue =      \
                                                          (RawValue);\
        (Response).Headers.KnownHeaders[(HeaderId)].RawValueLength = \
            (USHORT) strlen(RawValue);                               \
    } while(FALSE)


PCWSTR formatDomainName(PCWSTR szDomainName, DWORD dwPort)
{
    const DWORD cbSizeToHoldPortString = 5 * sizeof(WCHAR);
    const PCWSTR szDomainFormat = L"%ws:%d/";
    const DWORD cbSizeToHoldFullDomainName = fnGetWStringSize((PWSTR)szDomainName, MAX_PATH)
                                             + cbSizeToHoldPortString
                                             + 1;
    STRSAFE_LPWSTR pszDomainString = reinterpret_cast<STRSAFE_LPWSTR>(fnAllocate(cbSizeToHoldFullDomainName));
    if (NULL != pszDomainString)
    {
        StringCbPrintfW(pszDomainString, cbSizeToHoldFullDomainName, szDomainFormat, szDomainName, dwPort);
        return pszDomainString;
    }

    return NULL;
}

SimpleHttpServer::SimpleHttpServer(PCWSTR szDomainName, DWORD dwPort, const PWSTR szServerRootPath, fpLogger lpfnLoggerFunction)
	:m_dwPort(dwPort),
	m_szDomainName(formatDomainName(szDomainName, dwPort)),
    m_lpLoggerFunction(lpfnLoggerFunction),
    m_szDefaultMessage(L"Not Found."),
    m_cbRequestMaxSize(2048),
    m_szServerRootPath(szServerRootPath)
{
    logInitializtionMessage();
}

SimpleHttpServer::~SimpleHttpServer()
{
	CloseHandle(m_RequestQueueHandle);
    HttpTerminate(HTTP_INITIALIZE_SERVER, NULL);
    if (NULL != m_szDomainName)
        HeapFree(GetProcessHeap(), 0, (LPVOID)m_szDomainName);
}



void SimpleHttpServer::fnStart()
{
    if (!isInitializedSuccessfully) {
        throw ServerInitilizationError();
    }

    ULONG result;
    ULONG cbBytesReadFrom;
    DWORD cbSizeOfRequestBuffer = sizeof(HTTP_REQUEST) + m_cbRequestMaxSize;
    PHTTP_REQUEST lpRequestBuffer = reinterpret_cast<HTTP_REQUEST*>(fnAllocate(cbSizeOfRequestBuffer));

    if (NULL == lpRequestBuffer)
    {
        SetLastError(STATUS_NO_MEMORY);
        return;
    }
    
    lpRequestBuffer->RequestId = HTTP_NULL_ID;

    while(TRUE)
    {
       result = HttpReceiveHttpRequest(
            m_RequestQueueHandle,
            NULL,
            HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY,
            lpRequestBuffer,
            cbSizeOfRequestBuffer,
            &cbBytesReadFrom,
            NULL
        );
       if (NO_ERROR == result)
       {
         //hnadle request
            PCWSTR sTextResponse = 
                fnHandleRequest((PHTTP_REQUEST)lpRequestBuffer);

            if (NULL != sTextResponse) {
                fnSendResponse(sTextResponse, lpRequestBuffer);
                HeapFree(GetProcessHeap(), 0, (LPVOID)sTextResponse);
            }
            else
            {              
                fnSendResponse(m_szDefaultMessage, lpRequestBuffer);
            }

       }
       Sleep(10);
    }

    HeapFree(GetProcessHeap(), 0, lpRequestBuffer);
}

BOOL SimpleHttpServer::fnSetupHttpServer()
{   
    //initialize service
    ULONG retCode = HttpInitialize(
        HTTPAPI_VERSION_1,
        HTTP_INITIALIZE_SERVER,   
        NULL
    );

    if (retCode != NO_ERROR)
    {
        return FALSE;
    }

    //set up requests handle:
    retCode = HttpCreateHttpHandle(
        &m_RequestQueueHandle,
        0);

    if (retCode != NO_ERROR)
    {
        return FALSE;
    }

	return TRUE;
}

BOOL SimpleHttpServer::fnRegisterUrl(PCWSTR szUrl) 
{
    if (NULL == szUrl)
    {
        return FALSE;
    }

    ULONG retCode = HttpAddUrl(
        m_RequestQueueHandle,
        szUrl,
        NULL
    );
    if (NO_ERROR != retCode)
    {
        return FALSE;
    }

    return TRUE;
}


PCWSTR appendToBasePath(PCWSTR basePath, PCWSTR path)
{
    DWORD sizeOfPath; 
    DWORD sizeOfBasePath;
    sizeOfBasePath = fnGetWStringLength((PWSTR)basePath, MAX_PATH);
    sizeOfPath = fnGetWStringLength((PWSTR)path, MAX_PATH);

    DWORD totalSizeOfFullPathBuffer = (sizeOfBasePath + sizeOfPath + 1) * 2;
    PWSTR fullPathWithBase = reinterpret_cast<PWSTR>(fnAllocate(totalSizeOfFullPathBuffer));
    
    StringCbCatW(static_cast<STRSAFE_LPWSTR>(fullPathWithBase), totalSizeOfFullPathBuffer, basePath);
    StringCbCatW(const_cast<STRSAFE_LPWSTR>(fullPathWithBase), totalSizeOfFullPathBuffer, path);
    return fullPathWithBase;


}

PCWSTR SimpleHttpServer::fnHandleRequest(LPVOID pDataStructure)
{
    PHTTP_REQUEST pRequest = reinterpret_cast<PHTTP_REQUEST>(pDataStructure);
    PCWSTR unicodeData;
    PCSTR renderedResponse;

    switch (pRequest->Verb)
    {
        case HttpVerbGET:
            //HandleGet
            m_lpLoggerFunction(L"[INFO] Got Valid Http Request!");
            unicodeData = fnHandleRequestGet(pRequest);        
            if (NULL == unicodeData)
            {
                m_lpLoggerFunction(L"[ERROR] Could not collect requested data!");
                return NULL;
            }

            renderedResponse = renderUnicodeToByteStrHtml(unicodeData, m_cbRequestMaxSize );
            HeapFree(GetProcessHeap(), 0, (LPVOID)unicodeData);
            return reinterpret_cast<PCWSTR>(renderedResponse);
            


        default:
            m_lpLoggerFunction(L"[ERROR] Got Unrecognized Http Request!");
            return NULL;
    }
}

PCWSTR SimpleHttpServer::fnHandleRequestGet(LPVOID pDataStructure)
{
    PHTTP_REQUEST pRequest = reinterpret_cast<PHTTP_REQUEST>(pDataStructure);
    PCWSTR fullPathToRead = appendToBasePath(
                                m_szServerRootPath,
                                pRequest->CookedUrl.pAbsPath + 1); // ignore first '/'
    PathIdentifier pathReader(fullPathToRead);
    std::wstring massage = std::wstring(L"[INFO] got file/path show request: ")
                            + std::wstring(fullPathToRead);
    m_lpLoggerFunction(massage.c_str());
    PCWSTR pszDataToReturn = pathReader.readNow();
    
    if (NULL != fullPathToRead)
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID)fullPathToRead);
    }

    return pszDataToReturn;
}

void SimpleHttpServer::fnSendResponse(std::wstring sTextToSend, LPVOID referenceRequest)
{
    PHTTP_REQUEST pReferenceRequest = reinterpret_cast<PHTTP_REQUEST>(referenceRequest);
    DWORD bytesSent;
    HTTP_DATA_CHUNK dataChunk;
    HTTP_RESPONSE  response;
    ULONG StatusCode = 200;
    INITIALIZE_HTTP_RESPONSE(&response, StatusCode, "OK");
    ADD_KNOWN_HEADER(response, HttpHeaderContentType, "text/html");

    dataChunk.DataChunkType = HttpDataChunkFromMemory;
    dataChunk.FromMemory.pBuffer = (PSTR)sTextToSend.c_str();
    dataChunk.FromMemory.BufferLength = sTextToSend.size() * 2; 

    response.EntityChunkCount = 1;
    response.pEntityChunks = &dataChunk;

    DWORD result = HttpSendHttpResponse(
        m_RequestQueueHandle,
        pReferenceRequest->RequestId,
        0,
        &response,
        NULL,
        &bytesSent,
        NULL,
        0,
        NULL,
        NULL
    );

    if (NO_ERROR != result)
    {
        m_lpLoggerFunction(L"[ERROR] could not send the repsponse");
    }

    else
    {
        m_lpLoggerFunction(L"[INFO] Response Sent Successfully!");

    }
}


void SimpleHttpServer::logInitializtionMessage()
{
    if (fnSetupHttpServer()
        && fnRegisterUrl(m_szDomainName))
    {
        m_lpLoggerFunction(L"[INFO] Server setup completed!");
        m_lpLoggerFunction(L"[INFO] Listening on:");
        m_lpLoggerFunction(m_szDomainName);
        isInitializedSuccessfully = TRUE;
    }

    else {
        m_lpLoggerFunction(L"[ERROR] Could not setup server");
        isInitializedSuccessfully = FALSE;
    }
    
}



PCSTR renderUnicodeToByteStrHtml(PCWSTR szOriginalUnicodeMessage, size_t resultMaxSize)
{

    CHAR szHtmlTemplate[] = "<div class=\"text\"><pre>%ws</pre></div>";
    DWORD cbSizeOfRenderedMessage =
        fnGetWStringSize(szOriginalUnicodeMessage, resultMaxSize)
        + sizeof(szHtmlTemplate) + 1;

       
    PCHAR rendered = reinterpret_cast<PCHAR>(fnAllocate(cbSizeOfRenderedMessage));
    if (NULL == rendered || NULL == szOriginalUnicodeMessage)
    {
        return NULL;
    }

    StringCbPrintfA(rendered, cbSizeOfRenderedMessage, szHtmlTemplate, szOriginalUnicodeMessage);
    return rendered;
}