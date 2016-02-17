// ClasificaBarrido.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ClasificaBarrido.h"
#include "ClasificaBarridoDlg.h"
#include "Clasificacion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CClasificaBarridoApp

BEGIN_MESSAGE_MAP(CClasificaBarridoApp, CWinApp)
	//{{AFX_MSG_MAP(CClasificaBarridoApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClasificaBarridoApp construction

CClasificaBarridoApp::CClasificaBarridoApp()
{
    m_nCampos_x     = -1;
    m_nCampos_y     = -1;
    m_bBarrido2d    = true;
    m_nBandas       = -1;
    m_nAnchoImagen  = -1;
    m_nAltoImagen   = -1;
    memset(m_arrFiltros,0,sizeof(int)*MAX_NUM_IMAGENES);
    m_dEscalaReflectancia = -1;
    m_nBitsProfundidad = -1;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CClasificaBarridoApp object

CClasificaBarridoApp theApp;

FILE* g_log; //necesario para uso desde modulo de minerales (codigo compartido por Analisis y Clasificabarrido, que no puede hacer por tanto uso de "theApp")
/////////////////////////////////////////////////////////////////////////////
// CClasificaBarridoApp initialization

BOOL CClasificaBarridoApp::InitInstance()
{
	// Standard initialization

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CoInitializeEx(NULL,COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY);					// Inicialización del servidor OLE, Threads

    m_log = fopen("log_ClasificaBarrido.txt","wt");
    g_log = fopen("log_ClasificaBarrido2.txt","wt");

    // Componer path (donde este el ejecutable)
    char szPath[_MAX_PATH + 1];
    GetAppPath(szPath);
    m_configuracion.m_csPathEjecutable = szPath;

    if (!m_configuracion.Cargar(""))//szPath))
    {
        CString csMensaje;
        csMensaje.Format("No se encuentra el fichero de configuracion");// en %s",szPath);
        AfxMessageBox(csMensaje);
        ASSERT(FALSE);
        return FALSE;
    }


#ifndef _DEBUG
    try
    {
#endif
        // cargar nombres y abreviaturas de minerales
        if (!m_minerales.CargarMinerales(m_configuracion.m_csFicheroMinerales))
        {
            CString csMensaje;
            csMensaje.Format("No se encuentra el fichero de minerales: %s",m_configuracion.m_csFicheroMinerales);
            AfxMessageBox(csMensaje);
            ASSERT(FALSE);
            return FALSE;
        }

        // Cargamos fichero con asociaciones de minerales por defecto
        m_asociaciones.Init(&m_minerales);
        m_asociaciones.CargarFichero(m_configuracion.m_csFicheroAsociaciones);

        //cargamos entrenamiento (matriz inversa de mahalanobis)
        if (m_minerales.CargarEntrenamiento(m_configuracion.m_csFicheroDatos, -1/*m_ParamIni.nBandas-1*/) == false)
        {
            AfxMessageBox("Error al cargar entrenamiento");
            fclose(m_log);
            fclose(g_log);
            ASSERT(FALSE);
            return FALSE;
        }
        m_minerales.CalcularIndicesBusqueda();

#ifndef _DEBUG
    } 
    catch (CMemoryException* e)
    {
        // Probablemente se trate de una excepcion de memoria porque hay demasiados datos de pixels que cargar
        AfxMessageBox("Excepcion de memoria al cargar minerales, es posible que el numero de datos a cargar sea excesivo para la memoria disponible");
        e->Delete();
        fclose(m_log);
        fclose(g_log);
        return FALSE;
    }
    catch (...)
    {
        // Probablemente se trate de una excepcion de memoria porque hay demasiados datos de pixels que cargar
        AfxMessageBox("Excepcion general al cargar minerales");
        fclose(m_log);
        fclose(g_log);
        return FALSE;
    }
#endif


    //Inicia MIL
    MIL_ID Milaplicacion;
	MappAlloc(M_DEFAULT, &Milaplicacion);	// Selecciono la aplicación MIL.
	if ( Milaplicacion == M_NULL ) {
        AfxMessageBox("Error: No se pudo inicilizar MIL", MB_ICONERROR | MB_OK);
	}

    m_clasificacionBarrido.Init();

    CClasificaBarridoDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
        MappFree(Milaplicacion);
    }
	else if (nResponse == IDCANCEL)
	{
        MappFree(Milaplicacion);
	}

    m_configuracion.Guardar(szPath);

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
    fclose(m_log);
    fclose(g_log);
	return FALSE;
}

// Gets the path to the executable file
void CClasificaBarridoApp::GetAppPath(LPTSTR pszPath)
{ 
    TCHAR szAppPath[_MAX_PATH + 1];
    GetModuleFileName(NULL, szAppPath, _MAX_PATH); 
    TCHAR* pszTemp = _tcsrchr(szAppPath, _T('\\'));
    int n = (int)(pszTemp - szAppPath + 1);
    szAppPath [ n ] = 0;
    _tcscpy(pszPath, szAppPath);
}
