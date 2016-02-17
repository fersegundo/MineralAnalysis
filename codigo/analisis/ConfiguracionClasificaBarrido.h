
#if !defined(CONFIGURACION_CLASIFICA_BARRIDO_H_)
#define CONFIGURACION_CLASIFICA_BARRIDO_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define FICHERO_CONFIGURACION "parametros_ClasificaBarrido.ini"

#include "defines.h"
#include "resourceClasificaBarrido.h"

//using namespace std;

class CConfiguracion : public CDialog
{
public:
    CString m_csDefaultPath; //directorio de trabajo actual
    CString m_csFicheroMinerales;
    CString m_csFicheroDatos;
    CString m_csFicheroAsociaciones; // path y nombre del fichero de asociaciones general (por defecto, copia del ultimo usado)
    CString m_csPathEjecutable;     // path ejecutable necesario para gestionar asociaciones seleccionadas
    double  m_dDistanciaPrimera;
    double  m_dDistanciaSegunda;
    double  m_dConfiabilidadPrimera;
    double  m_dConfiabilidadSegunda;
    double  m_dRefMin;
    bool    m_bCargado; //si el fichero existia al cargarlo
// Dialog Data
	//{{AFX_DATA(CConfiguracion)
	enum { IDD = IDD_OPCIONES };
	CString	m_csUmbralConfiabilidad;
	CString	m_csUmbralDistancia;
	CString	m_csUmbralConfiabilidad2;
	CString	m_csUmbralDistancia2;
	CString	m_csRefMin;
	//}}AFX_DATA
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CConfiguracion)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CConfiguracion)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
//	CConfiguracion();
	CConfiguracion(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfiguracion();
    bool Cargar(const char* szPath);
    bool Guardar(const char* szPath);

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif 

