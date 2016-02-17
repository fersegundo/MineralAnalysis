// ProgresoMosaico.cpp : implementation file
//

#include "stdafx.h"
#include "TomaAuto.h"
#include "ProgresoMosaico.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ProgresoMosaico dialog

extern CTomaAutoApp theApp;

ProgresoMosaico::ProgresoMosaico(CWnd* pParent /*=NULL*/)
	: CDialog(ProgresoMosaico::IDD, pParent)
{
	//{{AFX_DATA_INIT(ProgresoMosaico)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void ProgresoMosaico::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(ProgresoMosaico)
	DDX_Control(pDX, IDC_STATIC_2, m_static2);
	DDX_Control(pDX, IDC_STATIC_1, m_static1);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ProgresoMosaico, CDialog)
	//{{AFX_MSG_MAP(ProgresoMosaico)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ProgresoMosaico message handlers

BOOL ProgresoMosaico::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    CenterWindow();	
    m_progress.SetRange(0,theApp.m_ParamIni.nBandas);
    m_progress.SetStep(1);

    // Cambiamos el tamaño etc del texto
    LOGFONT lfNew;
    ZeroMemory (&lfNew, sizeof(LOGFONT));
    lfNew.lfHeight = 18;
    lfNew.lfWeight = FW_BOLD;
    strcpy (lfNew.lfFaceName, "Arial"); 
    m_fntTexto.CreateFontIndirect(&lfNew);

    m_static1.SetFont (&m_fntTexto, TRUE);    
    m_static2.SetFont (&m_fntTexto, TRUE);    
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void ProgresoMosaico::IncrementarProgreso()
{
    int prevoius = m_progress.StepIt();
}
