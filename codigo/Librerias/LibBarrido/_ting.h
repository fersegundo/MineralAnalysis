/*
	ting.h	Declaración de funciones de ting.cpp

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Abril 2002
*/

#ifndef TING_H
#define TING_H

#if !defined (__BORLANDC__)
/*	Definiciones de constantes. */

#define	 POSICION_POLARIZADOR_90	2
#define	 POSICION_POLARIZADOR_135	3
#define	 POSICION_POLARIZADOR_0		4
#define	 POSICION_POLARIZADOR_45	5
#endif

#define  MAX_NUMPIXELS		(MAX_ANCHO_IMAGEN * MAX_ALTO_IMAGEN)

/*	Funciones declaradas en este módulo. */

void  aplica_formulas_Ting(int banda1, int banda2, int banda3);
int  graba_imagenes_Ting(barrido *serie);

#endif // TING_H

