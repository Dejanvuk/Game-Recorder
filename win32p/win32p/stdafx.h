// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include <algorithm>

#pragma warning(disable:4996)

// TODO: reference additional headers your program requires here
#include "MinHook.h"
#pragma comment(lib,"libMinHook.x86.lib")

#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

//#include <d3d12.h>
//#include <d3dx12.h> //open source d3d12 helper functions and structures provided by Microsoft
//#pragma comment(lib, "d3d12.lib")

#include <d3d11.h>
#include <d3dx11.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dx11.lib")

#include <d3d10.h>
#include <d3dx10.h>
#pragma comment (lib, "d3d10.lib")
#pragma comment (lib, "d3dx10.lib")

#include "FW1FontWrapper/FW1FontWrapper.h"
#pragma comment (lib, "FW1FontWrapper.lib")

#include <gl\GL.h>
#include <gl\GLU.h>

#pragma comment(lib,"opengl32.lib")

#include <glut.h>
#pragma comment(lib, "glut32.lib")

#include "vulkan/vulkan.h"
#pragma comment(lib, "Lib32/vulkan-1.lib") 

#include <stdio.h>
//#include <wchar.h>
#include <tchar.h>
#include <ctime>

#include <vfw.h>
#include <Winnt.h>
#pragma comment(lib, "vfw32.lib")

#include <string.h>

