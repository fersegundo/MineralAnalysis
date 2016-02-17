#ifndef BARRIDO_PATRON_H
#define BARRIDO_PATRON_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "mil.h"
#include "..\Librerias\include\procesoAux.h" //MAX_FILTROS_DTA,
#include "..\Librerias\ControlRevolver\control_revolver_dta.h" // MAX_BANDAS

#define NUM_PUNTOS_PLANO 9

struct PARAM_THREAD;
class CFormPatron;
class CBarridoPatron 
{
private:
    plane   m_plano;
    bool    m_bCancelarBarrido;    // para poder cancelar el barrido

    // factor de multiplicacion de la sigma, que se restará a la moda para calcular el umbral en CalcularMascara. 
    double UMBRAL_MULT_SIGMA[MAX_BANDAS];
    // el nivel de gris que se restará a la moda para calcular el umbral en CalcularMascara. Solo si UMBRAL_MULT_SIGMA es 0
    double UMBRAL_DIF[MAX_BANDAS];

    /*	Vectores de imágenes, para el almacenamiento de los sucesivos
	    campos adquiridos de los patrones. */
    MIL_ID M_banda_acum[MAX_FILTROS_DTA]; //acumulacion de campos validos
    MIL_ID M_banda_enmascarada_acum[MAX_FILTROS_DTA];

    MIL_ID M_cont_campos[MAX_BANDAS]; // Por cada pixel, cuantos campos fueron validos en el barrido. Comun para todas las bandas
    MIL_ID M_mascara_aux; // Mascara auxiliar con 0 y 1 por cada campo que se usará internamente
    MIL_ID M_acum_aux; // Acumulador auxiliar que se usará internamente

    MIL_ID M_histo12;
    MIL_ID M_histo16;

    // Declaración de variables globales declaradas en este módulo
    int campoPat;
public:
    // Los 9 puntos que definen el barrido
    //  -------> (x)
    // | 0 1 2
    // | 3 4 5
    // | 6 7 8
    // \/
    // (y)
    point m_arrPuntosControl[NUM_PUNTOS_PLANO];

public:
	CBarridoPatron();
	virtual ~CBarridoPatron();

    bool CBarridoPatron::SetFin(point& pActual);
    bool CBarridoPatron::SetInicio(point& pActual);
    void CBarridoPatron::Intermedios();
    int CBarridoPatron::EnfoquePuntosIntermedios(int& nIndicePuntoEnfocado);
    bool CBarridoPatron::Barrer(CFormPatron* pDialogo);
    void CBarridoPatron::Cancelar();

    void CBarridoPatron::AjusteAutomaticoExposicion(CFormPatron* pDialogo);
private:
    int ini_TomaPatrones(PARAM_THREAD* paramThread);
    int fin_TomaPatrones(PARAM_THREAD* paramThread);
    void CBarridoPatron::InitPuntos();
    int CBarridoPatron::avanza_y_toma_campo(PARAM_THREAD* paramThread,
                            double& X, double& Y, double& Z, 
                            int i, int j, double saltoX);
    void CBarridoPatron::CalcularMascara(PARAM_THREAD* paramThread, int nBanda);
    int CBarridoPatron::toma_campoPat(PARAM_THREAD* paramThread);

    static UINT CBarridoPatron::BarrerThread( LPVOID Param );
    static UINT CBarridoPatron::AjusteAutomaticoExposicionThread( LPVOID Param );
};

#endif 

