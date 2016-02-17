// Analisis.h : main header file for the ANALISIS application
//

#if !defined(AFX_ANALISIS_H__C9172AB9_C62A_45C0_B8FC_E5CF2ED1AD08__INCLUDED_)
#define AFX_ANALISIS_H__C9172AB9_C62A_45C0_B8FC_E5CF2ED1AD08__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "PGL.h" //PlotGraphicLibrary
#include "resource.h"		// main symbols
#include "Clasificacion.h"
#include "Configuracion.h"
#include "Minerales.h"
#include "AsociacionesMinerales.h"
#include "..\librerias\include\procesoAux.h"


/////////////////////////////////////////////////////////////////////////////
// CAnalisisApp:
// See Analisis.cpp for the implementation of this class
//

class CAnalisisApp : public CWinApp
{
public:
    parametros	            m_ParamIni;				// Parámetros iniciales de barrido
    CMinerales              m_minerales; //nombres y abreviaturas de minerales
    CAsociacionesMinerales  m_asociaciones;
    Clasificacion           clasificacion;
    CConfiguracion          m_configuracion;    // Configuracion para esta aplicacion
    double                  m_dEscalaReflectancia; // Escala de reflectancia-gris en la que están guardadas las imagenes que se estan procesando actualmente
    int                     m_nBitsProfundidad;    // Profundidad en bits de las imagenes que se estan procesando actualmente

    CAnalisisApp();

    void GetAppPath(LPTSTR pszPath);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnalisisApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CAnalisisApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    int CAnalisisApp::lee_param(parametros *paramAdq);

};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANALISIS_H__C9172AB9_C62A_45C0_B8FC_E5CF2ED1AD08__INCLUDED_)
