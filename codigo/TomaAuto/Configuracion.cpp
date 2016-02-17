
#include "stdafx.h"
#include "TomaAuto.h"
#include "Configuracion.h"
#include "..\librerias\include\procesoAux.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CTomaAutoApp theApp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfiguracion::CConfiguracion()
{
    m_nDimension_barrido_x      = -1;
    m_nDimension_barrido_y      = -1;
    m_nDimension_toma_x         = -1;
    m_nDimension_toma_y         = -1;
    m_ratio_inicial             = -1;
    m_dEscalaReflectancia       = -1;
    m_nBits                     = -1;
    m_nImagenAcum               = -1;
}

CConfiguracion::~CConfiguracion()
{

}

bool CConfiguracion::Cargar()
{
    // Componer path y nombre del fichero (donde este el ejecutable)
    char szPath[_MAX_PATH + 1];
    theApp.GetAppPath(szPath);
    strcat(szPath, FICHERO_CONFIGURACION);

	if ( LoadVars(szPath) == NULL) {
		return false;
	}

    char* dir_inicial_barridos = m_csDirectorioBarrido.GetBuffer(_MAX_PATH + 1);
	LOADSTR(dir_inicial_barridos);
    m_csDirectorioBarrido.ReleaseBuffer(-1);

    char* dir_inicial_toma = m_csDirectorioToma.GetBuffer(_MAX_PATH + 1);
	LOADSTR(dir_inicial_toma);
    m_csDirectorioToma.ReleaseBuffer(-1);

	double ratio;
	LOADDBL(ratio);
    m_ratio_inicial = ratio;

    int dimension_barrido_x = -1;
	LOADINT(dimension_barrido_x);
    m_nDimension_barrido_x = dimension_barrido_x;

	int dimension_barrido_y = -1;
	LOADINT(dimension_barrido_y);
    m_nDimension_barrido_y = dimension_barrido_y;

    int dimension_toma_x = -1;
	LOADINT(dimension_toma_x);
    m_nDimension_toma_x = dimension_toma_x;

	int dimension_toma_y = -1;
	LOADINT(dimension_toma_y);
    m_nDimension_toma_y = dimension_toma_y;

	double escala_reflectancia = -1;
	LOADDBL(escala_reflectancia);
    if (escala_reflectancia < MIN_ESCALA_REFLECTANCIA || escala_reflectancia > MAX_ESCALA_REFLECTANCIA)
        m_dEscalaReflectancia = theApp.m_ParamIni.escala;
    else
        m_dEscalaReflectancia = escala_reflectancia;

	int bits_profundidad = -1;
	LOADINT(bits_profundidad);
    m_nBits = bits_profundidad;

	int acumulacion_campos = -1;
	LOADINT(acumulacion_campos);
    if (acumulacion_campos < 1 || acumulacion_campos > 16)
        m_nImagenAcum = 1;
    else
        m_nImagenAcum = acumulacion_campos;

    return true;
}

bool CConfiguracion::Guardar()
{
	FILE  *archivo;

    // Componer path y nombre del fichero (donde este el ejecutable)
    char szPath[_MAX_PATH + 1];
    theApp.GetAppPath(szPath);
    strcat(szPath, FICHERO_CONFIGURACION);

	if ((archivo = fopen(szPath, "wt")) == NULL)
		return false;

	fprintf(archivo, "# PARÁMETROS DE CONFIGURACION PARA LA HERRAMIENTA DE TOMA AUTOMATICA\n\n");

   	fprintf(archivo, "# Directorios iniciales\n");
	fprintf(archivo, "dir_inicial_barridos = %s\n\n", m_csDirectorioBarrido);
	fprintf(archivo, "dir_inicial_toma = %s\n\n", m_csDirectorioToma);

	fprintf(archivo, "# Ratio inicial de presentacion de la imagen.\n");
	fprintf(archivo, "ratio = %.2f\n\n", m_ratio_inicial);

	fprintf(archivo, "# Dimensiones adquisicion\n");
	fprintf(archivo, "dimension_barrido_x = %d \n", m_nDimension_barrido_x);
	fprintf(archivo, "dimension_barrido_y = %d \n", m_nDimension_barrido_y);
	fprintf(archivo, "dimension_toma_x = %d \n", m_nDimension_toma_x);
	fprintf(archivo, "dimension_toma_y = %d \n", m_nDimension_toma_y);

	fprintf(archivo, "# Escala de reflectancia - nivle de gris con el que se guardan las imágenes adquiridas.\n");
	fprintf(archivo, "escala_reflectancia = %.4lf\n\n", m_dEscalaReflectancia);

	fprintf(archivo, "# Profundidad en bits con el que se guardan las imágenes adquiridas.\n");
	fprintf(archivo, "bits_profundidad = %d\n\n", m_nBits);

	fprintf(archivo, "# Numero de imagenes a acumular en la adquisicion (toma manual y barrido) de cada campo.\n");
	fprintf(archivo, "acumulacion_campos = %d\n\n", m_nImagenAcum);

	fclose(archivo);

    return true;
}
