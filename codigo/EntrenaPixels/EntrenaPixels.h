// EntrenPixels.h: interface for the 
//
//////////////////////////////////////////////////////////////////////

#if !defined(ENTRENA_PIXELS_H_)
#define ENTRENA_PIXELS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Analisis\defines.h"
#include "..\Analisis\minerales.h"
#include "..\Analisis\matrices.h"

#define MULT_DESV_TIPICA_TEST_ZONAS 4

class CEntrenaPixels 
{
public:
    FILE* m_log;
    int   m_nNumBandas;
    CMinerales m_minerales;

public:
    CEntrenaPixels()
    {
        m_log = fopen("log_entrena_pixels.txt","wt");
        m_nNumBandas = -1;

    }
    virtual ~CEntrenaPixels()
    {
        fclose(m_log);
    }
    void GuardarDatosEntrenamiento(CString csFicheroSalidaMahalanobis = "");
    void LimitarPixels(int nLimitePixels);
    bool Entrenar();
private:
    void CalcularPromedioZonas(CMineral* pMineral);
};

#endif
