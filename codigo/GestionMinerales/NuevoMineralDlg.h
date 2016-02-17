#if !defined(AFX_NUEVOMINERALDLG_H__BF26A7E4_AC70_4CB2_B3E8_EB8AC551F51B__INCLUDED_)
#define AFX_NUEVOMINERALDLG_H__BF26A7E4_AC70_4CB2_B3E8_EB8AC551F51B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NuevoMineralDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// NuevoMineralDlg dialog

class CMineral;
class NuevoMineralDlg : public CDialog
{
public:
    CMineral*         m_pMineral; //puntero al mineral que esta siendo editado o creado
// Construction
public:
	NuevoMineralDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(NuevoMineralDlg)
	enum { IDD = IDD_NUEVO_MINERAL };
	CString	m_csNombre;
	CString	m_csAbreviatura;
	short	m_nB;
	short	m_nG;
	short	m_nR;
	int		m_nId;
	short	m_nIndiceColor;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NuevoMineralDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NuevoMineralDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnGenerarIndice();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NUEVOMINERALDLG_H__BF26A7E4_AC70_4CB2_B3E8_EB8AC551F51B__INCLUDED_)
