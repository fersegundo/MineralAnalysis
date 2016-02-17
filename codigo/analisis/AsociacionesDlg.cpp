// AsociacionesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AsociacionesDlg.h"
#include "AsociacionesMinerales.h"
#include "Minerales.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CAsociacionesDlg::CAsociacionesDlg(UINT nIDTemplate /* = IDD_ASOCIACIONES*/, CWnd* pParent /*=NULL*/) //por defecto, se construye el dialogo "base" IDD_ASOCIACIONES, si se le pasa otro derivado, se construirá ese otro
	: CDialog(nIDTemplate, pParent)
{
	//{{AFX_DATA_INIT(CAsociacionesDlg)
	//}}AFX_DATA_INIT

    m_pAsociaciones           = NULL;
    m_pMinerales              = NULL;   
    m_arrCheck                = NULL;   // lista de controles check box de cada asociacion mineral
}


void CAsociacionesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAsociacionesDlg)
	DDX_Control(pDX, IDC_ASOCIACION_USUARIO, m_checkUsuario);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAsociacionesDlg, CDialog)
	//{{AFX_MSG_MAP(CAsociacionesDlg)
	ON_BN_CLICKED(IDC_DEFINIR_MINERALES, OnDefinirMinerales)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAsociacionesDlg message handlers
BOOL CAsociacionesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	int i;
    CRect rect;

    this->GetClientRect(rect);
    //int nAlto = rect.bottom - rect.top;

    m_checkUsuario.SetCheck((BOOL)m_pAsociaciones->m_bAsociacionUsuario);

    // Poner en negrita "ASOCIACION PERSONALIZADA"
    CFont *fnt;
    LOGFONT lf;
    fnt=m_checkUsuario.GetFont();
    fnt->GetLogFont(&lf);
    lf.lfWeight=FW_BOLD;
    // you can also try other FW_ constants.
    CFont fontBold;
    fontBold.CreateFontIndirect(&lf);
    m_checkUsuario.SetFont(&fontBold);

    BOOL bAlgunMineral = 0;
    if (m_pAsociaciones->m_bAsociacionUsuario)
        bAlgunMineral = 1;
    else
    {
        for (i = 0; i<m_pMinerales->GetCount();i++)
        {
            if (m_pMinerales->m_list[i]->m_bPerteneceAsociacionUsuario)
            {
                bAlgunMineral = 1;
                break;
            }
        }
    }
    // habilitamos check y lo seleccionamos
    m_checkUsuario.EnableWindow(bAlgunMineral);

    for (i= 0; i<m_pAsociaciones->m_nAsociaciones;i++)
    {
        m_arrCheck[i].Create(m_pAsociaciones->m_arrNombresAsociaciones[i],WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX /*| BS_LEFTTEXT*/, 
            CRect(40, rect.bottom + 25*i, 100, rect.bottom + 15 + 25*i), this, CONTROLES_DINAMICOS_AS+i*2);
        m_arrCheck[i].SetCheck(m_pAsociaciones->m_arrCompatibilidadAsociaciones[i]);
        m_arrDescripciones[i].Create(m_pAsociaciones->m_arrDescripcionAsociaciones[i],WS_CHILD | WS_VISIBLE /*| BS_LEFTTEXT*/, 
            CRect(100, rect.bottom + 25*i, rect.right, rect.bottom + 15 + 25*i), this, CONTROLES_DINAMICOS_AS+i*2+1);
    }
    //this->MoveWindow(CRect(40, 100+20*i, 140, 115+20*i));	
    this->SetWindowPos(NULL,0,0,rect.right,rect.bottom + 15 + 25*i + 50, SWP_NOMOVE | SWP_NOZORDER); //+50 para el menu y barra de titulo


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Reserva memoria y recupera el estado de las selecciones
// Devuelve true si hay algun yacimiento seleccionado
bool CAsociacionesDlg::Init(CAsociacionesMinerales* pAsociaciones, CMinerales* pMinerales)
{

    m_pAsociaciones     = pAsociaciones;
    m_pMinerales        = pMinerales;

    m_arrCheck          = new CButton[m_pAsociaciones->m_nAsociaciones];
    m_arrDescripciones  = new CStatic[m_pAsociaciones->m_nAsociaciones];

    m_seleccionMineralesDlg.Init(pMinerales);

    return true;
}

// Guarda las opciones marcadas y libera memoria
void CAsociacionesDlg::Liberar()
{
    // Liberamos memoria
    m_seleccionMineralesDlg.Liberar();

    delete [] m_arrCheck;
    delete [] m_arrDescripciones;
}

void CAsociacionesDlg::OnOK() 
{
    m_pAsociaciones->m_bAsociacionUsuario = m_checkUsuario.GetCheck() == TRUE;

    for (int i= 0; i<m_pAsociaciones->m_nAsociaciones;i++)
    {
        m_pAsociaciones->m_arrCompatibilidadAsociaciones[i] = m_arrCheck[i].GetCheck();
    }
	CDialog::OnOK();
}

void CAsociacionesDlg::OnDefinirMinerales() 
{
	int nResponse = m_seleccionMineralesDlg.DoModal();
	if (nResponse == IDOK)
	{
        BOOL bAlgunMineral = 0;
        for (int i= 0; i<m_pMinerales->GetCount();i++)
        {
            if (m_pMinerales->m_list[i]->m_bPerteneceAsociacionUsuario)
            {
                bAlgunMineral = 1;
                break;
            }
        }
        // habilitamos check y lo seleccionamos
        m_checkUsuario.EnableWindow(bAlgunMineral);
        m_checkUsuario.SetCheck(bAlgunMineral);
	}
	else if (nResponse == IDCANCEL)
	{
	}	
}
