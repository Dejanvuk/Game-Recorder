#pragma once

#ifdef AVIFILE_EXPORTS 
#define AVIFILE_API __declspec(dllexport)   
#else  
#define AVIFILE_API __declspec(dllimport)   
#endif

#include <vfw.h>

class __declspec(dllexport) CAviFile
{
	HDC				m_hAviDC;
	HANDLE			m_hHeap;
	LPVOID			m_lpBits;
	LONG			m_lSample;
	TCHAR			m_szFileName[MAX_PATH];
	PAVIFILE		m_pAviFile;
	PAVISTREAM		m_pAviStream;
	PAVISTREAM		m_pAviCompressedStream;
	AVISTREAMINFO	m_AviStreamInfo;
	AVICOMPRESSOPTIONS	m_AviCompressOptions;

	int		nAppendFuncSelector;		//0=Dummy	1=FirstTime	2=Usual
	HRESULT	AppendFrameFirstTime(HBITMAP );
	HRESULT	AppendFrameUsual(HBITMAP);
	HRESULT	AppendDummy(HBITMAP);
	HRESULT	(CAviFile::*pAppendFrame[3])(HBITMAP hBitmap);

	HRESULT	AppendFrameFirstTime(int, int, LPVOID,int );
	HRESULT	AppendFrameUsual(int, int, LPVOID,int );
	HRESULT	AppendDummy(int, int, LPVOID,int );
	HRESULT	(CAviFile::*pAppendFrameBits[3])(int, int, LPVOID,int );
	void	ReleaseMemory();
public:
	CAviFile(LPCSTR	lpszFileName=_T("Output.avi"));
	~CAviFile(void);
	HRESULT	AppendNewFrame(HBITMAP hBitmap);
	HRESULT	AppendNewFrame(int nWidth, int nHeight, LPVOID pBits,int nBitsPerPixel=32);
};

