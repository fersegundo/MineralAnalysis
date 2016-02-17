// NuevoMineralDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GestionMinerales.h"
#include "NuevoMineralDlg.h"
#include "..\Analisis\Minerales.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CGestionMineralesApp theApp;

/////////////////////////////////////////////////////////////////////////////
// NuevoMineralDlg dialog


NuevoMineralDlg::NuevoMineralDlg(CWnd* pParent /*=NULL*/)
	: CDialog(NuevoMineralDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(NuevoMineralDlg)
	m_csNombre = _T("");
	m_csAbreviatura = _T("");
	m_nB = 0;
	m_nG = 0;
	m_nR = 0;
	m_nId = 0;
	m_nIndiceColor = 0;
	//}}AFX_DATA_INIT
}


void NuevoMineralDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NuevoMineralDlg)
	DDX_Text(pDX, IDC_NOMBRE, m_csNombre);
	DDX_Text(pDX, IDC_ABREVIATURA, m_csAbreviatura);
	DDX_Text(pDX, IDC_B, m_nB);
	DDV_MinMaxInt(pDX, m_nB, 0, 255);
	DDX_Text(pDX, IDC_G, m_nG);
	DDV_MinMaxInt(pDX, m_nG, 0, 255);
	DDX_Text(pDX, IDC_R, m_nR);
	DDV_MinMaxInt(pDX, m_nR, 0, 255);
	DDX_Text(pDX, IDC_ID, m_nId);
	DDX_Text(pDX, IDC_INDICE_COLOR, m_nIndiceColor);
	DDV_MinMaxInt(pDX, m_nIndiceColor, 0, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(NuevoMineralDlg, CDialog)
	//{{AFX_MSG_MAP(NuevoMineralDlg)
	ON_BN_CLICKED(IDC_GENERAR_INDICE, OnGenerarIndice)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// NuevoMineralDlg message handlers

BOOL NuevoMineralDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    m_nId                       = m_pMineral->m_nId ;
    m_csNombre      = m_pMineral->m_csNombre        ;
    m_csAbreviatura = m_pMineral->m_csAbreviatura   ;
    if (m_pMineral->m_RGB[0] >= 0)
        m_nR            = m_pMineral->m_RGB[0]          ;
    if (m_pMineral->m_RGB[1] >= 0)
        m_nG            = m_pMineral->m_RGB[1]          ;
    if (m_pMineral->m_RGB[2] >= 0)
        m_nB            = m_pMineral->m_RGB[2]          ;
    if (m_pMineral->m_nIndiceColor >= 0)
        m_nIndiceColor  = m_pMineral->m_nIndiceColor          ;

	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void NuevoMineralDlg::OnOK() 
{
    UpdateData();

    if (m_csNombre == "")
    {
        AfxMessageBox("Por favor, rellene el nombre");
        return;
    }
    if (m_csAbreviatura == "")
    {
        AfxMessageBox("Por favor, rellene la abrebiatura");
        return;
    }

    m_pMineral->m_csNombre      = m_csNombre;
    m_pMineral->m_csAbreviatura = m_csAbreviatura;
    m_pMineral->m_RGB[0]        = m_nR;
    m_pMineral->m_RGB[1]        = m_nG;
    m_pMineral->m_RGB[2]        = m_nB;
    m_pMineral->m_nIndiceColor  = m_nIndiceColor;
	
	CDialog::OnOK();
}

void NuevoMineralDlg::OnGenerarIndice() 
{
	UpdateData();

    m_pMineral->m_nIndiceColor = (m_nR + m_nG + m_nB) / 3;
    m_pMineral->m_nIndiceColor = theApp.m_minerales.VerificaIndiceColor(m_pMineral->m_nIndiceColor, m_pMineral->m_nId); //comprueba si el indice color asignado no esta repetido y en tal caso devuelve el mas cercano posible
    if (m_pMineral->m_nIndiceColor == -1)
        AfxMessageBox("Error al asignar indice de color para este mineral");
    m_nIndiceColor  = m_pMineral->m_nIndiceColor          ;

	UpdateData(FALSE);
}
