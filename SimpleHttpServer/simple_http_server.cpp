#include <time.h>
#include<stdlib.h>
#include <string>
#include <strsafe.h>
#include "simple_http_server.h"
#include "path_identifier.h"
#include <strsafe.h>

PSTR renderUnicodeToByteStrHtml(PCWSTR originalUnicodeMessage);


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



PCWSTR formatDomainName(PCWSTR szDomainName, DWORD port)
{
    const DWORD cbSizeToHoldPortString = 5 * 2 ;
    const PCWSTR pszDomainFormat = L"%ws:%d/";
    const DWORD cbSizeToHoldFullDomainName = lstrlenW(szDomainName)  * 2
                                             + cbSizeToHoldPortString
                                             + 1;
    STRSAFE_LPWSTR pszDomainString = (STRSAFE_LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbSizeToHoldFullDomainName); // up to d digits for port
    StringCbPrintfW(pszDomainString, cbSizeToHoldFullDomainName, pszDomainFormat, szDomainName, port);
    return pszDomainString;
}

SimpleHttpServer::SimpleHttpServer(PCWSTR szDomainName, DWORD dwPort, const PWSTR szServerRootPath, fpLogger lpLoggerFunction)
	:m_dwPort(dwPort),
	m_szDomainName(formatDomainName(szDomainName, dwPort)),
    m_lpLoggerFunction(lpLoggerFunction),
    m_DefaultMessage(L"Not Found."),
    m_cbRequestMaxSize(2048),
    m_serverRootPath(szServerRootPath)
{
    if (fnSetupHttpServer() 
        && fnRegisterUrl(m_szDomainName))
    {
        m_lpLoggerFunction(L"[INFO] Server setup completed!");

    }
    else {
        m_lpLoggerFunction(L"[ERROR] Could not setup server");
    }
}

SimpleHttpServer::~SimpleHttpServer()
{
	CloseHandle(m_RequestQueueHandle);
    HttpTerminate(HTTP_INITIALIZE_SERVER,
        NULL);
    if (NULL != m_szDomainName)
        HeapFree(GetProcessHeap(), 0, (LPVOID)m_szDomainName);
}



void SimpleHttpServer::fnStart()
{
    ULONG result;
    ULONG cbBytesReadFrom;
    DWORD cbSizeOfRequestBuffer = sizeof(HTTP_REQUEST) + m_cbRequestMaxSize;
    PHTTP_REQUEST lpRequestBuffer = (HTTP_REQUEST*)HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        cbSizeOfRequestBuffer);

    lpRequestBuffer->RequestId = HTTP_NULL_ID;

    while(TRUE)
    {
       result = HttpReceiveHttpRequest(
            m_RequestQueueHandle,
            NULL,//figuree out
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
                fnSendResponse(m_DefaultMessage, lpRequestBuffer);
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

BOOL SimpleHttpServer::fnRegisterUrl(PCWSTR szUrl) noexcept
{
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

PSTR renderUnicodeToByteStrHtml(PCWSTR originalUnicodeMessage)
{
    char htmlTemplate[60] = "<div class=\"text\"><pre>%ws</pre></div>";
    DWORD cbSizeOfRenderedMessage =
        lstrlenW(originalUnicodeMessage) * 2
        + strlen(htmlTemplate) + 1;
    
    PCHAR rendered = (PCHAR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 
        cbSizeOfRenderedMessage);
    if (NULL == rendered || NULL == originalUnicodeMessage)
    {
        return NULL;
    }

    StringCbPrintfA(rendered, cbSizeOfRenderedMessage, htmlTemplate, originalUnicodeMessage);
    return rendered;
}

PCWSTR appendToBasePath(PCWSTR basePath, PCWSTR path)
{
    DWORD sizeOfPath; 
    DWORD sizeOfBasePath;
    StringCchLengthW(basePath, MAX_PATH, (size_t*)&sizeOfBasePath);
    StringCchLengthW(basePath, MAX_PATH, (size_t*)&sizeOfPath);

    DWORD totalSizeOfFullPathBuffer = (sizeOfBasePath + sizeOfPath + 1) * 2;
    PCWSTR fullPathWithBase = (PCWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, totalSizeOfFullPathBuffer);
    StringCbCatW((STRSAFE_LPWSTR)fullPathWithBase, totalSizeOfFullPathBuffer, basePath);
    StringCbCatW((STRSAFE_LPWSTR)fullPathWithBase, totalSizeOfFullPathBuffer, path);
    return fullPathWithBase;


}

PCWSTR SimpleHttpServer::fnHandleRequest(LPVOID pDataStructure)
{
    PHTTP_REQUEST pRequest = (PHTTP_REQUEST)pDataStructure;
    PCWSTR unicodeData;
    PSTR renderedResponse;

    switch (pRequest->Verb)
    {

        case HttpVerbGET:
            //HandleGet
            m_lpLoggerFunction(L"[INFO] Got Valid Http Request!");
            unicodeData = fnHandleRequestGet(pRequest);        
            if (NULL == unicodeData)
            {
                return NULL;
            }

            renderedResponse = renderUnicodeToByteStrHtml(unicodeData);
            HeapFree(GetProcessHeap(), 0, (LPVOID)unicodeData);
            return (PCWSTR)renderedResponse;
            


        default:
            m_lpLoggerFunction(L"[ERROR] Got Unrecognized Http Request!");
            return NULL;
    }
}

PCWSTR SimpleHttpServer::fnHandleRequestGet(LPVOID pDataStructure)
{
    PHTTP_REQUEST pRequest = (PHTTP_REQUEST)pDataStructure;
    //std::wstring absPath(pRequest->CookedUrl.pAbsPath + 1
                         //,pRequest->CookedUrl.AbsPathLength);
    PCWSTR fullPathToRead = appendToBasePath(m_serverRootPath, pRequest->CookedUrl.pAbsPath + 1);
    PathIdentifier pathReader(fullPathToRead);

    std::wstring massage = std::wstring(L"[INFO] got file/path show request: ") + std::wstring(fullPathToRead);
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
    PHTTP_REQUEST pReferenceRequest = (PHTTP_REQUEST)referenceRequest;
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
        m_RequestQueueHandle,           // ReqQueueHandle
        pReferenceRequest->RequestId, // Request ID
        0,                   // Flags
        &response,           // HTTP response
        NULL,                // pReserved1
        &bytesSent,          // bytes sent  (OPTIONAL)
        NULL,                // pReserved2  (must be NULL)
        0,                   // Reserved3   (must be 0)
        NULL,                // LPOVERLAPPED(OPTIONAL)
        NULL                 // pReserved4  (must be NULL)
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
