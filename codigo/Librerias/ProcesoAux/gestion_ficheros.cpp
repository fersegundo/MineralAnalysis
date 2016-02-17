/*****************************************************************************
	gestion_ficheros.cpp	Funciones para el manejo de ficheros

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Enero 2002
*****************************************************************************/
#include "stdafx.h"

#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#include <stdio.h>
#if !defined (__BORLANDC__)
#include <stdlib.h>
#endif

#include "estructuras.h"
#include "control_proceso_imagenes.h" //para M_sistema

#include "gestion_ficheros.h"

#include "gestion_mensajes.h"
#include "myVars.h"
#include "..\LibTiff\InterfazLibTiff.h"

#if !defined (__BORLANDC__)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif


/**************************  abre_fichero_ASCII  *****************************
   Función para abrir un fichero ASCII ya existente para entrada de datos.
*****************************************************************************/
FILE * abre_fichero_ASCII(char *nombre)
{
   FILE  *fich;
#if !defined (__BORLANDC__)
   char text[256];
#endif

   if ((fich = fopen(nombre, "rt")) == NULL) {
#if !defined (__BORLANDC__)
      sprintf(text, "Fichero %s inaceptable", nombre);
      error_leve("abre_fichero_ASCII", text);
#endif
      return  NULL;
   }
   return  fich;
}


/**************************  crea_fichero_ASCII  *****************************
   Función para crear un fichero ASCII para salida de datos.
*****************************************************************************/
FILE * crea_fichero_ASCII(char *nombre)
{
   FILE  *fich;
#if !defined (__BORLANDC__)
   char text[256];
#endif

   if ((fich = fopen(nombre, "wt")) == NULL) {
#if !defined (__BORLANDC__)
      sprintf(text, "Fichero %s inaceptable", nombre);
      error_leve("crea_fichero_ASCII", text);
#endif
      return  NULL;
   }
   return  fich;
}

/**************************  graba_imagen_campo  ****************************
   Función para efectuar la grabación en disco de una imagen del campo.
   "nombre" es el nombre del archivo a almacenar.
   "buffer" es el buffer que quiero copiar a disco.
****************************************************************************/
int graba_imagen_campo(char *nombre, MIL_ID buffer, int nBits, double dEscala)
{
	char  nombre_ext[1024], *ext, mensaje[1024], *fn = "graba_imagen_campo";

	strcpy(nombre_ext, nombre);		// Se copia en nombre_ext el nombre de la imagen a cargar
	ext = strrchr(nombre_ext, '.');	// Se busca la extensión del fichero que se desea guardar
	strcpy(nombre_ext, ext+1);

	// Copiamos el buffer seleccionado en el archivo seleccionado (nombre_ext).
	if ( strcmp(nombre_ext,"jpg")==0 )  MbufExport(nombre, M_JPEG_LOSSLESS, buffer);
	if ( strcmp(nombre_ext,"tif")==0 )
    {
        MbufExport(nombre, M_TIFF, buffer);
        if (nBits >0 && dEscala > 0)
            INTERFAZ_LIBTIF::EscribirTags(nombre, nBits, dEscala);
        else if (dEscala > 0)
            INTERFAZ_LIBTIF::EscribirTags(nombre, 0, dEscala);
        else if (nBits > 0)
            INTERFAZ_LIBTIF::EscribirTags(nombre, nBits, 0);
        else 
            INTERFAZ_LIBTIF::EscribirTags(nombre, 0, 0);
    }
	if ( strcmp(nombre_ext,"mim")==0 )  MbufExport(nombre, M_MIL, buffer);

	/* Compruebo que no ha habido ningún error durante el proceso de grabación a disco. */

	if ( MbufDiskInquire(nombre, M_SIZE_X, M_NULL) == 0)  {
		sprintf(mensaje, "No puedo crear la imagen %s", nombre_ext);
		error_fatal(fn, mensaje, 0);
		return 1;
	}

	return  0;
}

/**************************  graba_imagen_campo  ****************************
   Función para efectuar la grabación en disco de una imagen del campo.
   "nombre" es el nombre del archivo a almacenar.
   "buffer" es el buffer que quiero copiar a disco. Se espera que este buffer
   este en 8 o 16 bits reales (dependiendo de la profundidad de la camara)
   LA IMAGEN SE GRABARA EN 8 BIT
****************************************************************************/
int graba_imagen_campo_8bit(parametros& param, char *nombre, MIL_ID buffer, double dEscala)
{
    return graba_imagen_campo_bits(param, nombre, buffer, 8, dEscala);
}


/**************************  graba_imagen_campo  ****************************
   Función para efectuar la grabación en disco de una imagen del campo.
   "nombre" es el nombre del archivo a almacenar.
   "buffer" es el buffer que quiero copiar a disco. Se espera que este buffer
   este en 8 o 16 bits reales (dependiendo de la profundidad de la camara)
   nBits - profundidad en bits en que se grabará la imagen (8,12,16)
****************************************************************************/
int graba_imagen_campo_bits(parametros& param, char *nombre, MIL_ID buffer, int nBits, double dEscala)
{
    int res = 0;

    if (param.Cam.profundidad == 16 && (nBits == 8 || nBits == 12))
    {
        MIL_ID M_buf16;
        long altoImagen,anchoImagen;
        MbufInquire(buffer, M_SIZE_X, &anchoImagen);
        MbufInquire(buffer, M_SIZE_Y, &altoImagen);
		MbufAlloc2d(M_sistema, anchoImagen, altoImagen, 16+M_UNSIGNED,
			M_IMAGE+M_DISP+M_PROC, &M_buf16 );
        if (nBits == 8)
        {
            MIL_ID M_buf8;
		    MbufAlloc2d(M_sistema, anchoImagen, altoImagen, 8+M_UNSIGNED,
			    M_IMAGE+M_DISP+M_PROC, &M_buf8 );
            MimShift(buffer,M_buf16,-8);//conversion a 8 bit 
		    MbufCopy(M_buf16, M_buf8); 
    //        MimShift(buffer,buffer,4);//conversion a 12 bit. Para que luego los histogramas funcionen correctamente

            res = graba_imagen_campo(nombre,M_buf8, nBits, dEscala);
            MbufFree(M_buf8);
        }
        else //nBits==12
        {
            MimShift(buffer,M_buf16,-4);//conversion a 12 bit 
            res = graba_imagen_campo(nombre,M_buf16, nBits, dEscala);
        }
        MbufFree(M_buf16);
    }
    else //Grabar tal cual (imagen origen esta en 8 bit o esta en 16 bit y se quiere dejar en 16 bit)
    {
        res = graba_imagen_campo(nombre,buffer, nBits, dEscala);
    }

    return res;
}

/**************************  carga_imagen_campo  ****************************
	Función para efectuar la carga desde disco de una imagen del campo.
	Se le pasan a esta función los siguientes argumentos:
		nombre: nombre del fichero que quiero cargar.
		buffer: le paso el buffer donde quiero almacenar la imagen a cargar.
				Esto define el tamaño del mismo.
	Esta función únicamente carga la imagen en el buffer seleccionado, no la
	muestra en el display.
****************************************************************************/
int carga_imagen_campo(char *nombre, MIL_ID buffer)
{
	FILE *fich;
#if !defined (__BORLANDC__)
	char  mensaje[1024];
    char *fn = "carga_imagen_campo";
#endif

	/*	Compruebo que la imagen solicitada (el archivo) existe. */

	fich = fopen(nombre, "rb");
	if (fich == NULL)  {
#if !defined (__BORLANDC__)
		sprintf(mensaje, "No puedo leer la imagen %s", nombre);
		error_leve(fn, mensaje);
#endif
		return -1;
	}
	fclose(fich);

	//	Si el archivo existe, lo importo, almacenándolo en el buffer solicitado.
	MbufImport(nombre, M_DEFAULT, M_LOAD, M_NULL, &buffer);

	return  0;
}

/**************************  carga_imagen_campo  ****************************
	Función para efectuar la carga desde disco de una imagen del campo.
	Se le pasan a esta función los siguientes argumentos:
		nombre: nombre del fichero que quiero cargar.
		buffer: le paso el buffer donde quiero almacenar la imagen a cargar.
				Esto define el tamaño del mismo.
	Esta función únicamente carga la imagen en el buffer seleccionado, no la
	muestra en el display.
   LA IMAGEN ESTA GUARDADA EN 8 BIT Y EL BUFFER ESTA EN 12 BIT
int carga_imagen_campo8bit(char *nombre, MIL_ID buffer, int nProfundidad)
{
    if (nProfundidad == 16)
    {
        MIL_ID M_buf8;
        long altoImagen,anchoImagen;
        MbufInquire(buffer, M_SIZE_X, &anchoImagen);
        MbufInquire(buffer, M_SIZE_Y, &altoImagen);
	    MbufAlloc2d(M_sistema, anchoImagen, altoImagen, 8+M_UNSIGNED,
		    M_IMAGE+M_DISP+M_PROC, &M_buf8 );

        if (carga_imagen_campo(nombre,M_buf8))
            //ERROR
            return 1;

        MIL_ID M_buf16;
	    MbufAlloc2d(M_sistema, anchoImagen, altoImagen, 16+M_UNSIGNED,
		    M_IMAGE+M_DISP+M_PROC, &M_buf16 );
	    MbufCopy(M_buf8, M_buf16); 
            MbufFree(M_buf8);
        MimShift(M_buf16,buffer,+8);//conversion a 12 bit 
    }
    else
    {
        if (carga_imagen_campo(nombre,buffer))
            //ERROR
            return 1;
    }

	return  0;
}
****************************************************************************/

/**************************  carga_imagen_campo  ****************************
	Función para efectuar la carga desde disco de una imagen del campo.
	Se le pasan a esta función los siguientes argumentos:
		nombre: nombre del fichero que quiero cargar.
		buffer: le paso el buffer donde quiero almacenar la imagen a cargar.
				Esto define el tamaño del mismo.
        nProfundidad: profundidad (bits) del buffer (8,12,16)
	Esta función únicamente carga la imagen en el buffer seleccionado, no la
	muestra en el display.
    LEE LA PROFUNDIDAD GUARDADA, EL BUFFER ESTA EN nProfundidad
****************************************************************************/
int carga_imagen_campo_bits(char *nombre, MIL_ID buffer, int nProfundidad)
{
    // Leer informacion TIF (escala reflectancia y profundidad)
    int nProfundidadFichero = -1;
    double dEscalaFichero;
    try
    {
        CString csNombreImagen;
        INTERFAZ_LIBTIF::LeerTags(nombre,nProfundidadFichero,dEscalaFichero); //Leemos de la imagen la profundidad en bits y la escala reflectancia-gris
        if (nProfundidadFichero < 0 )
            nProfundidadFichero = 8; // por defecto las imagenes estan en 8 bit
    }
    catch(...)
    {
        nProfundidadFichero = 8;
    }

    if (nProfundidad != nProfundidadFichero) 
    {
        // Buffer auxiliar de 16 es necesario para poder operar (M_PROC)
        MIL_ID M_buf16;
        long altoImagen,anchoImagen;
        MbufInquire(buffer, M_SIZE_X, &anchoImagen);
        MbufInquire(buffer, M_SIZE_Y, &altoImagen);
	    MbufAlloc2d(M_sistema, anchoImagen, altoImagen, 16+M_UNSIGNED,
		    M_IMAGE+M_DISP+M_PROC, &M_buf16 );
        if (nProfundidad >= 12 && nProfundidadFichero >= 12) 
        {
            if (carga_imagen_campo(nombre,M_buf16))
                return 1;//ERROR
            MimShift(M_buf16,buffer,nProfundidad - nProfundidadFichero);//conversion de 12 a 16 bit 
        }
        else if (nProfundidadFichero == 8) //nProfundidad>8
        {
            MIL_ID M_buf8;
	        MbufAlloc2d(M_sistema, anchoImagen, altoImagen, 8+M_UNSIGNED,
		        M_IMAGE+M_DISP+M_PROC, &M_buf8 );

            if (carga_imagen_campo(nombre,M_buf8))
                //ERROR
                return 1;

	        MbufCopy(M_buf8, M_buf16); 
            MbufFree(M_buf8);
            MimShift(M_buf16,buffer,nProfundidad - nProfundidadFichero);//conversion de 8 a 12 bit 
        }
        else //(nProfundidad == 8 && nProfundidadFichero > 8
        {
            if (carga_imagen_campo(nombre,M_buf16))
                //ERROR
                return 1;

            MimShift(M_buf16,buffer,nProfundidad - nProfundidadFichero);//conversion de 8 a 12 bit 
        }
        MbufFree(M_buf16);
    }
    else
    {
        if (carga_imagen_campo(nombre,buffer))
            //ERROR
            return 1;
    }

	return  0;
}

#if !defined (__BORLANDC__)
/**************************  imprime_histo  **********************************
   Función para imprimir un histograma por un canal de salida.
*****************************************************************************/
int imprime_histo(FILE *canal, long *histo, int numval, long total,
		int diferencial)
{
	int  i, maxdif;

	if ( canal == NULL )
		return -1;

	maxdif = numval/2 - 1;              // Calculamos la máxima diferencia.

	for (i=0; i < numval; i++)  {
		if ((i % 10) == 0)
			fprintf(canal, "\n%5d> ", (diferencial)? i - maxdif : i);
		if (diferencial && i == numval-1)
			fprintf(canal, "\nResto> %7lu", histo[i]);
		else
			fprintf(canal, "%7lu", histo[i]);
	}
	fprintf(canal, "     Total de pixels: %lu\n", total);

	return  0;
}
#endif


/**************************  graba_fichero_histo  ****************************
   Función para pasar un vector que contiene un histograma a un fichero.
*****************************************************************************/
graba_fichero_histo(FILE *fich, long *histo, int numval, long maxval,
   int diferencial)
{
	int  i, maxdif;

	if ( fich == NULL )
		return -1;

	/*	En la primera línea grabamos la longitud del histograma
		(el número de valores) y el valor máximo alcanzado. */

	fprintf(fich, "%5d %7ld\n", numval, maxval);

	maxdif = numval/2 - 1;              // Calculamos la máxima diferencia.
	for (i=0; i < numval; i++)          // Grabamos los "numval" valores.
		fprintf(fich, "%5d %7ld\n", (diferencial)? i - maxdif : i, histo[i]);

	return  0;
}


/**************************  lee_fichero_histo  ******************************
   Función para pasar a un vector un histograma contenido en un fichero.
*****************************************************************************/
int lee_fichero_histo(FILE *fich, long **histo, int *numval, long *maxval,
		long *total)
{
	char  linea[80];
	int  i, k;
	long  *hist;

	if (fgets(linea, sizeof(linea), fich) == NULL)	// Leemos la primera línea.
		return -1;									// Extraemos la longitud del
	sscanf(linea, "%d %ld", numval, maxval);		// histograma y el valor máximo.

	if ( (hist = *histo = (long *) malloc(sizeof(long) * *numval)) == NULL ) {
#if !defined (__BORLANDC__)
        sprintf(linea, "No hay memoria para un histograma de %d valores", *numval);
        error_leve("lee_fichero_histo", linea);
#endif
		return  -1;
	}

	for (*total=0, i=0; i < *numval; i++)  {
		fgets(linea, sizeof(linea), fich);			// Al leer el histograma, sólo
		sscanf(linea, "%d %ld", &k, &hist[i]);		// conservamos las frecuencias.
		*total += hist[i];
	}

	return  0;
}


