#define _WIN32_DCOM
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <string>

#pragma comment(lib, "wbemuuid.lib")

using namespace std;

int main()
{
    HRESULT hres;
    // 1 initialize com for the current thread 
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        // cout << "COM init failed\n";
                cout << "Failed to initialize COM library. Error code = 0x" 
             << hex << hres << endl;
        return 1;
    }

    // 2 Initialize security
    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );

    if (FAILED(hres)) {
        cout << "Security init failed\n";
        CoUninitialize();
        return 1;
    }

    // 3 Create IWbemLocator
    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (LPVOID*)&pLoc
    );

    // 4 Connect to WMI
    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL, NULL, 0,
        NULL, 0, 0,
        &pSvc
    );

    // 5 Set proxy 
    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );

    // 6 Execute query
    IEnumWbemClassObject* pEnumerator = NULL;//it is iterator that point to the result set which hold the collection of logs
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),//it is  wmi query language
        bstr_t("SELECT * FROM Win32_NTLogEvent WHERE Logfile='Application'"),
        WBEM_FLAG_FORWARD_ONLY //saying the enumerator to move in forward direction only not in backward direection or random access
        |WBEM_FLAG_RETURN_IMMEDIATELY,//return the contol to main thread the data can come whe next() is executed
        NULL,//maintain the default wmi settings
        &pEnumerator//making this pointer to the result set
    );


if (FAILED(hres))
    {
        printf("ExecQuery failed "
            "with = 0x%X\n", hres);
        pSvc->Release();//connection to wmi released
        pLoc->Release();//wmi locator com object is released
         CoUninitialize();    //curent thread of com is relaesed
        return 1;
    }

    // 7 read results
    IWbemClassObject* pObj = NULL;//it is used to hold the wmi object
    ULONG uReturn = 0;//it is count variable, tracking the no of element retured by the next
FILE* fp = nullptr;
_wfopen_s(&fp, L"win32eventlogs.txt", L"a+, ccs=UTF-8");

    while (pEnumerator) {
        // next method is used to get the next available object
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pObj, &uReturn);
        // 1) WBEM_INFINITE wait untill the data is avaiable 
         // 2) fetching one object at a time
         //3)&pObj here only object is holded
        //4)&uReturn no object returned usually 0 or 1


        if (uReturn == 0) break;//not returned any object
if (fp)
{
        VARIANT vt;//it is data type that is used to hold different data type values used by wmi
// uintVal it unsignes 0 ond +ve number 32 bit which 4 bytes 
         //fwprintf(fp, L"\n");
          //fwprintf(fp, L"Event occurred\n");


       pObj->Get(L"EventIdentifier", 0, &vt, 0, 0);
        wcout << L"Event ID: " << vt.uintVal << endl;
   fwprintf(fp, L" Event ID: %lu\n", vt.uintVal);

        VariantClear(&vt);

        pObj->Get(L"SourceName", 0, &vt, 0, 0);
        wcout << L"Source: " << vt.bstrVal << endl;
           fwprintf(fp, L"  Source: %s\n", vt.bstrVal);

        VariantClear(&vt);

        pObj->Get(L"Logfile", 0, &vt, 0, 0);
wcout << L"Log: " << vt.bstrVal << endl;
   fwprintf(fp, L" Logfile : %s\n", vt.bstrVal);

VariantClear(&vt);
pObj->Get(L"TimeGenerated", 0, &vt, 0, 0);
// wcout << L"TimeGenerated: " << vt.bstrVal << endl;

wstring cimTime = vt.bstrVal;

int year   = stoi(cimTime.substr(0, 4));
int month  = stoi(cimTime.substr(4, 2));
int day    = stoi(cimTime.substr(6, 2));

fwprintf(fp,
    L"Date: %02d/%02d/%04d \n",
    day, month, year
);
wcout << L"Date: "
      << day << L"/" << month << L"/" << year << L" "
      << endl;



//    fwprintf(fp, L"TimeGenerated: %s\n", vt.bstrVal);

VariantClear(&vt);
pObj->Get(L"Type", 0, &vt, 0, 0);
wcout << L"Type: " << vt.bstrVal << endl;
fwprintf(fp, L"Type: %s\n", vt.bstrVal);

    VariantClear(&vt);
    fwprintf(fp, L"\n");
        wcout << L"\n";//it is wide character output string 

        pObj->Release();
    }
}
    // 8 Cleanup
    fclose(fp);
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return 0;
}

// cl /EHsc fetchevemtusingwin32.cpp wbemuuid.lib ole32.lib
// fetchevemtusingwin32



// Win32_NTLogEvent is a WMI class that represents Event Viewer records as WMI objects. When a query is executed, WMI invokes the Event Log WMI provider, which retrieves the event data from the Windows Event Log service and returns it as Win32_NTLogEvent objects.