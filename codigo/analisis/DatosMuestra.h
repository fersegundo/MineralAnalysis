
#if !defined(DATOS_MUESTRA_H_)
#define DATOS_MUESTRA_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "defines.h"

//using namespace std;

class CDatosMuestra 
{
public:
	CDatosMuestra();
	virtual ~CDatosMuestra();
    bool Generar(CString csMuestra, int nCampoActual);
};

#endif
