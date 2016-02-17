// EntrenPixels.h: interface for the 
//
//////////////////////////////////////////////////////////////////////

#if !defined(PRUEBA_PIXELS_H_)
#define PRUEBA_PIXELS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\Analisis\defines.h"
#include "..\Analisis\minerales.h"
#include "..\Analisis\matrices.h"

class CPruebaPixels 
{
public:
    FILE* m_log;
    int   m_nNumBandas;
    CMinerales m_minerales;

public:
    CPruebaPixels()
    {
        m_log = fopen("log_entrena_pixels.txt","wt");
        m_nNumBandas = -1;

    }
    virtual ~CPruebaPixels()
    {
        fclose(m_log);
    }
    bool Pruebas(CString csFicheroPruebas,double dUmbralDistancia, double dUmbralConfiabilidad);
private:
};

#endif
