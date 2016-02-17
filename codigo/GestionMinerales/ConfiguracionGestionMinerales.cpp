
#include "stdafx.h"
#include "ConfiguracionGestionMinerales.h"
#include "GestionMinerales.h"
//#include "..\librerias\include\procesoAux.h"
#include "..\librerias\ProcesoAux\myVars.h" //LoadVars

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CGestionMineralesApp theApp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfiguracion::CConfiguracion()
{
    m_bCargado = false;
    m_csFicheroMinerales.Empty();
}

CConfiguracion::~CConfiguracion()
{
}

bool CConfiguracion::Cargar()
{
    CString csPathFichero;
    csPathFichero.Format("%s",FICHERO_CONFIGURACION);

	if ( LoadVars(csPathFichero) == NULL) {
		return false;
	}
    
    char* fich_minerales ;
    fich_minerales  = m_csFicheroMinerales.GetBuffer(100);
	LOADSTR(fich_minerales );
    m_csFicheroMinerales.ReleaseBuffer(-1);

    m_bCargado = true;
    return true;
}

bool CConfiguracion::Guardar()
{
	FILE  *archivo;

    CString csPathFichero;
    csPathFichero.Format("%s",FICHERO_CONFIGURACION);

	if ((archivo = fopen(csPathFichero, "wt")) == NULL)
		return false;

    fprintf(archivo, "# PARÁMETROS DE CONFIGURACION PARA LA HERRAMIENTA DE GESTION MINERALES\n\n");


   	fprintf(archivo, "# Fichero de minerales\n");
	fprintf(archivo, "fich_minerales = %s\n\n", m_csFicheroMinerales);

	fclose(archivo);

    return true;
}

