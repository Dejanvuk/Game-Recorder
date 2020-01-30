#ifndef __DLLMAIN_H
#define __DLLMAIN_H

#ifdef  MYDLL_EXPORTS 
    /*Enabled as "export" while compiling the dll project*/
    #define DLLEXPORT __declspec(dllexport)  
 #else
    /*Enabled as "import" in the Client side for using already created dll file*/
    #define DLLEXPORT __declspec(dllimport)  
 #endif

// Exported functions

extern "C"  DLLEXPORT void SetServerHwnd(HWND hwnd);
extern "C" DLLEXPORT HWND GetServerHwnd();

extern "C"  DLLEXPORT void SetComboBoxAID(int x); 
extern "C" DLLEXPORT int GetComboBoxAID();
extern "C"  DLLEXPORT void SetComboBoxChannel(int);
extern "C" DLLEXPORT int GetComboBoxChannel();
extern "C"  DLLEXPORT void SetComboBoxDF(int);
extern "C" DLLEXPORT int GetComboBoxDF();
extern "C"  DLLEXPORT void SetEditBl(int);
extern "C" DLLEXPORT int GetEditBl();
extern "C"  DLLEXPORT void SetCheckBoxShowFPS(bool);
extern "C" DLLEXPORT bool GetCheckBoxShowFPS();
extern "C"  DLLEXPORT void SetTargetFR(int);
extern "C" DLLEXPORT int GetTargetFR();
extern "C"  DLLEXPORT void SetIndexComboBoxVC(int);
extern "C" DLLEXPORT int GetIndexComboBoxVC();
extern "C"  DLLEXPORT void SetLpstrEditVRS(LPSTR);
extern "C" DLLEXPORT LPSTR GetLpstrEditVRS();
extern "C"  DLLEXPORT void SetLpstrEditVRP(LPSTR);
extern "C" DLLEXPORT LPSTR GetLpstrEditVRP();
extern "C"  DLLEXPORT void SetLpstrEditScreen(LPSTR);
extern "C" DLLEXPORT LPSTR GetLpstrEditScreen();


#endif