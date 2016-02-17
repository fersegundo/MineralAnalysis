// GestionMinerales.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "GestionMinerales.h"
#include "GestionMineralesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGestionMineralesApp

BEGIN_MESSAGE_MAP(CGestionMineralesApp, CWinApp)
	//{{AFX_MSG_MAP(CGestionMineralesApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGestionMineralesApp construction

CGestionMineralesApp::CGestionMineralesApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGestionMineralesApp object

CGestionMineralesApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CGestionMineralesApp initialization

BOOL CGestionMineralesApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

    m_log = fopen("log_ClasificaBarrido.txt","wt");

#ifndef _DEBUG
    try
    {
#endif
        if (m_configuracion.Cargar() == false)
        {
            AfxMessageBox("No se ha podido cargar la configuración");
            fclose(m_log);
            return FALSE;
        }

        // cargar nombres y abreviaturas de minerales
        if (m_minerales.CargarMinerales(m_configuracion.m_csFicheroMinerales) == false)
        {
            AfxMessageBox("No se ha podido cargar minerales " + m_configuracion.m_csFicheroMinerales);
            fclose(m_log);
            return FALSE;
        }


#ifndef _DEBUG
    } 
    catch (CMemoryException* e)
    {
        // Probablemente se trate de una excepcion de memoria porque hay demasiados datos de pixels que cargar
        AfxMessageBox("Excepcion de memoria al cargar minerales, es posible que el numero de datos a cargar sea excesivo para la memoria disponible");
        e->Delete();
        fclose(m_log);
        return FALSE;
    }
    catch (...)
    {
        // Probablemente se trate de una excepcion de memoria porque hay demasiados datos de pixels que cargar
        AfxMessageBox("Excepcion general al cargar minerales");
        fclose(m_log);
        return FALSE;
    }
#endif

	CGestionMineralesDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
        //Guardamos minerales.dat
        m_minerales.GuardarMinerales(m_configuracion.m_csFicheroMinerales);
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
