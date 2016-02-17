/*
	control_camara.h		Declaraciones para control_camara.cpp

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Septiembre 2001
*/

#ifndef CONTROL_CAMARA_H
#define CONTROL_CAMARA_H

#define TIEMPO_AJUSTE_EXPOSICION 100

/*  Declaración de funciones incluidas en control_camara.cpp */

typedef long MIL_ID;

void  toma2_unica(MIL_ID fb);
void  toma2_continua(MIL_ID fb);
void  fin_toma();
void  toma2_acumulada(MIL_ID buf, int nAcum);
void  toma2_promediada(int nAcum);
void  ModificaExposicion(int exposicion);
void  ModificaExposicionSleep(int exposicion);
bool  ModificaBaseExposicion(int nBaseExposicion);
bool  GetLimitesBaseExposicion(long& nMin, long&nMax);

#endif // CONTROL_CAMARA_H

