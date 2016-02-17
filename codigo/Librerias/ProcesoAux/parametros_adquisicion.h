/*
	parametros_adquisicion.h

	Autores:	JCC, BLM
	Copyright:	� AITEMIN Febrero 2002
*/

#ifndef PARAMETROS_ADQUISICION_H
#define PARAMETROS_ADQUISICION_H

#define FICH_PARAM_APLICACION		                "parametros_aplicacion"
#define FICH_PARAM_MICROSCOPIO		                "parametros_microscopio"
#define FICH_PARAM_REVOLVER			                "parametros_revolver"
#define FICH_PARAM_CAMARA			                "parametros_camara"
#define  EXT_INI			                        ".ini"	// Extensi�n de los ficheros iniciales ( *.ini)

/*  Par�metros de correcci�n de la falta de uniformidad de iluminaci�n. */

#if !defined (__BORLANDC__)
#define PAT_OSCURO				0
#define PAT_CLARO				1
#endif

#define  NOM_PATRON_OS		"PAT_OS"	// Sufijo de la imagen del patr�n oscuro.
#define  NOM_PATRON_CL		"PAT_CL"	// Sufijo de la imagen del patr�n claro.

/*  Par�metros relativos a la escala de reflectividades. */

#define	 ESCALA_BASE			0.03	// Escala de reflectividad por defecto.

//Indices de los parametros para el control de cambio de parametros
#define EXPOSICION		0
#define LAMPARA			1
#define BRILLO			2
#define GANANCIA		3
#define FILTRO			66

/*  Declaraci�n de funciones incluidas en parametros_adquisicion.cpp */

/*	Lectura de ficheros de configuracion */
int lee_paramMicroscopio(char *nom_fich, parametros* param);
int lee_paramRevolver(char *nom_fich, parametros* param);
int lee_paramCamara(char *nom_fich, parametros *paramAdq);

/*	Creaci�n del ficheros de configuracion */
int crea_paramRevolver(char *raizIni, parametros* param);
int crea_paramMicroscopio(char *raizIni, parametros* param);
int crea_paramCamara(char *raizIni, parametros* param);
/*
int crea_paramAplicacion(char *raizIni, parametros *param);
*/
#endif // PARAMETROS_ADQUISICION_H

