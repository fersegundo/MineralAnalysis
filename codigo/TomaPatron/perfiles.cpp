/*****************************************************************************
	analisis.cpp	Módulo en el que se definen las funciones que obtienen el
					valor de una imagen.
		
	Autores:	JCC, BLM
	Copyright:	© AITEMIN Marzo 2002
*****************************************************************************/
#include "stdafx.h"

#include <stdio.h>
#include <mil.h>
#include <conio.h>
#include <math.h>

#include "..\librerias\include\procesoAux.h"

#include "perfiles.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


double x[NUMPIXEL], y[NUMPIXEL];	// Posición de los píxeles a analizar.


/**************************  perfiles  *********************************
   Función para iniciar el proceso de análisis, obteniendo las imágenes
   de disco.
*****************************************************************************/
int perfiles(MIL_ID M_fb, parametros *param)
{
	char mensaje[128];
	int  tecla, posX, posY;
	MIL_ID M_ancho, M_alto;

	posX = param->Cam.anchoImagen / 2;	// Inicio en la posición central de la imagen
	posY = param->Cam.altoImagen / 2;
	
	M_ancho = MbufInquire(M_fb, M_SIZE_X, M_NULL);	// Pregunto sobre el ancho y alto
	M_alto = MbufInquire(M_fb, M_SIZE_Y, M_NULL);	// del buffer seleccionado.

	do {
		sprintf(mensaje, "   - X: %ld.   - Y: %ld.		", posX, posY);
		sendcont(mensaje);
		do {
			dibuja_linea_activa(M_overlay_normal, posX, posY, 0);
			Sleep(200);		// Espera mientras no se pulse una tecla
		} while (!kbhit());
		tecla = getKey();
		switch (tecla) {
			case K_ESC:
				break;
			case K_LEFT:
				/* borro la anterior linea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 1);

				posX = posX - MOV_LENTO;
				posX = (posX < 0)? 0 : posX;
				posY = posY;
				/* Dibujo la nueva línea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 0);
				break;
			case K_RIGHT:
				/* borro la anterior linea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 1);
				
				posX = posX + MOV_LENTO;
				posX = (posX > M_ancho)? M_ancho : posX;
				posY = posY;
				/* Dibujo la nueva línea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 0);
				break;
			case K_UP:
				/* borro la anterior linea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 1);

				posX = posX;
				posY = posY - MOV_LENTO;
				posY = (posY < 0)? 0 : posY;
				/* Dibujo la nueva línea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 0);
				break;
			case K_DOWN:
				/* borro la anterior linea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 1);

				posX = posX;
				posY = posY + MOV_LENTO;
				posY = (posY > M_alto)? M_alto : posY;
				/* Dibujo la nueva línea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 0);
				break;

			case K_CR_LEFT:
				/* borro la anterior linea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 1);

				posX = posX - MOVIMIENTO;
				posX = (posX < 0)? 0 : posX;	// Compruebo que no sobrepaso el límite.
				posY = posY;
				/* Dibujo la nueva línea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 0);
				break;
			case K_CR_RIGHT:
				/* borro la anterior linea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 1);
				
				posX = posX + MOVIMIENTO;
				posX = (posX > M_ancho)? M_ancho : posX;
				posY = posY;
				/* Dibujo la nueva línea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 0);
				break;
			case K_CR_UP:
				/* borro la anterior linea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 1);

				posX = posX;
				posY = posY - MOVIMIENTO;
				posY = (posY < 0)? 0 : posY;
				/* Dibujo la nueva línea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 0);
				break;
			case K_CR_DOWN:
				/* borro la anterior linea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 1);

				posX = posX;
				posY = posY + MOVIMIENTO;
				posY = (posY > M_alto)? M_alto : posY;
				/* Dibujo la nueva línea. */
				dibuja_linea_activa(M_overlay_normal, posX, posY, 0);
				break;
			case 'H':
			case 'h':
				if ( calcula_valores_linea(M_imagen1, posX, posY, L_HORIZ) )
					error_leve("perfiles", "No se puede mostrar el análisis solicitado");
				break;
			case 'V':
			case 'v':
				calcula_valores_linea(M_imagen1, posX, posY, L_VERT);
				break;
			default :
				putch('\a');
		}
	} while (tecla != K_ESC);
	MbufClear(M_overlay_normal, TRANSPARENTE);

	return 0;
}


/**************************  calcula_valores_linea  *************************
	Calcula los valores de nivel de gris de una determinada línea.
	(horizontal o vertical).
*****************************************************************************/
int calcula_valores_linea(MIL_ID M_fb, int posX, int posY, int linea)
{
	BYTE  val_pixel_h[MAX_ANCHO_IMAGEN], val_pixel_v[MAX_ALTO_IMAGEN];
	int altoImagen, anchoImagen;
	
	MbufCopy(M_fb, M_imagen6);
	
	/*	Compruebo que los valores a calcular no están fuera de los límites definidos por
		el buffer. */

	anchoImagen = MbufInquire(M_imagen6, M_SIZE_X, M_NULL);
	altoImagen = MbufInquire(M_imagen6, M_SIZE_Y, M_NULL);

	posX = (posX < 0)? 0 : posX;
	posX = (posX > anchoImagen)? anchoImagen : posX;
	posY = (posY < 0)? 0 : posY;
	posY = (posY > altoImagen)? altoImagen : posY;

	// Línea horizontal.
	if (linea == L_HORIZ)  {
		MbufGetLine(M_imagen6, 0, posY, anchoImagen-1, posY, M_DEFAULT, M_NULL, &val_pixel_h);
		dibuja_analisis_linea(val_pixel_h, linea, altoImagen, anchoImagen);
	}
	
	// Línea vertical.
	else if (linea == L_VERT)  {
		MbufGetLine(M_imagen6, posX, 0, posX, altoImagen-1, M_DEFAULT, M_NULL, &val_pixel_v);
		dibuja_analisis_linea(val_pixel_v, linea, altoImagen, anchoImagen);
	}
	else  {
		error_leve("calcula_valores_linea", "no se puede mostrar el análisis de la línea");
		return -1;
	}

	return 0;
}


/**************************  analiza_pixel  *********************************
	Función para determinar el valor  promedio del nivel de gris de un pixel
	y su desviación.
*****************************************************************************/
void analiza_pixel(MIL_ID M_fb, int n, double suma[], double sumaCuad[])
{
	int i, val[NUMPIXEL];
	double promedio[NUMPIXEL], sigma[NUMPIXEL];
	FILE *fich;

	for (i=0; i < NUMPIXEL; i++)  { 
		val[i] = 0;
	}

	for(i = 0; i < NUMPIXEL; i++)  {
		
		//	Obtengo los valores de nivel de gris de cada pixel.
		MbufGet2d(M_fb, x[i], y[i], 1, 1, &val[i]);
		
		suma[i] += val[i];				// Suma de los valores de cada pixel.
		sumaCuad[i] += val[i] * val[i];	// Suma de los valores de cada pixel al cuadrado.
		
		// Valor medio de nivel de gris de cada pixel analizado.
		promedio[i] = suma[i]/n;

		// Valor de desviación estandar de cada pixel analizado.
		sigma[i] = sqrt( (n * sumaCuad[i] - suma[i] * suma[i]) / (n * (n-1)) );
	}
	
	//	Muestro en pantalla los valores medios y la sigma de cada pixel analizado.
	for (i = 0; i < NUMPIXEL; i++)  {
		printf("Pixel[%d]=%6.2lf ", i, promedio[i] );
	}
	printf("\n");

	//	Creo el fichero de resultados analisis.txt
	fich = fopen("analisis.txt", "at");
	
	fprintf(fich, "n: %3d ", n);
	
	for (i = 0; i < NUMPIXEL; i++)  {
		fprintf(fich, "  val[%d]: %d media[%d]: %6.2lf desv[%d]: %6.2lf",
			i, val[i], i, promedio[i], i, sigma[i]);
	}
	fprintf(fich, "\n");	// Nueva línea.
	fclose(fich);
}


/**************************  dibuja_analisis_linea  ************************
   Función para mostrar los valores de nivel de gris obtenidos en el análisis
   de la línea.
*****************************************************************************/
void dibuja_analisis_linea(BYTE val_pixel[], int linea, int altoImagen, int anchoImagen)
{
	int i;
	
	if (linea == L_HORIZ)  {
		MgraColor(M_DEFAULT, AMARILLO_OSC);
		//	Represento el corte horizontal
		for (i = 0; i < anchoImagen; i++)
			MgraLine(M_DEFAULT, M_overlay_normal, i, altoImagen, i, altoImagen - val_pixel[i]);

		sendcont("Pulse una tecla para continuar.");	// Borro la línea anterior.
		getch();
	
		//	Borro el corte horizontal
		MgraColor(M_DEFAULT, TRANSPARENTE);
		for (i = 0; i < altoImagen; i++)
			MgraLine(M_DEFAULT, M_overlay_normal, i, altoImagen, i, altoImagen - val_pixel[i]);
	}
	if (linea == L_VERT)  {
		
		//	Represento el corte vertical
		MgraColor(M_DEFAULT, AMARILLO_OSC);
		for (i = 0; i < altoImagen; i++)
			MgraLine(M_DEFAULT, M_overlay_normal, 0, i, 0 + val_pixel[i], i);

		sendcont("Pulse una tecla para continuar.");		// Borro la línea anterior.
		getch();
		
		//	Borro el corte vertical
		MgraColor(M_DEFAULT, TRANSPARENTE);
		for (i = 0; i < altoImagen; i++)
			MgraLine(M_DEFAULT, M_overlay_normal, 0, i, 0 + val_pixel[i], i);
	}
}


/**************************  selecciona_pixel  ******************************
	Función para seleccionar el valor de los píxeles a analizar.
	
	NUMPIXEL: define el número de píxeles a analizar.
*****************************************************************************/
/*
void selecciona_pixel()
{
		
	x[0] = ANCHO_IMAGEN_N/2;				// Posición de los píxeles a analizar.
	x[1] = x[3] = ANCHO_IMAGEN_N/6;
	x[2] = x[4] = 5 * (ANCHO_IMAGEN_N/6);
	y[0] = ALTO_IMAGEN_N/2;
	y[1] = y[2] = ALTO_IMAGEN_N/6;
	y[3] = y[4] = 5 * (ALTO_IMAGEN_N/6);

	dibuja_pixeles(M_display_normal, x, y);
}
*/

