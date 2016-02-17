/*****************************************************************************
	gestion_mensajes.cpp	Programa para presentar y registrar mensajes
							Este m�dulo incluye tanto los mensajes que interactuan
							con el operario, como los mensajes de error.
	Autores:	JCC, BLM
	Copyright:	� AITEMIN Septiembre 2001
*****************************************************************************/

#include "stdafx.h"

#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#include <stdio.h>
#if !defined (__BORLANDC__)
#include <stdlib.h>
#endif

#include "gestion_mensajes.h"

#if !defined (__BORLANDC__)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

#if defined (__BORLANDC__)
VER_ERRORES MuestraMensaje;
#endif

#if !defined (__BORLANDC__)
int modo;
#endif

extern char fichLog[LONGITUD_TEXTO];



/**************************  sendlog  ***************************************
   Funci�n para enviar un mensaje al fichero de log's de la aplicaci�n.
*****************************************************************************/
int sendlog(const char *funcion, const char *mensaje)
{
	FILE *fich;

	if ( (fich = fopen(fichLog, "at")) == NULL )  
    {
        #ifdef _DEBUG
		    error_fatal("sendlog", "No se encuentra el fichero", 0);
        #endif
		return -1;
	}

	fprintf(fich, "%s: %s\n", funcion, mensaje);

	fclose(fich);

	return 0;
}

/**************************  sendlog  ***************************************
   Funci�n para enviar un mensaje al fichero de log's de la aplicaci�n.
   Incluye la hora de escritura
*****************************************************************************/
int sendlog_timestamp(const char *funcion, const char *mensaje)
{
	FILE *fich;
    
	if ( (fich = fopen(fichLog, "at")) == NULL )  {
		error_fatal("sendlog - No se puede abrir el fichero: ", fichLog, 0);
		return -1;
	}

    //timestamp
    SYSTEMTIME systime;
    GetSystemTime(&systime);

    CString str;
    str.Format(_T("%u/%u/%u  %u:%u:%u:%u"), 
       systime.wYear, systime.wMonth, systime.wDay,
       systime.wHour, systime.wMinute, systime.wSecond,
       systime.wMilliseconds);

	fprintf(fich, "%s - %s: %s\n", (LPCTSTR)str, funcion, mensaje);

	fclose(fich);

	return 0;
}

/**************************  sendlog  ***************************************
   Funci�n para enviar un mensaje al fichero de log's de la aplicaci�n.
   Incluye la hora de escritura
   Version especificando fichero
*****************************************************************************/
int sendlog_timestamp(FILE* fich, const char *mensaje)
{
	if ( fich == NULL )  
    {
		sendlog_timestamp("", mensaje);
	}

    //timestamp
    SYSTEMTIME systime;
    GetSystemTime(&systime);

    CString str;
    str.Format(_T("%u/%u/%u  %u:%u:%u:%u"), 
       systime.wYear, systime.wMonth, systime.wDay,
       systime.wHour, systime.wMinute, systime.wSecond,
       systime.wMilliseconds);

	fprintf(fich, "%s: %s\n", (LPCTSTR)str, mensaje);

	return 0;
}


#if defined (__BORLANDC__)
#pragma argsused
#endif
/**************************  error_fatal  ************************************
   Funci�n para imprimir un mensaje de error y terminar la
   ejecuci�n del programa.
*****************************************************************************/
int error_fatal(const char *funcion,const char *mensaje, int terminar)
{
  static  char  mensaje2[2048];

#if !defined (__BORLANDC__)
    #if _AFXDLL //entorno visual de windows MFC
        sprintf(mensaje2, "***  %s:  %s  ***", funcion, mensaje);
        AfxMessageBox(mensaje2); 
    #else
	    CharToOemBuff(mensaje, mensaje2, sizeof(mensaje2));
	    if (*funcion)
		    fprintf(stderr, "\n***  %s:  %s  ***\n", funcion, mensaje2);
	    else
		    fprintf(stderr, "\n***  %s  ***\n", mensaje2);

	    // si: terminar = true => liberamos la memoria ocupada y terminamos la ejecuci�n
	    if ( terminar ) {
		    //fin_control_proceso_imagenes();
		    exit(-1);
	    }
    #endif
#else
  if (funcion) {
    Application->MessageBox(mensaje, funcion, MB_OK | MB_ICONEXCLAMATION);
    sprintf(mensaje2, "***  %s:  %s  ***", funcion, mensaje);
  } else {
    Application->MessageBox(mensaje, "Error fatal",
                            MB_OK | MB_ICONEXCLAMATION);
    sprintf(mensaje2, "***  %s  ***", mensaje);
  }
  MuestraMensaje(mensaje2);
#endif
  return 0;
}

/**************************  error_leve  *************************************
   Funci�n para imprimir un mensaje de error y devolver
   el c�digo de "error en la funci�n" (valor -1).
*****************************************************************************/
void  error_leve(const char *funcion,const char *mensaje)
{
	static  char  mensaje2[2048];

    #if _AFXDLL //entorno visual de windows MFC
        sendlog(funcion,mensaje);
    #else
	    if (*funcion) {
		    CharToOemBuff(mensaje, mensaje2, sizeof(mensaje2));
		    fprintf(stderr, "\n***  %s:  %s  ***\n", funcion, mensaje2);
	    }
	    else {
		    CharToOemBuff(mensaje, mensaje2, sizeof(mensaje2));
		    fprintf(stderr, "\n***  %s  ***\n", mensaje2);
	    }
    #endif
}


#if !defined (__BORLANDC__)
/**************************  act_modo  **************************************
	Funci�n para actualizar el modo en el que se esta procesando la serie.
*****************************************************************************/
void  act_modo(char tecla)
{
	switch(tecla)  {
		case 'c': case'C':
			modo = CONTINUO;
			break;
		case 'p': case 'P':
			modo = PARADAS;
			break;
	}
}
#endif

