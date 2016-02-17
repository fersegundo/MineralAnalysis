#include "stdafx.h"

#include "control_revolver_dta.h"
#include "..\..\TomaAuto\TomaAutoDlg.h"
#include "..\ControlDisp\control_camara.h" //para cambiar la exposicion segun el filtro
#include "..\ControlMicroscopio\control_microscopio.h" //para cambiar la z segun el filtro
#include "..\LibBarrido\control_barrido.h" //para BACKSLASHZ
#include "..\ProcesoAux\PLNRGR_P.h"

#include <mil.h> //HACER BORRAR solo para MappControl (que hay que quitar) ... tambien quitar el path mil en project settings de este proyecto

#if !defined (__BORLANDC__)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

extern MIL_ID	M_imagen1;				// M_imagen: tamaño normal

DtaRevolver::DtaRevolver()
{
    m_pParam = NULL;
    m_pDialogo = NULL;
}

DtaRevolver::~DtaRevolver()
{
}

void DtaRevolver::Init(parametros* pParam)
{
	/*	Nos desplazamos a la ultima banda antes de posicionarnos en el filtro predefinido
		Esto se hace para asegurarnos que la rueda cambia en la misma direccion que cuando
	    cambie al hacer el barriro. Puede ser que si cambiase en diferente direccion, la posicion
	    del filtro no fuese exactamente la misma y por tanto alguna mota de polvo nos afectase */
        m_pParam = pParam;

        zObservacion = mspWhere(Z_);
		revolver.ChangeFilter(MAX_FILTROS_DTA-1);
		revolver.ChangeFilter(m_pParam->Rueda.posFiltro);
}

void DtaRevolver::SetDialogo(CTomaAutoDlg* pDialogo)
{
    m_pDialogo = pDialogo;
}

// Posiciona la rueda en el filtro 'nFilter' [0..MAX_FILTROS_DTA]
// Devuelve true cuando se ha completado satisfactoriamente el posicionamiento. Si nFilter esta 
// fuera de rango, hay un problema de hardware o la recalibracion no se hizo, se devuelve false
bool DtaRevolver::ChangeFilter(int nFilter)
{
	bool	bRet = true;

	int posFiltro_old = revolver.GetFilter();

    if (!revolver.ChangeFilter(nFilter))
        return false;

	//modificar la exposicion correspondiente al filtro seleccionado
	ModificaExposicionSleep(m_pParam->Rueda.exposicion[nFilter]);

    // En caso necesario, modificar la base de exposicion para este nuevo filtro
    int nNuevaBaseExp = m_pParam->Rueda.base_exp[nFilter];
    if (m_pParam->Rueda.base_exp[posFiltro_old] != nNuevaBaseExp)
	    ModificaBaseExposicion(nNuevaBaseExp);

// HACER PRUEBAS, esto se hace para que no de error MdigGrab TIMEOUT con filtro 15
// deshabilitar errores una sola vez en la inicializacion de las aplicaciones (o en algun sitio comun)
if (m_pParam->Rueda.base_exp[nFilter] > 200)
    MappControl(M_ERROR,M_PRINT_DISABLE);
else
    MappControl(M_ERROR,M_PRINT_ENABLE);


    // En caso necesario, corrijo la z para este nuevo filtro
    if (m_pParam->Rueda.diferencia_z[posFiltro_old] != m_pParam->Rueda.diferencia_z[nFilter])
    {
        double zFoco = zObservacion + m_pParam->Rueda.diferencia_z[nFilter] - m_pParam->Rueda.diferencia_z[m_pParam->Rueda.posFiltro] ;
        mspGo(Z_, zFoco + BACKSLASHZ);
        bRet = (mspGoCheck(Z_, zFoco) != -1); // mspGoCheck espera un poco despues de moverse
    }

    //Informo del cambio al dialogo
    if (m_pDialogo != NULL)
        m_pDialogo->MostrarFiltro(nFilter);

	return bRet;
}

// Devuelve el numero de filtro actual [0..MAX_FILTROS_DTA]
int DtaRevolver::GetFilter()
{
	return revolver.GetFilter();
}

