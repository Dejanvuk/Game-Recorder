// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#define MYDLL_EXPORTS 
#include "dllmain.h"
#include "DllFunctions.h"

//#include "GLToMovie.h"

// Shared memory between processes 
#pragma data_seg (".myseg")
   HWND serverHwnd = NULL;
   int indexComboBoxAID = 0;
   int indexComboBoxChannel = 0;
   int indexComboBoxDF = 0;
   int editBl = 0;
   bool CheckBoxShowFPS = TRUE;
   int targetFR = 30;
   int indexComboBoxVC = 1;
   LPSTR lpstrEditVRS = "R";
   LPSTR lpstrEditVRP = "T";
   LPSTR lpstrEditScreen = "O";
#pragma data_seg()

#pragma comment(linker, "/section:.myseg,rws")

// Pointer to functions that hold the original function
LoadLibrary_t orig_LoadLibrary; 
LoadLibraryW_t orig_LoadLibraryW; 
LoadLibraryEx_t orig_LoadLibraryEx; 
// DX 9
Direct3DCreate9_t orig_Direct3DCreate9; 
CreateDevice_t orig_CreateDevice;
EndScene_t orig_EndScene;
Present_t orig_Present;
Reset_t orig_Reset;
GETBACKBUFFER orig_getbackbuffer;
LOCKRECT orig_lockrect;
// OPENGL
wglSwapBuffers_t orig_wglSwapBuffers;
// DX10
D3D10PresentHook phookD3D10Present = NULL;
// DX11
D3D11PresentHook phookD3D11Present = NULL;
// DX 10,11,12
D3DPresentHook phookD3DPresent = NULL;
BOOL isDirectx10 = false; BOOL isDirectx11 = false; BOOL isDirectx12 = false;
BOOL primaDataDX = true;
// VULKAN
vkCmdDrawIndexedHook orig_vkCmdDrawIndexed = NULL;
vkCreateDeviceHook orig_vkCreateDevice = NULL;
vkCreateSwapchainKHRHook orig_vkCreateSwapchainKHR = NULL;
vkAcquireNextImageKHRHook orig_vkAcquireNextImageKHR = NULL;
vkQueuePresentKHRHook orig_vkQueuePresentKHR = NULL;

//other variable definitions
HINSTANCE g_hInst;
HMODULE hAPI;
HMODULE hAPIDXGI; BOOL dxgiHooked = FALSE;
HMODULE hAPID9;   BOOL d9Hooked   =   FALSE;
HMODULE hAPIGL;   BOOL glHooked   =   FALSE;
HMODULE hAPIVK;   BOOL vkHooked   =   FALSE;
DWORD* pVtable;
#define createDeviceI 16
PBYTE pDirect3DCreate9;
HMODULE hD3d9;
IDirect3D9* direct;
IDirect3DDevice9* d3ddev;
FILE *file;

// DX11 variables
ID3D11Device *pDevice = NULL;
ID3D11DeviceContext *pContext = NULL;
IFW1Factory *pFW1Factory = NULL;
IFW1FontWrapper *pFontWrapper = NULL;
//rendertarget
ID3D11Texture2D* RenderTargetTexture;
ID3D11RenderTargetView* RenderTargetView = NULL;
BOOL primaDataDx11 = true; // folosit pentru a lua contextul dx11 prima oara doar
UINT nrViewport = 0;
D3D11_VIEWPORT pViewports;
ID3D11Texture2D* pSurface;
DXGI_SWAP_CHAIN_DESC pDesc;
ID3D11Texture2D* pNewTexture;
D3D11_TEXTURE2D_DESC description;

// DX10 variables
ID3D10Device *pDeviceD10 = NULL;
BOOL primaDataDx10 = TRUE;
ID3D10Device *pDeviceDX10 = NULL;
ID3D10RenderTargetView* g_renderTargetView[1];
ID3D10DepthStencilView* depthViews[1];
//rendertarget
ID3D10Texture2D* RenderTargetTextureDX10;
ID3D10RenderTargetView* RenderTargetViewDX10 = NULL;
D3DX10_FONT_DESC FontDesc;
ID3DX10Font* Font = NULL;
ID3D10Texture2D* pSurfaceDX10;
ID3D10Texture2D* pNewTextureDX10;
D3D10_TEXTURE2D_DESC descriptionDX10;


//DX variables
UINT deviceVersion = FALSE; // 0 DX10 1 DX11 2 DX12

// VULKAN variables
VkSwapchainKHR* pSwapchainVk = NULL;
BOOL primaDataVulkan = TRUE;

// FPS
INT Fps = 0;
FLOAT LastTickCount = 0.0f;
FLOAT CurrentTickCount;
wchar_t FrameRate[50];
LPD3DXFONT pFont = 0; 

// Video Record
#define BITSPERPIXEL		32
int captureVideo = 0; // 0 - not capturing 1 - capturing 2 - stop capturing
LPVOID	pBits=NULL;

//CGLToMovie g_MovieRecorder;
double ViewPortParams[4];
RECT actualDesktop;
//CGLToMovie g_MovieRecorder(L"Output.Avi",1024,768,24, mmioFOURCC('X','2','6','4'),30);
BOOL firstTime = 1;
LONG w,h, total;
HWND hwnd;
RECT rect;

/* CAVI File vars*/
HDC					m_hAviDC = NULL;
HANDLE				m_hHeap = NULL;
LPVOID				m_lpBits = NULL;					// Useful for holding the bitmap content, if any
LONG				m_lSample = NULL;					// Keeps track of the current Frame Index
PAVIFILE			m_pAviFile = NULL;
PAVISTREAM			m_pAviStream = NULL;
PAVISTREAM			m_pAviCompressedStream = NULL;
AVISTREAMINFO		m_AviStreamInfo;
AVICOMPRESSOPTIONS	m_AviCompressOptions;
DWORD				m_dwFrameRate = GetTargetFR();				// Frames Per Second Rate (FPS) default- 30
DWORD				m_dwFCCHandler = mmioFOURCC('X','2','6','4');				// DEFAULT
TCHAR				m_szFileName[260] = _T("Output.Avi");		// Holds the Output Movie File Name

UINT_PTR nTimerId;	
HINSTANCE           gl_hThisInstance;

int					m_nAppendFuncSelector = 1;		//0=Dummy	1=FirstTime	2=Usual

LPDIRECT3DSURFACE9	m_pRenderTargetDepthBuffer;
LPDIRECT3DSURFACE9	m_pRenderTarget;			//The Surface Onto which the scene would be Blit
LPDIRECT3DTEXTURE9	m_pRenderTargetTexture;		//The Render Target is a Surface of this Texture
LPDIRECT3DSURFACE9	m_pOffscreenPlainSurface;	//This is useful to get the render target data 

LPDIRECT3DSURFACE9	m_pOldBackBuffer;			//Original back Buffer
LPDIRECT3DSURFACE9	m_pOldDepthBuffer;			//Original Depth Buffer

// Takes care of releasing the memory and movie related handles
void ReleaseMemory() {
	m_nAppendFuncSelector=0;		//Point to DummyFunction
	if(m_hAviDC)
	{
		DeleteDC(m_hAviDC);
		m_hAviDC=NULL;
	}
	if(m_pAviCompressedStream)
	{
		AVIStreamRelease(m_pAviCompressedStream);
		m_pAviCompressedStream=NULL;
	}
	if(m_pAviStream)
	{
		AVIStreamRelease(m_pAviStream);
		m_pAviStream=NULL;
	}
	if(m_pAviFile)
	{
		AVIFileRelease(m_pAviFile);
		m_pAviFile=NULL;
	}
	if(m_lpBits)
	{
		HeapFree(m_hHeap,HEAP_NO_SERIALIZE,m_lpBits);
		m_lpBits=NULL;
	}
	if(m_hHeap)
	{
		HeapDestroy(m_hHeap);
		m_hHeap=NULL;
	}
}
// Takes care of creating the memory, streams, compression options etc. required for the movie
HRESULT InitMovieCreation(int nFrameWidth, int nFrameHeight, int nBitsPerPixel) {
	int	nMaxWidth=GetSystemMetrics(SM_CXSCREEN),nMaxHeight=GetSystemMetrics(SM_CYSCREEN);
	m_hAviDC = CreateCompatibleDC(NULL);
	if(m_hAviDC==NULL)	
	{
		MessageBoxA(NULL, "Unable to Create Compatible DC", "Error", MB_OK);
		return E_FAIL;
	}
	
	if(nFrameWidth > nMaxWidth)	nMaxWidth= nFrameWidth;
	if(nFrameHeight > nMaxHeight)	nMaxHeight = nFrameHeight;

	
	m_hHeap=HeapCreate(HEAP_NO_SERIALIZE, nMaxWidth*nMaxHeight*4, 0);
	if(m_hHeap==NULL)
	{
		MessageBoxA(NULL, "Unable to Create Heap", "Error", MB_OK);
		return E_FAIL;
	}
	
	m_lpBits=HeapAlloc(m_hHeap, HEAP_ZERO_MEMORY|HEAP_NO_SERIALIZE, nMaxWidth*nMaxHeight*4);
	if(m_lpBits==NULL)	
	{	
		MessageBoxA(NULL, "Unable to Allocate Memory on Heap", "Error", MB_OK);
		return E_FAIL;
	}
	
	if(FAILED(AVIFileOpen(&m_pAviFile, m_szFileName, OF_CREATE|OF_WRITE, NULL)))
	{
		MessageBoxA(NULL, "Unable to Create the Movie File", "Error", MB_OK);
		return E_FAIL;
	}

	if(GetIndexComboBoxVC() == 0) m_dwFCCHandler = NULL;
	else if(GetIndexComboBoxVC() == 1) m_dwFCCHandler = mmioFOURCC('X','2','6','4');
	else if(GetIndexComboBoxVC() == 2) m_dwFCCHandler = NULL;
	else if(GetIndexComboBoxVC() == 3) m_dwFCCHandler = NULL;
	else {
		m_dwFCCHandler = NULL; // nici un codec gasit
		MessageBoxA(NULL, "Unable to find any video codec.UNCOMPRESSED ASSUMED", "Error", MB_OK);
	}

	ZeroMemory(&m_AviStreamInfo,sizeof(AVISTREAMINFO));
	m_AviStreamInfo.fccType		= streamtypeVIDEO;
	m_AviStreamInfo.fccHandler	= m_dwFCCHandler;
	m_AviStreamInfo.dwScale		= 1;
	m_AviStreamInfo.dwRate		= GetTargetFR();	// Frames Per Second;
	m_AviStreamInfo.dwQuality	= -1;				// Default Quality
	m_AviStreamInfo.dwSuggestedBufferSize = nMaxWidth*nMaxHeight*4;
    SetRect(&m_AviStreamInfo.rcFrame, 0, 0, nFrameWidth, nFrameHeight);
	_tcscpy(m_AviStreamInfo.szName, _T("Video Stream"));

	if(FAILED(AVIFileCreateStream(m_pAviFile,&m_pAviStream,&m_AviStreamInfo)))
	{
		MessageBoxA(NULL, "Unable to Create Video Stream in the Movie File", "Error", MB_OK);
		return E_FAIL;
	}

	ZeroMemory(&m_AviCompressOptions,sizeof(AVICOMPRESSOPTIONS));
	m_AviCompressOptions.fccType=streamtypeVIDEO;
	m_AviCompressOptions.fccHandler=m_AviStreamInfo.fccHandler;
	m_AviCompressOptions.dwFlags=AVICOMPRESSF_KEYFRAMES|AVICOMPRESSF_VALID;//|AVICOMPRESSF_DATARATE;
	m_AviCompressOptions.dwKeyFrameEvery=1;
	//m_AviCompressOptions.dwBytesPerSecond=1000/8;
	//m_AviCompressOptions.dwQuality=100;

	if(FAILED(AVIMakeCompressedStream(&m_pAviCompressedStream,m_pAviStream,&m_AviCompressOptions,NULL)))
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

	if(FAILED(AVIStreamSetFormat(m_pAviCompressedStream,0,(LPVOID)&bmpInfo, bmpInfo.bmiHeader.biSize)))
	{
		// One reason this error might occur is if your bitmap does not meet the Codec requirements.
		// For example, 
		//   your bitmap is 32bpp while the Codec supports only 16 or 24 bpp; Or
		//   your bitmap is 274 * 258 size, while the Codec supports only sizes that are powers of 2; etc...
		// Possible solution to avoid this is: make your bitmap suit the codec requirements,
		// or Choose a codec that is suitable for your bitmap.
		MessageBoxA(NULL, "Unable to Set Video Stream Format", "Error", MB_OK);
		return E_FAIL;
	}

	return S_OK;	// Everything went Fine
}
/* CAVI File Functions*/
HRESULT	AppendFrameUsual(int nWidth, int nHeight, LPVOID pBits,int nBitsPerPixel) {
	DWORD	dwSize=nWidth*nHeight*nBitsPerPixel/8;

	if(FAILED(AVIStreamWrite(m_pAviCompressedStream,m_lSample++,1,pBits,dwSize,0,NULL,NULL)))
	{

		ReleaseMemory();

		return E_FAIL;
	}
	return S_OK;
}
HRESULT	AppendFrameFirstTime(int nWidth, int nHeight, LPVOID pBits,int nBitsPerPixel) {
	if(SUCCEEDED(InitMovieCreation(nWidth, nHeight, nBitsPerPixel)))
	{
		m_nAppendFuncSelector=2;		//Point to the UsualAppend Function

		return AppendFrameUsual(nWidth, nHeight, pBits, nBitsPerPixel);
	}

	ReleaseMemory();

	return E_FAIL;
}
HRESULT	AppendDummy(int nWidth, int nHeight, LPVOID pBits,int nBitsPerPixel) {
	return E_FAIL;
}
HRESULT	(*pAppendFrameBits[3])(int, int, LPVOID, int) = {&AppendDummy,&AppendFrameFirstTime,&AppendFrameUsual};
// Actual Function which will decide which CAVI file function to call
HRESULT	AppendNewFrame(int nWidth, int nHeight, LPVOID pBits,int nBitsPerPixel) {
	return (pAppendFrameBits[m_nAppendFuncSelector])(nWidth,nHeight,pBits,nBitsPerPixel);
}

LRESULT CALLBACK DXGIMsgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){ return DefWindowProc(hwnd, uMsg, wParam, lParam); }


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		g_hInst = hModule;
		file = fopen("win32p.txt","w");
		if (MH_Initialize() != MH_OK)
		{
			MessageBoxA(NULL, "Failed to initialise MinHook!", "Error", MB_OK);
		}
		/* Hook the LoadLibrayA, Ex, W
		if (MH_CreateHook(LoadLibraryA, LoadLibrary_Hook, reinterpret_cast<LPVOID*>(&orig_LoadLibrary)) != MH_OK)
		{
			MessageBoxA(NULL, "Failed hooking the load library!", "Error", MB_OK); 
		}
		if (MH_CreateHook(LoadLibraryW, LoadLibraryW_Hook, reinterpret_cast<LPVOID*>(&orig_LoadLibraryW)) != MH_OK)
		{
			MessageBoxA(NULL, "Failed hooking the load library!", "Error", MB_OK); 
		}
		if (MH_CreateHook(LoadLibraryEx, LoadLibraryEx_Hook, reinterpret_cast<LPVOID*>(&orig_LoadLibraryEx)) != MH_OK)
		{
			MessageBoxA(NULL, "Failed hooking the load library!", "Error", MB_OK); 
		}
		MH_EnableHook(LoadLibraryA);
		MH_EnableHook(LoadLibraryW);
		MH_EnableHook(LoadLibraryEx);
		*/
		//hD3d9 = GetModuleHandle(L"d3d9.dll");
		//if(!hD3d9)
		//hD3d9 = LoadLibrary(L"d3d9.dll");
		CreateThread(NULL, 0, InitializeHook, NULL, 0, NULL);
		/*
		while((hD3d9 = GetModuleHandle(L"d3d9.dll"))==NULL) {
                Sleep(250);
            }
		*/
	    //pDirect3DCreate9 = (PBYTE)GetProcAddress(hD3d9 , "Direct3DCreate9");
        //HookAPI();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

BOOL hookDX9(void) {
	HWND hWnd;
	D3DDISPLAYMODE d3ddm;
	D3DPRESENT_PARAMETERS d3dpp; 
	HRESULT hRes;
	//MessageBoxA(NULL, "Initializing the hook!", "Error", MB_OK);
	hAPID9 = LoadLibrary(L"d3d9.dll");
	orig_Direct3DCreate9 = (Direct3DCreate9_t)GetProcAddress(hAPID9, "Direct3DCreate9");
	direct = orig_Direct3DCreate9(D3D_SDK_VERSION); 
	if(direct == NULL) {
		MessageBoxA(NULL, "Failed to create the IDirect3D9 object!", "Error", MB_OK);
		return NULL;
	}
	hRes = direct->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm );
	ZeroMemory( &d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = true;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = d3ddm.Format;
	d3dpp.Flags =  D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		
	WNDCLASSEX wc = { sizeof(WNDCLASSEX),CS_CLASSDC,DXGIMsgProc,0L,0L,GetModuleHandle(NULL),NULL,NULL,NULL,NULL,(LPCWSTR)("1"),NULL};
	RegisterClassEx(&wc);
	hWnd = CreateWindow((LPCWSTR)("1"),NULL,WS_OVERLAPPEDWINDOW,100,100,300,300,GetDesktopWindow(),NULL,wc.hInstance,NULL);
	hRes = direct->CreateDevice( D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,&d3dpp, &d3ddev);

	direct->Release();
	DestroyWindow(hWnd);

	pVtable = (DWORD*)*((DWORD*)d3ddev);

	// Detour EndScene
	if(MH_CreateHook((void*)pVtable[42],(void*)&hook_EndScene,reinterpret_cast<LPVOID*>(&orig_EndScene)) != MH_OK) {
		//MessageBoxA(NULL, "Failed Detouring EndSceneDevice!", "Error", MB_OK);
	}
	MH_EnableHook((void*)pVtable[42]);
	// Detour Present
	if(MH_CreateHook((void*)pVtable[17],(void*)&hook_Present,reinterpret_cast<LPVOID*>(&orig_Present)) != MH_OK) {
		//MessageBoxA(NULL, "Failed Detouring Present!", "Error", MB_OK);
	}
	MH_EnableHook((void*)pVtable[17]);
	// Detour Reset
	if(MH_CreateHook((void*)pVtable[16],(void*)&hook_Reset,reinterpret_cast<LPVOID*>(&orig_Reset)) != MH_OK) {
		//MessageBoxA(NULL, "Failed Detouring Reset!", "Error", MB_OK);
	}
	MH_EnableHook((void*)pVtable[16]);
	return true;
}

BOOL hookDX10(void) {
	WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DXGIMsgProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "DX", NULL };
	RegisterClassExA(&wc);
	HWND hWnd = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

	DXGI_RATIONAL rational = {};
	rational.Numerator = 60;
	rational.Denominator = 1;

	DXGI_MODE_DESC modeDesc = {};
	modeDesc.Height = 1;
	modeDesc.Width = 1;
	modeDesc.RefreshRate = rational;
	modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;
	sampleDesc.Quality = 0;

	DXGI_SWAP_CHAIN_DESC scDesc = {};
	scDesc.BufferCount = 1;
	scDesc.Flags = 0;
	scDesc.Windowed = ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;
	scDesc.OutputWindow = hWnd;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.SampleDesc = sampleDesc;
	scDesc.BufferDesc = modeDesc;

	IDXGISwapChain* pSwapChain;

	if (FAILED(D3D10CreateDeviceAndSwapChain(
		nullptr,
		D3D10_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		D3D10_SDK_VERSION,
		&scDesc,
		&pSwapChain,
		&pDeviceD10)))
	{
		MessageBox(NULL, L"Failed to create directX device and swapchain!", L"Error", MB_ICONERROR);
		return NULL;
	}
	
	pVtable = (DWORD*)*((DWORD*)pSwapChain);

	// Hook IDXGISwapChain::Present method
	if(MH_CreateHook((void*)pVtable[8],(void*)&hookD3DPresent,reinterpret_cast<LPVOID*>(&phookD3DPresent)) != MH_OK) {
		//MessageBoxA(NULL, "Failed Detouring EndSceneDevice!", "Error", MB_OK);
	}
	MH_EnableHook((void*)pVtable[8]);

	return TRUE;
}

BOOL hookDX11(void) {
	WNDCLASSEXA wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DXGIMsgProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, "DX", NULL };
	RegisterClassExA(&wc);
	HWND hWnd = CreateWindowA("DX", NULL, WS_OVERLAPPEDWINDOW, 100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

	D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
	D3D_FEATURE_LEVEL obtainedLevel;
	ID3D11Device* d3dDevice = nullptr;
	ID3D11DeviceContext* d3dContext = nullptr;

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(scd));
	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 1; // Set to 1 to disable multisampling
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Windowed = ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_POPUP) != 0) ? false : true;

	// LibOVR 0.4.3 requires that the width and height for the backbuffer is set even if
	// you use windowed mode, despite being optional according to the D3D11 documentation.
	scd.BufferDesc.Width = 1;
	scd.BufferDesc.Height = 1;
	scd.BufferDesc.RefreshRate.Numerator = 0;
	scd.BufferDesc.RefreshRate.Denominator = 1;

	UINT createFlags = 0;
	IDXGISwapChain* pSwapChain;

	if (FAILED(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createFlags,
		requestedLevels,
		sizeof(requestedLevels) / sizeof(D3D_FEATURE_LEVEL),
		D3D11_SDK_VERSION,
		&scd,
		&pSwapChain,
		&pDevice,
		&obtainedLevel,
		&pContext)))
	{
		MessageBox(NULL, L"Failed to create directX device and swapchain!", L"Error", MB_ICONERROR);
		return NULL;
	}
	//else MessageBoxA(NULL, "Failed to create the IDirect3D9 object!", "Error", MB_OK);
	pVtable = (DWORD*)*((DWORD*)pSwapChain);

	// Hook IDXGISwapChain::Present method
	if(MH_CreateHook((void*)pVtable[8],(void*)&hookD3DPresent,reinterpret_cast<LPVOID*>(&phookD3DPresent)) != MH_OK) {
		//MessageBoxA(NULL, "Failed Detouring EndSceneDevice!", "Error", MB_OK);
	}
	MH_EnableHook((void*)pVtable[8]);

}

BOOL hookDX12(void) {

	return TRUE;
}

BOOL hookOpenGL() {
	//use GetProcAddress to find address of wglSwapBuffers in opengl32.dll
	orig_wglSwapBuffers = (wglSwapBuffers_t)GetProcAddress(hAPIGL, "wglSwapBuffers");
	if (MH_CreateHook(GetProcAddress(hAPIGL, "wglSwapBuffers"), (void*)&hook_wglSwapBuffers, reinterpret_cast<LPVOID*>(&orig_wglSwapBuffers)) != MH_OK)
	{
		//MessageBoxA(NULL, "Failed detouring wglSwapBuffers!", "Error", MB_OK); 
	}
	MH_EnableHook(GetProcAddress(hAPIGL, "wglSwapBuffers"));
	return true;
}

BOOL hookVulkan() {
	/*
	void* aptr = (vkCmdDrawIndexedHook)GetProcAddress(hAPIVK, "vkCmdDrawIndexed");
	if (MH_CreateHook(aptr, (void*)&hook_vkCmdDrawIndexed, reinterpret_cast<LPVOID*>(&orig_vkCmdDrawIndexed)) != MH_OK)
	{
		//MessageBoxA(NULL, "Failed detouring vkCmdDrawIndexed!", "Error", MB_OK); 
	}
	MH_EnableHook(aptr);
    void* aptr2 = (vkCreateSwapchainKHRHook)GetProcAddress(hAPIVK, "vkCreateSwapchainKHR");
	if (MH_CreateHook(aptr2, (void*)&hook_vkCreateSwapchainKHR, reinterpret_cast<LPVOID*>(&orig_vkCreateSwapchainKHR)) != MH_OK)
	{
		//MessageBoxA(NULL, "Failed detouring vkCmdDrawIndexed!", "Error", MB_OK); 
	}
	MH_EnableHook(aptr2);
	void* aptr3 = (vkAcquireNextImageKHRHook)GetProcAddress(hAPIVK, "vkAcquireNextImageKHR"); 
	if (MH_CreateHook(aptr3, (void*)&hook_vkAcquireNextImageKHR, reinterpret_cast<LPVOID*>(&orig_vkAcquireNextImageKHR)) != MH_OK)
	{
		//MessageBoxA(NULL, "Failed detouring vkCmdDrawIndexed!", "Error", MB_OK); 
	}
	MH_EnableHook(aptr3);
	
	void* aptr4 = (vkQueuePresentKHRHook)GetProcAddress(hAPIVK, "vkQueuePresentKHR"); 
	if (MH_CreateHook(aptr4, (void*)&hook_vkQueuePresentKHR, reinterpret_cast<LPVOID*>(&orig_vkQueuePresentKHR)) != MH_OK)
	{
		//MessageBoxA(NULL, "Failed detouring vkCmdDrawIndexed!", "Error", MB_OK); 
	}
	MH_EnableHook(aptr4);
	*/
	void* aptr4 = (vkCreateDeviceHook)GetProcAddress(hAPIVK, "vkCreateDevice"); 
	if (MH_CreateHook(aptr4, (void*)&hook_vkCreateDevice, reinterpret_cast<LPVOID*>(&orig_vkCreateDevice)) != MH_OK)
	{
		//MessageBoxA(NULL, "Failed detouring vkCmdDrawIndexed!", "Error", MB_OK); 
	}
	MH_EnableHook(aptr4);

	// Creeaza manual swap chainu 

	return true;
}

DWORD __stdcall InitializeHook(LPVOID) {
	AVIFileInit();

	if(!glHooked) {
		hAPIGL = GetModuleHandle(L"opengl32.dll");
		if (!hAPIGL) {
			hAPIGL = LoadLibraryA("opengl32.dll");
		}
			hookOpenGL();
			glHooked = true; 
	}
	if(!dxgiHooked) {
		hAPIDXGI = GetModuleHandle(L"dxgi.dll");
		if (!hAPIDXGI) {
			hAPIDXGI = LoadLibraryA("dxgi.dll");
		}
			hookDX10();
			//hookDX11();
			//hookDX12();
			dxgiHooked = true;
	}
	if(!d9Hooked) {
		hAPID9 = GetModuleHandle(L"d3d9.dll");
		if (!hAPID9) {
			hAPID9  = LoadLibraryA("d3d9.dll");
		}
			hookDX9();
			d9Hooked = true;
	}
	if(!vkHooked) {
		hAPIVK = GetModuleHandle(L"vulkan-1.dll");
		if (!hAPIVK) {
			hAPIVK  = LoadLibraryA("vulkan-1.dll");
		}
			hookVulkan();
			vkHooked = true;
	}
	
	
	/*
	while(true) {
		if (!((hAPIGL = GetModuleHandle(L"opengl32.dll")) == NULL)) {
			hookOpenGL();
		}
		else if (!((hAPID9 = GetModuleHandle(L"d3d9.dll")) == NULL)) {
			hookDX9();
		}
		else if (!((hAPIDXGI = GetModuleHandle(L"dxgi.dll")) == NULL)) {
			hookDX11();
		}
		else Sleep(100);
	}
	*/
	return NULL;
}

HMODULE WINAPI LoadLibrary_Hook ( LPCSTR lpFileName ) // Our hooked LoadLibrary
{
	static int hooked = 0;

	if ( strcmp( lpFileName, "d3d9.dll" ) == 0) 
    {
        //pDirect3DCreate9 = (PBYTE)GetProcAddress(hM, "Direct3DCreate9");
		//MessageBoxA(NULL, "Got Direct3DCreate9 address", "Error", MB_OK);
        //HookAPI();
		//hookDX9();
    }
	else if (strcmp( lpFileName, "opengl32.dll" ) == 0) {
		int hr = 'c';
		for(int i = 0; i < 4; i++) {
				fputc(hr, file);
			}
		//fwrite((int*)&hr,sizeof(int),1,file);
		fclose (file);
		MessageBoxA(NULL, "Opengl loaded!", "Error", MB_OK);
	}
	else if (GetModuleHandle(L"opengl32.dll")) {
		
	}
	HMODULE hM = orig_LoadLibrary( lpFileName );
    return hM;
}

HMODULE WINAPI LoadLibraryW_Hook ( LPCSTR lpFileName ) // Our hooked LoadLibrary
{

	if ( strcmp( lpFileName, "d3d9.dll" ) == 0) 
    {
        //pDirect3DCreate9 = (PBYTE)GetProcAddress(hM, "Direct3DCreate9");
		//MessageBoxA(NULL, "Got Direct3DCreate9 address", "Error", MB_OK);
        //HookAPI();
		//hookDX9();
    }
	else if (strcmp( lpFileName, "opengl32.dll" ) == 0) {
		int hr = 'c';
		for(int i = 0; i < 4; i++) {
				fputc(hr, file);
			}
		//fwrite((int*)&hr,sizeof(int),1,file);
		fclose (file);
		MessageBoxA(NULL, "Opengl loaded!", "Error", MB_OK);
	}
	else if (GetModuleHandle(L"opengl32.dll")) {
		
	}
	HMODULE hM = orig_LoadLibraryW( lpFileName );
    return hM;
}

HMODULE WINAPI LoadLibraryEx_Hook ( LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags) // Our hooked LoadLibrary
{
	static int hooked = 0;

	if ( strcmp( lpFileName, "d3d9.dll" ) == 0) 
    {
        //pDirect3DCreate9 = (PBYTE)GetProcAddress(hM, "Direct3DCreate9");
		//MessageBoxA(NULL, "Got Direct3DCreate9 address", "Error", MB_OK);
        //HookAPI();
		//hookDX9();
    }
	else if (strcmp( lpFileName, "opengl32.dll" ) == 0) {
		int hr = 'c';
		for(int i = 0; i < 4; i++) {
				fputc(hr, file);
			}
		//fwrite((int*)&hr,sizeof(int),1,file);
		fclose (file);
		MessageBoxA(NULL, "Opengl loaded!", "Error", MB_OK);
	}
	else if (GetModuleHandle(L"opengl32.dll")) {
		
	}
	HMODULE hM = orig_LoadLibraryEx( lpFileName, hFile, dwFlags );
    return hM;
}

void HookAPI()
{
    // Detour Direct3DCreate9
	if (MH_CreateHook(pDirect3DCreate9, (void*)&hook_Direct3DCreate9, reinterpret_cast<LPVOID*>(&orig_Direct3DCreate9)) != MH_OK)
	{
		MessageBoxA(NULL, "Failed detouring Direct3DCreate9!", "Error", MB_OK); 
	}
	MH_EnableHook(pDirect3DCreate9);
}

IDirect3D9* __stdcall hook_Direct3DCreate9(UINT sdkVers) {
	IDirect3D9* pD3d9 = orig_Direct3DCreate9(sdkVers); 
	direct = pD3d9;
    // Use a vtable hook on CreateDevice to get the device pointer later
    DWORD* pVtable = (DWORD*)(*(DWORD*)pD3d9);
	// Detour CreateDevice
	
	if(MH_CreateHook((void*)pVtable[16],(void*)&hook_CreateDevice,reinterpret_cast<LPVOID*>(&orig_CreateDevice)) != MH_OK) {
		//MessageBoxA(NULL, "Failed Detouring CreateDevice!", "Error", MB_OK);
	}
	MH_EnableHook((void*)pVtable[16]);
	
    //HookFunction(pVtable, (void*)&hook_CreateDevice, (void*)&orig_CreateDevice, 16);
	
    return pD3d9;
}

HRESULT APIENTRY hook_CreateDevice(IDirect3D9* pInterface, UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface) {
	HRESULT ret = orig_CreateDevice(pInterface, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
	d3ddev = *ppReturnedDeviceInterface; // get the pointer to the IDirect3DDevice9
	DWORD* pVtable = (DWORD*)*((DWORD*)d3ddev);
	//MessageBoxA(NULL, "Detouring EndScene!", "Error", MB_OK);
	// Detour EndScene
	if(MH_CreateHook((void*)pVtable[42],(void*)&hook_EndScene,reinterpret_cast<LPVOID*>(&orig_EndScene)) != MH_OK) {
		MessageBoxA(NULL, "Failed Detouring EndSceneDevice!", "Error", MB_OK);
	}
	MH_EnableHook((void*)pVtable[42]);
	if(MH_CreateHook((void*)pVtable[17],(void*)&hook_Present,reinterpret_cast<LPVOID*>(&orig_Present)) != MH_OK) {
		MessageBoxA(NULL, "Failed Detouring Present!", "Error", MB_OK);
	}
	MH_EnableHook((void*)pVtable[17]);
	
	//HookFunction(pVtable, (void*)&hook_EndScene, (void*)&orig_EndScene, 42);
	// Detour Present
	//HookFunction(pVtable, (void*)&hook_Present, (void*)&orig_Present, 17);
	return ret;
}

HRESULT APIENTRY hook_EndScene(IDirect3DDevice9* pInterface) {
	CurrentTickCount = clock() * 0.001f;
	Fps++;
	 if((CurrentTickCount - LastTickCount) > 1.0f)
	{
		LastTickCount = CurrentTickCount;
		swprintf_s(FrameRate, L"%d", Fps);
		//_itoa_s(Fps,FrameRate,10);
		Fps = 0;
	}
	D3DCOLOR fontColor = D3DCOLOR_ARGB(255,255,0,0);
	HRESULT result = D3DXCreateFont(pInterface , 22, 0, FW_NORMAL, 1, false,DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial", &pFont);
	if (FAILED(result))
    {
        MessageBoxA(NULL, "Failed creating font!", "Error", MB_OK);
    }
	else {
		RECT rct;
		rct.left=10;
		rct.right=0;
		rct.top=25;
		rct.bottom=100;

		CHAR text[50];
		WideCharToMultiByte(CP_ACP,0,FrameRate,-1,text,sizeof(text),NULL,NULL);

		int height = pFont->DrawText(NULL,LPWSTR(FrameRate),-1,&rct,DT_LEFT | DT_NOCLIP,fontColor);
			if (height == 0)
			{
				//fwrite((char*)&height,sizeof(int),1,file);
				//fclose (file);
				//MessageBoxA(NULL, "Failed drawing text!", "Error", MB_OK);;
			}
			(void)pFont->Release();
		}
	return orig_EndScene(pInterface);
}

HRESULT APIENTRY hook_Present(IDirect3DDevice9* pDevice, const RECT* pSourceRect,const RECT* pDestRect, HWND hDestWindowOverride,const RGNDATA* pDirtyRegion) {
	HRESULT hr;
	IDirect3DSurface9*back_buffer;
	IDirect3DSurface9* ir;

	RECT rect;
	GetClientRect(hDestWindowOverride, &rect); // or cparams.hFocusWindow
	D3DLOCKED_RECT	lockedRect;

	D3DDISPLAYMODE ddr;
	if(FAILED(hr = pDevice->GetDisplayMode(0, &ddr)))  {
		fwrite((char*)&hr,sizeof(int),1,file);
		fclose (file);
		MessageBoxA(NULL, "Failed get adapter!", "Error", MB_OK);
		return orig_Present(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	}

	if (GetAsyncKeyState('O') & 1) {
		pDevice->CreateOffscreenPlainSurface((ddr.Width),(ddr.Height),ddr.Format,D3DPOOL_SYSTEMMEM,&back_buffer, NULL);
		pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
		D3DXSaveSurfaceToFile(L"C:\\rahat.bmp", D3DXIFF_BMP, back_buffer, 0, NULL );
		
		IDirect3DSurface9_Release(back_buffer);
	}
	if (GetAsyncKeyState('R') & 1) {
		// Create the AVI file
		if(captureVideo == 0) {
			nTimerId=SetTimer(NULL,12345,33,enableVideoRecording);	//Timer set to 33 ms for 30 fps
			w = ddr.Width;
			h = ddr.Height;
			m_lpBits = (LPVOID)GlobalAlloc(GPTR, w * h * 4);	
			pDevice->SetRenderTarget(0, m_pRenderTarget);
			pDevice->SetDepthStencilSurface(m_pRenderTargetDepthBuffer);
		}
		else if(captureVideo == 1) {
			KillTimer((HWND)gl_hThisInstance,33);
			captureVideo = 0; // stop capturing
			ReleaseMemory();
		}
	}

	if(captureVideo == 1) {
		// Capture the current frame
		hr = pDevice->GetRenderTargetData(m_pRenderTarget, m_pOffscreenPlainSurface);

		hr = m_pOffscreenPlainSurface->LockRect(&lockedRect,NULL,D3DLOCK_READONLY|D3DLOCK_NO_DIRTY_UPDATE);

		BYTE *pSrc	=	(BYTE*) lockedRect.pBits;
		BYTE *pDest	=	(BYTE*)	m_lpBits;
		int nBytesPerPixel = 4;

		for(int i=0, nPitch=0, nRevPitch = (h-1) * lockedRect.Pitch; i< h; i++, nPitch += lockedRect.Pitch, nRevPitch -= lockedRect.Pitch)
			memcpy(pDest + nRevPitch, pSrc + nPitch, w * nBytesPerPixel);

		m_pOffscreenPlainSurface->UnlockRect();

		/*
		//pDevice->CreateOffscreenPlainSurface((ddr.Width),(ddr.Height),ddr.Format,D3DPOOL_SYSTEMMEM,&back_buffer, NULL);
		/*
		DWORD* vTable = (DWORD*)*((DWORD*)pDevice);
		orig_getbackbuffer = (GETBACKBUFFER)vTable[18];

		(orig_getbackbuffer)(pDevice,0,0,D3DBACKBUFFER_TYPE_MONO,&back_buffer);

		vTable = (DWORD*)*((DWORD*)back_buffer);
		orig_lockrect = (LOCKRECT)vTable[13];
		*/
		/*
		if(FAILED(hr = pDevice->GetBackBuffer(0, 0,D3DBACKBUFFER_TYPE_MONO,&back_buffer))) {
			MessageBoxA(NULL, "Failed getting back buffer!", "Error", MB_OK);	
			return orig_Present(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
		}

		//pDevice->GetRenderTargetData(ir, back_buffer);
		
		/*
		if(FAILED((orig_lockrect)(back_buffer,&lockedRect,&rect,D3DLOCK_READONLY))) {
			MessageBoxA(NULL, "Failed locking the back buffer!", "Error", MB_OK);
			return orig_Present(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
		}
		*/
		/*
		if(FAILED(hr = ir->LockRect(&lockedRect,&rect,D3DLOCK_READONLY))) {
			//fwrite((char*)&hr,sizeof(int),1,file);
			//fclose (file);
			//MessageBoxA(NULL, "Failed locking the back buffer!", "Error", MB_OK);
			//return orig_Present(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
		}
		
		for(int i=0;i<rect.bottom;i++)
			{
				memcpy((BYTE*)m_lpBits+(rect.bottom-i-1)*rect.right*BITSPERPIXEL/8,(BYTE*)lockedRect.pBits+i*lockedRect.Pitch,rect.right*BITSPERPIXEL/8);
			}
		ir->UnlockRect();
		*/
		AppendNewFrame(w, h, m_lpBits,30);
		captureVideo = 0;
		//pAviFile->AppendNewFrame(rect.right,rect.bottom,pBits);
	}
	else if (captureVideo == 2) {
		// Close the AVI file
	}
	hr = orig_Present(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	return hr;
}

HRESULT APIENTRY hook_Reset (IDirect3DDevice9* pDevice,D3DPRESENT_PARAMETERS* pPresentationParameters) {
	HRESULT hr;
	if(pFont) {
		pFont->Release();
	}

	hr =  orig_Reset(pDevice, pPresentationParameters);

	D3DPRESENT_PARAMETERS d3dpp; 
	D3DDISPLAYMODE d3ddm;
	direct->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm );
	ZeroMemory( &d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = true;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = d3ddm.Format;
	d3dpp.Flags =  D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	return hr;
}

BOOL APIENTRY hook_wglSwapBuffers(_In_ HDC hDc) {
	/* It's illegal to call SwapBuffers with a NULL dc. To keep Windows
     * error behavior we bail out, forwarding the call to the system
     * SwapBuffers if dc == NULL */
	if(!hDc) {
		return orig_wglSwapBuffers(hDc);
	}

	if (GetCheckBoxShowFPS()) {
		CurrentTickCount = clock() * 0.001f;
		Fps++;
		if((CurrentTickCount - LastTickCount) > 1.0f)
		{
			LastTickCount = CurrentTickCount;
			swprintf_s(FrameRate, L"%d", Fps);
			Fps = 0;
		}
		CHAR text[50];
		WideCharToMultiByte(CP_ACP,0,FrameRate,-1,text,sizeof(text),NULL,NULL);

		if (captureVideo == 1)glColor3f(255, 0, 0);
		else glColor3f(0, 255, 0);
		glRasterPos3f( 0 , 50, 0);
		for(int i = 0; text[i] != '\0'; i++)
		 glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
	}

	if (GetAsyncKeyState('R') & 1) {
		// Create the AVI file
		if(captureVideo == 0) {

			captureVideo = 1; // start capturing
			glGetDoublev(GL_VIEWPORT, ViewPortParams);
			hwnd = WindowFromDC(hDc);
			GetClientRect(hwnd, &rect);
			w = ViewPortParams[2];
			h = ViewPortParams[3];
			m_lpBits = (LPVOID)GlobalAlloc(GPTR, w * h * 4);
			// send message to client to start sound capture
			PostMessage(GetServerHwnd(), WM_APP + 6, (WPARAM)0, (LPARAM)0);
		}
		else if(captureVideo == 1) {
			captureVideo = 0; // stop capturing
			// send message to client to stop sound capture
			PostMessage(GetServerHwnd(), WM_APP + 7, (WPARAM)0, (LPARAM)0);
			ReleaseMemory();
		}
	}
	
	if(captureVideo == 1) {
		glFlush(); glFinish();
		glReadBuffer(GL_BACK);
		glReadPixels((GLint)0, (GLint)0,(GLint)w, (GLint)h,0x80E0, GL_UNSIGNED_BYTE, m_lpBits); //0x80E0
		AppendNewFrame(w, h, m_lpBits,24);
	}
	
	if (GetAsyncKeyState('O') & 1) { 
		int w = rect.right - rect.left;
		int h = rect.top - rect.bottom;
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		int nSize = w*h*3;
		char* dataBuffer = (char*)malloc(nSize*sizeof(char));

		if (!dataBuffer) return false;
		// Let's fetch them from the backbuffer	
		// We request the pixels in GL_BGR format, thanks to Berzeger for the tip
		glReadPixels((GLint)0, (GLint)0,(GLint)w, (GLint)h,GL_RGB, GL_UNSIGNED_BYTE, dataBuffer); //0x80E0

		
		FILE *filePtr = fopen("win32.tga", "wb");
		if (!filePtr) return false;

		unsigned char TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};
		unsigned char header[6] = { w%256,w/256,h%256,h/256,24,0};
		// We write the headers
		fwrite(TGAheader,	sizeof(unsigned char),	12,	filePtr);
		fwrite(header,	sizeof(unsigned char),	6,	filePtr);
		// And finally our image data
		fwrite(dataBuffer,	sizeof(GLubyte),	nSize,	filePtr);
		fclose(filePtr);
 
		free(dataBuffer);
	}
	
	return orig_wglSwapBuffers(hDc);
}

HRESULT __stdcall hookD3D10Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
	if (primaDataDx10) { 
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D10Device), (void **)&pDeviceDX10)))
		{
			pSwapChain->GetDevice(__uuidof(pDeviceDX10), (void**)&pDeviceDX10);
			pDeviceDX10->OMGetRenderTargets(1, g_renderTargetView, depthViews);
			pSwapChain->GetDesc(&pDesc);
			w = pDesc.BufferDesc.Width;
			h = pDesc.BufferDesc.Height;
		}
		if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (LPVOID*)&RenderTargetTextureDX10)))
		{
			pDeviceDX10->CreateRenderTargetView(RenderTargetTextureDX10, NULL, &RenderTargetViewDX10);
			RenderTargetTextureDX10->Release();
		}
		FontDesc.Height					= 30;
		FontDesc.Width					= 0;
		FontDesc.Weight				= 0;
		FontDesc.MipLevels			= 1;
		FontDesc.Italic					= false;
		FontDesc.CharSet				= OUT_DEFAULT_PRECIS;
		FontDesc.Quality				= DEFAULT_QUALITY;
		FontDesc.PitchAndFamily	= DEFAULT_PITCH | FF_DONTCARE;
 
		strcpy( (char*)FontDesc.FaceName,"Tahoma");
 
		D3DX10CreateFontIndirect( pDeviceDX10, &FontDesc, &Font );

		primaDataDx10 = FALSE;
	}

	CurrentTickCount = clock() * 0.001f;
	Fps++;
	 if((CurrentTickCount - LastTickCount) > 1.0f)
	{
		LastTickCount = CurrentTickCount;
		swprintf_s(FrameRate, L"%d", Fps);
		//_itoa_s(Fps,FrameRate,10);
		Fps = 0;
	}

	//call before you draw
	pDeviceDX10->OMSetRenderTargets(1, &RenderTargetViewDX10, NULL);

	RECT FontRect								= { 35, 50, 0, 0 };
	D3DXCOLOR FontColor( 1.0f, 0.0f, 0.0f, 1.0f );
	if( Font ) Font->DrawTextA( 0, (LPCSTR)FrameRate, wcslen(FrameRate) + 1, &FontRect, DT_NOCLIP, FontColor );

	// Captureaza video
	if (GetAsyncKeyState('R') & 1) { 
		if (captureVideo == 0) { 
			captureVideo = 1;
			//nTimerId=SetTimer(NULL,12345,30,enableVideoRecording);	//Timer set to 33 ms for 30 fps
			m_lpBits = (LPVOID)GlobalAlloc(GPTR, w * h * 4);
			PostMessage(GetServerHwnd(), WM_APP + 6, (WPARAM)0, (LPARAM)0);
		}
		else if (captureVideo == 1) { 
			captureVideo = 0; 
			PostMessage(GetServerHwnd(), WM_APP + 7, (WPARAM)0, (LPARAM)0);
			ReleaseMemory();
		}
	}

	if (captureVideo == 1) {
		pSurfaceDX10 = NULL;
		pSwapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), reinterpret_cast< void** >( &pSurfaceDX10 ) ); 
		pSurfaceDX10->GetDesc( &descriptionDX10 ); 
		descriptionDX10.Width = w;
		descriptionDX10.Height = h;
		descriptionDX10.Format = pDesc.BufferDesc.Format; // DXGI_FORMAT_R8G8B8A8_UNORM
		descriptionDX10.ArraySize = 1;
		descriptionDX10.MiscFlags = 0;
		descriptionDX10.SampleDesc.Count = 1;
		descriptionDX10.SampleDesc.Quality = 0;
		descriptionDX10.MipLevels = 1;

		descriptionDX10.BindFlags = 0;
		descriptionDX10.CPUAccessFlags = D3D10_CPU_ACCESS_READ;
		descriptionDX10.Usage = D3D10_USAGE_STAGING;

		HRESULT hr = pDeviceDX10->CreateTexture2D( &descriptionDX10, NULL, &pNewTextureDX10 );
		if( pNewTextureDX10 )
		{
			pDeviceDX10->CopyResource(pNewTextureDX10, pSurfaceDX10);
			D3D10_MAPPED_TEXTURE2D resource;
			pNewTextureDX10->Map(D3D10CalcSubresource( 0, 0, descriptionDX10.MipLevels), D3D10_MAP_READ, 0, &resource);
			
			const int pitch = resource.RowPitch;
			const unsigned char* source = static_cast< const unsigned char* >( resource.pData );
			unsigned char* dest = static_cast< unsigned char* >(m_lpBits);
			
			for( int i = 0; i < h; ++i )
			{
				memcpy( dest, source, pitch);
				source += pitch;
				dest += pitch;
			}

			void* b = m_lpBits;                       // Pointer To The Buffer
			total = w * h;
			__asm                               // Assembler Code To Follow
			{
				mov ecx, total                   // Set Up A Counter (Dimensions Of Memory Block)
				mov ebx, b                      // Points ebx To Our Data (b)
				label:                          // Label Used For Looping
					mov al,[ebx+0]                  // Loads Value At ebx Into al
					mov ah,[ebx+2]                  // Loads Value At ebx+2 Into ah
					mov [ebx+2],al                  // Stores Value In al At ebx+2
					mov [ebx+0],ah                  // Stores Value In ah At ebx
             
					add ebx,4                   // Moves Through The Data By 4 Bytes
					dec ecx                     // Decreases Our Loop Counter
					jnz label                   // If Not Zero Jump Back To Label
			 }
				
			unsigned int* pPix = (unsigned int*)m_lpBits;

			int rows    = 0;
			int rowsMax = h;
			while( rows < rowsMax )
			{
				unsigned int* pRowStart = pPix + (rows * w);
				unsigned int* pRowEnd   = pRowStart + w;
				std::reverse( pRowStart, pRowEnd );
				rows++;
			}
			
			int temp;
			for(int i = 0; i < total/2; ++i){
			temp = pPix[total - i -1];
			pPix[total - i -1] = pPix[i];
			pPix[i] = temp;
			}	

			AppendNewFrame(w, h, m_lpBits,32);
			pNewTextureDX10->Unmap(D3D10CalcSubresource( 0, 0, descriptionDX10.MipLevels));
			pDeviceDX10->Flush();
			pNewTextureDX10->Release();
			pSurfaceDX10->Release();

		}
	}

	if (GetAsyncKeyState('O') & 1) {
		HRESULT hr;
		ID3D10Resource *backbufferRes;
		g_renderTargetView[0]->GetResource(&backbufferRes);

		D3D10_TEXTURE2D_DESC texDesc;
		texDesc.ArraySize = 1;
		texDesc.BindFlags = 0;
		texDesc.CPUAccessFlags = 0;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.Width = w;  // must be same as backbuffer
		texDesc.Height = h; // must be same as backbuffer
		texDesc.MipLevels = 1;
		texDesc.MiscFlags = 0;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D10_USAGE_DEFAULT;

		ID3D10Texture2D *texture;
		pDeviceDX10->CreateTexture2D(&texDesc, 0, &texture);
		pDeviceDX10->CopyResource(texture, backbufferRes);

		D3DX10SaveTextureToFile(texture, D3DX10_IFF_PNG, L"test.png");
		texture->Release();
		backbufferRes->Release();
	}
	return phookD3DPresent(pSwapChain,SyncInterval, Flags);
}

HRESULT __stdcall hookD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
	if (primaDataDx11) {
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void **)&pDevice)))
		{
			pSwapChain->GetDevice(__uuidof(pDevice), (void**)&pDevice);
			pDevice->GetImmediateContext(&pContext);
		}

		HRESULT hResult = FW1CreateFactory(FW1_VERSION, &pFW1Factory);
		hResult = pFW1Factory->CreateFontWrapper(pDevice, L"Tahoma", &pFontWrapper);
		pFW1Factory->Release();

		if (SUCCEEDED(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&RenderTargetTexture)))
		{
			pDevice->CreateRenderTargetView(RenderTargetTexture, NULL, &RenderTargetView);
			RenderTargetTexture->Release();
		}

		primaDataDx11 = false;
	}

	CurrentTickCount = clock() * 0.001f;
	Fps++;
	 if((CurrentTickCount - LastTickCount) > 1.0f)
	{
		LastTickCount = CurrentTickCount;
		swprintf_s(FrameRate, L"%d", Fps);
		//_itoa_s(Fps,FrameRate,10);
		Fps = 0;
	}

	//call before you draw
	pContext->OMSetRenderTargets(1, &RenderTargetView, NULL);
	//draw
	if (pFontWrapper)
	pFontWrapper->DrawString(pContext, FrameRate, 30.0f, 30.0f, 30.0f, 0xff0000ff, FW1_RESTORESTATE);

	if (GetAsyncKeyState('R') & 1) {
		if (captureVideo == 0) { // START RECORDING
			//pContext->RSGetViewports(&nrViewport, &pViewports);
			pSwapChain->GetDesc(&pDesc);
			w = pDesc.BufferDesc.Width;
			h = pDesc.BufferDesc.Height;
			captureVideo = 1;
			//nTimerId=SetTimer(NULL,12345,30,enableVideoRecording);	//Timer set to 33 ms for 30 fps
			m_lpBits = (LPVOID)GlobalAlloc(GPTR, w * h * 4);
			// send message to client to start sound capture
			PostMessage(GetServerHwnd(), WM_APP + 6, (WPARAM)0, (LPARAM)0);
		}
		else if (captureVideo == 1) { // STOP RECORDING
			captureVideo = 0; // stop capturing
			// send message to client to stop sound capture
			PostMessage(GetServerHwnd(), WM_APP + 7, (WPARAM)0, (LPARAM)0);
			ReleaseMemory();
		}
	}

	// Get the back buffer bits and add the frame to the .avi
	if (captureVideo == 1) {
		pNewTexture = NULL;

		// Use the IDXGISwapChain::GetBuffer API to retrieve a swap chain surface ( use the uuid  ID3D11Texture2D for the result type ).
		pSwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), reinterpret_cast< void** >( &pSurface ) ); 

		/* The swap chain buffers are not mapable, so I need to copy it to a staging resource. */

		pSurface->GetDesc( &description ); //Use ID3D11Texture2D::GetDesc to retrieve the surface description
		
		
		description.Width = w;
		description.Height = h;
		description.Format = pDesc.BufferDesc.Format; // DXGI_FORMAT_R8G8B8A8_UNORM
		description.ArraySize = 1;
		description.MiscFlags = 0;
		description.SampleDesc.Count = 1;
		description.SampleDesc.Quality = 0;
		description.MipLevels = 1;
		

		// Patch it with a D3D11_USAGE_STAGING usage and a cpu access flag of D3D11_CPU_ACCESS_READ
		description.BindFlags = 0;
		description.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		description.Usage = D3D11_USAGE_STAGING;

		// Create a temporary surface ID3D11Device::CreateTexture2D
		HRESULT hr = pDevice->CreateTexture2D( &description, NULL, &pNewTexture );
		if( pNewTexture )
		{
			//pContext->ResolveSubresource(pNewTexture, D3D11CalcSubresource(0, 0, 1), pSurface, D3D11CalcSubresource(0, 0, 1), description.Format);
			//pContext->CopySubresourceRegion(pNewTexture, 0, 0, 0, 0, pSurface, 0, 0);
			// Copy to the staging surface ID3D11DeviceContext::CopyResource
			pContext->CopyResource( pNewTexture, pSurface );
			// Now I have a ID3D11Texture2D with the content of your swap chain buffer that allow you to use the ID3D11DeviceContext::Map API to read it on the CPU
			D3D11_MAPPED_SUBRESOURCE resource;
			pContext->Map( pNewTexture, D3D11CalcSubresource( 0, 0, description.MipLevels), D3D11_MAP_READ, 0, &resource );

			const int pitch = resource.RowPitch;
			const unsigned char* source = static_cast< const unsigned char* >( resource.pData );
			unsigned char* dest = static_cast< unsigned char* >(m_lpBits);
			
			for( int i = 0; i < h; ++i )
			{
				memcpy( dest, source, pitch);
				source += pitch;
				dest += pitch;
			}
			
			//memcpy( m_lpBits, resource.pData, w * h );

			
			/* BGRA -> RGBA */
			
			void* b = m_lpBits;                       // Pointer To The Buffer
			total = w * h;
			__asm                               // Assembler Code To Follow
			{
				mov ecx, total                   // Set Up A Counter (Dimensions Of Memory Block)
				mov ebx, b                      // Points ebx To Our Data (b)
				label:                          // Label Used For Looping
					mov al,[ebx+0]                  // Loads Value At ebx Into al
					mov ah,[ebx+2]                  // Loads Value At ebx+2 Into ah
					mov [ebx+2],al                  // Stores Value In al At ebx+2
					mov [ebx+0],ah                  // Stores Value In ah At ebx
             
					add ebx,4                   // Moves Through The Data By 4 Bytes
					dec ecx                     // Decreases Our Loop Counter
					jnz label                   // If Not Zero Jump Back To Label
			 }
			
			

			/* flip image horizontally */
			
			unsigned int* pPix = (unsigned int*)m_lpBits;

			int rows    = 0;
			int rowsMax = h;
			while( rows < rowsMax )
			{
				unsigned int* pRowStart = pPix + (rows * w);
				unsigned int* pRowEnd   = pRowStart + w;
				std::reverse( pRowStart, pRowEnd );
				rows++;
			}
			
			int temp;
			for(int i = 0; i < total/2; ++i){
			temp = pPix[total - i -1];
			pPix[total - i -1] = pPix[i];
			pPix[i] = temp;
			}	
			
				
			// Add the frame to the movie
			AppendNewFrame(w, h, m_lpBits,32);

			pContext->Unmap( pNewTexture, 0);
			pContext->Flush();
			pNewTexture->Release();
			pSurface->Release();
		}
	}

	/* TAKE SCREENSHOT  */
	if (GetAsyncKeyState('O') & 1) { 
		ID3D11Texture2D *pBackBuffer = NULL;
		pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
 
		D3D11_TEXTURE2D_DESC resolveDesc;
		pBackBuffer->GetDesc(&resolveDesc);
 
		resolveDesc.Usage = D3D11_USAGE_DEFAULT;
		resolveDesc.BindFlags = 0;
		resolveDesc.CPUAccessFlags = 0;
		resolveDesc.SampleDesc.Count = 1;    //resolved buffer is not multisampled
		resolveDesc.SampleDesc.Quality = 0;
 
		ID3D11Texture2D *pResolveBuffer = NULL;
		pDevice->CreateTexture2D(&resolveDesc, NULL, &pResolveBuffer);
		pContext->ResolveSubresource(pResolveBuffer, D3D11CalcSubresource(0, 0, 1), pBackBuffer, D3D11CalcSubresource(0, 0, 1), resolveDesc.Format);
 
		D3DX11SaveTextureToFileA(pContext, pResolveBuffer, D3DX11_IFF_PNG, "C:\\xxx.png");
 
		pBackBuffer->Release();
		pResolveBuffer->Release();
	}

	return phookD3DPresent(pSwapChain, SyncInterval, Flags);
}

HRESULT __stdcall hookD3DPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
	if(primaDataDX) {
		primaDataDX = false;
		if(FAILED(pSwapChain->GetDevice(__uuidof(ID3D10Device), (PVOID*)&pDeviceDX10))) {
			isDirectx11 = TRUE;
		}
		else {
			isDirectx10 = TRUE;
		}
	}
	if (isDirectx11) hookD3D11Present(pSwapChain, SyncInterval, Flags);
	else if (isDirectx10) hookD3D10Present(pSwapChain, SyncInterval, Flags);
	else return phookD3DPresent(pSwapChain,SyncInterval, Flags);
}

VOID hook_vkCmdDrawIndexed(VkCommandBuffer commandBuffer,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,int32_t vertexOffset,int32_t firstInstance) {
	if (GetAsyncKeyState('R') & 1) {}
	if (GetAsyncKeyState('O') & 1) {
		MessageBoxA(NULL, "Inside vulkan cmddraw", "Error", MB_OK);
	}
	return orig_vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

VkResult hook_vkCreateDevice(VkPhysicalDevice physicalDevice,const VkDeviceCreateInfo* pCreateInfo,const VkAllocationCallbacks* pAllocator,VkDevice* pDevice) {
	MessageBoxA(NULL, "Inside vulkan cmddraw", "Error", MB_OK);
	// Creeaza device si swpapchain inainte
	if (GetAsyncKeyState('O') & 1) {
		MessageBoxA(NULL, "Inside vulkan cmddraw", "Error", MB_OK);
	}
	return orig_vkCreateDevice(physicalDevice,pCreateInfo,pAllocator,pDevice);
}

VkResult hook_vkCreateSwapchainKHR(VkDevice device,const VkSwapchainCreateInfoKHR* pCreateInfo,const VkAllocationCallbacks* pAllocator,VkSwapchainKHR* pSwapchain) {
	if(primaDataVulkan) {
		pSwapchainVk = pSwapchain;
		primaDataVulkan = FALSE;
		MessageBoxA(NULL, "Took a pointer to the VK swapchain interface ", "Error", MB_OK);
	}
	if (GetAsyncKeyState('t') & 1) {
		MessageBoxA(NULL, "Took a pointer to the VK swapchain interface", "Error", MB_OK);
	}
	return orig_vkCreateSwapchainKHR(device,pCreateInfo,pAllocator,pSwapchain);
}

VkResult hook_vkAcquireNextImageKHR(VkDevice device,VkSwapchainKHR swapchain,uint64_t timeout,VkSemaphore semaphore,VkFence fence,uint32_t* pImageIndex) {
	
		MessageBoxA(NULL, "Before acquiring next image", "Error", MB_OK);
	
	return orig_vkAcquireNextImageKHR(device,swapchain,timeout,semaphore, fence,pImageIndex);
}

VkResult hook_vkQueuePresentKHR(VkQueue queue,const VkPresentInfoKHR* pPresentInfo) {
	MessageBoxA(NULL, "Before acquiring next image", "Error", MB_OK);
	if (GetAsyncKeyState('R') & 1) {
		MessageBoxA(NULL, "Took a pointer to the VK swapchain interface", "Error", MB_OK);
	}
	return orig_vkQueuePresentKHR(queue, pPresentInfo);
}

void HookFunction(DWORD* pVtable, void* pHookProc, void* pOldProc, int iIndex)
{
    // Enable writing to the vtable at address we aquired
    DWORD lpflOldProtect;
	//MessageBoxA(NULL, "Hooking function!", "Error", MB_OK);
    VirtualProtect((void*)&pVtable[iIndex], sizeof(DWORD), PAGE_EXECUTE_READWRITE, &lpflOldProtect);
    // Store old address
    if (pOldProc) {
        *(DWORD*)pOldProc = pVtable[iIndex];
    }
 
    // Overwrite original address
    pVtable[iIndex] = (DWORD)pHookProc;
 
    // Restore protection
    VirtualProtect((void*)pVtable[iIndex], sizeof(DWORD), lpflOldProtect, &lpflOldProtect);
}

VOID CALLBACK enableVideoRecording(HWND x, UINT y, UINT_PTR z, DWORD w) {
	captureVideo = 1;
}

extern "C" DLLEXPORT void SetServerHwnd(HWND hwnd) {
	serverHwnd = hwnd;
}
extern "C" DLLEXPORT  HWND GetServerHwnd() {
	return serverHwnd;
}

extern "C"  DLLEXPORT void SetComboBoxAID(int x) {
	indexComboBoxAID = x;
}
extern "C" DLLEXPORT int GetComboBoxAID() {
	return indexComboBoxAID;
}

extern "C"  DLLEXPORT void SetComboBoxChannel(int x) {
	indexComboBoxChannel = x;
}
extern "C" DLLEXPORT int GetComboBoxChannel() {
	return indexComboBoxChannel;
}

extern "C"  DLLEXPORT void SetComboBoxDF(int x) {
	indexComboBoxDF = x;
}
extern "C" DLLEXPORT int GetComboBoxDF() {
	return indexComboBoxDF;
}

extern "C"  DLLEXPORT void SetEditBl(int x) {
	editBl = x;
}
extern "C" DLLEXPORT int GetEditBl() {
	return editBl;
}

extern "C"  DLLEXPORT void SetCheckBoxShowFPS(bool x) {
	CheckBoxShowFPS = x;
}
extern "C" DLLEXPORT bool GetCheckBoxShowFPS() {
	return CheckBoxShowFPS;
}

extern "C"  DLLEXPORT void SetTargetFR(int x) {
	targetFR = x;
}
extern "C" DLLEXPORT int GetTargetFR() {
	return targetFR;
}

extern "C"  DLLEXPORT void SetIndexComboBoxVC(int x) {
	indexComboBoxVC = x;
}
extern "C" DLLEXPORT int GetIndexComboBoxVC() {
	return indexComboBoxVC;
}

extern "C"  DLLEXPORT void SetLpstrEditVRS(LPSTR x) {
	lpstrEditVRS = x;
}
extern "C" DLLEXPORT LPSTR GetLpstrEditVRS() {
	return lpstrEditVRS;
}

extern "C"  DLLEXPORT void SetLpstrEditVRP(LPSTR x) {
	lpstrEditVRP = x;
}
extern "C" DLLEXPORT LPSTR GetLpstrEditVRP() {
	return lpstrEditVRP;
}

extern "C"  DLLEXPORT void SetLpstrEditScreen(LPSTR x) {
	lpstrEditScreen = x;
}
extern "C" DLLEXPORT LPSTR GetLpstrEditScreen() {
	return lpstrEditScreen;
}


void createSwapChain() {
	    /*
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = {(uint32_t) indices.graphicsFamily, (uint32_t) indices.presentFamily};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;

        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
		*/
    }
