// AsociacionesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AsociacionesBarridoDlg.h"
#include "ClasificaBarrido.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CClasificaBarridoApp theApp;

CAsociacionesBarridoDlg::CAsociacionesBarridoDlg(CWnd* pParent /*=NULL*/)
	: CAsociacionesDlg(CAsociacionesBarridoDlg::IDD, pParent)
{
    m_bBotonFiltrarHabilitado = false;
}


void CAsociacionesBarridoDlg::DoDataExchange(CDataExchange* pDX)
{
	CAsociacionesDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAsociacionesBarridoDlg)
	DDX_Control(pDX, IDC_FILTRAR, m_BotonFiltrar);
	DDX_Text(pDX, IDC_STATIC_CAMPO_AS, m_csCamposAs);
	DDX_Text(pDX, IDC_TOTAL_AS, m_csTotalAs);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAsociacionesBarridoDlg, CAsociacionesDlg)
	//{{AFX_MSG_MAP(CAsociacionesBarridoDlg)
    ON_MESSAGE(WM_UPDATEFIELDS, UpdateDisplay) //Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
	ON_BN_CLICKED(IDC_FILTRAR, OnFiltrar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAsociacionesDlg message handlers
BOOL CAsociacionesBarridoDlg::OnInitDialog() 
{
	CAsociacionesDlg::OnInitDialog();

    m_BotonFiltrar.EnableWindow(m_bBotonFiltrarHabilitado);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
LRESULT CAsociacionesBarridoDlg::UpdateDisplay(WPARAM wParam, LPARAM lParam)
{
    UpdateData((BOOL)wParam);
    return 0;
}

// hay informacion necesaria para filtrar automaticamente las asociaciones
// csMuestra - para filtrar necesitaremos cargar imagenes y para cargar imagenes necesitaremos el nombre de la muestra a cargar
void CAsociacionesBarridoDlg::HabilitaFiltrar(BOOL bHabilitar, CString csMuestra) 
{
    m_bBotonFiltrarHabilitado = (bHabilitar == TRUE);
    if (bHabilitar == TRUE)
        m_csMuestra = csMuestra;
}

void CAsociacionesBarridoDlg::OnFiltrar() 
{
    if(theApp.m_configuracion.m_dConfiabilidadPrimera <= 0 || theApp.m_configuracion.m_dDistanciaPrimera <= 0)
    {
        AfxMessageBox("Es necesario dar valores a los umbrales de distancia y confiabilidad");
        return;
    }

    m_BotonFiltrar.EnableWindow(FALSE);
    m_csTotalAs.Format("%d",theApp.m_nCampos_x*theApp.m_nCampos_y);
    m_csCamposAs.Format("%d",0);
    UpdateData(FALSE);
    theApp.m_clasificacionBarrido.PrimeraPasada(theApp.m_configuracion.m_dDistanciaPrimera, theApp.m_configuracion.m_dConfiabilidadPrimera,
                                            m_csMuestra, this);	
}


void CAsociacionesBarridoDlg::ActualizaProgresionMinerales(unsigned int* arrTotal, int nCamposTerminados)
{

//    ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)TRUE, (LPARAM)0); //Update display
    m_csCamposAs.Format("%d",nCamposTerminados);
//    UpdateData(FALSE);
    ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)FALSE, (LPARAM)0); //Update display

}

void CAsociacionesBarridoDlg::FinFiltrar(int* arrAsocCompatibles) 
{
    m_BotonFiltrar.EnableWindow(TRUE);
    for (int i= 0; i<theApp.m_asociaciones.m_nAsociaciones;i++)
    {
        if (arrAsocCompatibles[i] > 0)
            m_arrCheck[i].SetCheck(1);
        else
            m_arrCheck[i].SetCheck(0);
    }
}
