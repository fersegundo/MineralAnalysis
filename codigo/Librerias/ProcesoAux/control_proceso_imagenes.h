/*
	control_proceso_imagenes.h		Declaraciones para control_proceso_imagenes.cpp

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Diciembre 2001.
*/

#ifndef CONTROL_PROCESO_IMAGENES_H
#define CONTROL_PROCESO_IMAGENES_H

#include <mil.h>

#define SISTEMA_METEOR_II_1394	0
#define	SISTEMA_VGA				1

#define MAX_ANCHO_IMAGEN	1400
#define MAX_ALTO_IMAGEN		1040

/* Define el formato normal. */
#define FORMATO_NORMAL		   1
#define ANCHO_IMAGEN_N		 800
#define ALTO_IMAGEN_N		 600
#define FORMATO_DIG_NORMAL	"M_1024X768_Y"

/* Define el formato enfoque. */
#define FORMATO_ENFOQUE		   2
#define ANCHO_IMAGEN_E		 640
#define ALTO_IMAGEN_E		 478
#define FORMATO_DIG_ENFOQUE	"M_640X480_Y"	// Formato enfoque

// Definiciones de colores para overlay. 
#define  NUM_COL_OVR_PRIM     10
#define  NUM_COL_OVR_SEC      10
#ifndef TRANSPARENTE
    #define  TRANSPARENTE          8
#endif

#define ANCHO_IMAGEN_R		1280		// Tamaño del buffer de resultados
#define ALTO_IMAGEN_R		1280

extern MIL_ID	M_aplicacion,			// Aplicación MIL.
				M_sistema,				// Sistema.
				M_display_normal,		// Display normal.
				M_digitalizador,		// Digitalizador.
				M_imagen1,				// M_imagen: tamaño normal
				M_imagen2,
				M_imagen3,
				M_imagen4,
				M_imagen5,
				M_imagen6,
				M_enfoque,				// M_enfoque: tamaño enfoque
				M_centro_imagen,		// Buffer hijo de M_imagen1. (tamaño enfoque).
				M_resultados,			// M_resultados: buffer donde se almacenan los resultados.
				M_imagen_acum,
				M_banda[MAX_FILTROS_DTA],
				M_correc_denom[MAX_FILTROS_DTA],
				M_correc_numer[MAX_FILTROS_DTA],
				M_correc_aux,
				M_overlay_normal,
				M_lut_overlay,
				M_histograma,
				M_histo_aux;

//	funciones de inicializacion MIL
void  crea_LUT_gamma(MIL_ID M_lut);
void  crea_LUT_overlay(MIL_ID M_lut, int gamma);
int  ini_control_proceso_imagenes(parametros *param, int sistema, int gamma);
int  ini_control_proceso_imagenes_lite(parametros *param);
void configura_display_normal();

int  fin_control_proceso_imagenes(void);

int  configura_overlay(MIL_ID display, MIL_ID buffer, MIL_ID *overlay, int val);

#endif // CONTROL_PROCESO_IMAGENES_H

