// probando tabsDlg.h : header file
//

#if !defined(AFX_PROBANDOTABSDLG_H__2EA89A43_987B_450C_854A_C0DF1CA8AB69__INCLUDED_)
#define AFX_PROBANDOTABSDLG_H__2EA89A43_987B_450C_854A_C0DF1CA8AB69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Form1.h"
#include "Form2.h"

/////////////////////////////////////////////////////////////////////////////
// CProbandotabsDlg dialog

class CProbandotabsDlg : public CDialog
{
private:
    Form1 m_form1;
    CForm2 m_form2;
// Construction
public:
	CProbandotabsDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CProbandotabsDlg)
	enum { IDD = IDD_PROBANDOTABS_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProbandotabsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CProbandotabsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROBANDOTABSDLG_H__2EA89A43_987B_450C_854A_C0DF1CA8AB69__INCLUDED_)
