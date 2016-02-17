
#if !defined(BARRIDO_H_)
#define BARRIDO_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "defines.h"
#include "..\librerias\LibBarrido\CorreccionGeometrica.h"

#define NUM_PUNTOS_PLANO 9

class CFormTomaAuto;
class CBarrido 
{
private:
    point   m_pActual;              //Posicion actual. Actualizado periodicamente mientras se define el barrido
    bool    m_bBarridoDefinido;     //true si el inicio y fin del barrido han sido definidos
    int     m_nCampos_x;            // numero de campos del barrido
    int     m_nCampos_y;
    plane   m_plano;
    double  m_dDimensionReal_x;       // micras entre campos (sumando lo que ocupa el propio campo (reescalado) y la holgura entre campos) de las imagenes finales 
    double  m_dDimensionReal_y;       
    bool    m_bCancelarBarrido;    // para poder cancelar el barrido

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
	CBarrido();
	virtual ~CBarrido();
    bool SetInicio(point& pActual);
    bool SetFin(point& pActual);
    void Intermedios();
    int EnfoquePuntosIntermedios(int& nIndicePuntoEnfocado);
    bool Barrer(CFormTomaAuto* pDialogo);
    void Cancelar();
    void CalcularCampos(int nCobertura, double dPaso_x, double dPaso_y, int& nCampos_x, int& nCampos_y, double& dAnchoTotal, double& dAltoTotal);
    void ActualizaPosicion(point& pActual);
    void GenerarMosaicos(CFormTomaAuto* pDialogo);
    int GetCamposX();
    int GetCamposY();
private:
    // funcion para ejecutar el thread
    static UINT BarrerThread (LPVOID param);
    void InitPuntos();

};

#endif 
