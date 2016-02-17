// Form2.cpp : implementation file
//

#include "stdafx.h"
#include "probando tabs.h"
#include "Form2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CForm2 dialog


CForm2::CForm2(CWnd* pParent /*=NULL*/)
	: CDialog(CForm2::IDD, pParent)
{
	//{{AFX_DATA_INIT(CForm2)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CForm2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CForm2)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CForm2, CDialog)
	//{{AFX_MSG_MAP(CForm2)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CForm2 message handlers
