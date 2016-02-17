#if !defined(AFX_PROGRESOMOSAICO_H__1BEA8EF4_2A3C_44CD_9E04_FFC1E268C913__INCLUDED_)
#define AFX_PROGRESOMOSAICO_H__1BEA8EF4_2A3C_44CD_9E04_FFC1E268C913__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"		// main symbols
#include <afxcmn.h>

/////////////////////////////////////////////////////////////////////////////
// ProgresoMosaico dialog

class ProgresoMosaico : public CDialog
{
private:
    CFont m_fntTexto;
// Construction
public:
	void IncrementarProgreso();
	ProgresoMosaico(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ProgresoMosaico)
	enum { IDD = IDD_PROGRESO_MOSAICO };
	CStatic	m_static2;
	CStatic	m_static1;
	CProgressCtrl	m_progress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ProgresoMosaico)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(ProgresoMosaico)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROGRESOMOSAICO_H__1BEA8EF4_2A3C_44CD_9E04_FFC1E268C913__INCLUDED_)
