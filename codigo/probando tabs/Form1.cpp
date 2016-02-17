// Form1.cpp : implementation file
//

#include "stdafx.h"
#include "probando tabs.h"
#include "Form1.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Form1 dialog


Form1::Form1(CWnd* pParent /*=NULL*/)
	: CDialog(Form1::IDD, pParent)
{
	//{{AFX_DATA_INIT(Form1)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void Form1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Form1)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Form1, CDialog)
	//{{AFX_MSG_MAP(Form1)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Form1 message handlers
