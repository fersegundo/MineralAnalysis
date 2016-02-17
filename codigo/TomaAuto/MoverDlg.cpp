// MoverDlg.cpp : implementation file
//

#include "stdafx.h"
#include "tomaauto.h"
#include "MoverDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MoverDlg dialog


CMoverDlg::CMoverDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMoverDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMoverDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

    m_dX = 0;
    m_dY = 0;
}



void CMoverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMoverDlg)
	DDX_Control(pDX, IDC_MOVERY, m_edit_moverY);
	DDX_Control(pDX, IDC_MOVERX, m_edit_moverX);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMoverDlg, CDialog)
	//{{AFX_MSG_MAP(CMoverDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MoverDlg message handlers

//! Inicializacion del dialogo
BOOL CMoverDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    // Establecemos la escala de reflectancia 
    CString csAux;
    csAux.Format("%.2lf",m_dX);
    m_edit_moverX.SetWindowText(csAux);

    csAux.Format("%.2lf",m_dY);
    m_edit_moverY.SetWindowText(csAux);

    UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMoverDlg::OnOK() 
{
    CString csAux;
    m_edit_moverX.GetWindowText(csAux);
    m_dX = atof((LPCSTR)csAux); 

    m_edit_moverY.GetWindowText(csAux);
    m_dY = atof((LPCSTR)csAux); 

	CDialog::OnOK();
}
