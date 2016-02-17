/*
	libAnalisis.h		Declaración de las funciones contenidas en la biblioteca
						libAnalisis.h
		
	Autores:	JCC, BLM
	Copyright:	© AITEMIN Noviembre 2002
*/

#ifndef LIBANALISIS_H
#define LIBANALISIS_H

#include "..\LibAnalisis\analisis_muestra.h"
#if !defined (__BORLANDC__)
#include "..\LibAnalisis\analisis_histo_frecuencia.h"
#include "..\LibAnalisis\analiza_histo_granos.h"
#include "..\LibAnalisis\caracteriza_punto.h"
#endif
#include "..\LibAnalisis\caracteriza_reflec.h"
#if !defined (__BORLANDC__)
#include "..\LibAnalisis\componentesAux.h"
#include "..\LibAnalisis\determina_nComponentes.h"
#include "..\LibAnalisis\etiqueta_granos.h"
#include "..\LibAnalisis\evalua_gausianas.h"
#include "..\LibAnalisis\extrae_granos.h"
#endif
#include "..\LibAnalisis\macerales.h"
#if !defined (__BORLANDC__)
#include "..\LibAnalisis\macerales2.h"
#endif
#include "..\LibAnalisis\memoria_analisis.h"
#include "..\LibAnalisis\leeParamAnalisis.h"

//#include "..\LibAnalisis\generaHTM.h"
//#include "..\LibAnalisis\GeneraResulVitri.h"

#endif // LIBANALISIS_H

