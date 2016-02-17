/*
	parametros_adquisicion.h

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Febrero 2002
*/

#ifndef PARAMETROS_ADQUISICION_H
#define PARAMETROS_ADQUISICION_H

#define FICH_PARAM_APLICACION		                "parametros_aplicacion"
#define FICH_PARAM_MICROSCOPIO		                "parametros_microscopio"
#define FICH_PARAM_REVOLVER			                "parametros_revolver"
#define FICH_PARAM_CAMARA			                "parametros_camara"
#define  EXT_INI			                        ".ini"	// Extensión de los ficheros iniciales ( *.ini)

/*  Parámetros de corrección de la falta de uniformidad de iluminación. */

#if !defined (__BORLANDC__)
#define PAT_OSCURO				0
#define PAT_CLARO				1
#endif

#define  NOM_PATRON_OS		"PAT_OS"	// Sufijo de la imagen del patrón oscuro.
#define  NOM_PATRON_CL		"PAT_CL"	// Sufijo de la imagen del patrón claro.

/*  Parámetros relativos a la escala de reflectividades. */

#define	 ESCALA_BASE			0.03	// Escala de reflectividad por defecto.

//Indices de los parametros para el control de cambio de parametros
#define EXPOSICION		0
#define LAMPARA			1
#define BRILLO			2
#define GANANCIA		3
#define FILTRO			66

/*  Declaración de funciones incluidas en parametros_adquisicion.cpp */

/*	Lectura de ficheros de configuracion */
int lee_paramMicroscopio(char *nom_fich, parametros* param);
int lee_paramRevolver(char *nom_fich, parametros* param);
int lee_paramCamara(char *nom_fich, parametros *paramAdq);

/*	Creación del ficheros de configuracion */
int crea_paramRevolver(char *raizIni, parametros* param);
int crea_paramMicroscopio(char *raizIni, parametros* param);
int crea_paramCamara(char *raizIni, parametros* param);
/*
int crea_paramAplicacion(char *raizIni, parametros *param);
*/
#endif // PARAMETROS_ADQUISICION_H

