// Analisis.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Analisis.h"
#include "AnalisisDlg.h"

#ifndef LONGITUD_TEXTO
    #define LONGITUD_TEXTO	  512 // necesario para fichLog (tambien definido en gestion_mensajes.h)
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CAnalisisApp

BEGIN_MESSAGE_MAP(CAnalisisApp, CWinApp)
	//{{AFX_MSG_MAP(CAnalisisApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnalisisApp construction

CAnalisisApp::CAnalisisApp()
{
    m_dEscalaReflectancia = -1;
    m_nBitsProfundidad = -1;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CAnalisisApp object

CAnalisisApp theApp;
FILE* g_log = NULL;
char fichLog[LONGITUD_TEXTO];				// Fichero '*.log' de la aplicación (necesario porque ProcesoAux usa "extern")
long* histo_estimado;                       //necesario porque ProcesoAux usa "extern"
parametros	paramIni;                       //necesario porque ProcesoAux usa "extern"
/////////////////////////////////////////////////////////////////////////////
// CAnalisisApp initialization

BOOL CAnalisisApp::InitInstance()
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

        g_log = fopen("log_analisis.txt","wt");
//    strcpy(fichLog,"log_analisis.txt");

	// Lectura de las variables de inicialización
	if (lee_param(&m_ParamIni) == -1) // si -2, no se han podido leer los ficheros de configuracion que NO necesitamos
    {
		AfxMessageBox("Error: No se han podido cargar los ficheros de configuración");
        fclose(g_log);
        return FALSE;
    }
    if (!m_configuracion.Cargar())
    {
		AfxMessageBox("Error: No se ha podido cargar el fichero de configuración de la aplicación");
        fclose(g_log);
        return FALSE;
    }


#ifndef _DEBUG
    try
    {
#endif
        // cargar nombres y abreviaturas de minerales
        m_minerales.CargarMinerales(m_configuracion.m_csFicheroMinerales);

        clasificacion.SetMinerales(&m_minerales);

        // Componer path (donde este el ejecutable)
        char szPath[_MAX_PATH + 1];
        GetAppPath(szPath);
        m_configuracion.m_csPathEjecutable = szPath;

        // Cargamos fichero con asociaciones de minerales
        m_asociaciones.Init(&m_minerales);
        m_asociaciones.CargarFichero(m_configuracion.m_csFicheroAsociaciones); 


        //cargamos entrenamiento (matriz inversa de mahalanobis)
        if (m_minerales.CargarEntrenamiento(m_configuracion.m_csFicheroDatos, m_ParamIni.nBandas-1) == false)
        {
            AfxMessageBox("Error al cargar entrenamiento");
            fclose(g_log);
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
        fclose(g_log);
        return FALSE;
    }
    catch (...)
    {
        // Probablemente se trate de una excepcion de memoria porque hay demasiados datos de pixels que cargar
        AfxMessageBox("Excepcion general al cargar minerales");
        fclose(g_log);
        return FALSE;
    }
#endif
//m_minerales.Pruebas();
//return 0;
    // initialize <code>GDI+ (gdi+ is in Gdiplus namespace) (Necesario para PlotGraphicLibrary)
    ULONG_PTR m_ulGdiplusToken; //Necesario para inicilizar GDI+
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&m_ulGdiplusToken, &gdiplusStartupInput, 
                            NULL);
#ifndef _DEBUG
    try{
#endif
	    CAnalisisDlg dlg;
	    m_pMainWnd = &dlg;
	    int nResponse = dlg.DoModal();
	    if (nResponse == IDOK)
	    {
	    }
	    else if (nResponse == IDCANCEL)
	    {
	    }
#ifndef _DEBUG
    }
    catch(...)
    {
        AfxMessageBox("excepcion en dialogo");
        ASSERT(FALSE);
    }
#endif
    // shutdown GDI+
    Gdiplus::GdiplusShutdown(m_ulGdiplusToken);

    // Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
    fclose(g_log);
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//	Función para leer los parámetros de adquisición 
//	Devuelve 0 si todo ha ido bien, -1 en otro caso
int CAnalisisApp::lee_param(parametros *paramAdq)
{
	// Hay que declarar una variable por cada valor a leer porque la funcion auxiliar
	// que usamos (LOADINT, etc) usa el nombre de la variable tambien como string a buscar 
//	int apaga_lamp;
	char  nom_fich[512];
	int	 num_bandas, *filtros_bandas;

	char	*fn = "lee_Param";

	int		nResultado = 0;

	paramAdq->nBandas = 0;
    memset(paramAdq->filtro, sizeof(paramAdq->filtro), 0);

	//	Indico el nombre del archivo donde se encuentran los parámetros
	//	de toma de imágenes, así como del microscopio y la cámara.
	sprintf(nom_fich, "%s%s", FICH_PARAM_APLICACION, EXT_INI);

	if ( LoadVars(nom_fich) == NULL) {
		error_fatal(nom_fich, "El fichero indicado no se encuentra", 0);
		return -1;
	}

	//	Leemos el número de bandas que procesamos en el barrido.
	//	Si el número de bandas es NULL se procesa por defecto una
	//	sola banda. 
	LOADINT(num_bandas);
	if (num_bandas > MAX_FILTROS_DTA || num_bandas <= 0) {
		error_fatal(fn, "Valor de 'num_bandas' inaceptable", 1);
		return -1;
	}
	paramAdq->nBandas = num_bandas;

	filtros_bandas = paramAdq->filtro;
	LOADVINT(filtros_bandas, MAX_FILTROS_DTA);

	// En el fichero, los filtros van de 1 a 16, en memoria trabajamos de 0 a 15
	for (int i=0; i<num_bandas;i++)
	{
		if (paramAdq->filtro[i]<1 || paramAdq->filtro[i]>MAX_FILTROS_DTA)
		{
			error_leve(fn,"Se ha encontrado un filtro con valor fuera del rango 1..MAX_FILTROS_DTA en el fichero de configruacion");
			paramAdq->filtro[i] = 0;
		}
		else
		{
			paramAdq->filtro[i]--;
		}
	}

    double escala;
	if ( LOADDBL(escala) == 0.0 )
		escala = ESCALA_BASE;
	paramAdq->escala = escala;

	//Leemeos otros ficheros de inicializacion

		//	Parámetros de adquisición relativos a la camara
	sprintf(nom_fich, "%s%s", FICH_PARAM_CAMARA, EXT_INI);
	nResultado = nResultado + lee_paramCamara(nom_fich, paramAdq);

		//	Parámetros de adquisición relativos a la camara
	sprintf(nom_fich, "%s%s", FICH_PARAM_REVOLVER, EXT_INI);
	nResultado = nResultado + lee_paramRevolver(nom_fich, paramAdq);

    return nResultado;
}

// Gets the path to the executable file
void CAnalisisApp::GetAppPath(LPTSTR pszPath)
{ 
    TCHAR szAppPath[_MAX_PATH + 1];
    GetModuleFileName(NULL, szAppPath, _MAX_PATH); 
    TCHAR* pszTemp = _tcsrchr(szAppPath, _T('\\'));
    int n = (int)(pszTemp - szAppPath + 1);
    szAppPath [ n ] = 0;
    _tcscpy(pszPath, szAppPath);
}
