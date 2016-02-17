
#if !defined(CONFIGURACION_H_)
#define CONFIGURACION_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define FICHERO_CONFIGURACION "parametros_analisis.ini"

#include "defines.h"

//using namespace std;

class CConfiguracion 
{
public:
    CString m_csDefaultPath;
    CString m_csFicheroMinerales;
    CString m_csFicheroDatos;
    CString m_csFicheroAsociaciones; // path y nombre del fichero de asociaciones general (por defecto, copia del ultimo usado)
    CString m_csPathEjecutable;     // path ejecutable necesario para gestionar asociaciones seleccionadas
    double m_ratio_inicial; //de la imagen
    int  m_arrPorcentajesR[MAX_NUM_IMAGENES];
    int  m_arrPorcentajesG[MAX_NUM_IMAGENES];
    int  m_arrPorcentajesB[MAX_NUM_IMAGENES];
    double m_dConfiabilidad;
    double m_dDistancia;
    double m_dMinRef;
    bool m_bCargado; //si el fichero existia al cargarlo

public:
	CConfiguracion();
	virtual ~CConfiguracion();
    bool Cargar();
    bool Guardar();
};

#endif 
