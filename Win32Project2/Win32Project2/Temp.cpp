#include "stdafx.h"
#include "Temp.h"

HRESULT creeazaFisier(fisierTemp* pFisierTemp) {
	HRESULT hr = S_OK;

	pFisierTemp->pFile = fopen("OptiuniEditare.tmp","wb+");

	if(pFisierTemp->pFile == NULL) {
		return E_FAIL;
	}

	strcpy(pFisierTemp->temp_header, "TEMP");

	for(int i = 0; i < 4; i++) fputc(pFisierTemp->temp_header[i], pFisierTemp->pFile);

	pFisierTemp->nr_efecte = 0;
	fwrite(&pFisierTemp->nr_efecte,sizeof(int),1,pFisierTemp->pFile);


	return hr;
}

HRESULT adaugaEfect(fisierTemp* pFisierTemp, char* numeEfect ,int frameInceput, int frameSfarsit) {
	pFisierTemp->nr_efecte += 1;

	for(int i = 0; i < 4; i++) 
		fputc(numeEfect[i], pFisierTemp->pFile);

	fwrite(&frameInceput,sizeof(int),1,pFisierTemp->pFile);
	fwrite(&frameSfarsit,sizeof(int),1,pFisierTemp->pFile);

	// Updataza campul cu numerele de efecte
	fseek (pFisierTemp->pFile, 4 , SEEK_SET );
	fwrite(&pFisierTemp->nr_efecte,sizeof(int),1,pFisierTemp->pFile);
	fseek (pFisierTemp->pFile, 0 , SEEK_END );

	return S_OK;
}

HRESULT closeTemp(FILE* pFile) {
	fclose(pFile);
	return S_OK;
	// sterge si fisierul dupa
}

efect*  ordoneazaEfecte(fisierTemp* pFisierTemp) {
	efect* efecte = (efect*) malloc(sizeof(efect) * pFisierTemp->nr_efecte);
	// aflam lungimea fisierului
	//fseek(pFisierTemp->pFile,0,SEEK_END);
	//int lungimeFisier = ftell(pFisierTemp->pFile);

	// citeste toate efectele din fisier
	fseek(pFisierTemp->pFile,8,SEEK_SET);
	for (int i = 0; i < pFisierTemp->nr_efecte;i++) {
		// citeste urmatorul efect
		fread(efecte[i].numeEfect,sizeof(char), 4, pFisierTemp->pFile); efecte[i].numeEfect[4] = '\0';
		fread(&efecte[i].frameInceput, sizeof(int), 1, pFisierTemp->pFile);
		fread(&efecte[i].frameSfarsit, sizeof(int), 1, pFisierTemp->pFile);
		//fseek(pFisierTemp->pFile,i + (sizeof(int) * 3),SEEK_SET); // sari peste 3 campuri
		
	}

	//if (efecte[0].frameInceput != 10) MessageBoxA(NULL, "Efecte 0 fI 10", "Error", MB_OK);
	//if (strcmp(efecte[0].numeEfect,"GRAY") == 0) MessageBoxA(NULL, "e gray", "Error", MB_OK);
	//if (efecte[0].frameSfarsit != 22) MessageBoxA(NULL, "Efecte 0 fs 22", "Error", MB_OK);
	//FILE* aaa = fopen("aaa.tmp","wb+");
	//for(int i = 0; i < 4; i++) 
		//fputc(efecte[0].numeEfect[i], aaa);
	//fflush(aaa);

	// order the effects
	
	for (int i = 0; i < pFisierTemp->nr_efecte; i++) {
		for (int j = 0; j < pFisierTemp->nr_efecte; j++) {
			if(efecte[j].frameInceput > efecte[i].frameInceput) {
				efect efectTemp = efecte[i];
				efecte[i] = efecte[j];
				efecte[j] = efectTemp;
			}
		}
	}

	/*
	fseek(pFisierTemp->pFile,0,SEEK_END);
	int lungime = pFisierTemp->nr_efecte;
	for (int i = 0; i < lungime; i++) {
		adaugaEfect(pFisierTemp, efecte[i].numeEfect, efecte[i].frameInceput, efecte[i].frameSfarsit);
	}
	*/
	//fflush(pFisierTemp->pFile);

	return efecte;
}
