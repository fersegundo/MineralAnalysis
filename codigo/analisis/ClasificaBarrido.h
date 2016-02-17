// ClasificaBarrido.h : main header file for the CLASIFICABARRIDO application
//

#if !defined(AFX_CLASIFICABARRIDO_H__7D77F31F_F534_4177_A9AF_56E468D8E2EB__INCLUDED_)
#define AFX_CLASIFICABARRIDO_H__7D77F31F_F534_4177_A9AF_56E468D8E2EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resourceClasificaBarrido.h"		// main symbols
#include "ConfiguracionClasificaBarrido.h"
#include "Minerales.h"
#include "AsociacionesMinerales.h"
#include "ClasificacionBarrido.h"

/////////////////////////////////////////////////////////////////////////////
// CClasificaBarridoApp:
// See ClasificaBarrido.cpp for the implementation of this class
//

class CClasificaBarridoApp : public CWinApp
{
public:
	CClasificaBarridoApp();
    FILE*                   m_log;
    CConfiguracion          m_configuracion;    // Configuracion para esta aplicacion
    CMinerales              m_minerales; //nombres y abreviaturas de minerales
    ClasificacionBarrido    m_clasificacionBarrido;
    CAsociacionesMinerales  m_asociaciones;
    double                  m_dEscalaReflectancia; // Escala de reflectancia-gris en la que están guardadas las imagenes que se estan procesando actualmente
    int                     m_nBitsProfundidad;    // Profundidad en bits de las imagenes que se estan procesando actualmente
    
    // datos que se leen del fichero de informacion de barrido
    bool m_bBarrido2d;
    int m_nCampos_x;
    int m_nCampos_y;
    int m_nAnchoImagen;
    int m_nAltoImagen;
    int m_nBandas;
    int m_arrFiltros[MAX_NUM_IMAGENES];

    void GetAppPath(LPTSTR pszPath);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClasificaBarridoApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CClasificaBarridoApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLASIFICABARRIDO_H__7D77F31F_F534_4177_A9AF_56E468D8E2EB__INCLUDED_)
