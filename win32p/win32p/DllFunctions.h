#ifndef __DLLFUNCTIONS_H
#define __DLLFUNCTIONS_H

/* Pointer to functions, will hold the original functions */
typedef HMODULE (WINAPI *LoadLibrary_t)(LPCSTR);
typedef HMODULE (WINAPI *LoadLibraryW_t)(LPCSTR);
typedef HMODULE (WINAPI *LoadLibraryEx_t)(LPCSTR, HANDLE, DWORD);
// DX 9 
typedef IDirect3D9* (__stdcall *Direct3DCreate9_t)(UINT SDKVersion);
typedef HRESULT (APIENTRY *CreateDevice_t)(IDirect3D9*,UINT,D3DDEVTYPE,HWND,DWORD,D3DPRESENT_PARAMETERS*,IDirect3DDevice9**);
typedef HRESULT (APIENTRY *EndScene_t)(IDirect3DDevice9*);
typedef HRESULT (APIENTRY *Present_t)(IDirect3DDevice9*,const RECT* ,const RECT* , HWND ,const RGNDATA*);
typedef HRESULT (APIENTRY *Reset_t)(IDirect3DDevice9*,D3DPRESENT_PARAMETERS*);
typedef HRESULT (STDMETHODCALLTYPE *GETBACKBUFFER)(IDirect3DDevice9*, UINT,UINT, DWORD, LPVOID);
typedef HRESULT (STDMETHODCALLTYPE *LOCKRECT)(IDirect3DSurface9*, LPVOID, LPVOID, DWORD);
// OPENGL
typedef BOOL (STDMETHODCALLTYPE *wglSwapBuffers_t) (_In_ HDC hDc);
// DX 10
typedef HRESULT(__stdcall *D3D10PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
// DX11
typedef HRESULT(__stdcall *D3D11PresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
// DX 10,11,12
typedef HRESULT(__stdcall *D3DPresentHook) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
// VULKAN
typedef VOID (*vkCmdDrawIndexedHook)(VkCommandBuffer,uint32_t ,uint32_t ,uint32_t,int32_t,int32_t);
typedef VkResult (*vkCreateDeviceHook)(VkPhysicalDevice,const VkDeviceCreateInfo*,const VkAllocationCallbacks*,VkDevice*);
typedef VkResult (*vkCreateSwapchainKHRHook)(VkDevice,const VkSwapchainCreateInfoKHR*,const VkAllocationCallbacks*,VkSwapchainKHR*);
typedef VkResult (*vkAcquireNextImageKHRHook)(VkDevice,VkSwapchainKHR,uint64_t,VkSemaphore,VkFence,uint32_t*);
typedef VkResult (*vkQueuePresentKHRHook)(VkQueue,const VkPresentInfoKHR*);

/* our hooked functions which will replace the original*/
HMODULE WINAPI LoadLibrary_Hook ( LPCSTR lpFileName );
HMODULE WINAPI LoadLibraryW_Hook ( LPCSTR lpFileName );
HMODULE WINAPI LoadLibraryEx_Hook ( LPCSTR lpFileName, HANDLE, DWORD);
// DX9
IDirect3D9* __stdcall hook_Direct3DCreate9(UINT sdkVers);
HRESULT APIENTRY hook_CreateDevice(IDirect3D9* pInterface, UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface);
HRESULT APIENTRY hook_EndScene(IDirect3DDevice9* pInterface);
HRESULT APIENTRY hook_Present (IDirect3DDevice9* , const RECT* ,const RECT* , HWND ,const RGNDATA*);
HRESULT APIENTRY hook_Reset (IDirect3DDevice9*,D3DPRESENT_PARAMETERS*);
// OPENGL
BOOL APIENTRY hook_wglSwapBuffers(_In_ HDC hDc);
// DX10
HRESULT __stdcall hookD3D10Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
// DX11
HRESULT __stdcall hookD3D11Present(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
//DIRECTX 10,11,12
HRESULT __stdcall hookD3DPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
// VULKAN
VOID hook_vkCmdDrawIndexed(VkCommandBuffer commandBuffer,uint32_t indexCount,uint32_t instanceCount,uint32_t firstIndex,int32_t vertexOffset,int32_t firstInstance);
VkResult hook_vkCreateDevice(VkPhysicalDevice physicalDevice,const VkDeviceCreateInfo* pCreateInfo,const VkAllocationCallbacks* pAllocator,VkDevice* pDevice);
VkResult hook_vkCreateSwapchainKHR(VkDevice device,const VkSwapchainCreateInfoKHR* pCreateInfo,const VkAllocationCallbacks* pAllocator,VkSwapchainKHR* pSwapchain);
VkResult hook_vkAcquireNextImageKHR(VkDevice device,VkSwapchainKHR swapchain,uint64_t timeout,VkSemaphore semaphore,VkFence fence,uint32_t* pImageIndex);
VkResult hook_vkQueuePresentKHR(VkQueue queue,const VkPresentInfoKHR* pPresentInfo);

// other function definitons
void HookAPI();
void HookFunction(DWORD* pVtable, void* pHookProc, void* pOldProc, int iIndex);
VOID CALLBACK enableVideoRecording(HWND x, UINT y, UINT_PTR z, DWORD w); // called every 33 ms
BOOL hookDX9(void);
BOOL hookDX11(void);
BOOL hookOpenGL();
DWORD __stdcall InitializeHook(LPVOID);

#endif