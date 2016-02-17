/*****************************************************************************
	calibracion.cpp		Módulo en el que defino una serie de funciones para el
						proceso de imágenes. (Ver MATROX_1.c)

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Enero 2002.
*****************************************************************************/
#include "stdafx.h"

#include <mil.h>

#include <stdio.h>
#include <math.h> //para pow
#include <conio.h>

#include "..\ProcesoAux\estructuras.h"
#include "..\ProcesoAux\control_proceso_imagenes.h"
#include "..\ProcesoAux\gestion_ficheros.h"
#include "..\ProcesoAux\gestion_mensajes.h"

#include "control_barrido.h"
#include "..\ControlMicroscopio\control_microscopio.h"

#include "calibracion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// numero de objetivo (1..MAX_OBJETIVOS) para el que esta preparada la correccion. Si no hay correccion el valor es 0
static int		m_nObjetivoCorregido = 0;					// Por defecto no hay corrección para ningun objetivo

/**************************  prepara_correccion  *******************************
	Llama a prepara_correccion_banda para cada banda. calcula las imágenes auxiliares (M_correc_denom[banda] y M_correc_numer[banda]

	Función que realiza la copia de los ficheros necesarios en la realización
	de la serie, así como el ajuste del parámetro de exposición.
	AUXILIAR
	¿INTERFAZ en la version grafica (borland)?
****************************************************************************/
int prepara_correccion(parametros *param)
{
	int   i;

	/*  Preparamos el proceso de corrección a partir de un fichero con la
		imagen de un patrón de reflectividad y un fichero con la tabla de
		valores para la conversión de los niveles de gris resultantes. */

	for (i=0; i < param->nBandas; i++)  
    {
		if ( prepara_correccion_banda(param, i))  
        {
			error_leve("prepara_correccion", "Imposible realizar la corrección");
			return -1;
		}
	}
    m_nObjetivoCorregido = mspGetObjetivo(); // 1..MAX_OBJETIVOS
	return 0;
}

/*************************************************************************
	Esta función calcula las imágenes auxiliares (M_correc_denom[banda] y M_correc_numer[banda])
	que se emplearán en la corrección de la imagen adquirida con las imágenes de los patrones.
	A partir de las imagenes de los patrones.

    Se usa param->escala (referida en 8 bit) de tal manera que las imagenes posteriormente
    corregidas quedaran referidas en esta escala (de reflectancia por nivel de gris)

	NOTA: todos los buffers los he definido como float, excepto M_imagen1.
****************************************************************************/
int  prepara_correccion_banda(parametros *param, int banda)
{
	char  *fn, nombre[512], nombre_ext[512], mensaje[512];
	long  patX_os, patY_os, patX_cl, patY_cl;
	long  offsetX, offsetY;
	int filtro = abs(param->filtro[banda]);

	FILE  *fich;
	MIL_ID  M_imagen0, M_aux;

	/* Variables relativas a los niveles de gris de la imagen. */
	double  ref_os;	// Nivel de gris nominal
	double  ref_cl;	// de los patrones oscuro y claro.


	fn = "prepara_correccion_banda";

	if ( param->raiz_patrones == NULL )		// Si no hay raíz, no hay corrección.
		return -1;

	/*	Reservo memoria para un buffer de tamaño máximo, donde se almacenará
		provisionalmente las imágenes de patrón para proceder a reencuadrarlas
		al mismo tamaño	en el que se están capturando las imágenes.  */
	MbufAlloc2d(M_sistema, MAX_ANCHO_IMAGEN, MAX_ALTO_IMAGEN, param->Cam.profundidad+M_UNSIGNED,
		M_IMAGE+M_DISP+M_PROC, &M_imagen0 );
	if ( M_imagen0 == M_NULL )  {
		error_fatal(fn , "M_imagen0", 0);
		return -1;
	}
/////////////////////////////////////////////
//	MdispSelect(M_display_normal, M_imagen0);
/////////////////////////////////////////////

	//calculamos el nivel de gris para esta banda
    if (BITS_CAMARA == 12) 
    {
	    ref_os = param->Rueda.reflectancia_nominal_pat_os[filtro] / (param->escala/16); // 0..4096 (12 bit)
	    ref_cl = param->Rueda.reflectancia_nominal_pat_cl[filtro] / (param->escala/16);
    }
    else 
    {
	    ref_os = param->Rueda.reflectancia_nominal_pat_os[filtro] / param->escala;
	    ref_cl = param->Rueda.reflectancia_nominal_pat_cl[filtro] / param->escala;
    }

    //las imagenes de los patrones estan en 16 bit REALES, hay que calcular el valor correspondiente
    ASSERT(param->Cam.profundidad >= BITS_CAMARA);
    //Factor por el que hay que multiplicar una unica toma (sin acumular) para alcanzar
    //la profundidad deseada. Si 8bit -> 1. Si 12bit-> 16
    int factorSinAcum = 1 << (param->Cam.profundidad - BITS_CAMARA); //Si 8bit -> 1. Si 12bit-> 16
    ref_os = ref_os * factorSinAcum;
    ref_cl = ref_cl * factorSinAcum;


	//sprintf(mensaje, "Banda %d: Se aplica filtro %d\n", banda, filtro);

	// Construimos el path y nombre del patrón
	sprintf(nombre, "%s%s_%02d", param->raiz_patrones, NOM_PATRON_OS, filtro+1);
	strcpy(nombre_ext, nombre);
	strcat(nombre_ext, EXT_PAT);								// Nombre con la extensión

	/*	Compruebo que el patrón oscuro existe. */

    sendlog("Leyendo patron oscuro", nombre_ext);
	fich = fopen(nombre_ext, "rb");
	if (fich == NULL)  {
#if !defined (__BORLANDC__)
		putch('\a');
#endif
		sprintf(mensaje, "No se encuentra el patrón %s", nombre_ext);
		error_leve(fn, mensaje);
		MbufFree(M_imagen0);
		return -1;
	}
	fclose(fich);

	// Tamaño de la imagen de patrón
	MbufDiskInquire(nombre_ext, M_SIZE_X, &patX_os);
	MbufDiskInquire(nombre_ext, M_SIZE_Y, &patY_os);

	if ( param->Cam.anchoImagen > patX_os || param->Cam.altoImagen > patY_os )  {
		sprintf(mensaje, "tamaño del buffer de adquisición mayor que el de patrones");
		error_leve(fn, mensaje);
		MbufFree(M_imagen0);
		return -1;
	}
	if ( carga_imagen_campo(nombre_ext, M_imagen0) ) {
		sprintf(mensaje, "Imagen del patrón de reflectancia %s no disponible",
			nombre);
		error_leve(fn, mensaje);
		MbufFree(M_imagen0);
		return -1;
	}

	offsetX = (patX_os - param->Cam.anchoImagen) / 2;
	offsetY = (patY_os - param->Cam.altoImagen) / 2;
	MbufChild2d(M_imagen0, offsetX, offsetY,
		param->Cam.anchoImagen, param->Cam.altoImagen, &M_aux);
/////////////////////////////////////////////
//	MgraRect(M_DEFAULT, M_overlay_normal, offsetX, offsetY,
//		offsetX+param->anchoImagen, offsetY+param->altoImagen);
/////////////////////////////////////////////

	// Patrón oscuro en M_imagen2
	MbufCopy(M_aux, M_imagen2);
	MbufFree(M_aux);
/////////////////////////////////////////////
//	getch();
/////////////////////////////////////////////
	/*  Lectura de la imagen del patrón de reflectancia claro. */

	// Construimos el path y nombre del patrón
	sprintf(nombre, "%s%s_%02d", param->raiz_patrones, NOM_PATRON_CL, filtro+1);
	strcpy(nombre_ext, nombre);
	strcat(nombre_ext, EXT_PAT);

	/*	Compruebo que el patrón claro existe. */

    sendlog("Leyendo patron claro", nombre_ext);
	fich = fopen(nombre_ext, "rb");
	if (fich == NULL)  {
#if !defined (__BORLANDC__)
		putch('\a');
#endif
		sprintf(mensaje, "No se encuentra el patrón %s", nombre_ext);
		error_fatal(fn, mensaje,0);
		MbufFree(M_imagen0);
		return -1;
	}
	fclose(fich);

	// Tamaño de la imagen de patrón
	MbufDiskInquire(nombre_ext, M_SIZE_X, &patX_cl);
	MbufDiskInquire(nombre_ext, M_SIZE_Y, &patY_cl);
	if ( param->Cam.anchoImagen > patX_cl || param->Cam.altoImagen > patY_cl )  {
		sprintf(mensaje, "tamaño del buffer de adquisición mayor que el de patrones");
		error_leve(fn, mensaje);
		MbufFree(M_imagen0);
		return -1;
	}
	if ( carga_imagen_campo(nombre_ext, M_imagen0) )  {
		sprintf(mensaje, "Imagen del patrón de reflectancia %s no disponible",
			nombre);
		error_leve(fn, mensaje);
		MbufFree(M_imagen0);
		return -1;
	}

	if ( patX_cl != patX_os )  {
		offsetX = (patX_cl - param->Cam.anchoImagen) / 2;
	}
	if ( patY_cl != patY_os )  {
		offsetY = (patY_cl - param->Cam.altoImagen) / 2;
	}
	MbufChild2d(M_imagen0, offsetX, offsetY,
		param->Cam.anchoImagen, param->Cam.altoImagen, &M_aux);

	// Patrón claro en M_imagen3
	MbufCopy(M_aux, M_imagen3);
	MbufFree(M_aux);

	/*	A continuación preparamos las imágenes auxiliares que se utilizarán
		en la corrección de las imágenes adquiridas:
		- una imagen que se restará a la imagen adquirida
		- una imagen por la que se dividirá el resultado
		De esta forma, la corrección se efectuará con sólo dos operaciones.

		La imagen denominador se obtiene dividiendo la diferencia de las imágenes
		patrón entre la diferencia de las reflectancias de los patrones.
		Utilizamos para almacenarla un buffer en coma flotante */

	MimArith (M_imagen3, M_imagen2, M_imagen3, M_SUB);
	MimArith (M_imagen3, ref_cl - ref_os, M_correc_denom[banda], M_DIV_CONST);

	/*	La imagen que se restará a la imagen adquirida se calcula restando
		a la imagen del patrón oscuro la diferencia de las imágenes patrón
		escalada por la constante:  ref_os / (ref_cl - ref_os) */

	MimArith (M_imagen3, ref_os / (ref_cl - ref_os) , M_correc_aux, M_MULT_CONST);
	MimArith (M_imagen2, M_correc_aux, M_correc_numer[banda], M_SUB);

	//	Libero la memoria reservada en la función. 
	if (M_imagen0 != M_NULL)		MbufFree(M_imagen0);

	return  0;
}


/**************************************************************************
	Función para efectuar la corrección de la falta de uniformidad en la
	iluminación.

    Las imagenes corregidas quedaran referidas a la escala (de reflectancia por nivel de gris)
    usada en prepara_correccion_banda. Si se quiere referir la imagen de salida en una escala
    diferente, se ha de multiplicar por la escala "de adquisicion" (la usada en prepara_correccion_banda,
    en la que estan referida los patrones) y dividir por la nueva escala

    Si no existe corrección posible se pasa la imagen sin corregir (!hay_correc),
	Si hay corrección (hay_correc = 2) se ajusta la imagen.
		El resultado se devuelve en M_banda[i]
****************************************************************************/
void  corrige_iluminacion(int banda, MIL_ID milBuffer)
{
    if (milBuffer == M_NULL)
        milBuffer = M_imagen1;

	if ( m_nObjetivoCorregido == 0 ) 
    {
	    //	Si no se dispone de las imágenes de los patrones,
	    //	no se realiza corrección alguna.
		MbufCopy(milBuffer, M_banda[banda]);
	}
    else
    {

        MimArith (milBuffer, M_correc_numer[banda], M_correc_aux, M_SUB + M_SATURATION);

	    MimArith(M_correc_aux, M_correc_denom[banda], M_correc_aux, M_DIV);
	    MimArith(M_correc_aux, 0.5, M_correc_aux, M_ADD_CONST);
//        MbufCopy(M_correc_aux,M_banda[banda]);
	    MimClip(M_correc_aux, M_banda[banda], M_OUT_RANGE, 0, float(pow(2,16)-1), 0, float(pow(2,16)-1)); // Para que si hay saturacion, se quede blanco (30 / 7 / 2009)
    }
}

// Devuelve el numero de objetivo (1..MAX_OBJETIVOS) para el que esta preparada la correccion. 
// Si no hay correccion el valor es 0
int get_objetivo_corregido()
{
    return m_nObjetivoCorregido;
}


