#include "stdafx.h"
#include "WavFile.h"
#include <strsafe.h>
#define WAVEFILE_READ   1
#define WAVEFILE_WRITE  2


void ErrorExit2(LPTSTR lpszFunction) 
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

//-----------------------------------------------------------------------------
// Name: CWaveFile::CWaveFile()
// Desc: Constructs the class.  Call Open() to open a wave file for reading.
//       Then call Read() as needed.  Calling the destructor or Close()
//       will close the file.
//-----------------------------------------------------------------------------
CWaveFile::CWaveFile()
{
    m_pwfx    = NULL;
    m_hmmio   = NULL;
    m_pResourceBuffer = NULL;
    m_dwSize  = 0;
    m_bIsReadingFromMemory = FALSE;
}


//-----------------------------------------------------------------------------
// Name: CWaveFile::~CWaveFile()
// Desc: Destructs the class
//-----------------------------------------------------------------------------
CWaveFile::~CWaveFile()
{
    Close();

    if( !m_bIsReadingFromMemory )
        SAFE_DELETE_ARRAY( m_pwfx );
}


//-----------------------------------------------------------------------------
// Name: CWaveFile::Open()
// Desc: Opens a wave file for reading
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Open( LPWSTR strFileName, WAVEFORMATEX* pwfx, DWORD dwFlags )
{
    HRESULT hr;

    m_dwFlags = dwFlags;
    m_bIsReadingFromMemory = FALSE;

    if( m_dwFlags == WAVEFILE_READ )
    {
        if( strFileName == NULL )
            return E_INVALIDARG;
        SAFE_DELETE_ARRAY( m_pwfx );

        m_hmmio = mmioOpen( (LPSTR)strFileName, NULL, MMIO_ALLOCBUF | MMIO_READ );

        if( NULL == m_hmmio )
        {
            HRSRC   hResInfo;
            HGLOBAL hResData;
            DWORD   dwSize;
            VOID*   pvRes;

            // Loading it as a file failed, so try it as a resource
            if( NULL == ( hResInfo = FindResource( NULL, (LPSTR)strFileName, (LPCSTR)"WAVE" ) ) )
            {
                if( NULL == ( hResInfo = FindResource( NULL, (LPSTR)strFileName, (LPCSTR)"WAV" ) ) )
                    //return DXUT_ERR( L"FindResource", E_FAIL );
					ErrorExit2(TEXT("FindResource"));
            }

            if( NULL == ( hResData = LoadResource( GetModuleHandle(NULL), hResInfo ) ) )
                //return DXUT_ERR( L"LoadResource", E_FAIL );
					ErrorExit2(TEXT("LoadResource"));

            if( 0 == ( dwSize = SizeofResource( GetModuleHandle(NULL), hResInfo ) ) )
                //return DXUT_ERR( L"SizeofResource", E_FAIL );
					ErrorExit2(TEXT("SizeofResource"));

            if( NULL == ( pvRes = LockResource( hResData ) ) )
               //return DXUT_ERR( L"LockResource", E_FAIL );
				   ErrorExit2(TEXT("LockResource"));

            m_pResourceBuffer = new CHAR[ dwSize ];
            if( m_pResourceBuffer == NULL )
                //return DXUT_ERR( L"new", E_OUTOFMEMORY );
					ErrorExit2(TEXT("new"));
            memcpy( m_pResourceBuffer, pvRes, dwSize );

            MMIOINFO mmioInfo;
            ZeroMemory( &mmioInfo, sizeof(mmioInfo) );
            mmioInfo.fccIOProc = FOURCC_MEM;
            mmioInfo.cchBuffer = dwSize;
            mmioInfo.pchBuffer = (CHAR*) m_pResourceBuffer;

            m_hmmio = mmioOpen( NULL, &mmioInfo, MMIO_ALLOCBUF | MMIO_READ );
        }

        if( FAILED( hr = ReadMMIO() ) )
        {
            // ReadMMIO will fail if its an not a wave file
            mmioClose( m_hmmio, 0 );
            //return DXUT_ERR( L"ReadMMIO", hr );
			ErrorExit2(TEXT("ReadMMIO"));
        }

        if( FAILED( hr = ResetFile() ) )
            //return DXUT_ERR( L"ResetFile", hr );
				ErrorExit2(TEXT("ResetFile"));

        // After the reset, the size of the wav file is m_ck.cksize so store it now
        m_dwSize = m_ck.cksize;
    }
    else
    {
        m_hmmio = mmioOpen( (LPSTR)strFileName, NULL, MMIO_ALLOCBUF  |
                                                  MMIO_READWRITE |
                                                  MMIO_CREATE );
        if( NULL == m_hmmio )
            //return DXUT_ERR( L"mmioOpen", E_FAIL );
				ErrorExit2(TEXT("mmioOpen"));

        if( FAILED( hr = WriteMMIO( pwfx ) ) )
        {
            mmioClose( m_hmmio, 0 );
            //return DXUT_ERR( L"WriteMMIO", hr );
			ErrorExit2(TEXT("WriteMMIO"));
        }

        if( FAILED( hr = ResetFile() ) )
            //return DXUT_ERR( L"ResetFile", hr );
				ErrorExit2(TEXT("ResetFile"));
    }

    return hr;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Write()
// Desc: Writes data to the open wave file
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Write( UINT nSizeToWrite, BYTE* pbSrcData, UINT* pnSizeWrote )
{
    UINT cT;

    if( m_bIsReadingFromMemory )
        return E_NOTIMPL;
    if( m_hmmio == NULL )
        return CO_E_NOTINITIALIZED;
    if( pnSizeWrote == NULL || pbSrcData == NULL )
        return E_INVALIDARG;

    *pnSizeWrote = 0;

    for( cT = 0; cT < nSizeToWrite; cT++ )
    {
        if( m_mmioinfoOut.pchNext == m_mmioinfoOut.pchEndWrite )
        {
            m_mmioinfoOut.dwFlags |= MMIO_DIRTY;
            if( 0 != mmioAdvance( m_hmmio, &m_mmioinfoOut, MMIO_WRITE ) )
                //return DXUT_ERR( L"mmioAdvance", E_FAIL );
					ErrorExit2(TEXT("mmioAdvance"));
        }

        *((BYTE*)m_mmioinfoOut.pchNext) = *((BYTE*)pbSrcData+cT);
        (BYTE*)m_mmioinfoOut.pchNext++;

        (*pnSizeWrote)++;
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::Close()
// Desc: Closes the wave file
//-----------------------------------------------------------------------------
HRESULT CWaveFile::Close()
{
    if( m_dwFlags == WAVEFILE_READ )
    {
        mmioClose( m_hmmio, 0 );
        m_hmmio = NULL;
        SAFE_DELETE_ARRAY( m_pResourceBuffer );
    }
    else
    {
        m_mmioinfoOut.dwFlags |= MMIO_DIRTY;

        if( m_hmmio == NULL )
            return CO_E_NOTINITIALIZED;

        if( 0 != mmioSetInfo( m_hmmio, &m_mmioinfoOut, 0 ) )
            //return DXUT_ERR( L"mmioSetInfo", E_FAIL );
				ErrorExit2(TEXT("mmioSetInfo"));

        // Ascend the output file out of the 'data' chunk -- this will cause
        // the chunk size of the 'data' chunk to be written.
        if( 0 != mmioAscend( m_hmmio, &m_ck, 0 ) )
            //return DXUT_ERR( L"mmioAscend", E_FAIL );
				ErrorExit2(TEXT("mmioAscend"));

        // Do this here instead...
        if( 0 != mmioAscend( m_hmmio, &m_ckRiff, 0 ) )
            //return DXUT_ERR( L"mmioAscend", E_FAIL );
				ErrorExit2(TEXT("mmioAscend"));

        mmioSeek( m_hmmio, 0, SEEK_SET );

        if( 0 != (INT)mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) )
            //return DXUT_ERR( L"mmioDescend", E_FAIL );
				ErrorExit2(TEXT("mmioDescend"));

        m_ck.ckid = mmioFOURCC('f', 'a', 'c', 't');

        if( 0 == mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) )
        {
            DWORD dwSamples = 0;
            mmioWrite( m_hmmio, (HPSTR)&dwSamples, sizeof(DWORD) );
            mmioAscend( m_hmmio, &m_ck, 0 );
        }

        // Ascend the output file out of the 'RIFF' chunk -- this will cause
        // the chunk size of the 'RIFF' chunk to be written.
        if( 0 != mmioAscend( m_hmmio, &m_ckRiff, 0 ) )
            //return DXUT_ERR( L"mmioAscend", E_FAIL );
				ErrorExit2(TEXT("mmioAscend"));

        mmioClose( m_hmmio, 0 );
        m_hmmio = NULL;
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::WriteMMIO()
// Desc: Support function for reading from a multimedia I/O stream
//       pwfxDest is the WAVEFORMATEX for this new wave file.
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_ck.
//-----------------------------------------------------------------------------
HRESULT CWaveFile::WriteMMIO( WAVEFORMATEX *pwfxDest )
{
    DWORD    dwFactChunk; // Contains the actual fact chunk. Garbage until WaveCloseWriteFile.
    MMCKINFO ckOut1;

    dwFactChunk = (DWORD)-1;

    // Create the output file RIFF chunk of form type 'WAVE'.
    m_ckRiff.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    m_ckRiff.cksize = 0;

    if( 0 != mmioCreateChunk( m_hmmio, &m_ckRiff, MMIO_CREATERIFF ) )
        //return DXUT_ERR( L"mmioCreateChunk", E_FAIL );
			ErrorExit2(TEXT("mmioCreateChunk"));

    // We are now descended into the 'RIFF' chunk we just created.
    // Now create the 'fmt ' chunk. Since we know the size of this chunk,
    // specify it in the MMCKINFO structure so MMIO doesn't have to seek
    // back and set the chunk size after ascending from the chunk.
    m_ck.ckid = mmioFOURCC('f', 'm', 't', ' ');
    m_ck.cksize = sizeof(PCMWAVEFORMAT);

    if( 0 != mmioCreateChunk( m_hmmio, &m_ck, 0 ) )
        //return DXUT_ERR( L"mmioCreateChunk", E_FAIL );
			ErrorExit2(TEXT("mmioCreateChunk"));

    // Write the PCMWAVEFORMAT structure to the 'fmt ' chunk if its that type.
    if( pwfxDest->wFormatTag == WAVE_FORMAT_PCM )
    {
        if( mmioWrite( m_hmmio, (HPSTR) pwfxDest,
                       sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT))
            //return DXUT_ERR( L"mmioWrite", E_FAIL );
			ErrorExit2(TEXT("mmioWrite"));
    }
    else
    {
        // Write the variable length size.
        if( (UINT)mmioWrite( m_hmmio, (HPSTR) pwfxDest,
                             sizeof(*pwfxDest) + pwfxDest->cbSize ) !=
                             ( sizeof(*pwfxDest) + pwfxDest->cbSize ) )
            //return DXUT_ERR( L"mmioWrite", E_FAIL );
			ErrorExit2(TEXT("mmioWrite"));
    }

    // Ascend out of the 'fmt ' chunk, back into the 'RIFF' chunk.
    if( 0 != mmioAscend( m_hmmio, &m_ck, 0 ) )
        //return DXUT_ERR( L"mmioAscend", E_FAIL );
			ErrorExit2(TEXT("mmioAscend"));

    // Now create the fact chunk, not required for PCM but nice to have.  This is filled
    // in when the close routine is called.
    ckOut1.ckid = mmioFOURCC('f', 'a', 'c', 't');
    ckOut1.cksize = 0;

    if( 0 != mmioCreateChunk( m_hmmio, &ckOut1, 0 ) )
        //return DXUT_ERR( L"mmioCreateChunk", E_FAIL );
			ErrorExit2(TEXT("mmioCreateChunk"));

    if( mmioWrite( m_hmmio, (HPSTR)&dwFactChunk, sizeof(dwFactChunk)) !=
                    sizeof(dwFactChunk) )
         //return DXUT_ERR( L"mmioWrite", E_FAIL );
		 ErrorExit2(TEXT("mmioWrite"));

    // Now ascend out of the fact chunk...
    if( 0 != mmioAscend( m_hmmio, &ckOut1, 0 ) )
        //return DXUT_ERR( L"mmioAscend", E_FAIL );
			ErrorExit2(TEXT("mmioAscend"));

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::ResetFile()
// Desc: Resets the internal m_ck pointer so reading starts from the
//       beginning of the file again
//-----------------------------------------------------------------------------
HRESULT CWaveFile::ResetFile()
{
    if( m_bIsReadingFromMemory )
    {
        m_pbDataCur = m_pbData;
    }
    else
    {
        if( m_hmmio == NULL )
            return CO_E_NOTINITIALIZED;

        if( m_dwFlags == WAVEFILE_READ )
        {
            // Seek to the data
            if( -1 == mmioSeek( m_hmmio, m_ckRiff.dwDataOffset + sizeof(FOURCC),
                            SEEK_SET ) )
                //return DXUT_ERR( L"mmioSeek", E_FAIL );
				ErrorExit2(TEXT("mmioSeek"));

            // Search the input file for the 'data' chunk.
            m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
            if( 0 != mmioDescend( m_hmmio, &m_ck, &m_ckRiff, MMIO_FINDCHUNK ) )
              //return DXUT_ERR( L"mmioDescend", E_FAIL );
				  ErrorExit2(TEXT("mmioDescend"));
        }
        else
        {
            // Create the 'data' chunk that holds the waveform samples.
            m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
            m_ck.cksize = 0;

            if( 0 != mmioCreateChunk( m_hmmio, &m_ck, 0 ) )
                //return DXUT_ERR( L"mmioCreateChunk", E_FAIL );
					ErrorExit2(TEXT("mmioCreateChunk"));

            if( 0 != mmioGetInfo( m_hmmio, &m_mmioinfoOut, 0 ) )
                //return DXUT_ERR( L"mmioGetInfo", E_FAIL );
					ErrorExit2(TEXT("mmioGetInfo"));
        }
    }

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: CWaveFile::ReadMMIO()
// Desc: Support function for reading from a multimedia I/O stream.
//       m_hmmio must be valid before calling.  This function uses it to
//       update m_ckRiff, and m_pwfx.
//-----------------------------------------------------------------------------
HRESULT CWaveFile::ReadMMIO()
{
    MMCKINFO        ckIn;           // chunk info. for general use.
    PCMWAVEFORMAT   pcmWaveFormat;  // Temp PCM structure to load in.

    m_pwfx = NULL;

    if( ( 0 != mmioDescend( m_hmmio, &m_ckRiff, NULL, 0 ) ) )
        //return DXUT_ERR( L"mmioDescend", E_FAIL );
			ErrorExit2(TEXT("mmioDescend"));

    // Check to make sure this is a valid wave file
    if( (m_ckRiff.ckid != FOURCC_RIFF) ||
        (m_ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E') ) )
        //return DXUT_ERR( L"mmioFOURCC", E_FAIL );
		ErrorExit2(TEXT("mmioFOURCC"));

    // Search the input file for for the 'fmt ' chunk.
    ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');
    if( 0 != mmioDescend( m_hmmio, &ckIn, &m_ckRiff, MMIO_FINDCHUNK ) )
        //return DXUT_ERR( L"mmioDescend", E_FAIL );
			ErrorExit2(TEXT("mmioDescend"));

    // Expect the 'fmt' chunk to be at least as large as <PCMWAVEFORMAT>;
    // if there are extra parameters at the end, we'll ignore them
       if( ckIn.cksize < (LONG) sizeof(PCMWAVEFORMAT) )
           //return DXUT_ERR( L"sizeof(PCMWAVEFORMAT)", E_FAIL );
			   ErrorExit2(TEXT("sizeof(PCMWAVEFORMAT)"));

    // Read the 'fmt ' chunk into <pcmWaveFormat>.
    if( mmioRead( m_hmmio, (HPSTR) &pcmWaveFormat,
                  sizeof(pcmWaveFormat)) != sizeof(pcmWaveFormat) )
        //return DXUT_ERR( L"mmioRead", E_FAIL );
		ErrorExit2(TEXT("mmioRead"));

    // Allocate the waveformatex, but if its not pcm format, read the next
    // word, and thats how many extra bytes to allocate.
    if( pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM )
    {
        m_pwfx = (WAVEFORMATEX*)new CHAR[ sizeof(WAVEFORMATEX) ];
        if( NULL == m_pwfx )
            //return DXUT_ERR( L"m_pwfx", E_FAIL );
				ErrorExit2(TEXT("m_pwfx"));

        // Copy the bytes from the pcm structure to the waveformatex structure
        memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
        m_pwfx->cbSize = 0;
    }
    else
    {
        // Read in length of extra bytes.
        WORD cbExtraBytes = 0L;
        if( mmioRead( m_hmmio, (CHAR*)&cbExtraBytes, sizeof(WORD)) != sizeof(WORD) )
            //return DXUT_ERR( L"mmioRead", E_FAIL );
				ErrorExit2(TEXT("mmioRead"));

        m_pwfx = (WAVEFORMATEX*)new CHAR[ sizeof(WAVEFORMATEX) + cbExtraBytes ];
        if( NULL == m_pwfx )
            //return DXUT_ERR( L"new", E_FAIL );
				ErrorExit2(TEXT("new"));

        // Copy the bytes from the pcm structure to the waveformatex structure
        memcpy( m_pwfx, &pcmWaveFormat, sizeof(pcmWaveFormat) );
        m_pwfx->cbSize = cbExtraBytes;

        // Now, read those extra bytes into the structure, if cbExtraAlloc != 0.
        if( mmioRead( m_hmmio, (CHAR*)(((BYTE*)&(m_pwfx->cbSize))+sizeof(WORD)),
                      cbExtraBytes ) != cbExtraBytes )
        {
            SAFE_DELETE( m_pwfx );
            //return DXUT_ERR( L"mmioRead", E_FAIL );
			ErrorExit2(TEXT("mmioRead"));
        }
    }

    // Ascend the input file out of the 'fmt ' chunk.
    if( 0 != mmioAscend( m_hmmio, &ckIn, 0 ) )
    {
        SAFE_DELETE( m_pwfx );
        //return DXUT_ERR( L"mmioAscend", E_FAIL );
		ErrorExit2(TEXT("mmioAscend"));
    }

    return S_OK;
}
