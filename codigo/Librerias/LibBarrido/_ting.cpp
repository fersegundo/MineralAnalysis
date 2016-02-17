/*****************************************************************************
	ting.cpp	Módulo con funciones para aplicar las fórmulas de Ting

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Abril 2002.
*****************************************************************************/
#include "stdafx.h"

#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#include <mil.h>

#if !defined (__BORLANDC__)
#include <conio.h>
#endif
#include <math.h>

#include "..\include\procesoAux.h"

#include "ting.h"

#if !defined (__BORLANDC__)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/*  Declaración de las variables auxiliares para proceso de Ting. */

BYTE  r1[MAX_NUMPIXELS], r2[MAX_NUMPIXELS], r3[MAX_NUMPIXELS];
BYTE  rmax[MAX_NUMPIXELS], rmin[MAX_NUMPIXELS];


/**************************  aplica_formulas_Ting  **************************
	Función para aplicar las fórmulas de Ting sobre las imágenes capturadas.
****************************************************************************/
void  aplica_formulas_Ting(int banda1, int banda2, int banda3)
{
	int		i, vmin, vmax;
	double  aux1, aux2, aux3, aux4;
	long	numPixels;

	/*	Volcamos las imágenes de partida en los arrays de trabajo. */

	MbufGet(M_banda[banda1], &r1);
	MbufGet(M_banda[banda2], &r2);
	MbufGet(M_banda[banda3], &r3);

	/*	Aplicamos las fórmulas de Ting. */
	numPixels = MAX_ANCHO_IMAGEN * MAX_ALTO_IMAGEN;
	for (i=numPixels-1; i >= 0; i--) {
		aux1 = ((int) r1[i] + (int) r3[i]) / 2.0;
		aux2 = ((int) r1[i] - (int) r2[i]);
		aux3 = ((int) r2[i] - (int) r3[i]);
		aux4 = sqrt((aux2 * aux2 + aux3 * aux3) / 2.0);
		vmax = (int) (aux1 + aux4 + 0.5);
		vmin = (int) (aux1 - aux4 + 0.5);
		rmax[i] = (BYTE)((vmax > 255)? 255 : vmax);
		rmin[i] = (BYTE)((vmin < 0)? 0 : vmin);
	}

	/*	Devolvemos los resultados como dos imágenes. */

	MbufPut(M_imagen1, &rmax);
	MbufPut(M_imagen2, &rmin);
}


/**************************  graba_imagenes_Ting  ***************************
	Función para grabar las imágenes que intervienen en el cálculo y los
	resultados de aplicar las fórmulas de Ting.
****************************************************************************/
int  graba_imagenes_Ting(barrido *serie)
{
	char  nombre[1024];

	/* Grabamos las imágenes obtenidas al aplicar las fórmulas de Ting. */

	sprintf(nombre, "%s%03d_rmax%s", serie->raiz_barrido, serie->validos, EXT_IMAG);
	if ( graba_imagen_campo( nombre, M_imagen1 ) )	// Imagen de reflectancias máximas
		return -1;

	sprintf(nombre, "%s%03d_rmin%s", serie->raiz_barrido, serie->validos, EXT_IMAG);
	if ( graba_imagen_campo( nombre, M_imagen2 ) )	// Imagen de reflectancias mínimas
		return -1;

	return 0;
}

