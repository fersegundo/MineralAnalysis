#if !defined(AFX_FORM2_H__8A595C89_9FAD_4198_B9C4_A08B403DA6AB__INCLUDED_)
#define AFX_FORM2_H__8A595C89_9FAD_4198_B9C4_A08B403DA6AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Form2.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CForm2 dialog

class CForm2 : public CDialog
{
// Construction
public:
	CForm2(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CForm2)
	enum { IDD = IDD_FORMVIEW2 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CForm2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CForm2)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FORM2_H__8A595C89_9FAD_4198_B9C4_A08B403DA6AB__INCLUDED_)
