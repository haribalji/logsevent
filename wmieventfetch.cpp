#include "eventsink.h"

int main(int iArgCnt, char ** argv)
{
    HRESULT hres;//HRESULT is a 32-bit integer type used by Windows APIs to report success or failure.

    // step 1
    // initialize COM. 
//COM it allows the software components to talk with eachother
    hres =  CoInitializeEx(0, COINIT_MULTITHREADED); //mtaMTA (Multi-Threaded Apartment)
    // hres is a 32-bit value that tells you success, or failure with hexdiaml value
// it creates the multi-threaded apartment which is a container
// where threads will commuicate with each other concurrently and they share the common container
// 1st parms is 0 or null reserved for futuree purpose
// S_OK (0x0) ,S_FALSE (0x1)-->it means already model created it is not falure
    if (FAILED(hres))
    {
// it is macro function
// the faliure maeans the value will be negative


// when a failure occure when thread present in  hres =  CoInitializeEx(0, COINIT_MULTITHREADED); STA (Single-Threaded Apartment)
// that time
// CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
// note thread can be part of one model only mulitiple model is present 
// that time com intialize will be failed 

// STA allows single threads, but only one thread can execute COM calls per apartment at a time.1 thread can call wmi at a time
// MTA allows multiple threads to execute COM calls concurrently.multiple thread can call wmi at a time
        cout << "Failed to initialize COM library. Error code = 0x" 
             << hex << hres << endl;
        return 1;                  // program has failed.
    }
    // hex it tells the output stream that, from now onwards print the 
   // integers in hexodecial format

    // Step 2:
    // Set general COM security levels 

//COM needs security settings to talk to WMI
// WMI will not allow access without proper security contex
    hres =  CoInitializeSecurity(
        NULL,                        //if null means it uses the default windows security we are not customize th security
        -1,                          // we are telling COM to  automatically select the authentication services basic one
        NULL,                        // authentication services,i am not providing please choose it from your side
        NULL,                        // Reserved for future purpose
        RPC_C_AUTHN_LEVEL_DEFAULT,   // default authentication means,just verify the identity, windows choose NTLM/Kerberos for the authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  ,
        NULL,                        // authentication info,use curent window user information
        EOAC_NONE,                   // do not add any extra security ,just follow the default the  windows security 
        NULL                         // Reserved for future purpose
        );

// -1 means automatically select the authentication services
// NTLM,Kerberos
// RPC_C_IMP_LEVEL_IMPERSONATE, // default Impersonation  ,what WMI can do with respect to you like acesses the files
        

    if (FAILED(hres))
    {
        cout << "Failed to initialize security. Error code = 0x" 
             << hex << hres << endl;
        CoUninitialize();
        return 1;                      // Program has failed.
    }
    
    // Step 3: 
    // Obtain the initial locator to WMI 

    IWbemLocator *pLoc = NULL;
//IWbemLocator it is com interface with help of that we talk with wmi
// as wmi is made of the com objects so it can be acceesed by pLoc pointers
    hres = CoCreateInstance(
        CLSID_WbemLocator,     //CLSID_WbemLocator it is a "id" that identifies the wmi locator com class in  windows,inside the the DLL(dynamic link libary), the COM class  code implementation is present.    
                                //so with the help  CLSID_WbemLocator we identify the dll location 
        0,                      //it says that com created as independent  objects no agregation ,which means one com object does not have another com  object
        CLSCTX_INPROC_SERVER, //create the com object in dll
        IID_IWbemLocator, //it is the IWbemLocator interface(to uniquly identify the com interface) id just saying  that  i need this interface from the object you created
        (LPVOID *) &pLoc);
        // in last parma CoCreateInstance creates a COM object and returns an interface(the inerface is IID_IWbemLocator which we mentioned ) pointer ,to that com object , so we prepare a pointer variable (pLoc) to receive it.
 
    if (FAILED(hres))
    {
        cout << "Failed to create IWbemLocator object. "
             << "Err code = 0x"
             << hex << hres << endl;
        CoUninitialize(); //just release the resouces as completed working with com object
        return 1;                 // program has failed.
    }


    // Step 4: 
    // Connect to WMI through the IWbemLocator::ConnectServer method

    IWbemServices *pSvc = NULL;//it is also com interface
 
    // Connect to the local root\cimv2 namespace( namespace it is a folder )
    // this namespace hold the class like Win32_Process running process ,Win32_Service ,Windows services
    // and obtain pointer pSvc to make IWbemServices calls.
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // it is namespace that is used to execute and handle most WMI queries.
        NULL,//considered as current loggined window user //here they ask username
        NULL, //password is null not passing it explicity  that means use the cureent user data
        0, //just saying the default system lanaguage and date format followed  by the system here they speak about system config
        NULL, ///for security--> what com uses,use that itself
        0, //authority=0 means windows decide who need to authicate the user  
        0, //reserved parameter
        &pSvc//IWbemServices ,here we will have interface using which we run quries in wmi 
    );
     
    if (FAILED(hres))
    {
        cout << "Could not connect. Error code = 0x" 
             << hex << hres << endl;
        pLoc->Release();     
        CoUninitialize();
        return 1;                // Program has failed.
    }

    cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


    // Step 5: 
    // Set security levels on the proxy 
// pSvc is a COM proxy for the WMI service.
// your program calls the proxy, and the proxy communicates with the real WMI service on your behalf.
// CoSetProxyBlanket is used to set authentication and impersonation limits on that proxy so it can securely access WMI.
    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy object to which we apply the settings
        RPC_C_AUTHN_WINNT,           // to identify the prxoy we use windows authentication like NTLM or Kerberos decedied by windows
        RPC_C_AUTHZ_NONE,            // from wmi side no seperate authrozation service,it will decide whether to allow or not
        NULL,                        // windows will automatically identify the  server to who proxy is speaking ,server principal name not needed  server --> (WmiPrvSE.exe here wmi runs)
        RPC_C_AUTHN_LEVEL_CALL,      // verify the proxy on every rpc call
        RPC_C_IMP_LEVEL_IMPERSONATE, // setting the limitation for  proxy to act as user to access the rules ,like not sharing our imfo to other user
        NULL,                        // client identity is used here logged in user get from windows
        EOAC_NONE                    // offing the extra security added by proxy just follow
                                    // windows security like, authentication settings you already specified,
    );

    //RPC_C_AUTHZ_NONE--> it has its own security in its namespace which is acl which is access control list which defines who can access the resouces
    //then wmi check whether this proxy user(by holding the windows user data) has the rights to access this resouces,impersonation level you set

    if (FAILED(hres))
    {
        cout << "Could not set proxy blanket. Error code = 0x" 
             << hex << hres << endl;
        pSvc->Release();//relase the connection from wmi
        pLoc->Release();     //release the wmi locator com object
        CoUninitialize();//unintialize com for the current thread  

        return 1;        //program has failed.
    }

    // Step 6: 
    // Receive event notifications 
    // Use an unsecured apartment for security
    IUnsecuredApartment* pUnsecApp = NULL;//it is com interface

     //wmi runs in different process 
    //it sends event back to us, with COM security it will block the events
    //so inorder to handle that event we create seperate contaner in which we run code ot handle event
   //IUnsecuredApartment it is not unsecure (meaning) it is secure only ,WMI already checked its permission to access it ,still you need the token to enter into it

    hres = CoCreateInstance(
        CLSID_UnsecuredApartment, //telling the com to create mentioned class
        NULL, //creating one com object,not embedding another object in it
        CLSCTX_LOCAL_SERVER, //it means created the object in same machine but differet prcess not in my program
        IID_IUnsecuredApartment,//getting this interface from the created object
        (void**)&pUnsecApp);//as the function returns interface pointer of com object
 
    EventSink* pSink = new EventSink;// it is the class and its object receive the event
//pSink it is a com object it's lifetime should be controlled by the reference count only
    pSink->AddRef();//it increae the com refernce count to safely handly the call back
    IUnknown* pStubUnk = NULL; //it is used as middle man it is interfcae belong to com object
    pUnsecApp->CreateObjectStub(pSink, &pStubUnk); //connecting both

    IWbemObjectSink* pStubSink = NULL;//it is listener object that listen to wmi 

    // pStubSink is the WMI listener object.
// It is part of pStubUnk, and it forwards WMI events to pSink.
    pStubUnk->QueryInterface(IID_IWbemObjectSink,
        (void **) &pStubSink);

    // The ExecNotificationQueryAsync method will call
    // The EventQuery::Indicate method when an event occurs
    hres = pSvc->ExecNotificationQueryAsync(
        _bstr_t("WQL"), //WMI Query Language currenty this support this only
        _bstr_t("SELECT * " 
            "FROM __InstanceCreationEvent WITHIN 1 "//checking every one second ,any new process creation is happened or not
            "WHERE TargetInstance ISA 'Win32_Process'"), 
        WBEM_FLAG_SEND_STATUS, //it return the staus message like query successfully registered or failed
        NULL, //i do not send any user defined context (like user defined data)
        pStubSink);//it recevive the event and internally run the  Indicate() method 
//__InstanceCreationEvent when a instance is created it will notify
    // Check for errors.
    //created proccess  is a TargetInstance object  whether it is part of Win32_Process class 
    if (FAILED(hres))
    {
        printf("ExecNotificationQueryAsync failed "
            "with = 0x%X\n", hres);
        pSvc->Release();//connection to wmi released
        pLoc->Release();//wmi locator com object is released
        pUnsecApp->Release();//unsecure appartment resoucres is released
        pStubUnk->Release();////middle man is relased
        pSink->Release();//event reciver is relaesd
        pStubSink->Release();//wmi event listener is released
        CoUninitialize();    //curent thread of com is relaesed
        return 1;
    }

    // Wait for the event

    // Sleep(10000);
    Sleep(INFINITE);

         
    hres = pSvc->CancelAsyncCall(pStubSink);
    // Cleanup
    
    pSvc->Release();
    pLoc->Release();
    pUnsecApp->Release();
    pStubUnk->Release();
    pSink->Release();
    pStubSink->Release();
    CoUninitialize();

    return 0;   // Program successfully completed.
 
}

// cd D:
// cl /EHsc wmieventfetch.cpp eventsink.cpp wbemuuid.lib

// note asynchronous COM callbacks do NOT run on your main thread.it should run com managed thread 
