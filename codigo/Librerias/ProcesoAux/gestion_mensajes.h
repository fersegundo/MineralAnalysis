/*
	gestion_mensajes.h: Declaración de las funciones en gestion_mensajes.cpp.
*/


#ifndef GESERROR_H
#define GESERROR_H

#if !defined (__BORLANDC__)
#define CONTINUO		   0	// Modo de procesar la separación y extracción de granos.
#define PARADAS			   1
#endif

#ifndef LONGITUD_TEXTO
    #define LONGITUD_TEXTO	  512
#endif

#if defined (__BORLANDC__)
typedef void (* VER_ERRORES)(char *);

extern VER_ERRORES MuestraMensaje;
#endif

int sendlog(const char *fn, const char *mensaje);
int sendlog_timestamp(const char *funcion, const char *mensaje);
int sendlog_timestamp(FILE* fich, const char *mensaje);

int	error_fatal(const char *funcion,const  char *mensaje, int terminar);
void error_leve(const char *funcion,const char *mensaje);

#if !defined (__BORLANDC__)
void	act_modo(char tecla);
#endif

#endif //GESERROR_H

