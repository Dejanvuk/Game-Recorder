// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <vfw.h>  

// TODO: reference additional headers your program requires here
#include <mmsystem.h>
#include <mmreg.h>
#include <Dsound.h>
#include <shellapi.h>
#include <commdlg.h>
#include <string.h>
#include <commctrl.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "vfw32.lib")
#pragma comment(lib, "avifile.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "vfw32.lib")

// WASAPI
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <comdef.h>
#include <AudioPolicy.h>
#include <assert.h>
#include <avrt.h>
#pragma comment(lib, "avrt.lib")

