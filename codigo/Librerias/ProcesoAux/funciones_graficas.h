/*
	funciones_graficas.h	Declaraciones de funciones contenidas en el módulo
							funciones_graficas.cpp

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Diciembre 2001.
*/

#ifndef FUNCIONES_GRAFICAS_H
#define FUNCIONES_GRAFICAS_H

#include <mil.h>

#define  TRANSPARENTE          8
    #define  COL_USUARIO         254
#if M_MIL_CURRENT_INT_VERSION == 0x0800
    #define  NEGRO               M_COLOR_BLACK       
    #define  ROJO                M_COLOR_RED         
    #define  VERDE               M_COLOR_GREEN       
    #define  AZUL                M_COLOR_BLUE        
    #define  AMARILLO            M_COLOR_YELLOW      
    #define  MAGENTA             M_COLOR_MAGENTA     
    #define  CYAN                M_COLOR_CYAN        
    #define  BLANCO              M_COLOR_WHITE       
    #define  GRIS                M_COLOR_GRAY        

    #define  GRIS_OSC            M_RGB888(64,64,64) 
    #define  ROJO_OSC            M_COLOR_DARK_RED    
    #define  VERDE_OSC           M_COLOR_DARK_GREEN  
    #define  AZUL_OSC            M_COLOR_DARK_BLUE   
    #define  AMARILLO_OSC        M_COLOR_DARK_YELLOW 
    #define  MAGENTA_OSC         M_COLOR_DARK_MAGENTA
    #define  CYAN_OSC            M_COLOR_DARK_CYAN   
    #define  BLANCO_OSC          M_COLOR_LIGHT_WHITE
    #define  GRIS_CLA            M_COLOR_BRIGHT_GRAY
#elif
    #define  NEGRO                 0
    #define  ROJO                  1
    #define  VERDE                 2
    #define  AZUL                  3
    #define  AMARILLO              4
    #define  MAGENTA               5
    #define  CYAN                  6
    #define  BLANCO                7
    #define  TRANSPARENTE          8
    #define  GRIS                  9

    #define  GRIS_OSC            246
    #define  ROJO_OSC            247
    #define  VERDE_OSC           248
    #define  AZUL_OSC            249
    #define  AMARILLO_OSC        250
    #define  MAGENTA_OSC         251
    #define  CYAN_OSC            252
    #define  BLANCO_OSC          253
    #define  COL_USUARIO         254
    #define  GRIS_CLA            255
#endif

// OTROS COLORES

// Definición de los colores empleados en la representación de histogramas. 

#define  COL_ACTI			ROJO_OSC
#define  COL_ACUM			VERDE_OSC
#define  COL_GLOB			CYAN_OSC
#define  COL_ORIG			CYAN_OSC
#define  COL_FILT			AMARILLO_OSC
#define  COL_RES1		    AZUL
#define  COL_RES2			ROJO
#define  COL_RES3			VERDE
#define  COL_PUNT			AMARILLO
#define  COL_CIRC			VERDE
#define  COL_CIMA			CYAN
#define  COLORSAT			GRIS			// Color empleado para zona saturada.
#define  COLORFONDETIQ		TRANSPARENTE	// Color empleado como fondo del texto.


/* Constantes utilizadas para marcar las coordenadas iniciales y finales
del rectángulo dibujado en: marca_zona_central. */
/*
#define X_INICIO_CENTRO		 (ANCHO_IMAGEN_N - ANCHO_IMAGEN_E) / 2
#define Y_INICIO_CENTRO		 (ALTO_IMAGEN_N - ALTO_IMAGEN_E) / 2
#define X_FINAL_CENTRO		 (ANCHO_IMAGEN_N - ANCHO_IMAGEN_E) / 2 + ANCHO_IMAGEN_E
#define Y_FINAL_CENTRO		 (ALTO_IMAGEN_N - ALTO_IMAGEN_E) / 2 + ALTO_IMAGEN_E
*/

#define LINE				1600		// Valor de la línea a representar en el análisis de línea.

#if !defined (__BORLANDC__)
#define L_HORIZ				   0		// Definiciones para el dibujo de la línea horizontal y
#define L_VERT				   1		// vertical en el estudio de los perfiles.

#define NUMPIXEL			   5		// Número de pixeles a analizar en una imagen en la función analiza_pixels.
#endif

//	Paso mínimo para la impresión de etiquetas en los histogramas
#define  PASOMINETIQ	15

/*	Declaración de funciones en este módulo. */

int	  marca_zona_central(parametros *param);
int	  dibuja_texto(MIL_ID M_fb, char *texto, int hpos, int vpos, int postexto);
void  dibuja_histo_reflec(long *histo, int numval, long maxval, int paso,
		double escala, MIL_ID M_fb, int ancho, int alto, int hpos, int vpos,
		int colorlin, int coloretiq, int solido);
void  dibuja_marca_histo(int valor, int numval, int paso, double escala, MIL_ID M_fb,
		int ancho, int alto, int hpos, int vpos, int colorlin, int coloretiq);
void borra_buffer(MIL_ID M_fb, int color,
				  int posXini, int posYini, int posXfin, int posYfin);

#if !defined (__BORLANDC__)
void muestra_mascara(MIL_ID M_mascara, MIL_ID M_aux, int color, int borrar);
#endif
void muestra_borde_mascara(MIL_ID M_destino, MIL_ID M_mascara, MIL_ID M_aux1,
					 MIL_ID M_aux2, int color, int radio, int borrar);
#if !defined (__BORLANDC__)
void muestra_pixel(int xx, int yy, int color, int borrar);
void muestra_punto(int xx, int yy, int grosor, int color, int borrar);
#endif
void  dibuja_linea_activa(MIL_ID M_fb, int posX, int posY, int borrar);
void dibuja_cruz(MIL_ID M_fb, int posX, int posY, int color, int nTamCruz, int borrar);
void dibuja_aspa(MIL_ID M_fb, int posX, int posY, int color, int nTamCruz, int borrar);
//int	 dibuja_pixeles(MIL_ID display, double *x, double *y);

#endif // FUNCIONES_GRAFICAS_H

