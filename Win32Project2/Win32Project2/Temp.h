#include <stdio.h>

#ifndef TEMP_H
#define TEMP_H

typedef struct fisierTemp {
	FILE * pFile;
	char temp_header[5]; // "EDIT" big endian
	int nr_efecte; // little endian - nr de efecte pe care le are fisierul
	/* Vor fi incarcate cu date pe parcurs */
	int contorEfectCurent; // va fi incrementat pe parcurs cand se adauga efectele
};

typedef struct efect {
	char numeEfect[4];
	int frameInceput;
	int frameSfarsit;
};

/* Chemata o singura data, la inceput
* Creeaza fisierul si scrie headeru
*/
HRESULT creeazaFisier(fisierTemp* pFisierTemp); 

/*
* Scrie numele si nr framelui la care incepe si se termina efectul apoi updateaza campul cu nr de efecte
* Inainte de a fi chemata, trebuie incrementat cu 1 nr_efecte
*/
HRESULT adaugaEfect(fisierTemp* pFisierTemp,char*,int, int); 

/* Chemata o singura data, inainte de a incepe adaugarea efectelor la videoclip 
* Ordoneaza efectele din fisier in functie de inceput, de la mic la mare
*/
efect* ordoneazaEfecte(fisierTemp* pFisierTemp);

/* Inchide si deleteaza fisierul */
HRESULT closeTemp(FILE* pFile);

#endif