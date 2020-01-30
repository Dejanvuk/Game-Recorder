#include "stdafx.h"

bool BuildControls(HWND parent) {
	HGDIOBJ hObj = GetStockObject(DEFAULT_GUI_FONT);;
	int x, y, indentareX = 10, indentareY = 40;
	SIZE size;
	RECT rect;
	HDC hdc;
	int iResult;
	HWND test;

	HWND hwndStaticInjector, hwndButtonExe, hwndStaticCmdla, hwndEditCmdla, hwndButtonInjector;

	x = 10;
	y = 10;

	/*  
	*********
	Executable
	*********
	*/
	hwndStaticInjector = CreateWindow( 
    (LPCWSTR)"Static",  // Predefined class; Unicode assumed 
    (LPCWSTR)"",      // Button text 
    WS_CHILD | WS_VISIBLE | WS_TABSTOP,  // Styles 
    x,         // x position 
    y,         // y position 
    60,        // Button width
    40,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
	NULL, 
    NULL);      // Pointer not needed.

	hdc = GetDC(hwndStaticInjector);
	iResult=GetTextExtentPoint32(hdc, (LPCWSTR)"Target executable:", sizeof("Target executable:"), &size);

	if(iResult!=0)
	{
		//SendDlgItemMessage(hwndStaticInjector, 0, SS_REALSIZEIMAGE , size.cx, NULL);
		SetWindowPos(hwndStaticInjector,NULL,x,y,size.cx,size.cy,SWP_SHOWWINDOW);
		SetWindowText(hwndStaticInjector, (LPCWSTR)"Target executable:");
		//SendMessage(hwndStaticInjector, WM_SETFONT, (WPARAM)hObj, true);
	}

	x += size.cx + indentareX;

	hwndButtonExe = CreateWindow( 
    (LPCWSTR)"Button",  // Predefined class; Unicode assumed 
    (LPCWSTR)"Select",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    x,         // x position 
    y,         // y position 
    160,        // Button width
    20,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
    NULL, 
    NULL);      // Pointer not needed.

	x = 10;
	y += indentareY;

	/*  
	**********************
	Command Line arguments 
	**********************
	*/
	hwndStaticCmdla = CreateWindow( 
    (LPCWSTR)"Static",  // Predefined class; Unicode assumed 
    (LPCWSTR)"",      // Button text 
    WS_CHILD | WS_VISIBLE | WS_TABSTOP,  // Styles 
    x,         // x position 
    y,         // y position 
    60,        // Button width
    40,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
	NULL, 
    NULL);      // Pointer not needed.

	hdc = GetDC(hwndStaticCmdla);
	iResult=GetTextExtentPoint32(hdc, (LPCWSTR)"Command line arguments:", sizeof("Command line arguments:"), &size);

	if(iResult!=0)
	{
		//SendDlgItemMessage(hwndStaticInjector, 0, SS_REALSIZEIMAGE , size.cx, NULL);
		SetWindowPos(hwndStaticCmdla,NULL,x,y,size.cx,size.cy,SWP_SHOWWINDOW);
		SetWindowText(hwndStaticCmdla, (LPCWSTR)"Command line arguments:");
		//SendMessage(hwndStaticInjector, WM_SETFONT, (WPARAM)hObj, true);
	}

	x += size.cx + indentareX;

	hwndEditCmdla = CreateWindow((LPCWSTR)"edit", (LPCWSTR)"",
                              WS_CHILD | WS_VISIBLE | WS_TABSTOP
                              | ES_LEFT | WS_BORDER,
                              x, y, 160, size.cy,
                              parent, NULL,
							  NULL, NULL);

	SetWindowText(hwndEditCmdla, (LPCWSTR)"");

	x = 10;
	y += indentareY;

	/*  
	**********************
	Options
	**********************
	*/

	/*  
	**********************
	Injector
	**********************
	*/

	hwndButtonInjector = CreateWindow( 
    (LPCWSTR)"Button",  // Predefined class; Unicode assumed 
    (LPCWSTR)"START",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    x,         // x position 
    y,         // y position 
    160,        // Button width
    20,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
    NULL, 
    NULL);      // Pointer not needed.


	x = 10;
	y += indentareY;



	return true;
}

bool beginEdit(HWND parent) {
	SetWindowPos(parent,HWND_TOP,200,50,1000,1000,SWP_SHOWWINDOW);

	HWND hwndTrack, hwndButtonPlay, hwndButtonPause, hwndButtonFastForward, hwndButtonFastBackward,
		hwndCheckFrame0,hwndCheckFrame1,hwndCheckFrame2,hwndCheckFrame3,hwndCheckFrame4,hwndButtonPreviousFrames,
hwndButtonNextFrames, hwndButtonDelFrames, hwndStaticFrom, hwndEditFrom,hwndStaticTo,hwndEditTo,
hwndEditVideoStatic0, hwndEditVideoStatic1,hwndEditVideoStatic2,hwndEditVideoStatic3,hwndEditVideoStatic4,
hwndEditEdit0,hwndEditListView,hwndButtonOutput,hwndButtonBrowse,hwndButtonMerge;

	// Create the slider
	hwndTrack = CreateWindowEx( 
        0,                               // no extended styles 
        TRACKBAR_CLASS,                  // class name 
       (LPCWSTR)"Trackbar Control",              // title (caption) 
        WS_CHILD | 
        WS_VISIBLE | 
        TBS_AUTOTICKS | 
        TBS_ENABLESELRANGE,              // style 
        50, 550,                          // position 
        720, 30,                         // size 
        parent,                         // parent window 
        NULL,                     // control identifier 
        NULL,                         // instance 
        NULL                             // no WM_CREATE parameter 
        ); 

	/* Create the Video Player buttons */
	hwndButtonPlay = CreateWindow( 
    (LPCWSTR)"Button",  // Predefined class; Unicode assumed 
    (LPCWSTR)"Play",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    70,         // x position 
    500,         // y position 
    50,        // Button width
    40,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
    NULL, 
    NULL);      // Pointer not needed.

	hwndButtonPause = CreateWindow( 
    (LPCWSTR)"Button",  // Predefined class; Unicode assumed 
   (LPCWSTR)"Pause",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    130,         // x position 
    500,         // y position 
    50,        // Button width
    40,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
    NULL, 
    NULL);      // Pointer not needed.

	hwndButtonFastForward = CreateWindow( 
    (LPCWSTR)"Button",  // Predefined class; Unicode assumed 
    (LPCWSTR)"Bwrd",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    190,         // x position 
    505,         // y position 
    50,        // Button width
    30,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
    NULL, 
    NULL);      // Pointer not needed.

	hwndButtonFastBackward = CreateWindow( 
    (LPCWSTR)"Button",  // Predefined class; Unicode assumed 
    (LPCWSTR)"Fwrd",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    250,         // x position 
    505,         // y position 
    50,        // Button width
    30,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
    NULL, 
    NULL);      // Pointer not needed.

	// Create the 4 Checkboxes
	hwndCheckFrame0 = CreateWindow( 
    (LPCWSTR)"Button", 
    (LPCWSTR)"",      
    WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,  
    780,        
    30,         
    15,        
    15,        
    parent,     
    NULL,       
    NULL, 
    NULL); 

	hwndCheckFrame1 = CreateWindow( 
    (LPCWSTR)"Button", 
    (LPCWSTR)"",      
    WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,  
    780,        
    140,         
    15,        
    15,        
    parent,     
    NULL,       
    NULL, 
    NULL);

	hwndCheckFrame2 = CreateWindow( 
    (LPCWSTR)"Button", 
    (LPCWSTR)"",      
    WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,  
    780,        
    250,         
    15,        
    15,        
    parent,     
    NULL,       
    NULL, 
    NULL);

	hwndCheckFrame3 = CreateWindow( 
    (LPCWSTR)"Button", 
    (LPCWSTR)"",      
    WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,  
    780,        
    360,         
    15,        
    15,        
    parent,     
    NULL,       
    NULL, 
    NULL);

	hwndCheckFrame4 = CreateWindow( 
    (LPCWSTR)"Button", 
    (LPCWSTR)"",      
    WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,  
    780,        
    470,         
    15,        
    15,        
    parent,     
    NULL,       
    NULL, 
    NULL);

	hwndButtonPreviousFrames =  CreateWindow( 
    (LPCWSTR)"Button",  // Predefined class; Unicode assumed 
    (LPCWSTR)"<",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    800,         // x position 
    590,         // y position 
    30,        // Button width
    30,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
    NULL, 
    NULL);      // Pointer not needed.

	hwndButtonNextFrames =  CreateWindow( 
    (LPCWSTR)"Button",  // Predefined class; Unicode assumed 
    (LPCWSTR)">",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    830,         // x position 
    590,         // y position 
    30,        // Button width
    30,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
    NULL, 
    NULL);      // Pointer not needed.

	hwndButtonDelFrames =  CreateWindow( 
    (LPCWSTR)"Button",  // Predefined class; Unicode assumed 
    (LPCWSTR)"Del",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    860,         // x position 
    590,         // y position 
    50,        // Button width
    30,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
    NULL, 
    NULL);      // Pointer not needed.

	hwndStaticFrom = CreateWindow((LPCWSTR)"STATIC",
                             (LPCWSTR)"From",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             800,
                             640,
                             30,
                             20,
                             parent,
                             NULL,
                             NULL,
                             NULL);
	hwndEditFrom = CreateWindow((LPCWSTR)"edit", NULL,
         WS_CHILD | WS_VISIBLE | WS_BORDER ,
             840, 640, 30, 20, parent, NULL, NULL, NULL);
	hwndStaticTo = CreateWindow((LPCWSTR)"STATIC",
                             (LPCWSTR)"To",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             880,
                             640,
                             30,
                             20,
                             parent,
                             NULL,
                             NULL,
                             NULL);
	hwndEditTo = CreateWindow((LPCWSTR)"edit", NULL,
         WS_CHILD | WS_VISIBLE | WS_BORDER ,
             920, 640, 30, 20, parent, NULL, NULL, NULL);

	hwndEditVideoStatic0 = CreateWindow((LPCWSTR)"STATIC",
                             (LPCWSTR)"Duration",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             320,
                             600,
                             60,
                             20,
                             parent,
                             NULL,
                             NULL,
                             NULL);

	hwndEditVideoStatic1 = CreateWindow((LPCWSTR)"STATIC",
                             (LPCWSTR)"Codec",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             320,
                             630,
                             60,
                             20,
                             parent,
                             NULL,
                             NULL,
                             NULL);

	hwndEditVideoStatic2 = CreateWindow((LPCWSTR)"STATIC",
                             (LPCWSTR)"Size",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             320,
                             660,
                             60,
                             20,
                             parent,
                             NULL,
                             NULL,
                             NULL);

	hwndEditVideoStatic3 = CreateWindow((LPCWSTR)"STATIC",
                             (LPCWSTR)"Output format",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             50,
                             820,
                             120,
                             20,
                             parent,
                             NULL,
                             NULL,
                             NULL);

	hwndEditVideoStatic4 = CreateWindow((LPCWSTR)"STATIC",
                             (LPCWSTR)"Output file",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             50,
                             850,
                             120,
                             20,
                             parent,
                             NULL,
                             NULL,
                             NULL);

	hwndEditEdit0 = CreateWindow((LPCWSTR)"edit", NULL,
         WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
             180, 820, 300, 20, parent, NULL, NULL, NULL);

	hwndEditListView = CreateWindow((LPCWSTR)"edit", NULL,
         WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
             180, 850, 300, 20, parent, NULL, NULL, NULL);

	//hwndButtonOutput, hwndButtonBrowse, hwndButtonMerge;
	hwndButtonOutput =  CreateWindow( 
    (LPCWSTR)"Button",  // Predefined class; Unicode assumed 
    (LPCWSTR)"Output Settings",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    490,         // x position 
    820,         // y position 
    110,        // Button width
    20,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
    NULL, 
    NULL);      // Pointer not needed.

	hwndButtonBrowse =  CreateWindow( 
    (LPCWSTR)"Button",  // Predefined class; Unicode assumed 
    (LPCWSTR)"Browse",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    490,         // x position 
    850,         // y position 
    60,        // Button width
    20,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
    NULL, 
    NULL);      // Pointer not needed.

	hwndButtonMerge =  CreateWindow( 
    (LPCWSTR)"Button",  // Predefined class; Unicode assumed 
    (LPCWSTR)"Start",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    610,         // x position 
    820,         // y position 
    100,        // Button width
    50,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
    NULL, 
    NULL);      // Pointer not needed.

	return true;
}

bool testProcessInj(int pid, char* dllName) {
	int  ProcessID = pid;

	HANDLE Proc;
   HANDLE Thread;
   char buf[50]={0};
   LPVOID RemoteString, LoadLibAddy;
   HMODULE hModule = NULL;
   DWORD dwOut;
 
   if(! ProcessID)
      return false;
 
   Proc = OpenProcess(PROCESS_ALL_ACCESS, 0, ProcessID);
 
   if(!Proc)
   {
      sprintf_s(buf, "OpenProcess() failed: %d", GetLastError());
      MessageBoxA(NULL, buf, "Loader", NULL);
      return false;
   }
 
   LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
   if (!LoadLibAddy) {
	    MessageBoxA(NULL, buf, "GetProceAdd", NULL);
        return false;
   }
 
 
   RemoteString = (LPVOID)VirtualAllocEx(Proc, NULL, strlen(dllName), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
   if (!RemoteString) {
	   MessageBoxA(NULL, buf, "RemoteString", NULL);
       return false;
   }
 
   if (!WriteProcessMemory(Proc, (LPVOID)RemoteString, dllName, strlen(dllName), NULL)) {
	   MessageBoxA(NULL, buf, "WRITEprocess", NULL);
       return false;
    }
 
   Thread = CreateRemoteThread(Proc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, NULL, NULL);   
   if (!Thread) {
	   MessageBoxA(NULL, buf, "Create THread", NULL);
        return false;
   } else {
        while(GetExitCodeThread(Thread, &dwOut)) {
            if(dwOut != STILL_ACTIVE) {
                hModule = (HMODULE)dwOut;
                break;
            }
        }
    }

   return true;
}

HRESULT CreateCaptureBuffer(LPDIRECTSOUNDCAPTURE8 pDSC, 
                            LPDIRECTSOUNDCAPTUREBUFFER8* ppDSCB8)
{
  HRESULT hr;
  DSCBUFFERDESC               dscbd;
  LPDIRECTSOUNDCAPTUREBUFFER  pDSCB;

  // Set up WAVEFORMATEX for 44.1 kHz 16-bit stereo. 
  WAVEFORMATEX                wfx =
  {WAVE_FORMAT_PCM, 2, 44100, 44100 * 4, 4, 16, 0};
    // wFormatTag, nChannels, nSamplesPerSec, mAvgBytesPerSec,
    // nBlockAlign, wBitsPerSample, cbSize

  /* CHECK IF FORMAT IS SUPPORTED 

  wReturn = IsFormatSupported(&wfx, WAVE_MAPPER); 
  if (wReturn == 0) 
    MessageBox(NULL, "44.1 kHz 16-bit stereo is supported.", "", MB_ICONINFORMATION); 
  else if (wReturn == WAVERR_BADFORMAT) 
    MessageBox(NULL, "44.1 kHz 16-bit stereo NOT supported.", 
      "", MB_ICONINFORMATION); 
  else 
    MessageBox(NULL, "Error opening waveform device.", 
      "Error", MB_ICONEXCLAMATION); 

  */

 
  if ((NULL == pDSC) || (NULL == ppDSCB8)) return E_INVALIDARG;
  dscbd.dwSize = sizeof(DSCBUFFERDESC);
  dscbd.dwFlags = 0;
  dscbd.dwBufferBytes = wfx.nAvgBytesPerSec;
  dscbd.dwReserved = 0;
  dscbd.lpwfxFormat = &wfx;
  dscbd.dwFXCount = 0;
  dscbd.lpDSCFXDesc = NULL;

 
  if (SUCCEEDED(hr = pDSC->CreateCaptureBuffer(&dscbd, &pDSCB, NULL)))
  {
	/* Use the method QueryInterface of  IDirectSoundCaptureBuffer(pDSCB->QueryInterface()) to get a 
    pointer(pDSCB8) to the interface IDirectSoundCaptureBuffer8 */
    hr = pDSCB->QueryInterface(IID_IDirectSoundCaptureBuffer8, (LPVOID*)ppDSCB8);
    pDSCB->Release();  
  }
  return hr;
}
