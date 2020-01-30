// Win32Project2.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Win32Project2.h"
#include "Wav.h"
#include "WavFile.h"
#include <stdio.h>
//#include "avifile.h"

#include <MMsystem.h>
#include <fstream>
#include <cstdlib>
#include <strsafe.h>

#include <Windowsx.h>
#include <Prsht.h>

#include <shobjidl.h> 

#include <dllmain.h>
#pragma comment(lib, "win32p.lib")

//#include "Injector.h"

#include "Temp.h"

HMODULE InjectDLL(DWORD ProcessID, char* dllName)
{
   HANDLE Proc;
   HANDLE Thread;
   char buf[50]={0};
   LPVOID RemoteString, LoadLibAddy;
   HMODULE hModule = NULL;
   DWORD dwOut;
 
   if(!ProcessID)
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

   // Send the PID to injected process
   
   
   //DWORD pid = GetCurrentProcessId();
   //HMODULE hLoaded = LoadLibrary(dllName);

   /*
  if( hLoaded == NULL ) {
    MessageBoxA(NULL, buf, "Failed to load the dll", NULL);
  } else {
    void* lpFunc   = GetProcAddress( hLoaded, "sendpid");
	DWORD dwOffset = (char*)lpFunc - (char*)hLoaded; 
	FreeLibrary( hLoaded );
   
   RemoteString = (LPVOID)VirtualAllocEx(Proc, NULL, sizeof(DWORD), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
   if (!RemoteString) {
	   MessageBoxA(NULL, buf, "Failed to allocate memory for pid", NULL);
       return false;
   }
   
   if (!WriteProcessMemory(Proc, (LPVOID)RemoteString, &pid, sizeof(DWORD), NULL)) {
	   MessageBoxA(NULL, buf, "Failed to write the pid value", NULL);
       return false;
   }
   
   LPVOID lpFunctionAddress = hModule + dwOffset;
   Thread = CreateRemoteThread(Proc, NULL, NULL, (LPTHREAD_START_ROUTINE)lpFunctionAddress, (LPVOID)RemoteString, NULL, NULL); 
   
   if (!Thread) {
	   MessageBoxA(NULL, buf, "Failed to call function", NULL);
        return false;
   }
   }
   */
   CloseHandle(Thread);
   CloseHandle(Proc);
 
   return hModule;
}

MMRESULT IsFormatSupported(LPWAVEFORMATEX pwfx, UINT uDeviceID) 
{ 
    return (waveOutOpen( 
        NULL,                 // ptr can be NULL for query 
        uDeviceID,            // the device identifier 
        pwfx,                 // defines requested format 
        NULL,                 // no callback 
        NULL,                 // no instance data 
        WAVE_FORMAT_QUERY));  // query only, do not open device 
} 


#define WAVEFILE_READ   1
#define WAVEFILE_WRITE  2

#define MAX(a,b)        ( (a) > (b) ? (a) : (b) )   

#define MAX_LOADSTRING 100

#define PS_PAGES 2

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

//CAviFile	*pAviFile=NULL;

HWND hWnd; // main handle of application

char path[MAX_PATH];
char exename[MAX_PATH];
char dllname[MAX_PATH];
const char* DLL_NAME = "win32p.dll";
DWORD pId; 

bool captureaza = false;
#define BITSPERPIXEL		32
void	SaveBitmap(char *szFilename,HBITMAP hBitmap);
UINT_PTR	nTimerId=0;
LPVOID		pBits=NULL;

CWaveFile wavTest;

bool captureazaSunet = false;
wav_header wavFile;
UINT totalData = 0;
LPDIRECTSOUNDCAPTURE8 capturer = NULL;
GUID guid = DSDEVID_DefaultCapture;
HRESULT err;
DSCBUFFERDESC dscbd;
LPDIRECTSOUNDCAPTUREBUFFER8 pDSCB8;
DWORD g_dwNextCaptureOffset = 0;
DWORD g_dwCaptureBufferSize = 0;
DWORD g_dwNotifySize = 0;

#define cEvents  3

WAVEFORMATEX         wfx;  
HANDLE     rghEvent[cEvents] = {0,0,0};
DSBPOSITIONNOTIFY  rgdsbpn[cEvents];

DWORD dwResult;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
HWND DoCreateTabControl(HWND hwndParent); 
HWND DoCreateDisplayWindow(HWND hwndTab);
HRESULT OnSize(HWND hwndTab, LPARAM lParam);
BOOL OnNotify(HWND hwndTab, HWND hwndDisplay, LPARAM lParam);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HWND CreatePropSheet (HWND hWndParent);
LRESULT CALLBACK PropSheetProc (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

void ErrorExit(LPTSTR);

// DirectSound
HRESULT CreateCaptureBuffer(LPDIRECTSOUNDCAPTURE8, LPDIRECTSOUNDCAPTUREBUFFER8*);
HRESULT SetCaptureNotifications(LPDIRECTSOUNDCAPTUREBUFFER8);
HRESULT RecordCapturedData();

// WASAPI
#define REFTIMES_PER_SEC  1000000
#define REFTIMES_PER_MILLISEC  1000
const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);
IAudioClient *pAudioClient = NULL;
IAudioCaptureClient *pCaptureClient = NULL;
HANDLE ProcessSoundThread;
HANDLE _AudioSamplesReadyEvent, _ShutdownEvent;
HRESULT InitAudio();
DWORD WINAPI ProcessSound(void*);

// Other
void openDialogBox(); 
void prepareAddVideo();
void aplicaOptiuni();
void addVideo();
void adaugaOptiuni();
void addEffect(efect* );
void salveazaEfecte();
HRESULT creeazaAvi(int, int, int ,PAVIFILE* , TCHAR* ,AVISTREAMINFO* , PAVISTREAM*,AVICOMPRESSOPTIONS* , PAVISTREAM* );
bool BuildControls(HWND parent);
bool beginEdit(HWND parent);
void OpenAVI(LPCSTR szFile);
void GrabAVIFrame(int frame);

// controls
HWND hwndStaticInjector, hwndStaticCmdla;
HWND hwndEditInjector, hwndEditExec, hwndEditCmdla;
HWND hwndButtonExe, hwndButtonInjector, hwndCheekBenchmark;
HWND hwndEditFrom, hwndEditTo, hwndStaticFrom, hwndStaticTo;
int lengthFrom, lengthTo;
LPSTR lpstrFrom, lpstrTo;
LONG fromInt, toInt, lengthTotal;



/* AVI file */
AVISTREAMINFO       psi;                    // Pointer To A Structure Containing Stream Info
PAVISTREAM      pavi;                       // Handle To An Open Stream
PAVISTREAM editablePavi;
PGETFRAME       pgf;                        // Pointer To A GetFrame Object
BITMAPINFOHEADER    bmih;                   // Header Information For DrawDibDraw Decoding
long            lastframe;                  // Last Frame Of The Stream
int         width;							// Video Width
int         height;							// Video Height
char            *pdata;                     // Pointer To Texture Data
LONG         mpf;							// Will Hold Rough Milliseconds Per Frame
int frame = 0;								// current frame for the video played
int editFrame = 0;						    // current frame for the frames to be edited
LPBITMAPINFOHEADER lpbi;                    // Holds The Bitmap Header Information
LPBITMAPINFOHEADER lpbiEditor;                    // Holds The address of the first 5 frames to be edited
LPBITMAPINFOHEADER lpbiEffect;                    // Holds the address of the frame to be edited

AVICOMPRESSOPTIONS	m_AviCompressOptions; 

/* AVI added file */
	AVISTREAMINFO   psiAdd;                    
	PAVISTREAM      paviAdd;                       
	PAVISTREAM		editablePaviAdd;
	PGETFRAME       pgfAdd;                        
	int				widthAdd, heightAdd;
	long			lastFrameAdd;
	LPBITMAPINFOHEADER lpbiAdd; 
	char            *pdataAdd;   // pointer to the video to-be-merged frame data 
	bool isAddingVideo = false;


// Video Player
BOOL playVideo = false;
BOOL playVideoFirst = false; // Video Preview
BOOL pausedVideo = false; 
BOOL loadedVideo = false;
BOOL isEditing = false;
BOOL isFrame0, isFrame1, isFrame2, isFrame3, isFrame4; 
LONG startPoint0,startPoint1,startPoint2,startPoint3,startPoint4 = 0; LONG length1 = 1;
HDC hBackDC; // handle to Drawable Window
HDC oHDC;
unsigned char* data = 0;                        // Pointer To Our Resized Image
HDRAWDIB hdd; // Handle For Our Dib
HBITMAP	hBackBitmap=NULL; // Handle To A Device Dependant Bitmap
HBITMAP	hOldBitmap=NULL;

HWND hwndTrack;
HWND hwndButtonPlay, hwndButtonPause, hwndButtonFastForward, hwndButtonFastBackward;
HWND hwndCheckFrame0, hwndCheckFrame1, hwndCheckFrame2, hwndCheckFrame3, hwndCheckFrame4, hwndButtonNextFrames, hwndButtonPreviousFrames, hwndButtonDelFrames;

HWND hwndEditVideoStatic0, hwndEditVideoStatic1, hwndEditVideoStatic2, hwndEditVideoStatic3, hwndEditVideoStatic4;
HWND hwndEditEdit0, hwndEditListView;
HWND hwndButtonOutput, hwndButtonBrowse, hwndButtonMerge;

HWND hWndComboBox, hwndStaticFromEffect, hwndEditFromEffect, hwndStaticToEffect, hwndEditToEffect;

// Optiuni
HWND handleforwindow2;
HWND hwndStaticSunet,hwndStaticAID,hwndStaticChannel,hwndStaticDF,hwndStaticBl, hwndStaticSunetMS;
HWND hwndEditBl;
HWND hwndComboBoxAID,hwndComboBoxChannel,hwndComboBoxDF;

HWND hwndStaticVideo,hwndStaticShowFPS,hwndStaticTargetFR,hwndStaticFPS, hwndStaticVC;
HWND hwndEditTargetFR;
HWND hwndCheckBoxShowFPS;
HWND hwndComboBoxVC;

HWND hwndStaticHotKey,hwndStaticVRS,hwndStaticVRP,hwndStaticScreen;
HWND hwndEditVRS,hwndEditVRP,hwndEditScreen;

HWND hwndButtonApplySettings;

// Editor

fisierTemp optiuniEditare;

char addVideoPath[MAX_PATH]; // path to the video to be merged with


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;
	BOOL  bDone; 

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WIN32PROJECT2, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PROJECT2));


	bDone = FALSE;
	// Main message loop:
	while( !bDone ) {
		 dwResult = MsgWaitForMultipleObjects( 2, rghEvent, FALSE, INFINITE, QS_ALLEVENTS ); 
		 switch( dwResult ) {
			case WAIT_OBJECT_0 + 0: 
				//MessageBox(NULL,TEXT("wait1"), TEXT("Error"), MB_OK);
				//if(FAILED(RecordCapturedData())) MessageBox(NULL,TEXT("EroareCapturare!"), TEXT("Error"), MB_OK);
				break;
			case WAIT_OBJECT_0 + 1: 
				MessageBox(NULL,TEXT("wait2"), TEXT("Error"), MB_OK);
				captureazaSunet = false;
				//if(FAILED(RecordCapturedData())) MessageBox(NULL,TEXT("EroareCapturare!"), TEXT("Error"), MB_OK);
				break;
			case WAIT_OBJECT_0 + 2:
				//MessageBox(NULL,TEXT("wait3"), TEXT("Error"), MB_OK);
				// Windows messages are available
				while (GetMessageW(&msg, NULL, 0, 0) > 0) // while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
					{
						if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
						{
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
						if( msg.message == WM_QUIT )   
							bDone = TRUE; 
					}
				break;
		 }
	}

	//CloseHandle(rghEvent);
	
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT2));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WIN32PROJECT2);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
  
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   SetWindowPos(hWnd,HWND_TOP,300,250,400,250,SWP_SHOWWINDOW);
   BuildControls(hWnd);

   // set shared memory server hWnd
   SetServerHwnd(hWnd);

   //DoCreateTabControl(hWnd);
   //DoCreateDisplayWindow(hWnd);
   //hwndPropCtrl = CreatePropSheet(hWnd);
   //SetParent(hwndPropCtrl,hWnd);
   //SetWindowPos(hwndPropCtrl,NULL,0,90,300,250,SWP_NOZORDER|SWP_NOACTIVATE);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   WNDCLASSEX windowclassforwindow2;
    ZeroMemory(&windowclassforwindow2,sizeof(WNDCLASSEX));
    windowclassforwindow2.cbClsExtra=NULL;
    windowclassforwindow2.cbSize=sizeof(WNDCLASSEX);
    windowclassforwindow2.cbWndExtra=NULL;
    windowclassforwindow2.hbrBackground=(HBRUSH)COLOR_WINDOW;
    windowclassforwindow2.hCursor=LoadCursor(NULL,IDC_ARROW);
    windowclassforwindow2.hIcon=NULL;
    windowclassforwindow2.hIconSm=NULL;
    windowclassforwindow2.hInstance=hInst;
    windowclassforwindow2.lpfnWndProc=(WNDPROC)WndProc;
    windowclassforwindow2.lpszClassName="window class2";
    windowclassforwindow2.lpszMenuName=NULL;
    windowclassforwindow2.style=CS_HREDRAW|CS_VREDRAW;

    if(!RegisterClassEx(&windowclassforwindow2))
    {
        int nResult=GetLastError();
        MessageBox(NULL,
            "Window class creation failed for window 2",
            "Window Class Failed",
            MB_ICONERROR);
    }

    handleforwindow2=CreateWindowEx(NULL,
        windowclassforwindow2.lpszClassName,
            "Optiuni",
            WS_OVERLAPPEDWINDOW,
            200,
            150,
            490,
            620,
            NULL,
            NULL,
            hInst,
            NULL);

    if(!handleforwindow2)
    {
        int nResult=GetLastError();

        MessageBox(NULL,
            "Window creation failed",
            "Window Creation Failed",
            MB_ICONERROR);
    }

	/*****************************************/// schimba IDC LA FIECARE

	hwndStaticSunet 	     = CreateWindow("STATIC",
                             "Sound",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             20,
                             20,
                             60,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	hwndStaticAID 	         = CreateWindow("STATIC",
                             "Audio Input Device:",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             20,
                             60,
                             160,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	hwndComboBoxAID = CreateWindow(WC_COMBOBOX, TEXT(""), 
		CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		160, 60, 300, 150, handleforwindow2, (HMENU)IDC_AID, hInst,NULL); 

	const TCHAR* ComboBoxItems[] = { _T( "Default" ), _T("Microphone")};

	for (int k = 0; k < 2; k++)
	{
		//wcscpy_s(A, sizeof(A)/sizeof(wchar_t),  (const wchar_t *)ComboBoxItems[k]);

		// Add string to combobox.
		SendMessage(hwndComboBoxAID,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) ComboBoxItems[k]); 
	}

	SendMessage(hwndComboBoxAID, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

	hwndStaticChannel 	             = CreateWindow("STATIC",
                             "Channels:",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             20,
                             100,
                             120,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	hwndComboBoxChannel = CreateWindow(WC_COMBOBOX, TEXT(""), 
		CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		160, 100, 300, 150, handleforwindow2, (HMENU)IDC_CHANNEL, hInst,NULL); 

	const TCHAR* ComboBoxItems2[] = { _T( "Mono" ), _T( "Stereo" )};

	for (int k = 0; k <= 1; k++)
	{
		// Add string to combobox.
		SendMessage(hwndComboBoxChannel,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) ComboBoxItems2[k]); 
	}

	SendMessage(hwndComboBoxChannel, CB_SETCURSEL, (WPARAM)1, (LPARAM)0);

	hwndStaticDF 	             = CreateWindow("STATIC",
                             "Default Format:",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             20,
                             140,
                             120,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	hwndComboBoxDF = CreateWindow(WC_COMBOBOX, TEXT(""), 
		CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		160, 140, 300, 150, handleforwindow2, (HMENU)IDC_FORMAT, hInst,NULL); 

	const TCHAR* ComboBoxItems3[] = { _T( "16 bit, 44100 Hz(CD Quality)" ), _T( "16 bit, 48000 Hz(DVD Quality)" ), _T( "16 bit, 96000 Hz(Studio Quality)" ), _T( "24 bit, 192000 Hz(Studio Quality)" )};

	for (int k = 0; k <= 3; k++)
	{
		// Add string to combobox.
		SendMessage(hwndComboBoxDF,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) ComboBoxItems3[k]); 
	}

	SendMessage(hwndComboBoxDF, CB_SETCURSEL, (WPARAM)1, (LPARAM)0);

	hwndStaticBl 	             = CreateWindow("STATIC",
                             "Buffer length:",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             20,
                             180,
                             120,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	hwndEditBl = CreateWindow("edit", "100",WS_CHILD | WS_VISIBLE | WS_BORDER , 160, 180, 260, 20, handleforwindow2, (HMENU) IDC_BFLENGTH, hInst, NULL);

	hwndStaticSunetMS 	      = CreateWindow("STATIC",
                             "ms",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             440,
                             180,
                             120,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	/*video*/
	hwndStaticVideo 	             = CreateWindow("STATIC",
                             "Video",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             20,
                             220,
                             120,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	hwndStaticShowFPS 	             = CreateWindow("STATIC",
                             "Show FPS:",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             20,
                             260,
                             120,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	hwndCheckBoxShowFPS =  CreateWindowEx(
            0, 
            "BUTTON", 
            "Yes", 
            WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 
            160, 
            260, 
            120, 
            20, 
            handleforwindow2, 
            (HMENU)IDC_SHOWFPS, 
            hInst, 
            NULL); 

	hwndStaticTargetFR 	             = CreateWindow("STATIC",
                             "Target Frame Rate:",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             20,
                             300,
                             140,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	hwndEditTargetFR = CreateWindow("edit", "30",WS_CHILD | WS_VISIBLE | WS_BORDER , 160, 300, 260, 20, handleforwindow2, (HMENU) IDC_TFR, hInst, NULL);

	hwndStaticFPS 	             = CreateWindow("STATIC",
                             "FPS",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             433,
                             300,
                             120,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	hwndStaticVC 	             = CreateWindow("STATIC",
                             "Video codecs:",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             20,
                             340,
                             120,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	hwndComboBoxVC = CreateWindow(WC_COMBOBOX, TEXT(""), 
		CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		160, 340, 300, 150, handleforwindow2, (HMENU)IDC_CODECS, hInst,NULL); 

	const TCHAR* ComboBoxItems4[] = { _T( "H.265/MPEG-HHEVC" ), _T( "H.264/MPEG-4 AVC(x264)" ), _T( "Lagarith" ), _T("MPEG-1")};

	for (int k = 0; k <= 3; k++)
	{
		// Add string to combobox.
		SendMessage(hwndComboBoxVC,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) ComboBoxItems4[k]); 
	}

	SendMessage(hwndComboBoxVC, CB_SETCURSEL, (WPARAM)1, (LPARAM)0);

	/*HOTKEY*/
	hwndStaticHotKey 	             = CreateWindow("STATIC",
                             "Hot Keys",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             20,
                             380,
                             120,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	hwndStaticVRS 	             = CreateWindow("STATIC",
                             "Video Record Start:",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             20,
                             420,
                             160,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	hwndEditVRS = CreateWindow("edit", "R",WS_CHILD | WS_VISIBLE | WS_BORDER , 160, 420, 300, 20, handleforwindow2, (HMENU) IDC_VRS, hInst, NULL);

	hwndStaticVRP 	             = CreateWindow("STATIC",
                             "Video Record Pause:",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             20,
                             460,
                             160,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	hwndEditVRP = CreateWindow("edit", "T",WS_CHILD | WS_VISIBLE | WS_BORDER , 160, 460, 300, 20, handleforwindow2, (HMENU) IDC_VRP, hInst, NULL);

	hwndStaticScreen 	             = CreateWindow("STATIC",
                             "Screenshot:",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             20,
                             500,
                             120,
                             20,
                             handleforwindow2,
                             NULL,
                             hInst,
                             NULL);

	hwndEditScreen = CreateWindow("edit", "O",WS_CHILD | WS_VISIBLE | WS_BORDER , 160, 500, 300, 20, handleforwindow2, (HMENU) IDC_SCREENSHOOT, hInst, NULL);

	hwndButtonApplySettings =  CreateWindow( 
    "Button",  // Predefined class; Unicode assumed 
    "APPLY",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    400,         // x position 
    540,         // y position 
    60,        // Button width
    30,        // Button height
    handleforwindow2,     // Parent window
    ( HMENU )IDC_BTNAPPLY,       // No menu.
    hInst, 
    NULL);      // Pointer not needed.

	/*****************************************/
    //SetParent(handleforwindow2,hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	static HBITMAP	hDesktopCompatibleBitmap=NULL;
	static HDC		hDesktopCompatibleDC=NULL;
	static HDC		hDesktopDC=NULL;
	static HWND		hDesktopWnd=NULL;
	static RECT		clientRect = {0,0,0,0};
	PWSTR a;

	int pos;
	switch (message)
	{
    case (WM_APP + 7):
			SetEvent(_ShutdownEvent);
			pAudioClient->Stop();
			//WaitForSingleObject(ProcessSoundThread, INFINITE);
			CloseHandle(ProcessSoundThread);
			//ProcessSoundThread = NULL;
			 //CloseHandle(_AudioSamplesReadyEvent);
			//_AudioSamplesReadyEvent = NULL;
			break;
	case (WM_APP + 6): // start capturing sound
			captureazaSunet = true;
			
			if (FAILED(createWav(&wavFile)))
			{
				closeWav(wavFile.pFile);
				MessageBox(NULL,TEXT("Error creating the sound file!"), TEXT("Error"), MB_OK);
			}

			// add the WAV header
			//fputs(wavFile.riff_header, wavFile.pFile);
			for(int i = 0; i < 4; i++) {
				fputc(wavFile.riff_header[i], wavFile.pFile);
			}
			fwrite((char*)&wavFile.wav_size,sizeof(int),1,wavFile.pFile);
			//fputs((const char*)wavFile.wave_header, wavFile.pFile);
			for(int i = 0; i < 4; i++) {
				fputc(wavFile.wave_header[i], wavFile.pFile);
			}
			//fputs((const char*)wavFile.fmt_header, wavFile.pFile);
			for(int i = 0; i < 4; i++) {
				fputc(wavFile.fmt_header[i], wavFile.pFile);
			}
			fwrite((const char*)&wavFile.fmt_chunk_size,4,1,wavFile.pFile);
			fwrite((const char*)&wavFile.audio_format,2,1,wavFile.pFile);
			fwrite((const char*)&wavFile.num_channels,2,1,wavFile.pFile);
			fwrite((const char*)&wavFile.sample_rate,4,1,wavFile.pFile);
			fwrite((const char*)&wavFile.byte_rate,4,1,wavFile.pFile);
			fwrite((const char*)&wavFile.sample_alignment,2,1,wavFile.pFile);
			fwrite((const char*)&wavFile.bit_depth,2,1,wavFile.pFile);
			//fputs((const char*)wavFile.data_header, wavFile.pFile);
			for(int i = 0; i < 4; i++) {
				fputc(wavFile.data_header[i], wavFile.pFile);
			}
			fwrite((const char*)&wavFile.data_bytes,4,1,wavFile.pFile);

			InitAudio();
			break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDC_BTNAPPLY:
			aplicaOptiuni();
			break;
		// checkbox Yes
		case IDC_SHOWFPS:
			switch (HIWORD(wParam)) {
				case BN_CLICKED:
					if (SendDlgItemMessage(hWnd, IDC_SHOWFPS, BM_GETCHECK, 0, 0)) {
						
					}
					else {
						
					}
					break;
			}
			break;
		case BTN_EFFECTS:
			//addEffect();
			salveazaEfecte();
			break;
		case BTN_MERGE: 
			addVideo();
			break;
		case IDM_ADDVIDEO:
			prepareAddVideo();
			break;
		case CHK_FRAME0:
			switch (HIWORD(wParam))
				{
                        case BN_CLICKED:
                            if (SendDlgItemMessage(hWnd, CHK_FRAME0, BM_GETCHECK, 0, 0)) 
								{
									isFrame0 = true;
									startPoint0 = editFrame + 0;
								}
                            else
								{
									isFrame0 = false;
								}
                        break; 
                }
			break;
		case CHK_FRAME1:
			switch (HIWORD(wParam))
				{
                        case BN_CLICKED:
                            if (SendDlgItemMessage(hWnd, CHK_FRAME1, BM_GETCHECK, 0, 0)) 
								{
									isFrame1 = true;
									startPoint1 = editFrame + 1;
								}
                            else
								{
									isFrame1 = false;
								}
                        break; 
                }
			break;
		case CHK_FRAME2:
			switch (HIWORD(wParam))
				{
                        case BN_CLICKED:
                            if (SendDlgItemMessage(hWnd, CHK_FRAME2, BM_GETCHECK, 0, 0)) 
								{
									isFrame2 = true;
									startPoint2 = editFrame + 2;
								}
                            else
								{
									isFrame2 = false;
								}
                        break; 
                }
			break;
		case CHK_FRAME3:
			switch (HIWORD(wParam))
				{
                        case BN_CLICKED:
                            if (SendDlgItemMessage(hWnd, CHK_FRAME3, BM_GETCHECK, 0, 0)) 
								{
									isFrame3 = true;
									startPoint3 = editFrame + 3;
								}
                            else
								{
									isFrame3 = false;
								}
                        break; 
                }
			break;
		case CHK_FRAME4:
			switch (HIWORD(wParam))
				{
                        case BN_CLICKED:
                            if (SendDlgItemMessage(hWnd, CHK_FRAME4, BM_GETCHECK, 0, 0)) 
								{
									isFrame4 = true;
									startPoint4 = editFrame + 4;
								}
                            else
								{
									isFrame4 = false;
								}
                        break; 
                }
			break;
		case BTN_FRAMEBACK:
			isEditing = true;
			editFrame-=5;
			break;
		case BTN_FRAMEFWRD:
			isEditing = true;
			editFrame+=5;
			break;
		case BTN_DELFRAMES:
			if(isFrame0) {
				if(EditStreamCut(editablePavi,&startPoint0, &length1, NULL) != 0 ) MessageBoxA(NULL, "Failed to delete frame 0!", "Error", MB_OK);  
			}
			if(isFrame1) if(EditStreamCut(editablePavi,&startPoint1, &length1, NULL) != 0 ) MessageBoxA(NULL, "Failed to delete frame 1!", "Error", MB_OK);  
			if(isFrame2) if(EditStreamCut(editablePavi,&startPoint2, &length1, NULL) != 0 ) MessageBoxA(NULL, "Failed to delete frame 2!", "Error", MB_OK);  
			if(isFrame3) if(EditStreamCut(editablePavi,&startPoint3, &length1, NULL) != 0 ) MessageBoxA(NULL, "Failed to delete frame 3!", "Error", MB_OK);  
			if(isFrame4) if(EditStreamCut(editablePavi,&startPoint4, &length1, NULL) != 0 ) MessageBoxA(NULL, "Failed to delete frame 4!", "Error", MB_OK);

			// delete From - To frames
			lengthFrom = SendMessage(hwndEditFrom, WM_GETTEXTLENGTH, 0, 0);
			lpstrFrom = (LPSTR)malloc(lengthFrom + 1);
			SendMessage(hwndEditFrom, WM_GETTEXT, lengthFrom+1, LPARAM(lpstrFrom));
			lengthTo = SendMessage(hwndEditTo, WM_GETTEXTLENGTH, 0, 0);
			lpstrTo = (LPSTR)malloc(lengthFrom + 1);
			SendMessage(hwndEditTo, WM_GETTEXT, lengthTo+1, LPARAM(lpstrTo));
			fromInt = atoi(lpstrFrom);
			toInt = atoi(lpstrTo);
			lengthTotal = toInt - fromInt;
			if(EditStreamCut(editablePavi,&fromInt, &lengthTotal, NULL) != 0 ) MessageBoxA(NULL, "Failed to delete frame 4!", "Error", MB_OK);

			if(AVISave("Output2.avi", NULL, NULL, 1, editablePavi, &m_AviCompressOptions) != AVIERR_OK) MessageBoxA(NULL, "Failed to save the edited video!", "Error", MB_OK);
			break;
		case BTN_INJECT:
			// The DLL is on the same folder as the Tool
			GetModuleFileNameA(0, path, MAX_PATH);
			pos = 0;
			for (int k = 0; k < strlen(path); k++) {
				if (path[k] == '\\') {
				pos = k;
				}
			}
			path[pos+1] = 0;
			//Merge the path with the dll name
			strcpy_s(dllname, path);
			strcat_s(dllname, DLL_NAME);

			// Start the process suspended 
			STARTUPINFOA siStartupInfo;
			PROCESS_INFORMATION piProcessInfo;
			memset(&siStartupInfo, 0, sizeof(siStartupInfo));
			memset(&piProcessInfo, 0, sizeof(piProcessInfo));
			siStartupInfo.cb = sizeof(siStartupInfo);

			if (!CreateProcessA(NULL,exename, 0, 0, false,CREATE_SUSPENDED, 0, 0,&siStartupInfo, &piProcessInfo)) {
				MessageBoxA(NULL, exename, "Error", MB_OK); 
			}

			// get the process id for injection
			pId = piProcessInfo.dwProcessId;

			HMODULE hModule;
			hModule = InjectDLL(pId, dllname);
			// Inject the dll
			if (!hModule) {
				MessageBoxA(NULL, "Injection failed", "Error", MB_OK);      
			}

			ResumeThread(piProcessInfo.hThread);

			break;
		case BTN_CHOOSE: 
			 openDialogBox();
			break;
		case IDM_ABOUT:
			beginEdit(hWnd);
			//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			
			/*
			if(pAviFile==NULL)
				{
					OPENFILENAME	ofn;
					char	szFileName[512];
					strcpy(szFileName,"Output.avi");
					ZeroMemory(&ofn,sizeof(ofn));
					ofn.lStructSize=sizeof(OPENFILENAME);
					ofn.Flags=OFN_HIDEREADONLY|OFN_PATHMUSTEXIST;
					ofn.lpstrFilter="Avi Files (*.avi)\0*.avi\0";
					ofn.lpstrDefExt="avi";				
					ofn.lpstrFile=szFileName;
					ofn.nMaxFile=512;
					ofn.hwndOwner = hWnd;
					if(!GetSaveFileName(&ofn))	break;

					//pAviFile = new CAviFile(szFileName);
				}
			*/
			//captureaza = true;
			break;
		case IDM_CAPTURESOUND:
			captureazaSunet = true;

			
			if (FAILED(createWav(&wavFile)))
			{
				closeWav(wavFile.pFile);
				MessageBox(NULL,TEXT("Error creating the sound file!"), TEXT("Error"), MB_OK);
			}

			// add the WAV header
			//fputs(wavFile.riff_header, wavFile.pFile);
			for(int i = 0; i < 4; i++) {
				fputc(wavFile.riff_header[i], wavFile.pFile);
			}
			fwrite((char*)&wavFile.wav_size,sizeof(int),1,wavFile.pFile);
			//fputs((const char*)wavFile.wave_header, wavFile.pFile);
			for(int i = 0; i < 4; i++) {
				fputc(wavFile.wave_header[i], wavFile.pFile);
			}
			//fputs((const char*)wavFile.fmt_header, wavFile.pFile);
			for(int i = 0; i < 4; i++) {
				fputc(wavFile.fmt_header[i], wavFile.pFile);
			}
			fwrite((const char*)&wavFile.fmt_chunk_size,4,1,wavFile.pFile);
			fwrite((const char*)&wavFile.audio_format,2,1,wavFile.pFile);
			fwrite((const char*)&wavFile.num_channels,2,1,wavFile.pFile);
			fwrite((const char*)&wavFile.sample_rate,4,1,wavFile.pFile);
			fwrite((const char*)&wavFile.byte_rate,4,1,wavFile.pFile);
			fwrite((const char*)&wavFile.sample_alignment,2,1,wavFile.pFile);
			fwrite((const char*)&wavFile.bit_depth,2,1,wavFile.pFile);
			//fputs((const char*)wavFile.data_header, wavFile.pFile);
			for(int i = 0; i < 4; i++) {
				fputc(wavFile.data_header[i], wavFile.pFile);
			}
			fwrite((const char*)&wavFile.data_bytes,4,1,wavFile.pFile);

			InitAudio();

			/*
			// Use DirectSoundCaptureCreate8() to create and initialize an object and get the pointer (pDSC8) to IDirectSoundCapture8 
            if(FAILED(DirectSoundCaptureCreate8(&guid, &capturer, NULL))) {
	            ErrorExit(TEXT("DirectSoundCaptureCreate8"));
            }
            // else MessageBox(NULL, TEXT("Device Driver found"), TEXT("Good"), MB_OK); 

            // Use the method CreateCaptureBuffer() of IDirectSoundCapture8(pDSC8->CreateSoundBuffer()) to create 
            // and initialize an object and get the  pointer (pDSCB) to IDirectSoundCaptureBuffer. 
            if(FAILED(CreateCaptureBuffer(capturer,&pDSCB8))) {
		         ErrorExit(TEXT("CreateCaptureBuffer"));	
            }
            //else MessageBox(NULL,TEXT("Created the buffer!"), TEXT("Good"), MB_OK);

            // Use the method QueryInterface of  IDirectSoundCaptureBuffer8(pDSCB8->QueryInterface()) to get a pointer(lpDsNotify) to the interface IDirectSoundNotify8. 
            if(FAILED(SetCaptureNotifications(pDSCB8))) {
		         ErrorExit(TEXT("SetCaptureNotifications"));	
            }
            //else MessageBox(NULL,TEXT("Made the notification!"), TEXT("Good"), MB_OK);

			// Start capturing 
			if(FAILED(pDSCB8->Start( DSCBSTART_LOOPING ) ) )
				ErrorExit(TEXT("Start"));	
			else MessageBox(NULL,TEXT("Started capturing!"), TEXT("Good"), MB_OK);
			*/
			break;
		case IDM_OPRESTE:
			SetEvent(_ShutdownEvent);
			pAudioClient->Stop();
			//WaitForSingleObject(ProcessSoundThread, INFINITE);

			CloseHandle(ProcessSoundThread);
			//ProcessSoundThread = NULL;
			 //CloseHandle(_AudioSamplesReadyEvent);
			//_AudioSamplesReadyEvent = NULL;
			// Stop the buffer, and read any data that was not    
			// caught by a notification 
			
			/*
			if( FAILED(pDSCB8->Stop() ) )   
				 ErrorExit(TEXT("Stop"));

			if(FAILED(RecordCapturedData())) MessageBox(NULL,TEXT("EroareCapturare!"), TEXT("Error"), MB_OK);
			

			// Update the fields in the AVI header and close the file 
	        fseek (wavFile.pFile, 4 , SEEK_SET );
	        wavFile.wav_size += totalData;
	        fwrite((char*)&wavFile.wav_size,sizeof(int),1,wavFile.pFile);

	        wavFile.data_bytes += totalData;
	        fseek (wavFile.pFile, 40 , SEEK_SET );
	        fwrite((char*)&wavFile.data_bytes,sizeof(int),1,wavFile.pFile);

			closeWav(wavFile.pFile);  

			wavTest.Close();
			*/

			// Stop timer and free resources
			/*
			if(nTimerId)
			{
				KillTimer(hWnd,nTimerId);
				nTimerId=0;
			}
			*/
			/*
			if(pAviFile)
			{
				delete pAviFile;
				pAviFile = NULL;
			}
			*/
			/*
			if(hDesktopCompatibleDC)
				DeleteDC(hDesktopCompatibleDC);
			if(hDesktopCompatibleBitmap)
				DeleteObject(hDesktopCompatibleBitmap);
			ReleaseDC(hDesktopWnd,hDesktopDC);
			*/
			break;
		case BTN_PLAY:
			if(frame == lastframe) frame = 0;
			playVideo = true;
			break;
		case BTN_PAUSE:
			pausedVideo = playVideo;
			playVideo = !pausedVideo;
			break;
		case BTN_FASTFORWARD:
			break;
		case BTN_FASTBACKWARD:
			break;
		case IDM_OPTIUNI:
			adaugaOptiuni();
			break;
		case IDM_SAVE:
			addEffect(ordoneazaEfecte(&optiuniEditare));
			//RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE | RDW_INTERNALPAINT);
			//UpdateWindow(hWnd);
			//InvalidateRect(hWnd,NULL,false);
			//InvalidateRect(hWnd, 0, NULL);
			//RedrawWindow(hWnd,0,0,0);
			//SendMessage(hWnd,WM_PAINT,NULL,NULL);
			//CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)addEffect, reinterpret_cast<LPVOID>(ordoneazaEfecte(&optiuniEditare)), 0, NULL);
			break;
		case IDM_EXIT:
			if((MessageBox(hWnd, "Iesire ?", "Message",MB_OKCANCEL))==IDOK)
			DestroyWindow(hWnd); // POSTS THE MESSAGE WM_DESTROY TO DESTROY THE CREATED WINDOW.
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CREATE: 
		BITMAPINFO	bmpInfo;
		ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
		bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		bmpInfo.bmiHeader.biBitCount=BITSPERPIXEL;
		bmpInfo.bmiHeader.biCompression = BI_RGB;
		bmpInfo.bmiHeader.biWidth=GetSystemMetrics(SM_CXSCREEN);
		bmpInfo.bmiHeader.biHeight=GetSystemMetrics(SM_CYSCREEN);
		bmpInfo.bmiHeader.biPlanes=1;
		bmpInfo.bmiHeader.biSizeImage=abs(bmpInfo.bmiHeader.biHeight)*bmpInfo.bmiHeader.biWidth*bmpInfo.bmiHeader.biBitCount/8;

		oHDC = CreateCompatibleDC(0);

		hDesktopWnd=GetDesktopWindow();
		hDesktopDC=GetDC(hDesktopWnd);
		hDesktopCompatibleDC=CreateCompatibleDC(hDesktopDC);
		hDesktopCompatibleBitmap=CreateDIBSection(hDesktopDC,&bmpInfo,DIB_RGB_COLORS,&pBits,NULL,0);

		if(hDesktopCompatibleDC==NULL || hDesktopCompatibleBitmap == NULL)
		{
			MessageBox(NULL, "Unable to Create Desktop Compatible DC/Bitmap", "Error!",MB_ICONEXCLAMATION | MB_OK);
			return -1;
		}
		SelectObject(hDesktopCompatibleDC,hDesktopCompatibleBitmap);
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		/*
		if(playVideo == true) {
			StretchBlt(hdc,0,0,clientRect.right,clientRect.bottom,hBackDC,0,0,256,256,SRCCOPY);
		}
		*/
		if (playVideoFirst == true) {
			lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, 0);
			StretchDIBits(hdc, 50, 0, 720, 480, 0, 0, lpbi->biWidth, lpbi->biHeight, lpbi+1, (LPBITMAPINFO)lpbi, DIB_RGB_COLORS, SRCCOPY);
			frame++;
			playVideoFirst = false;
		} 
		if (playVideo == true) {
			if(frame != lastframe)
			{
				lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, frame);
				StretchDIBits(hdc, 50, 0, 720, 480, 0, 0, lpbi->biWidth, lpbi->biHeight, lpbi+1, (LPBITMAPINFO)lpbi, DIB_RGB_COLORS, SRCCOPY);
				//BitBlt(hdc, 50, 0, 720, 480, lpbi+1, 0, 0, SRCCOPY);
				frame++;
			}
			else {
				playVideo = false;
			}
		}
		if(loadedVideo == true && isEditing == true) {
			// Draw the next 5 frames to edit
				lpbiEditor = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, editFrame+0);
				StretchDIBits(hdc, 800, 30, 150, 100, 0, 0, lpbiEditor->biWidth, lpbiEditor->biHeight, lpbiEditor+1, (LPBITMAPINFO)lpbiEditor, DIB_RGB_COLORS, SRCCOPY);
				lpbiEditor = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, editFrame+1);
				StretchDIBits(hdc, 800, 140, 150, 100, 0, 0, lpbiEditor->biWidth, lpbiEditor->biHeight, lpbiEditor+1, (LPBITMAPINFO)lpbiEditor, DIB_RGB_COLORS, SRCCOPY);
				lpbiEditor = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, editFrame+2);
				StretchDIBits(hdc, 800, 250, 150, 100, 0, 0, lpbiEditor->biWidth, lpbiEditor->biHeight, lpbiEditor+1, (LPBITMAPINFO)lpbiEditor, DIB_RGB_COLORS, SRCCOPY);
				lpbiEditor = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, editFrame+3);
				StretchDIBits(hdc, 800, 360, 150, 100, 0, 0, lpbiEditor->biWidth, lpbiEditor->biHeight, lpbiEditor+1, (LPBITMAPINFO)lpbiEditor, DIB_RGB_COLORS, SRCCOPY);
				lpbiEditor = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, editFrame+4);
				StretchDIBits(hdc, 800, 470, 150, 100, 0, 0, lpbiEditor->biWidth, lpbiEditor->biHeight, lpbiEditor+1, (LPBITMAPINFO)lpbiEditor, DIB_RGB_COLORS, SRCCOPY);
				isEditing = false;
		}
		if(isAddingVideo == true) {
			lpbiAdd = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgfAdd, 0);
			StretchDIBits(hdc, 50, 600, 250, 200, 0, 0, lpbiAdd->biWidth, lpbiAdd->biHeight, lpbiAdd+1, (LPBITMAPINFO)lpbiAdd, DIB_RGB_COLORS, SRCCOPY);
			isAddingVideo = false;
		}
		EndPaint(hWnd, &ps);
		break;
	case WM_TIMER:
		{
			InvalidateRect(hWnd,NULL,false);
			/*
			if(playVideo == true) {
			GrabAVIFrame(frame);
			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hBackDC,hBackBitmap);
			BitBlt(oHDC,0,0,256,256,hBackDC,256,256,SRCCOPY|CAPTUREBLT);
			UpdateWindow(hWnd);
			SelectObject(hBackDC,hOldBitmap);
			}
			else break;
			*/
			break;
		}
	case WM_DESTROY:
		
		PostQuitMessage(1);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void ErrorExit(LPTSTR lpszFunction) 
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"), 
        lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}

HRESULT CreateCaptureBuffer(LPDIRECTSOUNDCAPTURE8 pDSC, 
                            LPDIRECTSOUNDCAPTUREBUFFER8* ppDSCB8)
{
  HRESULT hr;
  DSCBUFFERDESC               dscbd;
  LPDIRECTSOUNDCAPTUREBUFFER  pDSCB;

  // Set up WAVEFORMATEX for 44.1 kHz 16-bit stereo. 
  WAVEFORMATEX                wfx =
  {WAVE_FORMAT_PCM, wavFile.num_channels, wavFile.sample_rate, wavFile.byte_rate, wavFile.sample_alignment, wavFile.bit_depth, 0};
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

  //wavTest.Open((LPWSTR)"sunet.wav", &wfx, WAVEFILE_WRITE);
 
  if ((NULL == pDSC) || (NULL == ppDSCB8)) return E_INVALIDARG;
  dscbd.dwSize = sizeof(DSCBUFFERDESC);
  dscbd.dwFlags = 0;
  dscbd.dwBufferBytes = wfx.nAvgBytesPerSec;
  dscbd.dwReserved = 0;
  dscbd.lpwfxFormat = &wfx;
  dscbd.dwFXCount = 0;
  dscbd.lpDSCFXDesc = NULL;

  //g_dwNotifySize = MAX( 1024, wavFile.byte_rate / 8 );
  //g_dwNotifySize -= g_dwNotifySize % wavFile.sample_alignment;
  g_dwCaptureBufferSize = wavFile.byte_rate;
 
  if (SUCCEEDED(hr = pDSC->CreateCaptureBuffer(&dscbd, &pDSCB, NULL)))
  {
	/* Use the method QueryInterface of  IDirectSoundCaptureBuffer(pDSCB->QueryInterface()) to get a 
    pointer(pDSCB8) to the interface IDirectSoundCaptureBuffer8 */
    hr = pDSCB->QueryInterface(IID_IDirectSoundCaptureBuffer8, (LPVOID*)ppDSCB8);
    pDSCB->Release();  


  }
  return hr;
}

HRESULT SetCaptureNotifications(LPDIRECTSOUNDCAPTUREBUFFER8 pDSCB)
{
  LPDIRECTSOUNDNOTIFY8 pDSNotify;
  HRESULT    hr;

  if (NULL == pDSCB) return E_INVALIDARG;
  if (FAILED(hr = pDSCB->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&pDSNotify)))
  {
    return hr;
  }
  if (FAILED(hr = pDSCB->GetFormat(&wfx, sizeof(WAVEFORMATEX), NULL)))
  {
    return hr;
  }

  // Create events.
  for (int i = 0; i < cEvents; ++i)
  {
    rghEvent[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (NULL == rghEvent[i])
    {
      hr = GetLastError();
      return hr;
    }
  }
 
  // Describe notifications. 
 
  rgdsbpn[0].dwOffset = (wfx.nAvgBytesPerSec/2) -1;
  rgdsbpn[0].hEventNotify = rghEvent[0];
 
  rgdsbpn[1].dwOffset = wfx.nAvgBytesPerSec - 1;
  rgdsbpn[1].hEventNotify = rghEvent[1];
 
  rgdsbpn[2].dwOffset = DSBPN_OFFSETSTOP;
  rgdsbpn[2].hEventNotify = rghEvent[2];
 
  /* Use the SetNotificationPositions() of IDirectSoundNotify8(lpDsNotify->SetNotificationPositions())
  to set the notification buffer positions */
 
  hr = pDSNotify->SetNotificationPositions(cEvents, rgdsbpn);
  pDSNotify->Release();
  return hr;
}

HRESULT RecordCapturedData() 
    {
      HRESULT hr;
      VOID* pbCaptureData  = NULL;
      DWORD dwCaptureLength;
      VOID* pbCaptureData2 = NULL;
      DWORD dwCaptureLength2;
      VOID* pbPlayData   = NULL;
      UINT  dwDataWrote;
      DWORD dwReadPos;
      LONG lLockSize;
	  g_dwNotifySize = wavFile.byte_rate / 8;
     
      if (NULL == pDSCB8)
          MessageBox(NULL,TEXT("Empty buffer!"), TEXT("Error"), MB_OK);
      if (NULL == wavFile.pFile)
          MessageBox(NULL,TEXT("Empty .wav file!"), TEXT("Error"), MB_OK);
     
      if (FAILED (hr = pDSCB8->GetCurrentPosition( 
        NULL, &dwReadPos)))
          MessageBox(NULL,TEXT("Failed to get current position!"), TEXT("Error"), MB_OK);
     
      // Lock everything between the private cursor 
      // and the read cursor, allowing for wraparound.
     
      lLockSize = dwReadPos - g_dwNextCaptureOffset;
      if( lLockSize < 0 ) lLockSize += g_dwCaptureBufferSize;

	  // Block align lock size so that we are always write on a boundary   
	  lLockSize -= (lLockSize % g_dwNotifySize);   
     
      if( lLockSize == 0 ) return S_FALSE;
     
      if (FAILED(hr = pDSCB8->Lock( 
            g_dwNextCaptureOffset, lLockSize, 
            &pbCaptureData, &dwCaptureLength, 
            &pbCaptureData2, &dwCaptureLength2, 0L)))
        MessageBox(NULL,TEXT("Lock failed!"), TEXT("Error"), MB_OK);
     
      // Write the data. This is done in two steps
      // to account for wraparound.

	  /*
	  if (FAILED( wavTest.Write(dwCaptureLength, (BYTE*)pbCaptureData, &dwDataWrote)))
        MessageBox(NULL,TEXT("Error writting to the file!"), TEXT("Error"), MB_OK); 
     
      if (pbCaptureData2 != NULL)
      {
        if (FAILED( wavTest.Write(dwCaptureLength2, (BYTE*)pbCaptureData2, &dwDataWrote)))
        MessageBox(NULL,TEXT("Error writting to the file 2!"), TEXT("Error"), MB_OK); 
      }
	  */
      
	  if (FAILED( addData(&wavFile, (BYTE*)pbCaptureData, dwCaptureLength, &dwDataWrote)))
        MessageBox(NULL,TEXT("Error writting to the file!"), TEXT("Error"), MB_OK); 
     
      if (pbCaptureData2 != NULL)
      {
        if (FAILED( addData(&wavFile, (BYTE*)pbCaptureData2, dwCaptureLength2, &dwDataWrote)))
          MessageBox(NULL,TEXT("Error writting to the file 2!"), TEXT("Error"), MB_OK); 
      }
     
      // Unlock the capture buffer.
     
     pDSCB8->Unlock( pbCaptureData, dwCaptureLength, 
        pbCaptureData2, dwCaptureLength2  );
      
      // Move the capture offset forward.
     
      g_dwNextCaptureOffset += dwCaptureLength; 
      g_dwNextCaptureOffset %= g_dwCaptureBufferSize; 
      g_dwNextCaptureOffset += dwCaptureLength2; 
      g_dwNextCaptureOffset %= g_dwCaptureBufferSize; 
     
	  totalData += (dwCaptureLength + dwCaptureLength2);
      return S_OK;
    }

bool BuildControls(HWND parent) {
	HGDIOBJ hObj = GetStockObject(DEFAULT_GUI_FONT);;
	int x, y, indentareX = 10, indentareY = 40;
	SIZE size;
	RECT rect;
	HDC hdc;
	int iResult;

	x = 10;
	y = 10;

	/*  
	*********
	Executable
	*********
	*/
	hwndStaticInjector = CreateWindow( 
    (LPCSTR)"Static",  // Predefined class; Unicode assumed 
    (LPCSTR)"",      // Button text 
    WS_CHILD | WS_VISIBLE | WS_TABSTOP,  // Styles 
    x,         // x position 
    y,         // y position 
    60,        // Button width
    40,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
	hInst, 
    NULL);      // Pointer not needed.

	hdc = GetDC(hwndStaticInjector);
	iResult=GetTextExtentPoint32(hdc, "Target executable:", sizeof("Target executable:"), &size);

	if(iResult!=0)
	{
		//SendDlgItemMessage(hwndStaticInjector, 0, SS_REALSIZEIMAGE , size.cx, NULL);
		SetWindowPos(hwndStaticInjector,NULL,x,y,size.cx,size.cy,SWP_SHOWWINDOW);
		SetWindowText(hwndStaticInjector, "Target executable:");
		//SendMessage(hwndStaticInjector, WM_SETFONT, (WPARAM)hObj, true);
	}

	x += size.cx + indentareX;

	hwndButtonExe = CreateWindow( 
    "Button",  // Predefined class; Unicode assumed 
    "Select",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    x,         // x position 
    y,         // y position 
    160,        // Button width
    20,        // Button height
    hWnd,     // Parent window
    ( HMENU )BTN_CHOOSE,       // No menu.
    hInst, 
    NULL);      // Pointer not needed.

	x = 10;
	y += indentareY;

	/*  
	**********************
	Command Line arguments 
	**********************
	*/
	hwndStaticCmdla = CreateWindow( 
    (LPCSTR)"Static",  // Predefined class; Unicode assumed 
    (LPCSTR)"",      // Button text 
    WS_CHILD | WS_VISIBLE | WS_TABSTOP,  // Styles 
    x,         // x position 
    y,         // y position 
    60,        // Button width
    40,        // Button height
    parent,     // Parent window
    NULL,       // No menu.
	hInst, 
    NULL);      // Pointer not needed.

	hdc = GetDC(hwndStaticCmdla);
	iResult=GetTextExtentPoint32(hdc, "Command line arguments:", sizeof("Command line arguments:"), &size);

	if(iResult!=0)
	{
		//SendDlgItemMessage(hwndStaticInjector, 0, SS_REALSIZEIMAGE , size.cx, NULL);
		SetWindowPos(hwndStaticCmdla,NULL,x,y,size.cx,size.cy,SWP_SHOWWINDOW);
		SetWindowText(hwndStaticCmdla, "Command line arguments:");
		//SendMessage(hwndStaticInjector, WM_SETFONT, (WPARAM)hObj, true);
	}

	x += size.cx + indentareX;

	hwndEditCmdla = CreateWindow("edit", "",
                              WS_CHILD | WS_VISIBLE | WS_TABSTOP
                              | ES_LEFT | WS_BORDER,
                              x, y, 160, size.cy,
                              hWnd, NULL,
							  hInst, NULL);

	SetWindowText(hwndEditCmdla, "");

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
    "Button",  // Predefined class; Unicode assumed 
    "START",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    x,         // x position 
    y,         // y position 
    160,        // Button width
    20,        // Button height
    hWnd,     // Parent window
    ( HMENU )BTN_INJECT,       // No menu.
    hInst, 
    NULL);      // Pointer not needed.


	x = 10;
	y += indentareY;



	return true;
}

void openDialogBox() {
	COMDLG_FILTERSPEC aFileTypes[] = {
    { L"Executable files", L"*.exe" }
	};
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			if (SUCCEEDED(hr)) {
				 IFileOpenDialog *pFileOpen;

				// Create the FileOpenDialog object.
				hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, 
                IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

				if (SUCCEEDED(hr)) {
					// Show the Open dialog box.
					pFileOpen->SetTitle(L"Choose a game");
					pFileOpen->SetFileTypes ( _countof(aFileTypes), aFileTypes );
					hr = pFileOpen->Show(NULL);

					// Get the file name from the dialog box.
					if (SUCCEEDED(hr)) {
						IShellItem *pItem;
						hr = pFileOpen->GetResult(&pItem);

						if (SUCCEEDED(hr)) {
							PWSTR pszFilePath;
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

							// Store the path to the executable to be injected
							if (SUCCEEDED(hr)) {
								WideCharToMultiByte(CP_ACP,0,pszFilePath,-1,exename,sizeof(exename),NULL,NULL);
								CoTaskMemFree(pszFilePath);
							}
						}
					}
				}
			}
}

void addVideo() {
	// merge videos
	LONG plPos = AVIStreamEnd(editablePavi);
	LONG plLength = AVIStreamLength(paviAdd);
	EditStreamPaste(editablePavi,&plPos,&plLength,paviAdd,AVIStreamStart(paviAdd),AVIStreamEnd(paviAdd));
	if(AVISave("Output3.avi", NULL, NULL, 1, editablePavi, &m_AviCompressOptions) != AVIERR_OK) MessageBoxA(NULL, "Failed to save the edited video!", "Error", MB_OK);
}

void prepareAddVideo() {
	COMDLG_FILTERSPEC aFileTypes[] = {
    { L"Executable files", L"*.avi" }
	};
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
			if (SUCCEEDED(hr)) {
				 IFileOpenDialog *pFileOpen;

				// Create the FileOpenDialog object.
				hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, 
                IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

				if (SUCCEEDED(hr)) {
					// Show the Open dialog box.
					pFileOpen->SetTitle(L"Choose a video");
					pFileOpen->SetFileTypes ( _countof(aFileTypes), aFileTypes );
					hr = pFileOpen->Show(NULL);

					// Get the file name from the dialog box.
					if (SUCCEEDED(hr)) {
						IShellItem *pItem;
						hr = pFileOpen->GetResult(&pItem);

						if (SUCCEEDED(hr)) {
							PWSTR pszFilePath;
							hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

							// Store the path to the executable to be injected
							if (SUCCEEDED(hr)) {
								WideCharToMultiByte(CP_ACP,0,pszFilePath,-1,addVideoPath,sizeof(addVideoPath),NULL,NULL);
								CoTaskMemFree(pszFilePath);
							}
						}
					}
				}
			}

	if (AVIStreamOpenFromFile(&paviAdd, addVideoPath, streamtypeVIDEO, 0, OF_READ, NULL) !=0)
    {
        MessageBox (HWND_DESKTOP, "Failed To Open The AVI Stream", "Error", MB_OK | MB_ICONEXCLAMATION);
    }
	AVIStreamInfo(paviAdd, &psiAdd, sizeof(psiAdd));    
	widthAdd=psiAdd.rcFrame.right-psiAdd.rcFrame.left;           
	heightAdd=psiAdd.rcFrame.bottom-psiAdd.rcFrame.top; 
	lastFrameAdd=AVIStreamLength(paviAdd);  
	pgfAdd=AVIStreamGetFrameOpen(paviAdd, NULL);              // Create The PGETFRAME Using Our Request Mode
	if (pgfAdd==NULL)
	{
		// An Error Occurred Opening The Frame
		MessageBox (HWND_DESKTOP, "Failed To Open The AVI Frame", "Error", MB_OK | MB_ICONEXCLAMATION);
	}
	isAddingVideo = true;
}

bool beginEdit(HWND parent) {
	DestroyWindow(hwndStaticInjector);
	DestroyWindow(hwndButtonExe);
	DestroyWindow(hwndStaticCmdla);
    DestroyWindow(hwndEditCmdla);
	DestroyWindow(hwndButtonInjector);
	SetWindowPos(hWnd,HWND_TOP,200,50,1000,1000,SWP_SHOWWINDOW);

	// Structure for control initialization.
	INITCOMMONCONTROLSEX icex; 
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	// Create the slider
	hwndTrack = CreateWindowEx( 
        0,                               // no extended styles 
        TRACKBAR_CLASS,                  // class name 
        (LPCSTR)"Trackbar Control",              // title (caption) 
        WS_CHILD | 
        WS_VISIBLE | 
        TBS_AUTOTICKS | 
        TBS_ENABLESELRANGE,              // style 
        50, 550,                          // position 
        720, 30,                         // size 
        parent,                         // parent window 
        (HMENU)ID_TRACKBAR,                     // control identifier 
        hInst,                         // instance 
        NULL                             // no WM_CREATE parameter 
        ); 

	/* Create the Video Player buttons */
	hwndButtonPlay = CreateWindow( 
    "Button",  // Predefined class; Unicode assumed 
    "Play",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    70,         // x position 
    500,         // y position 
    50,        // Button width
    40,        // Button height
    hWnd,     // Parent window
    ( HMENU )BTN_PLAY,       // No menu.
    hInst, 
    NULL);      // Pointer not needed.

	hwndButtonPause = CreateWindow( 
    "Button",  // Predefined class; Unicode assumed 
    "Pause",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    130,         // x position 
    500,         // y position 
    50,        // Button width
    40,        // Button height
    hWnd,     // Parent window
    ( HMENU )BTN_PAUSE,       // No menu.
    hInst, 
    NULL);      // Pointer not needed.

	hwndButtonFastForward = CreateWindow( 
    "Button",  // Predefined class; Unicode assumed 
    "Bwrd",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    190,         // x position 
    505,         // y position 
    50,        // Button width
    30,        // Button height
    hWnd,     // Parent window
    ( HMENU )BTN_PAUSE,       // No menu.
    hInst, 
    NULL);      // Pointer not needed.

	hwndButtonFastBackward = CreateWindow( 
    "Button",  // Predefined class; Unicode assumed 
    "Fwrd",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    250,         // x position 
    505,         // y position 
    50,        // Button width
    30,        // Button height
    hWnd,     // Parent window
    ( HMENU )BTN_PAUSE,       // No menu.
    hInst, 
    NULL);      // Pointer not needed.

	// Create the 4 Checkboxes
	hwndCheckFrame0 = CreateWindow( 
    "Button", 
    "",      
    WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,  
    780,        
    30,         
    15,        
    15,        
    hWnd,     
    ( HMENU )CHK_FRAME0,       
    hInst, 
    NULL); 

	hwndCheckFrame1 = CreateWindow( 
    "Button", 
    "",      
    WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,  
    780,        
    140,         
    15,        
    15,        
    hWnd,     
    ( HMENU )CHK_FRAME1,       
    hInst, 
    NULL);

	hwndCheckFrame2 = CreateWindow( 
    "Button", 
    "",      
    WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,  
    780,        
    250,         
    15,        
    15,        
    hWnd,     
    ( HMENU )CHK_FRAME2,       
    hInst, 
    NULL);

	hwndCheckFrame3 = CreateWindow( 
    "Button", 
    "",      
    WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,  
    780,        
    360,         
    15,        
    15,        
    hWnd,     
    ( HMENU )CHK_FRAME3,       
    hInst, 
    NULL);

	hwndCheckFrame4 = CreateWindow( 
    "Button", 
    "",      
    WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,  
    780,        
    470,         
    15,        
    15,        
    hWnd,     
    ( HMENU )CHK_FRAME4,       
    hInst, 
    NULL);

	hwndButtonPreviousFrames =  CreateWindow( 
    "Button",  // Predefined class; Unicode assumed 
    "<",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    800,         // x position 
    590,         // y position 
    30,        // Button width
    30,        // Button height
    hWnd,     // Parent window
    ( HMENU )BTN_FRAMEBACK,       // No menu.
    hInst, 
    NULL);      // Pointer not needed.

	hwndButtonNextFrames =  CreateWindow( 
    "Button",  // Predefined class; Unicode assumed 
    ">",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    830,         // x position 
    590,         // y position 
    30,        // Button width
    30,        // Button height
    hWnd,     // Parent window
    ( HMENU )BTN_FRAMEFWRD,       // No menu.
    hInst, 
    NULL);      // Pointer not needed.

	hwndButtonDelFrames =  CreateWindow( 
    "Button",  // Predefined class; Unicode assumed 
    "Del",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    860,         // x position 
    590,         // y position 
    50,        // Button width
    30,        // Button height
    hWnd,     // Parent window
    ( HMENU )BTN_DELFRAMES,       // No menu.
    hInst, 
    NULL);      // Pointer not needed.

	hwndStaticFrom = CreateWindow("STATIC",
                             "From",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             800,
                             640,
                             30,
                             20,
                             hWnd,
                             NULL,
                             hInst,
                             NULL);
	hwndEditFrom = CreateWindow("edit", NULL,
         WS_CHILD | WS_VISIBLE | WS_BORDER ,
             840, 640, 30, 20, hWnd, (HMENU) EDIT_FROM, hInst, NULL);
	hwndStaticTo = CreateWindow("STATIC",
                             "To",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             880,
                             640,
                             30,
                             20,
                             hWnd,
                             NULL,
                             hInst,
                             NULL);
	hwndEditTo = CreateWindow("edit", NULL,
         WS_CHILD | WS_VISIBLE | WS_BORDER ,
             920, 640, 30, 20, hWnd, (HMENU) EDIT_FROM, hInst, NULL);

	hwndEditVideoStatic0 = CreateWindow("STATIC",
                             "Duration",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             320,
                             600,
                             60,
                             20,
                             hWnd,
                             NULL,
                             hInst,
                             NULL);

	hwndEditVideoStatic1 = CreateWindow("STATIC",
                             "Codec",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             320,
                             630,
                             60,
                             20,
                             hWnd,
                             NULL,
                             hInst,
                             NULL);

	hwndEditVideoStatic2 = CreateWindow("STATIC",
                             "Size",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             320,
                             660,
                             60,
                             20,
                             hWnd,
                             NULL,
                             hInst,
                             NULL);

	hwndEditVideoStatic3 = CreateWindow("STATIC",
                             "Output format",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             50,
                             820,
                             120,
                             20,
                             hWnd,
                             NULL,
                             hInst,
                             NULL);

	hwndEditVideoStatic4 = CreateWindow("STATIC",
                             "Output file",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             50,
                             850,
                             120,
                             20,
                             hWnd,
                             NULL,
                             hInst,
                             NULL);

	hwndEditEdit0 = CreateWindow("edit", NULL,
         WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
             180, 820, 300, 20, hWnd, (HMENU) EDIT_FROM, hInst, NULL);

	hwndEditListView = CreateWindow("edit", NULL,
         WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
             180, 850, 300, 20, hWnd, (HMENU) EDIT_FROM, hInst, NULL);

	//hwndButtonOutput, hwndButtonBrowse, hwndButtonMerge;
	hwndButtonOutput =  CreateWindow( 
    "Button",  // Predefined class; Unicode assumed 
    "Output Settings",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    490,         // x position 
    820,         // y position 
    110,        // Button width
    20,        // Button height
    hWnd,     // Parent window
    ( HMENU )BTN_OUTPUT,       // No menu.
    hInst, 
    NULL);      // Pointer not needed.

	hwndButtonBrowse =  CreateWindow( 
    "Button",  // Predefined class; Unicode assumed 
    "Browse",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    490,         // x position 
    850,         // y position 
    60,        // Button width
    20,        // Button height
    hWnd,     // Parent window
    ( HMENU )BTN_OUTPUT,       // No menu.
    hInst, 
    NULL);      // Pointer not needed.

	hwndButtonMerge =  CreateWindow( 
    "Button",  // Predefined class; Unicode assumed 
    "Start",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    610,         // x position 
    820,         // y position 
    100,        // Button width
    50,        // Button height
    hWnd,     // Parent window
    ( HMENU )BTN_MERGE,       // No menu.
    hInst, 
    NULL);      // Pointer not needed.

	hwndButtonMerge =  CreateWindow( 
    "Button",  // Predefined class; Unicode assumed 
    "Add",      // Button text 
    WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    660,         // x position 
    600,         // y position 
    50,        // Button width
    50,        // Button height
    hWnd,     // Parent window
    ( HMENU )BTN_EFFECTS,       // No menu.
    hInst, 
    NULL);      // Pointer not needed.

	hWndComboBox = CreateWindow(WC_COMBOBOX, TEXT(""), 
     CBS_DROPDOWN | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
     500, 600, 152, 200, hWnd, (HMENU)IDC_ALEGEFECT, hInst,NULL);
	if (!hWndComboBox) MessageBox(NULL, "ComboBox Failed.", "Error", MB_OK | MB_ICONERROR);


	const TCHAR* ComboBoxItems[] = { _T( "Grayscale" ), _T( "Vertix" )};

	wchar_t A[2]; 
	int  k = 0; 

	memset(&A,0,sizeof(A));   

	for (k = 0; k <= 1; k++)
	{
		//wcscpy_s(A, sizeof(A)/sizeof(wchar_t),  (const wchar_t *)ComboBoxItems[k]);

		// Add string to combobox.
		SendMessage(hWndComboBox,(UINT) CB_ADDSTRING,(WPARAM) 0,(LPARAM) ComboBoxItems[k]); 
	}

	SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)1, (LPARAM)0);

	hwndStaticFromEffect = CreateWindow("STATIC",
                             "From",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             500,
                             640,
                             30,
                             20,
                             hWnd,
                             NULL,
                             hInst,
                             NULL);
	hwndEditFromEffect = CreateWindow("edit", NULL,
         WS_CHILD | WS_VISIBLE | WS_BORDER ,
             540, 640, 30, 20, hWnd, (HMENU) EDIT_FROM_EFFECT, hInst, NULL);
	hwndStaticToEffect = CreateWindow("STATIC",
                             "To",
                             WS_VISIBLE | SS_LEFT | WS_CHILD,
                             580,
                             640,
                             30,
                             20,
                             hWnd,
                             NULL,
                             hInst,
                             NULL);
	hwndEditToEffect = CreateWindow("edit", NULL,
         WS_CHILD | WS_VISIBLE | WS_BORDER ,
             620, 640, 30, 20, hWnd, (HMENU) EDIT_FROM_EFFECT, hInst, NULL);


	/*
	// Create the List View
	HWND hWndListView = CreateWindow(WC_LISTVIEW, 
                                     (LPCSTR)"",
                                     WS_CHILD | LVS_REPORT | LVS_EDITLABELS,
                                     850, 50,
                                     100,
                                     100,
                                     parent,
                                     (HMENU)IDM_LIST,
									 hInst,
                                     NULL); 

	// Create ImageList
	HIMAGELIST imgList = ImageList_Create(100,100,ILC_COLOR,4,10);

	// Add the ImageList to the List View
	ListView_SetImageList(hWndListView, imgList, LVSIL_NORMAL); 
	*/

	/* Creeaza fisierul temporar unde tin datele despre efecte */

	if(FAILED(creeazaFisier(&optiuniEditare))) MessageBoxA(NULL, "Eroare la crearea OptiuniEditare", "Error", MB_OK);

	//Open the AVI file for play 
	OpenAVI("Output.avi");
	loadedVideo = true;
	isEditing = true;
	return true;
}

void OpenAVI(LPCSTR szFile) {
	AVIFileInit();
	// Opens The AVI Stream
    if (AVIStreamOpenFromFile(&pavi, szFile, streamtypeVIDEO, 0, OF_READ, NULL) !=0)
    {
        // An Error Occurred Opening The Stream
        MessageBox (HWND_DESKTOP, "Failed To Open The AVI Stream", "Error", MB_OK | MB_ICONEXCLAMATION);
    }
	AVIStreamInfo(pavi, &psi, sizeof(psi));             // Reads Information About The Stream Into psi
	width=psi.rcFrame.right-psi.rcFrame.left;           // Width Is Right Side Of Frame Minus Left
	height=psi.rcFrame.bottom-psi.rcFrame.top;          // Height Is Bottom Of Frame Minus Top
 
	lastframe=AVIStreamLength(pavi);                // The Last Frame Of The Stream
	SendMessage(hwndTrack, TBM_SETRANGE, 0, MAKELONG(0, lastframe));
 
	mpf=AVIStreamSampleToTime(pavi,lastframe)/lastframe;        // Calculate Rough Milliseconds Per Frame

	bmih.biSize     = sizeof (BITMAPINFOHEADER);        // Size Of The BitmapInfoHeader
	bmih.biPlanes       = 1;                    // Bitplanes
	bmih.biBitCount     = 24;                   // Bits Format We Want (24 Bit, 3 Bytes)
	bmih.biWidth        = 720;                  // Width We Want (256 Pixels)
	bmih.biHeight       = 480;                  // Height We Want (256 Pixels)
	bmih.biCompression  = BI_RGB;               // Requested Mode = RGB
 
	hBackBitmap = CreateDIBSection (oHDC, (BITMAPINFO*)(&bmih), DIB_RGB_COLORS, (void**)(&data), NULL, NULL);
	SelectObject (oHDC, hBackBitmap);                    // Select hBitmap Into Our Device Context (hdc)

	pgf=AVIStreamGetFrameOpen(pavi, NULL);              // Create The PGETFRAME Using Our Request Mode
	if (pgf==NULL)
	{
		// An Error Occurred Opening The Frame
		MessageBox (HWND_DESKTOP, "Failed To Open The AVI Frame", "Error", MB_OK | MB_ICONEXCLAMATION);
	}
	//hdd = DrawDibOpen(); 
	nTimerId=SetTimer(hWnd,12345,GetTargetFR() / 2,NULL);	// mpf/2
	//Create editable stream
	if(CreateEditableStream(&editablePavi, pavi) != 0) MessageBox (HWND_DESKTOP, "Failed creating editable stream!", "Error", MB_OK | MB_ICONEXCLAMATION);
	ZeroMemory(&m_AviCompressOptions,sizeof(AVICOMPRESSOPTIONS));
	m_AviCompressOptions.fccType=streamtypeVIDEO;
	m_AviCompressOptions.fccHandler=psi.fccHandler;
	m_AviCompressOptions.dwFlags=psi.dwFlags;//|AVICOMPRESSF_DATARATE;
	m_AviCompressOptions.dwKeyFrameEvery=1;
	playVideoFirst = true;
}

void GrabAVIFrame(int frame)                        // Grabs A Frame From The Stream
{
    lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, frame);   // Grab Data From The AVI Stream
    pdata=(char *)lpbi+lpbi->biSize+lpbi->biClrUsed * sizeof(RGBQUAD);    // Pointer To Data Returned By AVIStreamGetFrame
                                        // (Skip The Header Info To Get To The Data)
    // Convert Data To Requested Bitmap Format
    //DrawDibDraw (hdd, oHDC, 0, 0, 256, 256, lpbi, pdata, 0, 0, width, height, 0);
	frame++;
}

HRESULT InitAudio() {
	HRESULT hr = E_FAIL;
    REFERENCE_TIME hnsRequestedDuration;
    UINT32 bufferFrameCount;
    IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDeviceCollection* pIMMDeviceCollection = NULL;
    IMMDevice *pDevice = NULL;

	_AudioSamplesReadyEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
	_ShutdownEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);

	WAVEFORMATEX                wfx =
  {WAVE_FORMAT_PCM, wavFile.num_channels, wavFile.sample_rate, wavFile.byte_rate, wavFile.sample_alignment, wavFile.bit_depth, 0};
    // wFormatTag, nChannels, nSamplesPerSec, mAvgBytesPerSec,
    // nBlockAlign, wBitsPerSample, cbSize

	hr = CoCreateInstance(
           CLSID_MMDeviceEnumerator, NULL,
           CLSCTX_ALL, IID_IMMDeviceEnumerator,
           (void**)&pEnumerator);

    //hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice);
	hr = pEnumerator->EnumAudioEndpoints(eCapture,DEVICE_STATE_ACTIVE,&pIMMDeviceCollection);
	hr = pIMMDeviceCollection->Item(GetComboBoxAID(), &pDevice);

    hr = pDevice->Activate(
                    IID_IAudioClient, CLSCTX_ALL,
                    NULL, (void**)&pAudioClient);

	hr = pAudioClient->GetDevicePeriod(NULL, &hnsRequestedDuration);
    hr = pAudioClient->Initialize(
                         AUDCLNT_SHAREMODE_SHARED,
                         AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
                         hnsRequestedDuration,
                         0,
                         &wfx,
                         NULL);

    // Get the size of the allocated buffer.
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);

	hr = pAudioClient->SetEventHandle(_AudioSamplesReadyEvent);
	if (FAILED(hr)) MessageBox(NULL,TEXT("Unable to set event handle!"), TEXT("Error"), MB_OK);

    hr = pAudioClient->GetService(
                         IID_IAudioCaptureClient,
                         (void**)&pCaptureClient);

	ProcessSoundThread = CreateThread(NULL, 0, ProcessSound, 0, 0, NULL);

    hr = pAudioClient->Start();  // Start recording.

	//BYTE *tempBuffer = new BYTE[wavFile.byte_rate];

	return hr;
}

DWORD WINAPI ProcessSound(void* arg) {
	UINT32 numFramesAvailable;
	UINT32 packetLength = 0;
	UINT32 dataSize = 0;
    BYTE *pData;
	DWORD  flags;
	HRESULT hr;
	UINT dwDataWrote;

	HANDLE waitArray[2] = { _AudioSamplesReadyEvent, _ShutdownEvent};
	HANDLE mmcssHandle = NULL;
    DWORD mmcssTaskIndex = 0;

	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) MessageBox(NULL,TEXT("Unable to initialize COM in render thread!"), TEXT("Error"), MB_OK);

	mmcssHandle = AvSetMmThreadCharacteristics("Audio", &mmcssTaskIndex);
	if(mmcssHandle == NULL) MessageBox(NULL,TEXT("Unable to enable MMCSS on capture thread!"), TEXT("Error"), MB_OK);

	while(captureazaSunet) {
		DWORD waitResult = WaitForMultipleObjects(2, waitArray, FALSE, INFINITE);
		switch (waitResult)
			{
			case WAIT_OBJECT_0 + 0:
				hr = pCaptureClient->GetBuffer(&pData,&numFramesAvailable,&flags,NULL,NULL);
				if(hr != AUDCLNT_S_BUFFER_EMPTY) {
					dataSize = numFramesAvailable * wavFile.sample_alignment; // wavFile.sample_alignment = frameSize = num_channels * (pWav->bit_depth)/8
					if (FAILED( addData(&wavFile, pData, dataSize, &dwDataWrote)))
						MessageBox(NULL,TEXT("Error writting to the file!"), TEXT("Error"), MB_OK); 
					totalData += dataSize;
				}
				pCaptureClient->ReleaseBuffer(numFramesAvailable);
				break;
			case WAIT_OBJECT_0 + 1:   
				captureazaSunet = false;
				fseek (wavFile.pFile, 4 , SEEK_SET );
				wavFile.wav_size += totalData;
				fwrite((char*)&wavFile.wav_size,sizeof(int),1,wavFile.pFile);

				wavFile.data_bytes += totalData;
				fseek (wavFile.pFile, 40 , SEEK_SET );
				fwrite((char*)&wavFile.data_bytes,sizeof(int),1,wavFile.pFile);

				closeWav(wavFile.pFile);  
				break;
			}
	}
	return 0;
}

void salveazaEfecte() { 
	int ItemIndex = SendMessage(hWndComboBox, (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);
	char* ListItem;
	if (ItemIndex == 0) ListItem = "GRAY";
	else if (ItemIndex == 1) ListItem = "REDD";

	LPSTR lpstrFromEffect, lpstrToEffect;int lungime;

	lungime = SendMessage(hwndEditFromEffect, WM_GETTEXTLENGTH, 0, 0);
	lpstrFromEffect = (LPSTR)malloc(lungime+1);
	GetWindowText(hwndEditFromEffect, lpstrFromEffect, lungime + 1);

	lungime = SendMessage(hwndEditToEffect, WM_GETTEXTLENGTH, 0, 0);
	lpstrToEffect = (LPSTR)malloc(lungime+1);
	GetWindowText(hwndEditToEffect, lpstrToEffect, lungime + 1);

	adaugaEfect(&optiuniEditare, ListItem ,atoi(lpstrFromEffect), atoi(lpstrToEffect));
}

void efectGray(BYTE* pPixelSrc, long awidth,long aheight) {
	for(int j = 0; j < awidth * aheight * 4; j+=4) {
				//BYTE gray = BYTE(0.30 * pPixelSrc[j]) + BYTE(0.59 * pPixelSrc[j+1]) + BYTE(0.11 * pPixelSrc[j+2]); LUMINOSITY
				//BYTE gray = BYTE((MAX(pPixelSrc[j],pPixelSrc[j+1],pPixelSrc[j+2]) + MIN(pPixelSrc[j],pPixelSrc[j+1],pPixelSrc[j+2])) / 2); // LIGHTNESS
				BYTE gray = BYTE((pPixelSrc[j] + pPixelSrc[j+1] + pPixelSrc[j+2]) / 3); // AVERAGE
				pPixelSrc[j] = gray;
				pPixelSrc[j+1] = gray;
				pPixelSrc[j+2] = gray;
		}
}

void efectRedd(BYTE* pPixelSrc, long awidth,long aheight) {
	for(int j = 0; j < awidth * aheight * 4; j+=4) {
				pPixelSrc[j] *= 0.30;
				pPixelSrc[j+1] *= 0.59;
				pPixelSrc[j+2] *= 0.11;
		}
}

void efectText(BYTE* pPixelSrc, long awidth,long aheight, char* text, BITMAPINFO* pbmi) {
	auto hdc = GetDC(0);
	auto memdc = CreateCompatibleDC(hdc);
	auto hbitmap = CreateBitmap(width, -height, 1, 32, pPixelSrc);
	auto oldbmp = SelectObject(memdc, hbitmap);
	//SetBkMode(memdc, TRANSPARENT);
	TextOut(memdc, 0, 0, "1235", 4);
	SelectObject(memdc, oldbmp);
	GetDIBits(memdc, hbitmap, 0, height, pPixelSrc, pbmi, 0);
}

void addEffect(efect* efecte) {
	//efect* efecte = reinterpret_cast<efect*>(efectee);
	PAVIFILE m_pAviFile = NULL;
	PAVISTREAM paviTemp = NULL;
	AVISTREAMINFO m_AviStreamInfo;
	AVICOMPRESSOPTIONS	m_AviCompressOptionss;
	PAVISTREAM m_pAviCompressedStream;

	PGETFRAME       pgfEffect; 
	LONG plStart = 0; 
	LONG plLength = AVIStreamLength(pavi);

	char nume = (char)"grayscale.avi";
	TCHAR		m_szFileName[260] = _T("OutputModificat.Avi");		// Holds the Output Movie File Name
	HRESULT hr = creeazaAvi(width,height,32,&m_pAviFile,m_szFileName,&m_AviStreamInfo,&paviTemp,&m_AviCompressOptionss,&m_pAviCompressedStream);

	int nrEfecteRamase = 0;

	for(int i = plStart; i < plLength; i++) {
		BITMAPINFO* pbmi;
		pbmi = (BITMAPINFO*)AVIStreamGetFrame(pgf, i);
		BYTE *pPixelSrc = (sizeof(BITMAPINFO) + (BYTE*)pbmi);
		long awidth,aheight;
		awidth = *((long*)(((BYTE*)pbmi)+4));
		aheight = *((long*)(((BYTE*)pbmi)+8));

		if(i >= efecte[nrEfecteRamase].frameInceput && i <= efecte[nrEfecteRamase].frameSfarsit && nrEfecteRamase < optiuniEditare.nr_efecte) {
			if (strncmp(efecte[nrEfecteRamase].numeEfect, "GRAY", 4) == 0) {
				efectGray(pPixelSrc,awidth,aheight);
			}
			else if (strncmp(efecte[nrEfecteRamase].numeEfect,"REDD", 4) == 0) {
				efectRedd(pPixelSrc,awidth,aheight); 
			}

			if(i == efecte[nrEfecteRamase].frameSfarsit) {
				nrEfecteRamase++;
			}
		}

		/*EFECT NR 1 GRAY 
		for(int j = 0; j < awidth * aheight * 4; j+=4) {
				//BYTE gray = BYTE(0.30 * pPixelSrc[j]) + BYTE(0.59 * pPixelSrc[j+1]) + BYTE(0.11 * pPixelSrc[j+2]); LUMINOSITY
				//BYTE gray = BYTE((MAX(pPixelSrc[j],pPixelSrc[j+1],pPixelSrc[j+2]) + MIN(pPixelSrc[j],pPixelSrc[j+1],pPixelSrc[j+2])) / 2); // LIGHTNESS
				BYTE gray = BYTE((pPixelSrc[j] + pPixelSrc[j+1] + pPixelSrc[j+2]) / 3); // AVERAGE
				pPixelSrc[j] = gray;
				pPixelSrc[j+1] = gray;
				pPixelSrc[j+2] = gray;
		}
		*/

		/*EFECT NR 2 REDD */

		/* EFECT TEXT
		auto hdc = GetDC(0);
		auto memdc = CreateCompatibleDC(hdc);
		auto hbitmap = CreateBitmap(width, height, 1, 32, pPixelSrc);
		auto oldbmp = SelectObject(memdc, hbitmap);
		//SetBkMode(memdc, TRANSPARENT);
		TextOut(memdc, 0, 0, "1235", 4);
		SelectObject(memdc, oldbmp);
		GetDIBits(memdc, hbitmap, 0, height, pPixelSrc, pbmi, 0);
		*/
		
		// adauga frame in video
		LONG       plSampWritten = 0; LONG plBytesWritten = 0;
		HRESULT hr = AVIStreamWrite(m_pAviCompressedStream,i,1,pPixelSrc,awidth * aheight, 0, &plSampWritten, &plBytesWritten);
		if (hr!=AVIERR_OK) MessageBoxA(NULL, "Failed to write sample to stream", "Error", MB_OK);
	}
	
	AVIStreamRelease(m_pAviCompressedStream); m_pAviCompressedStream = NULL;
	AVIStreamRelease(paviTemp); paviTemp = NULL;
	AVIFileRelease(m_pAviFile); m_pAviFile = NULL;

	DestroyWindow(hwndTrack);DestroyWindow(hwndButtonPlay);DestroyWindow(hwndButtonPause);DestroyWindow(hwndButtonFastForward);DestroyWindow(hwndButtonFastBackward);
DestroyWindow(hwndCheckFrame0);DestroyWindow(hwndCheckFrame1);DestroyWindow(hwndCheckFrame2);DestroyWindow(hwndCheckFrame3);DestroyWindow(hwndCheckFrame4);
DestroyWindow(hwndButtonNextFrames);DestroyWindow(hwndButtonPreviousFrames);DestroyWindow(hwndButtonDelFrames);
DestroyWindow(hwndEditVideoStatic0);DestroyWindow(hwndEditVideoStatic1);DestroyWindow(hwndEditVideoStatic2);DestroyWindow(hwndEditVideoStatic3);DestroyWindow(hwndEditVideoStatic4);
DestroyWindow(hwndEditEdit0);DestroyWindow(hwndEditListView);
DestroyWindow(hwndButtonOutput);DestroyWindow(hwndButtonBrowse);DestroyWindow(hwndButtonMerge);DestroyWindow(hWndComboBox);DestroyWindow(hwndStaticFromEffect);DestroyWindow(hwndEditFromEffect);
DestroyWindow(hwndStaticToEffect);DestroyWindow(hwndEditToEffect);
	AVIStreamRelease(pavi);
	beginEdit(hWnd);

	//loadedVideo = true;
	//isEditing = true;
}

HRESULT creeazaAvi(int nFrameWidth, int nFrameHeight, int nBitsPerPixel,PAVIFILE* ppAviFile, TCHAR* m_szFileName,AVISTREAMINFO* m_AviStreamInfo, PAVISTREAM* m_pAviStream, AVICOMPRESSOPTIONS* m_AviCompressOptions, PAVISTREAM* m_pAviCompressedStream) {
	int nMaxWidth=GetSystemMetrics(SM_CXSCREEN),nMaxHeight=GetSystemMetrics(SM_CYSCREEN);
	if(nFrameWidth > nMaxWidth)	nMaxWidth= nFrameWidth;
	if(nFrameHeight > nMaxHeight)	nMaxHeight = nFrameHeight;

	DWORD	m_dwFCCHandler = mmioFOURCC('X','2','6','4');
	DWORD	m_dwFrameRate = GetTargetFR();				

	HRESULT hr = S_OK;
	if(FAILED(AVIFileOpen(ppAviFile,(LPCSTR) m_szFileName, OF_CREATE|OF_WRITE, NULL)))
	{
		MessageBoxA(NULL, "Unable to Create the Movie File", "Error", MB_OK);
		return E_FAIL;
	}

	ZeroMemory(m_AviStreamInfo,sizeof(AVISTREAMINFO));
	m_AviStreamInfo->fccType		= streamtypeVIDEO;
	m_AviStreamInfo->fccHandler	= m_dwFCCHandler;
	m_AviStreamInfo->dwScale		= 1;
	m_AviStreamInfo->dwRate		= m_dwFrameRate;	// Frames Per Second;
	m_AviStreamInfo->dwQuality	= -1;				// Default Quality
	m_AviStreamInfo->dwSuggestedBufferSize = nMaxWidth*nMaxHeight*4;
    SetRect(&m_AviStreamInfo->rcFrame, 0, 0, nFrameWidth, nFrameHeight);
	_tcscpy(m_AviStreamInfo->szName, _T("Video Stream"));

	if(FAILED(AVIFileCreateStream(*ppAviFile,m_pAviStream,m_AviStreamInfo)))
	{
		MessageBoxA(NULL, "Unable to Create Video Stream in the Movie File", "Error", MB_OK);
		return E_FAIL;
	}

	ZeroMemory(m_AviCompressOptions,sizeof(AVICOMPRESSOPTIONS));
	m_AviCompressOptions->fccType=streamtypeVIDEO;
	m_AviCompressOptions->fccHandler=m_AviStreamInfo->fccHandler;
	m_AviCompressOptions->dwFlags=AVICOMPRESSF_KEYFRAMES|AVICOMPRESSF_VALID;//|AVICOMPRESSF_DATARATE;
	m_AviCompressOptions->dwKeyFrameEvery=1;

	if(FAILED(AVIMakeCompressedStream(m_pAviCompressedStream,*m_pAviStream,m_AviCompressOptions,NULL)))
	{
		// One reason this error might occur is if you are using a Codec that is not 
		// available on your system. Check the mmioFOURCC() code you are using and make
		// sure you have that codec installed properly on your machine.
		MessageBoxA(NULL, "Unable to Create Compressed Stream: Check your CODEC options", "Error", MB_OK);
		return E_FAIL;
	}

	BITMAPINFO bmpInfo;
	ZeroMemory(&bmpInfo,sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biPlanes		= 1;
	bmpInfo.bmiHeader.biWidth		= nFrameWidth;
	bmpInfo.bmiHeader.biHeight		= nFrameHeight;
	bmpInfo.bmiHeader.biCompression	= BI_RGB;
	bmpInfo.bmiHeader.biBitCount	= nBitsPerPixel;
	bmpInfo.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biSizeImage	= bmpInfo.bmiHeader.biWidth*bmpInfo.bmiHeader.biHeight*bmpInfo.bmiHeader.biBitCount/8;

	if(FAILED(AVIStreamSetFormat(*m_pAviCompressedStream,0,(LPVOID)&bmpInfo, bmpInfo.bmiHeader.biSize)))
	{
		MessageBoxA(NULL, "Unable to Set Video Stream Format", "Error", MB_OK);
		return E_FAIL;
	}

	return hr;
}

void adaugaOptiuni() {
	ShowWindow(handleforwindow2,SW_SHOW);
}


void aplicaOptiuni() {
	int indexComboBoxAID = SendMessage(hwndComboBoxAID, (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);

	int indexComboBoxChannel = SendMessage(hwndComboBoxChannel, (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);

	int indexComboBoxDF = SendMessage(hwndComboBoxDF, (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);

	int lungime;
	lungime = SendMessage(hwndEditBl, WM_GETTEXTLENGTH, 0, 0);
	LPSTR lpstrEditBl = (LPSTR) malloc(lungime + 1);
	GetWindowText(hwndEditBl, lpstrEditBl, lungime + 1);

	bool checkBoxShowFPS;
	if (SendDlgItemMessage(handleforwindow2, IDC_SHOWFPS, BM_GETCHECK, 0, 0) == BST_CHECKED) checkBoxShowFPS = TRUE;
	else checkBoxShowFPS = FALSE;

	lungime = SendMessage(hwndEditTargetFR, WM_GETTEXTLENGTH, 0, 0);
	LPSTR lpstrEditTargetFR = (LPSTR) malloc(lungime + 1);
	GetWindowText(hwndEditTargetFR, lpstrEditTargetFR, lungime + 1);

	int indexComboBoxVC = SendMessage(hwndComboBoxVC, (UINT) CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0);

	lungime = SendMessage(hwndEditVRS, WM_GETTEXTLENGTH, 0, 0);
	LPSTR lpstrEditVRS;
	GetWindowText(hwndEditVRS, lpstrEditVRS, lungime + 1);

	lungime = SendMessage(hwndEditVRP, WM_GETTEXTLENGTH, 0, 0);
	LPSTR lpstrEditVRP;
	GetWindowText(hwndEditVRP, lpstrEditVRP, lungime + 1);

	lungime = SendMessage(hwndEditScreen, WM_GETTEXTLENGTH, 0, 0);
	LPSTR lpstrEditScreen;
	GetWindowText(hwndEditScreen, lpstrEditScreen, lungime + 1);

	SetComboBoxAID(indexComboBoxAID);
	SetComboBoxChannel(indexComboBoxChannel); 
	SetComboBoxDF(indexComboBoxDF);
	SetEditBl(atoi(lpstrEditBl));
	SetCheckBoxShowFPS(checkBoxShowFPS);
	SetTargetFR(atoi(lpstrEditTargetFR));
	SetIndexComboBoxVC(indexComboBoxVC);
	SetLpstrEditVRS(lpstrEditVRS);
	SetLpstrEditVRP(lpstrEditVRP);
	SetLpstrEditScreen(lpstrEditScreen);

	ShowWindow(handleforwindow2,SW_HIDE);

}