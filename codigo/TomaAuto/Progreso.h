
#if !defined(PROGRESO_H_)
#define PROGRESO_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define DIMENSION_PROGRESO 500

#include "defines.h"

//using namespace std;
typedef long MIL_ID;

class CProgreso 
{
private:
    MIL_ID M_Progreso;
    MIL_ID M_Progreso_child;
    MIL_ID M_Display_progreso;
    int m_nDimensionCampoX;
    int m_nDimensionCampoY;

public:
	CProgreso();
	virtual ~CProgreso();
    void Borrar();
    void Inicializar(int nCamposX, int nCamposY,CStatic& m_progreso);
    void Fin();
    void NuevoCampo(int nCampoX, int nCampoY);
private:
    void CalcularRGB(MIL_ID M_rgb);
    void Acumular(unsigned char* pAcumulado, unsigned __int16* pTotal, double dPorcentaje, int nTamBuffers);

};

#endif 
