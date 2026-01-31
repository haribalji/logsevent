#include <windows.h>
#include <stdio.h>
#include <winevt.h>
#pragma comment(lib, "wevtapi.lib")//libary for windows event log api

// comment  function tells the msvc compliere to pass the message to linkerto add this library
// pragma.it is used to give the instruction to complier
#define ARRAY_SIZE 10//it is the constant value that used whereever it is specified its name and cannot changed
DWORD DumpEvents(LPCWSTR pwsLogFile);
DWORD PrintResults(EVT_HANDLE hResults);
DWORD PrintEvent(EVT_HANDLE hEvent);
void main(void)
{
    DWORD status = ERROR_SUCCESS;
    // LPWSTR pPath = L"<path to channel goes here>";
    LPWSTR pPath = L"System";//name of the channel from here only we fetch the data
    LPWSTR pQuery = NULL;//In-order to export all the events we specify it to null
    LPWSTR pTargetLogFile = L".\\log.evtx";//load the data here in this location file
DeleteFileW(pTargetLogFile); //delete the file if it already exists

    // Export all the events in the specified channel to the target log file.
    if (!EvtExportLog(NULL, pPath, pQuery, pTargetLogFile, EvtExportLogChannelPath))
    {
// NULL-->getting the data from the local computer itself
// 4th parameter indicating from where the log file will come
        wprintf(L"EvtExportLog failed for initial export with %lu.\n", GetLastError());
        goto cleanup;
    }

    // Dump the events from the log file.
    wprintf(L"Events from %s log file\n\n", pTargetLogFile);
    
//getting the events related data from the log file
    DumpEvents(pTargetLogFile);

    // Create a new log file that will contain all events from the specified 
    // log file where the event ID is 2.
    pPath =  L".\\log.evtx";
    pQuery = L"Event/System[EventID=2]";//it is XPath 1.0 expression
    pTargetLogFile = L".\\log2.evtx";
DeleteFileW(pTargetLogFile);

    // Export all events from the specified log file that have an ID of 2 and
    // write them to a new log file.
    if (!EvtExportLog(NULL, pPath, pQuery, pTargetLogFile, EvtExportLogFilePath))
    {
        wprintf(L"EvtExportLog failed for relog with %lu.\n", GetLastError());
        goto cleanup;
    }

    // Dump the events from the log file.
    wprintf(L"\n\n\nEvents from %s log file\n\n", pTargetLogFile);
    DumpEvents(pTargetLogFile);

cleanup:
    return;
}
// Dump all the events from the log file.
DWORD DumpEvents(LPCWSTR pwsPath)
{
    EVT_HANDLE hResults = NULL;
    DWORD status = ERROR_SUCCESS;

    hResults = EvtQuery(NULL, pwsPath, NULL, EvtQueryFilePath); // 1-->here we are getting the data from log file
//   hResults = EvtQuery(NULL,L"System", NULL, EvtQueryChannelPath); // 2-->here we are getting the data from channel ,EvtQueryChannelPath-->specify that getting the data from one or more channel
       

// params purpose
// 1) it says whether it is collecting data from remote or from the 
        // local computer  NULL->it indcates the local computer
        // 2)it indicates the  path from where the data is fetched the file can evt,evtx,etl 
        // it can be the (channe then specify the channel name) also ,then this path can be specefied by xml format also
        //3) you need to specify the types of event (which act as filter
        //like errors,(or) login events)
        //NULL or * it says we need to get all the types of events
        //4)the order of data need to be fetched
        //it specify the order in which the data need to be fetched 
        // like old to new or new to old 
        // here we are specified that data is fetched from 1 or more log files
        // it can be one or more channel also ,if file  means specify path
        // if channel means specify the name of the channel
        // event tolerate query is used or try to execute the valid part of the path
  
  
  
  
  
        if (NULL == hResults)
    // if query fails it will be executed
    {
        // getting the last current thread error
        wprintf(L"EvtQuery failed with %lu.\n", status = GetLastError());
        goto cleanup;
    }
    status = PrintResults(hResults);
cleanup:

    if (hResults)
        EvtClose(hResults);

    return status;
}
// getting all the events in the result set. 
DWORD PrintResults(EVT_HANDLE hResults)
{
    //   DWORD--> ACCEPT  Zero and positive values only
    DWORD status = ERROR_SUCCESS;
    EVT_HANDLE hEvents[ARRAY_SIZE];  //EVT_HANDLE events[10] then it can hold by -->PEVT_HANDLE Events = events it is used to receive the collection handle event object
    DWORD dwReturned = 0;

    while (true)
    {
        // Get a block of events from the result set.
        if (!EvtNext(hResults, ARRAY_SIZE, hEvents, INFINITE, 0, &dwReturned))
        {
            
            
            
            
//it will come inside  if function return false 
// ARRAY_SIZE --> to retrieve this number of elements from the result 
// INFINITE---> it indicate that i am waiting till all the events get retrived,
// if you specify the time til that time only it will wait, after the the time expire
// the last error is set to ERROR_TIMEOUT. for the current thread
//0--> it  is the flag value reserved for purpose
// dwReturned how many events the returuned by the evtnext
// this value will be wrtie by evtnext
// evtnext wrtie the event in hevents


            if (ERROR_NO_MORE_ITEMS != (status = GetLastError()))
            // GetLastError it error code of the error caused by current thread 

            {
                wprintf(L"EvtNext failed with %lu\n", status);
            }

            goto cleanup;
        }




        // For each event, call the PrintEvent function which renders the
        // event for display. PrintEvent is shown in RenderingEvents.
        for (DWORD i = 0; i < dwReturned; i++)
        {
            if (ERROR_SUCCESS == (status = PrintEvent(hEvents[i])))
            // as we also need to catch the erroe code
            {
                // after sucessfuly rendering just closing the event
                EvtClose(hEvents[i]);
                hEvents[i] = NULL;
            }
            else
            {
                goto cleanup;
            }
        }
    }
cleanup:
    // executed only if there was an error.
    for (DWORD i = 0; i < dwReturned; i++)
    {
        // closing all the event which not null
        if (NULL != hEvents[i])
            EvtClose(hEvents[i]);
    }

    return status;
}

DWORD PrintEvent(EVT_HANDLE hEvent)
{
    DWORD status = ERROR_SUCCESS;//it is just the sucess code which is  0
    DWORD bufferSize = 0;//to tell windows how much space is available
    DWORD bufferUsed = 0;//
    DWORD propertyCount = 0;
    LPWSTR buffer = NULL;

    // EvtRender it is used to get the event data in xml
    // First call to get buffer size
LPCWSTR paths[] =//it is just collection of xpath
{
    L"Event/System/EventID",
    L"Event/System/Level",
    L"Event/System/Provider/@Name",
    L"Event/System/Execution/@ProcessID"
};
EVT_HANDLE hContext = EvtCreateRenderContext(
    _countof(paths),//no of elements in array
    paths,
    EvtRenderContextValues//rendering specific properties from the event
);
if (!EvtRender(hContext, hEvent, EvtRenderEventValues,0, NULL, &bufferUsed, &propertyCount))
    {
if (GetLastError() != ERROR_INSUFFICIENT_BUFFER){
            wprintf(L"EvtRender failed (%lu)\n", GetLastError());
            return GetLastError();
}
}


  bufferSize = bufferUsed;//how much data can be hold in buffer

//PEVT_VARIANT it is pointer that is used to refer to all the  
// data or fields of the one event  
// EventID,Level,Provider name,ProcessID
// values[0] → EVT_VARIANT(EventID)
// values[1] → EVT_VARIANT(Level)
// values[2] → EVT_VARIANT(Provider)

// values[i].Type --> it is used to identify the type of data
// values[i].StringVal -->here we get the string value
// values[i].UInt32Val number field
// count is used to identify whether it is array or if count>1 means it is array otherwise it is fields



PEVT_VARIANT values = (PEVT_VARIANT)malloc(bufferUsed);

// purpose  malloc
// why mallco is used because windows expect the  plain memory
// no constuctor should be run 
    if (!values)
        return ERROR_OUTOFMEMORY;
EvtRender(
    hContext,
    hEvent,
    EvtRenderEventValues,
    bufferUsed,
    values,
    &bufferUsed,
    &propertyCount
);

wprintf(L"EventID     : %u\n", values[0].UInt16Val);
wprintf(L"Level       : %u\n", values[1].ByteVal);//it is 8bit
wprintf(L"Provider    : %s\n", values[2].StringVal);
wprintf(L"Execution Processid: %u\n\n", values[3].UInt32Val);//exactly 32 bits wide 
// in windows UInt32Val  used because it perfers the fixed size only
// \n\n 
// first \n → go to next line
// second \n → go to another next line

// inserting the data in file
FILE* fp = nullptr;
_wfopen_s(&fp, L"events.txt", L"a+, ccs=UTF-8");
// _wfopen_s it is used in msvc for (_s)secure opening  of file for wide characters
// here we are opening the file in append mode with read and write
// ccs=UTF-8 tells the windows to write the wide character(unicode characters)
// in file in UTF-8 bytes so that it can be readable format in editors
if (fp)
{
    fwprintf(fp, L"EventID     : %u\n", values[0].UInt16Val);
    fwprintf(fp, L"Level       : %u\n", values[1].ByteVal);   // Level is BYTE
    fwprintf(fp, L"Provider    : %s\n", values[2].StringVal);
    fwprintf(fp, L"Execution ProcessID : %u\n\n", values[3].UInt32Val);
    fclose(fp);
}
        free(values);

    return ERROR_SUCCESS;
}
// D:
// D:\zohol3\readlogs>cl /EHsc newfile.cpp wevtapi.lib ole32.lib
// newfile  



// git remote add origin https://github.com/haribalji/logsevent.git
// git branch -M main
// git push -u origin main