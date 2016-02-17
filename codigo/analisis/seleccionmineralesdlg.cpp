// SeleccionMineralesDlg.cpp : implementation file
//

#include "stdafx.h"
//#include "clasificabarrido.h"
#include "minerales.h"
#include "SeleccionMineralesDlg.h"
#include <algorithm> //for std::sort

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Controles comunes a Clasifica barrido y Analisis
#ifndef IDC_MARCAR
    #define IDC_MARCAR 1121
#endif
#ifndef IDC_DESMARCAR
    #define IDC_DESMARCAR 1122
#endif
#ifndef CONTROLES_DINAMICOS_MIN
    #define CONTROLES_DINAMICOS_MIN 500
#endif

extern bool ComparadorMinerales(CMineral* mineral1,CMineral* mineral2); //definido en Minerales.cpp, necesario para ordenar la lista de minerales

/////////////////////////////////////////////////////////////////////////////
// SeleccionMineralesDlg dialog
SeleccionMineralesDlg::SeleccionMineralesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(SeleccionMineralesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(SeleccionMineralesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void SeleccionMineralesDlg::Init(CMinerales* pMinerales)
{
    m_pMinerales = pMinerales;
    m_arrCheck          = new CButton[m_pMinerales->GetCount()];
}

void SeleccionMineralesDlg::Liberar()
{
    delete [] m_arrCheck;
}

void SeleccionMineralesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(SeleccionMineralesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SeleccionMineralesDlg, CDialog)
	//{{AFX_MSG_MAP(SeleccionMineralesDlg)
	ON_BN_CLICKED(IDC_MARCAR, OnMarcar)
	ON_BN_CLICKED(IDC_DESMARCAR, OnDesmarcar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SeleccionMineralesDlg message handlers

BOOL SeleccionMineralesDlg::OnInitDialog() 
{
    const unsigned __int8 nFilasMax = 20;
    const unsigned __int8 nTamCheck = 15;
    const unsigned __int8 nEspaciado = 25;
    const unsigned __int8 nDistanciaColumnas = 200;
    const unsigned __int8 nSangriaInicial = 40;
	CDialog::OnInitDialog();
	
    CRect rect;
    this->GetClientRect(rect);
    //int nAlto = rect.bottom - rect.top;

    // Odenamos minerales por abreviatura
    CMineral::m_enumOrden = ORDEN_ABREVIATURA; //m_enumOrden es static
    std::sort(m_pMinerales->m_list.begin(),m_pMinerales->m_list.end(),ComparadorMinerales);

    unsigned int fila, columna;
    for (int i= 0; i<m_pMinerales->GetCount();i++)
    {
        fila = i % nFilasMax;
        columna = i / nFilasMax;
        CString csInfo;
        csInfo.Format("%-4s - %s",m_pMinerales->GetAbreviatura(i), m_pMinerales->GetNombre(i));
        m_arrCheck[i].Create(csInfo,WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX /*| BS_LEFTTEXT*/, 
            CRect(nSangriaInicial + columna*nDistanciaColumnas, rect.bottom + nEspaciado*fila, nSangriaInicial + (columna+1)*nDistanciaColumnas, rect.bottom + nTamCheck + nEspaciado*fila), this, CONTROLES_DINAMICOS_MIN+i);
        if (m_pMinerales->m_list[i]->m_bActivo)
            m_arrCheck[i].SetCheck(m_pMinerales->m_list[i]->m_bPerteneceAsociacionUsuario);
        else
        {
            m_pMinerales->m_list[i]->m_bPerteneceAsociacionUsuario = false;
            m_arrCheck[i].EnableWindow(FALSE);
        }
    }
    //this->MoveWindow(CRect(40, 100+20*i, 140, 115+20*i));	
    this->SetWindowPos(NULL,0,0,max(nSangriaInicial + (columna+1)*nDistanciaColumnas + 50, rect.right),rect.bottom + nTamCheck + nEspaciado*nFilasMax + 50, SWP_NOMOVE | SWP_NOZORDER); //+50 para el menu y barra de titulo


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void SeleccionMineralesDlg::OnOK() 
{
    for (int i= 0; i<m_pMinerales->GetCount();i++)
    {
        m_pMinerales->m_list[i]->m_bPerteneceAsociacionUsuario = m_arrCheck[i].GetCheck() == TRUE;
    }

    // Ordenamos por reflectancia media (de todas las bandas) para usar arrays de busqueda (para ahorrar tiempo en la clasificacion)
    CMineral::m_enumOrden = ORDEN_REFLECTANCIA; //m_enumOrden es static
    std::sort(m_pMinerales->m_list.begin(),m_pMinerales->m_list.end(),ComparadorMinerales);
    
	CDialog::OnOK();
}

void SeleccionMineralesDlg::OnCancel() 
{
    // Ordenamos por reflectancia media (de todas las bandas) para usar arrays de busqueda (para ahorrar tiempo en la clasificacion)
    CMineral::m_enumOrden = ORDEN_REFLECTANCIA; //m_enumOrden es static
    std::sort(m_pMinerales->m_list.begin(),m_pMinerales->m_list.end(),ComparadorMinerales);
	
	CDialog::OnCancel();
}

void SeleccionMineralesDlg::OnMarcar() 
{
    for (int i= 0; i<m_pMinerales->GetCount();i++)
    {
        m_arrCheck[i].SetCheck(TRUE);
    }
}

void SeleccionMineralesDlg::OnDesmarcar() 
{
    for (int i= 0; i<m_pMinerales->GetCount();i++)
    {
        m_arrCheck[i].SetCheck(FALSE);
    }
}
