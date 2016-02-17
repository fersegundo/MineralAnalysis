// DivideDatos.h: interface for the 
//
//////////////////////////////////////////////////////////////////////

#if !defined(DIVIDE_PRUEBAS_H_)
#define DIVIDE_PRUEBAS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Analisis\defines.h" //NUM_BANDAS
#include "..\Analisis\minerales.h"

class CDivideDatos 
{
public:
    int   m_nNumBandas;
    CMinerales m_minerales;
    FILE* m_log;
private:
    int m_total_entrenamiento ;
    int m_total_pruebas ;

public:
    CDivideDatos()
    {
        m_log = fopen("log_divide_pruebas.txt","wt");
        m_nNumBandas = -1;
    }
    virtual ~CDivideDatos()
    {
        fclose(m_log);
    }
    void SepararAleatoreamentePruebas(int nPorcentajeZonas);
    void GuardaPixels(CString csFicheroEntrenamiento, int nPorcentajeZonas, bool bEntrenamiento);
private:
};

#endif
