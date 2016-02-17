// TomaAuto.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "TomaAuto.h"
#include "TomaAutoDlg.h"

#include "..\Librerias\ProcesoAux\control_proceso_imagenes.h"
#include "..\Librerias\ControlDisp\control_digitalizador.h"
#include "..\Librerias\ControlMicroscopio\control_microscopio.h"

#include "..\librerias\LibBarrido\calibracion.h" //prepara_correccion
#include "..\Librerias\ControlMicroscopio\control_objetivos.h" //actualiza_objetivo

#include "..\librerias\LibTiff\InterfazLibTiff.h"

#ifndef LONGITUD_TEXTO
    #define LONGITUD_TEXTO	  512 // necesario para fichLog (tambien definido en gestion_mensajes.h)
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTomaAutoApp

BEGIN_MESSAGE_MAP(CTomaAutoApp, CWinApp)
	//{{AFX_MSG_MAP(CTomaAutoApp)
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

// Estructura para pasar al thread de preparar barrido
typedef struct PARAM_PREARA_BARRIDO_THREAD
{
    CTomaAutoApp*   pThis;
} PARAM_PREARA_BARRIDO_THREAD;



/////////////////////////////////////////////////////////////////////////////
// CTomaAutoApp construction
CTomaAutoApp::CTomaAutoApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTomaAutoApp object

CTomaAutoApp theApp;
char fichLog[LONGITUD_TEXTO];				// Fichero '*.log' de la aplicación (necesario porque ProcesoAux usa "extern")
long* histo_estimado;                       //necesario porque ProcesoAux usa "extern"
parametros	paramIni;                       //necesario porque ProcesoAux usa "extern"

/////////////////////////////////////////////////////////////////////////////
// CTomaAutoApp initialization
limitesMtb	limMtb;				// Máximos y mínimos de los parámetros ++
limitesCam	limCam;				// parametros camara. Se rellena en control_proceso_imagenes.ini_control_digitalizador 

bool CTomaAutoApp::Init()
{
    char fn[5] = "Init"; //mensajes de error

    //	Inicialización del control del microscopio (MTB)
	CoInitializeEx(NULL,COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY);					// Inicialización del servidor OLE

	//	Parámetros de adquisición relativos al microscopio
	char  nom_fich[512];
	sprintf(nom_fich, "%s%s%s", m_ParamIni.raiz_patrones, FICH_PARAM_MICROSCOPIO, EXT_INI);
	if (lee_paramMicroscopio(nom_fich, &m_ParamIni) == -1)
    {
		AfxMessageBox("fallo al leer los parametros del microscopio");
        return false;
    }

	if ( ini_control_microscopio(theApp.m_ParamIni.Mtb.rueda_filtros.posFiltro, theApp.m_ParamIni.Mtb.voltLamp, &limMtb, 
		theApp.m_ParamIni.Mtb.rueda_filtros.filtros[theApp.m_ParamIni.Mtb.rueda_filtros.numFiltros-1]) )  {
		AfxMessageBox("fallo en la inicialización del MTB");
        return false;
	}

	// Lectura de las variables de inicialización
	if (lee_param( &m_ParamIni) == -1) // si -2, no se han podido leer los ficheros de configuracion que NO necesitamos
    {
		AfxMessageBox("Error: No se han podido cargar los ficheros de configuracion");
        return FALSE;
    }
	if ( theApp.m_ParamIni.nBandas == 0 )  {
		AfxMessageBox("Debe haber al menos una banda definida");
        return false;
	}

    m_CorreccionGeometrica.Leer(m_ParamIni);

    m_configuracion.Cargar();
    csNombreToma = "";

    if (m_configuracion.m_dEscalaReflectancia <= 0)
        m_configuracion.m_dEscalaReflectancia = m_ParamIni.escala; // por defecto la escala reflectancia-gris de adquisicion será la misma que la de los patrones


	//Inicializacion Revolver
	Rueda.Init(&theApp.m_ParamIni);
	
	//	Inicialización de la librería de proceso de imágen (MIL)
	if ( ini_control_proceso_imagenes_lite(&theApp.m_ParamIni) )  {
		AfxMessageBox("fallo en la inicialización del proceso de imágenes");
        return false;
	}

	//	Inicialización del control y configuración del digitalizador (cámara)
	if ( ini_control_digitalizador(&theApp.m_ParamIni) )  {
		AfxMessageBox("fallo en la inicialización del proceso de digitalizador");
        return false;
	}

	// prepara_correccion calcula las imagenes auxiliares (numerador y denominador) para cada banda
	// a partir de las imagenes de los patrones
    // Este proceso tarda varios segundos, asi que lo lanzamos como un thread para acortar la inicializacion
    PARAM_PREARA_BARRIDO_THREAD* paramPreparaBarridoThread = new PARAM_PREARA_BARRIDO_THREAD; //debe estar en el heap para que sea accesible desde el thread. Se borra al final del propio thread
    paramPreparaBarridoThread->pThis = this;
    AfxBeginThread(PreparaBarridoThread, (void *)paramPreparaBarridoThread);

    return true;
}

// prepara_barrido calcula las imagenes auxiliares (numerador y denominador) para cada banda
// a partir de las imagenes de los patrones
UINT CTomaAutoApp::PreparaBarridoThread( LPVOID Param )
{
    PARAM_PREARA_BARRIDO_THREAD* paramThread = (PARAM_PREARA_BARRIDO_THREAD*)Param;

	if (prepara_correccion(&theApp.m_ParamIni) == 0)
        paramThread->pThis->HayCorreccion(true);

    delete paramThread;

    return 0;
}    

// Si la preparacion de la correccion es correcta, hay que habilitar barrer y capturar
void CTomaAutoApp::HayCorreccion(bool bHayCorreccion)
{
    ((CTomaAutoDlg*)m_pMainWnd)->HayCorreccion(bHayCorreccion, true);
}

BOOL CTomaAutoApp::InitInstance()
{
	// Compongo el nombre de log de la aplicación
	FILE *fich;
	char mensaje[512];
	sprintf(fichLog, "%sTomaAuto%s", paramIni.raiz_patrones, EXT_LOG);
	if ( (fich = fopen(fichLog, "wt")) == NULL )  {
		sprintf(mensaje, "NO se puede crear el fichero: %s", fichLog);
		AfxMessageBox(mensaje);
        return false;
	}
	fclose(fich);
    sendlog_timestamp("InitInstance", "inicio");

	// Standard initialization
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

    sendlog_timestamp("InitInstance", "afx y lee_param fin");

    if (Init() == false)
        return FALSE;
    sendlog_timestamp("InitInstance", "Init fin");

	CTomaAutoDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//	Función para leer los parámetros de adquisición 
//	Devuelve 0 si todo ha ido bien, -1 en otro caso
int CTomaAutoApp::lee_param(parametros *paramAdq)
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
	sprintf(nom_fich, "%s%s%s", paramAdq->raiz_patrones, FICH_PARAM_APLICACION, EXT_INI);

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

    // escala "de adquisicion" de reflectancia por nivel de gris en la que estan referidos los patrones 
    // Expresado en 8bit
    double escala;
	if ( LOADDBL(escala) == 0.0 )
		escala = ESCALA_BASE;
	paramAdq->escala = escala;

    // Numero de imagenes a acumular en la adquisicion
    int nImagenAcum = -1;
	LOADINT(nImagenAcum);
    paramAdq->nImagenAcum = nImagenAcum;
    paramAdq->BarridoPatrones.nImagenAcum = nImagenAcum;

    int camposEjeX = -1;
	LOADINT(camposEjeX);
    paramAdq->BarridoPatrones.camposEjeX = camposEjeX;

    int camposEjeY = -1;
	LOADINT(camposEjeY);
    paramAdq->BarridoPatrones.camposEjeY = camposEjeY;


	//Leemeos otros ficheros de inicializacion

		//	Parámetros de adquisición relativos a la camara
	sprintf(nom_fich, "%s%s", FICH_PARAM_CAMARA, EXT_INI);
	nResultado = nResultado + lee_paramCamara(nom_fich, paramAdq);

        // Antes de cargar los parametros relativos al objetivo, hay que saber en que objetivo estamos
        //corregimos la escala objetivo segun los aumentos en los que estemos
    bool bHayCambioObjetivo;
    if (!actualizar_configuracion_objetivo(paramAdq, bHayCambioObjetivo)) 
    {
		error_fatal(fn, "Error: No se ha encontrado el objetivo actual en la configuración del microscopio o no se ha podido cargar el fichero de configuración de revólver", 1);
        return -1;
    }

    return nResultado;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// ACCESO CORRECCION GEOMETRICA
///////////////////////////////////////////////////////////////////////////////////////////////////

// Calcula y devuelve el ancho maximo que pueden tener las imagenes de destino teniendo en cuenta la escala y los factores de correccion de imagen
// bRotacion - indica si se aplica el angulo de rotacion en el calculo del maximo
int CTomaAutoApp::CalcularAnchoMax(bool bRotacion)
{
    return m_CorreccionGeometrica.CalcularAnchoMax(m_ParamIni.Cam.anchoImagen,m_ParamIni.Cam.altoImagen, m_ParamIni.nBandas, bRotacion);
}

// Calcula y devuelve el alto maximo que pueden tener las imagenes de destino teniendo en cuenta la escala y los factores de correccion de imagen
// bRotacion - indica si se aplica el angulo de rotacion en el calculo del maximo
int CTomaAutoApp::CalcularAltoMax(bool bRotacion)
{
    return m_CorreccionGeometrica.CalcularAltoMax(m_ParamIni.Cam.anchoImagen,m_ParamIni.Cam.altoImagen, m_ParamIni.nBandas, bRotacion);

}
// correcciones (posicion, escala, etc)
void CTomaAutoApp::CorregirImagenes(bool bCorregirRotacion)
{
    m_CorreccionGeometrica.CorregirImagenes(M_banda, m_ParamIni, bCorregirRotacion);
}

// Lee la configuracion geometrica de fichero
void CTomaAutoApp::ActualizarCorreccionGeometrica()
{
    // Cargar correccion geometrica
    m_CorreccionGeometrica.Leer(m_ParamIni);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// AUXILIAR
///////////////////////////////////////////////////////////////////////////////////////////////////

// Gets the path to the executable file
void CTomaAutoApp::GetAppPath(LPTSTR pszPath)
{ 
    TCHAR szAppPath[_MAX_PATH + 1];
    GetModuleFileName(NULL, szAppPath, _MAX_PATH); 
    TCHAR* pszTemp = _tcsrchr(szAppPath, _T('\\'));
    int n = (int)(pszTemp - szAppPath + 1);
    szAppPath [ n ] = 0;
    _tcscpy(pszPath, szAppPath);
}
