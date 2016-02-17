/*
	calibracion.h	Declaración de funciones de calibracion.cpp

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Enero 2002
*/

#ifndef CALIBRACION_H
#define CALIBRACION_H

#define  EXT_PAT			".tif"	// Extensión de las imágenes de los patrones

#if !defined (__BORLANDC__)
/*  Parámetros de corrección de la falta de uniformidad de iluminación. */

#define  NOM_PATRON_OS		"PAT_OS"	// Sufijo de la imagen del patrón oscuro.
#define  NOM_PATRON_CL		"PAT_CL"	// Sufijo de la imagen del patrón claro.
//#define  NOM_TABLA			"PAT.TAB"	// Sufijo del fichero de datos.

//extern double  escala;
#endif
// fer se declaran en calibracion.cpp y no se usan desde fuera extern double  ref_os;			// Reflectancias
//extern double  ref_cl;			// de los patrones oscuro y claro.

/*	Funciones declaradas en este módulo. */

int  prepara_correccion(parametros *param);
int  prepara_correccion_banda(parametros *param, int banda);
void  corrige_iluminacion(int banda, MIL_ID milBuffer);
int get_objetivo_corregido();

#endif // CALIBRACION_H

