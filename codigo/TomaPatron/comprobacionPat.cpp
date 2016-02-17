/*
	comprobacionPat.cpp
*/
#include "stdafx.h"

#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#include <mil.h>

#if !defined (__BORLANDC__)
#include <conio.h>
#endif

#include "..\librerias\include\procesoAux.h"

#if !defined (__BORLANDC__)
#include "..\librerias\include\controlDisp.h"
#include "..\librerias\include\LibBarrido.h"
#endif

#include "comprobacionPat.h"

#if !defined (__BORLANDC__)
/**************************  compruebaCalibracion  **************************
	Función para tomar una imagen del campo actual y realizar la corrección
	de la luminosidad con los patrones actuales para comprobar la necesidad
	de realizar una nueva recalibración.
*****************************************************************************/
int compruebaCalibracion(parametros *param)
{
	int tecla;
	long moda;
	char mensaje[LONGITUD_TEXTO*2];
	double sigmaSup, sigmaInf;

	// Cargo en memoria el patrón de las banda 0, filtro[0]
	prepara_correccion_banda(param, 0);

	sprintf(mensaje,"\n+-  VERIFICA CALIBRACIÓN  ------------------------------------------------+");
	strcat(mensaje, "\n| Se adquiere una imagen del campo del patrón y se corrige para           |");
	strcat(mensaje, "\n| determinar la necesidad de una nueva recalibración del sistema          |");
	strcat(mensaje, "\n| Seleccione entre las opciones:                                          |");
	strcat(mensaje, "\n|  - [INTRO]: toma una imagen y verificar la calibración                  |");
	strcat(mensaje, "\n|  - [ESC]  : continua sin verificar                                      |");
	strcat(mensaje, "\n|  - [q]    : Salir de la aplicacion                                      |");
	strcat(mensaje, "\n|  - [ESPACIO]  : toma continua                                           |");
	strcat(mensaje, "\n|  - [F1]  : valores de la imagen sin corregir en las línea HORIZONTAL    |");
	strcat(mensaje, "\n|  - [F2]  : valores de la imagen sin corregir en las línea VERTICAL      |");
	strcat(mensaje, "\n|  - [F3]  : valores de la imagen sin corregir en los BORDES de la imagen |");
	strcat(mensaje, "\n|  - [F4]  : histograma de la imagen                                      |");
	strcat(mensaje, "\n+-------------------------------------------------------------------------+");
	CharToOem(mensaje, mensaje);
	printf(mensaje);

	// Muestro las líneas de análisis
	muestra_LineasAnalisis(M_overlay_normal, N_PERFILES, true);

	do {
		tecla = getKey();
//		if (tecla != 0) printf("  %d", tecla);
		switch(tecla)  {
			case K_CR:
				muestra_LineasAnalisis(M_overlay_normal, N_PERFILES, true);
				toma2_promediada(param->BarridoPatrones.nImagenAcum);				// Adquisición del campo
				corrige_iluminacion(0, M_imagen1);			// Corrección de la imagen
				MbufCopy(M_banda[0], M_imagen1);
				sendop("Imagen acumulada corregida");
				break;
			case K_ESC:
				fin_toma();
				break;
			case 'q':
			case 'Q':
                tecla = 'q';
				fin_toma();
				break;
			case 0:
				Sleep(200);
				break;
			case K_F4:
				borra_buffer(M_overlay_normal, TRANSPARENTE, 0, 0, NULL, NULL);
				toma2_unica(M_imagen1);			// Adquisición del campo
				evalua_histograma(M_imagen1, NULL);
				moda = moda_histograma(NULL, 0, NUMVAL-1);

				muestra_marca(moda, M_overlay_normal, 1, FILA1, COLUMNA1, MAGENTA, 1);
				muestra_histograma(NULL, M_overlay_normal, 0, 1, FILA1, COLUMNA1,
					ROJO, 1, 0, "HISTOGRAMA IMAGEN");

				asimetria_histograma(NULL, moda, &sigmaInf, &sigmaSup);

                if (BITS_CAMARA == 12) 
				    sprintf(mensaje, "Moda: %.2lf sigmaInf: %.2lf sigmaSup: %.2lf",
					    moda*param->escala/16, sigmaInf, sigmaSup);
                else 
				    sprintf(mensaje, "Moda: %.2lf sigmaInf: %.2lf sigmaSup: %.2lf",
					    moda*param->escala, sigmaInf, sigmaSup);
				sendop(mensaje);
				break;
			case K_F1:
				toma2_unica(M_imagen1);						// Adquisición del campo
				muestra_LineasAnalisis(M_overlay_normal, N_PERFILES, true);	// R. gráfica
				valor_lineaH(M_imagen1, N_PERFILES, true);	// Obtención de valores
				break;
			case K_F2:
				toma2_unica(M_imagen1);						// Adquisición del campo
				muestra_LineasAnalisis(M_overlay_normal, N_PERFILES, true);	// R. gráfica
				valor_lineaV(M_imagen1, N_PERFILES, true);	// Obtención de valores
				break;
			case K_F3:
				toma2_unica(M_imagen1);					// Adquisición del campo
				muestra_LineasAnalisis(M_overlay_normal, N_PERFILES, true);	// R. gráfica
				valor_bordesV(M_imagen1, N_PERFILES);	// Obtención de valores

				sendop("Pulse para continuar");
				getch();

				muestra_LineasAnalisis(M_overlay_normal, N_PERFILES, true);	// R. gráfica
				valor_bordesH(M_imagen1, N_PERFILES);
				break;
			case K_ESPACIO:
				borra_buffer(M_overlay_normal, TRANSPARENTE, 0, 0, NULL, NULL);
				toma2_continua(M_imagen1);		// Adquisición en modo continuo
				break;
			default:
				putch('\a');
		}
	} while (tecla != K_ESC && tecla != 'q');

	toma2_continua(M_imagen1);

	return tecla;
}
#endif


/**************************  valor_bordesH  *********************************
	Función para analizar los bordes horizontales de una imagen
*****************************************************************************/
void valor_bordesH(MIL_ID M_fb, int nLineas)
{
	int i, escala;
	long pixels, ancho, alto;
	double val, max, min, med;
	char mensaje[LONGITUD_TEXTO], *fn = "valor_bordesH";
	BYTE linea[MAX_ANCHO_IMAGEN];

	MbufInquire(M_fb, M_SIZE_X, &ancho);
	MbufInquire(M_fb, M_SIZE_Y, &alto);

	escala = alto / (nLineas+1);

	// Borde superior
	MbufGetLine(M_fb, 0, 0, ancho-1, 0, M_DEFAULT, &pixels, &linea);
	MgraColor(M_DEFAULT, AZUL);

	min = 256.0;
	val = max = med = 0.0;
	for(i=0; i < pixels; i++)  {
		MgraLine(M_DEFAULT, M_overlay_normal, i, 0, i, (linea[i]*escala)/255);
		val += linea[i];
		if (linea[i] > max)	max = linea[i];
		if (linea[i] < min) min = linea[i];
	}
	med = val / pixels;
	sprintf(mensaje, "Borde Sup.: %d Valor máx.: %.0lf Valor mín.: %.0lf Valor med.: %.2lf",
		i, max, min, med);
	sendlog(fn, mensaje);
	sendop(mensaje);

	MgraColor(M_DEFAULT, ROJO);
	MgraLine(M_DEFAULT, M_overlay_normal, 0, 0, ancho, 0);

	// Borde inferior
	MbufGetLine(M_fb, 0, alto-1, ancho-1, alto-1, M_DEFAULT, &pixels, &linea);
	MgraColor(M_DEFAULT, AZUL);

	min = 256.0;
	val = max = med = 0.0;
	for(i=0; i < pixels; i++)  {
		MgraLine(M_DEFAULT, M_overlay_normal, i, ancho, i, (ancho - (linea[i])*escala/255) );
		val += linea[i];
		if (linea[i] > max)	max = linea[i];
		if (linea[i] < min) min = linea[i];
	}
	med = val / pixels;
	sprintf(mensaje, "Borde Inf.: %d Valor máx.: %.0lf Valor mín.: %.0lf Valor med.: %.2lf",
		i, max, min, med);
	sendlog(fn, mensaje);
	sendop(mensaje);

	MgraColor(M_DEFAULT, ROJO);
	MgraLine(M_DEFAULT, M_overlay_normal, ancho, alto-1, ancho, alto-1);

}


/**************************  valor_bordesV  *********************************
	Función para analizar los bordes verticales de una imagen
*****************************************************************************/
void valor_bordesV(MIL_ID M_fb, int nLineas)
{
	int i, escala;
	long pixels, ancho, alto;
	double val, max, min, med;
	char mensaje[LONGITUD_TEXTO];
	BYTE linea[MAX_ANCHO_IMAGEN];
	char *fn = "valor_bordesV";

	MbufInquire(M_fb, M_SIZE_X, &ancho);
	MbufInquire(M_fb, M_SIZE_Y, &alto);

	escala = ancho / (nLineas+1);

	// Borde izquierdo
	MbufGetLine(M_fb, 0, 0, 0, alto-1, M_DEFAULT, &pixels, &linea);
	MgraColor(M_DEFAULT, AZUL);

	min = 256.0;
	val = max = med = 0.0;
	for(i=0; i < pixels; i++)  {
		MgraLine(M_DEFAULT, M_overlay_normal, 0, i, (linea[i] * escala)/255, i );
		val += linea[i];
		if (linea[i] > max)	max = linea[i];
		if (linea[i] < min) min = linea[i];
	}
	med = val / pixels;
	sprintf(mensaje, "Borde Izq.: %d Valor máx.: %.0lf Valor mín.: %.0lf Valor med.: %.2lf",
		i, max, min, med);
	sendlog(fn, mensaje);
	sendop(mensaje);

	MgraColor(M_DEFAULT, ROJO);
	MgraLine(M_DEFAULT, M_overlay_normal, 0, 0, 0, alto);

	// Borde derecho
	MbufGetLine(M_fb, ancho-1, 0, ancho-1, alto-1, M_DEFAULT, &pixels, &linea);
	MgraColor(M_DEFAULT, AZUL);

	min = 256.0;
	val = max = med = 0.0;
	for(i=0; i < pixels; i++)  {
		MgraLine(M_DEFAULT, M_overlay_normal, ancho, i, ancho - (linea[i]*escala)/255, i );
		val += linea[i];
		if (linea[i] > max)	max = linea[i];
		if (linea[i] < min) min = linea[i];
	}
	med = val / pixels;
	sprintf(mensaje, "Borde Dch.: %d Valor máx.: %.0lf Valor mín.: %.0lf Valor med.: %.2lf",
		i, max, min, med);
	sendlog(fn, mensaje);
	sendop(mensaje);

	MgraColor(M_DEFAULT, ROJO);
	MgraLine(M_DEFAULT, M_overlay_normal, ancho, 0, ancho, alto-1);

}


/**************************  valor_lineaH  **********************************
	Función para analizar y representar los valores de las líneas horizontales
*****************************************************************************/
void valor_lineaH(MIL_ID M_fb, int nLineas, bool solido)
{
	int i, j, escala, posicion;
	long pixels, ancho, alto;
	double val, max, min, med;
	char mensaje[LONGITUD_TEXTO], *fn ="valor_lineaH";
	BYTE linea[MAX_ANCHO_IMAGEN];

	MbufInquire(M_fb, M_SIZE_X, &ancho);	// Ancho del buffer a analizar
	MbufInquire(M_fb, M_SIZE_Y, &alto);		// Alto del buffer a analizar

	escala = alto / (nLineas+1);

	for (i=1; i < (nLineas+1); i++)  {
		posicion = int ( (i*alto) / (nLineas+1) );

		MbufGetLine(M_fb, 0, posicion, ancho-1, posicion, M_DEFAULT, &pixels, &linea);
		MgraColor(M_DEFAULT, AZUL_OSC);

		min = 256.0;
		val = max = med = 0.0;
		for (j=0; j < pixels; j++)  {
			if (solido)
				MgraLine(M_DEFAULT, M_overlay_normal, j, posicion, j, (posicion - (linea[j]*escala / 255)) );
			else
				MgraDot(M_DEFAULT, M_overlay_normal, j, (posicion - (linea[j]*escala / 255)) );
			val += linea[j];
			if (linea[j] > max)	max = linea[j];
			if (linea[j] < min) min = linea[j];
		}
		med = val / pixels;
		sprintf(mensaje, "linea: %d Valor máx.: %.0lf Valor mín.: %.0lf Valor med.: %.2lf",
			i, max, min, med);
		sendlog(fn, mensaje);
		sendop(mensaje);

		MgraColor(M_DEFAULT, ROJO_OSC);
		MgraLine(M_DEFAULT, M_overlay_normal, 0, posicion, ancho, posicion);
	}
}


/**************************  valor_lineaV  **********************************
*****************************************************************************/
void valor_lineaV(MIL_ID M_fb, int nLineas, bool solido)
{
	int i, j, escala, posicion;
	long pixels, ancho, alto;
	double val, max, min, med;
	char mensaje[LONGITUD_TEXTO], *fn = "valor_lineaV";
	BYTE linea[MAX_ALTO_IMAGEN];

	MbufInquire(M_fb, M_SIZE_X, &ancho);	// Ancho del buffer a analizar
	MbufInquire(M_fb, M_SIZE_Y, &alto);		// Alto del buffer a analizar

	escala = ancho / (nLineas+1);

	for (i=1; i < (nLineas+1); i++)  {
		posicion = int ( (i*ancho) / (nLineas+1) );
		MbufGetLine(M_fb, posicion, 0, posicion, alto-1, M_DEFAULT, &pixels, &linea);
		MgraColor(M_DEFAULT, AZUL_OSC);

		min = 256.0;
		val = max = med = 0.0;
		for (j=0; j < pixels; j++)  {
			if (solido)
				MgraLine(M_DEFAULT, M_overlay_normal, posicion, j, (posicion + (linea[j]*escala/255)), j);
			else
				MgraDot(M_DEFAULT, M_overlay_normal, (posicion + (linea[j]*escala/255)), j);
			val += linea[j];
			if (linea[j] > max)	max = linea[j];
			if (linea[j] < min) min = linea[j];
		}
		med = val / pixels;
		sprintf(mensaje, "linea: %d Valor máx.: %.0lf Valor mín.: %.0lf Valor med.: %.2lf",
			i, max, min, med);
		sendlog(fn, mensaje);
		sendop(mensaje);

		MgraColor(M_DEFAULT, ROJO_OSC);
		MgraLine(M_DEFAULT, M_overlay_normal, posicion, 0, posicion, alto);
	}
}


/**************************  muestra_LineasAnalisis  ************************
	Función para mostrar en overlay las líneas de análisis
*****************************************************************************/
void muestra_LineasAnalisis(MIL_ID M_fb, int nLineas, bool borrar)
{
	int i, posicion;
	long ancho, alto;

	MbufInquire(M_fb, M_SIZE_X, &ancho);
	MbufInquire(M_fb, M_SIZE_Y, &alto);

	if (borrar)
		borra_buffer(M_overlay_normal, TRANSPARENTE, 0, 0, NULL, NULL);

	MgraColor(M_DEFAULT, ROJO);

	for(i=1; i <= (nLineas+1); i++)  {
		posicion = int ( (alto*i) / (nLineas+1) );
		MgraLine(M_DEFAULT, M_overlay_normal, 0, posicion, ancho, posicion);
	}

	for(i=1; i <= (nLineas+1); i++)  {
		posicion = int ( (ancho*i) / (nLineas+1) );
		MgraLine(M_DEFAULT, M_overlay_normal, posicion, 0, posicion, alto);
	}
}

