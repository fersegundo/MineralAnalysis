/*****************************************************************************
	control_barrido.cpp		Rutinas para control del proceso de barrido

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Septiembre 2001
*****************************************************************************/

#include "stdafx.h"

#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#include <math.h>

#include <mil.h>

#include "..\include\procesoAux.h"

#include "control_barrido.h"
#include "..\ControlDisp\control_camara.h"
#include "..\ControlRevolver\control_revolver_dta.h"
#include "calibracion.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/**************************************************************************
	Función para capturar y corregir las imágenes necesarias con el filtro
	correspondiente a las bandas definidas en los parámetros iniciales.
	Las imagenes se dejan en M_banda[banda]

    dEscalaReflectanciaSalida: escala de reflectancia-nivel de gris que se
    quiere dar a las imagenes adquiridas. Si negativo, las imagenes adquiridas
    se dejaran en la escala "de aquisicion", es decir en la escala de los patrones
****************************************************************************/
int  captura_bandas(parametros& param,int nImagenAcum, DtaRevolver& Rueda, double dEscalaReflectanciaSalida, bool bVolverAObservacion)
{
	int  banda, filtro_ant;
#if !defined (__BORLANDC__)
	CString  nomFiltro;
#else
	AnsiString  nomFiltro;
#endif

	/*	Nos aseguramos de que el filtro sea el correspondiente a la banda
		que deseamos seleccionar. */
	filtro_ant = Rueda.GetFilter();
	for (banda=0; banda < param.nBandas; banda++)  
    {
        if (captura_banda(banda,param, nImagenAcum,Rueda,filtro_ant, dEscalaReflectanciaSalida ) == -1)
            return -1;
	}

	/*	Una vez capturadas las imágenes, vuelvo a la posición de enfoque (filtro predeterminado). */
    if (bVolverAObservacion)
	    if ( filtro_ant != param.Rueda.posFiltro )  {
		    if (Rueda.ChangeFilter(param.Rueda.posFiltro) == false)
			    return -1;
	}

	return 0;
}

/**************************  captura_banda  *******************************
	Función para capturar y corregir una imagen con el filtro
	correspondiente a la banda definidas en los parámetros iniciales.
	Las imagenes se dejan en M_banda[banda]
    filtro_ant - es el numero de filtro anterior (para comprobar si es necesario cambiar). Se actualiza
                 Si -1, no se realiza la comprobacion y se cambia de todas formas
    dEscalaReflectanciaSalida: escala de reflectancia-nivel de gris que se
                                quiere dar a la imagen adquiridas. Si negativo, la imagen adquirida
                                se dejara en la escala "de aquisicion", es decir en la escala de los patrones
****************************************************************************/
int  captura_banda(int nBanda, parametros& param, int nImagenesAcum, DtaRevolver& Rueda, int& filtro_ant, double dEscalaReflectanciaSalida)
{
    int nFiltro = abs(param.filtro[nBanda]);
	if ( filtro_ant == -1 || filtro_ant != nFiltro )  
    {
		if (Rueda.ChangeFilter(nFiltro)==false)
			return -1;
        toma2_unica(M_imagen1); //nos aseguramos que el cambio de exposicion se haya hecho efectivo
        toma2_unica(M_imagen1); //nos aseguramos que el cambio de exposicion se haya hecho efectivo
	}
	toma2_acumulada(M_imagen_acum, nImagenesAcum); // imagen acumulada en M_imagen_acum (fin_toma implicito)
    toma2_continua(M_imagen1);	// Adquisición en modo continuo.

    // En M_imagen_acum tenemos 8bit o 16 bit acumulados y no promediados
    // Promediar convenientemente hasta conseguir 8bit o 16bit y dejar en M_imagen1

    //Factor por el que hay que multiplicar una unica toma (sin acumular) para alcanzar
    //la profundidad deseada. Si 8bit -> 1. Si 12bit-> 16
    ASSERT(param.Cam.profundidad >= BITS_CAMARA);
    int factorSinAcum = 1 << (param.Cam.profundidad - BITS_CAMARA); //Si 8bit -> 1. Si 12bit-> 16
    // Factor por el que hay que dividir la imagen acumulada para alcanzar la profundidad deseada
    int factorDivision = nImagenesAcum;

    // M_imagen_acum -> promediado -> M_imagen1
	MimArith(M_imagen_acum, factorSinAcum/factorDivision, M_imagen_acum, M_MULT_CONST); //para que el resultado quede en 16 bit y no 12
	//MimArith(M_imagen_acum, factorDivision, M_imagen_acum, M_DIV_CONST); //promediado

	corrige_iluminacion(nBanda, M_imagen_acum); //M_imagen_acum -> M_banda[banda]


    // aplicamos la escala de salida deseada
    if (dEscalaReflectanciaSalida>0 && dEscalaReflectanciaSalida != param.escala)
    {
        //MbufCopy(M_banda[nBanda],M_correc_aux); //para realizar operacion en coma flotante
        MimArith(M_banda[nBanda], param.escala / dEscalaReflectanciaSalida, M_correc_aux, M_MULT_CONST);
	    MimClip(M_correc_aux, M_banda[nBanda], M_OUT_RANGE, 0, float(pow(2,16)-1), 0, float(pow(2,16)-1)); // Para que si hay saturacion, se quede blanco (30 / 7 / 2009)
        //MbufCopy(M_correc_aux, M_banda[nBanda]);
    }

    filtro_ant = nFiltro;

	return 0;
}
