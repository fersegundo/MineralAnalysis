/*****************************************************************************
	funciones_graficas.cpp:	Funciones genéricas para el dibujo.

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Enero 2002.
*****************************************************************************/
#include "stdafx.h"

#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#if !defined (__BORLANDC__)
#include <conio.h>
#endif
#include <math.h>

#include "estructuras.h"

#include "funciones_graficas.h"

#include "control_proceso_imagenes.h"

#include "gestion_mensajes.h"

#if !defined (__BORLANDC__)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

/**************************  marca_zona_central  ****************************
   Función para marcar en overlay la zona central de la imagen.
****************************************************************************/
int marca_zona_central (parametros *param)
{
	long  x1, x2, y1, y2; //1 sup-izq; 2 sup-dcha; 3 inf-izq; 4 inf-dcha.

	x1 = (param->Cam.anchoImagen - param->Cam.anchoImagen_e) / 2;
	x2 = x1 + param->Cam.anchoImagen_e - 1;
	y1 = (param->Cam.altoImagen - param->Cam.altoImagen_e) / 2;
	y2 =  y1 + param->Cam.altoImagen_e - 1;

	/*	Selecciono el color con el que mostraré la marca central. */

	MgraColor(M_DEFAULT, AMARILLO);

	/*	Dibujo un rectangulo de la zona centro. */

	MgraRect(M_DEFAULT, M_overlay_normal, x1, y1, x2, y2);

	/*	Dibujo una cruz en el centro de la imagen de formato normal. */

	MgraLine(M_DEFAULT, M_overlay_normal, param->Cam.anchoImagen / 2,
		(param->Cam.altoImagen / 2) + 30, param->Cam.anchoImagen / 2,
		(param->Cam.altoImagen / 2) + 3);
	MgraLine(M_DEFAULT, M_overlay_normal, param->Cam.anchoImagen / 2,
		(param->Cam.altoImagen / 2) - 30, param->Cam.anchoImagen / 2,
		(param->Cam.altoImagen / 2) - 3);
	MgraLine(M_DEFAULT, M_overlay_normal, (param->Cam.anchoImagen / 2) + 30,
		param->Cam.altoImagen / 2, (param->Cam.anchoImagen / 2) +3,
		param->Cam.altoImagen / 2);
	MgraLine(M_DEFAULT, M_overlay_normal, (param->Cam.anchoImagen / 2) - 30,
		param->Cam.altoImagen / 2, (param->Cam.anchoImagen / 2) -3,
		param->Cam.altoImagen / 2);

	/*	Vuelvo a seleccionar el color por defecto como TRANSPARENTE. */
	MgraColor(M_DEFAULT, TRANSPARENTE);

	return 0;
}


/**************************  dibuja_texto  ***********************************
   Función para dibujar un texto en una posición dada de un frame buffer.
   El texto se puede situar en nueve posiciones distintas respecto a la
   posición indicada, numeradas del 1 al 9 según el siguiente diagrama:

               2 -------------- 5 ---------------- 8
               1   AQUI SE ES-  4  CRIBE EL TEXTO  7
               0 -------------- 3 ---------------- 6

*****************************************************************************/
int dibuja_texto(MIL_ID M_fb, char *texto, int hpos, int vpos, int postexto)
{
	int	 hfont, vfont, numcar;
	long  font;
	double  escalaX, escalaY, anchoTexto, altoTexto;

	/*	Determinamos la longitud del texto. */

	numcar = strlen(texto);
	if (numcar == 0)
		return 0;

	/*	Determinamos las dimensiones básicas del font seleccionado. */

//	MgraFont(M_DEFAULT, M_FONT_DEFAULT_LARGE);
	MgraInquire(M_DEFAULT, M_FONT, &font);

	if ( font == (long)M_FONT_DEFAULT_SMALL )  {
		hfont = 8;	vfont = 16;
	} else if ( font == (long)M_FONT_DEFAULT_MEDIUM )  {
		hfont = 12;	vfont = 24;
	} else if ( font == (long)M_FONT_DEFAULT_LARGE )  {
		hfont = 16;	vfont = 32;
	} else  {
#if !defined (__BORLANDC__)
		error_leve("dibuja_texto", "valor de font inaceptable");
#endif
		return 1;
	}

	/*	Determinamos las dimensiones del texto en función de los factores
		de escala de font seleccionados. */

	MgraInquire(M_DEFAULT, M_FONT_X_SCALE, &escalaX);
	anchoTexto = hfont * numcar * escalaX;
	MgraInquire(M_DEFAULT, M_FONT_Y_SCALE, &escalaY);
	altoTexto = vfont * escalaY;

	/*	Determinamos los reajustes de la posición de dibujo en función
		del tamaño efectivo del font y de la longitud del texto. */

	if ( postexto < 0 || postexto > 8 ) {
#if !defined (__BORLANDC__)
		error_leve("dibuja_texto", "posición relativa del texto inaceptable");
#endif
		return  -1;
	}
	hpos -= (int)(((postexto / 3) * anchoTexto) / 2);
	vpos -= (int)(((2 - (postexto % 3)) * altoTexto) / 2);

	/* Dibujamos en la posición reajustada el mensaje seleccionado. */

	MgraText(M_DEFAULT, M_fb, hpos, vpos, texto);

	return 0;
}


/**************************  dibuja_histo_reflec  ****************************
	Función para dibujar un histograma de reflectancias en el frame buffer M_fb
*****************************************************************************/
void  dibuja_histo_reflec(long *histo, int numval, long maxval, int paso,
		double escala, MIL_ID M_fb, int ancho, int alto, int hpos, int vpos,
		int colorlin, int coloretiq, int solido)
{
	char  numero[32];
	int  i, j, k, l, m, nelem, paso_text, hfont, vfont, u1, v1, u2, v2;
	long  suma;
    long  histo_aux[NUMVAL]; //histograma condensado para que quepa en el display (puede no estar completo)
    memset(histo_aux,0,NUMVAL*sizeof(long)); //inicializacion
	double  factor;


	/*  Seleccionamos el tamaño de font que se va a emplear.
		Por defecto tomamos el tamaño pequeño definido por defecto,
		salvo que el histograma sea muy grande. */

	if ( ancho <= 640 ) {
		MgraFont(M_DEFAULT, M_FONT_DEFAULT_SMALL);
		hfont = 8;
		vfont = 16;
	} else {
		MgraFont(M_DEFAULT, M_FONT_DEFAULT_MEDIUM);
		hfont = 12;
		vfont = 24;
	}

	/*  Reajustamos la zona destinada a presentar el histograma. */

	hpos += 2 * hfont;
	vpos += hfont;
	ancho -= 4 * hfont;
	alto -= hfont + 6 + vfont;

	/*  Preprocesamos el histograma de partida para condensar los datos. */

	nelem = (numval + paso - 1) / paso;		// Nuevo número de elementos.
	paso *= (nelem + ancho - 1) / ancho;	// Comprobamos que son representables.
	nelem = (numval + paso - 1) / paso;		// Confirmamos el número de elementos.
	for (i=k=0; k < nelem; i+=paso, k++) {
		for (suma=0, j=0; j < paso && i+j < numval; j++)
			suma += histo[i+j];				// Acumulamos "paso" elementos en cada
		histo_aux[k] = suma;				// elemento del histograma auxiliar.
	}

	/*  Se determina la escala vertical de dibujo, si no se ha indicado. */

	if ( maxval == 0 ) {
		for (i=0; i < nelem; i++)
			if ( histo_aux[i] > maxval )
				maxval = histo_aux[i];		// Determinamos el máximo valor.
		if ( maxval == 0 )
			maxval = 1;						// Para evitar la división por 0.
	}
	factor = alto / (double) maxval;		// Factor de escala vertical.

	/*  Dibujamos el histograma sólido o su contorno. */

	MgraColor(M_DEFAULT, colorlin);			// Fijamos color de líneas.

	/* Bucle para realizar el histograma en modo sólido. */
	if ( solido )
		for (i=j=0; i < nelem; j=k, i++) {
			k = (int) ((ancho * (long) (i+1)) / nelem);
			l = (int) floor(factor * histo_aux[i] + 0.5);
			if ( l )
				if ( l > alto ) {
					MgraColor(M_DEFAULT, COLORSAT);	// Color para indicar saturación.
					MgraRectFill(M_DEFAULT, M_fb, hpos+j, vpos+alto-1, hpos+k-1, vpos);
					MgraColor(M_DEFAULT, colorlin);	// Retornamos al color de linea.
				}
			else
				MgraRectFill(M_DEFAULT, M_fb, hpos+j, vpos+alto-1,
					hpos+k-1, vpos+alto-l);
		}

	/* Bucle para realizar el histograma en modo linea. */
	else {
		k = ancho / nelem;
		l = (int) floor(factor * histo_aux[0] + 0.5);

		u1 = hpos + (k-1)/2;
		v1 = (l > alto)? vpos : vpos+alto-((l)? l : 1);
		for (i=1; i < nelem; i++) {
			j = k;
			m = l;
			k = (int) ((ancho * (long) (i+1)) / nelem);
			l = (int) floor(factor * histo_aux[i] + 0.5);
			u2 = hpos + (j+k-1)/2;
			if ( l > alto )
				v2 = vpos;
			else
				v2 = vpos+alto-l;
			if ( l > alto && m > alto )
				MgraColor(M_DEFAULT, COLORSAT);	// Color para indicar saturación.
			MgraLine(M_DEFAULT, M_fb, u1, v1, u2, v2);
			if ( l > alto && m > alto )
				MgraColor(M_DEFAULT, colorlin);	// Retornamos al color de líneas.
			u1 = u2;
			v1 = v2;
		}
	}

	/*  Determinamos el espaciado entre etiquetas a partir de la máxima
		longitud de etiqueta. */

	l = sprintf(numero, "%.1lf", numval * paso * escala);
	paso_text = PASOMINETIQ;			// Paso mínimo entre etiquetas
	while ( (int) ((l * hfont * (long) nelem) / paso_text) > (8*ancho)/10 )
		paso_text *= 2;					// Duplicamos el paso entre etiquetas.

	/*  Dibujamos los números correspondientes a las celdas del histograma. */

	MgraColor(M_DEFAULT, coloretiq);	// Selecciono el color con el que muestro el texto

	for (i=0; i < nelem; i+=paso_text) {
		sprintf(numero, "%.1lf", i * paso * escala);
		k = (int) ((ancho * (long) i) / nelem);
		MgraLine(M_DEFAULT, M_fb, hpos+k, vpos+alto+2, hpos+k, vpos+alto+6);
		dibuja_texto(M_fb, numero, hpos+k, vpos+alto+6, 5);
//		MgraText(M_DEFAULT, M_fb, hpos+k,vpos+alto+6, numero);
	}
}


/**************************  dibuja_marca_histo  *****************************
   Función para dibujar una marca sobre un histograma de reflectancias.
*****************************************************************************/
void  dibuja_marca_histo(int valor, int numval, int paso, double escala,
	MIL_ID M_fb, int ancho, int alto, int hpos, int vpos, int colorlin,
	int coloretiq)
{
	char  numero[32];
	int  j, k, nelem, hfont, vfont;

	/*  Seleccionamos el tamaño de font que se va a emplear.
		Por defecto tomamos el tamaño pequeño definido por defecto,
		salvo que el histograma sea muy grande. */

	if ( ancho <= 640 ) {
		MgraFont(M_DEFAULT, M_FONT_DEFAULT_SMALL);
		hfont = 8;
		vfont = 16;
	} else {
		MgraFont(M_DEFAULT, M_FONT_DEFAULT_MEDIUM);
		hfont = 12;
		vfont = 24;
	}

   /*  Reajustamos la zona destinada a presentar el histograma. */

   hpos += 2 * hfont;
   vpos += hfont;
   ancho -= 4 * hfont;
   alto -= hfont + 6 + vfont;

   /*  Se recalculan el paso y el numero de elementos. */

   nelem = (numval + paso - 1) / paso;	// Nuevo número de elementos.
   paso *= (nelem + ancho - 1) / ancho;	// Comprobamos que son representables.
   nelem = (numval + paso - 1) / paso;	// Confirmamos el número de elementos.

   /*  Dibujamos la marca sobre el histograma. */

   j = (int) ((ancho * (long) valor) / (nelem * paso));
   k = (int) ((ancho * (long) (valor+1)) / (nelem * paso));
   MgraColor(M_DEFAULT, colorlin);		// Fijamos color de líneas.
   MgraRectFill(M_DEFAULT, M_fb, hpos+j, vpos+alto-1, hpos+k-1, vpos);

   /*  Dibujamos el número correspondiente a la marca del histograma. */

   MgraColor(M_DEFAULT, coloretiq);				// Fijamos color de etiqueta
   sprintf(numero, "%.2lf", valor * escala);	// Texto de la etiqueta.

   MgraFontScale(M_DEFAULT, 0.6, 0.8);			// Reduzco la escala de la letra.
   dibuja_texto(M_fb, numero, hpos+(j+k-1)/2, vpos, 0);
   MgraFontScale(M_DEFAULT, 1.0, 1.0);
}


/**************************  borra_buffer  *********************************
	Función para borrar una parte del overlay.
****************************************************************************/
void borra_buffer(MIL_ID M_fb, int color,
				  int posXini, int posYini, int posXfin, int posYfin)
{
	long ancho, alto;

	ancho = MbufInquire(M_fb, M_SIZE_X, M_NULL);
	alto = MbufInquire(M_fb, M_SIZE_Y, M_NULL);

	if (posXfin == NULL)		posXfin = ancho;
	if (posYfin == NULL)		posYfin = alto;

	MgraColor(M_DEFAULT, color);
	if ((posXfin-posXini) == ancho  && (posYfin-posYini) == alto )  {
		MbufClear(M_fb, color);
	}
	else  {
		MgraRectFill(M_DEFAULT, M_fb, posXini, posYini, posXfin, posYfin);
	}
}


#if !defined (__BORLANDC__)
/****************************  muestra_mascara  *****************************
	Función para mostrar en el overlay normal la máscara resultante de un
	proceso de segmentación.
	La máscara debe ser binaria: 0 ó 255.
*****************************************************************************/
void muestra_mascara(MIL_ID M_mascara, MIL_ID M_aux, int color, int borrar)
{
	if (borrar)
		borra_buffer(M_overlay_normal, TRANSPARENTE, 0, 0, NULL, NULL);

	MimClip(M_mascara, M_aux, M_EQUAL, 0, M_NULL, TRANSPARENTE, M_NULL);
	MimClip(M_aux, M_overlay_normal, M_EQUAL, 255, M_NULL, color, M_NULL);
}
#endif


/*************************  muestra_borde_mascara  **************************
	Función para mostrar en el overlay normal la máscara resultante de un
	proceso de segmentación.
	La máscara debe ser binaria: 0 ó 255.
*****************************************************************************/
void muestra_borde_mascara(MIL_ID M_destino, MIL_ID M_mascara, MIL_ID M_aux1,
					 MIL_ID M_aux2, int color, int radio, int borrar)
{
	if (borrar)
		borra_buffer(M_destino, TRANSPARENTE, 0, 0, NULL, NULL);

	//	Obtengo el borde exterior de la máscara
	MimBinarize(M_mascara, M_aux1, M_NOT_EQUAL, 0, M_NULL);

	MimDilate(M_aux1, M_aux2, radio, M_BINARY);
	MimArith(M_aux2, M_aux1, M_aux1, M_SUB);

	//	Transformo la imagen para mostrarla en el overlay
	MimClip(M_aux1, M_aux2, M_EQUAL, 0, M_NULL, TRANSPARENTE, M_NULL);
	MimClip(M_aux2, M_destino, M_EQUAL, 255, M_NULL, color, M_NULL);
}


#if !defined (__BORLANDC__)
/*****************************  muestra_pixel  ******************************
*****************************************************************************/
void muestra_pixel(int xx, int yy, int color, int borrar)
{
	MgraColor(M_DEFAULT, color);

	if (borrar || color == NULL)
		MgraColor(M_DEFAULT, TRANSPARENTE);
	else
		MgraColor(M_DEFAULT, color);

	MgraLine(M_DEFAULT, M_overlay_normal, xx - 25, yy, xx - 1, yy);
	MgraLine(M_DEFAULT, M_overlay_normal, xx + 25, yy, xx + 1, yy);
	MgraLine(M_DEFAULT, M_overlay_normal, xx, yy - 25, xx, yy - 1);
	MgraLine(M_DEFAULT, M_overlay_normal, xx, yy + 25, xx, yy + 1);
}

//***************************** muestra_punto ******************************/
void muestra_punto(int xx, int yy, int grosor, int color, int borrar)
{
	MgraColor(M_DEFAULT, color);

	if (borrar || color == NULL)
		MgraColor(M_DEFAULT, TRANSPARENTE);
	else
		MgraColor(M_DEFAULT, color);

    //Restar el ofset adecuado para centrar la cuadricula
    xx -= grosor/2;
    yy -= grosor/2;
    for (int i=0;i<grosor;i++)
    {
        for (int j=0;j<grosor;j++)
        {
	        MgraDot(M_DEFAULT, M_overlay_normal, xx + i  , yy + j);
        }
    }
}
#endif


/**************************  dibuja_linea_activa  ***************************
	Función para representar una línea en overlay.
	Argumentos que le paso: posición centro a representar.
*****************************************************************************/
void dibuja_linea_activa(MIL_ID M_fb, int posX, int posY, int borrar)
{
	//	Borro la línea anterior
	if (borrar)  {
		MgraColor(M_DEFAULT, TRANSPARENTE);
		// Línea horizontal.
		MgraLine(M_DEFAULT, M_fb, posX-LINE, posY, posX+LINE, posY);
		// Línea vertical.
		MgraLine(M_DEFAULT, M_fb, posX, posY-LINE, posX, posY+LINE);
		return;
	}

	/* Muestro las líneas donde realizo el corte. */

	MgraColor(M_DEFAULT, ROJO);

	// Línea horizontal.
	MgraLine(M_DEFAULT, M_fb, posX-LINE, posY, posX+LINE, posY);
	// Línea vertical.
	MgraLine(M_DEFAULT, M_fb, posX, posY-LINE, posX, posY+LINE);

	MgraColor(M_DEFAULT, TRANSPARENTE);
}

//	Función para representar una cruz en overlay.
//	Argumentos que le paso: posición centro a representar, color y tamaño
//  borrar!=0 para eliminar una cruz ya existente en este punto
void dibuja_cruz(MIL_ID M_fb, int posX, int posY, int color, int nTamCruz, int borrar)
{
	if (borrar)  
    	// Borro una cruz anterior
		MgraColor(M_DEFAULT, TRANSPARENTE);
    else
	    // Dibujo una cruz 
	    MgraColor(M_DEFAULT, color);
	// Línea horizontal.
	MgraLine(M_DEFAULT, M_fb, posX-nTamCruz, posY, posX+nTamCruz, posY);
	MgraLine(M_DEFAULT, M_fb, posX-nTamCruz, posY+1, posX+nTamCruz, posY+1);
	MgraLine(M_DEFAULT, M_fb, posX-nTamCruz, posY-1, posX+nTamCruz, posY-1);
	// Línea vertical.
	MgraLine(M_DEFAULT, M_fb, posX, posY-nTamCruz, posX, posY+nTamCruz);
	MgraLine(M_DEFAULT, M_fb, posX+1, posY-nTamCruz, posX+1, posY+nTamCruz);
	MgraLine(M_DEFAULT, M_fb, posX-1, posY-nTamCruz, posX-1, posY+nTamCruz);
	MgraColor(M_DEFAULT, TRANSPARENTE);
}

//	Función para representar un aspa en overlay.
//	Argumentos que le paso: posición centro a representar, color y tamaño
//  borrar!=0 para eliminar una cruz ya existente en este punto
void dibuja_aspa(MIL_ID M_fb, int posX, int posY, int color, int nTamCruz, int borrar)
{
	if (borrar)  
    	// Borro un aspa anterior
		MgraColor(M_DEFAULT, TRANSPARENTE);
    else
	    // Dibujo un aspa 
	    MgraColor(M_DEFAULT, color);
	// Línea diagonal.
	MgraLine(M_DEFAULT, M_fb, posX-nTamCruz, posY-nTamCruz, posX+nTamCruz, posY+nTamCruz);
	MgraLine(M_DEFAULT, M_fb, posX-nTamCruz, posY-nTamCruz+1, posX+nTamCruz, posY+nTamCruz+1);
	MgraLine(M_DEFAULT, M_fb, posX-nTamCruz, posY-nTamCruz-1, posX+nTamCruz, posY+nTamCruz-1);
	// Otra línea diagonal.
	MgraLine(M_DEFAULT, M_fb, posX-nTamCruz, posY+nTamCruz, posX+nTamCruz, posY-nTamCruz);
	MgraLine(M_DEFAULT, M_fb, posX-nTamCruz+1, posY+nTamCruz, posX+nTamCruz+1, posY-nTamCruz);
	MgraLine(M_DEFAULT, M_fb, posX-nTamCruz-1, posY+nTamCruz, posX+nTamCruz-1, posY-nTamCruz);
	MgraColor(M_DEFAULT, TRANSPARENTE);
}

/**************************  dibuja_pixeles  ****************************
   Función para marcar los pixeles a analizar.
****************************************************************************/
/*
int dibuja_pixeles(MIL_ID display, double *x, double *y)
{
	int i;
	char nombre[256];
	long x1, y1, x2, y2;

	x1 = ( paramIni.anchoImagen - paramIni.anchoImagen_e ) / 2;
	x2 = x1 + paramIni.anchoImagen_e;
	y1 = ( paramIni.altoImagen - paramIni.altoImagen_e ) / 2;
	y2 = y1 + paramIni.altoImagen_e;

	//	Selecciono el color con el que mostraré la marca central

	MgraColor(M_DEFAULT, ROJO);

	//	Dibujo una serie de líneas para separar diferentes regiones
	//	del buffer a analizar

	MgraLine(M_DEFAULT, M_overlay_normal, 0, y1, paramIni.anchoImagen, y1);
	MgraLine(M_DEFAULT, M_overlay_normal, 0, y2, paramIni.anchoImagen, y2);
	MgraLine(M_DEFAULT, M_overlay_normal, x1, 0, x1, paramIni.altoImagen);
	MgraLine(M_DEFAULT, M_overlay_normal, x2, 0, x2, paramIni.altoImagen);

	//	Dibujo los pixeles a analizar

	for (i = 0; i < NUMPIXEL; i++)  {
		MgraColor(M_DEFAULT, AMARILLO);
		MgraDot(M_DEFAULT, M_overlay_normal, x[i], y[i]);
		MgraArc(M_DEFAULT, M_overlay_normal, x[i], y[i], 7, 7, 0, 360);

		MgraColor(M_DEFAULT, AMARILLO_OSC);
		sprintf(nombre, "%d", i);
		dibuja_texto(M_overlay_normal, nombre, x[i], y[i] - 12, 3);
	}

	//	Vuelvo a seleccionar el color por defecto como TRANSPARENTE

	MgraColor(M_DEFAULT, TRANSPARENTE);

	return 0;
}*/

