#if !defined(AFX_SELECCIONMINERALES_H__8C55187A_34C4_4F96_9100_7F6607B759A0__INCLUDED_)
#define AFX_SELECCIONMINERALES_H__8C55187A_34C4_4F96_9100_7F6607B759A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#include "resourceClasificaBarrido.h"

#ifndef IDD_SELECCION_MINERALES
    #define IDD_SELECCION_MINERALES 133
#endif

// SeleccionMineralesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SeleccionMineralesDlg dialog

class CMinerales;
class SeleccionMineralesDlg : public CDialog
{
private:
    CButton*    m_arrCheck;   // lista de controles check box de cada mineral
    CMinerales* m_pMinerales; // acceso a minerales
// Construction
public:
	SeleccionMineralesDlg(CWnd* pParent = NULL);   // standard constructor
    void Init(CMinerales* pMinerales);
    void Liberar();

// Dialog Data
	//{{AFX_DATA(SeleccionMineralesDlg)
	enum { IDD = IDD_SELECCION_MINERALES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SeleccionMineralesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SeleccionMineralesDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnMarcar();
	afx_msg void OnDesmarcar();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECCIONMINERALES_H__8C55187A_34C4_4F96_9100_7F6607B759A0__INCLUDED_)
