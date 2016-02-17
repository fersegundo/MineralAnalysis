
#include "stdafx.h"
#include "Analisis.h"
#include "Configuracion.h"
#include "..\librerias\include\procesoAux.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CAnalisisApp theApp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfiguracion::CConfiguracion()
{
    m_bCargado = false;
    m_csFicheroMinerales.Empty();
    m_csFicheroDatos.Empty();
    //Inicializamos a 0
    memset(m_arrPorcentajesB,0,MAX_NUM_IMAGENES*sizeof(int));
    memset(m_arrPorcentajesG,0,MAX_NUM_IMAGENES*sizeof(int));
    memset(m_arrPorcentajesR,0,MAX_NUM_IMAGENES*sizeof(int));
    m_ratio_inicial = -1; //de la imagen
    m_csFicheroAsociaciones.Empty();
    m_dConfiabilidad    = -1;
    m_dDistancia        = -1;
    m_dMinRef           = -1;

}

CConfiguracion::~CConfiguracion()
{
}

bool CConfiguracion::Cargar()
{
    // Componer path y nombre del fichero (donde este el ejecutable)
    char szPath[_MAX_PATH + 1];
    memset(szPath,0,_MAX_PATH + 1);
//    theApp.GetAppPath(szPath);
    strcat(szPath, FICHERO_CONFIGURACION);

	if ( LoadVars(szPath) == NULL) {
		return false;
	}

    char* fich_minerales = m_csFicheroMinerales.GetBuffer(100);
	LOADSTR(fich_minerales);
    m_csFicheroMinerales.ReleaseBuffer(-1);

    char* fich_datos = m_csFicheroDatos.GetBuffer(100);
	LOADSTR(fich_datos);
    m_csFicheroDatos.ReleaseBuffer(-1);

    char* fich_asociaciones  ;
    fich_asociaciones  = m_csFicheroAsociaciones.GetBuffer(100);
	LOADSTR(fich_asociaciones );
    m_csFicheroAsociaciones.ReleaseBuffer(-1);

    char* dir_inicial  ;
    dir_inicial  = m_csDefaultPath.GetBuffer(100);
	LOADSTR(dir_inicial);
    m_csDefaultPath.ReleaseBuffer(-1);

	double ratio;
	LOADDBL(ratio);
    m_ratio_inicial = ratio;

	int* R = m_arrPorcentajesR;
	LOADVINT(R, MAX_NUM_IMAGENES);

	int* G = m_arrPorcentajesG;
	LOADVINT(G, MAX_NUM_IMAGENES);

	int* B = m_arrPorcentajesB;
	LOADVINT(B, MAX_NUM_IMAGENES);

	double umbral_distancia;
	LOADDBL(umbral_distancia);
    m_dDistancia = umbral_distancia;

	double umbral_confiabilidad;
	LOADDBL(umbral_confiabilidad);
    m_dConfiabilidad = umbral_confiabilidad;

	double umbral_reflectancia_minima_oscuros;
	LOADDBL(umbral_reflectancia_minima_oscuros);
    m_dMinRef = umbral_reflectancia_minima_oscuros;

    m_bCargado = true;
    return true;
}

bool CConfiguracion::Guardar()
{
	int  i;
	FILE  *archivo;

    // Componer path y nombre del fichero (donde este el ejecutable)
    char szPath[_MAX_PATH + 1];
    theApp.GetAppPath(szPath);
    strcat(szPath, FICHERO_CONFIGURACION);

	if ((archivo = fopen(szPath, "wt")) == NULL)
		return false;

	fprintf(archivo, "# PARÁMETROS DE CONFIGURACION PARA LA HERRAMIENTA DE ANALISIS\n\n");

   	fprintf(archivo, "# Fichero de minerales\n");
	fprintf(archivo, "fich_minerales = %s\n\n", m_csFicheroMinerales);

   	fprintf(archivo, "# Fichero de entrenamiento\n");
	fprintf(archivo, "fich_datos = %s\n\n", m_csFicheroDatos);

   	fprintf(archivo, "# Fichero asociaciones\n");
	fprintf(archivo, "fich_asociaciones = %s\n\n", m_csFicheroAsociaciones);

   	fprintf(archivo, "# Directorio inicial\n");
	fprintf(archivo, "dir_inicial = %s\n\n", m_csDefaultPath);

	fprintf(archivo, "# Ratio inicial de presentacion de la imagen.\n");
	fprintf(archivo, "ratio = %.2f\n\n", m_ratio_inicial);

	fprintf(archivo, "# Valores RGB\n");
	fprintf(archivo, "R =  ");
	for (i=0; i < MAX_NUM_IMAGENES; i++)
		fprintf(archivo, "%3d ", m_arrPorcentajesR[i]);
	fprintf(archivo, "\n");

	fprintf(archivo, "G =  ");
	for (i=0; i < MAX_NUM_IMAGENES; i++)
		fprintf(archivo, "%3d ", m_arrPorcentajesG[i]);
  	fprintf(archivo, "\n");

	fprintf(archivo, "B =  ");
	for (i=0; i < MAX_NUM_IMAGENES; i++)
		fprintf(archivo, "%3d ", m_arrPorcentajesB[i]);
	fprintf(archivo, "\n\n");

	fprintf(archivo, "# Umbral de distancia para la clasificacion selectiva.\n");
	fprintf(archivo, "umbral_distancia = %.2f\n", m_dDistancia);

	fprintf(archivo, "# Umbral de confiabilidad para la clasificacion selectiva.\n");
	fprintf(archivo, "umbral_confiabilidad = %.2f\n", m_dConfiabilidad);

	fprintf(archivo, "# Umbral de reflectancia minima para la clasificacion.\n");
	fprintf(archivo, "umbral_reflectancia_minima_oscuros = %.2f\n\n", m_dMinRef);

	fclose(archivo);

    return true;
}
