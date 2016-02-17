#if !defined(AFX_MOVERDLG_H__09D8BAF6_BEDF_48AF_8F83_8CEBE3671D49__INCLUDED_)
#define AFX_MOVERDLG_H__09D8BAF6_BEDF_48AF_8F83_8CEBE3671D49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MoverDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// MoverDlg dialog

class CMoverDlg : public CDialog
{
public:
    double  m_dX;
    double  m_dY;

// Construction
public:
	CMoverDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMoverDlg)
	enum { IDD = IDD_MOVER_DIALOG };
	CEdit	m_edit_moverY;
	CEdit	m_edit_moverX;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMoverDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMoverDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOVERDLG_H__09D8BAF6_BEDF_48AF_8F83_8CEBE3671D49__INCLUDED_)
