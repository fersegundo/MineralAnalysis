#if !defined(AFX_OPCIONESCONFIGURACION_H_INCLUDED_)
#define AFX_OPCIONESCONFIGURACION_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OpcionesConfiguracionDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// COpcionesConfiguracionDlg dialog

class COpcionesConfiguracionDlg : public CDialog
{
private:
    bool m_bCambiadoManualmente;            //para evitar que se procesen cambios "manuales" (mediante codigo) al editar 
// Construction
public:
	COpcionesConfiguracionDlg(CWnd* pParent = NULL);   // standard constructor
// Dialog Data
	//{{AFX_DATA(COpcionesConfiguracionDlg)
	enum { IDD = IDD_CONFIGURACION_DIALOG };
	CEdit	m_campos_acumular;
	CComboBox	m_combo_bits;
	CEdit	m_escala_reflectancia;
	CString	m_csRefMax;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpcionesConfiguracionDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(COpcionesConfiguracionDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEscalaReflectancia();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif 
