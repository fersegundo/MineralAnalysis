/*
	calibracion.h	Declaraci�n de funciones de calibracion.cpp

	Autores:	JCC, BLM
	Copyright:	� AITEMIN Enero 2002
*/

#ifndef CALIBRACION_H
#define CALIBRACION_H

#define  EXT_PAT			".tif"	// Extensi�n de las im�genes de los patrones

#if !defined (__BORLANDC__)
/*  Par�metros de correcci�n de la falta de uniformidad de iluminaci�n. */

#define  NOM_PATRON_OS		"PAT_OS"	// Sufijo de la imagen del patr�n oscuro.
#define  NOM_PATRON_CL		"PAT_CL"	// Sufijo de la imagen del patr�n claro.
//#define  NOM_TABLA			"PAT.TAB"	// Sufijo del fichero de datos.

//extern double  escala;
#endif
// fer se declaran en calibracion.cpp y no se usan desde fuera extern double  ref_os;			// Reflectancias
//extern double  ref_cl;			// de los patrones oscuro y claro.

/*	Funciones declaradas en este m�dulo. */

int  prepara_correccion(parametros *param);
int  prepara_correccion_banda(parametros *param, int banda);
void  corrige_iluminacion(int banda, MIL_ID milBuffer);
int get_objetivo_corregido();

#endif // CALIBRACION_H

