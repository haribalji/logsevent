// EventSink.cpp
#include "eventsink.h"
using namespace std;
#include<string>
ULONG EventSink::AddRef()
{
    return InterlockedIncrement(&m_lRef);//it iused for increasing the reference count  to the com object
// only one thread can access the m_lRef other thread need to wait which is forced by the cpu
}

ULONG EventSink::Release()
{
    LONG lRef = InterlockedDecrement(&m_lRef);//decrese the com object refernce
    //when count rreached 0 avoid it other wise program clash will happen 
  
//   lRef active references still hold the com object
    if(lRef == 0)
        delete this;
    return lRef;
}

HRESULT EventSink::QueryInterface(REFIID riid, void** ppv)
{
    //  riid it expects the interface id interface which listen to emi
    if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
    {
        *ppv = (IWbemObjectSink *) this; //adding this interface in this ppv interface
        AddRef();//increase the reference
        return WBEM_S_NO_ERROR;//sucess messages
    }
    else return E_NOINTERFACE;//condtion falied error situation
}


HRESULT EventSink::Indicate(
    LONG lObjectCount,//event count
    IWbemClassObject** apObjArray// array of event 
)
{
    for (LONG i = 0; i < lObjectCount; i++)
    {
        printf("Event occurred\n");

        VARIANT vtTarget;
        VariantInit(&vtTarget);//removes the garbage value

        // TargetInstance is the  the object that triggered the event
        HRESULT hr = apObjArray[i]->Get(L"TargetInstance", 0, &vtTarget, nullptr, nullptr);

        if (SUCCEEDED(hr) &&
        // VT_UNKNOWN com bjiect type
            (vtTarget.vt == VT_UNKNOWN ||
             vtTarget.vt == (VT_UNKNOWN | VT_BYREF)))
        {

            // vtTarget can store the object in two different ways:Directly,by reference


                            //    if it is by reference 
            IUnknown* pUnk = (vtTarget.vt & VT_BYREF) ? *vtTarget.ppunkVal: vtTarget.punkVal;

                // extracting the object 

            IWbemClassObject* pProc = nullptr;//it is used to wmi object

            // convert genneric objec into wmi class object
            hr = pUnk->QueryInterface(
                IID_IWbemClassObject,
                (void**)&pProc);

            if (SUCCEEDED(hr))
            {
                VARIANT vtName;
                VariantInit(&vtName);
                    pProc->Get(L"Name", 0, &vtName, nullptr, nullptr);
//0 indicate just give the value
//null i donnot want the property data type
//null i did not  want rules of the property ,read only
                // if (SUCCEEDED(
                //     vtName.vt == VT_BSTR)
                // {
                    // std::wcout << L"Process started: "
                    //            << vtName.bstrVal << endl;
                    //                   std::wcout <<"process id:"
                    //            << vtName.lVal << endl<<"memory size"<<vtName.ulVal
                    //            <<endl<<"date :";
               
VARIANT vtPid;//it can values of different type used by the com
VariantInit(&vtPid);

pProc->Get(L"ProcessId", 0, &vtPid, 0, 0);
//0 indicate just give the value
//0 i donnot want the property data type
//0 i did not  want rules of the property ,read only
                // if (SUCCEEDED(
    
// if (vtPid.vt == VT_UI4)
// {
//     std::wcout << L"Process ID: "
//                << vtPid.ulVal << std::endl;
// }


VARIANT vtMem;
VariantInit(&vtMem);

pProc->Get(L"WorkingSetSize", 0, &vtMem, 0, 0);


        
VARIANT vtDate;
VariantInit(&vtDate);

pProc->Get(L"CreationDate", 0, &vtDate, 0, 0);


FILE* fp = nullptr;
_wfopen_s(&fp, L"process_events.txt", L"a+, ccs=UTF-8");

if (fp)
{
    fwprintf(fp, L"\n");
    fwprintf(fp, L"Event occurred\n");
    fwprintf(fp, L"Process started: %s\n", vtName.bstrVal);
    fwprintf(fp, L"Process ID     : %lu\n", vtPid.ulVal );
    fwprintf(fp, L"Memory size    : %llu bytes\n", vtMem.bstrVal);

wstring time = vtDate.bstrVal;//as windows use wide unicode string,so we define wstring 
int year   = stoi(time.substr(0, 4));
int month  = stoi(time.substr(4, 2));
int day    = stoi(time.substr(6, 2));

fwprintf(fp,L"Date: %02d/%02d/%04d \n",day, month, year);

    // fwprintf(fp, L"Creation date  : %s\n",vtDate.bstrVal );
    fwprintf(fp, L"\n");

    fclose(fp);
}

                VariantClear(&vtPid);//it frees the data persent init 
                   VariantClear(&vtMem);      
                   VariantClear(&vtDate);
   
// }
                VariantClear(&vtName);
                pProc->Release();//releae the wmi object
            }
        }

        VariantClear(&vtTarget);
    }

    return WBEM_S_NO_ERROR;//scess message
}
HRESULT EventSink::SetStatus(
            /* [in] */ LONG lFlags,//gives the staus code
            /* [in] */ HRESULT hResult,//indicate the sucess or falure 
            /* [in] */ BSTR strParam,//staus messagec or error  like access denied while executing query

            /* [in] */ IWbemClassObject __RPC_FAR *pObjParam//it is also used for getting the details of spcific data status related 
        )
{
    if(lFlags == WBEM_STATUS_COMPLETE)
    {
        printf("%d\n",lFlags);//sucess message of completion wmi process

        printf("Call complete. hResult = 0x%X\n", hResult);
    }
    else if(lFlags == WBEM_STATUS_PROGRESS)//status in progress
    {
        printf("Call in progress.\n");
    }
    return WBEM_S_NO_ERROR;
}    // end of EventSink.cpp