/*
	control_revolver_dta.h

	Control especifico para las aplicaciones del proyecto AMAMET del revolver "RPF MAX 16" del fabricante DTA

	Autores:	FSG
	Copyright:	© AITEMIN  2007
*/

#ifndef CONTROL_REVOLVER_DTA_H
#define CONTROL_REVOLVER_DTA_H

#include "revolver_dta.h"
#include "..\ProcesoAux\estructuras.h" //para parametros

class CTomaAutoDlg;
class DtaRevolver
{
//CLASS ATTRIBUTES
private:
	Revolver					revolver;
    parametros*                 m_pParam;
    CTomaAutoDlg*               m_pDialogo;
public:
    double      zObservacion; //z absoluta del filtro de observacion (necesaria para cambiar la z al cambiar de filtro)
// CLASS METHODS
public:
	DtaRevolver();
	~DtaRevolver();
	void Init(parametros* pParam);
    void SetDialogo(CTomaAutoDlg* pDialogo);
    bool ChangeFilter(int nFilter);
	int GetFilter();

private:

};

#endif // CONTROL_REVOLVER_DTA_H

