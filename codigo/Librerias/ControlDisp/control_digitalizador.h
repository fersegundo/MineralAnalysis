/*
	control_digitalizador.h		Declaraciones para control_proceso_imagenes.cpp

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Diciembre 2001.
*/

#ifndef control_digitalizador_H
#define control_digitalizador_H

#define PROFUNDIDAD_ACUMULACION	 32	// Profundidad a la que se quiere trabajar a la hora de promediar

/*	Declaración de las funciones contenidas en control_digitalizador.cpp */
int  ini_control_digitalizador(parametros *param);
int  fin_control_digitalizador(void);
int  configura_digitalizador(int formato);
int  info_digitalizador(MIL_ID digitalizador);
int  libera_digitalizador(void);

#endif // control_digitalizador_H

