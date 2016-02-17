/*****************************************************************************
	histograma.cpp		Módulo en el que defino una serie de funciones
							para el proceso de imágenes.

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Enero 2002.
*****************************************************************************/
#include "stdafx.h"

#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#if !defined (__BORLANDC__)
#include <stdlib.h>
#endif
#include <stdio.h>
#include <math.h>

#include <mil.h>

#include "estructuras.h"
#include "histograma.h"
#include "control_proceso_imagenes.h"
// #include "analisis_histograma.h" OBSOLETO

#include "funciones_graficas.h"
#include "gestion_mensajes.h"
#include "gestion_ficheros.h"

#if !defined (__BORLANDC__)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

#define AREA_GAUSSIANA		30

/*	Variables externas a este módulo. */

extern parametros	paramIni;				// Parámetros iniciales globales ala aplicacion
extern long		histo_estimado[NUMVAL];	// Histograma estimado de la muestra.

/* Definición de variables locales a este módulo. */

/*  Declaramos el vector de coeficientes para el filtrado gaussiano. */

double  filtro_gauss[NUMELEMFIL];
long  histograma[NUMVAL];			// Vector auxiliar para cálculo de histogramas.

#if defined (__BORLANDC__)
void PintaHistoEstimado(int);
#endif

#if !defined (__BORLANDC__)
/**************************  construye_histograma  *************************
	Función para mostrar en el overlay del display normal el histograma
	de la imagen actual.
* OBSOLETO
void  construye_histograma(MIL_ID M_fb, int desplazamiento, int signo,
	int coma_flotante, double kte)
{
	char  mensaje[256];
	int  i, numpix; 
	double  acum, acumcuad, media, sigma;

	//	Transfiero la imagen original a la imagen auxiliar M_imagen1. 

	if ( coma_flotante )
		if ( desplazamiento < 0 )
			MimArith(M_fb, 1 << -desplazamiento, M_imagen1, M_DIV_CONST);
		else
			MimArith(M_fb, 1 << desplazamiento, M_imagen1, M_MULT_CONST);
	else
		MimShift(M_fb, M_imagen1, desplazamiento);

	evalua_histograma(M_imagen1, NULL);			// Se calcula, presenta y acumula

	// Muestro en el buffer resultados el histograma de la imagen. 

	muestra_histograma(NULL, M_fb, 0, 1, FILA3, COLUMNA1, COL_ACTI, 1, 1,
		"HISTOGRAMA DE LA IMAGEN");

	//	Calculo y presento el valor medio del histograma. 

	acum = acumcuad = 0.0;
	if (signo) {
		for (numpix=i=0; i < NUMVAL/2; i++) {
			acum += i * (double) histograma[i];
			acumcuad += i * i * (double) histograma[i];
			numpix += histograma[i];
		}
		for (; i < NUMVAL; i++) {
			acum += (i - NUMVAL) * (double) histograma[i];
			acumcuad += (i - NUMVAL) * (i - NUMVAL) * (double) histograma[i];
			numpix += histograma[i];
		}
	} else
		for (numpix=i=0; i < NUMVAL; i++) {
			acum += i * (double) histograma[i];
			acumcuad += i * i * (double) histograma[i];
			numpix += histograma[i];
		}
	media = acum / numpix;
	sigma = sqrt((numpix * acumcuad - acum * acum) / (numpix * (double) (numpix - 1)));

	sprintf(mensaje, "Media: %.2lf  Sigma: %.2lf  (en reflectancias: %.2lf  %.2lf)",
		media, sigma, kte * media, kte * sigma);
	sendop(mensaje);
}
*/
#endif


/**************************  construye_histo_base  *************************
	Función para mostrar en el overlay del display normal el histograma
	de la imagen actual.
****************************************************************************/
void  construye_histo_base()
{
	evalua_histograma(M_imagen1, NULL);	// Se calcula, presenta y acumula

	/* Muestro en el buffer resultados el histograma de la imagen. */

	muestra_histograma(NULL, M_overlay_normal, 0, 1, FILA1, COLUMNA1, COL_ACTI, 1, 1,
		"HISTOGRAMA DE LA IMAGEN");
}

/**************************  construye_histo_base  *************************
	Función para mostrar en el overlay del display normal el histograma
	de la imagen actual.
****************************************************************************/
void  construye_histo_base(int nNumVal)
{
    MIL_ID M_histograma_auxiliar;
	MimAllocResult(M_sistema, nNumVal, M_HIST_LIST, &M_histograma_auxiliar);
    long* histo = new long[nNumVal];

	//	Calculamos el histograma de toda la imagen
	MimHistogram(M_imagen1, M_histograma_auxiliar);

	// Obtengo los resultados, almacenándolos en el array: histograma
	MimGetResult(M_histograma_auxiliar, M_VALUE, histo);

	// Muestro en el buffer resultados el histograma de la imagen.
	muestra_histograma(histo, M_overlay_normal, 0, 1, FILA1, COLUMNA1, COL_ACTI, 1, 1,
		"HISTOGRAMA DE LA IMAGEN",nNumVal);

    MbufFree(M_histograma_auxiliar);
    delete [] histo;
}


/**************************  construye_histograma_estimado  *****************
	Función para construir un histograma estimado de la muestra mediante
	la acumulación de los histogramas de una serie de campos.
	Cada llamada a la función agrega el histograma de la imagen actual al
	histograma estimado, y presenta ambos en pantalla.

	NOTA: "histo_estimado" es una variable global.
* OBSOLETO
void  construye_histograma_estimado()
{
	evalua_histograma(M_imagen1, NULL);			// Se calcula, presenta y acumula
	acumula_histograma(NULL, NUMVAL, histo_estimado);	// el histograma de la imagen.

	// Muestro en el buffer resultados el histograma de la imagen. 

	muestra_histograma(NULL, M_resultados, 0, 1, FILA1, COLUMNA2, COL_ACTI, 1, 1,
		"HISTOGRAMA DE LA IMAGEN");

	// Muestro en el buffer resultados el histograma global estimado. 

	muestra_histograma(histo_estimado, M_resultados, 0, 1, FILA1, COLUMNA1, COL_ACUM, 1, 1,
		"HISTOGRAMA GLOBAL ESTIMADO");
#if defined (__BORLANDC__)
    PintaHistoEstimado(0);
#endif
}
*/

/**************************  evalua_histograma  *****************************
	Función para calcular el histograma de la imagen.
	Los argumentos de la función son:
		- El buffer en el que se calcula el histograma.
		- La variable donde se almacenan los resultados.

	NOTA: en esta función se reserva memoria.

	NOTA: antigua definición: void  evalua_histograma(long *histo)
****************************************************************************/
void  evalua_histograma(MIL_ID buffer, long *histo)
{
	if ( buffer == M_NULL)  {
#if !defined (__BORLANDC__)
		error_leve("evalua_histograma", "Valor de buffer incorrecto");
#endif
//		memset(histo, 0, sizeof(histo));
		return;
	}

	if ( histo == NULL )			// Si no se especifica un histograma,
		histo = histograma;			// por defecto se toma el global.

	//	Calculamos el histograma de toda la imagen
	MimHistogram(buffer, M_histograma);

	// Obtengo los resultados, almacenándolos en el array: histograma
	MimGetResult(M_histograma, M_VALUE, histo);
}


#if !defined (__BORLANDC__)
/**************************  evalua_histograma_central  *********************
	Función para calcular el histograma de la imagen en el área central.
	El buffer donde estará la imagen a procesar es M_enfoque1.

	NOTA: en esta función se reserva memoria.
****************************************************************************/
void  evalua_histograma_central(MIL_ID buffer, long *histo)
{
	MIL_ID  M_histo_central;		// Variable local.

	/*	Compruebo que no esta vacío el buffer. */

	if ( MbufInquire(buffer, M_SIZE_BYTE, 0) == NULL)
		return;

	if ( histo == NULL )				// Si no se especifica un histograma,
		histo = histograma;				// por defecto se toma el global.

	/*	Se fija un buffer de resultados para almacenar los resultados
		del histograma. */
	MimAllocResult(M_sistema, NUMVAL, M_HIST_LIST, &M_histo_central);

	//  Calculamos el histograma en la zona central de la imagen: M_enfoque1
	MimHistogram(buffer, M_histo_central);

	// Obtengo los resultados, almacenándolos en el array: histograma
	MimGetResult(M_histo_central, M_VALUE, histo);

	// Libero el buffer de resultados del histograma
	MbufFree(M_histo_central);
}
#endif


/**************************  acumula_histograma  ****************************
   Función para acumular el histograma recién calculado en un vector.
****************************************************************************/
void  acumula_histograma(long *histo, int num_val, long *histoacum)
{
	int  i;

	if ( histo == NULL )				// Si no se especifica un histograma,
		histo = histograma;				// por defecto se toma el global.

	if (histoacum != NULL)
    {
        memset(histoacum,0,num_val*sizeof(long));
		histoacum[0] = histo[0];	// vamos a asignar el valor por induccion ("efecto domino"), asi que inicializamos
		for (i=1; i < num_val; i++)		// Empezamos en el segundo
        {
		    histoacum[i] += histo[i];	   
		    histoacum[i] += histoacum[i-1];	
        }
    }
}

/**************************  muestra_histograma  ****************************
   Función para presentar un histograma en pantalla en overlay.
   Los argumentos que se le pasan a esta función son:
	- histo: tipo de histograma a representar. Si no es especifica (NULL)
			se toma por defecto el global.
    - fondo_escala: valor maximo posible del histograma. se calcula automaticamente
	- paso: numero de valores de gris que se agrupan para hacer un valor en el histograma
            se calcula automaticamente en dibuja_histo_reflec asi que no es necesario
	- fila, columna:coordenadas de posición de los histogramas.
	- display: display en el que se desea mostrar el histograma.
	- buffer: buffer asociado al display.
	- titulo: string a mostrar en el título del histograma.
    - nNumVal: profundidad del buffer
****************************************************************************/
void  muestra_histograma(long *histo, MIL_ID M_fb, long fondo_escala, int paso,
	int fila, int columna, int color, int solido, int borrar, char *titulo, int nNumVal)
{
	int  nmin, hpos, vpos;

	if ( histo == NULL )				// Si no se especifica un histograma,
		histo = histograma;				// por defecto se toma el global.

	/*	En primer lugar, determinamos la escala de dibujo. */

	nmin = 0; //nmin = (int) ceil(UMBRAL_RESINA / escala); FSG obsoleto carbones

	long  mayor;
	//	Si se ha especificado un fondo de escala dado, lo utilizamos. En caso contrario,
	//	lo calculamos como el valor máximo en una cierta banda del histograma. 
    /* Esto se hace en dibuja_histo_reflec con paso automaticamente calculado (si mayor es 0)
	int  i, j;
	long  suma;
	if (fondo_escala > 0)
		mayor = fondo_escala;
	else
        for (mayor=0, i=0; i < NUMVAL-1; i+=paso) { //i=paso*(nmin/paso) FSG obsoleto carbones
			for (suma=0, j=0; j < paso && i+j < NUMVAL-1; j++)
				suma += histo[i+j];
			if ( suma > mayor )
				mayor = suma;
		}
    */
    mayor = 0;

	//  Determinamos su posición en coordenadas gráficas. 
	hpos = POSIH + columna * ANCH;
	vpos = POSIV + fila * ALTH;

	if ( borrar ) {
		MgraColor(M_DEFAULT, TRANSPARENTE);	// Selecciono el color por defecto TRANSPARENTE.
		MgraRectFill(M_DEFAULT, M_fb, hpos, vpos, hpos+ANCH-1, vpos+ALTH-1);
		MgraColor(M_DEFAULT, AMARILLO);		// Selecciono el color por defecto AMARILLO.
	}

	//	Función para dibujar un histograma en un frame buffer.
    if (BITS_CAMARA == 12) 
	    dibuja_histo_reflec(histo, nNumVal, mayor, paso, paramIni.escala/16, M_fb, ANCH, ALTH-ALTOBANDA,
		    hpos+8, vpos+ALTOBANDA, color, AMARILLO, solido);
    else 
	    dibuja_histo_reflec(histo, nNumVal, mayor, paso, paramIni.escala, M_fb, ANCH, ALTH-ALTOBANDA,
		    hpos+8, vpos+ALTOBANDA, color, AMARILLO, solido);

	//	Escribo el título del histograma. 
	MgraColor(M_DEFAULT, AMARILLO);
	dibuja_texto(M_fb, titulo, hpos+ANCH/2, vpos+ALTOBANDA/2, 4);
}

/**************************  muestra_histograma  ****************************
   Función para presentar un histograma en pantalla en overlay.
   Los argumentos que se le pasan a esta función son:
	- histo: tipo de histograma a representar. Si no es especifica (NULL)
			se toma por defecto el global.
    - fondo_escala: valor maximo posible del histograma. se calcula automaticamente
	- paso: numero de valores de gris que se agrupan para hacer un valor en el histograma
            se calcula automaticamente en dibuja_histo_reflec asi que no es necesario
	- fila, columna:coordenadas de posición de los histogramas.
	- display: display en el que se desea mostrar el histograma.
	- buffer: buffer asociado al display.
	- titulo: string a mostrar en el título del histograma.
****************************************************************************/
void  muestra_histograma(long *histo, MIL_ID M_fb, long fondo_escala, int paso,
	int fila, int columna, int color, int solido, int borrar, char *titulo)
{
    muestra_histograma(histo, M_fb, fondo_escala, paso,
	fila, columna, color, solido, borrar, titulo, NUMVAL);
}



/**************************  graba_histograma  ******************************
   Función para grabar en un fichero un histograma.
****************************************************************************/
int graba_histograma(char *nombre, long *histo)
{
	char  nombre_ext[1024];
	int  i;
	long  mayor;
	FILE  *fich;

	strcpy(nombre_ext, nombre);
	strcat(nombre_ext, ".his");			// Nombre del fichero de histograma.
	if ((fich = crea_fichero_ASCII(nombre_ext)) == NULL)
		return -1;

	if ( histo == NULL )				// Si no se especifica un histograma,
		histo = histograma;				// por defecto se toma el global.

	for (mayor=0, i=1; i < NUMVAL-1; i++)
		if (histo[i] > mayor)
			mayor = histo[i];

	graba_fichero_histo(fich, histo, NUMVAL, mayor, 0);

	fclose(fich);						// Cierro el fichero creado.
	return  0;
}


/**************************  recupera_histograma  ***************************
	Función para recuperar un histograma grabado en un fichero.
****************************************************************************/
int recupera_histograma(char *nombre, long *histo)
{
	char  nombre_ext[1024];
	int  i, numval;
	FILE  *fich;
	long  *histotemp, mayor, total;

	if ( histo == NULL )				// Si no se especifica un histograma,
		histo = histograma;				// por defecto se toma el global.

	strcpy(nombre_ext, nombre);
	strcat(nombre_ext, ".his");			// Nombre del fichero de histograma.
	if ((fich = abre_fichero_ASCII(nombre_ext)) == NULL)
		return -1;

	lee_fichero_histo(fich, &histotemp, &numval, &mayor, &total);

	for (i=0; i < NUMVAL; i++)			// Copiamos los valores de uno a otro.
		histo[i] = histotemp[i];

	free((char *) histotemp);
	fclose(fich);
	return  0;
}


#if !defined (__BORLANDC__)
/**************************  procesa_histograma  ****************************
   Función para determinar la media y la desviación del nivel de gris
   de una zona de una imagen a partir de su histograma.
****************************************************************************/
int procesa_histograma(long *histo, double *media, double *sigma)
{
	unsigned int  i;
	int  total;
	double  suma, sumacuad;

	if ( histo == NULL )			// Si no se especifica un histograma,
		histo = histograma;			// por defecto se toma el global.

	total = 0L;
	suma = sumacuad = 0.0;
	for (i=0; i < NUMVAL; i++) {
		total += histo[i];
		suma += histo[i] * i;
		sumacuad += histo[i] * i * (double) i;
	}
	*media = (total > 0)? suma / total : 0.0;
	*sigma = (total > 1)? sqrt((total * sumacuad - suma * suma) /
		(total * (double) (total-1))) : 0.0;

	return  total;
}
/**************************  procesa_histograma  ****************************
   Función que devuelve el percentil de orden dado de un histograma
****************************************************************************/
int percentil_histograma(long *histo, int num_val, int nOrden)
{
    ASSERT(nOrden<= 100); 
    long* histo_acum = new long[num_val];

    acumula_histograma(histo,num_val,histo_acum);
    
    long total = histo_acum[num_val-1];

    for (int i=0; i < num_val; i++) {
		if (histo_acum[i] >= (double)(total* nOrden)/100)
        {
            delete [] histo_acum;
            return i;
        }
	}

    delete [] histo_acum;
    ASSERT(FALSE); // necesariamente tiene que salir antes
	return  -1;
}
#endif


/**************************  moda_histograma  *******************************
   Función para localizar la moda de una banda de un histograma.
****************************************************************************/
int  moda_histograma(long *histo, int desde, int hasta)
{
	int  i, posicion;
	long  maximo;

	if ( histo == NULL )				// Si no se especifica un histograma,
		histo = histograma;				// por defecto se toma el global.

	maximo = histo[posicion=desde];
	for (i=desde+1; i <= hasta; i++)
		if ( histo[i] > maximo ) {
			maximo = histo[i];
			posicion = i;
		}

	return posicion;
}


/**************************  antimoda_histograma  ***************************
	Función para localizar la antimoda de una banda de un histograma.
*****************************************************************************/
int antimoda_histograma(long *histo, int desde, int hasta)
{
	int  i, posicion;
	long  minimo;

	if ( histo == NULL )
		histo = histograma;

	minimo = histo[posicion=desde];
	for (i=desde+1; i <= hasta; i++)  {
		if ( histo[i] < minimo )  {
			minimo = histo[i];
			posicion = i;
		}
	}

	return  posicion;
}


/**************************  asimetria_histograma  *************************
   Función para determinar las desviaciones del nivel de gris en una zona de
   una imagen por encima y por debajo de la moda a partir de su histograma.
****************************************************************************/
void  asimetria_histograma(long *histo, int moda, double *sigma_inf,
					double *sigma_sup)
{
	int  i;
	long  total;
	double  sumacuadif;

	if ( histo == NULL )				// Si no se especifica un histograma,
		histo = histograma;				// por defecto se toma el global.

	// Cálculo de la sigma inferior
	total = histo[moda] / 2;
	sumacuadif = 0.0;
	for (i=0; i < moda; i++) {
		total += histo[i];
		sumacuadif += histo[i] * (moda - i) * (double) (moda - i);
	}
	*sigma_inf = (total > 0)? sqrt(sumacuadif / total) : 0.0;

	// Cálculo de la sigma superior
	total = histo[moda] - histo[moda]/2;
	sumacuadif = 0.0;
	for (i=moda+1; i < NUMVAL; i++) {
		total += histo[i];
		sumacuadif += histo[i] * (i - moda) * (double) (i - moda);
	}
	*sigma_sup = (total > 0)? sqrt(sumacuadif / total) : 0.0;
}


/**************************  normaliza_histograma  *************************
	Función para calcular el histograma normalizado de uno dado.
****************************************************************************/
double  normaliza_histograma(long *histo, double *histo_norm)
{
	int i;
	long  num_pixel;
	double  factor;

	/*	Definimos como factor de normalización el área de una curva gaussiana (30)
		dividido por número de pixels del histograma. */

	num_pixel = 0;
	for (i=0; i < NUMVAL; i++)
		num_pixel += histo[i];
	factor = AREA_GAUSSIANA / (double) num_pixel;	// AREA_GAUSSIANA = 30

	// Normalización del histograma
	for (i=0; i < NUMVAL; i++)
		histo_norm[i] = histo[i] * factor;

	return factor;
}


/**************************  filtra_histograma  *****************************
   Función para calcular el filtrado de un histograma por una gaussiana
   con radio progresivamente variable.
****************************************************************************/
void  filtra_histograma(long *histo, long *histo_fil, double rIni, double rFin)
{
	int  i, j, k, paso;
	double  radio, coef, suma, sumcoef, valmax, aux[NUMVAL];

	if ( histo == NULL )				// Si no se especifica un histograma,
		histo = histograma;				// por defecto se toma el global.
//	if ( histo_fil == NULL || histo_fil == histo )
	if ( histo_fil == NULL )
		error_fatal("filtra_histograma", "Se requiere un histograma destino", 0);

	/*  Aplicamos el filtro al histograma. */

	for (valmax=0.0, i=0; i < NUMVAL; i++) {

		/*	Se determina el radio aplicable en cada nivel de gris,
			y a partir de él se fija el paso para recorrer el vector
			que contiene los coeficientes del filtro gaussiano. */

		radio = rIni + i * (rFin - rIni) / (NUMVAL - 1);
		paso = (int) ((NUMELEMFIL - 1) / radio);

		sumcoef = filtro_gauss[0];
		suma = filtro_gauss[0] * histo[i];
		for (j=(int)radio, k=j*paso; j > 0; j--, k-=paso) {
			coef = filtro_gauss[k];
			if (i-j >= 0) {
				sumcoef += coef;
				suma += coef * histo[i-j];
			}
			if (i+j < NUMVAL) {
				sumcoef += coef;
				suma += coef * histo[i+j];
			}
		}
		aux[i] = suma / sumcoef;
		if (aux[i] > valmax)
			valmax = aux[i];
	}

	for (i=NUMVAL-1; i >= 0; i--)
		histo_fil[i] = (long) aux[i];

	/*  Ajustamos la escala del filtro para que la moda valga 1000. */
/*
	valmax /= 1000.0;
	for (i=NUMVAL-1; i >= 0; i--)
		histo_fil[i] = (long) (aux[i] / valmax);
*/
}

/**************************  calcula_coef_filtro_histograma  ****************
	Función para calcular los coeficientes del filtro gaussiano que se
	aplicará posteriormente a los histogramas. Los coeficientes se guardan
	como enteros, empleando un factor de escala de valor KTEFIL.
****************************************************************************/
void  calcula_coef_filtro_histograma()
{
   int  i;
   double  sigma;

   /*	Creamos el vector que contiene el filtro gaussiano considerando la
		desviación igual a un tercio del radio (coeficiente mínimo: 0.011109) */

   sigma = ((double) NUMELEMFIL-1) / 3;
   for (i=NUMELEMFIL-1; i >= 0; i--)
      filtro_gauss[i] = exp((i/sigma)*(i/sigma)/-2);
}


#if !defined (__BORLANDC__)
/**************************  evalua_histograma_zona  ************************
   Función para calcular el histograma de la intersección de la imagen
   de partida con una zona binaria dada.
****************************************************************************/
void  evalua_histograma_zona(MIL_ID M_origen, MIL_ID M_mascara, long *hist)
{
	long area, area_no_interes, histoaux[NUMVAL];
	long ancho, alto;
	MIL_ID M_aux;

	ancho = MbufInquire(M_origen, M_SIZE_X, M_NULL);
	alto = MbufInquire(M_origen, M_SIZE_Y, M_NULL);

	MbufAlloc2d(M_sistema, ancho, alto, 8 + M_UNSIGNED, M_IMAGE+M_PROC, &M_aux);

	if ( hist == NULL )		// Si no se especifica un histograma,
		hist = histograma;		// por defecto se toma el global.

	MimBinarize(M_mascara, M_aux, M_NOT_EQUAL, 0, 255);

	//	Evaluo el área de la región
	evalua_histograma(M_aux, histoaux);
	area = histoaux[255];
	area_no_interes = (ancho * alto) - area;

	//	Realizo una operación lógica
	MimArith(M_origen, M_aux, M_aux, M_AND);
	evalua_histograma(M_aux, hist);

	hist[0] -= area_no_interes;

	MbufFree(M_aux);
}
#endif


/**************************  muestra_marca  *********************************
   Función para presentar una marca sobre un histograma en overlay.
****************************************************************************/
void  muestra_marca(int valor, MIL_ID M_fb, int paso, int fila, int columna,
					int color, int borrar)
{
	int  hpos, vpos;

	//	Determinamos la posición del histograma en coordenadas gráficas. 

	hpos = POSIH + columna * ANCH;
	vpos = POSIV + fila * ALTH;

	if ( borrar ) {
		MgraColor(M_DEFAULT, TRANSPARENTE);	// Selecciono el color por defecto TRANSPARENTE.
		MgraRectFill(M_DEFAULT, M_fb, hpos, vpos, hpos+ANCH-1, vpos+ALTH-1);
		MgraColor(M_DEFAULT, AMARILLO);		// Selecciono el color por defecto AMARILLO.
	}

	//	Dibujo las marcas en overlay. 
    if (BITS_CAMARA == 12) 
	    dibuja_marca_histo(valor, NUMVAL, paso, paramIni.escala/16, M_fb, ANCH, ALTH-ALTOBANDA,
		    hpos+8, vpos+ALTOBANDA, color, BLANCO);
    else 
	    dibuja_marca_histo(valor, NUMVAL, paso, paramIni.escala, M_fb, ANCH, ALTH-ALTOBANDA,
		    hpos+8, vpos+ALTOBANDA, color, BLANCO);
}


/***************************  copia_histograma  *****************************
	Función para copiar un intervalo de un histograma a otro.
	El intervalo definido por iInt e iSup es cerrado.
	El resto del histograma es puesto a cero
*****************************************************************************/
int copia_histograma(long *histo_o, long *histo_d, int iInf, int iSup)
{
	int i;
	int area;

	area = 0;

	for(i=0; i < NUMVAL; i++)		histo_d[i] = 0;

	for(i=iInf; i <= iSup; i++)  {
		histo_d[i] = histo_o[i];
		area += histo_o[i];
	}

	return area;
}

