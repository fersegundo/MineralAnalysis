/*
	adquisicion_imagenPat.h
*/

#ifndef ADQUISICION_IMAGENPAT_H
#define ADQUISICION_IMAGENPAT_H

#if !defined (__BORLANDC__)
#include <mil.h>
#endif



#if !defined (__BORLANDC__)
int adquierePat(parametros *param, char *tipoPat, bool bExposicionAjustada);
#endif

int haz_movimiento(parametros *param, ppoint inicio, ppoint final);

//int lee_paramTomaPat(char *raiz, int *LutColor, bool *aLamp);
int ini_TomaPatrones(parametros *param);
int fin_TomaPatrones(int num_bandas);

#endif // ADQUISICION_IMAGENPAT_H

