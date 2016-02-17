
#if !defined(CONFIGURACION_GESTION_MINERALES_H_)
#define CONFIGURACION_GESTION_MINERALES_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define FICHERO_CONFIGURACION "parametros_GestionMinerales.ini"

//using namespace std;

class CConfiguracion
{
public:
    CString m_csFicheroMinerales;
    bool m_bCargado; //si el fichero existia al cargarlo

public:
	CConfiguracion();
//	CConfiguracion(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfiguracion();
    bool Cargar();
    bool Guardar();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif 

