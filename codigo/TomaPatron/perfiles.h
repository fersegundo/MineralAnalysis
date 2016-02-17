/*
	perfiles.h		Declaraci�n de las funciones de perfiles.cpp
		
	Autores:	JCC, BLM
	Copyright:	� AITEMIN Febrero 2002
*/

#ifndef PERFILES_H
#define PERFILES_H

#include <mil.h>

#define LINE_END_X	 ANCHO_IMAGEN_N-1
#define LINE_END_Y	  ALTO_IMAGEN_N-1

#define MOV_LENTO		1
#define MOVIMIENTO	   10

#define MAXIMAG		  256		// N�mero m�x. de im�genes acumulables en un buffer de 16 bits.


/*	Declaraci�n de funciones residentes en el m�dulo perfiles.cpp */

// Funciones necesarias para el an�lisis de perfiles de una imagen.
int perfiles(MIL_ID M_fb, parametros *param);
int  calcula_valores_linea(MIL_ID M_fb, int posX, int posY, int linea);

// Funciones para analizar p�xeles de una imagen.
void  selecciona_pixel(void);
void  analiza_pixel(MIL_ID M_fb, int n, double suma[], double sumaCuad[]);

void  dibuja_analisis_linea(BYTE val_pixel[], int linea, int altoImagen, int anchoImagen);

#endif // PERFILES_H
