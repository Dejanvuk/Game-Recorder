#include "stdafx.h"
#include "CppUnitTest.h"
#include "Wav.h"
#include "TestGUI.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

/* METODE DE ADAUGAT
Citit fiecare byte din fisier
*/


namespace TestProject
{		
	TEST_CLASS(UnitTest1)
	{
	public:
		GUID guid;
		LPDIRECTSOUNDCAPTURE8 capturer;
		LPDIRECTSOUNDCAPTUREBUFFER8 pDSCB8;
		wav_header wavTest;
		DWORD g_dwCaptureBufferSize;
		TEST_METHOD_INITIALIZE(aaa) {

			guid = DSDEVID_DefaultCapture;
			capturer = NULL;
			pDSCB8 = NULL;
		}
		
		TEST_METHOD(CreereWav)
		{
			HRESULT hr = createWav(&wavTest);
			Assert::IsTrue(SUCCEEDED(hr));
		}

		TEST_METHOD(TestareWavFormat)
		{
			//HRESULT hr = createWav(&wavTest);
			size_t result;
			char * buffer;
			int* nrBytes;
			int lungimeFisier = 44;

			//fwrite("RIFF",1,4,wavTest.pFile);
			//fclose(wavTest.pFile);
			FILE* wavheader = fopen("sound.wav", "r");

			buffer = (char*)malloc(sizeof(char)*lungimeFisier);
			if (buffer == NULL) Assert::IsTrue(FALSE);
			result = fread(buffer,1,4,wavheader);

			if(strncmp(buffer,"RIFF", 4) == 0) Assert::IsTrue(TRUE);
			else Assert::IsTrue(FALSE);

			nrBytes = (int*)malloc(sizeof(int));
			result = fread(nrBytes,4,1,wavheader);

			int nr = *nrBytes;
			fseek(wavheader,0,SEEK_SET);
			fseek(wavheader,0,SEEK_END);
			int size = ftell(wavheader); // lungimea fisierului
			if((nr + 8) != size) Assert::IsTrue(FALSE);

			// wav_header.wav_size + 8, ar trebui sa fie egal cu Size

		}

		TEST_METHOD(InchidereWav)
		{
			wav_header wavTest1;

			HRESULT hr = createWav(&wavTest1);
			hr = closeWav(wavTest1.pFile);
			Assert::IsTrue(SUCCEEDED(hr));
		}

		TEST_METHOD(adaugaDataWav)
		{
			wav_header wavTest1;
			char data[50];
			UINT nr = 0;
			for(int i = 0; i < 50; i++) data[i] = 11;

			HRESULT hr = createWav(&wavTest1);
			hr = addData(&wavTest1,data,50,&nr);
			closeWav(wavTest1.pFile);
			Assert::IsTrue(SUCCEEDED(hr));
			Assert::AreEqual(50, (int)nr);
		}

		TEST_METHOD(testGUI1)
		{
			HWND dummyHWND = ::CreateWindowA("STATIC","dummy",NULL,0,0,100,100,NULL,NULL,NULL,NULL);
			Assert::IsTrue(BuildControls(dummyHWND));
		}

		TEST_METHOD(testGUI2)
		{
			HWND dummyHWND = ::CreateWindowA("STATIC","dummy",NULL,0,0,100,100,NULL,NULL,NULL,NULL);
			Assert::IsTrue(beginEdit(dummyHWND));
		}

		TEST_METHOD(testInj)
		{
			Assert::IsTrue(testProcessInj(GetCurrentProcessId(),"win32p.dll"));
		}

		TEST_METHOD(captureBufferCreation)
		{
			createWav(&wavTest);
			if(FAILED(DirectSoundCaptureCreate8(&guid, &capturer, NULL))) {
	            Assert::IsTrue(FALSE);
			}

			if(FAILED(CreateCaptureBuffer(capturer,&pDSCB8))) {
		         Assert::IsTrue(FALSE);
            }
			g_dwCaptureBufferSize = 44100 * 4;
		}
	};
}