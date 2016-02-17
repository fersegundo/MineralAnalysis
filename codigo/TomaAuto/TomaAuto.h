// TomaAuto.h : main header file for the TOMAAUTO application
//

#if !defined(AFX_TOMAAUTO_H__048478CB_B527_479C_A007_F014E0C2F261__INCLUDED_)
#define AFX_TOMAAUTO_H__048478CB_B527_479C_A007_F014E0C2F261__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "Configuracion.h"
#include "..\librerias\include\procesoAux.h"
#include "..\librerias\ControlRevolver\control_revolver_dta.h"
#include "..\librerias\LibBarrido\CorreccionGeometrica.h"

/////////////////////////////////////////////////////////////////////////////
// CTomaAutoApp:
// See TomaAuto.cpp for the implementation of this class
//

class CTomaAutoApp : public CWinApp
{
private:
    bool Init();
    static UINT PreparaBarridoThread( LPVOID Param );
    int CTomaAutoApp::lee_param(parametros *paramAdq);
    void CTomaAutoApp::HayCorreccion(bool bHayCorreccion);

public:
    parametros	m_ParamIni;				// Parámetros iniciales de barrido
    CConfiguracion  m_configuracion;    // Configuracion para esta aplicacion
    CString csNombreToma;               // nombre del barrido

    DtaRevolver Rueda;           // Objeto revolver para el control de la rueda de filtros
    CCorreccionGeometrica m_CorreccionGeometrica; // correcion geometrica

public:
	CTomaAutoApp();
    void GetAppPath(LPTSTR pszPath);

    int CalcularAnchoMax(bool bRotacion = true);
    int CalcularAltoMax(bool bRotacion = true);
    void ActualizarCorreccionGeometrica();
    void CorregirImagenes(bool bCorregirRotacion = true);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTomaAutoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTomaAutoApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOMAAUTO_H__048478CB_B527_479C_A007_F014E0C2F261__INCLUDED_)
