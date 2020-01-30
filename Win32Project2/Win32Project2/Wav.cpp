#include "stdafx.h"
#include "Wav.h"
#include <dllmain.h>
#pragma comment(lib, "win32p.lib")


HRESULT createWav(wav_header* pWav) {
	pWav->pFile = fopen("sound.wav","w");
	if(pWav->pFile == NULL) {
		return 0;
	}
	strcpy(pWav->riff_header, "RIFF");
	strcpy(pWav->wave_header, "WAVE");
	strcpy(pWav->fmt_header, "fmt ");
	pWav->fmt_chunk_size = 16;
	pWav->audio_format = 1;
	pWav->num_channels = GetComboBoxChannel() + 1; // index 0

	if(GetComboBoxDF() == 0) {
		pWav->sample_rate = 44100; // 44.1 KHz - CD-quality audio 
		pWav->bit_depth = 16; // Bits per Sample – The number of bits available for one sample. 
	}
	else if(GetComboBoxDF() == 1) {
		pWav->sample_rate = 48000; // 48kHz - DVD quality audio
		pWav->bit_depth = 16; 
	}
	else if (GetComboBoxDF() == 2) {
		pWav->sample_rate = 96000; // 96 KHz - Studio-quality audio  
		pWav->bit_depth = 16; 
	}
	else if (GetComboBoxDF() == 3) {
		pWav->sample_rate = 192000; // 192 KHz - Studio-quality audio  
		pWav->bit_depth = 24;  
	}

	pWav->sample_alignment = pWav->num_channels * (pWav->bit_depth)/8; // This is the number of bytes in a frame
	pWav->byte_rate = pWav->sample_rate * pWav->sample_alignment; // number of bytes per second captured
	strcpy(pWav->data_header, "data");
	pWav->data_bytes = 0; // n * frames , n = 0 empty data default, to be updated each time new data is added
	pWav->wav_size = 36 + pWav->data_bytes; // to be updated each time new data is added

	return S_OK;
}

HRESULT closeWav(FILE * pFile) {
	fclose(pFile);
	return S_OK;
}

HRESULT addData(wav_header* pWav, VOID* data,UINT dataLength, UINT* dataWrote) {
	// set the positon back at the end of the file
	fseek (pWav->pFile, 0 , SEEK_END );
	*dataWrote = fwrite((const char*)data,sizeof(byte),(size_t)dataLength, pWav->pFile);


	//pWav->wav_size += dataLength;
	//fwrite((const void *)pWav->wav_size,sizeof(int),1,pWav->pFile);
	return S_OK;
}