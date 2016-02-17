
#include "stdafx.h"
#include "ConfiguracionClasificaBarrido.h"
#include "clasificabarrido.h"
//#include "..\librerias\include\procesoAux.h"
#include "..\librerias\ProcesoAux\myVars.h" //LoadVars

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CClasificaBarridoApp theApp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfiguracion::CConfiguracion(CWnd* pParent)
	: CDialog(CConfiguracion::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfiguracion)
	m_csUmbralConfiabilidad = _T("");
	m_csUmbralDistancia = _T("");
	m_csUmbralConfiabilidad2 = _T("");
	m_csUmbralDistancia2 = _T("");
	m_csRefMin = _T("");
	//}}AFX_DATA_INIT

    m_bCargado = false;
    m_csFicheroMinerales.Empty();
    m_csFicheroDatos.Empty();
    m_csFicheroAsociaciones.Empty();

    //Inicializamos a 0
    m_dDistanciaPrimera         = -1;
    m_dDistanciaSegunda         = -1;
    m_dConfiabilidadPrimera     = -1;
    m_dConfiabilidadSegunda     = -1;
}

CConfiguracion::~CConfiguracion()
{
}

/////////////////////////////////////////////////////////////////////////////
// CConfiguracion dialog


BEGIN_MESSAGE_MAP(CConfiguracion, CDialog)
	//{{AFX_MSG_MAP(CConfiguracion)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CConfiguracion::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfiguracion)
	DDX_Text(pDX, IDC_UMBRAL_CONFIABILIDAD, m_csUmbralConfiabilidad);
	DDX_Text(pDX, IDC_UMBRAL_DISTANCIA, m_csUmbralDistancia);
	DDX_Text(pDX, IDC_UMBRAL_CONFIABILIDAD2, m_csUmbralConfiabilidad2);
	DDX_Text(pDX, IDC_UMBRAL_DISTANCIA2, m_csUmbralDistancia2);
	DDX_Text(pDX, IDC_UMBRAL_REF_MIN, m_csRefMin);
	//}}AFX_DATA_MAP
}

BOOL CConfiguracion::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    if (m_dDistanciaPrimera > 0)
        m_csUmbralDistancia.Format("%.2lf", m_dDistanciaPrimera);
    if (m_dConfiabilidadPrimera > 0)
        m_csUmbralConfiabilidad.Format("%.2lf", m_dConfiabilidadPrimera);
    if (m_dDistanciaSegunda > 0)
        m_csUmbralDistancia2.Format("%.2lf", m_dDistanciaSegunda);
    if (m_dConfiabilidadSegunda > 0)
        m_csUmbralConfiabilidad2.Format("%.2lf", m_dConfiabilidadSegunda);
    if (m_dRefMin > 0)
        m_csRefMin.Format("%.2lf", m_dRefMin);
    UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfiguracion::OnOK() 
{
    UpdateData(TRUE);
    m_dDistanciaPrimera     = atof(m_csUmbralDistancia);
    m_dConfiabilidadPrimera = atof(m_csUmbralConfiabilidad);
    m_dDistanciaSegunda     = atof(m_csUmbralDistancia2);
    m_dConfiabilidadSegunda = atof(m_csUmbralConfiabilidad2);
    m_dRefMin = atof(m_csRefMin);

	CDialog::OnOK();
}

//-szPath será el directorio donde esta el ejecutable, que es donde hay que cargar el fichero de configuracion
//        (puede ser que sea diferente que el directorio de trabajo actual)
bool CConfiguracion::Cargar(const char* szPath)
{
    CString csPathFichero;
    csPathFichero.Format("%s%s",szPath,FICHERO_CONFIGURACION);

	if ( LoadVars(csPathFichero) == NULL) {
		return false;
	}
    
    char* fich_minerales ;
    fich_minerales  = m_csFicheroMinerales.GetBuffer(100);
	LOADSTR(fich_minerales );
    m_csFicheroMinerales.ReleaseBuffer(-1);

    char* fich_datos  ;
    fich_datos  = m_csFicheroDatos.GetBuffer(100);
	LOADSTR(fich_datos );
    m_csFicheroDatos.ReleaseBuffer(-1);

    char* fich_asociaciones  ;
    fich_asociaciones  = m_csFicheroAsociaciones.GetBuffer(100);
	LOADSTR(fich_asociaciones );
    m_csFicheroAsociaciones.ReleaseBuffer(-1);

    char* dir_inicial  ;
    dir_inicial  = m_csDefaultPath.GetBuffer(100);
	LOADSTR(dir_inicial);
    m_csDefaultPath.ReleaseBuffer(-1);

    double distancia_primera_pasada;
    LOADDBL(distancia_primera_pasada);
    m_dDistanciaPrimera = distancia_primera_pasada;

    double confiabilidad_primera_pasada;
    LOADDBL(confiabilidad_primera_pasada);
    m_dConfiabilidadPrimera = confiabilidad_primera_pasada;

    double distancia_segunda_pasada;
    LOADDBL(distancia_segunda_pasada);
    m_dDistanciaSegunda = distancia_segunda_pasada;

    double confiabilidad_segunda_pasada;
    LOADDBL(confiabilidad_segunda_pasada);
    m_dConfiabilidadSegunda = confiabilidad_segunda_pasada;

    double reflectancia_minima_oscuros;
    LOADDBL(reflectancia_minima_oscuros);
    m_dRefMin = reflectancia_minima_oscuros;


    m_bCargado = true;
    return true;
}

//-szPath será el directorio donde esta el ejecutable, que es donde hay que cargar el fichero de configuracion
//        (puede ser que saa diferente que el directorio de trabajo actual)
bool CConfiguracion::Guardar(const char* szPath)
{
	FILE  *archivo;

    CString csPathFichero;
    csPathFichero.Format("%s%s",szPath,FICHERO_CONFIGURACION);

	if ((archivo = fopen(csPathFichero, "wt")) == NULL)
		return false;

    fprintf(archivo, "# PARÁMETROS DE CONFIGURACION PARA LA HERRAMIENTA DE CLASIFICA BARRIDO\n\n");


   	fprintf(archivo, "# Fichero de minerales\n");
	fprintf(archivo, "fich_minerales = %s\n\n", m_csFicheroMinerales);

   	fprintf(archivo, "# Fichero de entrenamiento\n");
	fprintf(archivo, "fich_datos = %s\n\n", m_csFicheroDatos);

   	fprintf(archivo, "# Fichero asociaciones\n");
	fprintf(archivo, "fich_asociaciones = %s\n\n", m_csFicheroAsociaciones);

   	fprintf(archivo, "# Directorio inicial\n");
	fprintf(archivo, "dir_inicial = %s\n\n", m_csDefaultPath);

   	fprintf(archivo, "# Umbrales\n");
	fprintf(archivo, "distancia_primera_pasada     = %.2lf\n", m_dDistanciaPrimera);
	fprintf(archivo, "confiabilidad_primera_pasada = %.2lf\n\n", m_dConfiabilidadPrimera);
	fprintf(archivo, "distancia_segunda_pasada     = %.2lf\n", m_dDistanciaSegunda);
	fprintf(archivo, "confiabilidad_segunda_pasada = %.2lf\n", m_dConfiabilidadSegunda);
	fprintf(archivo, "reflectancia_minima_oscuros = %.2lf\n", m_dRefMin);

	fclose(archivo);

    return true;
}

