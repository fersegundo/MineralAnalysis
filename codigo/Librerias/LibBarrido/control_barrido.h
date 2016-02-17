/*
	control_barrido.h		Declaraciones para control_barrido.cpp

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Septiembre 2001
*/

#ifndef CONTROL_BARRIDO_H
#define CONTROL_BARRIDO_H

#include "..\ProcesoAux\estructuras.h"


class DtaRevolver;


int  captura_bandas(parametros& param, int nImagenAcum, DtaRevolver& Rueda, double dEscalaReflectanciaSalida = -1, bool bVolverAObservacion = true);
int  captura_banda(int nBanda, parametros& param, int nImagenAcum, DtaRevolver& Rueda, int& filtro_ant, double dEscalaReflectanciaSalida = -1);

#endif // CONTROL_BARRIDO_H

