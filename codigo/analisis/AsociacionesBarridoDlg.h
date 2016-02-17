#if !defined(AFX_ASOCIACION_BARRIDO_DLG_)
#define AFX_ASOCIACION_BARRIDO_DLG_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resourceClasificaBarrido.h"
#include "AsociacionesDlg.h"

#define WM_UPDATEFIELDS WM_APP + 1 //Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
/////////////////////////////////////////////////////////////////////////////
// CAsociacionesDlg dialog
class CAsociacionesBarridoDlg : public CAsociacionesDlg
{
private:
    CString                 m_csMuestra; //Para filtrar necesitaremos cargar imagenes y para cargar imagenes necesitaremos el nombre de la muestra a cargar
    bool                    m_bBotonFiltrarHabilitado; //tenemos la informacion necesaria para filtrar asociaciones
// Construction
public:
	CAsociacionesBarridoDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAsociacionesBarridoDlg)
	enum { IDD = IDD_ASOCIACIONES_BARRIDO };
	CButton	m_BotonFiltrar;
	CString	m_csCamposAs;
	CString	m_csTotalAs;
	//}}AFX_DATA

    void CAsociacionesBarridoDlg::HabilitaFiltrar(BOOL bHabilitar, CString csMuestra = "");
    void CAsociacionesBarridoDlg::ActualizaProgresionMinerales(unsigned int* arrTotal, int nCamposTerminados);
    void CAsociacionesBarridoDlg::FinFiltrar(int* arrAsocCompatibles);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAsociacionesBarridoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAsociacionesDlg)
    afx_msg LRESULT UpdateDisplay(UINT wParam, LONG lParam);//Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
	afx_msg void OnFiltrar();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined
