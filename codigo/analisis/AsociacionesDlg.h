#if !defined(AFX_ASOCIACIONESDLG_H__A45562E0_0C84_4A68_BA14_5FC6CD70FCFC__INCLUDED_)
#define AFX_ASOCIACIONESDLG_H__A45562E0_0C84_4A68_BA14_5FC6CD70FCFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "seleccionmineralesdlg.h"

// Controles comunes a Clasifica barrido y Analisis
#ifndef IDD_ASOCIACIONES
    #define IDD_ASOCIACIONES 156
#endif
#ifndef IDC_ASOCIACION_USUARIO
    #define IDC_ASOCIACION_USUARIO 1119
#endif
#ifndef IDC_DEFINIR_MINERALES
    #define IDC_DEFINIR_MINERALES 1120
#endif
#ifndef CONTROLES_DINAMICOS_AS
    #define CONTROLES_DINAMICOS_AS 200
#endif


#define FICHERO_ASOCIACIONES_SELECCIONADAS "AsociacionesSeleccionadas.ini"

/////////////////////////////////////////////////////////////////////////////
// CAsociacionesDlg dialog
class CAsociacionesMinerales;
class CMinerales;
class CAsociacionesDlg : public CDialog
{
private:
    CAsociacionesMinerales* m_pAsociaciones; // acceso a los datos de asociaciones
    CMinerales*             m_pMinerales;    // acceso a los datos de asociaciones
    CStatic*                m_arrDescripciones; // lista de controles static con las descripciones de cada asociacion mineral
    SeleccionMineralesDlg   m_seleccionMineralesDlg;
    CFont                   m_fontBold; //auxiliar para poner en negrita "ASOCIACION PERSONALIZADA"
protected:
    CButton*                m_arrCheck;         // lista de controles check box de cada asociacion mineral
// Construction
public:
	CAsociacionesDlg(UINT nIDTemplate = IDD_ASOCIACIONES, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAsociacionesDlg)
	CButton	m_checkUsuario;
	//}}AFX_DATA

    bool CAsociacionesDlg::Init(CAsociacionesMinerales* pAsociaciones, CMinerales* pMinerales);
    void CAsociacionesDlg::Liberar();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAsociacionesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAsociacionesDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnDefinirMinerales();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ASOCIACIONESDLG_H__A45562E0_0C84_4A68_BA14_5FC6CD70FCFC__INCLUDED_)
