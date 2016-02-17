/*****************************************************************************
	gestion_ficheros.h		Fichero para declarar las funciones auxiliares.
							gestion_ficheros.cpp

	Autores:	JCC, BLM
	Copyright:	� AITEMIN Enero 2002
*****************************************************************************/

#ifndef GESTION_FICHEROS_H
#define GESTION_FICHEROS_H

#include <mil.h>

/*  Definici�n de nombres y extensiones para archivos. */

//#define  EXT_MASC			".jpg"	// Extensi�n de las im�genes de m�scara
#define  EXT_MASC			".mim"	// Extensi�n de las im�genes de m�scara
#define  EXT_IMAG			".tif"	// Extensi�n de las im�genes
#define  EXT_HIS			".his"	// Extensi�n de los ficheros de histogramas
#define  EXT_LOG			".log"  // Extensi�n de los ficheros de logs.

#define  HISTO_ESTI			"est"	// Sufijos de los ficheros de datos
#define  HISTO_ACUM			"acu"	// para guardar los histogramas
#define  HISTO_ACUM_GRANOS	"gra"
#if !defined (__BORLANDC__)
#define  HISTO_GRANOS		""
#define  HISTO_COMP			"C_"
#endif


/*	Declaraci�n de funciones residentes en el m�dulo gestion_ficheros.cpp */

FILE  * abre_fichero_ASCII(char *nombre);		//  Apertura y creaci�n de ficheros ASCII
FILE  * crea_fichero_ASCII(char *nombre);

int  carga_imagen_campo(char *nombre, MIL_ID buffer);
int  carga_imagen_campo_bits(char *nombre, MIL_ID buffer, int nProfundidad);
int  graba_imagen_campo(char *nombre, MIL_ID buffer, int nBits = -1, double dEscala = -1);
int  graba_imagen_campo_8bit(parametros& param, char *nombre, MIL_ID buffer, double dEscala = -1);
int graba_imagen_campo_bits(parametros& param, char *nombre, MIL_ID buffer, int nBits, double dEscala = -1);

int  lee_fichero_histo(FILE *fich, long **histo, int *numval, long *maxval, long *total);
#if !defined (__BORLANDC__)
int  imprime_histo(FILE *canal, long *histo, int numval, long total, int diferencial);
#endif
int  graba_fichero_histo(FILE *fich, long *histo, int numval, long maxval, int diferencial);

#endif // GESTION_FICHEROS_H

