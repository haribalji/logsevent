#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <winevt.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "httplib.h"
using namespace std;
#define MAX_LOG_SIZE 2048 // 2 KB
FILE *fp = nullptr; // at intially
int logIndex = 1;
wchar_t currentLogFile[MAX_PATH]; // it is array of characters of unicode (wide character string) of 260 in windows
// used by windows that it will identifes the value based on their unicode
#pragma comment(lib, "wevtapi.lib")

bool upload_and_delete(const string& zipPath) {

    httplib::Client cli("localhost", 8080); 
//here creating the http client object and this object knows where to send
// request



     ifstream file(zipPath,  ios::binary);// ifstream  is input file stream
    //  that is used to get the data from the file and ios::binary it indicating the 
    // ifstream to read the data as bytes,10010101 01101010
          wprintf(L"Channel s was not found2.\n");

    if (!file) return false;//if the file is falied to open it will be false,successfully done then it will 
    // then it is ture

     string file_content(
        ( istreambuf_iterator<char>(file)),
         istreambuf_iterator<char>()
    );
    //here the entire file content is read as bytes 
//istreambuf_iterator it is just a file iterator that read file content byte by byte
//start from the begining
file.close();


     string boundary = "----CppBoundary123456";
    //it is used to seperate the bytes  that is going in the http 
    // eg:- from where the  image  start and end

    //-----   --> not nesscary but using it is a best practice and this 12345 also for best pratice ,the http will not parse it

     string body =
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data;name=\"file\"; filename=\""+//it is used define each part of the formadata like it tells this part is a file, its form field name, and its filename
         filesystem::path(zipPath).filename().string() + "\"\r\n"         //here we are just extractinng the file name from the path object(D:/logs/archive/events.zip)from here we are extracting the events.zip and converting them to string
        "Content-Type: application/zip\r\n\r\n" +               //it says what kind of bytes  that we are sending
        file_content + "\r\n"                 //here passing the content of the file
        "--" + boundary + "--\r\n";

    httplib::Headers headers = {  //here just passing the header
        { "Content-Type", "multipart/form-data; boundary=" + boundary }//here we passing the that requst body is mulitpart and inoder to differeitate the data we the boundary //this boundary can be used by the tomcat to differeitate the data  
    };

    auto res = cli.Post("/server1/upload", headers, body, "multipart/form-data");
    //here we sending the data

    if (res && res->status == 200) {

         filesystem::remove(zipPath);//it delete the file from this give path

        return true;
    }
   wprintf(L"isssus is ther%d",res->status);

    return false;
}

DWORD WINAPI SubscriptionCallback(EVT_SUBSCRIBE_NOTIFY_ACTION action, PVOID pContext, EVT_HANDLE hEvent);
DWORD PrintEvent(EVT_HANDLE hEvent);
long GetFileSize(const wchar_t *filename)
{
    // here only we check the size of the file
    struct _stat st;
    // it is a structure that  defines the details of the file ,which has various field
    // to define the  file
    if (_wstat(filename, &st) == 0) // as it need to accept the wide charater string unicode
        // it is function that talks to  os about the details of the file  and store in st
        return st.st_size; // getting the size of the os
    return 0;

    // _wstat in order to work with  wide character unicode string
}

void OpenNewLogFile()
{
    if (fp) // already open means close , it will be helpful when making  the zip file the file shouls be close
        fclose(fp);

    swprintf(currentLogFile, MAX_PATH, L"SubscribeToFutureEvents%d.txt", logIndex++);
    // here building the new file

    _wfopen_s(&fp, currentLogFile, L"a+, ccs=UTF-8");

    // then based on the current file name the file will be opened
}

void ZipLogFile(const wchar_t *filename)
{
    // const usecase file name will not  be changed
    wchar_t zipCmd[1024]; // it will hold the powershell command text

    // swprintf it is just used for formating the string
    swprintf(
        zipCmd, // to hold the string
        1024,   // size of the string
        L"powershell -command \"Compress-Archive -Force '%s' '%s.zip'\"",
        filename,
        filename);
    // eg:-powershell -command "Compress-Archive -Force 'SubscribeToFutureEvents1.txt' 'SubscribeToFutureEvents1.txt.zip'"
    // sourec file also kept and zip file also avaiable
    //  -Force if the file is already exists then overwrite it
    _wsystem(zipCmd); // this command will be excuted by the windows
    // here cmd launched first then powershell begin then it will execute the
    //  commad and retun control to the application

    // powershell read the data and flush it


  wstring zipPathW = wstring(filename) + L".zip";//zip file name
  string zipPath(zipPathW.begin(), zipPathW.end());
//in the above process we are converting the wide character of string 
//into character of string 

   if( upload_and_delete(zipPath)){
                 wprintf(L"all ok\n");
   }
   else{
       wprintf(L"zip file deletion is failed\n");

   }
    
}

void main(void)
{
    // ZipLogFile(L"a");
    // upload_and_delete("d");
    DWORD status = ERROR_SUCCESS;
    EVT_HANDLE hSubscription = NULL;
    // LPWSTR pwsPath = L"<channel name goes here>";
    // LPWSTR pwsQuery = L"<xpath query goes here>";

    LPWSTR pwsPath = L"System"; // name of the channel from here only we fetch the data
    LPWSTR pwsQuery = NULL;     // In-order to export all the events we specify it to null //i needed event need to fetch then specify xpath expression

    // Subscribe to events beginning with the oldest event in the channel. The subscription
    // will return all current events in the channel and any future events that are raised
    // here i am fetching  the future events
    // while the application is active.
    hSubscription = EvtSubscribe(NULL, NULL, pwsPath, pwsQuery, NULL, NULL,(EVT_SUBSCRIBE_CALLBACK)SubscriptionCallback, EvtSubscribeToFutureEvents);

    // 1) as we fetch the data from local computer
    // 2)SignalEvent is used by Windows to notify your application
    //  that some events matching your subscription or interest have
    // occurred  ,you can now try to get it,used in pull model
    // but here we are not using it beacues EvtSubscribe is implemented using
    // push model so callback is used so we kept that area null , as windows push the event automtically to my  callback

    // 5) here i am not using any bookmark to fetch the data from any particualr event so we kept this area null
    // 6) context -->it is just a userdefined data passing to windows and windows passing this to callbacks
    // and also callbacks need to know who is calling it usually we pass the file pointer to it
    //  but here we are not passing anything to so null

    // casting takes place, just we want to pass the pointer of callback function to windows, so that windows event log service know to
    // call which callback function

    // EvtSubscribeToFutureEvents here i am jsut indicating that i need only future event data ato me

    // hSubscription it is event object will hold the channael and query,and callback details it will not hold the data

    if (NULL == hSubscription)
    {
        // if subsciption not done sucessfully we get null
        status = GetLastError();

        if (ERROR_EVT_CHANNEL_NOT_FOUND == status)
            wprintf(L"Channel %s was not found.\n", pwsPath);
        else if (ERROR_EVT_INVALID_QUERY == status)
            // You can call EvtGetExtendedStatus to get information as to why the query is not valid.
            wprintf(L"The query \"%s\" is not valid.\n", pwsQuery);
        else
            wprintf(L"EvtSubscribe failed with %lu.\n", status);

        goto cleanup;
    }

    wprintf(L"Hit any key to quit\n\n");
    while (!_kbhit())
        Sleep(10); // to prevent  cpu utilization ,if not use cpu will continuous runing waste of cpu usage
    // _kbhit() only checks whether a key has been pressed on the keyboard.  pressed means true ,otherwise false
    // if yes then  stop the program
    // if not still run the program

    // it helps us to run the program untill user not pressed any key

cleanup:

    if (hSubscription)
        EvtClose(hSubscription);
}

// The callback that receives the events that match the query criteria.
DWORD WINAPI SubscriptionCallback(EVT_SUBSCRIBE_NOTIFY_ACTION action, PVOID pContext, EVT_HANDLE hEvent)
{

    // WINAPI specifies the calling rules that our function follows
    // so it matches the rules Windows uses when calling it.
    // it tells the compiler to let  Windows pass the parameters, call the function,
    //  and clean the stack. if we do not specify WINAPI, the calling conventions may mismatch,
    // and although Windows will still call the function,lead to crashes sometimes
    // WINAPI it indicates to the complier
    // if we do not pass the WINAPI then function will clear its stack by itself that lead to carsh sometimes

    // action it is message code indicate or saying why i called this callback i here windows
    // 0-->error in getting data
    // EvtSubscribeActionDeliver-->it indicate that you got the event that matched your query

    // UNREFERENCED_PARAMETER(pContext);// just saying that i am not using it
    DWORD status = ERROR_SUCCESS;

    switch (action)
    {
    // You should only get the EvtSubscribeActionError action if your subscription flags
    // includes EvtSubscribeStrict and the channel contains missing event records.
    case EvtSubscribeActionError:
        if (ERROR_EVT_QUERY_RESULT_STALE == (DWORD)hEvent) // extracting the value in a 32 bit number
        {
            // when the event is not available and the callback is called with an error notification, hEvent
            //  contains a Win32 error code instead of an event handle.
            wprintf(L"The subscription callback was notified that event records are missing.\n");
            // Handle if this is an issue for your application.
        }
        else
        {
            // if any other kind of reason for the faliure
            wprintf(L"The subscription callback received the following Win32 error: %lu\n", (DWORD)hEvent);
        }
        break;
    case EvtSubscribeActionDeliver: // this is case when we  receive the data
        if (ERROR_SUCCESS != (status = PrintEvent(hEvent)))
        {
            goto cleanup;
        }
        break;

    default:
        wprintf(L"SubscriptionCallback: Unknown action.\n");
    }

cleanup:

    if (ERROR_SUCCESS != status)
    {
        // End subscription - Use some kind of IPC mechanism to signal
        // your application to close the subscription handle.
        EvtClose(hEvent);
    }

    return status; // The service ignores the returned status.
}

// Render the event as an XML string and print it.
DWORD PrintEvent(EVT_HANDLE hEvent)
{ 
    DWORD status = ERROR_SUCCESS;
    DWORD dwBufferSize = 0;
    DWORD dwBufferUsed = 0;
    DWORD dwPropertyCount = 0;
    LPWSTR pRenderedContent = NULL;
    PEVT_VARIANT values = (PEVT_VARIANT)malloc(dwBufferSize);

    LPCWSTR paths[] = // it is just collection of xpath expression only this data will be fetched from event data
        {
            L"Event/System/EventID",
            L"Event/System/Level",
            L"Event/System/Provider/@Name",
            L"Event/System/Execution/@ProcessID"};
    EVT_HANDLE hContext = EvtCreateRenderContext(
        _countof(paths), // no of elements in array
        paths,
        EvtRenderContextValues // rendering specific properties from the event
    );

    if (!EvtRender(hContext, hEvent, EvtRenderEventValues, dwBufferSize, NULL, &dwBufferUsed, &dwPropertyCount))
    {
        // first call to get the dwBufferUsed it will be got from the  windows
        if (ERROR_INSUFFICIENT_BUFFER == (status = GetLastError()))
        {
            dwBufferSize = dwBufferUsed;
            // pRenderedContent = (LPWSTR)malloc(dwBufferSize);
             values = (PEVT_VARIANT)malloc(dwBufferSize);
            // PEVT_VARIANT it is used  to represent all the fields values of the event
            if (values)
            {
                EvtRender(hContext, hEvent, EvtRenderEventValues, dwBufferSize, values, &dwBufferUsed, &dwPropertyCount);
                // dwPropertyCount it says no of property count it returned

                wprintf(L"EventID     : %u\n", values[0].UInt16Val);//it is 16 bit
                wprintf(L"Level       : %u\n", values[1].ByteVal); // it is 8bit 0 and +ve values
                wprintf(L"Provider    : %s\n", values[2].StringVal);
                wprintf(L"Execution Processid: %u\n\n", values[3].UInt32Val); // exactly 32 bits wide

                if (!fp)
                {
                    // if it is null means
                    OpenNewLogFile();
                }

                // Write event
                fwprintf(fp, L"EventID     : %u\n", values[0].UInt16Val);
                fwprintf(fp, L"Level       : %u\n", values[1].ByteVal);//it is 
                fwprintf(fp, L"Provider    : %s\n", values[2].StringVal);
                fwprintf(fp, L"Execution ProcessID : %u\n\n", values[3].UInt32Val);

                fflush(fp);//forcing the process of writing the content form from buffer to disk

                // Check size
                if (GetFileSize(currentLogFile) >= MAX_LOG_SIZE)
                {
                    fclose(fp);
                    fp = nullptr;
                    // wprintf(L"NEW FILE CREATION TAKES PLACE\n");
                    ZipLogFile(currentLogFile);
                    OpenNewLogFile();
                }
            }
            else
            {
                wprintf(L"malloc failed\n"); //;//if not able to allcate the meomry then it is null
                status = ERROR_OUTOFMEMORY;
                goto cleanup;
            }
        }


    }


cleanup:

    if (values)
        free(values);

    return status;
}

// cl /EHsc eventsubscribe.cpp wevtapi.lib
